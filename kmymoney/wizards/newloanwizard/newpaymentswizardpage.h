/***************************************************************************
                         newpaymentswizardpage  -  description
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

#ifndef NEWPAYMENTSWIZARDPAGE_H
#define NEWPAYMENTSWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class NewPaymentsWizardPage; }

/**
 * This class implements the New Payments page of the
 * @ref KNewLoanWizard.
 */

class NewPaymentsWizardPage : public QWizardPage
{
  Q_OBJECT
public:
  explicit NewPaymentsWizardPage(QWidget *parent = nullptr);
  ~NewPaymentsWizardPage();

private:
  Ui::NewPaymentsWizardPage *ui;
};

#endif
