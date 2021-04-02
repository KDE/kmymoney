/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INTERESTCHARGECHECKINGSWIZARDPAGE_H
#define INTERESTCHARGECHECKINGSWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui {
class InterestChargeCheckingsWizardPage;
}

/**
 * This class implements the InterestChargeCheckings page of the
 * @ref KEndingBalanceDlg wizard.
 */

class InterestChargeCheckingsWizardPage : public QWizardPage
{
    Q_OBJECT
    Q_DISABLE_COPY(InterestChargeCheckingsWizardPage)

public:
    explicit InterestChargeCheckingsWizardPage(QWidget *parent = nullptr);
    ~InterestChargeCheckingsWizardPage() override;

    /**
     * Overload the isComplete function to control the Next button
     */
    bool isComplete() const final override;

    Ui::InterestChargeCheckingsWizardPage *ui;
};

#endif
