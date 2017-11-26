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

#ifndef INTERNATIONALACCOUNTIDENTIFIERTEST_H
#define INTERNATIONALACCOUNTIDENTIFIERTEST_H

#include <QObject>

#define KMM_MYMONEY_UNIT_TESTABLE friend class internationalAccountIdentifierTest;

class internationalAccountIdentifierTest : public QObject
{
  Q_OBJECT

  bool dataInstalled(const QString& countryCode);

private Q_SLOTS:
  void initTestCase();
  void cleanupTestCase();

  void init();
  void cleanup();

  void comparison();

  void ibanChecksum_data();
  void ibanChecksum();

  void paperformatIban_data();
  void paperformatIban();

  void electronicformatIban_data();
  void electronicformatIban();

  void setIban_data();
  void setIban();

  void setBic_data();
  void setBic();

  void equalOperator_data();
  void equalOperator();

  void uneqalOperator_data();
  void uneqalOperator();

  void getProperties_data();
  void getProperties();

  void iban2bic_data();
  void iban2bic();

  void nameByBic_data();
  void nameByBic();

  void bicAndNameByIban_data();
  void bicAndNameByIban();

  void qStringNullAndEmpty();

  void bicAllocated_data();
  void bicAllocated();
};

#endif // INTERNATIONALACCOUNTIDENTIFIERTEST_H
