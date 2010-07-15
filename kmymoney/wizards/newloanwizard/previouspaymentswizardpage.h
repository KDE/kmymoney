/***************************************************************************
                         previouspaymentswizardpage  -  description
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

#ifndef PREVIOUSPAYMENTSWIZARDPAGE_H
#define PREVIOUSPAYMENTSWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_previouspaymentswizardpagedecl.h"

/**
 * This class implements the Previous Payments page of the
 * @ref KNewLoanWizard.
 */
class PreviousPaymentsWizardPageDecl : public QWizardPage, public Ui::PreviousPaymentsWizardPageDecl
{
public:
  PreviousPaymentsWizardPageDecl(QWidget *parent) : QWizardPage(parent) {
    setupUi(this);
  }
};

class PreviousPaymentsWizardPage : public PreviousPaymentsWizardPageDecl
{
  Q_OBJECT
public:
  explicit PreviousPaymentsWizardPage(QWidget *parent = 0);

};

#endif
