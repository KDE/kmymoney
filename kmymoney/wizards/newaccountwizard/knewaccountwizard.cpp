/***************************************************************************
                             knewaccountwizard.cpp
                             -------------------
    begin                : Tue Sep 25 2006
    copyright            : (C) 2007 Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "knewaccountwizard.h"
#include "knewaccountwizard_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QCheckBox>
#include <QLabel>
#include <QList>
#include <qmath.h>
#include <QIcon>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KLineEdit>
#include <KMessageBox>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneycurrencyselector.h"
#include "kmymoneyaccountselector.h"
#include "mymoneyfinancialcalculator.h"
#include "kcurrencycalculator.h"
#include "kmymoneyglobalsettings.h"
#include <kguiutils.h>

#include "mymoneyutils.h"
#include "ksplittransactiondlg.h"
#include "kequitypriceupdatedlg.h"
#include "accountsmodel.h"
#include "accountsproxymodel.h"
#include "models.h"
#include "modelenums.h"
#include "icons.h"
#include "mymoneyfile.h"
#include "mymoneyinstitution.h"
#include "mymoneyaccountloan.h"
#include "mymoneypayee.h"
#include "mymoneytransaction.h"

using namespace NewAccountWizard;
using namespace Icons;
using namespace eMyMoney;

namespace NewAccountWizard
{
enum steps {
  StepInstitution = 1,
  StepAccount,
  StepBroker,
  StepDetails,
  StepPayments,
  StepFees,
  StepSchedule,
  StepPayout,
  StepParentAccount,
  StepFinish
};

Wizard::Wizard(QWidget *parent, bool modal, Qt::WindowFlags flags)
    : KMyMoneyWizard(parent, modal, flags)
{
  setTitle(i18n("KMyMoney New Account Setup"));
  addStep(i18n("Institution"));
  addStep(i18n("Account"));
  addStep(i18n("Broker"));
  addStep(i18n("Details"));
  addStep(i18n("Payments"));
  addStep(i18n("Fees"));
  addStep(i18n("Schedule"));
  addStep(i18n("Payout"));
  addStep(i18n("Parent Account"));
  addStep(i18nc("Finish the wizard", "Finish"));
  setStepHidden(StepBroker);
  setStepHidden(StepSchedule);
  setStepHidden(StepPayout);
  setStepHidden(StepDetails);
  setStepHidden(StepPayments);
  setStepHidden(StepFees);

  m_institutionPage = new InstitutionPage(this);
  m_accountTypePage = new AccountTypePage(this);
  // Investment Pages
  m_brokeragepage = new BrokeragePage(this);
  // Credit Card Pages
  m_schedulePage = new CreditCardSchedulePage(this);
  // Loan Pages
  m_generalLoanInfoPage = new GeneralLoanInfoPage(this);
  m_loanDetailsPage = new LoanDetailsPage(this);
  m_loanPaymentPage = new LoanPaymentPage(this);
  m_loanSchedulePage = new LoanSchedulePage(this);
  m_loanPayoutPage = new LoanPayoutPage(this);
  // Not a loan page
  m_hierarchyPage = new HierarchyPage(this);
  // Finish
  m_accountSummaryPage = new AccountSummaryPage(this);

  setFirstPage(m_institutionPage);
}

void Wizard::setAccount(const MyMoneyAccount& acc)
{
  m_account = acc;
  m_accountTypePage->setAccount(m_account);

  if (!acc.institutionId().isEmpty()) {
    m_institutionPage->selectExistingInstitution(acc.institutionId());
  }
}

const MyMoneySecurity& Wizard::currency() const
{
  return m_accountTypePage->currency();
}

MyMoneyMoney Wizard::interestRate() const
{
  return m_loanDetailsPage->m_interestRate->value() / MyMoneyMoney(100, 1);
}

int Wizard::precision() const
{
  return MyMoneyMoney::denomToPrec(currency().smallestAccountFraction());
}

const MyMoneyAccount& Wizard::account()
{
  m_account = MyMoneyAccountLoan();
  m_account.setName(m_accountTypePage->m_accountName->text());
  m_account.setOpeningDate(m_accountTypePage->m_openingDate->date());
  m_account.setAccountType(m_accountTypePage->accountType());
  m_account.setInstitutionId(m_institutionPage->institution().id());
  m_account.setNumber(m_institutionPage->m_accountNumber->text());
  m_account.setValue("iban", m_institutionPage->m_iban->text());
  if (m_accountTypePage->m_preferredAccount->isChecked())
    m_account.setValue("PreferredAccount", "Yes");
  else
    m_account.deletePair("PreferredAccount");

  m_account.setCurrencyId(currency().id());
  if (m_account.isLoan()) {
    // in case we lend the money we adjust the account type
    if (!moneyBorrowed())
      m_account.setAccountType(Account::AssetLoan);
    m_account.setLoanAmount(m_loanDetailsPage->m_loanAmount->value());
    m_account.setInterestRate(m_loanSchedulePage->firstPaymentDueDate(), m_loanDetailsPage->m_interestRate->value());
    m_account.setInterestCalculation(m_loanDetailsPage->m_paymentDue->currentIndex() == 0 ? MyMoneyAccountLoan::paymentReceived : MyMoneyAccountLoan::paymentDue);
    m_account.setFixedInterestRate(m_generalLoanInfoPage->m_interestType->currentIndex() == 0);
    m_account.setFinalPayment(m_loanDetailsPage->m_balloonAmount->value());
    m_account.setTerm(m_loanDetailsPage->term());
    m_account.setPeriodicPayment(m_loanDetailsPage->m_paymentAmount->value());
    m_account.setPayee(m_generalLoanInfoPage->m_payee->selectedItem());
    m_account.setInterestCompounding((int)m_generalLoanInfoPage->m_compoundFrequency->currentItem());

    if (!m_account.fixedInterestRate()) {
      m_account.setNextInterestChange(m_generalLoanInfoPage->m_interestChangeDateEdit->date());
      m_account.setInterestChangeFrequency(m_generalLoanInfoPage->m_interestFrequencyAmountEdit->value(), m_generalLoanInfoPage->m_interestFrequencyUnitEdit->currentIndex());
    }
  }
  return m_account;
}

MyMoneyTransaction Wizard::payoutTransaction()
{
  MyMoneyTransaction t;
  if (m_account.isLoan()                                      // we're creating a loan
      && openingBalance().isZero()                                // and don't have an opening balance
      && !m_loanPayoutPage->m_noPayoutTransaction->isChecked()) { // and the user wants to have a payout transaction
    t.setPostDate(m_loanPayoutPage->m_payoutDate->date());
    t.setCommodity(m_account.currencyId());
    MyMoneySplit s;
    s.setAccountId(m_account.id());
    s.setShares(m_loanDetailsPage->m_loanAmount->value());
    if (moneyBorrowed())
      s.setShares(-s.shares());
    s.setValue(s.shares());
    t.addSplit(s);

    s.clearId();
    s.setValue(-s.value());
    s.setAccountId(m_loanPayoutPage->payoutAccountId());
    MyMoneyMoney shares;
    KCurrencyCalculator::setupSplitPrice(shares, t, s, QMap<QString, MyMoneyMoney>(), this);
    s.setShares(shares);
    t.addSplit(s);
  }
  return t;
}

const MyMoneyAccount& Wizard::parentAccount()
{
  return m_accountTypePage->allowsParentAccount()
         ? m_hierarchyPage->parentAccount()
         : (m_accountTypePage->accountType() == Account::Loan
            ? m_generalLoanInfoPage->parentAccount()
            : m_accountTypePage->parentAccount());
}

MyMoneyAccount Wizard::brokerageAccount() const
{
  MyMoneyAccount account;
  if (m_account.accountType() == Account::Investment
      && m_brokeragepage->m_createBrokerageButton->isChecked()) {
    account.setName(m_account.brokerageName());
    account.setAccountType(Account::Checkings);
    account.setInstitutionId(m_account.institutionId());
    account.setOpeningDate(m_account.openingDate());
    account.setCurrencyId(m_brokeragepage->m_brokerageCurrency->security().id());
    if (m_brokeragepage->m_accountNumber->isEnabled() && !m_brokeragepage->m_accountNumber->text().isEmpty())
      account.setNumber(m_brokeragepage->m_accountNumber->text());
    if (m_brokeragepage->m_iban->isEnabled() && !m_brokeragepage->m_iban->text().isEmpty())
      account.setValue("iban", m_brokeragepage->m_iban->text());
  }
  return account;
}

const MyMoneySchedule& Wizard::schedule()
{
  m_schedule = MyMoneySchedule();

  if (!m_account.id().isEmpty()) {
    if (m_schedulePage->m_reminderCheckBox->isChecked() && (m_account.accountType() == Account::CreditCard)) {
      m_schedule.setName(m_schedulePage->m_name->text());
      m_schedule.setType(Schedule::Type::Transfer);
      m_schedule.setPaymentType(static_cast<Schedule::PaymentType>(m_schedulePage->m_method->currentItem()));
      m_schedule.setFixed(false);
      m_schedule.setOccurrencePeriod(Schedule::Occurrence::Monthly);
      m_schedule.setOccurrenceMultiplier(1);
      MyMoneyTransaction t;
      MyMoneySplit s;
      s.setPayeeId(m_schedulePage->m_payee->selectedItem());
      s.setAccountId(m_schedulePage->m_paymentAccount->selectedItem());
      s.setMemo(i18n("Credit card payment"));
      s.setShares(-m_schedulePage->m_amount->value());
      s.setValue(s.shares());
      t.addSplit(s);

      s.clearId();
      s.setAccountId(m_account.id());
      s.setShares(m_schedulePage->m_amount->value());
      s.setValue(s.shares());
      t.addSplit(s);

      // setup the next due date
      t.setPostDate(m_schedulePage->m_date->date());
      m_schedule.setTransaction(t);

    } else if (m_account.isLoan()) {
      m_schedule.setName(i18n("Loan payment for %1", m_accountTypePage->m_accountName->text()));
      m_schedule.setType(Schedule::Type::LoanPayment);
      if (moneyBorrowed())
        m_schedule.setPaymentType(Schedule::PaymentType::DirectDebit);
      else
        m_schedule.setPaymentType(Schedule::PaymentType::DirectDeposit);

      m_schedule.setFixed(true);
      m_schedule.setOccurrence(m_generalLoanInfoPage->m_paymentFrequency->currentItem());

      MyMoneyTransaction t;
      t.setCommodity(m_account.currencyId());
      MyMoneySplit s;
      // payment split
      s.setPayeeId(m_generalLoanInfoPage->m_payee->selectedItem());
      s.setAccountId(m_loanSchedulePage->m_paymentAccount->selectedItem());
      s.setMemo(i18n("Loan payment"));
      s.setValue(m_loanPaymentPage->basePayment() + m_loanPaymentPage->additionalFees());
      if (moneyBorrowed()) {
        s.setValue(-s.value());
      }
      s.setShares(s.value());
      if (m_account.id() != QLatin1String("Phony-ID")) {
        // if the real account is already set perform the currency conversion if it's necessary
        MyMoneyMoney paymentShares;
        KCurrencyCalculator::setupSplitPrice(paymentShares, t, s, QMap<QString, MyMoneyMoney>(), this);
        s.setShares(paymentShares);
      }
      t.addSplit(s);

      // principal split
      s.clearId();
      s.setAccountId(m_account.id());
      s.setShares(MyMoneyMoney::autoCalc);
      s.setValue(MyMoneyMoney::autoCalc);
      s.setMemo(i18n("Amortization"));
      s.setAction(MyMoneySplit::ActionAmortization);
      t.addSplit(s);

      // interest split
      //only add if interest is above zero
      if (!m_loanDetailsPage->m_interestRate->value().isZero()) {
        s.clearId();
        s.setAccountId(m_loanSchedulePage->m_interestCategory->selectedItem());
        s.setShares(MyMoneyMoney::autoCalc);
        s.setValue(MyMoneyMoney::autoCalc);
        s.setMemo(i18n("Interest"));
        s.setAction(MyMoneySplit::ActionInterest);
        t.addSplit(s);
      }

      // additional fee splits
      QList<MyMoneySplit> additionalSplits;
      m_loanPaymentPage->additionalFeesSplits(additionalSplits);
      QList<MyMoneySplit>::const_iterator it;
      MyMoneyMoney factor(moneyBorrowed() ? 1 : -1, 1);

      for (it = additionalSplits.constBegin(); it != additionalSplits.constEnd(); ++it) {
        s = (*it);
        s.clearId();
        s.setShares(s.shares() * factor);
        s.setValue(s.value() * factor);
        t.addSplit(s);
      }

      // setup the next due date
      t.setPostDate(m_loanSchedulePage->firstPaymentDueDate());
      m_schedule.setTransaction(t);
    }
  }
  return m_schedule;
}

MyMoneyMoney Wizard::openingBalance() const
{
  // equity accounts don't have an opening balance
  if (m_accountTypePage->accountType() == Account::Equity)
    return MyMoneyMoney();

  if (m_accountTypePage->accountType() == Account::Loan) {
    if (m_generalLoanInfoPage->recordAllPayments())
      return MyMoneyMoney();
    if (moneyBorrowed())
      return -(m_generalLoanInfoPage->m_openingBalance->value());
    return m_generalLoanInfoPage->m_openingBalance->value();
  }
  return m_accountTypePage->m_openingBalance->value();
}

MyMoneyPrice Wizard::conversionRate() const
{
  if (MyMoneyFile::instance()->baseCurrency().id() == m_accountTypePage->m_currencyComboBox->security().id())
    return MyMoneyPrice(MyMoneyFile::instance()->baseCurrency().id(),
                        m_accountTypePage->m_currencyComboBox->security().id(),
                        m_accountTypePage->m_openingDate->date(),
                        MyMoneyMoney::ONE,
                        i18n("User"));
  return MyMoneyPrice(MyMoneyFile::instance()->baseCurrency().id(),
                      m_accountTypePage->m_currencyComboBox->security().id(),
                      m_accountTypePage->m_openingDate->date(),
                      m_accountTypePage->m_conversionRate->value(),
                      i18n("User"));
}

bool Wizard::moneyBorrowed() const
{
  return m_generalLoanInfoPage->m_loanDirection->currentIndex() == 0;
}

class InstitutionPage::Private
{
public:
  QList<MyMoneyInstitution>  m_list;
};

InstitutionPage::InstitutionPage(Wizard* wizard) :
    KInstitutionPageDecl(wizard),
    WizardPage<Wizard>(StepInstitution, this, wizard),
    d(new Private())
{
  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadWidgets()));
  connect(m_newInstitutionButton, SIGNAL(clicked()), this, SLOT(slotNewInstitution()));
  connect(m_institutionComboBox, SIGNAL(activated(int)), this, SLOT(slotSelectInstitution(int)));

  slotLoadWidgets();
  m_institutionComboBox->setCurrentItem(0);
  slotSelectInstitution(0);
}

InstitutionPage::~InstitutionPage()
{
  delete d;
}

void InstitutionPage::slotLoadWidgets()
{
  m_institutionComboBox->clear();

  d->m_list.clear();
  MyMoneyFile::instance()->institutionList(d->m_list);
  qSort(d->m_list);

  QList<MyMoneyInstitution>::const_iterator it_l;
  m_institutionComboBox->addItem("");
  for (it_l = d->m_list.constBegin(); it_l != d->m_list.constEnd(); ++it_l) {
    m_institutionComboBox->addItem((*it_l).name());
  }
}

void InstitutionPage::slotNewInstitution()
{
  MyMoneyInstitution institution;

  emit m_wizard->createInstitution(institution);

  if (!institution.id().isEmpty()) {
    QList<MyMoneyInstitution>::const_iterator it_l;
    int i = 0;
    for (it_l = d->m_list.constBegin(); it_l != d->m_list.constEnd(); ++it_l) {
      if ((*it_l).id() == institution.id()) {
        // select the item and remember that the very first one is the empty item
        m_institutionComboBox->setCurrentIndex(i + 1);
        slotSelectInstitution(i + 1);
        m_accountNumber->setFocus();
        break;
      }
      ++i;
    }
  }
}

void InstitutionPage::slotSelectInstitution(const int index)
{
  m_accountNumber->setEnabled(index != 0);
  m_iban->setEnabled(index != 0);
}

void InstitutionPage::selectExistingInstitution(const QString& id)
{
  for (int i = 0; i < d->m_list.length(); ++i) {
    if (d->m_list[i].id() == id) {
      m_institutionComboBox->setCurrentIndex(i + 1);
      slotSelectInstitution(i + 1);
      break;
    }
  }
}

const MyMoneyInstitution& InstitutionPage::institution() const
{
  static MyMoneyInstitution emptyInstitution;
  if (m_institutionComboBox->currentIndex() == 0)
    return emptyInstitution;

  return d->m_list[m_institutionComboBox->currentIndex()-1];
}

KMyMoneyWizardPage* InstitutionPage::nextPage() const
{
  return m_wizard->m_accountTypePage;
}

AccountTypePage::AccountTypePage(Wizard* wizard) :
    KAccountTypePageDecl(wizard),
    WizardPage<Wizard>(StepAccount, this, wizard),
    m_showPriceWarning(true)
{
  m_typeSelection->insertItem(i18n("Checking"), (int)Account::Checkings);
  m_typeSelection->insertItem(i18n("Savings"), (int)Account::Savings);
  m_typeSelection->insertItem(i18n("Credit Card"), (int)Account::CreditCard);
  m_typeSelection->insertItem(i18n("Cash"), (int)Account::Cash);
  m_typeSelection->insertItem(i18n("Loan"), (int)Account::Loan);
  m_typeSelection->insertItem(i18n("Investment"), (int)Account::Investment);
  m_typeSelection->insertItem(i18n("Asset"), (int)Account::Asset);
  m_typeSelection->insertItem(i18n("Liability"), (int)Account::Liability);
  if (KMyMoneyGlobalSettings::expertMode()) {
    m_typeSelection->insertItem(i18n("Equity"), (int)Account::Equity);
  }

  m_typeSelection->setCurrentItem((int)Account::Checkings);

  m_currencyComboBox->setSecurity(MyMoneyFile::instance()->baseCurrency());

  m_mandatoryGroup->add(m_accountName);
  m_mandatoryGroup->add(m_conversionRate->lineedit());

  m_conversionRate->setValue(MyMoneyMoney::ONE);
  slotUpdateCurrency();

  connect(m_typeSelection, SIGNAL(itemSelected(int)), this, SLOT(slotUpdateType(int)));
  connect(m_currencyComboBox, SIGNAL(activated(int)), this, SLOT(slotUpdateCurrency()));
  connect(m_conversionRate, SIGNAL(textChanged(QString)), this, SLOT(slotUpdateConversionRate(QString)));
  connect(m_conversionRate, SIGNAL(valueChanged(QString)), this, SLOT(slotPriceWarning()));
  connect(m_onlineQuote, SIGNAL(clicked()), this, SLOT(slotGetOnlineQuote()));
}

void AccountTypePage::slotUpdateType(int i)
{
  hideShowPages(static_cast<Account>(i));
  m_openingBalance->setDisabled(static_cast<Account>(i) == Account::Equity);
}

void AccountTypePage::hideShowPages(Account accountType) const
{
  bool hideSchedulePage = (accountType != Account::CreditCard)
                          && (accountType != Account::Loan);
  bool hideLoanPage     = (accountType != Account::Loan);
  m_wizard->setStepHidden(StepDetails, hideLoanPage);
  m_wizard->setStepHidden(StepPayments, hideLoanPage);
  m_wizard->setStepHidden(StepFees, hideLoanPage);
  m_wizard->setStepHidden(StepSchedule, hideSchedulePage);
  m_wizard->setStepHidden(StepPayout, (accountType != Account::Loan));
  m_wizard->setStepHidden(StepBroker, accountType != Account::Investment);
  m_wizard->setStepHidden(StepParentAccount, accountType == Account::Loan);
  // Force an update of the steps in case the list has changed
  m_wizard->reselectStep();
}

KMyMoneyWizardPage* AccountTypePage::nextPage() const
{
  if (accountType() == Account::Loan)
    return m_wizard->m_generalLoanInfoPage;
  if (accountType() == Account::CreditCard)
    return m_wizard->m_schedulePage;
  if (accountType() == Account::Investment)
    return m_wizard->m_brokeragepage;
  return m_wizard->m_hierarchyPage;
}

void AccountTypePage::slotUpdateCurrency()
{
  MyMoneyAccount acc;
  acc.setAccountType(accountType());

  m_openingBalance->setPrecision(MyMoneyMoney::denomToPrec(acc.fraction(currency())));

  bool show =  m_currencyComboBox->security().id() != MyMoneyFile::instance()->baseCurrency().id();
  m_conversionLabel->setVisible(show);
  m_conversionRate->setVisible(show);
  m_conversionExample->setVisible(show);
  m_onlineQuote->setVisible(show);
  m_conversionRate->setEnabled(show);       // make sure to include/exclude in mandatoryGroup
  m_conversionRate->setPrecision(m_currencyComboBox->security().pricePrecision());
  m_mandatoryGroup->changed();
  slotUpdateConversionRate(m_conversionRate->lineedit()->text());
}

void AccountTypePage::slotGetOnlineQuote()
{
  QString id = MyMoneyFile::instance()->baseCurrency().id() + ' ' + m_currencyComboBox->security().id();
  QPointer<KEquityPriceUpdateDlg> dlg = new KEquityPriceUpdateDlg(this, id);
  if (dlg->exec() == QDialog::Accepted) {
    const MyMoneyPrice &price = dlg->price(id);
    if (price.isValid()) {
      m_conversionRate->setValue(price.rate(m_currencyComboBox->security().id()));
      if (price.date() != m_openingDate->date()) {
        priceWarning(true);
      }
    }
  }
  delete dlg;
}

void AccountTypePage::slotPriceWarning()
{
  priceWarning(false);
}

void AccountTypePage::priceWarning(bool always)
{
  if (m_showPriceWarning || always) {
    KMessageBox::information(this, i18n("Please make sure to enter the correct conversion for the selected opening date. If you requested an online quote it might be provided for a different date."), i18n("Check date"));
  }
  m_showPriceWarning = false;
}

void AccountTypePage::slotUpdateConversionRate(const QString& txt)
{
  m_conversionExample->setText(i18n("1 %1 equals %2", MyMoneyFile::instance()->baseCurrency().tradingSymbol(), MyMoneyMoney(txt).formatMoney(m_currencyComboBox->security().tradingSymbol(), m_currencyComboBox->security().pricePrecision())));
}

bool AccountTypePage::isComplete() const
{
  // check that the conversion rate is positive if enabled
  bool rc = !m_conversionRate->isVisible() || (!m_conversionRate->value().isZero() && !m_conversionRate->value().isNegative());
  if (!rc) {
    m_wizard->m_nextButton->setToolTip(i18n("Conversion rate is not positive"));

  } else {
    rc = KMyMoneyWizardPage::isComplete();

    if (!rc) {
      m_wizard->m_nextButton->setToolTip(i18n("No account name supplied"));
    }
  }
  hideShowPages(accountType());
  return rc;
}

Account AccountTypePage::accountType() const
{
  return static_cast<Account>(m_typeSelection->currentItem());
}

const MyMoneySecurity& AccountTypePage::currency() const
{
  return m_currencyComboBox->security();
}

void AccountTypePage::setAccount(const MyMoneyAccount& acc)
{
  if (acc.accountType() != Account::Unknown) {
    if (acc.accountType() == Account::AssetLoan) {
      m_typeSelection->setCurrentItem((int)Account::Loan);
    } else {
      m_typeSelection->setCurrentItem((int)acc.accountType());
    }
  }
  m_openingDate->setDate(acc.openingDate());
  m_accountName->setText(acc.name());
}

const MyMoneyAccount& AccountTypePage::parentAccount()
{
  switch (accountType()) {
    case Account::CreditCard:
    case Account::Liability:
    case Account::Loan: // Can be either but we return liability here
      return MyMoneyFile::instance()->liability();
      break;
    case Account::Equity:
      return MyMoneyFile::instance()->equity();
    default:
      break;
  }
  return MyMoneyFile::instance()->asset();
}

bool AccountTypePage::allowsParentAccount() const
{
  return accountType() != Account::Loan;
}

BrokeragePage::BrokeragePage(Wizard* wizard) :
    KBrokeragePageDecl(wizard),
    WizardPage<Wizard>(StepBroker, this, wizard)
{
  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadWidgets()));
}

void BrokeragePage::slotLoadWidgets()
{
  m_brokerageCurrency->update(QString("x"));
}

void BrokeragePage::enterPage()
{
  // assign the currency of the investment account to the
  // brokerage account if nothing else has ever been selected
  if (m_brokerageCurrency->security().id().isEmpty()) {
    m_brokerageCurrency->setSecurity(m_wizard->m_accountTypePage->m_currencyComboBox->security());
  }

  // check if the institution relevant fields should be enabled or not
  bool enabled = m_wizard->m_institutionPage->m_accountNumber->isEnabled();
  m_accountNumberLabel->setEnabled(enabled);
  m_accountNumber->setEnabled(enabled);
  m_ibanLabel->setEnabled(enabled);
  m_iban->setEnabled(enabled);
}

KMyMoneyWizardPage* BrokeragePage::nextPage() const
{
  return m_wizard->m_hierarchyPage;
}

CreditCardSchedulePage::CreditCardSchedulePage(Wizard* wizard) :
    KSchedulePageDecl(wizard),
    WizardPage<Wizard>(StepSchedule, this, wizard)
{
  m_mandatoryGroup->add(m_name);
  m_mandatoryGroup->add(m_payee);
  m_mandatoryGroup->add(m_amount->lineedit());
  m_mandatoryGroup->add(m_paymentAccount);
  connect(m_paymentAccount, SIGNAL(itemSelected(QString)), object(), SIGNAL(completeStateChanged()));
  connect(m_payee, SIGNAL(itemSelected(QString)), object(), SIGNAL(completeStateChanged()));
  connect(m_date, SIGNAL(dateChanged(QDate)), object(), SIGNAL(completeStateChanged()));

  connect(m_payee, SIGNAL(createItem(QString,QString&)), wizard, SIGNAL(createPayee(QString,QString&)));

  m_reminderCheckBox->setChecked(true);
  connect(m_reminderCheckBox, SIGNAL(toggled(bool)), object(), SIGNAL(completeStateChanged()));

  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadWidgets()));

  m_method->insertItem(i18n("Write check"), (int)Schedule::PaymentType::WriteChecque);
#if 0
  m_method->insertItem(i18n("Direct debit"), Schedule::PaymentType::DirectDebit);
  m_method->insertItem(i18n("Bank transfer"), Schedule::PaymentType::BankTransfer);
#endif
  m_method->insertItem(i18n("Standing order"), (int)Schedule::PaymentType::StandingOrder);
  m_method->insertItem(i18n("Manual deposit"), (int)Schedule::PaymentType::ManualDeposit);
  m_method->insertItem(i18n("Direct deposit"), (int)Schedule::PaymentType::DirectDeposit);
  m_method->insertItem(i18nc("Other payment method", "Other"), (int)Schedule::PaymentType::Other);
  m_method->setCurrentItem((int)Schedule::PaymentType::DirectDebit);

  slotLoadWidgets();
}

void CreditCardSchedulePage::enterPage()
{
  if (m_name->text().isEmpty())
    m_name->setText(i18n("Credit Card %1 monthly payment", m_wizard->m_accountTypePage->m_accountName->text()));
}

bool CreditCardSchedulePage::isComplete() const
{
  bool rc = true;
  QString msg = i18n("Finish entry and create account");
  if (m_reminderCheckBox->isChecked()) {
    msg = i18n("Finish entry and create account and schedule");
    if (m_date->date() < m_wizard->m_accountTypePage->m_openingDate->date()) {
      rc = false;
      msg = i18n("Next due date is prior to opening date");
    }
    if (m_paymentAccount->selectedItem().isEmpty()) {
      rc = false;
      msg = i18n("No account selected");
    }
    if (m_amount->lineedit()->text().isEmpty()) {
      rc = false;
      msg = i18n("No amount for payment selected");
    }
    if (m_payee->selectedItem().isEmpty()) {
      rc = false;
      msg = i18n("No payee for payment selected");
    }
    if (m_name->text().isEmpty()) {
      rc = false;
      msg = i18n("No name assigned for schedule");
    }
  }
  m_wizard->m_finishButton->setToolTip(msg);

  return rc;
}

void CreditCardSchedulePage::slotLoadWidgets()
{
  AccountSet set;
  set.addAccountGroup(Account::Asset);
  set.load(m_paymentAccount->selector());

  m_payee->loadPayees(MyMoneyFile::instance()->payeeList());
}

KMyMoneyWizardPage* CreditCardSchedulePage::nextPage() const
{
  return m_wizard->m_hierarchyPage;
}

GeneralLoanInfoPage::GeneralLoanInfoPage(Wizard* wizard) :
    KGeneralLoanInfoPageDecl(wizard),
    WizardPage<Wizard>(StepDetails, this, wizard),
    m_firstTime(true)
{
  m_mandatoryGroup->add(m_payee);

  // remove the unsupported payment and compounding frequencies and setup default
  m_paymentFrequency->removeItem((int)Schedule::Occurrence::Once);
  m_paymentFrequency->removeItem((int)Schedule::Occurrence::EveryOtherYear);
  m_paymentFrequency->setCurrentItem((int)Schedule::Occurrence::Monthly);
  m_compoundFrequency->removeItem((int)Schedule::Occurrence::Once);
  m_compoundFrequency->removeItem((int)Schedule::Occurrence::EveryOtherYear);
  m_compoundFrequency->setCurrentItem((int)Schedule::Occurrence::Monthly);

  slotLoadWidgets();

  connect(m_payee, SIGNAL(createItem(QString,QString&)), wizard, SIGNAL(createPayee(QString,QString&)));
  connect(m_anyPayments, SIGNAL(activated(int)), object(),  SIGNAL(completeStateChanged()));
  connect(m_recordings, SIGNAL(activated(int)), object(), SIGNAL(completeStateChanged()));

  connect(m_interestType, SIGNAL(activated(int)), object(),  SIGNAL(completeStateChanged()));
  connect(m_interestChangeDateEdit, SIGNAL(dateChanged(QDate)), object(),  SIGNAL(completeStateChanged()));
  connect(m_openingBalance, SIGNAL(textChanged(QString)), object(),  SIGNAL(completeStateChanged()));

  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadWidgets()));
}

KMyMoneyWizardPage* GeneralLoanInfoPage::nextPage() const
{
  return m_wizard->m_loanDetailsPage;
}

bool GeneralLoanInfoPage::recordAllPayments() const
{
  bool rc = true;     // all payments
  if (m_recordings->isEnabled()) {
    if (m_recordings->currentIndex() != 0)
      rc = false;
  }
  return rc;
}

void GeneralLoanInfoPage::enterPage()
{
  if (m_firstTime) {
    // setup default dates to last of this month and one year on top
    QDate firstDay(QDate::currentDate().year(), QDate::currentDate().month(), 1);
    m_firstPaymentDate->setDate(firstDay.addMonths(1).addDays(-1));
    m_interestChangeDateEdit->setDate(m_firstPaymentDate->date().addYears(1));;
    m_firstTime = false;
  }
}

bool GeneralLoanInfoPage::isComplete() const
{
  m_wizard->setStepHidden(StepPayout, !m_wizard->openingBalance().isZero());
  bool rc = KMyMoneyWizardPage::isComplete();
  if (!rc) {
    m_wizard->m_nextButton->setToolTip(i18n("No payee supplied"));
  }

  // fixup availability of items on this page
  m_recordings->setDisabled(m_anyPayments->currentIndex() == 0);

  m_interestFrequencyAmountEdit->setDisabled(m_interestType->currentIndex() == 0);
  m_interestFrequencyUnitEdit->setDisabled(m_interestType->currentIndex() == 0);
  m_interestChangeDateEdit->setDisabled(m_interestType->currentIndex() == 0);

  m_openingBalance->setDisabled(recordAllPayments());

  if (m_openingBalance->isEnabled() && m_openingBalance->lineedit()->text().length() == 0) {
    rc = false;
    m_wizard->m_nextButton->setToolTip(i18n("No opening balance supplied"));
  }

  if (rc
      && (m_interestType->currentIndex() != 0)
      && (m_interestChangeDateEdit->date() <= m_firstPaymentDate->date())) {
    rc = false;
    m_wizard->m_nextButton->setToolTip(i18n("An interest change can only happen after the first payment"));
  }
  return rc;
}

const MyMoneyAccount& GeneralLoanInfoPage::parentAccount()
{
  return (m_loanDirection->currentIndex() == 0)
         ? MyMoneyFile::instance()->liability()
         : MyMoneyFile::instance()->asset();
}

void GeneralLoanInfoPage::slotLoadWidgets()
{
  m_payee->loadPayees(MyMoneyFile::instance()->payeeList());
}

LoanDetailsPage::LoanDetailsPage(Wizard* wizard) :
    KLoanDetailsPageDecl(wizard),
    WizardPage<Wizard>(StepPayments, this, wizard),
    m_needCalculate(true)
{
  // force the balloon payment to zero (default)
  m_balloonAmount->setValue(MyMoneyMoney());
  // allow any precision for the interest rate
  m_interestRate->setPrecision(-1);

  connect(m_paymentDue, SIGNAL(activated(int)), this, SLOT(slotValuesChanged()));

  connect(m_termAmount, SIGNAL(valueChanged(int)), this, SLOT(slotValuesChanged()));
  connect(m_termUnit, SIGNAL(highlighted(int)), this, SLOT(slotValuesChanged()));
  connect(m_loanAmount, SIGNAL(textChanged(QString)), this, SLOT(slotValuesChanged()));
  connect(m_interestRate, SIGNAL(textChanged(QString)), this, SLOT(slotValuesChanged()));
  connect(m_paymentAmount, SIGNAL(textChanged(QString)), this, SLOT(slotValuesChanged()));
  connect(m_balloonAmount, SIGNAL(textChanged(QString)), this, SLOT(slotValuesChanged()));

  connect(m_calculateButton, SIGNAL(clicked()), this, SLOT(slotCalculate()));
}

void LoanDetailsPage::enterPage()
{
  // we need to remove a bunch of entries of the payment frequencies
  m_termUnit->clear();

  m_mandatoryGroup->clear();
  if (!m_wizard->openingBalance().isZero()) {
    m_mandatoryGroup->add(m_loanAmount->lineedit());
    if (m_loanAmount->lineedit()->text().length() == 0) {
      m_loanAmount->setValue(m_wizard->openingBalance().abs());
    }
  }

  switch (m_wizard->m_generalLoanInfoPage->m_paymentFrequency->currentItem()) {
    default:
      m_termUnit->insertItem(i18n("Payments"), (int)Schedule::Occurrence::Once);
      m_termUnit->setCurrentItem((int)Schedule::Occurrence::Once);
      break;
    case Schedule::Occurrence::Monthly:
      m_termUnit->insertItem(i18n("Months"), (int)Schedule::Occurrence::Monthly);
      m_termUnit->insertItem(i18n("Years"), (int)Schedule::Occurrence::Yearly);
      m_termUnit->setCurrentItem((int)Schedule::Occurrence::Monthly);
      break;
    case Schedule::Occurrence::Yearly:
      m_termUnit->insertItem(i18n("Years"), (int)Schedule::Occurrence::Yearly);
      m_termUnit->setCurrentItem((int)Schedule::Occurrence::Yearly);
      break;
  }
}

void LoanDetailsPage::slotValuesChanged()
{
  m_needCalculate = true;
  m_wizard->completeStateChanged();
}

void LoanDetailsPage::slotCalculate()
{
  MyMoneyFinancialCalculator calc;
  double val;
  int PF, CF;
  QString result;
  bool moneyBorrowed = m_wizard->moneyBorrowed();
  bool moneyLend = !moneyBorrowed;

  // FIXME: for now, we only support interest calculation at the end of the period
  calc.setBep();
  // FIXME: for now, we only support periodic compounding
  calc.setDisc();

  PF = m_wizard->m_generalLoanInfoPage->m_paymentFrequency->eventsPerYear();
  CF = m_wizard->m_generalLoanInfoPage->m_compoundFrequency->eventsPerYear();

  if (PF == 0 || CF == 0)
    return;

  calc.setPF(PF);
  calc.setCF(CF);


  if (!m_loanAmount->lineedit()->text().isEmpty()) {
    val = m_loanAmount->value().abs().toDouble();
    if (moneyBorrowed)
      val = -val;
    calc.setPv(val);
  }

  if (!m_interestRate->lineedit()->text().isEmpty()) {
    val = m_interestRate->value().abs().toDouble();
    calc.setIr(val);
  }

  if (!m_paymentAmount->lineedit()->text().isEmpty()) {
    val = m_paymentAmount->value().abs().toDouble();
    if (moneyLend)
      val = -val;
    calc.setPmt(val);
  }

  if (!m_balloonAmount->lineedit()->text().isEmpty()) {
    val = m_balloonAmount->value().abs().toDouble();
    if (moneyLend)
      val = -val;
    calc.setFv(val);
  }

  if (m_termAmount->value() != 0) {
    calc.setNpp(term());
  }

  // setup of parameters is done, now do the calculation
  try {
    if (m_loanAmount->lineedit()->text().isEmpty()) {
      // calculate the amount of the loan out of the other information
      val = calc.presentValue();
      m_loanAmount->loadText(MyMoneyMoney(static_cast<double>(val)).abs().formatMoney("", m_wizard->precision()));
      result = i18n("KMyMoney has calculated the amount of the loan as %1.", m_loanAmount->lineedit()->text());

    } else if (m_interestRate->lineedit()->text().isEmpty()) {
      // calculate the interest rate out of the other information
      val = calc.interestRate();

      m_interestRate->loadText(MyMoneyMoney(static_cast<double>(val)).abs().formatMoney("", 3));
      result = i18n("KMyMoney has calculated the interest rate to %1%.", m_interestRate->lineedit()->text());

    } else if (m_paymentAmount->lineedit()->text().isEmpty()) {
      // calculate the periodical amount of the payment out of the other information
      val = calc.payment();
      m_paymentAmount->setValue(MyMoneyMoney(static_cast<double>(val)).abs());
      // reset payment as it might have changed due to rounding
      val = m_paymentAmount->value().abs().toDouble();
      if (moneyLend)
        val = -val;
      calc.setPmt(val);

      result = i18n("KMyMoney has calculated a periodic payment of %1 to cover principal and interest.", m_paymentAmount->lineedit()->text());

      val = calc.futureValue();
      if ((moneyBorrowed && val < 0 && qAbs(val) >= qAbs(calc.payment()))
          || (moneyLend && val > 0 && qAbs(val) >= qAbs(calc.payment()))) {
        calc.setNpp(calc.npp() - 1);
        // updateTermWidgets(calc.npp());
        val = calc.futureValue();
        MyMoneyMoney refVal(static_cast<double>(val));
        m_balloonAmount->loadText(refVal.abs().formatMoney("", m_wizard->precision()));
        result += QString(" ");
        result += i18n("The number of payments has been decremented and the balloon payment has been modified to %1.", m_balloonAmount->lineedit()->text());
      } else if ((moneyBorrowed && val < 0 && qAbs(val) < qAbs(calc.payment()))
                 || (moneyLend && val > 0 && qAbs(val) < qAbs(calc.payment()))) {
        m_balloonAmount->loadText(MyMoneyMoney().formatMoney("", m_wizard->precision()));
      } else {
        MyMoneyMoney refVal(static_cast<double>(val));
        m_balloonAmount->loadText(refVal.abs().formatMoney("", m_wizard->precision()));
        result += i18n("The balloon payment has been modified to %1.", m_balloonAmount->lineedit()->text());
      }

    } else if (m_termAmount->value() == 0) {
      // calculate the number of payments out of the other information
      val = calc.numPayments();
      if (val == 0)
        throw MYMONEYEXCEPTION("incorrect fincancial calculation");

      // if the number of payments has a fractional part, then we
      // round it to the smallest integer and calculate the balloon payment
      result = i18n("KMyMoney has calculated the term of your loan as %1. ", updateTermWidgets(qFloor(val)));

      if (val != qFloor(val)) {
        calc.setNpp(qFloor(val));
        val = calc.futureValue();
        MyMoneyMoney refVal(static_cast<double>(val));
        m_balloonAmount->loadText(refVal.abs().formatMoney("", m_wizard->precision()));
        result += i18n("The balloon payment has been modified to %1.", m_balloonAmount->lineedit()->text());
      }

    } else {
      // calculate the future value of the loan out of the other information
      val = calc.futureValue();

      // we differentiate between the following cases:
      // a) the future value is greater than a payment
      // b) the future value is less than a payment or the loan is overpaid
      // c) all other cases
      //
      // a) means, we have paid more than we owed. This can't be
      // b) means, we paid more than we owed but the last payment is
      //    less in value than regular payments. That means, that the
      //    future value is to be treated as  (fully payed back)
      // c) the loan is not payed back yet
      if ((moneyBorrowed && val < 0 && qAbs(val) > qAbs(calc.payment()))
          || (moneyLend && val > 0 && qAbs(val) > qAbs(calc.payment()))) {
        // case a)
        qDebug("Future Value is %f", val);
        throw MYMONEYEXCEPTION("incorrect fincancial calculation");

      } else if ((moneyBorrowed && val < 0 && qAbs(val) <= qAbs(calc.payment()))
                 || (moneyLend && val > 0 && qAbs(val) <= qAbs(calc.payment()))) {
        // case b)
        val = 0;
      }

      MyMoneyMoney refVal(static_cast<double>(val));
      result = i18n("KMyMoney has calculated a balloon payment of %1 for this loan.", refVal.abs().formatMoney("", m_wizard->precision()));

      if (!m_balloonAmount->lineedit()->text().isEmpty()) {
        if ((m_balloonAmount->value().abs() - refVal.abs()).abs().toDouble() > 1) {
          throw MYMONEYEXCEPTION("incorrect fincancial calculation");
        }
        result = i18n("KMyMoney has successfully verified your loan information.");
      }
      m_balloonAmount->loadText(refVal.abs().formatMoney("", m_wizard->precision()));
    }

  } catch (const MyMoneyException &) {
    KMessageBox::error(0,
                       i18n("You have entered mis-matching information. Please modify "
                            "your figures or leave one value empty "
                            "to let KMyMoney calculate it for you"),
                       i18n("Calculation error"));
    return;
  }

  result += i18n("\n\nAccept this or modify the loan information and recalculate.");

  KMessageBox::information(0, result, i18n("Calculation successful"));
  m_needCalculate = false;

  // now update change
  m_wizard->completeStateChanged();
}

int LoanDetailsPage::term() const
{
  int factor = 0;

  if (m_termAmount->value() != 0) {
    factor = 1;
    switch (m_termUnit->currentItem()) {
      case Schedule::Occurrence::Yearly: // years
        factor = 12;
        // intentional fall through

      case Schedule::Occurrence::Monthly: // months
        factor *= 30;
        factor *= m_termAmount->value();
        // factor now is the duration in days. we divide this by the
        // payment frequency and get the number of payments
        factor /= m_wizard->m_generalLoanInfoPage->m_paymentFrequency->daysBetweenEvents();
        break;

      default:
        qDebug("Unknown term unit %d in LoanDetailsPage::term(). Using payments.", (int)m_termUnit->currentItem());
        // intentional fall through

      case Schedule::Occurrence::Once: // payments
        factor = m_termAmount->value();
        break;
    }
  }
  return factor;
}

QString LoanDetailsPage::updateTermWidgets(const double val)
{
  long vl = qFloor(val);

  QString valString;
  Schedule::Occurrence unit = m_termUnit->currentItem();

  if ((unit == Schedule::Occurrence::Monthly)
      && ((vl % 12) == 0)) {
    vl /= 12;
    unit = Schedule::Occurrence::Yearly;
  }

  switch (unit) {
    case Schedule::Occurrence::Monthly:
      valString = i18np("one month", "%1 months", vl);
      m_termUnit->setCurrentItem((int)Schedule::Occurrence::Monthly);
      break;
    case Schedule::Occurrence::Yearly:
      valString = i18np("one year", "%1 years", vl);
      m_termUnit->setCurrentItem((int)Schedule::Occurrence::Yearly);
      break;
    default:
      valString = i18np("one payment", "%1 payments", vl);
      m_termUnit->setCurrentItem((int)Schedule::Occurrence::Once);
      break;
  }
  m_termAmount->setValue(vl);
  return valString;
}

bool LoanDetailsPage::isComplete() const
{
  // bool rc = KMyMoneyWizardPage::isComplete();

  int fieldCnt = 0;

  if (m_loanAmount->lineedit()->text().length() > 0) {
    fieldCnt++;
  }

  if (m_interestRate->lineedit()->text().length() > 0) {
    fieldCnt++;
  }

  if (m_termAmount->value() != 0) {
    fieldCnt++;
  }

  if (m_paymentAmount->lineedit()->text().length() > 0) {
    fieldCnt++;
  }

  if (m_balloonAmount->lineedit()->text().length() > 0) {
    fieldCnt++;
  }

  m_calculateButton->setEnabled(fieldCnt == 4 || (fieldCnt == 5 && m_needCalculate));

  m_calculateButton->setAutoDefault(false);
  m_calculateButton->setDefault(false);
  if (m_needCalculate && fieldCnt == 4) {
    m_wizard->m_nextButton->setToolTip(i18n("Press Calculate to verify the values"));
    m_calculateButton->setAutoDefault(true);
    m_calculateButton->setDefault(true);
  } else if (fieldCnt != 5) {
    m_wizard->m_nextButton->setToolTip(i18n("Not all details supplied"));
    m_calculateButton->setAutoDefault(true);
    m_calculateButton->setDefault(true);
  }
  m_wizard->m_nextButton->setAutoDefault(!m_calculateButton->autoDefault());
  m_wizard->m_nextButton->setDefault(!m_calculateButton->autoDefault());

  return (fieldCnt == 5) && !m_needCalculate;
}

KMyMoneyWizardPage* LoanDetailsPage::nextPage() const
{
  return m_wizard->m_loanPaymentPage;
}


class LoanPaymentPage::Private
{
public:
  MyMoneyAccount      phonyAccount;
  MyMoneySplit        phonySplit;
  MyMoneyTransaction  additionalFeesTransaction;
  MyMoneyMoney        additionalFees;
};

LoanPaymentPage::LoanPaymentPage(Wizard* wizard) :
    KLoanPaymentPageDecl(wizard),
    WizardPage<Wizard>(StepFees, this, wizard),
    d(new Private)
{
  d->phonyAccount = MyMoneyAccount(QLatin1String("Phony-ID"), MyMoneyAccount());

  d->phonySplit.setAccountId(d->phonyAccount.id());
  d->phonySplit.setValue(MyMoneyMoney());
  d->phonySplit.setShares(MyMoneyMoney());

  d->additionalFeesTransaction.addSplit(d->phonySplit);

  connect(m_additionalFeesButton, SIGNAL(clicked()), this, SLOT(slotAdditionalFees()));
}

LoanPaymentPage::~LoanPaymentPage()
{
  delete d;
}

MyMoneyMoney LoanPaymentPage::basePayment() const
{
  return m_wizard->m_loanDetailsPage->m_paymentAmount->value();
}

MyMoneyMoney LoanPaymentPage::additionalFees() const
{
  return d->additionalFees;
}

void LoanPaymentPage::additionalFeesSplits(QList<MyMoneySplit>& list)
{
  list.clear();

  QList<MyMoneySplit>::ConstIterator it;
  for (it = d->additionalFeesTransaction.splits().constBegin(); it != d->additionalFeesTransaction.splits().constEnd(); ++it) {
    if ((*it).accountId() != d->phonyAccount.id()) {
      list << (*it);
    }
  }
}

void LoanPaymentPage::updateAmounts()
{
  m_additionalFees->setText(d->additionalFees.formatMoney(m_wizard->currency().tradingSymbol(), m_wizard->precision()));
  m_totalPayment->setText((basePayment() + d->additionalFees).formatMoney(m_wizard->currency().tradingSymbol(), m_wizard->precision()));
}

void LoanPaymentPage::enterPage()
{
  const MyMoneySecurity& currency = m_wizard->currency();

  m_basePayment->setText(basePayment().formatMoney(currency.tradingSymbol(), m_wizard->precision()));
  d->phonyAccount.setCurrencyId(currency.id());
  d->additionalFeesTransaction.setCommodity(currency.id());

  updateAmounts();
}

void LoanPaymentPage::slotAdditionalFees()
{
  QMap<QString, MyMoneyMoney> priceInfo;
  QPointer<KSplitTransactionDlg> dlg = new KSplitTransactionDlg(d->additionalFeesTransaction, d->phonySplit, d->phonyAccount, false, !m_wizard->moneyBorrowed(), MyMoneyMoney(), priceInfo);

  // connect(dlg, SIGNAL(newCategory(MyMoneyAccount&)), this, SIGNAL(newCategory(MyMoneyAccount&)));

  if (dlg->exec() == QDialog::Accepted) {
    d->additionalFeesTransaction = dlg->transaction();
    // sum up the additional fees
    QList<MyMoneySplit>::ConstIterator it;

    d->additionalFees = MyMoneyMoney();
    for (it = d->additionalFeesTransaction.splits().constBegin(); it != d->additionalFeesTransaction.splits().constEnd(); ++it) {
      if ((*it).accountId() != d->phonyAccount.id()) {
        d->additionalFees += (*it).shares();
      }
    }
    updateAmounts();
  }

  delete dlg;
}

KMyMoneyWizardPage* LoanPaymentPage::nextPage() const
{
  return m_wizard->m_loanSchedulePage;
}


LoanSchedulePage::LoanSchedulePage(Wizard* wizard) :
    KLoanSchedulePageDecl(wizard),
    WizardPage<Wizard>(StepSchedule, this, wizard)
{
  m_mandatoryGroup->add(m_interestCategory->lineEdit());
  m_mandatoryGroup->add(m_paymentAccount->lineEdit());
  connect(m_interestCategory, SIGNAL(createItem(QString,QString&)), this, SLOT(slotCreateCategory(QString,QString&)));
  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadWidgets()));
}

void LoanSchedulePage::slotCreateCategory(const QString& name, QString& id)
{
  MyMoneyAccount acc, parent;
  acc.setName(name);

  if (m_wizard->moneyBorrowed())
    parent = MyMoneyFile::instance()->expense();
  else
    parent = MyMoneyFile::instance()->income();

  emit m_wizard->createCategory(acc, parent);

  // return id
  id = acc.id();
}

QDate LoanSchedulePage::firstPaymentDueDate() const
{
  if (m_firstPaymentDueDate->isEnabled())
    return m_firstPaymentDueDate->date();
  return m_wizard->m_generalLoanInfoPage->m_firstPaymentDate->date();
}

void LoanSchedulePage::enterPage()
{
  m_interestCategory->setFocus();
  m_firstPaymentDueDate->setDisabled(m_wizard->m_generalLoanInfoPage->recordAllPayments());
  slotLoadWidgets();
}

void LoanSchedulePage::slotLoadWidgets()
{
  AccountSet set;
  if (m_wizard->moneyBorrowed())
    set.addAccountGroup(Account::Expense);
  else
    set.addAccountGroup(Account::Income);
  set.load(m_interestCategory->selector());

  set.clear();
  set.addAccountGroup(Account::Asset);
  set.load(m_paymentAccount->selector());
}

KMyMoneyWizardPage* LoanSchedulePage::nextPage() const
{
  // if the balance widget of the general loan info page is enabled and
  // the value is not zero, then the payout already happened and we don't
  // aks for it.
  if (m_wizard->openingBalance().isZero())
    return m_wizard->m_loanPayoutPage;
  return m_wizard->m_accountSummaryPage;
}

LoanPayoutPage::LoanPayoutPage(Wizard* wizard) :
    KLoanPayoutPageDecl(wizard),
    WizardPage<Wizard>(StepPayout, this, wizard)
{
  m_mandatoryGroup->add(m_assetAccount->lineEdit());
  m_mandatoryGroup->add(m_loanAccount->lineEdit());

  KGuiItem createAssetButtenItem(i18n("&Create..."),
                                 QIcon::fromTheme(g_Icons[Icon::DocumentNew]),
                                 i18n("Create a new asset account"),
                                 i18n("If the asset account does not yet exist, press this button to create it."));
  KGuiItem::assign(m_createAssetButton, createAssetButtenItem);
  m_createAssetButton->setToolTip(createAssetButtenItem.toolTip());
  m_createAssetButton->setWhatsThis(createAssetButtenItem.whatsThis());
  connect(m_createAssetButton, SIGNAL(clicked()), this, SLOT(slotCreateAssetAccount()));

  connect(m_noPayoutTransaction, SIGNAL(toggled(bool)), this, SLOT(slotButtonsToggled()));
  connect(m_refinanceLoan, SIGNAL(toggled(bool)), this, SLOT(slotButtonsToggled()));

  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadWidgets()));
  slotLoadWidgets();
}

void LoanPayoutPage::slotButtonsToggled()
{
  // we don't go directly, as the order of the emission of signals to slots is
  // not defined. Using a single shot timer postpones the call of m_mandatoryGroup::changed()
  // until the next round of the main loop so we can be sure to see all relevant changes
  // that happened in the meantime (eg. widgets are enabled and disabled)
  QTimer::singleShot(0, m_mandatoryGroup, SLOT(changed()));
}

void LoanPayoutPage::slotCreateAssetAccount()
{
  MyMoneyAccount acc;
  acc.setAccountType(Account::Asset);
  acc.setOpeningDate(m_wizard->m_accountTypePage->m_openingDate->date());

  emit m_wizard->createAccount(acc);

  if (!acc.id().isEmpty()) {
    m_assetAccount->setSelectedItem(acc.id());
  }
}

void LoanPayoutPage::slotLoadWidgets()
{
  AccountSet set;
  set.addAccountGroup(Account::Asset);
  set.load(m_assetAccount->selector());

  set.clear();
  set.addAccountType(Account::Loan);
  set.load(m_loanAccount->selector());
}

void LoanPayoutPage::enterPage()
{
  // only allow to create new asset accounts for liability loans
  m_createAssetButton->setEnabled(m_wizard->moneyBorrowed());
  m_refinanceLoan->setEnabled(m_wizard->moneyBorrowed());
  if (!m_wizard->moneyBorrowed()) {
    m_refinanceLoan->setChecked(false);
  }
  m_payoutDetailFrame->setDisabled(m_noPayoutTransaction->isChecked());
}

KMyMoneyWizardPage* LoanPayoutPage::nextPage() const
{
  return m_wizard->m_accountSummaryPage;
}

bool LoanPayoutPage::isComplete() const
{
  return KMyMoneyWizardPage::isComplete() | m_noPayoutTransaction->isChecked();
}

const QString& LoanPayoutPage::payoutAccountId() const
{
  if (m_refinanceLoan->isChecked()) {
    return m_loanAccount->selectedItem();
  } else {
    return m_assetAccount->selectedItem();
  }
}

HierarchyFilterProxyModel::HierarchyFilterProxyModel(QObject *parent)
    : AccountsProxyModel(parent)
{
}

/**
  * Filter the favorites accounts group.
  */
