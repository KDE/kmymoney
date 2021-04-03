/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CHECKINGSTARTWIZARDPAGE_H
#define CHECKINGSTARTWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui {
class CheckingStartWizardPage;
}

/**
 * This class implements the CheckingStart page of the
 * @ref KEndingBalanceDlg wizard.
 */

class CheckingStartWizardPage : public QWizardPage
{
    Q_OBJECT
    Q_DISABLE_COPY(CheckingStartWizardPage)

public:
    explicit CheckingStartWizardPage(QWidget *parent = nullptr);
    ~CheckingStartWizardPage();

private:
    Ui::CheckingStartWizardPage *ui;
};

#endif
