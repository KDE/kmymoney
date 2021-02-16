/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FINALPAYMENTWIZARDPAGE_H
#define FINALPAYMENTWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class FinalPaymentWizardPage; }

/**
 * This class implements the Online Update page of the
 * @ref KNewInvestmentWizard.
 */

class FinalPaymentWizardPage : public QWizardPage
{
  Q_OBJECT
public:
  explicit FinalPaymentWizardPage(QWidget *parent = nullptr);
  ~FinalPaymentWizardPage();

  void resetCalculator();

  Ui::FinalPaymentWizardPage *ui;
};

#endif
