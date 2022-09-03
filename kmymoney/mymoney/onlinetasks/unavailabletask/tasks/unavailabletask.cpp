/*

    SPDX-FileCopyrightText: 2014 Christian Dávid <christian-david@web.de>
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "unavailabletask.h"

#include <QSet>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <KLocalizedString>

unavailableTask::unavailableTask(const QString& xmlData)
    : m_data(xmlData)
{
}

QString unavailableTask::jobTypeName() const
{
    return i18n("Could not load responsible plugin to view this task.");
}

QString unavailableTask::responsibleAccount() const
{
    return QString();
}

QString unavailableTask::purpose() const
{
    return QString();
}

static void addElements(QXmlStreamReader* reader, QXmlStreamWriter* writer)
{
    while (reader->readNextStartElement()) {
        writer->writeStartElement(reader->name().toString());
        writer->writeAttributes(reader->attributes());
        addElements(reader, writer);
        writer->writeEndElement();
    }
}

unavailableTask* unavailableTask::createFromXml(QXmlStreamReader* reader) const
{
    QString taskXml;
    QXmlStreamWriter writer(&taskXml);

    writer.setAutoFormatting(false);
    writer.writeStartDocument();
    writer.writeStartElement(reader->name().toString());
    writer.writeAttributes(reader->attributes());
    addElements(reader, &writer);
    writer.writeEndElement();
    writer.writeEndDocument();

    return new unavailableTask(taskXml);
}

void unavailableTask::writeXML(QXmlStreamWriter* writer) const
{
    Q_UNUSED(writer)
}

bool unavailableTask::hasReferenceTo(const QString& id) const
{
    Q_UNUSED(id);
    return false;
}

QSet<QString> unavailableTask::referencedObjects() const
{
    return {};
}

unavailableTask* unavailableTask::clone() const
{
    return new unavailableTask(m_data);
}

bool unavailableTask::isValid() const
{
    return true;
}
