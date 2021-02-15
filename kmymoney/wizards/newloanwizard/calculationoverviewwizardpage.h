/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CALCULATIONOVERVIEWWIZARDPAGE_H
#define CALCULATIONOVERVIEWWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class CalculationOverviewWizardPage; }

/**
 * This class implements the Calculation Overview page of the
 * @ref KNewLoanWizard.
 */

class CalculationOverviewWizardPage : public QWizardPage
{
  Q_OBJECT
public:
  explicit CalculationOverviewWizardPage(QWidget *parent = nullptr);
  ~CalculationOverviewWizardPage();

private:
  Ui::CalculationOverviewWizardPage *ui;
};

#endif
