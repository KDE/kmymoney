/*
    SPDX-FileCopyrightText: 2014 Christian Dávid <christian-david@web.de>
    SPDX-FileCopyrightText: 2022 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "unavailableplugin.h"

#include <typeinfo>

namespace payeeIdentifiers
{

payeeIdentifierUnavailable::payeeIdentifierUnavailable()
    : payeeIdentifierData()
{
}

payeeIdentifierUnavailable::payeeIdentifierUnavailable(const QString& data)
    : payeeIdentifierData()
    , m_data(data)
{
}

payeeIdentifierUnavailable* payeeIdentifierUnavailable::clone() const
{
    return new payeeIdentifierUnavailable(m_data);
}

void payeeIdentifierUnavailable::writeXML(QXmlStreamWriter* writer) const
{
    Q_UNUSED(writer)
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

payeeIdentifierUnavailable* payeeIdentifierUnavailable::createFromXml(QXmlStreamReader* reader) const
{
    QString payeeIdentifier;
    QXmlStreamWriter writer(&payeeIdentifier);

    writer.setAutoFormatting(false);
    writer.writeStartDocument();
    writer.writeStartElement(reader->name().toString());
    writer.writeAttributes(reader->attributes());
    addElements(reader, &writer);
    writer.writeEndElement();
    writer.writeEndDocument();

    return new payeeIdentifierUnavailable(payeeIdentifier);
}

bool payeeIdentifierUnavailable::isValid() const
{
    return false;
}

bool payeeIdentifierUnavailable::operator==(const payeeIdentifierData& other) const
{
    if (payeeIdentifierId() == other.payeeIdentifierId()) {
        try {
            const payeeIdentifierUnavailable& otherCasted = dynamic_cast<const payeeIdentifierUnavailable&>(other);
            return operator==(otherCasted);
        } catch (const std::bad_cast&) {
        }
    }
    return false;
}

bool payeeIdentifierUnavailable::operator==(const payeeIdentifierUnavailable& other) const
{
    return (m_data == other.m_data);
}


} // namespace payeeIdentifiers
