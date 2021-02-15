/***************************************************************************
                         interestcalculationwizardpage  -  description
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

#ifndef INTERESTCALCULATIONWIZARDPAGE_H
#define INTERESTCALCULATIONWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class InterestCalculationWizardPage; }

/**
 * This class implements the Interest Calculation page of the
 * @ref KNewLoanWizard.
 */

class InterestCalculationWizardPage : public QWizardPage
{
  Q_OBJECT
public:
  explicit InterestCalculationWizardPage(QWidget *parent = nullptr);
  ~InterestCalculationWizardPage();

private:
  Ui::InterestCalculationWizardPage *ui;
};

#endif
