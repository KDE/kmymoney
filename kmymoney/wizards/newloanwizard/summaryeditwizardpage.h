/***************************************************************************
                         summaryeditwizardpage  -  description
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

#ifndef SUMMARYEDITWIZARDPAGE_H
#define SUMMARYEDITWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_summaryeditwizardpagedecl.h"

/**
 * This class implements the Summary Edit page of the
 * @ref KNewLoanWizard.
 */
class SummaryEditWizardPageDecl : public QWizardPage, public Ui::SummaryEditWizardPageDecl
{
public:
  SummaryEditWizardPageDecl(QWidget *parent) : QWizardPage(parent) {
    setupUi(this);
  }
};

class SummaryEditWizardPage : public SummaryEditWizardPageDecl
{
  Q_OBJECT
public:
  explicit SummaryEditWizardPage(QWidget *parent = 0);

  /**
   * Overload the initializePage function to set widgets based on
   * the inputs from previous pages.
   */
  void initializePage();
};

#endif