bool HierarchyFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
  if (!source_parent.isValid()) {
    auto accCol = m_mdlColumns->indexOf(eAccountsModel::Column::Account);
    QVariant data = sourceModel()->index(source_row, accCol, source_parent).data((int)eAccountsModel::Role::ID);
    if (data.isValid() && data.toString() == AccountsModel::favoritesAccountId)
      return false;
  }
  return AccountsProxyModel::filterAcceptsRow(source_row, source_parent);
}

/**
  * Filter all but the first column.
  */
bool HierarchyFilterProxyModel::filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const
{
  Q_UNUSED(source_parent)
  if (source_column == 0)
    return true;
  return false;
}

HierarchyPage::HierarchyPage(Wizard* wizard) :
    KHierarchyPageDecl(wizard),
    WizardPage<Wizard>(StepParentAccount, this, wizard),
    m_filterProxyModel(nullptr)
{
  // the proxy filter model
  m_filterProxyModel = new HierarchyFilterProxyModel(this);
  m_filterProxyModel->setHideClosedAccounts(true);
  m_filterProxyModel->setHideEquityAccounts(!KMyMoneyGlobalSettings::expertMode());
  m_filterProxyModel->addAccountGroup(QVector<Account> {Account::Asset, Account::Liability});
  auto const model = Models::instance()->accountsModel();
  m_filterProxyModel->setSourceModel(model);
  m_filterProxyModel->setSourceColumns(model->getColumns());
  m_filterProxyModel->setDynamicSortFilter(true);

  m_parentAccounts->setModel(m_filterProxyModel);
  m_parentAccounts->sortByColumn((int)eAccountsModel::Column::Account, Qt::AscendingOrder);

  connect(m_parentAccounts->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(parentAccountChanged()));
}

