/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SUMMARYWIZARDPAGE_H
#define SUMMARYWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class SummaryWizardPage; }

/**
 * This class implements the Summary page of the
 * @ref KNewLoanWizard.
 */

class SummaryWizardPage : public QWizardPage
{
  Q_OBJECT
public:
  explicit SummaryWizardPage(QWidget *parent = nullptr);
  ~SummaryWizardPage();

  /**
   * Overload the initializePage function to set widgets based on
   * the inputs from previous pages.
   */
  void initializePage() final override;

private:
  Ui::SummaryWizardPage *ui;
};

#endif
