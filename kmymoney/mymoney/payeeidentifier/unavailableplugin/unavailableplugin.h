/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * SPDX-FileCopyrightText: 2014 Christian Dávid <christian-david@web.de>
 *
 *SPDX-License-Identifier: GPL-2.0-or-laterrg/licenses/>.
 */

#ifndef UNAVAILABLEPLUGIN_H
#define UNAVAILABLEPLUGIN_H

#include "payeeidentifier/payeeidentifierdata.h"
#include <QDomElement>

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
  void writeXML(QDomDocument& document, QDomElement& parent) const final override;
  payeeIdentifierUnavailable* createFromXml(const QDomElement& element) const final override;
  bool isValid() const final override;
  bool operator==(const payeeIdentifierData& other) const final override;
  bool operator==(const payeeIdentifierUnavailable& other) const;

  friend class payeeIdentifierLoader;
  /** @todo make private */
  explicit payeeIdentifierUnavailable(QDomElement data);

protected:
  payeeIdentifierUnavailable* clone() const final override;

private:
  QDomElement m_data;
};

} // namespace payeeidentifiers

#endif // UNAVAILABLEPLUGIN_H
