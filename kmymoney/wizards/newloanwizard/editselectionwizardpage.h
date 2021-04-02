/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef EDITSELECTIONWIZARDPAGE_H
#define EDITSELECTIONWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui {
class EditSelectionWizardPage;
}

/**
 * This class implements the Edit Selection page of the
 * @ref KNewLoanWizard.
 */

class EditSelectionWizardPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit EditSelectionWizardPage(QWidget *parent = nullptr);
    ~EditSelectionWizardPage();

    Ui::EditSelectionWizardPage *ui;
};

#endif
