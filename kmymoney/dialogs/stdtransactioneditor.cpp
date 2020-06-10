/*
 * Copyright 2009-2018  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "stdtransactioneditor.h"
#include "transactioneditor_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QLabel>
#include <QApplication>
#include <QList>
#include <QPushButton>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KTextEdit>
#include <KLocalizedString>
#include <KMessageBox>
#include <KStandardGuiItem>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyreconcilecombo.h"
#include "kmymoneycashflowcombo.h"
#include "kmymoneypayeecombo.h"
#include "kmymoneytagcombo.h"
#include "ktagcontainer.h"
#include "tabbar.h"
#include "kmymoneycategory.h"
#include "kmymoneymvccombo.h"
#include "kmymoneydateinput.h"
#include "amountedit.h"
#include "kmymoneylineedit.h"
#include "kmymoneyaccountselector.h"
#include "mymoneyfile.h"
#include "mymoneypayee.h"
#include "mymoneytag.h"
#include "kmymoneyutils.h"
#include "kmymoneycompletion.h"
#include "transaction.h"
#include "transactionform.h"
#include "mymoneytransactionfilter.h"
#include "kmymoneysettings.h"
#include "transactioneditorcontainer.h"

#include "ksplittransactiondlg.h"
#include "kcurrencycalculator.h"
#include "kselecttransactionsdlg.h"
#include "widgetenums.h"

using namespace eWidgets;
using namespace KMyMoneyRegister;
using namespace KMyMoneyTransactionForm;

class StdTransactionEditorPrivate : public TransactionEditorPrivate
{
  Q_DISABLE_COPY(StdTransactionEditorPrivate)

public:
  explicit StdTransactionEditorPrivate(StdTransactionEditor *qq) :
    TransactionEditorPrivate(qq),
    m_inUpdateVat(false)
  {
  }

  ~StdTransactionEditorPrivate()
  {
  }

  MyMoneyMoney m_shares;
  bool         m_inUpdateVat;
};

StdTransactionEditor::StdTransactionEditor() :
  TransactionEditor(*new StdTransactionEditorPrivate(this))
{
}

StdTransactionEditor::StdTransactionEditor(TransactionEditorContainer* regForm,
                                           KMyMoneyRegister::Transaction* item,
                                           const KMyMoneyRegister::SelectedTransactions& list,
                                           const QDate& lastPostDate) :
    TransactionEditor(*new StdTransactionEditorPrivate(this),
                      regForm,
                      item,
                      list,
                      lastPostDate)
{
}

StdTransactionEditor::~StdTransactionEditor()
{
}

void StdTransactionEditor::createEditWidgets()
{
  Q_D(StdTransactionEditor);
  // we only create the account widget in case it is needed
  // to avoid confusion in the tab order later on.
  if (d->m_item->showRowInForm(0)) {
    auto account = new KMyMoneyCategory;
    account->setPlaceholderText(i18n("Account"));
    account->setObjectName(QLatin1String("Account"));
    d->m_editWidgets["account"] = account;
    connect(account, &QComboBox::editTextChanged, this, &StdTransactionEditor::slotUpdateButtonState);
    connect(account, &KMyMoneyCombo::itemSelected, this, &StdTransactionEditor::slotUpdateAccount);
  }

  auto payee = new KMyMoneyPayeeCombo;
  payee->setPlaceholderText(i18n("Payer/Receiver"));
  payee->setObjectName(QLatin1String("Payee"));
  d->m_editWidgets["payee"] = payee;

  connect(payee, &KMyMoneyMVCCombo::createItem, this, &StdTransactionEditor::slotNewPayee);
  connect(payee, &KMyMoneyMVCCombo::objectCreation, this, &StdTransactionEditor::objectCreation);
  connect(payee, &KMyMoneyMVCCombo::itemSelected, this, &StdTransactionEditor::slotUpdatePayee);
  connect(payee, &QComboBox::editTextChanged, this, &StdTransactionEditor::slotUpdateButtonState);

  auto category = new KMyMoneyCategory(true, nullptr);
  category->setPlaceholderText(i18n("Category/Account"));
  category->setObjectName(QLatin1String("Category/Account"));
  d->m_editWidgets["category"] = category;
  connect(category, &KMyMoneyCombo::itemSelected, this, &StdTransactionEditor::slotUpdateCategory);
  connect(category, &QComboBox::editTextChanged, this, &StdTransactionEditor::slotUpdateButtonState);
  connect(category, &KMyMoneyCombo::createItem, this, &StdTransactionEditor::slotCreateCategory);
  connect(category, &KMyMoneyCombo::objectCreation, this, &StdTransactionEditor::objectCreation);
  connect(category->splitButton(), &QAbstractButton::clicked, this, &StdTransactionEditor::slotEditSplits);
  // initially disable the split button since we don't have an account set
  if (category->splitButton())
    category->splitButton()->setDisabled(d->m_account.id().isEmpty());

  auto tag = new KTagContainer;
  tag->tagCombo()->setPlaceholderText(i18n("Tag"));
  tag->tagCombo()->setObjectName(QLatin1String("Tag"));
  d->m_editWidgets["tag"] = tag;
  connect(tag->tagCombo(), &KMyMoneyMVCCombo::createItem, this, &StdTransactionEditor::slotNewTag);
  connect(tag->tagCombo(), &KMyMoneyMVCCombo::objectCreation, this, &StdTransactionEditor::objectCreation);

  auto memo = new KTextEdit;
  memo->setObjectName(QLatin1String("Memo"));
  memo->setTabChangesFocus(true);
  connect(memo, &QTextEdit::textChanged, this, &StdTransactionEditor::slotUpdateMemoState);
  connect(memo, &QTextEdit::textChanged, this, &StdTransactionEditor::slotUpdateButtonState);
  d->m_editWidgets["memo"] = memo;
  d->m_memoText.clear();
  d->m_memoChanged = false;

  bool showNumberField = true;
  switch (d->m_account.accountType()) {
    case eMyMoney::Account::Type::Savings:
    case eMyMoney::Account::Type::Cash:
    case eMyMoney::Account::Type::Loan:
    case eMyMoney::Account::Type::AssetLoan:
    case eMyMoney::Account::Type::Asset:
    case eMyMoney::Account::Type::Liability:
    case eMyMoney::Account::Type::Equity:
      showNumberField = KMyMoneySettings::alwaysShowNrField();
      break;

    case eMyMoney::Account::Type::Income:
    case eMyMoney::Account::Type::Expense:
      showNumberField = false;
      break;

    default:
      break;
  }

  if (showNumberField) {
    auto number = new KMyMoneyLineEdit;
    number->setPlaceholderText(i18n("Number"));
    number->setObjectName(QLatin1String("Number"));
    d->m_editWidgets["number"] = number;
    connect(number, &KMyMoneyLineEdit::lineChanged, this, &StdTransactionEditor::slotNumberChanged);
    // number->installEventFilter(this);
  }

  auto postDate = new KMyMoneyDateInput;
  d->m_editWidgets["postdate"] = postDate;
  postDate->setObjectName(QLatin1String("PostDate"));
  connect(postDate, &KMyMoneyDateInput::dateChanged, this, &StdTransactionEditor::slotUpdateButtonState);
  postDate->setDate(QDate());

  auto value = new AmountEdit;
  d->m_editWidgets["amount"] = value;
  value->setObjectName(QLatin1String("Amount"));
  value->setCalculatorButtonVisible(true);
  connect(value, &AmountEdit::valueChanged, this, &StdTransactionEditor::slotUpdateAmount);
  connect(value, &AmountEdit::textChanged, this, &StdTransactionEditor::slotUpdateButtonState);

  value = new AmountEdit;
  d->m_editWidgets["payment"] = value;
  value->setObjectName(QLatin1String("Payment"));
  value->setCalculatorButtonVisible(true);
  connect(value, &AmountEdit::valueChanged, this, &StdTransactionEditor::slotUpdatePayment);
  connect(value, &AmountEdit::textChanged, this, &StdTransactionEditor::slotUpdateButtonState);

  value = new AmountEdit;
  d->m_editWidgets["deposit"] = value;
  value->setObjectName(QLatin1String("Deposit"));
  value->setCalculatorButtonVisible(true);
  connect(value, &AmountEdit::valueChanged, this, &StdTransactionEditor::slotUpdateDeposit);
  connect(value, &AmountEdit::textChanged, this, &StdTransactionEditor::slotUpdateButtonState);

  auto cashflow = new KMyMoneyCashFlowCombo(d->m_account.accountGroup(), nullptr);
  d->m_editWidgets["cashflow"] = cashflow;
  cashflow->setObjectName(QLatin1String("Cashflow"));
  connect(cashflow, &KMyMoneyCashFlowCombo::directionSelected, this, &StdTransactionEditor::slotUpdateCashFlow);

  auto reconcile = new KMyMoneyReconcileCombo;
  d->m_editWidgets["status"] = reconcile;
  reconcile->setObjectName(QLatin1String("Reconcile"));

  KMyMoneyRegister::QWidgetContainer::iterator it_w;
  for (it_w = d->m_editWidgets.begin(); it_w != d->m_editWidgets.end(); ++it_w) {
    (*it_w)->installEventFilter(this);
  }
  // if we don't have more than 1 selected transaction, we don't need
  // the "don't change" item in some of the combo widgets
  if (!isMultiSelection()) {
    reconcile->removeDontCare();
    cashflow->removeDontCare();
  }

  QLabel* label;
  d->m_editWidgets["account-label"] = label = new QLabel(i18n("Account"));
  label->setAlignment(Qt::AlignVCenter);

  d->m_editWidgets["category-label"] = label = new QLabel(i18n("Category"));
  label->setAlignment(Qt::AlignVCenter);

  d->m_editWidgets["tag-label"] = label = new QLabel(i18n("Tags"));
  label->setAlignment(Qt::AlignVCenter);

  d->m_editWidgets["memo-label"] = label = new QLabel(i18n("Memo"));
  label->setAlignment(Qt::AlignVCenter);

  d->m_editWidgets["number-label"] = label = new QLabel(i18n("Number"));
  label->setAlignment(Qt::AlignVCenter);

  d->m_editWidgets["date-label"] = label = new QLabel(i18n("Date"));
  label->setAlignment(Qt::AlignVCenter);

  d->m_editWidgets["amount-label"] = label = new QLabel(i18n("Amount"));
  label->setAlignment(Qt::AlignVCenter);

  d->m_editWidgets["status-label"] = label = new QLabel(i18n("Status"));
  label->setAlignment(Qt::AlignVCenter);

  // create a copy of tabbar above the form (if we are created for a form)
  auto form = dynamic_cast<KMyMoneyTransactionForm::TransactionForm*>(d->m_regForm);
  if (form) {
    auto tabbar = new KMyMoneyTransactionForm::TabBar;
    d->m_editWidgets["tabbar"] = tabbar;
    tabbar->setObjectName(QLatin1String("TabBar"));
    tabbar->copyTabs(form->getTabBar());
    connect(tabbar, &KMyMoneyTransactionForm::TabBar::tabCurrentChanged, this, &StdTransactionEditor::slotUpdateAction);
    connect(tabbar, &KMyMoneyTransactionForm::TabBar::tabCurrentChanged, this, &TransactionEditor::operationTypeChanged);
  }

  setupPrecision();
}

void StdTransactionEditor::setupCategoryWidget(QString& categoryId)
{
  Q_D(StdTransactionEditor);
  if (auto categoryWidget = dynamic_cast<KMyMoneyCategory*>(d->m_editWidgets["category"]))
    TransactionEditor::setupCategoryWidget(categoryWidget, d->m_splits, categoryId, SLOT(slotEditSplits()));

  if (d->m_splits.count() == 1)
    d->m_shares = d->m_splits[0].shares();
}

bool StdTransactionEditor::isTransfer(const QString& accId1, const QString& accId2) const
{
  if (accId1.isEmpty() || accId2.isEmpty())
    return false;

  return MyMoneyFile::instance()->account(accId1).isIncomeExpense() == MyMoneyFile::instance()->account(accId2).isIncomeExpense();
}

void StdTransactionEditor::loadEditWidgets(eRegister::Action action)
{
  Q_D(StdTransactionEditor);
  // don't kick off VAT processing from here
  d->m_inUpdateVat = true;

  QWidget* w;
  AccountSet aSet;

  // load the account widget
  if (auto account = dynamic_cast<KMyMoneyCategory*>(haveWidget("account"))) {
    aSet.addAccountGroup(eMyMoney::Account::Type::Asset);
    aSet.addAccountGroup(eMyMoney::Account::Type::Liability);
    aSet.removeAccountType(eMyMoney::Account::Type::AssetLoan);
    aSet.removeAccountType(eMyMoney::Account::Type::CertificateDep);
    aSet.removeAccountType(eMyMoney::Account::Type::Investment);
    aSet.removeAccountType(eMyMoney::Account::Type::Stock);
    aSet.removeAccountType(eMyMoney::Account::Type::MoneyMarket);
    aSet.removeAccountType(eMyMoney::Account::Type::Loan);
    aSet.load(account->selector());
    account->completion()->setSelected(d->m_account.id());
    account->slotItemSelected(d->m_account.id());
  }

  // load the payee widget
  auto payee = dynamic_cast<KMyMoneyPayeeCombo*>(d->m_editWidgets["payee"]);
  if (payee)
    payee->loadPayees(MyMoneyFile::instance()->payeeList());

  // load the category widget
  auto category = dynamic_cast<KMyMoneyCategory*>(d->m_editWidgets["category"]);

  if (category)
    disconnect(category, &KMyMoneyCategory::focusIn, this, &StdTransactionEditor::slotEditSplits);

  // load the tag widget
  //auto tag = dynamic_cast<KMyMoneyTagCombo*>(m_editWidgets["tag"]);
  auto tag = dynamic_cast<KTagContainer*>(d->m_editWidgets["tag"]);
  if (tag)
    tag->loadTags(MyMoneyFile::instance()->tagList());

  // check if the current transaction has a reference to an equity account
  auto haveEquityAccount = false;
  foreach (const auto split, d->m_transaction.splits()) {
    auto acc = MyMoneyFile::instance()->account(split.accountId());
    if (acc.accountType() == eMyMoney::Account::Type::Equity) {
      haveEquityAccount = true;
      break;
    }
  }

  aSet.clear();
  aSet.addAccountGroup(eMyMoney::Account::Type::Asset);
  aSet.addAccountGroup(eMyMoney::Account::Type::Liability);
  aSet.addAccountGroup(eMyMoney::Account::Type::Income);
  aSet.addAccountGroup(eMyMoney::Account::Type::Expense);
  if (KMyMoneySettings::expertMode() || haveEquityAccount)
    aSet.addAccountGroup(eMyMoney::Account::Type::Equity);

  aSet.removeAccountType(eMyMoney::Account::Type::CertificateDep);
  aSet.removeAccountType(eMyMoney::Account::Type::Investment);
  aSet.removeAccountType(eMyMoney::Account::Type::Stock);
  aSet.removeAccountType(eMyMoney::Account::Type::MoneyMarket);
  if (category)
    aSet.load(category->selector());

  // if an account is specified then remove it from the widget so that the user
  // cannot create a transfer with from and to account being the same account
  if (category && !d->m_account.id().isEmpty())
    category->selector()->removeItem(d->m_account.id());

  //  also show memo text if isMultiSelection()
  if (auto memoWidget = dynamic_cast<KTextEdit*>(d->m_editWidgets["memo"]))
    memoWidget->setText(d->m_split.memo());
  // need to know if it changed
  d->m_memoText = d->m_split.memo();
  d->m_memoChanged = false;

  if (!isMultiSelection()) {
    if (auto dateWidget = dynamic_cast<KMyMoneyDateInput*>(d->m_editWidgets["postdate"])) {
      if (d->m_transaction.postDate().isValid())
        dateWidget->setDate(d->m_transaction.postDate());
      else if (d->m_lastPostDate.isValid())
        dateWidget->setDate(d->m_lastPostDate);
      else
        dateWidget->setDate(QDate::currentDate());
    }

    if ((w = haveWidget("number")) != 0) {
      if (auto lineEdit = dynamic_cast<KMyMoneyLineEdit*>(w))
        lineEdit->loadText(d->m_split.number());
      if (d->m_transaction.id().isEmpty()                              // new transaction
          && dynamic_cast<KMyMoneyLineEdit*>(w)->text().isEmpty()   // no number filled in
          && d->m_account.accountType() == eMyMoney::Account::Type::Checkings   // checkings account
          && KMyMoneySettings::autoIncCheckNumber()           // and auto inc number turned on?
          && action != eRegister::Action::Deposit              // only transfers or withdrawals
          && d->m_paymentMethod == eMyMoney::Schedule::PaymentType::WriteChecque) {// only for WriteChecque
        assignNextNumber();
      }
    }
    if (auto statusWidget = dynamic_cast<KMyMoneyReconcileCombo*>(d->m_editWidgets["status"]))
      statusWidget->setState(d->m_split.reconcileFlag());

    QString payeeId = d->m_split.payeeId();
    if (payee && !payeeId.isEmpty())
      payee->setSelectedItem(payeeId);

    QList<QString> t = d->m_split.tagIdList();
    if (tag && !t.isEmpty())
      for (auto i = 0; i < t.size(); ++i)
        tag->addTagWidget(t[i]);

    d->m_splits.clear();
    if (d->m_transaction.splitCount() < 2) {
      if (category && category->completion()) {
        category->completion()->setSelected(QString());
      }
    } else {
      foreach (const auto split, d->m_transaction.splits()) {
        if (split == d->m_split)
          continue;
        d->m_splits.append(split);
      }
    }
    QString categoryId;
    setupCategoryWidget(categoryId);

    if ((w = haveWidget("cashflow")) != 0) {
      if (auto cashflow = dynamic_cast<KMyMoneyCashFlowCombo*>(w))
        cashflow->setDirection(!d->m_split.value().isPositive() ? eRegister::CashFlowDirection::Payment : eRegister::CashFlowDirection::Deposit);  //  include isZero case
    }

    if ((w = haveWidget("category-label")) != 0) {
      if (auto categoryLabel = dynamic_cast<QLabel*>(w)) {
        if (isTransfer(d->m_split.accountId(), categoryId)) {
          if (d->m_split.value().isPositive())
            categoryLabel->setText(i18n("Transfer from"));
          else
            categoryLabel->setText(i18n("Transfer to"));
        }
      }
    }

    MyMoneyMoney value = d->m_split.shares();

    if (haveWidget("deposit")) {
      auto depositWidget = dynamic_cast<AmountEdit*>(d->m_editWidgets["deposit"]);
      auto paymentWidget = dynamic_cast<AmountEdit*>(d->m_editWidgets["payment"]);
      if (depositWidget && paymentWidget) {
        if (d->m_split.shares().isNegative()) {
          depositWidget->setText(QString());
          paymentWidget->setValue(value.abs());
        } else {
          depositWidget->setValue(value.abs());
          paymentWidget->setText(QString());
        }
      }
    }
    if ((w = haveWidget("amount")) != 0) {
      if (auto amountWidget = dynamic_cast<AmountEdit*>(w))
        amountWidget->setValue(value.abs());
    }

    slotUpdateCategory(categoryId);

    // try to preset for specific action if a new transaction is being started
    if (d->m_transaction.id().isEmpty()) {
      if ((w = haveWidget("category-label")) != 0) {
        auto tabbar = dynamic_cast<KMyMoneyTransactionForm::TabBar*>(haveWidget("tabbar"));
        if (action == eRegister::Action::None) {
          if (tabbar) {
            action = static_cast<eRegister::Action>(tabbar->currentIndex());
          }
        }
        if (action != eRegister::Action::None) {
          if (auto categoryLabel = dynamic_cast<QLabel*>(w)) {
            if (action == eRegister::Action::Transfer) {
              if (d->m_split.value().isPositive())
                categoryLabel->setText(i18n("Transfer from"));
              else
                categoryLabel->setText(i18n("Transfer to"));
            }
          }
          if ((w = haveWidget("cashflow")) != 0) {
            if (auto cashflow = dynamic_cast<KMyMoneyCashFlowCombo*>(w)) {
              if (action == eRegister::Action::Deposit || (action == eRegister::Action::Transfer && d->m_split.value().isPositive()))
                cashflow->setDirection(eRegister::CashFlowDirection::Deposit);
              else
                cashflow->setDirection(eRegister::CashFlowDirection::Payment);
            }
          }
          if (tabbar) {
            tabbar->setCurrentIndex((int)action);
          }
        }
      }
    } else {
      if (auto tabbar = dynamic_cast<KMyMoneyTransactionForm::TabBar*>(haveWidget("tabbar"))) {
        if (!isTransfer(d->m_split.accountId(), categoryId))
          tabbar->setCurrentIndex(d->m_split.value().isNegative() ? (int)eRegister::Action::Withdrawal : (int)eRegister::Action::Deposit);
        else
          tabbar->setCurrentIndex((int)eRegister::Action::Transfer);
      }
    }

  } else {  //  isMultiSelection()
    if (auto postDateWidget = dynamic_cast<KMyMoneyDateInput*>(d->m_editWidgets["postdate"]))
      postDateWidget->loadDate(QDate());
    if (auto statusWidget = dynamic_cast<KMyMoneyReconcileCombo*>(d->m_editWidgets["status"]))
      statusWidget->setState(eMyMoney::Split::State::Unknown);
    if (haveWidget("deposit")) {
      if (auto depositWidget = dynamic_cast<AmountEdit*>(d->m_editWidgets["deposit"])) {
        depositWidget->setText(QString());
        depositWidget->setAllowEmpty();
      }
      if (auto paymentWidget = dynamic_cast<AmountEdit*>(d->m_editWidgets["payment"])) {
        paymentWidget->setText(QString());
        paymentWidget->setAllowEmpty();
      }
    }
    if ((w = haveWidget("amount")) != 0) {
      if (auto amountWidget = dynamic_cast<AmountEdit*>(w)) {
        amountWidget->setText(QString());
        amountWidget->setAllowEmpty();
      }
    }

    slotUpdateAction((int)action);

    if ((w = haveWidget("tabbar")) != 0) {
      w->setEnabled(false);
    }

    if (category && category->completion())
      category->completion()->setSelected(QString());
  }

  // allow kick off VAT processing again
  d->m_inUpdateVat = false;
}

void StdTransactionEditor::loadEditWidgets()
{
  loadEditWidgets(eRegister::Action::None);
}

QWidget* StdTransactionEditor::firstWidget() const
{
  Q_D(const StdTransactionEditor);
  QWidget* w = nullptr;
  if (d->m_initialAction != eRegister::Action::None) {
    w = haveWidget("payee");
  }
  return w;
}

void StdTransactionEditor::slotReloadEditWidgets()
{
  Q_D(StdTransactionEditor);
  // reload category widget
  if (auto category = dynamic_cast<KMyMoneyCategory*>(d->m_editWidgets["category"])) {
    QString categoryId = category->selectedItem();

    AccountSet aSet;
    aSet.addAccountGroup(eMyMoney::Account::Type::Asset);
    aSet.addAccountGroup(eMyMoney::Account::Type::Liability);
    aSet.addAccountGroup(eMyMoney::Account::Type::Income);
    aSet.addAccountGroup(eMyMoney::Account::Type::Expense);
    if (KMyMoneySettings::expertMode())
      aSet.addAccountGroup(eMyMoney::Account::Type::Equity);
    aSet.load(category->selector());

    // if an account is specified then remove it from the widget so that the user
    // cannot create a transfer with from and to account being the same account
    if (!d->m_account.id().isEmpty())
      category->selector()->removeItem(d->m_account.id());

    if (!categoryId.isEmpty())
      category->setSelectedItem(categoryId);
  }


  // reload payee widget
  if (auto payee = dynamic_cast<KMyMoneyPayeeCombo*>(d->m_editWidgets["payee"])) {
    QString payeeId = payee->selectedItem();

    payee->loadPayees(MyMoneyFile::instance()->payeeList());

    if (!payeeId.isEmpty()) {
      payee->setSelectedItem(payeeId);
    }
  }

  // reload tag widget
  if (auto tag = dynamic_cast<KTagContainer*>(d->m_editWidgets["tag"])) {
    QString tagId = tag->tagCombo()->selectedItem();

    tag->loadTags(MyMoneyFile::instance()->tagList());

    if (!tagId.isEmpty()) {
      tag->RemoveAllTagWidgets();
      tag->addTagWidget(tagId);
    }
  }
}

void StdTransactionEditor::slotUpdatePayee(const QString& payeeId)
{
  // in case of an empty payee, there is nothing to do
  if (payeeId.isEmpty())
    return;

  Q_D(StdTransactionEditor);
  // we have a new payee assigned to this transaction.
  // in case there is no category assigned, no value entered and no
  // memo available, we search for the last transaction of this payee
  // in the account.
  if (d->m_transaction.id().isEmpty()
      && d->m_splits.count() == 0
      && KMyMoneySettings::autoFillTransaction() != 0) {
    // check if category is empty
    if (auto category = dynamic_cast<KMyMoneyCategory*>(d->m_editWidgets["category"])) {
      QStringList list;
      category->selectedItems(list);
      if (!list.isEmpty())
        return;
    }

    // check if memo is empty
    auto memo = dynamic_cast<KTextEdit*>(d->m_editWidgets["memo"]);
    if (memo && !memo->toPlainText().isEmpty())
      return;

    // check if all value fields are empty
    QStringList fields;
    fields << "amount" << "payment" << "deposit";
    QStringList::const_iterator it_f;
    for (it_f = fields.constBegin(); it_f != fields.constEnd(); ++it_f) {
      const auto amount = dynamic_cast<AmountEdit*>(haveWidget(*it_f));
      if (amount && !amount->value().isZero())
        return;
    }

#if 0
    // Tony mentioned, that autofill does not work when he changed the date. Well,
    // that certainly makes sense when you enter transactions in register mode as
    // opposed to form mode, because the date field is located prior to the date
    // field in the tab order of the widgets and the user might have already
    // changed it.
    //
    // So I commented out the code that checks the date but left it in for reference.
    // (ipwizard, 2008-04-07)

    // check if date has been altered by user
    auto postDate = dynamic_cast<KMyMoneyDateInput*>(m_editWidgets["postdate"]);
    if (postDate && (m_lastPostDate.isValid() && (postDate->date() != m_lastPostDate))
        || (!m_lastPostDate.isValid() && (postDate->date() != QDate::currentDate())))
      return;
#endif

    // if we got here, we have to autofill
    autoFill(payeeId);
  }

  // If payee has associated default account (category), set that now.
  const MyMoneyPayee& payeeObj = MyMoneyFile::instance()->payee(payeeId);
  if (!payeeObj.defaultAccountId().isEmpty()) {
    if (auto category = dynamic_cast<KMyMoneyCategory*>(d->m_editWidgets["category"]))
      category->slotItemSelected(payeeObj.defaultAccountId());
  }
}

MyMoneyMoney StdTransactionEditor::shares(const MyMoneyTransaction& t) const
{
  Q_D(const StdTransactionEditor);
  MyMoneyMoney result;
  foreach (const auto split, t.splits()) {
    if (split.accountId() == d->m_account.id()) {
      result += split.shares();
    }
  }
  return result;
}

struct uniqTransaction {
  const MyMoneyTransaction* t;
  int   cnt;
};

void StdTransactionEditor::autoFill(const QString& payeeId)
{
  Q_D(StdTransactionEditor);
  QList<QPair<MyMoneyTransaction, MyMoneySplit> >  list;
  MyMoneyTransactionFilter filter(d->m_account.id());
  filter.addPayee(payeeId);
  MyMoneyFile::instance()->transactionList(list, filter);
  if (!list.empty()) {
    // ok, we found at least one previous transaction. now we clear out
    // what we have collected so far and add those splits from
    // the previous transaction.
    QList<QPair<MyMoneyTransaction, MyMoneySplit> >::const_iterator  it_t;
    QMap<QString, struct uniqTransaction> uniqList;

    // collect the transactions and see if we have any duplicates
    for (it_t = list.constBegin(); it_t != list.constEnd(); ++it_t) {
      QString key = (*it_t).first.accountSignature();
      int cnt = 0;
      QMap<QString, struct uniqTransaction>::iterator it_u;
      do {
        QString ukey = QString("%1-%2").arg(key).arg(cnt);
        it_u = uniqList.find(ukey);
        if (it_u == uniqList.end()) {
          uniqList[ukey].t = &((*it_t).first);
          uniqList[ukey].cnt = 1;
        } else if (KMyMoneySettings::autoFillTransaction() == 1) {
          // we already have a transaction with this signature. we must
          // now check, if we should really treat it as a duplicate according
          // to the value comparison delta.
          MyMoneyMoney s1 = shares(*((*it_u).t));
          MyMoneyMoney s2 = shares((*it_t).first);
          if (s2.abs() > s1.abs()) {
            MyMoneyMoney t(s1);
            s1 = s2;
            s2 = t;
          }
          MyMoneyMoney diff;
          if (s2.isZero())
            diff = s1.abs();
          else
            diff = ((s1 - s2) / s2).convert(10000);
          if (diff.isPositive() && diff <= MyMoneyMoney(KMyMoneySettings::autoFillDifference(), 100)) {
            uniqList[ukey].t = &((*it_t).first);
            break;    // end while loop
          }
        } else if (KMyMoneySettings::autoFillTransaction() == 2) {
          uniqList[ukey].t = &((*it_t).first);
          (*it_u).cnt++;
          break;      // end while loop
        }
        ++cnt;
      } while (it_u != uniqList.end());

    }

    MyMoneyTransaction t;
    if (KMyMoneySettings::autoFillTransaction() != 2) {
#if 0
      // I removed this code to allow cancellation of an autofill if
      // it does not match even if there is only a single matching
      // transaction for the payee in question. In case, we want to revert
      // to the old behavior, don't forget to uncomment the closing
      // brace further down in the code as well. (ipwizard 2009-01-16)
      if (uniqList.count() == 1) {
        t = list.last().first;
      } else {
#endif
        QPointer<KSelectTransactionsDlg> dlg = new KSelectTransactionsDlg(d->m_account, d->m_regForm);
        dlg->setWindowTitle(i18n("Select autofill transaction"));

        QMap<QString, struct uniqTransaction>::const_iterator it_u;
        for (it_u = uniqList.constBegin(); it_u != uniqList.constEnd(); ++it_u) {
          dlg->addTransaction(*(*it_u).t);
        }

        auto tRegister = dlg->getRegister();
        // setup sort order
        tRegister->setSortOrder("1,-9,-4");
        // sort the transactions according to the sort setting
        tRegister->sortItems();

        // and select the last item
        if (tRegister->lastItem())
          tRegister->selectItem(tRegister->lastItem());

        if (dlg->exec() == QDialog::Accepted) {
          t = dlg->transaction();
        }
#if 0
      }
#endif
    } else {
      int maxCnt = 0;
      QMap<QString, struct uniqTransaction>::const_iterator it_u;
      for (it_u = uniqList.constBegin(); it_u != uniqList.constEnd(); ++it_u) {
        if ((*it_u).cnt > maxCnt) {
          t = *(*it_u).t;
          maxCnt = (*it_u).cnt;
        }
      }
    }

    if (t != MyMoneyTransaction()) {
      d->m_transaction.removeSplits();
      d->m_split = MyMoneySplit();
      MyMoneySplit otherSplit;
      foreach (const auto split, t.splits()) {
        MyMoneySplit s(split);
        s.setReconcileFlag(eMyMoney::Split::State::NotReconciled);
        s.setReconcileDate(QDate());
        s.clearId();
        s.setBankID(QString());
        // older versions of KMyMoney used to set the action
        // we don't need this anymore
        if (s.action() != MyMoneySplit::actionName(eMyMoney::Split::Action::Amortization)
            && s.action() != MyMoneySplit::actionName(eMyMoney::Split::Action::Interest))  {
          s.setAction(QString());
        }

        // FIXME update check number. The old comment contained
        //
        // <quote>
        // If a check number is already specified by the user it is
        // used. If the input field is empty and the previous transaction
        // contains a checknumber, the next usable check number will be assigned
        // to the transaction.
        // </quote>

        auto editNr = dynamic_cast<KMyMoneyLineEdit*>(haveWidget("number"));
        if (editNr && !editNr->text().isEmpty()) {
          s.setNumber(editNr->text());
        } else if (!s.number().isEmpty()) {
          s.setNumber(KMyMoneyUtils::nextFreeCheckNumber(d->m_account));
        }

        // if the memos should not be used with autofill or
        // if the transaction has exactly two splits, remove
        // the memo text of the split that does not reference
        // the current account. This allows the user to change
        // the autofilled memo text which will then also be used
        // in this split. See createTransaction() for this logic.
        if ((s.accountId() != d->m_account.id() && t.splitCount() == 2) || !KMyMoneySettings::autoFillUseMemos())
          s.setMemo(QString());

        d->m_transaction.addSplit(s);
        if (s.accountId() == d->m_account.id() && d->m_split == MyMoneySplit()) {
          d->m_split = s;
        } else {
          otherSplit = s;
        }
      }

      // make sure to extract the right action
      eRegister::Action action;
      action = d->m_split.shares().isNegative() ? eRegister::Action::Withdrawal : eRegister::Action::Deposit;

      if (d->m_transaction.splitCount() == 2) {
        auto acc = MyMoneyFile::instance()->account(otherSplit.accountId());
        if (acc.isAssetLiability())
          action = eRegister::Action::Transfer;
      }

      // now setup the widgets with the new data but keep the date
      if (auto postdateWidget = dynamic_cast<KMyMoneyDateInput*>(d->m_editWidgets["postdate"])) {
        auto date = postdateWidget->date();
        loadEditWidgets(action);
        postdateWidget->setDate(date);
      }
    }
  }

  // focus jumps into the category field
  QWidget* w;
  if ((w = haveWidget("payee")) != 0) {
    w->setFocus();
  }
}

void StdTransactionEditor::slotUpdateAction(int action)
{
  Q_D(StdTransactionEditor);
  auto tabbar = dynamic_cast<KMyMoneyTransactionForm::TabBar*>(haveWidget("tabbar"));
  if (tabbar) {
    auto categoryLabel = dynamic_cast<QLabel*>(haveWidget("category-label"));
    auto cashflow = dynamic_cast<KMyMoneyCashFlowCombo*>(d->m_editWidgets["cashflow"]);
    if (!categoryLabel || !cashflow)
      return;
    switch (action) {
      case (int)eRegister::Action::Deposit:
        categoryLabel->setText(i18n("Category"));
        cashflow->setDirection(eRegister::CashFlowDirection::Deposit);
        break;
      case (int)eRegister::Action::Transfer:
        if (d->m_split.shares().isNegative()) {
          cashflow->setDirection(eRegister::CashFlowDirection::Payment);
          categoryLabel->setText(i18n("Transfer to"));
        } else {
          cashflow->setDirection(eRegister::CashFlowDirection::Deposit);
          categoryLabel->setText(i18n("Transfer from"));
        }
        tabbar->setCurrentIndex((int)eRegister::Action::Transfer);
        slotUpdateCashFlow(cashflow->direction());
        break;
      case (int)eRegister::Action::Withdrawal:
        categoryLabel->setText(i18n("Category"));
        cashflow->setDirection(eRegister::CashFlowDirection::Payment);
        break;
    }
    resizeForm();
  }
}

void StdTransactionEditor::slotUpdateCashFlow(eRegister::CashFlowDirection dir)
{
  auto categoryLabel = dynamic_cast<QLabel*>(haveWidget("category-label"));
  if (auto cashflow = dynamic_cast<KMyMoneyCashFlowCombo*>(haveWidget("cashflow")))
    cashflow->setDirection(dir);
  // qDebug("Update cashflow to %d", dir);
  if (categoryLabel) {
    auto tabbar = dynamic_cast<KMyMoneyTransactionForm::TabBar*>(haveWidget("tabbar"));
    if (!tabbar) return;  //  no transaction form
    if (categoryLabel->text() != i18n("Category")) {
      tabbar->setCurrentIndex((int)eRegister::Action::Transfer);
      if (dir == eRegister::CashFlowDirection::Deposit) {
        categoryLabel->setText(i18n("Transfer from"));
      } else {
        categoryLabel->setText(i18n("Transfer to"));
      }
      resizeForm();
    } else {
      if (dir == eRegister::CashFlowDirection::Deposit)
        tabbar->setCurrentIndex((int)eRegister::Action::Deposit);
      else
        tabbar->setCurrentIndex((int)eRegister::Action::Withdrawal);
    }
  }
}

void StdTransactionEditor::slotUpdateCategory(const QString& id)
{
  Q_D(StdTransactionEditor);
  auto categoryLabel = dynamic_cast<QLabel*>(haveWidget("category-label"));
  // qDebug("Update category to %s", qPrintable(id));

  if (categoryLabel) {
    auto tabbar = dynamic_cast<KMyMoneyTransactionForm::TabBar*>(haveWidget("tabbar"));
    auto amount = dynamic_cast<AmountEdit*>(d->m_editWidgets["amount"]);
    auto val = amount ? amount->value() : MyMoneyMoney();

    if (categoryLabel->text() == i18n("Transfer from")) {
      val = -val;
    } else {
      val = val.abs();
    }

    if (tabbar) {
      tabbar->setTabEnabled((int)eRegister::Action::Transfer, true);
      tabbar->setTabEnabled((int)eRegister::Action::Deposit, true);
      tabbar->setTabEnabled((int)eRegister::Action::Withdrawal, true);
    }

    bool disableTransferTab = false;
    if (!id.isEmpty()) {
      auto acc = MyMoneyFile::instance()->account(id);
      if (acc.isAssetLiability()
          || acc.accountGroup() == eMyMoney::Account::Type::Equity) {
        if (tabbar) {
          tabbar->setCurrentIndex((int)eRegister::Action::Transfer);
          tabbar->setTabEnabled((int)eRegister::Action::Deposit, false);
          tabbar->setTabEnabled((int)eRegister::Action::Withdrawal, false);
        }
        auto cashflow = dynamic_cast<KMyMoneyCashFlowCombo*>(d->m_editWidgets["cashflow"]);
        if (val.isZero()) {
          if (cashflow && (cashflow->direction() == eRegister::CashFlowDirection::Deposit)) {
            categoryLabel->setText(i18n("Transfer from"));
          } else {
            categoryLabel->setText(i18n("Transfer to"));
          }
        } else if (val.isNegative()) {
          categoryLabel->setText(i18n("Transfer from"));
          if (cashflow)
            cashflow->setDirection(eRegister::CashFlowDirection::Deposit);
        } else
          categoryLabel->setText(i18n("Transfer to"));
      } else {
        disableTransferTab = true;
        categoryLabel->setText(i18n("Category"));
      }
      updateAmount(val);
    } else {  //id.isEmpty()
      if (auto category = dynamic_cast<KMyMoneyCategory*>(d->m_editWidgets["category"]))
        disableTransferTab = !category->currentText().isEmpty();
      categoryLabel->setText(i18n("Category"));
    }
    if (tabbar) {
      if (disableTransferTab) {
        // set the proper tab before disabling the currently active tab
        if (tabbar->currentIndex() == (int)eRegister::Action::Transfer) {
          tabbar->setCurrentIndex(val.isPositive() ? (int)eRegister::Action::Withdrawal : (int)eRegister::Action::Deposit);
        }
        tabbar->setTabEnabled((int)eRegister::Action::Transfer, false);
      }
      tabbar->update();
    }

    resizeForm();
  }
  updateVAT(false);
}

void StdTransactionEditor::slotUpdatePayment(const QString& txt)
{
  Q_D(StdTransactionEditor);
  MyMoneyMoney val(txt);

  auto depositWidget = dynamic_cast<AmountEdit*>(d->m_editWidgets["deposit"]);
  auto paymentWidget = dynamic_cast<AmountEdit*>(d->m_editWidgets["payment"]);
  if (!depositWidget || !paymentWidget)
    return;

  if (val.isNegative()) {
    depositWidget->setValue(val.abs());
    paymentWidget->setText(QString());
  } else {
    depositWidget->setText(QString());
  }
  updateVAT();
}

void StdTransactionEditor::slotUpdateDeposit(const QString& txt)
{
  Q_D(StdTransactionEditor);
  MyMoneyMoney val(txt);

  auto depositWidget = dynamic_cast<AmountEdit*>(d->m_editWidgets["deposit"]);
  auto paymentWidget = dynamic_cast<AmountEdit*>(d->m_editWidgets["payment"]);
  if (!depositWidget || !paymentWidget)
    return;

  if (val.isNegative()) {
    paymentWidget->setValue(val.abs());
    depositWidget->setText(QString());
  } else {
    paymentWidget->setText(QString());
  }
  updateVAT();
}

void StdTransactionEditor::slotUpdateAmount(const QString& txt)
{
  // qDebug("Update amount to %s", qPrintable(txt));
  MyMoneyMoney val(txt);
  updateAmount(val);
  updateVAT(true);
}

void StdTransactionEditor::updateAmount(const MyMoneyMoney& val)
{
  // we don't do anything if we have multiple transactions selected
  if (isMultiSelection())
    return;

  Q_D(StdTransactionEditor);
  auto categoryLabel = dynamic_cast<QLabel*>(haveWidget("category-label"));
  if (categoryLabel) {
    if (auto cashflow = dynamic_cast<KMyMoneyCashFlowCombo*>(d->m_editWidgets["cashflow"])) {
      if (!val.isPositive())  {  //   fixes BUG321317
        if (categoryLabel->text() != i18n("Category")) {
          if (cashflow->direction() == eRegister::CashFlowDirection::Payment) {
            categoryLabel->setText(i18n("Transfer to"));
          }
        } else {
          slotUpdateCashFlow(cashflow->direction());
        }
        if (auto amountWidget = dynamic_cast<AmountEdit*>(d->m_editWidgets["amount"]))
          amountWidget->setValue(val.abs());
      } else {
        if (categoryLabel->text() != i18n("Category")) {
          if (cashflow->direction() == eRegister::CashFlowDirection::Payment) {
            categoryLabel->setText(i18n("Transfer to"));
          } else {
            categoryLabel->setText(i18n("Transfer from"));
            cashflow->setDirection(eRegister::CashFlowDirection::Deposit);  //  editing with +ve shows 'from' not 'pay to'
          }
        }
        if (auto amountWidget = dynamic_cast<AmountEdit*>(d->m_editWidgets["amount"]))
          amountWidget->setValue(val.abs());
      }
    }
  }
}

void StdTransactionEditor::updateVAT(bool amountChanged)
{
  Q_D(StdTransactionEditor);
  // make sure that we don't do this recursively
  if (d->m_inUpdateVat)
    return;

  // we don't do anything if we have multiple transactions selected
  if (isMultiSelection())
    return;

  // if auto vat assignment for this account is turned off
  // we don't care about taxes
  if (d->m_account.value("NoVat") == "Yes")
    return;

  // more splits than category and tax are not supported
  if (d->m_splits.count() > 2)
    return;

  // in order to do anything, we need an amount
  MyMoneyMoney amount, newAmount;
  bool amountOk;
  amount = amountFromWidget(&amountOk);
  if (!amountOk)
    return;

  // If the transaction has a tax and a category split, remove the tax split
  if (d->m_splits.count() == 2) {
    newAmount = removeVatSplit();
    if (d->m_splits.count() == 2) // not removed?
      return;

  } else if (auto category = dynamic_cast<KMyMoneyCategory*>(d->m_editWidgets["category"])) {
    // otherwise, we need a category
    if (category->selectedItem().isEmpty())
      return;

    // if no VAT account is associated with this category/account, then we bail out
    MyMoneyAccount cat = MyMoneyFile::instance()->account(category->selectedItem());
    if (cat.value("VatAccount").isEmpty())
      return;

    newAmount = amount;
  }

  // seems we have everything we need
  if (amountChanged)
    newAmount = amount;

  MyMoneyTransaction transaction;
  if (createTransaction(transaction, d->m_transaction, d->m_split)) {
    if (addVatSplit(transaction, newAmount)) {
      d->m_transaction = transaction;
      if (!d->m_transaction.splits().isEmpty())
        d->m_split = d->m_transaction.splits().front();

      loadEditWidgets();

      // if we made this a split transaction, then move the
      // focus to the memo field
      if (qApp->focusWidget() == haveWidget("category")) {
        QWidget* w = haveWidget("memo");
        if (w)
          w->setFocus();
      }
    }
  }
}

bool StdTransactionEditor::addVatSplit(MyMoneyTransaction& tr, const MyMoneyMoney& amount)
{
  if (tr.splitCount() != 2)
    return false;

  Q_D(StdTransactionEditor);
  auto file = MyMoneyFile::instance();
  // extract the category split from the transaction
  MyMoneyAccount category = file->account(tr.splitByAccount(d->m_account.id(), false).accountId());
  return file->addVATSplit(tr, d->m_account, category, amount);
}

MyMoneyMoney StdTransactionEditor::removeVatSplit()
{
  Q_D(StdTransactionEditor);
  // we only deal with splits that have three splits
  if (d->m_splits.count() != 2)
    return amountFromWidget();

  MyMoneySplit c; // category split
  MyMoneySplit t; // tax split

  auto netValue = false;
  foreach (const auto split , d->m_splits) {
    auto acc = MyMoneyFile::instance()->account(split.accountId());
    if (!acc.value("VatAccount").isEmpty()) {
      netValue = (acc.value("VatAmount").toLower() == "net");
      c = split;
    } else if (!acc.value("VatRate").isEmpty()) {
      t = split;
    }
  }

  // bail out if not all splits are setup
  if (c.id().isEmpty() || t.id().isEmpty())
    return amountFromWidget();

  MyMoneyMoney amount;
  // reduce the splits
  if (netValue) {
    amount = -c.shares();
  } else {
    amount = -(c.shares() + t.shares());
  }

  // remove tax split from the list, ...
  d->m_splits.clear();
  d->m_splits.append(c);

  // ... make sure that the widget is updated ...
  // block the signals to avoid popping up the split editor dialog
  // for nothing
  d->m_editWidgets["category"]->blockSignals(true);
  QString id;
  setupCategoryWidget(id);
  d->m_editWidgets["category"]->blockSignals(false);

  // ... and return the updated amount
  return amount;
}

bool StdTransactionEditor::isComplete(QString& reason) const
{
  Q_D(const StdTransactionEditor);
  reason.clear();
  QMap<QString, QWidget*>::const_iterator it_w;

  auto postDate = dynamic_cast<KMyMoneyDateInput*>(d->m_editWidgets["postdate"]);
  if (postDate) {
    QDate accountOpeningDate = d->m_account.openingDate();
    for (QList<MyMoneySplit>::const_iterator it_s = d->m_splits.constBegin(); it_s != d->m_splits.constEnd(); ++it_s) {
      const MyMoneyAccount& acc = MyMoneyFile::instance()->account((*it_s).accountId());
      // compute the newest opening date of all accounts involved in the transaction
      if (acc.openingDate() > accountOpeningDate)
        accountOpeningDate = acc.openingDate();
    }
    // check the selected category in case m_splits hasn't been updated yet
    auto category = dynamic_cast<KMyMoneyCategory*>(d->m_editWidgets["category"]);
    if (category && !category->selectedItem().isEmpty()) {
      MyMoneyAccount cat = MyMoneyFile::instance()->account(category->selectedItem());
      if (cat.openingDate() > accountOpeningDate)
        accountOpeningDate = cat.openingDate();
    }

    if (postDate->date().isValid() && (postDate->date() < accountOpeningDate)) {
      postDate->markAsBadDate(true, KMyMoneySettings::schemeColor(SchemeColor::Negative));
      reason = i18n("Cannot enter transaction with postdate prior to account's opening date.");
      postDate->setToolTip(reason);
      return false;
    }
    postDate->markAsBadDate();
    postDate->setToolTip(QString());
  }

  for (it_w = d->m_editWidgets.begin(); it_w != d->m_editWidgets.end(); ++it_w) {
    auto payee = dynamic_cast<KMyMoneyPayeeCombo*>(*it_w);
    auto tagContainer = dynamic_cast<KTagContainer*>(*it_w);
    auto category = dynamic_cast<KMyMoneyCategory*>(*it_w);
    auto amount = dynamic_cast<AmountEdit*>(*it_w);
    auto reconcile = dynamic_cast<KMyMoneyReconcileCombo*>(*it_w);
    auto cashflow = dynamic_cast<KMyMoneyCashFlowCombo*>(*it_w);
    auto memo = dynamic_cast<KTextEdit*>(*it_w);

    if (payee && !(payee->currentText().isEmpty()))
      break;

    if (category && !category->lineEdit()->text().isEmpty())
      break;

    if (amount && !(amount->value().isZero()))
      break;

    // the following widgets are only checked if we are editing multiple transactions
    if (isMultiSelection()) {
      if (auto tabbar = dynamic_cast<KMyMoneyTransactionForm::TabBar*>(haveWidget("tabbar")))
        tabbar->setEnabled(true);

      if (reconcile && reconcile->state() != eMyMoney::Split::State::Unknown)
        break;

      if (cashflow && cashflow->direction() != eRegister::CashFlowDirection::Unknown)
        break;

      if (postDate && postDate->date().isValid() && (postDate->date() >= d->m_account.openingDate()))
        break;

      if (memo && d->m_memoChanged)
        break;

      if (tagContainer && !(tagContainer->selectedTags().isEmpty()))  //  Tag is optional field
        break;
    }
  }
  return it_w != d->m_editWidgets.end();
}

void StdTransactionEditor::slotCreateCategory(const QString& name, QString& id)
{
  Q_D(StdTransactionEditor);
  MyMoneyAccount acc, parent;
  acc.setName(name);

  auto cashflow = dynamic_cast<KMyMoneyCashFlowCombo*>(haveWidget("cashflow"));
  if (cashflow) {
    // form based input
    if (cashflow->direction() == eRegister::CashFlowDirection::Deposit)
      parent = MyMoneyFile::instance()->income();
    else
      parent = MyMoneyFile::instance()->expense();

  } else if (haveWidget("deposit")) {
    // register based input
    if (auto deposit = dynamic_cast<AmountEdit*>(d->m_editWidgets["deposit"])) {
      if (deposit->value().isPositive())
        parent = MyMoneyFile::instance()->income();
      else
        parent = MyMoneyFile::instance()->expense();
    }

  } else
    parent = MyMoneyFile::instance()->expense();

  // TODO extract possible first part of a hierarchy and check if it is one
  // of our top categories. If so, remove it and select the parent
  // according to this information.

  slotNewCategory(acc, parent);

  // return id
  id = acc.id();
}

int StdTransactionEditor::slotEditSplits()
{
  Q_D(StdTransactionEditor);
  int rc = QDialog::Rejected;

  if (!d->m_openEditSplits) {
    // only get in here in a single instance
    d->m_openEditSplits = true;

    // force focus change to update all data
    auto categoryWidget = dynamic_cast<KMyMoneyCategory*>(d->m_editWidgets["category"]);
    QWidget* w = categoryWidget ? categoryWidget->splitButton() : nullptr;
    if (w)
      w->setFocus();

    auto amount = dynamic_cast<AmountEdit*>(haveWidget("amount"));
    auto deposit = dynamic_cast<AmountEdit*>(haveWidget("deposit"));
    auto payment = dynamic_cast<AmountEdit*>(haveWidget("payment"));
    KMyMoneyCashFlowCombo* cashflow = 0;
    eRegister::CashFlowDirection dir = eRegister::CashFlowDirection::Unknown;
    bool isValidAmount = false;

    if (amount) {
      isValidAmount = amount->text().length() != 0;
      if ((cashflow = dynamic_cast<KMyMoneyCashFlowCombo*>(haveWidget("cashflow"))))
        dir = cashflow->direction();

    } else {
      if (deposit) {
        if (deposit->text().length() != 0) {
          isValidAmount = true;
          dir = eRegister::CashFlowDirection::Deposit;
        }
      }
      if (payment) {
        if (payment->text().length() != 0) {
          isValidAmount = true;
          dir = eRegister::CashFlowDirection::Payment;
        }
      }
      if (!deposit || !payment) {
        qDebug("Internal error: deposit(%p) & payment(%p) widgets not found but required", deposit, payment);
        return rc;
      }
    }

    if (dir == eRegister::CashFlowDirection::Unknown)
      dir = eRegister::CashFlowDirection::Payment;

    MyMoneyTransaction transaction;
    if (createTransaction(transaction, d->m_transaction, d->m_split)) {
      MyMoneyMoney value;

      QPointer<KSplitTransactionDlg> dlg =
        new KSplitTransactionDlg(transaction,
                                 transaction.splits().isEmpty() ? MyMoneySplit() : transaction.splits().front(),
                                 d->m_account,
                                 isValidAmount,
                                 dir == eRegister::CashFlowDirection::Deposit,
                                 MyMoneyMoney(),
                                 d->m_priceInfo,
                                 d->m_regForm);
      connect(dlg.data(), &KSplitTransactionDlg::objectCreation, this, &StdTransactionEditor::objectCreation);
      connect(dlg.data(), &KSplitTransactionDlg::createCategory, this, &StdTransactionEditor::slotNewCategory);

      if ((rc = dlg->exec()) == QDialog::Accepted) {
        d->m_transaction = dlg->transaction();
        if (!d->m_transaction.splits().isEmpty()) {
          d->m_split = d->m_transaction.splits().front();
          // if we have only two splits left, we copy the memo
          // of the second (data from the split editor) to the
          // first (data used in the transaction editor)
          if (d->m_transaction.splitCount() == 2) {
            d->m_split.setMemo(d->m_transaction.splits().last().memo());
            d->m_transaction.modifySplit(d->m_split);
          }
        }
        loadEditWidgets();
      }

      delete dlg;
    }

    // focus jumps into the tag field
    if ((w = haveWidget("tag")) != 0) {
      w->setFocus();
    }

    d->m_openEditSplits = false;
  }

  return rc;
}

void StdTransactionEditor::checkPayeeInSplit(MyMoneySplit& s, const QString& payeeId)
{
  if (s.accountId().isEmpty())
    return;

  auto acc = MyMoneyFile::instance()->account(s.accountId());
  if (acc.isIncomeExpense()) {
    s.setPayeeId(payeeId);
  } else {
    if (s.payeeId().isEmpty())
      s.setPayeeId(payeeId);
  }
}

MyMoneyMoney StdTransactionEditor::amountFromWidget(bool* update) const
{
  Q_D(const StdTransactionEditor);
  bool updateValue = false;
  MyMoneyMoney value;

  auto cashflow = dynamic_cast<KMyMoneyCashFlowCombo*>(haveWidget("cashflow"));
  if (cashflow) {
    // form based input
    if (auto amount = dynamic_cast<AmountEdit*>(d->m_editWidgets["amount"])) {
      // if both fields do not contain changes -> no need to update
      if (cashflow->direction() != eRegister::CashFlowDirection::Unknown
          && !amount->text().isEmpty())
        updateValue = true;
      value = amount->value();
      if (cashflow->direction() == eRegister::CashFlowDirection::Payment)
        value = -value;
    }

  } else if (haveWidget("deposit")) {
    // register based input
    auto deposit = dynamic_cast<AmountEdit*>(d->m_editWidgets["deposit"]);
    auto payment = dynamic_cast<AmountEdit*>(d->m_editWidgets["payment"]);
    if (deposit && payment) {
      // if both fields do not contain text -> no need to update
      if (!(deposit->text().isEmpty() && payment->text().isEmpty()))
        updateValue = true;

      if (deposit->value().isPositive())
        value = deposit->value();
      else
        value = -(payment->value());
    }
  }

  if (update)
    *update = updateValue;

  // determine the max fraction for this account and
  // adjust the value accordingly
  return value.convert(d->m_account.fraction());
}

bool StdTransactionEditor::createTransaction(MyMoneyTransaction& t, const MyMoneyTransaction& torig, const MyMoneySplit& sorig, bool skipPriceDialog)
{
  Q_D(StdTransactionEditor);
  // extract price info from original transaction
  d->m_priceInfo.clear();
  if (!torig.id().isEmpty()) {
    foreach (const auto split, torig.splits()) {
      if (split.id() != sorig.id()) {
        MyMoneyAccount cat = MyMoneyFile::instance()->account(split.accountId());
        if (cat.currencyId() != d->m_account.currencyId()) {
          if (!split.shares().isZero() && !split.value().isZero()) {
            d->m_priceInfo[cat.currencyId()] = (split.shares() / split.value()).reduce();
          }
        }
      }
    }
  }

  t = torig;

  t.removeSplits();
  t.setCommodity(d->m_account.currencyId());

  auto postDate = dynamic_cast<KMyMoneyDateInput*>(d->m_editWidgets["postdate"]);
  if (postDate && postDate->date().isValid()) {
    t.setPostDate(postDate->date());
  }

  // we start with the previous values, make sure we can add them later on
  MyMoneySplit s0 = sorig;
  s0.clearId();

  // make sure we reference this account here
  s0.setAccountId(d->m_account.id());

  // memo and number field are special: if we have multiple transactions selected
  // and the edit field is empty, we treat it as "not modified".
  // FIXME a better approach would be to have a 'dirty' flag with the widgets
  //       which identifies if the originally loaded value has been modified
  //       by the user
  auto memo = dynamic_cast<KTextEdit*>(d->m_editWidgets["memo"]);
  if (memo) {
    if (!isMultiSelection() || d->m_memoChanged)
      s0.setMemo(memo->toPlainText());
  }

  if (auto number = dynamic_cast<KMyMoneyLineEdit*>(haveWidget("number"))) {
    if (!isMultiSelection() || !number->text().isEmpty())
      s0.setNumber(number->text());
  }

  auto payee = dynamic_cast<KMyMoneyPayeeCombo*>(d->m_editWidgets["payee"]);
  QString payeeId;
  if (payee && (!isMultiSelection() || !payee->currentText().isEmpty())) {
    payeeId = payee->selectedItem();
    s0.setPayeeId(payeeId);
  }

  //KMyMoneyTagCombo* tag = dynamic_cast<KMyMoneyTagCombo*>(m_editWidgets["tag"]);
  auto tag = dynamic_cast<KTagContainer*>(d->m_editWidgets["tag"]);
  if (tag && (!isMultiSelection() || !tag->selectedTags().isEmpty())) {
    s0.setTagIdList(tag->selectedTags());
  }

  bool updateValue;
  MyMoneyMoney value = amountFromWidget(&updateValue);

  if (updateValue) {
    // for this account, the shares and value is the same
    s0.setValue(value);
    s0.setShares(value);
  } else {
    value = s0.value();
  }

  // if we mark the split reconciled here, we'll use today's date if no reconciliation date is given
  auto status = dynamic_cast<KMyMoneyReconcileCombo*>(d->m_editWidgets["status"]);
  if (status && status->state() != eMyMoney::Split::State::Unknown)
    s0.setReconcileFlag(status->state());

  if (s0.reconcileFlag() == eMyMoney::Split::State::Reconciled && !s0.reconcileDate().isValid())
    s0.setReconcileDate(QDate::currentDate());

  checkPayeeInSplit(s0, payeeId);

  // add the split to the transaction
  t.addSplit(s0);

  // if we have no other split we create it
  // if we have none or only one other split, we reconstruct it here
  // if we have more than one other split, we take them as they are
  // make sure to perform all those changes on a local copy
  QList<MyMoneySplit> splits = d->m_splits;

  MyMoneySplit s1;
  if (splits.isEmpty()) {
    s1.setMemo(s0.memo());
    splits.append(s1);

    // make sure we will fill the value and share fields later on
    updateValue = true;
  }

  // FIXME in multiSelection we currently only support transactions with one
  // or two splits. So we check the original transaction and extract the other
  // split or create it
  if (isMultiSelection()) {
    if (torig.splitCount() == 2) {
      foreach (const auto split, torig.splits()) {
        if (split.id() == sorig.id())
          continue;
        s1 = split;
        s1.clearId();
        break;
      }
    }
  } else {
    if (splits.count() == 1) {
      s1 = splits[0];
      s1.clearId();
    }
  }

  if (isMultiSelection() || splits.count() == 1) {
    auto category = dynamic_cast<KMyMoneyCategory*>(d->m_editWidgets["category"]);
    if (category && (!isMultiSelection() || !category->currentText().isEmpty())) {
      s1.setAccountId(category->selectedItem());
    }

    // if the first split has a memo but the second split is empty,
    // we just copy the memo text over
    if (memo) {
      if (!isMultiSelection() || !memo->toPlainText().isEmpty()) {
        // if the memo is filled, we check if the
        // account referenced by s1 is a regular account or a category.
        // in case of a regular account, we just leave the memo as is
        // in case of a category we simply copy the new value over the old.
        // in case we don't even have an account id, we just skip because
        // the split will be removed later on anyway.
        if (!s1.memo().isEmpty() && s1.memo() != s0.memo()) {
          if (!s1.accountId().isEmpty()) {
            auto acc = MyMoneyFile::instance()->account(s1.accountId());
            if (acc.isIncomeExpense())
              s1.setMemo(s0.memo());
            else if (KMessageBox::questionYesNo(d->m_regForm,
                                                i18n("Do you want to replace memo<p><i>%1</i></p>with memo<p><i>%2</i></p>in the other split?", s1.memo(), s0.memo()), i18n("Copy memo"),
                                                KStandardGuiItem::yes(), KStandardGuiItem::no(),
                                                QStringLiteral("CopyMemoOver")) == KMessageBox::Yes)
              s1.setMemo(s0.memo());
          }
        } else {
          s1.setMemo(s0.memo());
        }
      }
    }

    if (updateValue && !s1.accountId().isEmpty()) {
      s1.setValue(-value);
      MyMoneyMoney shares;
      if (!skipPriceDialog) {
        if (!KCurrencyCalculator::setupSplitPrice(shares, t, s1, d->m_priceInfo, d->m_regForm))
          return false;
      } else {
        MyMoneyAccount cat = MyMoneyFile::instance()->account(s1.accountId());
        if (d->m_priceInfo.find(cat.currencyId()) != d->m_priceInfo.end()) {
          shares = (s1.value() * d->m_priceInfo[cat.currencyId()]).reduce().convert(cat.fraction());
        } else
          shares = s1.value();
      }
      s1.setShares(shares);
    }

    checkPayeeInSplit(s1, payeeId);

    if (!s1.accountId().isEmpty())
      t.addSplit(s1);

    // check if we need to add/update a VAT assignment
    MyMoneyFile::instance()->updateVAT(t);

  } else {
    foreach (const auto split, splits) {
      s1 = split;
      s1.clearId();
      checkPayeeInSplit(s1, payeeId);
      t.addSplit(s1);
    }
  }
  return true;
}

void StdTransactionEditor::setupFinalWidgets()
{
  addFinalWidget(haveWidget("deposit"));
  addFinalWidget(haveWidget("payment"));
  addFinalWidget(haveWidget("amount"));
  addFinalWidget(haveWidget("status"));
}

void StdTransactionEditor::slotUpdateAccount(const QString& id)
{
  Q_D(StdTransactionEditor);
  TransactionEditor::slotUpdateAccount(id);
  auto category = dynamic_cast<KMyMoneyCategory*>(d->m_editWidgets["category"]);
  if (category && category->splitButton()) {
    category->splitButton()->setDisabled(id.isEmpty());
  }
}
