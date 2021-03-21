/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SCHEDULEWIZARDPAGE_H
#define SCHEDULEWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui {
class ScheduleWizardPage;
}

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
