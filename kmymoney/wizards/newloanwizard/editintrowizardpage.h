/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef EDITINTROWIZARDPAGE_H
#define EDITINTROWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui {
class EditIntroWizardPage;
}

/**
 * This class implements the Edit Intro page of the
 * @ref KNewLoanWizard.
 */

class EditIntroWizardPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit EditIntroWizardPage(QWidget *parent = nullptr);
    ~EditIntroWizardPage();

private:
    Ui::EditIntroWizardPage *ui;
};

#endif
