/*
    SPDX-FileCopyrightText: 2026 Anirban Ghosh

    SPDX-License-Identifier: GPL-2.0-or-later
*/

// Own
#include "TmuxControlTest.h"

// Qt
#include <QTest>

// Konsole
#include "../tmuxcontrol/TmuxControlCommandQueue.h"
#include "../tmuxcontrol/TmuxControlParser.h"
#include "../tmuxcontrol/TmuxControlStateModel.h"

using namespace Konsole;

void TmuxControlTest::parsesCommandEnvelopeAndOutput()
{
    TmuxControlParser parser;
    const QList<TmuxControlEvent> events = parser.feed("%begin 1700000000 7 0\nfirst\nsecond\n%end 1700000000 7 0\n");

    QCOMPARE(events.size(), 4);
    QCOMPARE(events.at(0).type, TmuxControlEventType::CommandBegin);
    QCOMPARE(events.at(0).envelope.commandNumber, 7ULL);

    QCOMPARE(events.at(1).type, TmuxControlEventType::CommandOutput);
    QCOMPARE(events.at(1).payload, QByteArrayLiteral("first"));

    QCOMPARE(events.at(2).type, TmuxControlEventType::CommandOutput);
    QCOMPARE(events.at(2).payload, QByteArrayLiteral("second"));

    QCOMPARE(events.at(3).type, TmuxControlEventType::CommandEnd);
    QCOMPARE(events.at(3).envelope.commandNumber, 7ULL);
    QCOMPARE(parser.inCommandBlock(), false);
}

void TmuxControlTest::parsesErrorEnvelope()
{
    TmuxControlParser parser;
    const QList<TmuxControlEvent> events = parser.feed("%begin 1700000000 8 0\nbad\n%error 1700000000 8 0\n");

    QCOMPARE(events.size(), 3);
    QCOMPARE(events.at(0).type, TmuxControlEventType::CommandBegin);
    QCOMPARE(events.at(1).type, TmuxControlEventType::CommandOutput);
    QCOMPARE(events.at(2).type, TmuxControlEventType::CommandError);
    QCOMPARE(events.at(2).envelope.commandNumber, 8ULL);
    QCOMPARE(parser.inCommandBlock(), false);
}

void TmuxControlTest::parsesNotification()
{
    TmuxControlParser parser;
    const QList<TmuxControlEvent> events = parser.feed("%window-add @12\n");

    QCOMPARE(events.size(), 1);
    QCOMPARE(events.at(0).type, TmuxControlEventType::Notification);
    QCOMPARE(events.at(0).notificationName, QByteArrayLiteral("window-add"));
    QCOMPARE(events.at(0).arguments.size(), 1);
    QCOMPARE(events.at(0).arguments.at(0), QByteArrayLiteral("@12"));
}

void TmuxControlTest::parsesOutputNotificationPayload()
{
    TmuxControlParser parser;
    const QList<TmuxControlEvent> events = parser.feed("%output %1 hello\\040world\\012\n");

    QCOMPARE(events.size(), 1);
    QCOMPARE(events.at(0).type, TmuxControlEventType::Notification);
    QCOMPARE(events.at(0).notificationName, QByteArrayLiteral("output"));
    QCOMPARE(events.at(0).arguments.size(), 2);
    QCOMPARE(events.at(0).arguments.at(0), QByteArrayLiteral("%1"));
    QCOMPARE(events.at(0).arguments.at(1), QByteArrayLiteral("hello\\040world\\012"));
}

void TmuxControlTest::parsesSplitInputChunks()
{
    TmuxControlParser parser;
    QList<TmuxControlEvent> events = parser.feed("%begin 1700000000 10 0\nhel");
    QCOMPARE(events.size(), 1);
    QCOMPARE(events.at(0).type, TmuxControlEventType::CommandBegin);

    events = parser.feed("lo\n%end 1700000000 10 0\n");
    QCOMPARE(events.size(), 2);
    QCOMPARE(events.at(0).type, TmuxControlEventType::CommandOutput);
    QCOMPARE(events.at(0).payload, QByteArrayLiteral("hello"));
    QCOMPARE(events.at(1).type, TmuxControlEventType::CommandEnd);
    QCOMPARE(events.at(1).envelope.commandNumber, 10ULL);
}

