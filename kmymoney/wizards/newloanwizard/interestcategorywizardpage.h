/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
  bool isComplete() const final override;

  Ui::InterestCategoryWizardPage *ui;

protected Q_SLOTS:
  void slotCreateCategory();
};

#endif
