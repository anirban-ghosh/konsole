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

QTEST_GUILESS_MAIN(TmuxControlTest)

#include "moc_TmuxControlTest.cpp"
