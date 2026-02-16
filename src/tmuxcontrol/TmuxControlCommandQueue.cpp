/*
    This source file is part of Konsole, a terminal emulator.

    SPDX-FileCopyrightText: 2026 Anirban Ghosh

    SPDX-License-Identifier: GPL-2.0-or-later
*/

// Own
#include "TmuxControlCommandQueue.h"

using namespace Konsole;

TmuxControlPendingCommand TmuxControlCommandQueue::enqueue(const QByteArray &command, TmuxControlCommandKind kind)
{
    TmuxControlPendingCommand pendingCommand;
    pendingCommand.commandNumber = _nextCommandNumber++;
    pendingCommand.wireCommand = normalizeCommand(command);
    pendingCommand.kind = kind;

    _queuedCommands.enqueue(pendingCommand);
    _awaitingReplies.insert(pendingCommand.commandNumber);
    return pendingCommand;
}

bool TmuxControlCommandQueue::hasQueuedCommands() const
{
    return !_queuedCommands.isEmpty();
}

TmuxControlPendingCommand TmuxControlCommandQueue::takeNextQueuedCommand()
{
    if (_queuedCommands.isEmpty()) {
        return {};
    }
    return _queuedCommands.dequeue();
}

bool TmuxControlCommandQueue::isAwaitingReply(quint64 commandNumber) const
{
    return _awaitingReplies.contains(commandNumber);
}

bool TmuxControlCommandQueue::markReplyReceived(quint64 commandNumber)
{
    return _awaitingReplies.remove(commandNumber) > 0;
}

QByteArray TmuxControlCommandQueue::normalizeCommand(const QByteArray &command)
{
    QByteArray normalized = command;
    while (!normalized.isEmpty() && (normalized.endsWith('\n') || normalized.endsWith('\r'))) {
        normalized.chop(1);
    }
    normalized.append('\n');
    return normalized;
}
