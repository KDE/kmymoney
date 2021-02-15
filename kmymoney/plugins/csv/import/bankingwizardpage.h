/*
 * SPDX-FileCopyrightText: 2011-2017 Allan Anderson <agander93@gmail.com>
 * SPDX-FileCopyrightText: 2016-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef BANKINGWIZARDPAGE_H
#define BANKINGWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "csvwizardpage.h"

// ----------------------------------------------------------------------------

class BankingProfile;
class MyMoneyStatement;

namespace Ui
{
class BankingPage;
}

class BankingPage : public CSVWizardPage
{
  Q_OBJECT

public:
  explicit BankingPage(CSVWizard *dlg, CSVImporterCore *imp);
  ~BankingPage();

  bool validateCreditDebit();
  /**
  * This method fills QIF file with bank/credit card data
  */
  void                makeQIF(const MyMoneyStatement &st, const QString &outFileName);

private:
  void initializePage() final override;
  bool isComplete() const final override;
  int nextId() const final override;

  bool                validateMemoComboBox();
  void                resetComboBox(const Column comboBox);
  bool                validateSelectedColumn(const int col, const Column type);

  BankingProfile       *m_profile;
  Ui::BankingPage      *ui;

  void                memoColSelected(int col);
  void                categoryColSelected(int col);
  void                numberColSelected(int col);
  void                payeeColSelected(int col);
  void                dateColSelected(int col);
  void                debitColSelected(int col);
  void                creditColSelected(int col);
  void                amountColSelected(int col);
  void                amountToggled(bool checked);
  void                debitCreditToggled(bool checked);
  void                oppositeSignsClicked(bool checked);
  void                clearColumns();
  void                updateCurrentMemoSelection();
  void                clearMemoColumns();

};

#endif // BANKINGWIZARDPAGE_H
