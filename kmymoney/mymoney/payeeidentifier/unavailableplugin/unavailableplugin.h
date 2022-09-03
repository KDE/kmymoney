/*
    SPDX-FileCopyrightText: 2014 Christian Dávid <christian-david@web.de>
    SPDX-FileCopyrightText: 2022 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef UNAVAILABLEPLUGIN_H
#define UNAVAILABLEPLUGIN_H

#include "payeeidentifier/payeeidentifierdata.h"

class payeeIdentifierLoader;

namespace payeeIdentifiers
{

/**
 * @brief A payeeIdentifier which is used to store the plain xml data
 *
 * To avoid data loss if a plugin could not be loaded this payeeIdentifier is used in the xml backend.
 * It stores the data in plain xml so it can be written back to the file.
 */
class payeeIdentifierUnavailable : public payeeIdentifierData
{
public:
    PAYEEIDENTIFIER_IID(payeeIdentifierUnavailable, "org.kmymoney.payeeIdentifier.payeeIdentifierUnavailable");

    payeeIdentifierUnavailable();
    void writeXML(QXmlStreamWriter* writer) const final override;
    payeeIdentifierUnavailable* createFromXml(QXmlStreamReader* reader) const final override;
    bool isValid() const final override;
    bool operator==(const payeeIdentifierData& other) const final override;
    bool operator==(const payeeIdentifierUnavailable& other) const;

    friend class payeeIdentifierLoader;
    /** @todo make private */
    explicit payeeIdentifierUnavailable(const QString& data);

protected:
    payeeIdentifierUnavailable* clone() const final override;

private:
    QString m_data;
};

} // namespace payeeidentifiers

#endif // UNAVAILABLEPLUGIN_H
