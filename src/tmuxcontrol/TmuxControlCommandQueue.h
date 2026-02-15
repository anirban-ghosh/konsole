/*
    This source file is part of Konsole, a terminal emulator.

    SPDX-FileCopyrightText: 2026 Anirban Ghosh

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TMUXCONTROLCOMMANDQUEUE_H
#define TMUXCONTROLCOMMANDQUEUE_H

#include <QByteArray>
#include <QQueue>
#include <QSet>

#include "konsoleprivate_export.h"

namespace Konsole
{
struct KONSOLEPRIVATE_EXPORT TmuxControlPendingCommand {
    quint64 commandNumber = 0;
    QByteArray wireCommand;
};

class KONSOLEPRIVATE_EXPORT TmuxControlCommandQueue
{
public:
    TmuxControlPendingCommand enqueue(const QByteArray &command);
    bool hasQueuedCommands() const;
    TmuxControlPendingCommand takeNextQueuedCommand();

    bool isAwaitingReply(quint64 commandNumber) const;
    bool markReplyReceived(quint64 commandNumber);

private:
    static QByteArray normalizeCommand(const QByteArray &command);

    quint64 _nextCommandNumber = 1;
    QQueue<TmuxControlPendingCommand> _queuedCommands;
    QSet<quint64> _awaitingReplies;
};
}

#endif // TMUXCONTROLCOMMANDQUEUE_H
