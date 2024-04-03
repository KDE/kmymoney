/*
    SPDX-FileCopyrightText: 2004 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KNEWINVESTMENTWIZARD_H
#define KNEWINVESTMENTWIZARD_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizard>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyAccount;
class MyMoneySecurity;

/**
  * This class contains the implementation of the new investment wizard.
  *
  * @author Thomas Baumgart
  */

class KNewInvestmentWizardPrivate;
class KNewInvestmentWizard : public QWizard
{
    Q_OBJECT
    /**
      * Use this constructor for the creation of a new investment
      */
    explicit KNewInvestmentWizard(QWidget *parent = nullptr);

    /**
      * Use this constructor for the modification of an existing investment
      */
    explicit KNewInvestmentWizard(const MyMoneyAccount& acc, QWidget *parent = nullptr);

    /**
      * Use this constructor for the modification of an existing security
      */
    explicit KNewInvestmentWizard(const MyMoneySecurity& sec, QWidget *parent = nullptr);

public:
    ~KNewInvestmentWizard();

    /**
      * Create a new investment in a given @p parent investment account
      */
    static void newInvestment(const MyMoneyAccount& parent);

    /**
     * Create a new investment in a given @p parent investment account and
     * preset the data for the new account as provided in @a account
     */
    static void newInvestment(MyMoneyAccount& account, const MyMoneyAccount& parent);

    /**
     * Edit the investment in @p parent and the security it
     * is denominated in
     */
    static void editInvestment(const MyMoneyAccount& investment);

    /**
     * Edit the @a security
     */
    static void editSecurity(const MyMoneySecurity& security);

private:
    Q_DISABLE_COPY(KNewInvestmentWizard)
    Q_DECLARE_PRIVATE(KNewInvestmentWizard)
    const QScopedPointer<KNewInvestmentWizardPrivate> d_ptr;
};

#endif
