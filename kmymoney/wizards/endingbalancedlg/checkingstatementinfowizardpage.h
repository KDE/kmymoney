/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-FileCopyrightText: 2023 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CHECKINGSTATEMENTINFOWIZARDPAGE_H
#define CHECKINGSTATEMENTINFOWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui {
class CheckingStatementInfoWizardPage;
}

/**
 * This class implements the CheckingStatementInfo page of the
 * @ref KEndingBalanceDlg wizard.
 */

class CheckingStatementInfoWizardPage : public QWizardPage
{
    Q_OBJECT
    Q_DISABLE_COPY(CheckingStatementInfoWizardPage)

public:
    explicit CheckingStatementInfoWizardPage(QWidget *parent = nullptr);
    ~CheckingStatementInfoWizardPage();

    Ui::CheckingStatementInfoWizardPage *ui;

    bool isComplete() const override;

    /**
     * Overridden to suppress modifying the statement date
     * when back button is pressed.
     */
    void cleanupPage() override;
};

#endif
