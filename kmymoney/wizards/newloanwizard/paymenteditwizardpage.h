/***************************************************************************
                         paymenteditwizardpage  -  description
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

#ifndef PAYMENTEDITWIZARDPAGE_H
#define PAYMENTEDITWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_paymenteditwizardpagedecl.h"

/**
 * This class implements the Payment Edit page of the
 * @ref KNewLoanWizard.
 */
class PaymentEditWizardPageDecl : public QWizardPage, public Ui::PaymentEditWizardPageDecl
{
public:
  PaymentEditWizardPageDecl(QWidget *parent) : QWizardPage(parent) {
    setupUi(this);
  }
};

class PaymentEditWizardPage : public PaymentEditWizardPageDecl
{
  Q_OBJECT
public:
  explicit PaymentEditWizardPage(QWidget *parent = 0);

};

#endif
