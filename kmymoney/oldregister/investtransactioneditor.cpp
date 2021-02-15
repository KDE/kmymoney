/*
    SPDX-FileCopyrightText: 2007-2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "investtransactioneditor.h"
#include "transactioneditor_p.h"

#include <typeinfo>

// ----------------------------------------------------------------------------
// QT Includes

#include <QLabel>
#include <QList>
#include <QPushButton>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KTextEdit>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyreconcilecombo.h"
#include "kmymoneyactivitycombo.h"
#include "ktagcontainer.h"
#include "investtransaction.h"
#include "selectedtransactions.h"
#include "transactioneditorcontainer.h"
#include "kmymoneycategory.h"
#include "kmymoneydateinput.h"
#include "amountedit.h"
#include "kmymoneyaccountselector.h"
#include "kmymoneymvccombo.h"
#include "mymoneyfile.h"
#include "mymoneyexception.h"
#include "mymoneysecurity.h"
#include "mymoneyprice.h"
#include "ksplittransactiondlg.h"
#include "kcurrencycalculator.h"
#include "kmymoneysettings.h"
#include "investactivities.h"
#include "kmymoneycompletion.h"
#include "dialogenums.h"
#include "mymoneyutils.h"

using namespace eMyMoney;
using namespace KMyMoneyRegister;
using namespace KMyMoneyTransactionForm;
using namespace Invest;

class OldInvestTransactionEditorPrivate : public TransactionEditorPrivate
{
  Q_DISABLE_COPY(OldInvestTransactionEditorPrivate)
  Q_DECLARE_PUBLIC(OldInvestTransactionEditor)
  friend class Invest::OldActivity;

public:
  explicit  OldInvestTransactionEditorPrivate(OldInvestTransactionEditor* qq) :
      TransactionEditorPrivate(qq),
      m_activity(nullptr),
      m_phonyAccount(MyMoneyAccount("Phony-ID", MyMoneyAccount())),
      m_transactionType(eMyMoney::Split::InvestmentTransactionType::BuyShares)
  {
  }

  ~OldInvestTransactionEditorPrivate()
  {
    delete m_activity;
  }

  void showCategory(const QString& name, bool visible = true)
  {
    Q_Q(OldInvestTransactionEditor);
    if (auto cat = dynamic_cast<KMyMoneyCategory*>(q->haveWidget(name))) {
      if (Q_LIKELY(cat->splitButton())) {
        cat->parentWidget()->setVisible(visible);  // show or hide the enclosing QFrame;
      } else {
        cat->setVisible(visible);  // show or hide the enclosing QFrame;
      }
    }
  }

  void activityFactory(eMyMoney::Split::InvestmentTransactionType type)
  {
    Q_Q(OldInvestTransactionEditor);
    if (!m_activity || type != m_activity->type()) {
      delete m_activity;
      switch (type) {
        default:
        case eMyMoney::Split::InvestmentTransactionType::BuyShares:
          m_activity = new Buy(q);
          break;
        case eMyMoney::Split::InvestmentTransactionType::SellShares:
          m_activity = new Sell(q);
          break;
        case eMyMoney::Split::InvestmentTransactionType::Dividend:
        case eMyMoney::Split::InvestmentTransactionType::Yield:
          m_activity = new Div(q);
          break;
        case eMyMoney::Split::InvestmentTransactionType::ReinvestDividend:
          m_activity = new Reinvest(q);
          break;
        case eMyMoney::Split::InvestmentTransactionType::AddShares:
          m_activity = new Add(q);
          break;
        case eMyMoney::Split::InvestmentTransactionType::RemoveShares:
          m_activity = new Remove(q);
          break;
        case eMyMoney::Split::InvestmentTransactionType::SplitShares:
          m_activity = new Invest::Split(q);
          break;
        case eMyMoney::Split::InvestmentTransactionType::InterestIncome:
          m_activity = new IntInc(q);
          break;
      }
    }
  }

  MyMoneyMoney subtotal(const QList<MyMoneySplit>& splits) const
  {
    MyMoneyMoney sum;

    foreach (const auto split, splits)
      sum += split.value();

    return sum;
  }

  /**
   * This method creates a transaction to be used for the split fee/interest editor.
   * It has a reference to a phony account and the splits contained in @a splits .
   */
  bool createPseudoTransaction(MyMoneyTransaction& t, const QList<MyMoneySplit>& splits)
  {
    t.removeSplits();

    MyMoneySplit split;
    split.setAccountId(m_phonyAccount.id());
    split.setValue(-subtotal(splits));
    split.setShares(split.value());
    t.addSplit(split);
    m_phonySplit = split;

    foreach (const auto it_s, splits) {
      split = it_s;
      split.clearId();
      t.addSplit(split);
    }
    return true;
  }

  /**
   * Convenience method used by slotEditInterestSplits() and slotEditFeeSplits().
   *
   * @param categoryWidgetName name of the category widget
   * @param amountWidgetName name of the amount widget
   * @param splits the splits that make up the transaction to be edited
   * @param isIncome @c false for fees, @c true for interest
   * @param slotEditSplits name of the slot to be connected to the focusIn signal of the
   *                       category widget named @p categoryWidgetName in case of multiple splits
   *                       in @p splits .
   */

  int editSplits(const QString& categoryWidgetName,
                 const QString& amountWidgetName,
                 QList<MyMoneySplit>& splits,
                 bool isIncome,
                 const char* slotEditSplits)
  {
    Q_Q(OldInvestTransactionEditor);
    int rc = QDialog::Rejected;

    if (!m_openEditSplits) {
      // only get in here in a single instance
      m_openEditSplits = true;

      // force focus change to update all data
      auto category = dynamic_cast<KMyMoneyCategory*>(m_editWidgets[categoryWidgetName]);
      if (!category)
        return rc;
      QWidget* w = category->splitButton();
      if (w)
        w->setFocus();

      auto amount = dynamic_cast<AmountEdit*>(q->haveWidget(amountWidgetName));
      if (!amount)
        return rc;

      MyMoneyTransaction transaction;
      transaction.setCommodity(m_currency.id());
      if (splits.count() == 0 && !category->selectedItem().isEmpty()) {
        MyMoneySplit s;
        s.setAccountId(category->selectedItem());
        s.setShares(amount->value());
        s.setValue(s.shares());
        splits << s;
      }
      // use the transactions commodity as the currency indicator for the splits
      // this is used to allow some useful setting for the fractions in the amount fields
      try {
        m_phonyAccount.setCurrencyId(m_transaction.commodity());
        m_phonyAccount.fraction(MyMoneyFile::instance()->security(m_transaction.commodity()));
      } catch (const MyMoneyException &) {
        qDebug("Unable to setup precision");
      }

      if (createPseudoTransaction(transaction, splits)) {
        MyMoneyMoney value;

        QPointer<KSplitTransactionDlg> dlg = new KSplitTransactionDlg(transaction,
            m_phonySplit,
            m_phonyAccount,
            false,
            isIncome,
            MyMoneyMoney(),
            m_priceInfo,
            m_regForm);
        // q->connect(dlg, SIGNAL(newCategory(MyMoneyAccount&)), q, SIGNAL(newCategory(MyMoneyAccount&)));

        if ((rc = dlg->exec()) == QDialog::Accepted) {
          transaction = dlg->transaction();
          // collect splits out of the transaction
          splits.clear();
          MyMoneyMoney fees;
          foreach (const auto split, transaction.splits()) {
            if (split.accountId() == m_phonyAccount.id())
              continue;
            splits << split;
            fees += split.shares();
          }
          if (isIncome)
            fees = -fees;

          QString categoryId;
          q->setupCategoryWidget(category, splits, categoryId, slotEditSplits);
          amount->setValue(fees);
          q->slotUpdateTotalAmount();
        }

        delete dlg;
      }

      // focus jumps into the memo field
      if ((w = q->haveWidget("memo")) != nullptr) {
        w->setFocus();
      }

      m_openEditSplits = false;
    }
    return rc;
  }

  void updatePriceMode(const MyMoneySplit& split = MyMoneySplit())
  {
    Q_Q(OldInvestTransactionEditor);
    if (auto label = dynamic_cast<QLabel*>(q->haveWidget("price-label"))) {
      auto sharesEdit = dynamic_cast<AmountEdit*>(q->haveWidget("shares"));
      auto priceEdit = dynamic_cast<AmountEdit*>(q->haveWidget("price"));

      if (!sharesEdit || !priceEdit)
        return;

      MyMoneyMoney price;
      if (!split.id().isEmpty())
        price = split.price().reduce();
      else
        price = priceEdit->value().abs();

      if (q->priceMode() == eDialogs::PriceMode::PricePerTransaction) {
        priceEdit->setPrecision(m_currency.pricePrecision());
        label->setText(i18n("Transaction amount"));
        if (!sharesEdit->value().isZero())
          priceEdit->setValue(sharesEdit->value().abs() * price);

      } else if (q->priceMode() == eDialogs::PriceMode::PricePerShare) {
        priceEdit->setPrecision(m_security.pricePrecision());
        label->setText(i18n("Price/Share"));
        priceEdit->setValue(price);
      } else
        priceEdit->setValue(price);
    }
  }

  OldActivity*                        m_activity;
  MyMoneyAccount                   m_phonyAccount;
  MyMoneySplit                     m_phonySplit;
  MyMoneySplit                     m_assetAccountSplit;
  QList<MyMoneySplit>              m_interestSplits;
  QList<MyMoneySplit>              m_feeSplits;
  MyMoneySecurity                  m_security;
  MyMoneySecurity                  m_currency;
  eMyMoney::Split::InvestmentTransactionType m_transactionType;
};


