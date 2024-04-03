/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KINVESTMENTTYPEWIZARDPAGE_H
#define KINVESTMENTTYPEWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyAccount;
class MyMoneySecurity;

namespace Ui {
class KInvestmentTypeWizardPage;
}

class KInvestmentTypeWizardPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit KInvestmentTypeWizardPage(QWidget *parent = nullptr);
    ~KInvestmentTypeWizardPage();

    void init(const MyMoneyAccount& account, const MyMoneySecurity& security);
    void setIntroLabelText(const QString& text);

    /**
     * Overload isComplete to handle the required fields
     */
    bool isComplete() const final override;

private:
    Ui::KInvestmentTypeWizardPage  *ui;
};

#endif
