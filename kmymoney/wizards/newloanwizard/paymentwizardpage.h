/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PAYMENTWIZARDPAGE_H
#define PAYMENTWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class PaymentWizardPage; }

/**
 * This class implements the Online Update page of the
 * @ref KNewInvestmentWizard.
 */

class PaymentWizardPage : public QWizardPage
{
  Q_OBJECT
public:
  explicit PaymentWizardPage(QWidget *parent = nullptr);
  ~PaymentWizardPage();

  void resetCalculator();

  Ui::PaymentWizardPage *ui;
};

#endif
