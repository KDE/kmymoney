/***************************************************************************
                         newintrowizardpage  -  description
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

#ifndef NEWINTROWIZARDPAGE_H
#define NEWINTROWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_newintrowizardpagedecl.h"

/**
 * This class implements the New Intro page of the
 * @ref KNewLoanWizard.
 */
class NewIntroWizardPageDecl : public QWizardPage, public Ui::NewIntroWizardPageDecl
{
public:
  NewIntroWizardPageDecl(QWidget *parent) : QWizardPage(parent) {
    setupUi(this);
  }
};

class NewIntroWizardPage : public NewIntroWizardPageDecl
{
  Q_OBJECT
public:
  explicit NewIntroWizardPage(QWidget *parent = 0);
};

#endif