OldInvestTransactionEditor::OldInvestTransactionEditor() :
  TransactionEditor(*new OldInvestTransactionEditorPrivate(this))
{
  Q_D(OldInvestTransactionEditor);
  d->m_transactionType = eMyMoney::Split::InvestmentTransactionType::UnknownTransactionType;
}

OldInvestTransactionEditor::~OldInvestTransactionEditor()
{
}

OldInvestTransactionEditor::OldInvestTransactionEditor(TransactionEditorContainer* regForm,
                                                 KMyMoneyRegister::InvestTransaction* item,
                                                 const KMyMoneyRegister::SelectedTransactions& list,
                                                 const QDate& lastPostDate) :
    TransactionEditor(*new OldInvestTransactionEditorPrivate(this),
                      regForm,
                      item,
                      list,
                      lastPostDate)
{
  Q_D(OldInvestTransactionEditor);
  // after the geometries of the container are updated hide the widgets which are not needed by the current activity
  connect(d->m_regForm, &TransactionEditorContainer::geometriesUpdated, this, &OldInvestTransactionEditor::slotTransactionContainerGeometriesUpdated);

  // dissect the transaction into its type, splits, currency, security etc.
  MyMoneyUtils::dissectTransaction(d->m_transaction, d->m_split,
                                    d->m_assetAccountSplit,
                                    d->m_feeSplits,
                                    d->m_interestSplits,
                                    d->m_security,
                                    d->m_currency,
                                    d->m_transactionType);

  // determine initial activity object
  d->activityFactory(d->m_transactionType);
}