void TmuxControlTest::queuesMonotonicCommands()
{
    TmuxControlCommandQueue queue;

    const auto first = queue.enqueue(QByteArrayLiteral("list-sessions"));
    const auto second = queue.enqueue(QByteArrayLiteral("list-windows\r\n"));

    QCOMPARE(first.commandNumber, 1ULL);
    QCOMPARE(second.commandNumber, 2ULL);
    QCOMPARE(first.wireCommand, QByteArrayLiteral("list-sessions\n"));
    QCOMPARE(second.wireCommand, QByteArrayLiteral("list-windows\n"));

    QVERIFY(queue.hasQueuedCommands());
    QCOMPARE(queue.takeNextQueuedCommand().commandNumber, 1ULL);
    QCOMPARE(queue.takeNextQueuedCommand().commandNumber, 2ULL);
    QCOMPARE(queue.hasQueuedCommands(), false);

    QVERIFY(queue.isAwaitingReply(1ULL));
    QVERIFY(queue.markReplyReceived(1ULL));
    QCOMPARE(queue.isAwaitingReply(1ULL), false);
    QVERIFY(queue.isAwaitingReply(2ULL));
}

void TmuxControlTest::buildsStateSnapshotFromListCommands()
{
    TmuxControlStateModel stateModel;

    stateModel.beginSnapshot(TmuxControlCommandKind::ListSessions);
    stateModel.applyCommandOutput(TmuxControlCommandKind::ListSessions, QByteArrayLiteral("$1\talpha"));
    stateModel.applyCommandOutput(TmuxControlCommandKind::ListSessions, QByteArrayLiteral("$2\tbeta"));
    stateModel.finalizeSnapshot(TmuxControlCommandKind::ListSessions, true);

    stateModel.beginSnapshot(TmuxControlCommandKind::ListWindows);
    stateModel.applyCommandOutput(TmuxControlCommandKind::ListWindows, QByteArrayLiteral("$1\t@10\t0\tmain"));
    stateModel.applyCommandOutput(TmuxControlCommandKind::ListWindows, QByteArrayLiteral("$2\t@11\t1\tops board"));
    stateModel.finalizeSnapshot(TmuxControlCommandKind::ListWindows, true);

    stateModel.beginSnapshot(TmuxControlCommandKind::ListPanes);
    stateModel.applyCommandOutput(TmuxControlCommandKind::ListPanes, QByteArrayLiteral("@10\t%100\t0\t1"));
    stateModel.applyCommandOutput(TmuxControlCommandKind::ListPanes, QByteArrayLiteral("@11\t%101\t1\t0"));
    stateModel.finalizeSnapshot(TmuxControlCommandKind::ListPanes, true);

    const auto sessions = stateModel.sessions();
    const auto windows = stateModel.windows();
    const auto panes = stateModel.panes();

    QCOMPARE(sessions.size(), 2);
    QCOMPARE(windows.size(), 2);
    QCOMPARE(panes.size(), 2);
}

void TmuxControlTest::updatesStateFromNotifications()
{
    TmuxControlStateModel stateModel;

    stateModel.beginSnapshot(TmuxControlCommandKind::ListSessions);
    stateModel.applyCommandOutput(TmuxControlCommandKind::ListSessions, QByteArrayLiteral("$1\talpha"));
    stateModel.finalizeSnapshot(TmuxControlCommandKind::ListSessions, true);

    stateModel.beginSnapshot(TmuxControlCommandKind::ListWindows);
    stateModel.applyCommandOutput(TmuxControlCommandKind::ListWindows, QByteArrayLiteral("$1\t@10\t0\tmain"));
    stateModel.finalizeSnapshot(TmuxControlCommandKind::ListWindows, true);

    stateModel.beginSnapshot(TmuxControlCommandKind::ListPanes);
    stateModel.applyCommandOutput(TmuxControlCommandKind::ListPanes, QByteArrayLiteral("@10\t%100\t0\t1"));
    stateModel.applyCommandOutput(TmuxControlCommandKind::ListPanes, QByteArrayLiteral("@10\t%101\t1\t0"));
    stateModel.finalizeSnapshot(TmuxControlCommandKind::ListPanes, true);

    stateModel.applyNotification(QByteArrayLiteral("window-renamed"), {QByteArrayLiteral("@10"), QByteArrayLiteral("dev")});
    stateModel.applyNotification(QByteArrayLiteral("pane-focus-in"), {QByteArrayLiteral("%101")});
    stateModel.applyNotification(QByteArrayLiteral("pane-exited"), {QByteArrayLiteral("%100")});

    const auto windows = stateModel.windows();
    const auto panes = stateModel.panes();

    QCOMPARE(windows.size(), 1);
    QCOMPARE(windows.at(0).name, QByteArrayLiteral("dev"));
    QCOMPARE(panes.size(), 1);
    QCOMPARE(panes.at(0).id, QByteArrayLiteral("%101"));
    QCOMPARE(panes.at(0).active, true);
}

QTEST_GUILESS_MAIN(TmuxControlTest)

#include "moc_TmuxControlTest.cpp"
