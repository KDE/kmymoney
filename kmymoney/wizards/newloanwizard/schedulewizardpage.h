/***************************************************************************
                         schedulewizardpage  -  description
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

#ifndef SCHEDULEWIZARDPAGE_H
#define SCHEDULEWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_schedulewizardpagedecl.h"

/**
 * This class implements the Schedule page of the
 * @ref KNewLoanWizard.
 */
class ScheduleWizardPageDecl : public QWizardPage, public Ui::ScheduleWizardPageDecl
{
public:
  ScheduleWizardPageDecl(QWidget *parent) : QWizardPage(parent) {
    setupUi(this);
  }
};

class ScheduleWizardPage : public ScheduleWizardPageDecl
{
  Q_OBJECT
public:
  explicit ScheduleWizardPage(QWidget *parent = 0);

  /**
   * Overload the isComplete function to control the Next button
   */
  bool isComplete() const;

  /**
   * Overload the initializePage function to set widgets based on
   * the inputs from previous pages.
   */
  void initializePage();
};

#endif
