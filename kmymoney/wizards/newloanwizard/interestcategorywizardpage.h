/***************************************************************************
                         interestcategorywizardpage  -  description
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

#ifndef INTERESTCATEGORYWIZARDPAGE_H
#define INTERESTCATEGORYWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_interestcategorywizardpagedecl.h"

/**
 * This class implements the Interest Category page of the
 * @ref KNewLoanWizard.
 */
class InterestCategoryWizardPageDecl : public QWizardPage, public Ui::InterestCategoryWizardPageDecl
{
public:
  InterestCategoryWizardPageDecl(QWidget *parent) : QWizardPage(parent) {
    setupUi(this);
  }
};

class InterestCategoryWizardPage : public InterestCategoryWizardPageDecl
{
  Q_OBJECT
public:
  explicit InterestCategoryWizardPage(QWidget *parent = 0);

  /**
   * Overload the isComplete function to control the Next button
   */
  bool isComplete() const;

protected slots:
  void slotCreateCategory();
};

#endif
