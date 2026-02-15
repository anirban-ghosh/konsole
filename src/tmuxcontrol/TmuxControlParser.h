/*
    This source file is part of Konsole, a terminal emulator.

    SPDX-FileCopyrightText: 2026 Anirban Ghosh

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TMUXCONTROLPARSER_H
#define TMUXCONTROLPARSER_H

#include "TmuxControlTypes.h"

namespace Konsole
{
class KONSOLEPRIVATE_EXPORT TmuxControlParser
{
public:
    QList<TmuxControlEvent> feed(const QByteArray &data);
    QList<TmuxControlEvent> flush();

    bool inCommandBlock() const;

private:
    static bool parseEnvelope(const QByteArray &line, const QByteArray &prefix, TmuxControlEnvelope &envelope);
    static TmuxControlEvent parseNotification(const QByteArray &line);

    QList<TmuxControlEvent> parseLine(const QByteArray &line);
    static QByteArray normalizeLine(const QByteArray &line);

    QByteArray _buffer;
    bool _inCommandBlock = false;
    TmuxControlEnvelope _currentEnvelope;
};
}

#endif // TMUXCONTROLPARSER_H
