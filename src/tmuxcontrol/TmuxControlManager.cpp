/*
    This source file is part of Konsole, a terminal emulator.

    SPDX-FileCopyrightText: 2026 Anirban Ghosh

    SPDX-License-Identifier: GPL-2.0-or-later
*/

// Own
#include "TmuxControlManager.h"

// Konsole
#include "session/Session.h"

using namespace Konsole;

TmuxControlManager::TmuxControlManager(QObject *parent)
    : QObject(parent)
{
}

void TmuxControlManager::watchSession(Session *session)
{
    if (session == nullptr) {
        return;
    }

    const int sessionId = session->sessionId();
    if (_sessions.value(sessionId) == session) {
        return;
    }

    _sessions.insert(sessionId, session);
    refreshSnapshot(session);

    connect(session, &Session::tmuxControlModeChanged, this, [this, sessionId](bool enabled) {
        Q_EMIT sessionTmuxControlModeChanged(sessionId, enabled);
    });

    connect(session, &Session::tmuxControlStateChanged, this, [this, session]() {
        refreshSnapshot(session);
    });

    connect(session, &Session::finished, this, [this](Session *finishedSession) {
        unwatchSession(finishedSession);
    });
}

void TmuxControlManager::unwatchSession(Session *session)
{
    if (session == nullptr) {
        return;
    }

    const int sessionId = session->sessionId();
    _sessions.remove(sessionId);
    _snapshots.remove(sessionId);
}

TmuxControlSnapshot TmuxControlManager::snapshotForSession(int sessionId) const
{
    return _snapshots.value(sessionId);
}

void TmuxControlManager::refreshSnapshot(Session *session)
{
    if (session == nullptr) {
        return;
    }

    const int sessionId = session->sessionId();
    TmuxControlSnapshot snapshot;
    snapshot.sessions = session->tmuxControlSessions();
    snapshot.windows = session->tmuxControlWindows();
    snapshot.panes = session->tmuxControlPanes();
    _snapshots.insert(sessionId, snapshot);
    Q_EMIT sessionSnapshotChanged(sessionId);
}
