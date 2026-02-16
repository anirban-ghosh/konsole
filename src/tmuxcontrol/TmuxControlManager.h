/*
    This source file is part of Konsole, a terminal emulator.

    SPDX-FileCopyrightText: 2026 Anirban Ghosh

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TMUXCONTROLMANAGER_H
#define TMUXCONTROLMANAGER_H

#include <QHash>
#include <QObject>
#include <QPointer>

#include "TmuxControlStateModel.h"
#include "konsoleprivate_export.h"

namespace Konsole
{
class Session;

struct KONSOLEPRIVATE_EXPORT TmuxControlSnapshot {
    QList<TmuxControlSessionState> sessions;
    QList<TmuxControlWindowState> windows;
    QList<TmuxControlPaneState> panes;
};

class KONSOLEPRIVATE_EXPORT TmuxControlManager : public QObject
{
    Q_OBJECT

public:
    explicit TmuxControlManager(QObject *parent = nullptr);

    void watchSession(Session *session);
    void unwatchSession(Session *session);

    TmuxControlSnapshot snapshotForSession(int sessionId) const;

Q_SIGNALS:
    void sessionTmuxControlModeChanged(int sessionId, bool enabled);
    void sessionSnapshotChanged(int sessionId);

private:
    void refreshSnapshot(Session *session);

    QHash<int, QPointer<Session>> _sessions;
    QHash<int, TmuxControlSnapshot> _snapshots;
};
}

#endif // TMUXCONTROLMANAGER_H