void OldInvestTransactionEditor::createEditWidgets()
{
  Q_D(OldInvestTransactionEditor);
  auto activity = new KMyMoneyActivityCombo();
  activity->setObjectName("activity");
  d->m_editWidgets["activity"] = activity;
  connect(activity, &KMyMoneyActivityCombo::activitySelected, this, &OldInvestTransactionEditor::slotUpdateActivity);
  connect(activity, &KMyMoneyActivityCombo::activitySelected, this, &OldInvestTransactionEditor::slotUpdateButtonState);

  auto postDate = d->m_editWidgets["postdate"] = new KMyMoneyDateInput;
  connect(postDate, SIGNAL(dateChanged(QDate)), this, SLOT(slotUpdateButtonState()));

  auto security = new KMyMoneySecurity;
  security->setObjectName("security");
  security->setPlaceholderText(i18n("Security"));
  d->m_editWidgets["security"] = security;
  connect(security, &KMyMoneyCombo::itemSelected, this, &OldInvestTransactionEditor::slotUpdateSecurity);
  connect(security, &QComboBox::editTextChanged, this, &OldInvestTransactionEditor::slotUpdateButtonState);
  connect(security, &KMyMoneyCombo::createItem, this, &OldInvestTransactionEditor::slotCreateSecurity);
  connect(security, &KMyMoneyCombo::objectCreation, this, &OldInvestTransactionEditor::objectCreation);

  auto asset = new KMyMoneyCategory(false, nullptr);
  asset->setObjectName("asset-account");
  asset->setPlaceholderText(i18n("Asset account"));
  d->m_editWidgets["asset-account"] = asset;
  connect(asset, &QComboBox::editTextChanged, this, &OldInvestTransactionEditor::slotUpdateButtonState);
  connect(asset, &KMyMoneyCombo::objectCreation, this, &OldInvestTransactionEditor::objectCreation);

  auto fees = new KMyMoneyCategory(true, nullptr);
  fees->setObjectName("fee-account");
  fees->setPlaceholderText(i18n("Fees"));
  d->m_editWidgets["fee-account"] = fees;
  connect(fees, &KMyMoneyCombo::itemSelected, this, &OldInvestTransactionEditor::slotUpdateFeeCategory);
  connect(fees, &QComboBox::editTextChanged, this, &OldInvestTransactionEditor::slotUpdateButtonState);
  connect(fees, &KMyMoneyCombo::createItem, this, &OldInvestTransactionEditor::slotCreateFeeCategory);
  connect(fees, &KMyMoneyCombo::objectCreation, this, &OldInvestTransactionEditor::objectCreation);
  connect(fees->splitButton(), &QAbstractButton::clicked, this, &OldInvestTransactionEditor::slotEditFeeSplits);

  auto interest = new KMyMoneyCategory(true, nullptr);
  interest->setPlaceholderText(i18n("Interest"));
  interest->setObjectName("interest-account");
  d->m_editWidgets["interest-account"] = interest;
  connect(interest, &KMyMoneyCombo::itemSelected, this, &OldInvestTransactionEditor::slotUpdateInterestCategory);
  connect(interest, &QComboBox::editTextChanged, this, &OldInvestTransactionEditor::slotUpdateButtonState);
  connect(interest, &KMyMoneyCombo::createItem, this, &OldInvestTransactionEditor::slotCreateInterestCategory);
  connect(interest, &KMyMoneyCombo::objectCreation, this, &OldInvestTransactionEditor::objectCreation);
  connect(interest->splitButton(), &QAbstractButton::clicked, this, &OldInvestTransactionEditor::slotEditInterestSplits);

  auto tag = new KTagContainer;
  tag->tagCombo()->setObjectName(QLatin1String("tag"));
  d->m_editWidgets["tag"] = tag;
  connect(tag->tagCombo(), &QComboBox::editTextChanged, this, &OldInvestTransactionEditor::slotUpdateButtonState);
#if 0
  connect(tag->tagCombo(), &KMyMoneyMVCCombo::createItem, this, &OldInvestTransactionEditor::slotNewTag);
  connect(tag->tagCombo(), &KMyMoneyMVCCombo::objectCreation, this, &OldInvestTransactionEditor::objectCreation);
#endif

  auto memo = new KTextEdit;
  memo->setObjectName("memo");
  memo->setTabChangesFocus(true);
  d->m_editWidgets["memo"] = memo;
  connect(memo, &QTextEdit::textChanged, this, &OldInvestTransactionEditor::slotUpdateInvestMemoState);
  connect(memo, &QTextEdit::textChanged, this, &OldInvestTransactionEditor::slotUpdateButtonState);

  d->m_activity->memoText().clear();
  d->m_activity->memoChanged() = false;

  AmountEdit* value = new AmountEdit;
  value->setObjectName("shares");
  value->setPlaceholderText(i18n("Shares"));
  value->setCalculatorButtonVisible(true);
  d->m_editWidgets["shares"] = value;
  connect(value, &AmountEdit::textChanged, this, &OldInvestTransactionEditor::slotUpdateButtonState);
  connect(value, &AmountEdit::valueChanged, this, &OldInvestTransactionEditor::slotUpdateTotalAmount);

  value = new AmountEdit;
  value->setObjectName("price");
  value->setPlaceholderText(i18n("Price"));
  value->setCalculatorButtonVisible(true);
  d->m_editWidgets["price"] = value;
  connect(value, &AmountEdit::textChanged, this, &OldInvestTransactionEditor::slotUpdateButtonState);
  connect(value, &AmountEdit::valueChanged, this, &OldInvestTransactionEditor::slotUpdateTotalAmount);

  value = new AmountEdit;
  value->setObjectName("fee-amount");
  // TODO once we have the selected transactions as array of Transaction
  // we can allow multiple splits for fee and interest
  value->setCalculatorButtonVisible(true);
  d->m_editWidgets["fee-amount"] = value;
  connect(value, &AmountEdit::textChanged, this, &OldInvestTransactionEditor::slotUpdateButtonState);
  connect(value, &AmountEdit::valueChanged, this, &OldInvestTransactionEditor::slotUpdateTotalAmount);

  value = new AmountEdit;
  value->setObjectName("interest-amount");
  // TODO once we have the selected transactions as array of Transaction
  // we can allow multiple splits for fee and interest
  value->setCalculatorButtonVisible(true);
  d->m_editWidgets["interest-amount"] = value;
  connect(value, &AmountEdit::textChanged, this, &OldInvestTransactionEditor::slotUpdateButtonState);
  connect(value, &AmountEdit::valueChanged, this, &OldInvestTransactionEditor::slotUpdateTotalAmount);

  auto reconcile = new KMyMoneyReconcileCombo;
  reconcile->setObjectName("reconcile");
  d->m_editWidgets["status"] = reconcile;
  connect(reconcile, &KMyMoneyMVCCombo::itemSelected, this, &OldInvestTransactionEditor::slotUpdateButtonState);

  KMyMoneyRegister::QWidgetContainer::iterator it_w;
  for (it_w = d->m_editWidgets.begin(); it_w != d->m_editWidgets.end(); ++it_w) {
    (*it_w)->installEventFilter(this);
  }

  QLabel* label;

  d->m_editWidgets["activity-label"] = label = new QLabel(i18n("Activity"));
  label->setAlignment(Qt::AlignVCenter);

  d->m_editWidgets["postdate-label"] = label = new QLabel(i18n("Date"));
  label->setAlignment(Qt::AlignVCenter);

  d->m_editWidgets["security-label"] = label = new QLabel(i18n("Security"));
  label->setAlignment(Qt::AlignVCenter);

  d->m_editWidgets["shares-label"] = label = new QLabel(i18n("Shares"));
  label->setAlignment(Qt::AlignVCenter);

  d->m_editWidgets["asset-label"] = label = new QLabel(i18n("Account"));
  label->setAlignment(Qt::AlignVCenter);

  d->m_editWidgets["price-label"] = label = new QLabel(i18n("Price/share"));
  label->setAlignment(Qt::AlignVCenter);

  d->m_editWidgets["fee-label"] = label = new QLabel(i18n("Fees"));
  label->setAlignment(Qt::AlignVCenter);

  d->m_editWidgets["fee-amount-label"] = label = new QLabel(QString());
  label->setAlignment(Qt::AlignVCenter);

  d->m_editWidgets["interest-label"] = label = new QLabel(i18n("Interest"));
  label->setAlignment(Qt::AlignVCenter);

  d->m_editWidgets["interest-amount-label"] = label = new QLabel(i18n("Interest"));
  label->setAlignment(Qt::AlignVCenter);

  d->m_editWidgets["memo-label"] = label = new QLabel(i18n("Memo"));
  label->setAlignment(Qt::AlignVCenter);

  d->m_editWidgets["total"] = label = new QLabel(QString());
  label->setAlignment(Qt::AlignVCenter | Qt::AlignRight);

  d->m_editWidgets["total-label"] = label = new QLabel(i18nc("Total value", "Total"));
  label->setAlignment(Qt::AlignVCenter);

  d->m_editWidgets["status-label"] = label = new QLabel(i18n("Status"));
  label->setAlignment(Qt::AlignVCenter);

  // if we don't have more than 1 selected transaction, we don't need
  // the "don't change" item in some of the combo widgets
  if (d->m_transactions.count() < 2) {
    reconcile->removeDontCare();
  }
}

