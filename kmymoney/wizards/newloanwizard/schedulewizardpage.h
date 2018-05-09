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

namespace Ui { class ScheduleWizardPage; }

/**
 * This class implements the Schedule page of the
 * @ref KNewLoanWizard.
 */

class ScheduleWizardPage : public QWizardPage
{
  Q_OBJECT
public:
  explicit ScheduleWizardPage(QWidget *parent = nullptr);
  ~ScheduleWizardPage();

  /**
   * Overload the isComplete function to control the Next button
   */
  bool isComplete() const final override;

  /**
   * Overload the initializePage function to set widgets based on
   * the inputs from previous pages.
   */
  void initializePage() final override;

  Ui::ScheduleWizardPage *ui;
};

#endif
