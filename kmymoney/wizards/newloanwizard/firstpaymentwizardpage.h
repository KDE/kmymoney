/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FIRSTPAYMENTWIZARDPAGE_H
#define FIRSTPAYMENTWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui {
class FirstPaymentWizardPage;
}

/**
 * This class implements the Online Update page of the
 * @ref KNewInvestmentWizard.
 */

class FirstPaymentWizardPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit FirstPaymentWizardPage(QWidget *parent = nullptr);
    ~FirstPaymentWizardPage();
    /**
     * Overload the isComplete function to control the Next button
     */
    bool isComplete() const final override;

    /**
     * Overload the initializePage function to set widgets based on
     * the inputs from previous pages.
     */
    void initializePage() final override;

    Ui::FirstPaymentWizardPage *ui;
};

#endif
