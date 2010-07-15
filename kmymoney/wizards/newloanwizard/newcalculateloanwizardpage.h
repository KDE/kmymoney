/***************************************************************************
                         newcalculateloanwizardpage  -  description
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

#ifndef NEWCALCULATELOANWIZARDPAGE_H
#define NEWCALCULATELOANWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_newcalculateloanwizardpagedecl.h"

/**
 * This class implements the New Calculate Loan page of the
 * @ref KNewLoanWizard.
 */
class NewCalculateLoanWizardPageDecl : public QWizardPage, public Ui::NewCalculateLoanWizardPageDecl
{
public:
  NewCalculateLoanWizardPageDecl(QWidget *parent) : QWizardPage(parent) {
    setupUi(this);
  }
};

class NewCalculateLoanWizardPage : public NewCalculateLoanWizardPageDecl
{
  Q_OBJECT
public:
  explicit NewCalculateLoanWizardPage(QWidget *parent = 0);

};

#endif
