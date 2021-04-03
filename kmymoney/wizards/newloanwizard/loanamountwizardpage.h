/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LOANAMOUNTWIZARDPAGE_H
#define LOANAMOUNTWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui {
class LoanAmountWizardPage;
}

/**
 * This class implements the Loan Amount page of the
 * @ref KNewLoanWizard.
 */

class LoanAmountWizardPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit LoanAmountWizardPage(QWidget *parent = nullptr);
    ~LoanAmountWizardPage();

    /**
     * Overload the isComplete function to control the Next button
     */
    bool isComplete() const final override;

    /**
     * Overload the initializePage function to set widgets based on
     * the inputs from previous pages.
     */
    void initializePage() final override;

    Ui::LoanAmountWizardPage *ui;

public Q_SLOTS:
    void resetCalculator();
};

#endif
