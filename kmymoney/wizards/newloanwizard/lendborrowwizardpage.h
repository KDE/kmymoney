/***************************************************************************
                         lendborrowwizardpage  -  description
                            -------------------
   begin                : Sun Jun 27 2010
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

#ifndef LENDBORROWWIZARDPAGE_H
#define LENDBORROWWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_lendborrowwizardpagedecl.h"

/**
 * This class implements the Online Update page of the
 * @ref KNewInvestmentWizard.
 */
class LendBorrowWizardPageDecl : public QWizardPage, public Ui::LendBorrowWizardPageDecl
{
public:
  LendBorrowWizardPageDecl(QWidget *parent) : QWizardPage(parent) {
    setupUi(this);
  }
};

class LendBorrowWizardPage : public LendBorrowWizardPageDecl
{
  Q_OBJECT
public:
  explicit LendBorrowWizardPage(QWidget *parent = 0);

};

#endif
