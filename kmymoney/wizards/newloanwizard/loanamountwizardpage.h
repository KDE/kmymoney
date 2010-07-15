/***************************************************************************
                         loanamountwizardpage  -  description
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

#ifndef LOANAMOUNTWIZARDPAGE_H
#define LOANAMOUNTWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_loanamountwizardpagedecl.h"

/**
 * This class implements the Loan Amount page of the
 * @ref KNewLoanWizard.
 */
class LoanAmountWizardPageDecl : public QWizardPage, public Ui::LoanAmountWizardPageDecl
{
public:
  LoanAmountWizardPageDecl(QWidget *parent) : QWizardPage(parent) {
    setupUi(this);
  }
};

class LoanAmountWizardPage : public LoanAmountWizardPageDecl
{
  Q_OBJECT
public:
  explicit LoanAmountWizardPage(QWidget *parent = 0);

  /**
   * Overload the isComplete function to control the Next button
   */
  bool isComplete() const;

  /**
   * Overload the initializePage function to set widgets based on
   * the inputs from previous pages.
   */
  void initializePage();

public slots:
  void resetCalculator(void);
};

#endif
