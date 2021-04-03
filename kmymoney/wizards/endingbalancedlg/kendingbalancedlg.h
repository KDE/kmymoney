/*
    SPDX-FileCopyrightText: 2000 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KENDINGBALANCEDLG_H
#define KENDINGBALANCEDLG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizard>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class QDate;

class MyMoneyMoney;
class MyMoneyAccount;
class MyMoneyTransaction;

/**
  * This dialog is wizard based and used to enter additional
  * information required to start the reconciliation process.
  * This version implements the behaviour for checkings,
  * savings and credit card accounts.
  *
  * @author Thomas Baumgart
  */

class KEndingBalanceDlgPrivate;
class KEndingBalanceDlg : public QWizard
{
    Q_OBJECT
    Q_DISABLE_COPY(KEndingBalanceDlg)

public:
    enum { Page_CheckingStart, Page_PreviousPostpone,
           Page_CheckingStatementInfo, Page_InterestChargeCheckings
         };

    explicit KEndingBalanceDlg(const MyMoneyAccount& account, QWidget *parent = nullptr);
    ~KEndingBalanceDlg();

    MyMoneyMoney endingBalance() const;
    MyMoneyMoney previousBalance() const;
    QDate statementDate() const;

    MyMoneyTransaction interestTransaction();
    MyMoneyTransaction chargeTransaction();

    /**
     * This method returns the id of the next page in the wizard.
     * It is overloaded here to support the dynamic nature of this wizard.
     *
     * @return id of the next page or -1 if there is no next page
     */
    int nextId() const final override;

protected:
    bool createTransaction(MyMoneyTransaction& t, const int sign, const MyMoneyMoney& amount, const QString& category, const QDate& date);
    MyMoneyMoney adjustedReturnValue(const MyMoneyMoney& v) const;
    void createCategory(const QString& txt, QString& id, const MyMoneyAccount& parent);

protected Q_SLOTS:
    void slotReloadEditWidgets();
    void help();
    void slotCreateInterestCategory(const QString& txt, QString& id);
    void slotCreateChargesCategory(const QString& txt, QString& id);
    void accept() final override;
    void slotUpdateBalances();

Q_SIGNALS:
    /**
      * proxy signal for KMyMoneyPayeeCombo::createItem(const QString&, QString&)
      */
    void createPayee(const QString&, QString&);

    /**
      * emit when a category is about to be created
      */
    void createCategory(MyMoneyAccount& acc, const MyMoneyAccount& parent);

private:
    KEndingBalanceDlgPrivate * const d_ptr;
    Q_DECLARE_PRIVATE(KEndingBalanceDlg)

private Q_SLOTS:
    void slotNewPayee(const QString& newnameBase, QString& id);
};

#endif
