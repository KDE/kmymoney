/***************************************************************************
                         previouspaymentswizardpage  -  description
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

#ifndef PREVIOUSPAYMENTSWIZARDPAGE_H
#define PREVIOUSPAYMENTSWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class PreviousPaymentsWizardPage; }

/**
 * This class implements the Previous Payments page of the
 * @ref KNewLoanWizard.
 */

class PreviousPaymentsWizardPage : public QWizardPage
{
  Q_OBJECT
public:
  explicit PreviousPaymentsWizardPage(QWidget *parent = nullptr);
  ~PreviousPaymentsWizardPage();

private:
  Ui::PreviousPaymentsWizardPage *ui;
};

#endif