void HierarchyPage::enterPage()
{
  // Ensure that the list reflects the Account Type
  MyMoneyAccount topAccount = m_wizard->m_accountTypePage->parentAccount();
  m_filterProxyModel->clear();
  m_filterProxyModel->addAccountGroup(QVector<Account> {topAccount.accountGroup()});
  m_parentAccounts->expandAll();
}

KMyMoneyWizardPage* HierarchyPage::nextPage() const
{
  return m_wizard->m_accountSummaryPage;
}

const MyMoneyAccount& HierarchyPage::parentAccount()
{
  QVariant data = m_parentAccounts->model()->data(m_parentAccounts->currentIndex(), (int)eAccountsModel::Role::Account);
  if (data.isValid()) {
    m_parentAccount = data.value<MyMoneyAccount>();
  } else {
    m_parentAccount = MyMoneyAccount();
  }
  return m_parentAccount;
}

bool HierarchyPage::isComplete() const
{
  return m_parentAccounts->currentIndex().isValid();
}

void HierarchyPage::parentAccountChanged()
{
  completeStateChanged();
}

AccountSummaryPage::AccountSummaryPage(Wizard* wizard) :
    KAccountSummaryPageDecl(wizard),
    WizardPage<Wizard>(StepFinish, this, wizard)
{
}

