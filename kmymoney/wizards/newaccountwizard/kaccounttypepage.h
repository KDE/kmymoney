/***************************************************************************
                             kaccounttypepage.h
                             -------------------
    begin                : Tue Sep 25 2007
    copyright            : (C) 2007 Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef KACCOUNTTYPEPAGE_H
#define KACCOUNTTYPEPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "wizardpage.h"

class MyMoneyAccount;
class MyMoneySecurity;

namespace eMyMoney { namespace Account { enum class Type; } }

namespace NewAccountWizard
{
  class Wizard;

  class AccountTypePagePrivate;
  class AccountTypePage : public QWidget, public WizardPage<Wizard>
  {
    Q_OBJECT
    Q_DISABLE_COPY(AccountTypePage)

  public:
    explicit AccountTypePage(Wizard* parent);
    ~AccountTypePage() override;

    virtual bool isComplete() const override;
    KMyMoneyWizardPage* nextPage() const override;

    QWidget* initialFocusWidget() const override;

    eMyMoney::Account::Type accountType() const;
    MyMoneyAccount parentAccount();
    bool allowsParentAccount() const;
    bool accountTypeSupportsOpeningBalance(eMyMoney::Account::Type type) const;
    const MyMoneySecurity& currency() const;

    void setAccount(const MyMoneyAccount& acc);

  private:
    void hideShowPages(eMyMoney::Account::Type i) const;
    void priceWarning(bool);

  private Q_SLOTS:
    void slotUpdateType(int i);
    void slotUpdateCurrency();
    void slotUpdateConversionRate(const QString&);
    void slotGetOnlineQuote();
    void slotPriceWarning();

  private:
    Q_DECLARE_PRIVATE_D(WizardPage<Wizard>::d_ptr, AccountTypePage)
    friend class Wizard;
    friend class AccountSummaryPage;
    friend class BrokeragePage;
    friend class CreditCardSchedulePage;
    friend class LoanPayoutPage;
  };
} // namespace

#endif
