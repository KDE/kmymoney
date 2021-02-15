/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RECORDPAYMENTWIZARDPAGE_H
#define RECORDPAYMENTWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class RecordPaymentWizardPage; }

/**
 * This class implements the Record Payment page of the
 * @ref KNewLoanWizard.
 */

class RecordPaymentWizardPage : public QWizardPage
{
  Q_OBJECT
public:
  explicit RecordPaymentWizardPage(QWidget *parent = nullptr);
  ~RecordPaymentWizardPage();

  /**
   * Overload the initializePage function to set widgets based on
   * the inputs from previous pages.
   */
  void initializePage() final override;

private:
  Ui::RecordPaymentWizardPage *ui;
};

#endif
