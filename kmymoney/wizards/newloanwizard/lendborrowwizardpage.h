/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LENDBORROWWIZARDPAGE_H
#define LENDBORROWWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui {
class LendBorrowWizardPage;
}

class LendBorrowWizardPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit LendBorrowWizardPage(QWidget *parent = nullptr);
    ~LendBorrowWizardPage();

private:
    Ui::LendBorrowWizardPage *ui;
};

#endif
