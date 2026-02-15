/*
    SPDX-FileCopyrightText: 2026 Anirban Ghosh

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TMUXCONTROLTEST_H
#define TMUXCONTROLTEST_H

#include <QObject>

namespace Konsole
{
class TmuxControlTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void parsesCommandEnvelopeAndOutput();
    void parsesErrorEnvelope();
    void parsesNotification();
    void parsesSplitInputChunks();
    void queuesMonotonicCommands();
};
}

#endif // TMUXCONTROLTEST_H