void AccountSummaryPage::enterPage()
{
  MyMoneyAccount acc = m_wizard->account();
  MyMoneySecurity sec = m_wizard->currency();
  acc.fraction(sec);

  // assign an id to the account inside the wizard which is required for a schedule
  // get the schedule and clear the id again in the wizards object.
  MyMoneyAccount tmp(QLatin1String("Phony-ID"), acc);
  m_wizard->setAccount(tmp);
  MyMoneySchedule sch = m_wizard->schedule();
  m_wizard->setAccount(acc);

  m_dataList->clear();



  // Account data
  m_dataList->setFontWeight(QFont::Bold);
  m_dataList->append(i18n("Account information"));
  m_dataList->setFontWeight(QFont::Normal);
  m_dataList->append(i18nc("Account name", "Name: %1", acc.name()));
  if (!acc.isLoan())
    m_dataList->append(i18n("Subaccount of %1", m_wizard->parentAccount().name()));
  QString accTypeText;
  if (acc.accountType() == Account::AssetLoan)
    accTypeText = i18n("Loan");
  else
    accTypeText = m_wizard->m_accountTypePage->m_typeSelection->currentText();
  m_dataList->append(i18n("Type: %1", accTypeText));

  m_dataList->append(i18n("Currency: %1", m_wizard->currency().name()));
  m_dataList->append(i18n("Opening date: %1", QLocale().toString(acc.openingDate())));
  if (m_wizard->currency().id() != MyMoneyFile::instance()->baseCurrency().id()) {
    m_dataList->append(i18n("Conversion rate: %1", m_wizard->conversionRate().rate(QString()).formatMoney("", m_wizard->currency().pricePrecision())));
  }
  if (!acc.isLoan() || !m_wizard->openingBalance().isZero())
    m_dataList->append(i18n("Opening balance: %1", MyMoneyUtils::formatMoney(m_wizard->openingBalance(), acc, sec)));

  if (!m_wizard->m_institutionPage->institution().id().isEmpty()) {
    m_dataList->append(i18n("Institution: %1", m_wizard->m_institutionPage->institution().name()));
    if (!acc.number().isEmpty()) {
      m_dataList->append(i18n("Number: %1", acc.number()));
    }
    if (!acc.value("iban").isEmpty()) {
      m_dataList->append(i18n("IBAN: %1", acc.value("iban")));
    }
  }

  if (acc.accountType() == Account::Investment) {
    if (m_wizard->m_brokeragepage->m_createBrokerageButton->isChecked()) {
      m_dataList->setFontWeight(QFont::Bold);
      m_dataList->append(i18n("Brokerage Account"));
      m_dataList->setFontWeight(QFont::Normal);

      m_dataList->append(i18nc("Account name", "Name: %1 (Brokerage)", acc.name()));
      m_dataList->append(i18n("Currency: %1", m_wizard->m_brokeragepage->m_brokerageCurrency->security().name()));
      if (m_wizard->m_brokeragepage->m_accountNumber->isEnabled() && !m_wizard->m_brokeragepage->m_accountNumber->text().isEmpty())
        m_dataList->append(i18n("Number: %1", m_wizard->m_brokeragepage->m_accountNumber->text()));
      if (m_wizard->m_brokeragepage->m_iban->isEnabled() && !m_wizard->m_brokeragepage->m_iban->text().isEmpty())
        m_dataList->append(i18n("IBAN: %1", m_wizard->m_brokeragepage->m_iban->text()));
    }
  }

  // Loan
  if (acc.isLoan()) {
    m_dataList->setFontWeight(QFont::Bold);
    m_dataList->append(i18n("Loan information"));
    m_dataList->setFontWeight(QFont::Normal);
    if (m_wizard->moneyBorrowed()) {
      m_dataList->append(i18n("Amount borrowed: %1", m_wizard->m_loanDetailsPage->m_loanAmount->value().formatMoney(m_wizard->currency().tradingSymbol(), m_wizard->precision())));
    } else {
      m_dataList->append(i18n("Amount lent: %1", m_wizard->m_loanDetailsPage->m_loanAmount->value().formatMoney(m_wizard->currency().tradingSymbol(), m_wizard->precision())));
    }
    m_dataList->append(i18n("Interest rate: %1 %", m_wizard->m_loanDetailsPage->m_interestRate->value().formatMoney("", -1)));
    m_dataList->append(i18n("Interest rate is %1", m_wizard->m_generalLoanInfoPage->m_interestType->currentText()));
    m_dataList->append(i18n("Principal and interest: %1", MyMoneyUtils::formatMoney(m_wizard->m_loanDetailsPage->m_paymentAmount->value(), acc, sec)));
    m_dataList->append(i18n("Additional Fees: %1", MyMoneyUtils::formatMoney(m_wizard->m_loanPaymentPage->additionalFees(), acc, sec)));
    m_dataList->append(i18n("Payment frequency: %1", m_wizard->m_generalLoanInfoPage->m_paymentFrequency->currentText()));
    m_dataList->append(i18n("Payment account: %1", m_wizard->m_loanSchedulePage->m_paymentAccount->currentText()));

    if (!m_wizard->m_loanPayoutPage->m_noPayoutTransaction->isChecked() && m_wizard->openingBalance().isZero()) {
      m_dataList->setFontWeight(QFont::Bold);
      m_dataList->append(i18n("Payout information"));
      m_dataList->setFontWeight(QFont::Normal);
      if (m_wizard->m_loanPayoutPage->m_refinanceLoan->isChecked()) {
        m_dataList->append(i18n("Refinance: %1", m_wizard->m_loanPayoutPage->m_loanAccount->currentText()));
      } else {
        if (m_wizard->moneyBorrowed())
          m_dataList->append(i18n("Transfer amount to %1", m_wizard->m_loanPayoutPage->m_assetAccount->currentText()));
        else
          m_dataList->append(i18n("Transfer amount from %1", m_wizard->m_loanPayoutPage->m_assetAccount->currentText()));
      }
      m_dataList->append(i18n("Payment date: %1 ", QLocale().toString(m_wizard->m_loanPayoutPage->m_payoutDate->date())));
    }
  }

  // Schedule
  if (!(sch == MyMoneySchedule())) {
    m_dataList->setFontWeight(QFont::Bold);
    m_dataList->append(i18n("Schedule information"));
    m_dataList->setFontWeight(QFont::Normal);
    m_dataList->append(i18nc("Schedule name", "Name: %1", sch.name()));
    if (acc.accountType() == Account::CreditCard) {
      MyMoneyAccount paymentAccount = MyMoneyFile::instance()->account(m_wizard->m_schedulePage->m_paymentAccount->selectedItem());
      m_dataList->append(i18n("Occurrence: Monthly"));
      m_dataList->append(i18n("Paid from %1", paymentAccount.name()));
      m_dataList->append(i18n("Pay to %1", m_wizard->m_schedulePage->m_payee->currentText()));
      m_dataList->append(i18n("Amount: %1", MyMoneyUtils::formatMoney(m_wizard->m_schedulePage->m_amount->value(), acc, sec)));
      m_dataList->append(i18n("First payment due on %1", QLocale().toString(sch.nextDueDate())));
      m_dataList->append(i18n("Payment method: %1", m_wizard->m_schedulePage->m_method->currentText()));
    }
    if (acc.isLoan()) {
      m_dataList->append(i18n("Occurrence: %1", m_wizard->m_generalLoanInfoPage->m_paymentFrequency->currentText()));
      m_dataList->append(i18n("Amount: %1", MyMoneyUtils::formatMoney(m_wizard->m_loanPaymentPage->basePayment() + m_wizard->m_loanPaymentPage->additionalFees(), acc, sec)));
      m_dataList->append(i18n("First payment due on %1", QLocale().toString(m_wizard->m_loanSchedulePage->firstPaymentDueDate())));
    }
  }
}
}
