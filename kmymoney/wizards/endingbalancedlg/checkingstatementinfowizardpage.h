/***************************************************************************
                         checkingstatementinfowizardpage.h  -  description
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

#ifndef CHECKINGSTATEMENTINFOWIZARDPAGE_H
#define CHECKINGSTATEMENTINFOWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_checkingstatementinfowizardpagedecl.h"

/**
 * This class implements the CheckingStatementInfo page of the
 * @ref KEndingBalanceDlg wizard.
 */
class CheckingStatementInfoWizardPageDecl : public QWizardPage, public Ui::CheckingStatementInfoWizardPageDecl
{
public:
  CheckingStatementInfoWizardPageDecl(QWidget *parent) : QWizardPage(parent) {
    setupUi(this);
  }
};

class CheckingStatementInfoWizardPage : public CheckingStatementInfoWizardPageDecl
{
  Q_OBJECT
public:
  explicit CheckingStatementInfoWizardPage(QWidget *parent = 0);

};

#endif
