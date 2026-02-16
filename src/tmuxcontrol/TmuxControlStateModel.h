/*
    This source file is part of Konsole, a terminal emulator.

    SPDX-FileCopyrightText: 2026 Anirban Ghosh

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TMUXCONTROLSTATEMODEL_H
#define TMUXCONTROLSTATEMODEL_H

#include <QHash>
#include <QList>
#include <QSet>

#include "TmuxControlTypes.h"

namespace Konsole
{
struct KONSOLEPRIVATE_EXPORT TmuxControlSessionState {
    QByteArray id;
    QByteArray name;
};

struct KONSOLEPRIVATE_EXPORT TmuxControlWindowState {
    QByteArray id;
    QByteArray sessionId;
    int index = -1;
    QByteArray name;
};

struct KONSOLEPRIVATE_EXPORT TmuxControlPaneState {
    QByteArray id;
    QByteArray windowId;
    int index = -1;
    bool active = false;
};

class KONSOLEPRIVATE_EXPORT TmuxControlStateModel
{
public:
    void reset();

    void beginSnapshot(TmuxControlCommandKind kind);
    void applyCommandOutput(TmuxControlCommandKind kind, const QByteArray &line);
    void finalizeSnapshot(TmuxControlCommandKind kind, bool success);

    void applyNotification(const QByteArray &name, const QList<QByteArray> &arguments);

    QList<TmuxControlSessionState> sessions() const;
    QList<TmuxControlWindowState> windows() const;
    QList<TmuxControlPaneState> panes() const;

private:
    static int parseIntOrDefault(const QByteArray &value, int defaultValue);
    static bool parseBoolActive(const QByteArray &value);

    QHash<QByteArray, TmuxControlSessionState> _sessions;
    QHash<QByteArray, TmuxControlWindowState> _windows;
    QHash<QByteArray, TmuxControlPaneState> _panes;

    QHash<QByteArray, TmuxControlSessionState> _snapshotSessions;
    QHash<QByteArray, TmuxControlWindowState> _snapshotWindows;
    QHash<QByteArray, TmuxControlPaneState> _snapshotPanes;

    QSet<TmuxControlCommandKind> _activeSnapshots;
};
}

#endif // TMUXCONTROLSTATEMODEL_H
