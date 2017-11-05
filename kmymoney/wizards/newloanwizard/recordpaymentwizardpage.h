/***************************************************************************
                         recordpaymentwizardpage  -  description
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
  void initializePage();

private:
  Ui::RecordPaymentWizardPage *ui;
};

#endif
