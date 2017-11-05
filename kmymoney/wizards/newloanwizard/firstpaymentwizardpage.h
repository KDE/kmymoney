/***************************************************************************
                         firstpaymentwizardpage  -  description
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

#ifndef FIRSTPAYMENTWIZARDPAGE_H
#define FIRSTPAYMENTWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class FirstPaymentWizardPage; }

/**
 * This class implements the Online Update page of the
 * @ref KNewInvestmentWizard.
 */

class FirstPaymentWizardPage : public QWizardPage
{
  Q_OBJECT
public:
  explicit FirstPaymentWizardPage(QWidget *parent = nullptr);
  ~FirstPaymentWizardPage();
  /**
   * Overload the isComplete function to control the Next button
   */
  bool isComplete() const;

  /**
   * Overload the initializePage function to set widgets based on
   * the inputs from previous pages.
   */
  void initializePage();

  Ui::FirstPaymentWizardPage *ui;
};

#endif
