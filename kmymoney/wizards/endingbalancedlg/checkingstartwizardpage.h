/***************************************************************************
                         checkingstartwizardpage.h  -  description
                            -------------------
   begin                : Sun Jul 18 2010
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

#ifndef CHECKINGSTARTWIZARDPAGE_H
#define CHECKINGSTARTWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_checkingstartwizardpagedecl.h"

/**
 * This class implements the CheckingStart page of the
 * @ref KEndingBalanceDlg wizard.
 */
class CheckingStartWizardPageDecl : public QWizardPage, public Ui::CheckingStartWizardPageDecl
{
public:
  CheckingStartWizardPageDecl(QWidget *parent) : QWizardPage(parent) {
    setupUi(this);
  }
};

class CheckingStartWizardPage : public CheckingStartWizardPageDecl
{
  Q_OBJECT
public:
  explicit CheckingStartWizardPage(QWidget *parent = 0);

};

#endif
