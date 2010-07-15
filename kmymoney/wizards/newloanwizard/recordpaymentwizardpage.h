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

#include "ui_recordpaymentwizardpagedecl.h"

/**
 * This class implements the Record Payment page of the
 * @ref KNewLoanWizard.
 */
class RecordPaymentWizardPageDecl : public QWizardPage, public Ui::RecordPaymentWizardPageDecl
{
public:
  RecordPaymentWizardPageDecl(QWidget *parent) : QWizardPage(parent) {
    setupUi(this);
  }
};

class RecordPaymentWizardPage : public RecordPaymentWizardPageDecl
{
  Q_OBJECT
public:
  explicit RecordPaymentWizardPage(QWidget *parent = 0);

  /**
   * Overload the initializePage function to set widgets based on
   * the inputs from previous pages.
   */
  void initializePage();

};

#endif
