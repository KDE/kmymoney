/***************************************************************************
                         interestcalculationwizardpage  -  description
                            -------------------
   begin                : Sun Jul 4 2010
   copyright            : (C) 2010 by Fernando Vilas
   email                : kmymoney-devel@kde.org
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
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
