/***************************************************************************
                         loanattributeswizardpage  -  description
                            -------------------
   begin                : Mon Dec 30 2013
   copyright            : (C) 2013 by Jeremy Whiting
   email                : jpwhiting@kde.org
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LOANATTRIBUTESWIZARDPAGE_H
#define LOANATTRIBUTESWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_loanattributeswizardpagedecl.h"

/**
 * This class implements the Loan Attributes page of the
 * @ref KNewLoanWizard.
 */
class LoanAttributesWizardPageDecl : public QWizardPage, public Ui::LoanAttributesWizardPageDecl
{
public:
  LoanAttributesWizardPageDecl(QWidget *parent) : QWizardPage(parent) {
    setupUi(this);
  }
};

class LoanAttributesWizardPage : public LoanAttributesWizardPageDecl
{
  Q_OBJECT
public:
  explicit LoanAttributesWizardPage(QWidget *parent = 0);

  /**
   * Overload the isComplete function to control the Next button
   */
  bool isComplete() const;

  /**
   * Overload the initializePage function to set widgets based on
   * the inputs from previous pages.
   */
  void initializePage();

  /**
   * Set the institution combobox to the name given
   */
  void setInstitution(const QString &institutionName);

protected slots:
  void slotNewClicked();

};

#endif
