/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INTERESTTYPEWIZARDPAGE_H
#define INTERESTTYPEWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui {
class InterestTypeWizardPage;
}

/**
 * This class implements the Interest Type page of the
 * @ref KNewLoanWizard.
 */

class InterestTypeWizardPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit InterestTypeWizardPage(QWidget *parent = nullptr);
    ~InterestTypeWizardPage();

    Ui::InterestTypeWizardPage *ui;
};

#endif
