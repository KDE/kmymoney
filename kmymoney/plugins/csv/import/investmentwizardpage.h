/*
    SPDX-FileCopyrightText: 2010-2017 Allan Anderson <agander93@gmail.com>
    SPDX-FileCopyrightText: 2016-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
  explicit InvestmentPage(CSVWizard *dlg, CSVImporterCore *imp);
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
  void initializePage() final override;
  bool isComplete() const final override;
  bool validatePage() final override;
  void cleanupPage() final override;

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

private Q_SLOTS:
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