int OldInvestTransactionEditor::slotEditFeeSplits()
{
  Q_D(OldInvestTransactionEditor);
  return d->editSplits("fee-account", "fee-amount", d->m_feeSplits, false, SLOT(slotEditFeeSplits()));
}

int OldInvestTransactionEditor::slotEditInterestSplits()
{
  Q_D(OldInvestTransactionEditor);
  return d->editSplits("interest-account", "interest-amount", d->m_interestSplits, true, SLOT(slotEditInterestSplits()));
}

void OldInvestTransactionEditor::slotCreateSecurity(const QString& name, QString& id)
{
  Q_D(OldInvestTransactionEditor);
  MyMoneyAccount acc;
  QRegExp exp("([^:]+)");
  if (exp.indexIn(name) != -1) {
    acc.setName(exp.cap(1));

    slotNewInvestment(acc, d->m_account);

    // return id
    id = acc.id();

    if (!id.isEmpty()) {
      slotUpdateSecurity(id);
      slotReloadEditWidgets();
    }
  }
}

void OldInvestTransactionEditor::slotCreateFeeCategory(const QString& name, QString& id)
{
  MyMoneyAccount acc;
  acc.setName(name);

  slotNewCategory(acc, MyMoneyFile::instance()->expense());

  // return id
  id = acc.id();
}

void OldInvestTransactionEditor::slotUpdateFeeCategory(const QString& id)
{
  haveWidget("fee-amount")->setDisabled(id.isEmpty());
}

void OldInvestTransactionEditor::slotUpdateInterestCategory(const QString& id)
{
  haveWidget("interest-amount")->setDisabled(id.isEmpty());
}

void OldInvestTransactionEditor::slotCreateInterestCategory(const QString& name, QString& id)
{
  MyMoneyAccount acc;
  acc.setName(name);

  slotNewCategory(acc, MyMoneyFile::instance()->income());

  id = acc.id();
}

void OldInvestTransactionEditor::slotReloadEditWidgets()
{
  Q_D(OldInvestTransactionEditor);
  auto interest = dynamic_cast<KMyMoneyCategory*>(haveWidget("interest-account"));
  auto fees = dynamic_cast<KMyMoneyCategory*>(haveWidget("fee-account"));
  auto security = dynamic_cast<KMyMoneySecurity*>(haveWidget("security"));

  if (!interest || !fees || !security)
    return;

  AccountSet aSet;
  QString id;

  // interest-account
  aSet.clear();
  aSet.addAccountGroup(Account::Type::Income);
  aSet.load(interest->selector());
  setupCategoryWidget(interest, d->m_interestSplits, id, SLOT(slotEditInterestSplits()));

  // fee-account
  aSet.clear();
  aSet.addAccountGroup(Account::Type::Expense);
  aSet.load(fees->selector());
  setupCategoryWidget(fees, d->m_feeSplits, id, SLOT(slotEditFeeSplits()));

  // security
  aSet.clear();
  aSet.load(security->selector(), i18n("Security"), d->m_account.accountList(), true);
}

