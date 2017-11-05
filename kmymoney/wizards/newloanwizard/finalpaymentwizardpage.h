/***************************************************************************
                         finalpaymentwizardpage  -  description
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
