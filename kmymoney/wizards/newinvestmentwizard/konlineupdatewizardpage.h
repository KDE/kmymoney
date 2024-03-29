/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KONLINEUPDATEWIZARDPAGE_H
#define KONLINEUPDATEWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneySecurity;

/**
 * This class implements the Online Update page of the
 * @ref KNewInvestmentWizard.
 */
class KOnlineUpdateWizardPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit KOnlineUpdateWizardPage(QWidget *parent = nullptr);
    ~KOnlineUpdateWizardPage();

    /**
     * Overload the isComplete function to control the Next button
     */
    bool isComplete() const final override;
    void init(const MyMoneySecurity& security);

private:
    class Private;
    Private* const d;
};

#endif
