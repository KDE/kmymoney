/*
    SPDX-FileCopyrightText: 2014 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
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
