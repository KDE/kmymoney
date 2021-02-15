/***************************************************************************
                         paymenteditwizardpage  -  description
                            -------------------
   begin                : Sun Jul 4 2010
   copyright            : (C) 2010 by Fernando Vilas
   email                : kmymoney-devel@kde.org
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef PAYMENTEDITWIZARDPAGE_H
#define PAYMENTEDITWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class PaymentEditWizardPage; }

/**
 * This class implements the Payment Edit page of the
 * @ref KNewLoanWizard.
 */

class PaymentEditWizardPage : public QWizardPage
{
  Q_OBJECT
public:
  explicit PaymentEditWizardPage(QWidget *parent = nullptr);
  ~PaymentEditWizardPage();

  Ui::PaymentEditWizardPage *ui;
};

#endif
