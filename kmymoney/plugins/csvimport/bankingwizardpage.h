/*******************************************************************************
*                                 bankingwizardpage.h
*                              ------------------
* begin                       : Thur Jan 01 2015
* copyright                   : (C) 2015 by Allan Anderson
* email                       : agander93@gmail.com
* copyright                   : (C) 2016 by Łukasz Wojniłowicz
* email                       : lukasz.wojnilowicz@gmail.com
********************************************************************************/

/*******************************************************************************
*                                                                              *
*   This program is free software; you can redistribute it and/or modify       *
*   it under the terms of the GNU General Public License as published by       *
*   the Free Software Foundation; either version 2 of the License, or          *
*   (at your option) any later version.                                        *
*                                                                              *
********************************************************************************/

#ifndef BANKINGWIZARDPAGE_H
#define BANKINGWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QtCore/QFile>
#include <QVBoxLayout>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneystatement.h>
#include <csvimporter.h>

// ----------------------------------------------------------------------------

class BankingProfile;

namespace Ui
{
class BankingPage;
}

class BankingPage : public CSVWizardPage
{
  Q_OBJECT

public:
  explicit BankingPage(CSVWizard *dlg, CSVImporter *imp);
  ~BankingPage();

  Ui::BankingPage     *ui;
  QVBoxLayout         *m_pageLayout;

  bool validateCreditDebit();
  /**
  * This method fills QIF file with bank/credit card data
  */
  void                makeQIF(MyMoneyStatement &st, QFile &file);

private:
  void                initializePage();
  bool                isComplete() const;
  int                 nextId() const;

  void                initializeComboBoxes();
  bool                validateMemoComboBox();
  void                resetComboBox(const columnTypeE comboBox);
  bool                validateSelectedColumn(const int col, const columnTypeE type);

  BankingProfile       *m_profile;

private slots:
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
};

#endif // BANKINGWIZARDPAGE_H