void OldInvestTransactionEditor::loadEditWidgets(eWidgets::eRegister::Action)
{
  loadEditWidgets();
}

void OldInvestTransactionEditor::loadEditWidgets()
{
  Q_D(OldInvestTransactionEditor);
  QString id;

  auto postDate = dynamic_cast<KMyMoneyDateInput*>(haveWidget("postdate"));
  auto reconcile = dynamic_cast<KMyMoneyReconcileCombo*>(haveWidget("status"));
  auto security = dynamic_cast<KMyMoneySecurity*>(haveWidget("security"));
  auto activity = dynamic_cast<KMyMoneyActivityCombo*>(haveWidget("activity"));
  auto asset = dynamic_cast<KMyMoneyCategory*>(haveWidget("asset-account"));
  auto memo = dynamic_cast<KTextEdit*>(d->m_editWidgets["memo"]);
  AmountEdit* value;
  auto interest = dynamic_cast<KMyMoneyCategory*>(haveWidget("interest-account"));
  auto fees = dynamic_cast<KMyMoneyCategory*>(haveWidget("fee-account"));

  if (!postDate || !reconcile || !security || !activity ||
      !asset || !memo || !interest || !fees)
    return;

  // check if the current transaction has a reference to an equity account
  auto haveEquityAccount = false;
  foreach (const auto split, d->m_transaction.splits()) {
    auto acc = MyMoneyFile::instance()->account(split.accountId());
    if (acc.accountType() == Account::Type::Equity) {
      haveEquityAccount = true;
      break;
    }
  }

  // asset-account
  AccountSet aSet;
  aSet.clear();
  aSet.addAccountType(Account::Type::Checkings);
  aSet.addAccountType(Account::Type::Savings);
  aSet.addAccountType(Account::Type::Cash);
  aSet.addAccountType(Account::Type::Asset);
  aSet.addAccountType(Account::Type::Currency);
  aSet.addAccountType(Account::Type::CreditCard);
  if (KMyMoneySettings::expertMode() || haveEquityAccount)
    aSet.addAccountGroup(Account::Type::Equity);
  aSet.load(asset->selector());

  // security
  security->setSuppressObjectCreation(false);    // allow object creation on the fly
  aSet.clear();
  aSet.load(security->selector(), i18n("Security"), d->m_account.accountList(), true);

  // memo
  memo->setText(d->m_split.memo());
  d->m_activity->memoText() = d->m_split.memo();
  d->m_activity->memoChanged() = false;

  if (!isMultiSelection()) {
    // date
    if (d->m_transaction.postDate().isValid())
      postDate->setDate(d->m_transaction.postDate());
    else if (d->m_lastPostDate.isValid())
      postDate->setDate(d->m_lastPostDate);
    else
      postDate->setDate(QDate::currentDate());

    // security (but only if it's not the investment account)
    if (d->m_split.accountId() != d->m_account.id()) {
      security->completion()->setSelected(d->m_split.accountId());
      security->slotItemSelected(d->m_split.accountId());
    }

    // activity
    activity->setActivity(d->m_activity->type());
    slotUpdateActivity(activity->activity());

    asset->completion()->setSelected(d->m_assetAccountSplit.accountId());
    asset->slotItemSelected(d->m_assetAccountSplit.accountId());

    // interest-account
    aSet.clear();
    aSet.addAccountGroup(Account::Type::Income);
    aSet.load(interest->selector());
    setupCategoryWidget(interest, d->m_interestSplits, id, SLOT(slotEditInterestSplits()));

    // fee-account
    aSet.clear();
    aSet.addAccountGroup(Account::Type::Expense);
    aSet.load(fees->selector());
    setupCategoryWidget(fees, d->m_feeSplits, id, SLOT(slotEditFeeSplits()));

    // shares
    // don't set the value if the number of shares is zero so that
    // we can see the hint
    value = dynamic_cast<AmountEdit*>(haveWidget("shares"));
    if (!value)
      return;
    if (typeid(*(d->m_activity)) != typeid(Invest::Split(this)))
      value->setPrecision(MyMoneyMoney::denomToPrec(d->m_security.smallestAccountFraction()));
    else
      value->setPrecision(-1);

    if (!d->m_split.shares().isZero())
      value->setValue(d->m_split.shares().abs());

    // price
    d->updatePriceMode(d->m_split);

    // fee amount
    value = dynamic_cast<AmountEdit*>(haveWidget("fee-amount"));
    if (!value)
      return;
    value->setPrecision(MyMoneyMoney::denomToPrec(d->m_currency.smallestAccountFraction()));
    value->setValue(d->subtotal(d->m_feeSplits));

    // interest amount
    value = dynamic_cast<AmountEdit*>(haveWidget("interest-amount"));
    if (!value)
      return;
    value->setPrecision(MyMoneyMoney::denomToPrec(d->m_currency.smallestAccountFraction()));
    value->setValue(-d->subtotal(d->m_interestSplits));

    // total
    slotUpdateTotalAmount();

    // status
    if (d->m_split.reconcileFlag() == eMyMoney::Split::State::Unknown)
      d->m_split.setReconcileFlag(eMyMoney::Split::State::NotReconciled);
    reconcile->setState(d->m_split.reconcileFlag());

  } else {
    postDate->loadDate(QDate());
    reconcile->setState(eMyMoney::Split::State::Unknown);

    // We don't allow to change the activity
    activity->setActivity(d->m_activity->type());
    slotUpdateActivity(activity->activity());
    activity->setDisabled(true);

    // scan the list of selected transactions and check that they have
    // the same activity.
    const QString& action = d->m_item->split().action();
    bool isNegative = d->m_item->split().shares().isNegative();
    bool allSameActivity = true;
    for (auto it_t = d->m_transactions.begin(); allSameActivity && (it_t != d->m_transactions.end()); ++it_t) {
      allSameActivity = (action == (*it_t).split().action() && (*it_t).split().shares().isNegative() == isNegative);
    }

    QStringList fields;
    fields << "shares" << "price" << "fee-amount" << "interest-amount";
    for (auto it_f = fields.constBegin(); it_f != fields.constEnd(); ++it_f) {
      value = dynamic_cast<AmountEdit*>(haveWidget((*it_f)));
      if (!value)
        return;
      value->setText("");
      value->setAllowEmpty();
    }

    // if we have transactions with different activities, disable some more widgets
    if (!allSameActivity) {
      fields << "asset-account" << "fee-account" << "interest-account";
      for (auto it_f = fields.constBegin(); it_f != fields.constEnd(); ++it_f) {
        haveWidget(*it_f)->setDisabled(true);
      }
    }
  }
}

