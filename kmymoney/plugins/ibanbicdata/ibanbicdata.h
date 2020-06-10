/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2014 Christian Dávid <christian-david@web.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef IBANBICDATA_H
#define IBANBICDATA_H

#ifndef KMM_MYMONEY_UNIT_TESTABLE
#  define KMM_MYMONEY_UNIT_TESTABLE
#endif

#include "kmymoneyplugin.h"

#include <QObject>
#include <QVariant>
#include <QSqlDatabase>

namespace eIBANBIC {enum bicAllocationStatus : unsigned int;}

/**
 * @brief This class implements everything that needs lookup
 *
 * Kind of a static private class of payeeIdentifier::ibanBic. It loads the iban/bic data and queries the
 * databases.
 *
 * @internal This class is made if a cache will be needed in future.
 */
class ibanBicData : public KMyMoneyPlugin::Plugin, public KMyMoneyPlugin::DataPlugin
{
  Q_OBJECT
  Q_INTERFACES(KMyMoneyPlugin::DataPlugin)
  KMM_MYMONEY_UNIT_TESTABLE

public:
    explicit ibanBicData(QObject *parent, const QVariantList &args);
  ~ibanBicData() override;

  QVariant requestData(const QString &arg, uint type) override;

  int bbanLength(const QString& countryCode);
  int bankIdentifierPosition(const QString& countryCode);
  int bankIdentifierLength(const QString& countryCode);

  /**
   * @brief Create a BIC from a given IBAN
   *
   * The bic is always 11 characters long.
   *
   * @return QString::isNull() == true means an internal error occurred, QString::isEmpty() == true means there is no BIC
   */
  QString iban2Bic(const QString& iban);

  QString bankNameByBic(QString bic);

  /**
   * @brief Create a BIC from a IBAN and get the institutes name
   *
   * first: bic, always 11 characters long.
   * second: institution name
   *
   * QString::isNull() == true means an internal error occurred, QString::isEmpty() == true means there is no data
   */
  QPair<QString, QString> bankNameAndBic(const QString& iban);

  QString extractBankIdentifier(const QString& iban);

  eIBANBIC::bicAllocationStatus isBicAllocated(const QString& bic);

private:
  QVariant findPropertyByCountry(const QString& countryCode, const QString& property, const QVariant::Type type);

  /**
   * @brief Create/get QSqlDatabase
   *
   * Returns a QSqlDatabase. It is invalid if something went wrong.
   *
   * @param database This string is used to locate the database in the data dir
   */
  QSqlDatabase createDatabaseConnection(const QString& database);
};

#endif // IBANBICDATA_H
