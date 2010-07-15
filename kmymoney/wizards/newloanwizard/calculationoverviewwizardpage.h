/***************************************************************************
                         calculationoverviewwizardpage  -  description
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

#ifndef CALCULATIONOVERVIEWWIZARDPAGE_H
#define CALCULATIONOVERVIEWWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_calculationoverviewwizardpagedecl.h"

/**
 * This class implements the Calculation Overview page of the
 * @ref KNewLoanWizard.
 */
class CalculationOverviewWizardPageDecl : public QWizardPage, public Ui::CalculationOverviewWizardPageDecl
{
public:
  CalculationOverviewWizardPageDecl(QWidget *parent) : QWizardPage(parent) {
    setupUi(this);
  }
};

class CalculationOverviewWizardPage : public CalculationOverviewWizardPageDecl
{
  Q_OBJECT
public:
  explicit CalculationOverviewWizardPage(QWidget *parent = 0);

};

#endif
