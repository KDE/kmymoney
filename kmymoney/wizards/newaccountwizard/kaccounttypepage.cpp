/***************************************************************************
                             kaccounttypepage.cpp
                             -------------------
    begin                : Tue Sep 25 2006
    copyright            : (C) 2007 Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#include "kaccounttypepage.h"
#include "kaccounttypepage_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>
#include <QDialog>
#include <QLabel>
#include <QPointer>
#include <QPushButton>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLineEdit>
#include <KLocalizedString>
#include <KMessageBox>

// ----------------------------------------------------------------------------
// Project Includes

/// @todo port to new model code
// #include "kequitypriceupdatedlg.h"
#include "kmymoneycurrencyselector.h"
#include "kmymoneydateinput.h"
#include "kmymoneygeneralcombo.h"
#include "kmymoneysettings.h"
#include "kmymoneywizardpage.h"
#include "knewaccountwizard.h"
#include "knewaccountwizard_p.h"
#include "kbrokeragepage.h"
#include "kcreditcardschedulepage.h"
#include "kgeneralloaninfopage.h"
#include "khierarchypage.h"
#include "mymoneyaccount.h"
#include "mymoneyfile.h"
#include "mymoneymoney.h"
#include "mymoneyprice.h"
#include "mymoneysecurity.h"
#include "wizardpage.h"
#include "kguiutils.h"
#include "mymoneyenums.h"
#include "accountsmodel.h"

using namespace eMyMoney;

namespace NewAccountWizard
{
  AccountTypePage::AccountTypePage(Wizard* wizard) :
    QWidget(wizard),
    WizardPage<Wizard>(*new AccountTypePagePrivate(wizard), StepAccount, this, wizard)
  {
    Q_D(AccountTypePage);
    d->ui->setupUi(this);
    d->m_showPriceWarning = true;
    d->ui->m_typeSelection->insertItem(i18n("Checking"), (int)Account::Type::Checkings, 0);
    d->ui->m_typeSelection->setItemIcon(0, Icons::get(Icon::Checking));
    d->ui->m_typeSelection->insertItem(i18n("Savings"), (int)Account::Type::Savings, 1);
    d->ui->m_typeSelection->setItemIcon(1, Icons::get(Icon::Savings));
    d->ui->m_typeSelection->insertItem(i18n("Credit Card"), (int)Account::Type::CreditCard, 2);
    d->ui->m_typeSelection->setItemIcon(2, Icons::get(Icon::CreditCard));
    d->ui->m_typeSelection->insertItem(i18n("Cash"), (int)Account::Type::Cash, 3);
    d->ui->m_typeSelection->setItemIcon(3, Icons::get(Icon::Cash));
    d->ui->m_typeSelection->insertItem(i18n("Loan"), (int)Account::Type::Loan, 4);
    d->ui->m_typeSelection->setItemIcon(4, Icons::get(Icon::Loan));
    d->ui->m_typeSelection->insertItem(i18n("Investment"), (int)Account::Type::Investment, 5);
    d->ui->m_typeSelection->setItemIcon(5, Icons::get(Icon::Investment));
    d->ui->m_typeSelection->insertItem(i18n("Asset"), (int)Account::Type::Asset, 6);
    d->ui->m_typeSelection->setItemIcon(6, Icons::get(Icon::Asset));
    d->ui->m_typeSelection->insertItem(i18n("Liability"), (int)Account::Type::Liability, 7);
    d->ui->m_typeSelection->setItemIcon(7, Icons::get(Icon::Liability));
    if (KMyMoneySettings::expertMode()) {
        d->ui->m_typeSelection->insertItem(i18n("Equity"), (int)Account::Type::Equity, 8);
        d->ui->m_typeSelection->setItemIcon(8, Icons::get(Icon::Equity));
      }

    d->ui->m_typeSelection->setCurrentItem((int)Account::Type::Checkings);

    d->ui->m_currencyComboBox->setSecurity(MyMoneyFile::instance()->baseCurrency());

    d->m_mandatoryGroup->add(d->ui->m_accountName);
    d->m_mandatoryGroup->add(d->ui->m_conversionRate);

    d->ui->m_conversionRate->setValue(MyMoneyMoney::ONE);
    slotUpdateCurrency();

    connect(d->ui->m_typeSelection, &KMyMoneyGeneralCombo::itemSelected, this, &AccountTypePage::slotUpdateType);
    connect(d->ui->m_currencyComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &AccountTypePage::slotUpdateCurrency);
    connect(d->ui->m_conversionRate, &AmountEdit::textChanged, this, &AccountTypePage::slotUpdateConversionRate);
    connect(d->ui->m_conversionRate, &AmountEdit::valueChanged, this, &AccountTypePage::slotPriceWarning);
    connect(d->ui->m_onlineQuote, &QAbstractButton::clicked, this, &AccountTypePage::slotGetOnlineQuote);
  }

  AccountTypePage::~AccountTypePage()
  {
  }

  void AccountTypePage::slotUpdateType(int i)
  {
    Q_D(AccountTypePage);
    hideShowPages(static_cast<Account::Type>(i));
    const bool enabled = accountTypeSupportsOpeningBalance(static_cast<Account::Type>(i));
    d->ui->m_openingBalance->setEnabled(enabled);
    d->ui->m_openingBalanceLabel->setEnabled(enabled);
  }

  void AccountTypePage::hideShowPages(Account::Type accountType) const
  {
    Q_D(const AccountTypePage);
    bool hideSchedulePage = (accountType != Account::Type::CreditCard)
        && (accountType != Account::Type::Loan);
    bool hideLoanPage     = (accountType != Account::Type::Loan);
    d->m_wizard->d_func()->setStepHidden(StepDetails, hideLoanPage);
    d->m_wizard->d_func()->setStepHidden(StepPayments, hideLoanPage);
    d->m_wizard->d_func()->setStepHidden(StepFees, hideLoanPage);
    d->m_wizard->d_func()->setStepHidden(StepSchedule, hideSchedulePage);
    d->m_wizard->d_func()->setStepHidden(StepPayout, (accountType != Account::Type::Loan));
    d->m_wizard->d_func()->setStepHidden(StepBroker, accountType != Account::Type::Investment);
    d->m_wizard->d_func()->setStepHidden(StepParentAccount, accountType == Account::Type::Loan);
    // Force an update of the steps in case the list has changed
    d->m_wizard->reselectStep();
  }

  KMyMoneyWizardPage* AccountTypePage::nextPage() const
  {
    Q_D(const AccountTypePage);
    if (accountType() == Account::Type::Loan)
      return d->m_wizard->d_func()->m_generalLoanInfoPage;
    if (accountType() == Account::Type::CreditCard)
      return d->m_wizard->d_func()->m_schedulePage;
    if (accountType() == Account::Type::Investment)
      return d->m_wizard->d_func()->m_brokeragepage;
    return d->m_wizard->d_func()->m_hierarchyPage;
  }

  QWidget* AccountTypePage::initialFocusWidget() const
  {
    Q_D(const AccountTypePage);
    return d->ui->m_accountName;
  }

  void AccountTypePage::slotUpdateCurrency()
  {
    Q_D(AccountTypePage);
    MyMoneyAccount acc;
    acc.setAccountType(accountType());

    d->ui->m_openingBalance->setPrecision(MyMoneyMoney::denomToPrec(acc.fraction(currency())));

    bool show =  d->ui->m_currencyComboBox->security().id() != MyMoneyFile::instance()->baseCurrency().id();
    d->ui->m_conversionLabel->setVisible(show);
    d->ui->m_conversionRate->setVisible(show);
    d->ui->m_conversionExample->setVisible(show);
    d->ui->m_onlineQuote->setVisible(show);
    d->ui->m_conversionRate->setEnabled(show);       // make sure to include/exclude in mandatoryGroup
    d->ui->m_conversionRate->setPrecision(d->ui->m_currencyComboBox->security().pricePrecision());
    d->m_mandatoryGroup->changed();
    slotUpdateConversionRate(d->ui->m_conversionRate->text());
  }

  void AccountTypePage::slotGetOnlineQuote()
  {
    /// @todo port to new model code
#if 0
    Q_D(AccountTypePage);
    QString id = MyMoneyFile::instance()->baseCurrency().id() + ' ' + d->ui->m_currencyComboBox->security().id();
    QPointer<KEquityPriceUpdateDlg> dlg = new KEquityPriceUpdateDlg(this, id);
    if (dlg->exec() == QDialog::Accepted) {
        const MyMoneyPrice &price = dlg->price(id);
        if (price.isValid()) {
            d->ui->m_conversionRate->setValue(price.rate(d->ui->m_currencyComboBox->security().id()));
            if (price.date() != d->ui->m_openingDate->date()) {
                priceWarning(true);
              }
          }
      }
    delete dlg;
#endif
  }

  void AccountTypePage::slotPriceWarning()
  {
    priceWarning(false);
  }

  void AccountTypePage::priceWarning(bool always)
  {
    Q_D(AccountTypePage);
    if (d->m_showPriceWarning || always) {
        KMessageBox::information(this, i18n("Please make sure to enter the correct conversion for the selected opening date. If you requested an online quote it might be provided for a different date."), i18n("Check date"));
      }
    d->m_showPriceWarning = false;
  }

  void AccountTypePage::slotUpdateConversionRate(const QString& txt)
  {
    Q_D(AccountTypePage);
    d->ui->m_conversionExample->setText(i18n("1 %1 equals %2", MyMoneyFile::instance()->baseCurrency().tradingSymbol(), MyMoneyMoney(txt).formatMoney(d->ui->m_currencyComboBox->security().tradingSymbol(), d->ui->m_currencyComboBox->security().pricePrecision())));
  }

  bool AccountTypePage::isComplete() const
  {
    Q_D(const AccountTypePage);
    // check that the conversion rate is positive if enabled
    bool rc = !d->ui->m_conversionRate->isVisible() || (!d->ui->m_conversionRate->value().isZero() && !d->ui->m_conversionRate->value().isNegative());
    if (!rc) {
        d->m_wizard->d_func()->m_nextButton->setToolTip(i18n("Conversion rate is not positive"));

      } else {
        rc = KMyMoneyWizardPage::isComplete();

        if (!rc) {
            d->m_wizard->d_func()->m_nextButton->setToolTip(i18n("No account name supplied"));
          }
      }
    hideShowPages(accountType());
    return rc;
  }

  bool AccountTypePage::accountTypeSupportsOpeningBalance(Account::Type type) const
  {
    switch(type) {
      case Account::Type::Equity:
      case Account::Type::Investment:
        return false;
      default:
        break;
    }
    return true;
  }

  Account::Type AccountTypePage::accountType() const
  {
    Q_D(const AccountTypePage);
    return static_cast<Account::Type>(d->ui->m_typeSelection->currentItem());
  }

  const MyMoneySecurity& AccountTypePage::currency() const
  {
    Q_D(const AccountTypePage);
    return d->ui->m_currencyComboBox->security();
  }

  void AccountTypePage::setAccount(const MyMoneyAccount& acc)
  {
    Q_D(AccountTypePage);
    if (acc.accountType() != Account::Type::Unknown) {
        if (acc.accountType() == Account::Type::AssetLoan) {
            d->ui->m_typeSelection->setCurrentItem((int)Account::Type::Loan);
          } else {
            d->ui->m_typeSelection->setCurrentItem((int)acc.accountType());
          }
    }
    d->ui->m_openingDate->setDate(acc.openingDate());
    d->ui->m_accountName->setText(acc.name());
    d->m_parentAccountId = acc.parentAccountId();
  }

  MyMoneyAccount AccountTypePage::parentAccount()
  {
    Q_D(AccountTypePage);
    const auto file = MyMoneyFile::instance();
    const auto acc = file->accountsModel()->itemById(d->m_parentAccountId);
    // in case we don't have a parent account or its group does
    // not match the selected account type, we use the corresponding
    // standard top level account
    if (acc.id().isEmpty() || (acc.accountGroup() != acc.accountGroup(accountType()))) {
      switch (accountType()) {
        case Account::Type::CreditCard:
        case Account::Type::Liability:
        case Account::Type::Loan: // Can be either but we return liability here
          return file->liability();
          break;
        case Account::Type::Equity:
          return file->equity();
        default:
          break;
      }
      return file->asset();
    }
    return acc;
  }

  bool AccountTypePage::allowsParentAccount() const
  {
    return accountType() != Account::Type::Loan;
  }
}
