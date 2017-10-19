/*******************************************************************************
*                                 investmentwizardpage.h
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

#ifndef INVESTMENTWIZARDPAGE_H
#define INVESTMENTWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QPointer>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "csvwizardpage.h"

// ----------------------------------------------------------------------------

class InvestmentProfile;
class SecurityDlg;
class SecuritiesDlg;
class MyMoneyStatement;

namespace Ui
{
class InvestmentPage;
}

class InvestmentPage : public CSVWizardPage
{
  Q_OBJECT

public:
  explicit InvestmentPage(CSVWizard *dlg, CSVImporter *imp);
  ~InvestmentPage();

  /**
  * This method fills QIF file with investment data
  */
  void                makeQIF(const MyMoneyStatement &st, const QString &outFileName);

  /**
  * This method validates the column numbers entered by the user.  It then
  * checks the values in those columns for compatibility with the input
  * investment activity type.
  */
  bool                validateActionType();

private:
  void                initializePage();
  bool                isComplete() const;
  bool                validatePage();
  void                cleanupPage();

  void                clearFeeCol();

  /**
  * This method will check whether memo combobox is still valid
  * after changing name or type column.
  */
  bool                validateMemoComboBox();

  void                resetComboBox(const Column comboBox);
  /**
  * This method is called column on investment page is selected.
  * It sets m_colTypeNum, m_colNumType and runs column validation.
  */
  bool                validateSelectedColumn(const int col, const Column type);

  /**
  * This method ensures that every security has symbol and name.
  */
  bool                validateSecurity();
  bool                validateSecurities();

  QPointer<SecurityDlg>     m_securityDlg;
  QPointer<SecuritiesDlg>   m_securitiesDlg;
  InvestmentProfile        *m_profile;
  Ui::InvestmentPage       *ui;

private slots:
  void                clearFee();
  void                memoColSelected(int col);
  void                dateColSelected(int col);
  void                feeColSelected(int col);
  void                typeColSelected(int col);
  void                quantityColSelected(int col);
  void                priceColSelected(int col);
  void                amountColSelected(int col);
  void                symbolColSelected(int col);
  void                nameColSelected(int col);
  void                feeIsPercentageClicked(bool checked);
  void                fractionChanged(int col);
  void                clearColumns();
  void                feeInputsChanged();
  void                feeRateChanged(const QString &text);
  void                minFeeChanged(const QString &text);
  void                calculateFee();
};

#endif // INVESTMENTWIZARDPAGE_H
