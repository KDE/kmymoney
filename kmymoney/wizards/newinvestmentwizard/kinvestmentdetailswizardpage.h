/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KINVESTMENTDETAILSWIZARDPAGE_H
#define KINVESTMENTDETAILSWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyAccount;
class MyMoneySecurity;

/**
 * This class implements the investment details page  of the
 * @ref KNewInvestmentWizard.
 */
class KInvestmentDetailsWizardPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit KInvestmentDetailsWizardPage(QWidget *parent = nullptr);
    ~KInvestmentDetailsWizardPage();

    /**
     * Setup the widgets based on @a account and @a security
     */
    void init(const MyMoneyAccount& account, const MyMoneySecurity& security);

    /**
     * Overload isComplete to handle the required fields
     */
    bool isComplete() const final override;

Q_SIGNALS:
    void securityIdChanged(const QString& id);

private:
    class Private;
    Private* const d;
};

#endif
