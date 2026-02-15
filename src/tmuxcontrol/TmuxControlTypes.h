/*
    This source file is part of Konsole, a terminal emulator.

    SPDX-FileCopyrightText: 2026 Anirban Ghosh

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TMUXCONTROLTYPES_H
#define TMUXCONTROLTYPES_H

#include <QByteArray>
#include <QList>

#include "konsoleprivate_export.h"

namespace Konsole
{
enum class TmuxControlEventType {
    CommandBegin,
    CommandOutput,
    CommandEnd,
    CommandError,
    Notification,
    ParseError,
};

struct KONSOLEPRIVATE_EXPORT TmuxControlEnvelope {
    qint64 epochSeconds = 0;
    quint64 commandNumber = 0;
    quint64 flags = 0;
};

struct KONSOLEPRIVATE_EXPORT TmuxControlEvent {
    TmuxControlEventType type = TmuxControlEventType::ParseError;
    TmuxControlEnvelope envelope;
    QByteArray payload;
    QByteArray notificationName;
    QList<QByteArray> arguments;
};
}

#endif // TMUXCONTROLTYPES_H
