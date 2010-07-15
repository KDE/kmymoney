/***************************************************************************
                         variableinterestdatewizardpage  -  description
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

#ifndef VARIABLEINTERESTDATEWIZARDPAGE_H
#define VARIABLEINTERESTDATEWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_variableinterestdatewizardpagedecl.h"

/**
 * This class implements the Variable Interest Date page of the
 * @ref KNewLoanWizard.
 */
class VariableInterestDateWizardPageDecl : public QWizardPage, public Ui::VariableInterestDateWizardPageDecl
{
public:
  VariableInterestDateWizardPageDecl(QWidget *parent) : QWizardPage(parent) {
    setupUi(this);
  }
};

class VariableInterestDateWizardPage : public VariableInterestDateWizardPageDecl
{
  Q_OBJECT
public:
  explicit VariableInterestDateWizardPage(QWidget *parent = 0);

};

#endif