QWidget* OldInvestTransactionEditor::firstWidget() const
{
  return nullptr; // let the creator use the first widget in the tab order
}

bool OldInvestTransactionEditor::isComplete(QString& reason) const
{
  Q_D(const OldInvestTransactionEditor);
  reason.clear();

  auto postDate = dynamic_cast<KMyMoneyDateInput*>(d->m_editWidgets["postdate"]);
  if (postDate) {
    QDate accountOpeningDate = d->m_account.openingDate();
    auto asset = dynamic_cast<KMyMoneyCategory*>(haveWidget("asset-account"));
    if (asset && asset->isVisible()) {
      if (!isMultiSelection() || !asset->currentText().isEmpty()) {
        const auto assetId = asset->selectedItem();
        if (!assetId.isEmpty()) {
          try {
            const auto acc = MyMoneyFile::instance()->account(assetId);
            if (acc.openingDate() > accountOpeningDate)
              accountOpeningDate = acc.openingDate();
          } catch(MyMoneyException& e) {
            qDebug() << "opening date check failed on account" << assetId << e.what();
          }
        }
      }
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

  return d->m_activity->isComplete(reason);
}

void OldInvestTransactionEditor::slotUpdateSecurity(const QString& stockId)
{
  Q_D(OldInvestTransactionEditor);
  auto file = MyMoneyFile::instance();
  MyMoneyAccount stock = file->account(stockId);
  d->m_security = file->security(stock.currencyId());
  d->m_currency = file->security(d->m_security.tradingCurrency());
  bool currencyKnown = !d->m_currency.id().isEmpty();
  if (!currencyKnown) {
    d->m_currency.setTradingSymbol("???");
  } else {
    auto sharesWidget = dynamic_cast<AmountEdit*>(haveWidget("shares"));
    if (sharesWidget) {
      if (typeid(*(d->m_activity)) != typeid(Invest::Split(this)))
        sharesWidget->setPrecision(MyMoneyMoney::denomToPrec(d->m_security.smallestAccountFraction()));
      else
        sharesWidget->setPrecision(-1);
    }
  }

  d->updatePriceMode();

  d->m_activity->preloadAssetAccount();

  haveWidget("shares")->setEnabled(currencyKnown);
  haveWidget("price")->setEnabled(currencyKnown);
  haveWidget("fee-amount")->setEnabled(currencyKnown);
  haveWidget("interest-amount")->setEnabled(currencyKnown);

  slotUpdateTotalAmount();
  slotUpdateButtonState();
  resizeForm();
}

bool OldInvestTransactionEditor::fixTransactionCommodity(const MyMoneyAccount& /* account */)
{
  return true;
}


MyMoneyMoney OldInvestTransactionEditor::totalAmount() const
{
  MyMoneyMoney amount;

  auto activityCombo = dynamic_cast<KMyMoneyActivityCombo*>(haveWidget("activity"));
  auto sharesEdit = dynamic_cast<AmountEdit*>(haveWidget("shares"));
  auto priceEdit = dynamic_cast<AmountEdit*>(haveWidget("price"));
  auto feesEdit = dynamic_cast<AmountEdit*>(haveWidget("fee-amount"));
  auto interestEdit = dynamic_cast<AmountEdit*>(haveWidget("interest-amount"));

  if (!activityCombo || !sharesEdit || !priceEdit ||
      !feesEdit || !interestEdit)
    return amount;

  if (priceMode() == eDialogs::PriceMode::PricePerTransaction)
    amount = priceEdit->value().abs();
  else
    amount = sharesEdit->value().abs() * priceEdit->value().abs();

  if (feesEdit->isVisible()) {
    MyMoneyMoney fee = feesEdit->value();
    MyMoneyMoney factor(-1, 1);
    switch (activityCombo->activity()) {
      case eMyMoney::Split::InvestmentTransactionType::BuyShares:
      case eMyMoney::Split::InvestmentTransactionType::ReinvestDividend:
        factor = MyMoneyMoney::ONE;
        break;
      default:
        break;
    }
    amount += (fee * factor);
  }

  if (interestEdit->isVisible()) {
    MyMoneyMoney interest = interestEdit->value();
    MyMoneyMoney factor(1, 1);
    switch (activityCombo->activity()) {
      case eMyMoney::Split::InvestmentTransactionType::BuyShares:
        factor = MyMoneyMoney::MINUS_ONE;
        break;
      default:
        break;
    }
    amount += (interest * factor);
  }
  return amount;
}

void OldInvestTransactionEditor::slotUpdateTotalAmount()
{
  Q_D(OldInvestTransactionEditor);
  auto total = dynamic_cast<QLabel*>(haveWidget("total"));

  if (total && total->isVisible()) {
    total->setText(totalAmount().convert(d->m_currency.smallestAccountFraction(), d->m_security.roundingMethod())
                   .formatMoney(d->m_currency.tradingSymbol(), MyMoneyMoney::denomToPrec(d->m_currency.smallestAccountFraction())));
  }
}

void OldInvestTransactionEditor::slotTransactionContainerGeometriesUpdated()
{
  Q_D(OldInvestTransactionEditor);
  // when the geometries of the transaction container are updated some edit widgets that were
  // previously hidden are being shown (see QAbstractItemView::updateEditorGeometries) so we
  // need to update the activity with the current activity in order to show only the widgets
  // which are needed by the current activity
  if (d->m_editWidgets.isEmpty())
    return;
  slotUpdateActivity(d->m_activity->type());
}

void OldInvestTransactionEditor::slotUpdateActivity(eMyMoney::Split::InvestmentTransactionType activity)
{
  Q_D(OldInvestTransactionEditor);
  // create new activity object if required
  d->activityFactory(activity);

  // hide all dynamic widgets
  d->showCategory("interest-account", false);
  d->showCategory("fee-account", false);

  QStringList dynwidgets;
  dynwidgets << "total-label" << "asset-label" << "fee-label" << "fee-amount-label" << "interest-label" << "interest-amount-label" << "price-label" << "shares-label";

  // hiding labels works by clearing them. hide() does not do the job
  // as the underlying text in the QTable object will shine through
  QStringList::const_iterator it_s;
  for (it_s = dynwidgets.constBegin(); it_s != dynwidgets.constEnd(); ++it_s) {
    QLabel* w = dynamic_cast<QLabel*>(haveWidget(*it_s));
    if (w)
      w->setText(QStringLiteral(" "));
  }

  // real widgets can be hidden
  dynwidgets.clear();
  dynwidgets << "asset-account" << "interest-amount" << "fee-amount" << "shares" << "price" << "total";

  for (it_s = dynwidgets.constBegin(); it_s != dynwidgets.constEnd(); ++it_s) {
    QWidget* w = haveWidget(*it_s);
    if (w)
      w->hide();
  }
  d->m_activity->showWidgets();
  d->m_activity->preloadAssetAccount();
}

eDialogs::PriceMode OldInvestTransactionEditor::priceMode() const
{
  Q_D(const OldInvestTransactionEditor);
  eDialogs::PriceMode mode = static_cast<eDialogs::PriceMode>(eDialogs::PriceMode::Price);
  auto sec = dynamic_cast<KMyMoneySecurity*>(d->m_editWidgets["security"]);

  QString accId;
  if (sec && !sec->currentText().isEmpty()) {
    accId = sec->selectedItem();
    if (accId.isEmpty())
      accId = d->m_account.id();
  }
  while (!accId.isEmpty() && mode == eDialogs::PriceMode::Price) {
    auto acc = MyMoneyFile::instance()->account(accId);
    if (acc.value("priceMode").isEmpty())
      accId = acc.parentAccountId();
    else
      mode = static_cast<eDialogs::PriceMode>(acc.value("priceMode").toInt());
  }

  // if mode is still <Price> then use that
  if (mode == eDialogs::PriceMode::Price)
    mode = eDialogs::PriceMode::PricePerShare;
  return mode;
}

MyMoneySecurity OldInvestTransactionEditor::security() const
{
  Q_D(const OldInvestTransactionEditor);
  return d->m_security;
}

QList<MyMoneySplit> OldInvestTransactionEditor::feeSplits() const
{
  Q_D(const OldInvestTransactionEditor);
  return d->m_feeSplits;
}

QList<MyMoneySplit> OldInvestTransactionEditor::interestSplits() const
{
  Q_D(const OldInvestTransactionEditor);
  return d->m_interestSplits;
}

bool OldInvestTransactionEditor::setupPrice(const MyMoneyTransaction& t, MyMoneySplit& split)
{
  Q_D(OldInvestTransactionEditor);
  auto file = MyMoneyFile::instance();
  auto acc = file->account(split.accountId());
  MyMoneySecurity toCurrency(file->security(acc.currencyId()));
  int fract = acc.fraction();

  if (acc.currencyId() != t.commodity()) {
    if (acc.currencyId().isEmpty())
      acc.setCurrencyId(t.commodity());

    QMap<QString, MyMoneyMoney>::Iterator it_p;
    QString key = t.commodity() + '-' + acc.currencyId();
    it_p = d->m_priceInfo.find(key);

    // if it's not found, then collect it from the user first
    MyMoneyMoney price;
    if (it_p == d->m_priceInfo.end()) {
      MyMoneySecurity fromCurrency = file->security(t.commodity());
      MyMoneyMoney fromValue, toValue;

      fromValue = split.value();
      const MyMoneyPrice &priceInfo = MyMoneyFile::instance()->price(fromCurrency.id(), toCurrency.id(), t.postDate());
      toValue = split.value() * priceInfo.rate(toCurrency.id());

      QPointer<KCurrencyCalculator> calc =
        new KCurrencyCalculator(fromCurrency,
                                toCurrency,
                                fromValue,
                                toValue,
                                t.postDate(),
                                fract,
                                d->m_regForm);

      if (calc->exec() == QDialog::Rejected) {
        delete calc;
        return false;
      }
      price = calc->price();
      delete calc;
      d->m_priceInfo[key] = price;
    } else {
      price = (*it_p);
    }

    // update shares if the transaction commodity is the currency
    // of the current selected account
    split.setShares(split.value() * price);
  } else {
    split.setShares(split.value());
  }

  return true;
}

bool OldInvestTransactionEditor::createTransaction(MyMoneyTransaction& t, const MyMoneyTransaction& torig, const MyMoneySplit& sorig, bool /* skipPriceDialog */)
{
  Q_D(OldInvestTransactionEditor);
  auto file = MyMoneyFile::instance();
  // we start with the previous values, make sure we can add them later on
  t = torig;
  MyMoneySplit s0 = sorig;
  s0.clearId();

  auto sec = dynamic_cast<KMyMoneySecurity*>(d->m_editWidgets["security"]);
  if (sec && (!isMultiSelection() || !sec->currentText().isEmpty())) {
    QString securityId = sec->selectedItem();
    if (!securityId.isEmpty()) {
      s0.setAccountId(securityId);
      MyMoneyAccount stockAccount = file->account(securityId);
      QString currencyId = stockAccount.currencyId();
      MyMoneySecurity security = file->security(currencyId);

      t.setCommodity(security.tradingCurrency());
    } else {
      s0.setAccountId(d->m_account.id());
      t.setCommodity(d->m_account.currencyId());
    }
  }

  // extract price info from original transaction
  d->m_priceInfo.clear();
  if (!torig.id().isEmpty()) {
    foreach (const auto split, torig.splits()) {
      if (split.id() != sorig.id()) {
        auto cat = file->account(split.accountId());
        if (cat.currencyId() != t.commodity()) {
          if (cat.currencyId().isEmpty())
            cat.setCurrencyId(d->m_account.currencyId());
          if (!split.shares().isZero() && !split.value().isZero()) {
            d->m_priceInfo[cat.currencyId()] = (split.shares() / split.value()).reduce();
          }
        }
      }
    }
  }

  t.removeSplits();

  auto postDate = dynamic_cast<KMyMoneyDateInput*>(d->m_editWidgets["postdate"]);
  if (postDate && postDate->date().isValid()) {
    t.setPostDate(postDate->date());
  }

  // memo and number field are special: if we have multiple transactions selected
  // and the edit field is empty, we treat it as "not modified".
  // FIXME a better approach would be to have a 'dirty' flag with the widgets
  //       which identifies if the originally loaded value has been modified
  //       by the user
  auto memo = dynamic_cast<KTextEdit*>(d->m_editWidgets["memo"]);
  if (memo) {
    if (!isMultiSelection() || d->m_activity->memoChanged())
      s0.setMemo(memo->toPlainText());
  }

  MyMoneySplit assetAccountSplit;
  QList<MyMoneySplit> feeSplits;
  QList<MyMoneySplit> interestSplits;
  MyMoneySecurity security;
  MyMoneySecurity currency = file->security(t.commodity());
  eMyMoney::Split::InvestmentTransactionType transactionType;

  // extract the splits from the original transaction, but only
  // if there is one because otherwise the currency is overridden
  if (torig.splitCount() != 0) {
    MyMoneyUtils::dissectTransaction(torig, sorig,
                     assetAccountSplit,
                     feeSplits,
                     interestSplits,
                     security,
                     currency,
                     transactionType);
  }
  // check if the trading currency is the same if the security has changed
  // in case it differs, check that we have a price (request from user)
  // and convert all splits
  // TODO

  // do the conversions here
  // TODO

  // keep the current activity object and create a new one
  // that can be destroyed later on
  auto activity = d->m_activity;
  d->m_activity = nullptr;      // make sure we create a new one
  d->activityFactory(activity->type());

  // if the activity is not set in the combo widget, we keep
  // the one which is used in the original transaction
  auto activityCombo = dynamic_cast<KMyMoneyActivityCombo*>(haveWidget("activity"));
  if (activityCombo && activityCombo->activity() == eMyMoney::Split::InvestmentTransactionType::UnknownTransactionType) {
    d->activityFactory(transactionType);
  }

  // if we mark the split reconciled here, we'll use today's date if no reconciliation date is given
  auto status = dynamic_cast<KMyMoneyReconcileCombo*>(d->m_editWidgets["status"]);
  if (status && status->state() != eMyMoney::Split::State::Unknown)
    s0.setReconcileFlag(status->state());

  if (s0.reconcileFlag() == eMyMoney::Split::State::Reconciled && !s0.reconcileDate().isValid())
    s0.setReconcileDate(QDate::currentDate());

  // call the creation logic for the current selected activity
  bool rc = d->m_activity->createTransaction(t, s0, assetAccountSplit, feeSplits, d->m_feeSplits, interestSplits, d->m_interestSplits, security, currency);

  // now switch back to the original activity
  delete d->m_activity;
  d->m_activity = activity;

  // add the splits to the transaction
  if (rc) {
    if (security.name().isEmpty())                                              // new transaction has no security filled...
      security = file->security(file->account(s0.accountId()).currencyId());    // ...so fetch it from s0 split

    QList<MyMoneySplit> resultSplits;  // concatenates splits for easy processing

    if (!assetAccountSplit.accountId().isEmpty())
      resultSplits.append(assetAccountSplit);

    if (!feeSplits.isEmpty())
      resultSplits.append(feeSplits);

    if (!interestSplits.isEmpty())
      resultSplits.append(interestSplits);

    AlkValue::RoundingMethod roundingMethod = AlkValue::RoundRound;
    if (security.roundingMethod() != AlkValue::RoundNever)
      roundingMethod = security.roundingMethod();

    int currencyFraction = currency.smallestAccountFraction();
    int securityFraction = security.smallestAccountFraction();

    // assuming that all non-stock splits are monetary
    foreach (auto split, resultSplits) {
      split.clearId();
      split.setShares(MyMoneyMoney(split.shares().convertDenominator(currencyFraction, roundingMethod)));
      split.setValue(MyMoneyMoney(split.value().convertDenominator(currencyFraction, roundingMethod)));
      t.addSplit(split);
    }

    // Don't do any rounding on a split factor
    if (d->m_activity->type() != eMyMoney::Split::InvestmentTransactionType::SplitShares) {
      s0.setShares(MyMoneyMoney(s0.shares().convertDenominator(securityFraction, roundingMethod))); // only shares variable from stock split isn't evaluated in currency
      s0.setValue(MyMoneyMoney(s0.value().convertDenominator(currencyFraction, roundingMethod)));
    }
    t.addSplit(s0);
  }

  return rc;
}

void OldInvestTransactionEditor::setupFinalWidgets()
{
  addFinalWidget(haveWidget("memo"));
}

void OldInvestTransactionEditor::slotUpdateInvestMemoState()
{
  Q_D(OldInvestTransactionEditor);
  auto memo = dynamic_cast<KTextEdit*>(d->m_editWidgets["memo"]);
  if (memo) {
    d->m_activity->memoChanged() = (memo->toPlainText() != d->m_activity->memoText());
  }
}
