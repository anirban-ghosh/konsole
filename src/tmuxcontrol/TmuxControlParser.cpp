/*
    This source file is part of Konsole, a terminal emulator.

    SPDX-FileCopyrightText: 2026 Anirban Ghosh

    SPDX-License-Identifier: GPL-2.0-or-later
*/

// Own
#include "TmuxControlParser.h"

using namespace Konsole;

QList<TmuxControlEvent> TmuxControlParser::feed(const QByteArray &data)
{
    _buffer.append(data);

    QList<TmuxControlEvent> events;
    qsizetype newlineIndex = _buffer.indexOf('\n');
    while (newlineIndex >= 0) {
        const QByteArray line = _buffer.first(newlineIndex);
        _buffer.remove(0, newlineIndex + 1);
        events.append(parseLine(normalizeLine(line)));
        newlineIndex = _buffer.indexOf('\n');
    }

    return events;
}

QList<TmuxControlEvent> TmuxControlParser::flush()
{
    QList<TmuxControlEvent> events;
    if (!_buffer.isEmpty()) {
        events.append(parseLine(normalizeLine(_buffer)));
        _buffer.clear();
    }
    return events;
}

bool TmuxControlParser::inCommandBlock() const
{
    return _inCommandBlock;
}

QByteArray TmuxControlParser::normalizeLine(const QByteArray &line)
{
    if (!line.isEmpty() && line.endsWith('\r')) {
        return line.first(line.size() - 1);
    }
    return line;
}

bool TmuxControlParser::parseEnvelope(const QByteArray &line, const QByteArray &prefix, TmuxControlEnvelope &envelope)
{
    if (!line.startsWith(prefix)) {
        return false;
    }

    const QList<QByteArray> tokens = line.mid(prefix.size()).split(' ');
    if (tokens.size() != 3) {
        return false;
    }

    bool epochOk = false;
    bool commandOk = false;
    bool flagsOk = false;

    envelope.epochSeconds = tokens.at(0).toLongLong(&epochOk, 10);
    envelope.commandNumber = tokens.at(1).toULongLong(&commandOk, 10);
    envelope.flags = tokens.at(2).toULongLong(&flagsOk, 10);

    return epochOk && commandOk && flagsOk;
}

TmuxControlEvent TmuxControlParser::parseNotification(const QByteArray &line)
{
    TmuxControlEvent event;
    event.type = TmuxControlEventType::Notification;

    const int firstSpace = line.indexOf(' ');
    if (firstSpace < 0) {
        event.notificationName = line.mid(1);
        return event;
    }

    event.notificationName = line.mid(1, firstSpace - 1);
    const QByteArray rest = line.mid(firstSpace + 1);
    if (!rest.isEmpty()) {
        event.arguments = rest.split(' ');
    }
    return event;
}

QList<TmuxControlEvent> TmuxControlParser::parseLine(const QByteArray &line)
{
    QList<TmuxControlEvent> events;
    if (line.isEmpty()) {
        return events;
    }

    TmuxControlEnvelope envelope;
    if (parseEnvelope(line, QByteArrayLiteral("%begin "), envelope)) {
        if (_inCommandBlock) {
            TmuxControlEvent parseError;
            parseError.type = TmuxControlEventType::ParseError;
            parseError.payload = QByteArrayLiteral("Nested %begin received while command block is open");
            events.append(parseError);
        }
        _inCommandBlock = true;
        _currentEnvelope = envelope;

        TmuxControlEvent beginEvent;
        beginEvent.type = TmuxControlEventType::CommandBegin;
        beginEvent.envelope = envelope;
        events.append(beginEvent);
        return events;
    }

    if (parseEnvelope(line, QByteArrayLiteral("%end "), envelope)) {
        if (!_inCommandBlock) {
            TmuxControlEvent parseError;
            parseError.type = TmuxControlEventType::ParseError;
            parseError.payload = QByteArrayLiteral("Unexpected %end outside command block");
            events.append(parseError);
            return events;
        }

        _inCommandBlock = false;
        TmuxControlEvent endEvent;
        endEvent.type = TmuxControlEventType::CommandEnd;
        endEvent.envelope = envelope;
        events.append(endEvent);
        return events;
    }

    if (parseEnvelope(line, QByteArrayLiteral("%error "), envelope)) {
        if (!_inCommandBlock) {
            TmuxControlEvent parseError;
            parseError.type = TmuxControlEventType::ParseError;
            parseError.payload = QByteArrayLiteral("Unexpected %error outside command block");
            events.append(parseError);
            return events;
        }

        _inCommandBlock = false;
        TmuxControlEvent errorEvent;
        errorEvent.type = TmuxControlEventType::CommandError;
        errorEvent.envelope = envelope;
        events.append(errorEvent);
        return events;
    }

    if (line.startsWith('%')) {
        events.append(parseNotification(line));
        return events;
    }

    if (_inCommandBlock) {
        TmuxControlEvent outputEvent;
        outputEvent.type = TmuxControlEventType::CommandOutput;
        outputEvent.envelope = _currentEnvelope;
        outputEvent.payload = line;
        events.append(outputEvent);
        return events;
    }

    TmuxControlEvent parseError;
    parseError.type = TmuxControlEventType::ParseError;
    parseError.payload = QByteArrayLiteral("Unexpected line outside command block: ") + line;
    events.append(parseError);
    return events;
}
