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

namespace Ui { class InterestCategoryWizardPage; }

/**
 * This class implements the Interest Category page of the
 * @ref KNewLoanWizard.
 */

class InterestCategoryWizardPage : public QWizardPage
{
  Q_OBJECT
public:
  explicit InterestCategoryWizardPage(QWidget *parent = nullptr);
  ~InterestCategoryWizardPage();

  /**
   * Overload the isComplete function to control the Next button
   */
  bool isComplete() const;

  Ui::InterestCategoryWizardPage *ui;

protected Q_SLOTS:
  void slotCreateCategory();
};

#endif
