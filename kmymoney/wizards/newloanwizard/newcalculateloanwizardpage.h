/***************************************************************************
                         newcalculateloanwizardpage  -  description
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

#ifndef NEWCALCULATELOANWIZARDPAGE_H
#define NEWCALCULATELOANWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class NewCalculateLoanWizardPage; }

/**
 * This class implements the New Calculate Loan page of the
 * @ref KNewLoanWizard.
 */

class NewCalculateLoanWizardPage : public QWizardPage
{
  Q_OBJECT
public:
  explicit NewCalculateLoanWizardPage(QWidget *parent = nullptr);
  ~NewCalculateLoanWizardPage();

private:
  Ui::NewCalculateLoanWizardPage *ui;
};

#endif
