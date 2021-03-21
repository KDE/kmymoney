/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INTERESTEDITWIZARDPAGE_H
#define INTERESTEDITWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui {
class InterestEditWizardPage;
}

/**
 * This class implements the Interest Edit page of the
 * @ref KNewLoanWizard.
 */

class InterestEditWizardPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit InterestEditWizardPage(QWidget *parent = nullptr);
    ~InterestEditWizardPage() override;

    /**
     * Overload the isComplete function to control the Next button
     */
    bool isComplete() const final override;

    Ui::InterestEditWizardPage *ui;
};

#endif
