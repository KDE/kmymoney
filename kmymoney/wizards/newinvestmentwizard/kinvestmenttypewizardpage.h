/***************************************************************************
                         kinvestmenttypewizardpage  -  description
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

#ifndef KINVESTMENTTYPEWIZARDPAGE_H
#define KINVESTMENTTYPEWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kinvestmenttypewizardpagedecl.h"

class MyMoneySecurity;

/**
 * This class implements the investment type page of the
 * @ref KNewInvestmentWizard.
 */
class KInvestmentTypeWizardPageDecl : public QWizardPage, public Ui::KInvestmentTypeWizardPageDecl
{
public:
  KInvestmentTypeWizardPageDecl(QWidget *parent) : QWizardPage(parent) {
    setupUi(this);
  }
};

class KInvestmentTypeWizardPage : public KInvestmentTypeWizardPageDecl
{
  Q_OBJECT
public:
  explicit KInvestmentTypeWizardPage(QWidget *parent = 0);
  void init2(const MyMoneySecurity& security);
  void setIntroLabelText(const QString& text);
};

#endif
