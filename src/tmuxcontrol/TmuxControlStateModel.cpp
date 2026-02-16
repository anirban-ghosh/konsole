/*
    This source file is part of Konsole, a terminal emulator.

    SPDX-FileCopyrightText: 2026 Anirban Ghosh

    SPDX-License-Identifier: GPL-2.0-or-later
*/

// Own
#include "TmuxControlStateModel.h"

using namespace Konsole;

void TmuxControlStateModel::reset()
{
    _sessions.clear();
    _windows.clear();
    _panes.clear();
    _snapshotSessions.clear();
    _snapshotWindows.clear();
    _snapshotPanes.clear();
    _activeSnapshots.clear();
}

void TmuxControlStateModel::beginSnapshot(TmuxControlCommandKind kind)
{
    _activeSnapshots.insert(kind);

    if (kind == TmuxControlCommandKind::ListSessions) {
        _snapshotSessions.clear();
    } else if (kind == TmuxControlCommandKind::ListWindows) {
        _snapshotWindows.clear();
    } else if (kind == TmuxControlCommandKind::ListPanes) {
        _snapshotPanes.clear();
    }
}

void TmuxControlStateModel::applyCommandOutput(TmuxControlCommandKind kind, const QByteArray &line)
{
    if (line.isEmpty()) {
        return;
    }

    const QList<QByteArray> fields = line.split('\t');
    if (kind == TmuxControlCommandKind::ListSessions) {
        if (fields.size() < 2) {
            return;
        }

        TmuxControlSessionState session;
        session.id = fields.at(0);
        session.name = fields.at(1);
        if (!session.id.isEmpty()) {
            _snapshotSessions.insert(session.id, session);
        }
        return;
    }

    if (kind == TmuxControlCommandKind::ListWindows) {
        if (fields.size() < 4) {
            return;
        }

        TmuxControlWindowState window;
        window.sessionId = fields.at(0);
        window.id = fields.at(1);
        window.index = parseIntOrDefault(fields.at(2), -1);
        window.name = fields.at(3);
        if (!window.id.isEmpty()) {
            _snapshotWindows.insert(window.id, window);
        }
        return;
    }

    if (kind == TmuxControlCommandKind::ListPanes) {
        if (fields.size() < 4) {
            return;
        }

        TmuxControlPaneState pane;
        pane.windowId = fields.at(0);
        pane.id = fields.at(1);
        pane.index = parseIntOrDefault(fields.at(2), -1);
        pane.active = parseBoolActive(fields.at(3));
        if (!pane.id.isEmpty()) {
            _snapshotPanes.insert(pane.id, pane);
        }
    }
}

void TmuxControlStateModel::finalizeSnapshot(TmuxControlCommandKind kind, bool success)
{
    _activeSnapshots.remove(kind);
    if (!success) {
        return;
    }

    if (kind == TmuxControlCommandKind::ListSessions) {
        _sessions = _snapshotSessions;
    } else if (kind == TmuxControlCommandKind::ListWindows) {
        _windows = _snapshotWindows;
    } else if (kind == TmuxControlCommandKind::ListPanes) {
        _panes = _snapshotPanes;
    }
}

void TmuxControlStateModel::applyNotification(const QByteArray &name, const QList<QByteArray> &arguments)
{
    if ((name == "session-closed" || name == "session-renamed") && !arguments.isEmpty()) {
        _sessions.remove(arguments.first());
        return;
    }

    if ((name == "window-close" || name == "window-renamed") && !arguments.isEmpty()) {
        const QByteArray windowId = arguments.first();
        _windows.remove(windowId);
        for (auto it = _panes.begin(); it != _panes.end();) {
            if (it->windowId == windowId) {
                it = _panes.erase(it);
            } else {
                ++it;
            }
        }
        return;
    }

    if ((name == "pane-died" || name == "pane-exited") && !arguments.isEmpty()) {
        _panes.remove(arguments.first());
    }
}

QList<TmuxControlSessionState> TmuxControlStateModel::sessions() const
{
    return _sessions.values();
}

QList<TmuxControlWindowState> TmuxControlStateModel::windows() const
{
    return _windows.values();
}

QList<TmuxControlPaneState> TmuxControlStateModel::panes() const
{
    return _panes.values();
}

int TmuxControlStateModel::parseIntOrDefault(const QByteArray &value, int defaultValue)
{
    bool ok = false;
    const int parsed = value.toInt(&ok, 10);
    return ok ? parsed : defaultValue;
}

bool TmuxControlStateModel::parseBoolActive(const QByteArray &value)
{
    return value == "1" || value == "true" || value == "on";
}
