/***************************************************************************
                         interesttypewizardpage  -  description
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

#ifndef INTERESTTYPEWIZARDPAGE_H
#define INTERESTTYPEWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_interesttypewizardpagedecl.h"

/**
 * This class implements the Interest Type page of the
 * @ref KNewLoanWizard.
 */
class InterestTypeWizardPageDecl : public QWizardPage, public Ui::InterestTypeWizardPageDecl
{
public:
  InterestTypeWizardPageDecl(QWidget *parent) : QWizardPage(parent) {
    setupUi(this);
  }
};

class InterestTypeWizardPage : public InterestTypeWizardPageDecl
{
  Q_OBJECT
public:
  explicit InterestTypeWizardPage(QWidget *parent = 0);

};

#endif
