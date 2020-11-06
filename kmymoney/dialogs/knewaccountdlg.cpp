/*
 * Copyright 2000-2003  Michael Edwardes <mte@users.sourceforge.net>
 * Copyright 2005-2019  Thomas Baumgart <tbaumgart@kde.org>
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

#include "knewaccountdlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPushButton>
#include <QLabel>
#include <QButtonGroup>
#include <QCheckBox>
#include <QTabWidget>
#include <QRadioButton>
#include <QList>
#include <QStringListModel>

// ----------------------------------------------------------------------------
// KDE Headers

#include <KMessageBox>
#include <KComboBox>
#include <kguiutils.h>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_knewaccountdlg.h"

#include "kmymoneydateinput.h"
#include <mymoneyexception.h>
#include "mymoneyfile.h"
#include "mymoneyinstitution.h"
#include "mymoneyaccount.h"
#include "kmymoneysettings.h"
#include "kmymoneycurrencyselector.h"
#include "knewinstitutiondlg.h"
#include "accountsmodel.h"
#include "accountsproxymodel.h"
#include "mymoneyenums.h"
#include "modelenums.h"
#include "columnselector.h"

using namespace eMyMoney;

class KNewAccountDlgPrivate
{
  Q_DISABLE_COPY(KNewAccountDlgPrivate)
  Q_DECLARE_PUBLIC(KNewAccountDlg)

public:
  explicit KNewAccountDlgPrivate(KNewAccountDlg *qq) :
    q_ptr(qq),
    ui(new Ui::KNewAccountDlg),
    m_filterProxyModel(nullptr),
    m_categoryEditor(false),
    m_isEditing(false)
  {
  }

  ~KNewAccountDlgPrivate()
  {
    delete ui;
  }

  void init()
  {
    Q_Q(KNewAccountDlg);
    ui->setupUi(q);

    auto file = MyMoneyFile::instance();

    // initialize the m_parentAccount member
    QVector<Account::Type> filterAccountGroup {m_account.accountGroup()};
    switch (m_account.accountGroup()) {
      case Account::Type::Asset:
        m_parentAccount = file->asset();
        break;
      case Account::Type::Liability:
        m_parentAccount = file->liability();
        break;
      case Account::Type::Income:
        m_parentAccount = file->income();
        break;
      case Account::Type::Expense:
        m_parentAccount = file->expense();
        break;
      case Account::Type::Equity:
        m_parentAccount = file->equity();
        break;
      default:
        qDebug("Seems we have an account that hasn't been mapped to the top five");
        if (m_categoryEditor) {
          m_parentAccount = file->income();
          filterAccountGroup[0] = Account::Type::Income;
        } else {
          m_parentAccount = file->asset();
          filterAccountGroup[0] = Account::Type::Asset;
        }
    }

    ui->m_amountGroup->setId(ui->m_grossAmount, 0);
    ui->m_amountGroup->setId(ui->m_netAmount, 1);

    // the proxy filter model
    m_filterProxyModel = ui->m_parentAccounts->proxyModel();
    m_filterProxyModel->setHideClosedAccounts(true);
    m_filterProxyModel->setHideEquityAccounts(!KMyMoneySettings::expertMode());
    m_filterProxyModel->addAccountGroup(filterAccountGroup);
    // don't allow to select ourself as parent
    m_filterProxyModel->setNotSelectable(m_account.id());
    auto const model = MyMoneyFile::instance()->accountsModel();
    m_filterProxyModel->setDynamicSortFilter(true);

    ui->m_parentAccounts->setModel(model);

    // only show the name column for the parent account
    auto columnSelector = new ColumnSelector(ui->m_parentAccounts);
    columnSelector->setAlwaysHidden(columnSelector->columns());
    columnSelector->setAlwaysVisible(QVector<int>({ AccountsModel::Column::AccountName }));

    ui->m_parentAccounts->sortByColumn(AccountsModel::Column::AccountName, Qt::AscendingOrder);

    columnSelector->setModel(m_filterProxyModel);

    ui->m_subAccountLabel->setText(i18n("Is a sub account"));
    q->connect(ui->m_parentAccounts->selectionModel(), &QItemSelectionModel::selectionChanged,
               q, &KNewAccountDlg::slotSelectionChanged);

    // select the current parent in the hierarchy
    QModelIndex idx = model->indexById(m_account.parentAccountId());
    if (idx.isValid()) {
      idx = model->mapFromBaseSource(m_filterProxyModel, idx);
      ui->m_parentAccounts->selectionModel()->select(idx, QItemSelectionModel::SelectCurrent);
      ui->m_parentAccounts->expand(idx);
      ui->m_parentAccounts->scrollTo(idx, QAbstractItemView::PositionAtTop);
    }

    ui->accountNameEdit->setText(m_account.name());
    ui->descriptionEdit->setText(m_account.description());

    ui->typeCombo->setEnabled(true);

    // load the price mode combo
    ui->m_priceMode->insertItem(i18nc("default price mode", "(default)"), 0);
    ui->m_priceMode->insertItem(i18n("Price per share"), 1);
    ui->m_priceMode->insertItem(i18n("Total for all shares"), 2);

    int priceMode = 0;
    if (m_account.accountType() == Account::Type::Investment) {
      ui->m_priceMode->setEnabled(true);
      if (!m_account.value("priceMode").isEmpty())
        priceMode = m_account.value("priceMode").toInt();
    }
    ui->m_priceMode->setCurrentItem(priceMode);

    bool haveMinBalance = false;
    bool haveMaxCredit = false;
    if (!m_account.openingDate().isValid()) {
      m_account.setOpeningDate(KMyMoneySettings::firstFiscalDate());
    }
    ui->m_openingDateEdit->setDate(m_account.openingDate());

    handleOpeningBalanceCheckbox(m_account.currencyId());

    if (m_categoryEditor) {
      // get rid of the tabs that are not used for categories
      int tab = ui->m_tab->indexOf(ui->m_institutionTab);
      if (tab != -1)
        ui->m_tab->removeTab(tab);
      tab = ui->m_tab->indexOf(ui->m_limitsTab);
      if (tab != -1)
        ui->m_tab->removeTab(tab);

      //m_qlistviewParentAccounts->setEnabled(true);
      ui->accountNoEdit->setEnabled(false);

      ui->m_institutionBox->hide();
      ui->m_qcheckboxNoVat->hide();

      ui->typeCombo->addItem(MyMoneyAccount::accountTypeToString(Account::Type::Income), (int)Account::Type::Income);
      ui->typeCombo->addItem(MyMoneyAccount::accountTypeToString(Account::Type::Expense), (int)Account::Type::Expense);

      // Hardcoded but acceptable - if above we set the default to income do the same here
      switch (m_account.accountType()) {
        case Account::Type::Expense:
          ui->typeCombo->setCurrentItem(MyMoneyAccount::accountTypeToString(Account::Type::Expense), false);
          break;

        case Account::Type::Income:
        default:
          ui->typeCombo->setCurrentItem(MyMoneyAccount::accountTypeToString(Account::Type::Income), false);
          break;
      }
      ui->m_currency->setEnabled(true);
      if (m_isEditing) {
        ui->typeCombo->setEnabled(false);
        ui->m_currency->setDisabled(MyMoneyFile::instance()->isReferenced(m_account));
      }
      ui->m_qcheckboxPreferred->hide();

      ui->m_qcheckboxTax->setChecked(m_account.value("Tax").toLower() == "yes");
      ui->m_costCenterRequiredCheckBox->setChecked(m_account.isCostCenterRequired());

      loadVatAccounts();
    } else {
      // get rid of the tabs that are not used for accounts
      int taxtab = ui->m_tab->indexOf(ui->m_taxTab);
      if (taxtab != -1) {
          ui->m_vatCategory->setText(i18n("VAT account"));
          ui->m_qcheckboxTax->setChecked(m_account.value("Tax") == "Yes");
          loadVatAccounts();
      } else {
          ui->m_tab->removeTab(taxtab);
      }

      ui->m_costCenterRequiredCheckBox->hide();

      switch (m_account.accountType()) {
        case Account::Type::Savings:
        case Account::Type::Cash:
          haveMinBalance = true;
          break;

        case Account::Type::Checkings:
          haveMinBalance = true;
          haveMaxCredit = true;
          break;

        case Account::Type::CreditCard:
          haveMaxCredit = true;
          break;

        default:
          // no limit available, so we might get rid of the tab
          int tab = ui->m_tab->indexOf(ui->m_limitsTab);
          if (tab != -1)
            ui->m_tab->removeTab(tab);
          // don't try to hide the widgets we just wiped
          // in the next step
          haveMaxCredit = haveMinBalance = true;
          break;
      }

      if (!haveMaxCredit) {
        ui->m_maxCreditLabel->setEnabled(false);
        ui->m_maxCreditLabel->hide();
        ui->m_maxCreditEarlyEdit->hide();
        ui->m_maxCreditAbsoluteEdit->hide();
      }
      if (!haveMinBalance) {
        ui->m_minBalanceLabel->setEnabled(false);
        ui->m_minBalanceLabel->hide();
        ui->m_minBalanceEarlyEdit->hide();
        ui->m_minBalanceAbsoluteEdit->hide();
      }

      QString typeString = MyMoneyAccount::accountTypeToString(m_account.accountType());

      if (m_isEditing) {
        if (m_account.isLiquidAsset()) {
          ui->typeCombo->addItem(MyMoneyAccount::accountTypeToString(Account::Type::Checkings), (int)Account::Type::Checkings);
          ui->typeCombo->addItem(MyMoneyAccount::accountTypeToString(Account::Type::Savings), (int)Account::Type::Savings);
          ui->typeCombo->addItem(MyMoneyAccount::accountTypeToString(Account::Type::Cash), (int)Account::Type::Cash);
        } else {
          ui->typeCombo->addItem(typeString, (int)m_account.accountType());
          // Once created, accounts of other account types are not
          // allowed to be changed.
          ui->typeCombo->setEnabled(false);
        }
        // Once created, a currency cannot be changed if it is referenced.
        ui->m_currency->setDisabled(MyMoneyFile::instance()->isReferenced(m_account));
      } else {
        ui->typeCombo->addItem(MyMoneyAccount::accountTypeToString(Account::Type::Checkings), (int)Account::Type::Checkings);
        ui->typeCombo->addItem(MyMoneyAccount::accountTypeToString(Account::Type::Savings), (int)Account::Type::Savings);
        ui->typeCombo->addItem(MyMoneyAccount::accountTypeToString(Account::Type::Cash), (int)Account::Type::Cash);
        ui->typeCombo->addItem(MyMoneyAccount::accountTypeToString(Account::Type::CreditCard), (int)Account::Type::CreditCard);
        ui->typeCombo->addItem(MyMoneyAccount::accountTypeToString(Account::Type::Loan), (int)Account::Type::Loan);
        ui->typeCombo->addItem(MyMoneyAccount::accountTypeToString(Account::Type::Investment), (int)Account::Type::Investment);
        ui->typeCombo->addItem(MyMoneyAccount::accountTypeToString(Account::Type::Asset), (int)Account::Type::Asset);
        ui->typeCombo->addItem(MyMoneyAccount::accountTypeToString(Account::Type::Liability), (int)Account::Type::Liability);
        ui->typeCombo->addItem(MyMoneyAccount::accountTypeToString(Account::Type::Stock), (int)Account::Type::Stock);
        /*
        ui->typeCombo->addItem(MyMoneyAccount::accountTypeToString(Account::Type::CertificateDep), (int)Account::Type::CertificateDep);
        ui->typeCombo->addItem(MyMoneyAccount::accountTypeToString(Account::Type::MoneyMarket), (int)Account::Type::MoneyMarket);
        ui->typeCombo->addItem(MyMoneyAccount::accountTypeToString(Account::Type::Currency), (int)Account::Type::Currency);
        */
        // Do not create account types that are not supported
        // by the current engine.
        if (m_account.accountType() == Account::Type::Unknown ||
            m_account.accountType() == Account::Type::CertificateDep ||
            m_account.accountType() == Account::Type::MoneyMarket ||
            m_account.accountType() == Account::Type::Currency)
          typeString = MyMoneyAccount::accountTypeToString(Account::Type::Checkings);
      }

      ui->typeCombo->setCurrentItem(typeString, false);

      if (m_account.isInvest())
        ui->m_institutionBox->hide();

      ui->accountNoEdit->setText(m_account.number());
      ui->m_qcheckboxPreferred->setChecked(m_account.value("PreferredAccount") == "Yes");
      ui->m_qcheckboxNoVat->setChecked(m_account.value("NoVat") == "Yes");
      loadKVP("iban", ui->ibanEdit);
      loadKVP("minBalanceAbsolute", ui->m_minBalanceAbsoluteEdit);
      loadKVP("minBalanceEarly", ui->m_minBalanceEarlyEdit);
      loadKVP("maxCreditAbsolute", ui->m_maxCreditAbsoluteEdit);
      loadKVP("maxCreditEarly", ui->m_maxCreditEarlyEdit);
      // reverse the sign for display purposes
      if (!ui->m_maxCreditAbsoluteEdit->text().isEmpty())
        ui->m_maxCreditAbsoluteEdit->setValue(ui->m_maxCreditAbsoluteEdit->value()*MyMoneyMoney::MINUS_ONE);
      if (!ui->m_maxCreditEarlyEdit->text().isEmpty())
        ui->m_maxCreditEarlyEdit->setValue(ui->m_maxCreditEarlyEdit->value()*MyMoneyMoney::MINUS_ONE);
      loadKVP("lastNumberUsed", ui->m_lastCheckNumberUsed);

      if (m_account.isInvest()) {
        ui->typeCombo->setEnabled(false);
        ui->m_qcheckboxPreferred->hide();
        ui->m_currencyText->hide();
        ui->m_currency->hide();
      } else {
        // use the old field and override a possible new value
        if (!MyMoneyMoney(m_account.value("minimumBalance")).isZero()) {
          ui->m_minBalanceAbsoluteEdit->setValue(MyMoneyMoney(m_account.value("minimumBalance")));
        }
      }

  //    ui->m_qcheckboxTax->hide(); TODO should only be visible for VAT category/account
    }

    ui->m_currency->setSecurity(file->currency(m_account.currencyId()));

    // Load the institutions
    // then the accounts
    QString institutionName;

    try {
      if (m_isEditing && !m_account.institutionId().isEmpty())
        institutionName = file->institution(m_account.institutionId()).name();
      else
        institutionName.clear();
    } catch (const MyMoneyException &e) {
      qDebug("exception in init for account dialog: %s", e.what());
    }

    if (m_account.isInvest())
      ui->m_parentAccounts->setEnabled(false);

    if (!m_categoryEditor)
      q->slotLoadInstitutions(institutionName);

    ui->accountNameEdit->setFocus();

    q->connect(ui->buttonBox, &QDialogButtonBox::rejected, q, &QDialog::reject);
    q->connect(ui->buttonBox, &QDialogButtonBox::accepted, q, &KNewAccountDlg::okClicked);
    q->connect(ui->m_qbuttonNew, &QAbstractButton::clicked, q, &KNewAccountDlg::slotNewClicked);
    q->connect(ui->typeCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), q, &KNewAccountDlg::slotAccountTypeChanged);

    q->connect(ui->accountNameEdit, &QLineEdit::textChanged, q, &KNewAccountDlg::slotCheckFinished);

    q->connect(ui->m_vatCategory,   &QAbstractButton::toggled,       q, &KNewAccountDlg::slotVatChanged);
    q->connect(ui->m_vatAssignment, &QAbstractButton::toggled,       q, &KNewAccountDlg::slotVatAssignmentChanged);
    q->connect(ui->m_vatCategory,   &QAbstractButton::toggled,       q, &KNewAccountDlg::slotCheckFinished);
    q->connect(ui->m_vatAssignment, &QAbstractButton::toggled,       q, &KNewAccountDlg::slotCheckFinished);
    q->connect(ui->m_vatRate,       &AmountEdit::textChanged,      q, &KNewAccountDlg::slotCheckFinished);
    q->connect(ui->m_vatAccount,    &KMyMoneySelector::stateChanged, q, &KNewAccountDlg::slotCheckFinished);
    q->connect(ui->m_currency, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), q, &KNewAccountDlg::slotCheckCurrency);

    q->connect(ui->m_minBalanceEarlyEdit,     &AmountEdit::valueChanged, q, &KNewAccountDlg::slotAdjustMinBalanceAbsoluteEdit);
    q->connect(ui->m_minBalanceAbsoluteEdit,  &AmountEdit::valueChanged, q, &KNewAccountDlg::slotAdjustMinBalanceEarlyEdit);
    q->connect(ui->m_maxCreditEarlyEdit,      &AmountEdit::valueChanged, q, &KNewAccountDlg::slotAdjustMaxCreditAbsoluteEdit);
    q->connect(ui->m_maxCreditAbsoluteEdit,   &AmountEdit::valueChanged, q, &KNewAccountDlg::slotAdjustMaxCreditEarlyEdit);

    q->connect(ui->m_qcomboboxInstitutions, static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::activated), q, &KNewAccountDlg::slotLoadInstitutions);

    ui->m_vatCategory->setChecked(false);
    ui->m_vatAssignment->setChecked(false);

    // make sure our account does not have an id and no parent assigned
    // and certainly no children in case we create a new account
    if (!m_isEditing) {
      m_account.clearId();
      m_account.setParentAccountId(QString());
      m_account.removeAccountIds();
    } else {
      if (!m_account.value("VatRate").isEmpty()) {
        ui->m_vatCategory->setChecked(true);
        ui->m_vatRate->setValue(MyMoneyMoney(m_account.value("VatRate"))*MyMoneyMoney(100, 1));
      } else {
        if (!m_account.value("VatAccount").isEmpty()) {
          QString accId = m_account.value("VatAccount").toLatin1();
          try {
            // make sure account exists
            MyMoneyFile::instance()->account(accId);
            ui->m_vatAssignment->setChecked(true);
            ui->m_vatAccount->setSelected(accId);
            ui->m_grossAmount->setChecked(true);
            if (m_account.value("VatAmount") == "Net")
              ui->m_netAmount->setChecked(true);
          } catch (const MyMoneyException &) {
          }
        }
      }
    }
    q->slotVatChanged(ui->m_vatCategory->isChecked());
    q->slotVatAssignmentChanged(ui->m_vatAssignment->isChecked());
    q->slotCheckFinished();

    auto requiredFields = new KMandatoryFieldGroup(q);
    requiredFields->setOkButton(ui->buttonBox->button(QDialogButtonBox::Ok)); // button to be enabled when all fields present
    requiredFields->add(ui->accountNameEdit);
  }

  void loadKVP(const QString& key, AmountEdit* widget)
  {
    if (!widget)
      return;

    if (m_account.value(key).isEmpty()) {
      widget->setText(QString());
    } else {
      widget->setValue(MyMoneyMoney(m_account.value(key)));
    }
  }

  void loadKVP(const QString& key, KLineEdit* widget)
  {
    if (!widget)
      return;

    widget->setText(m_account.value(key));
  }

  void storeKVP(const QString& key, const QString& text, const QString& value)
  {
    if (text.isEmpty())
      m_account.deletePair(key);
    else
      m_account.setValue(key, value);
  }

  void storeKVP(const QString& key, QCheckBox* widget)
  {
    if (widget) {
      if(widget->isChecked()) {
        m_account.setValue(key, "Yes");;
      } else {
        m_account.deletePair(key);
      }
    }
  }

  void storeKVP(const QString& key, AmountEdit* widget)
  {
    storeKVP(key, widget->text(), widget->value().toString());
  }

  void storeKVP(const QString& key, KLineEdit* widget)
  {
    storeKVP(key, widget->text(), widget->text());
  }

  void loadVatAccounts()
  {
    QList<MyMoneyAccount> list;
    MyMoneyFile::instance()->accountList(list);
    QList<MyMoneyAccount>::Iterator it;
    QStringList loadListExpense;
    QStringList loadListIncome;
    QStringList loadListAsset;
    QStringList loadListLiability;
    for (it = list.begin(); it != list.end(); ++it) {
      if (!(*it).value("VatRate").isEmpty()) {
        if ((*it).accountType() == Account::Type::Expense)
          loadListExpense += (*it).id();
        else if ((*it).accountType() == Account::Type::Income)
          loadListIncome += (*it).id();
        else if ((*it).accountType() == Account::Type::Asset)
          loadListAsset += (*it).id();
        else if ((*it).accountType() == Account::Type::Liability)
          loadListLiability += (*it).id();
      }
    }
    AccountSet vatSet;
    if (!loadListAsset.isEmpty())
      vatSet.load(ui->m_vatAccount, i18n("Asset"), loadListAsset, true);
    if (!loadListLiability.isEmpty())
      vatSet.load(ui->m_vatAccount, i18n("Liability"), loadListLiability, false);
    if (!loadListIncome.isEmpty())
      vatSet.load(ui->m_vatAccount, i18n("Income"), loadListIncome, false);
    if (!loadListExpense.isEmpty())
      vatSet.load(ui->m_vatAccount, i18n("Expense"), loadListExpense, false);
  }

  void adjustEditWidgets(AmountEdit* dst, AmountEdit* src, char mode, int corr)
  {
    MyMoneyMoney factor(corr, 1);
    if (m_account.accountGroup() == Account::Type::Asset)
      factor = -factor;

    switch (mode) {
      case '<':
        if (src->value()*factor < dst->value()*factor)
          dst->setValue(src->value());
        break;

      case '>':
        if (src->value()*factor > dst->value()*factor)
          dst->setValue(src->value());
        break;
    }
  }

  void handleOpeningBalanceCheckbox(const QString &currencyId)
  {
    if (m_account.accountType() == Account::Type::Equity) {
      // check if there is another opening balance account with the same currency
      bool isOtherOpenBalancingAccount = false;
      QList<MyMoneyAccount> list;
      MyMoneyFile::instance()->accountList(list);
      QList<MyMoneyAccount>::Iterator it;
      for (it = list.begin(); it != list.end(); ++it) {
        if (it->id() == m_account.id() || currencyId != it->currencyId()
            || it->accountType() != Account::Type::Equity)
          continue;
        if (it->value("OpeningBalanceAccount") == "Yes") {
          isOtherOpenBalancingAccount = true;
          break;
        }
      }
      if (!isOtherOpenBalancingAccount) {
        bool isOpenBalancingAccount = m_account.value("OpeningBalanceAccount") == "Yes";
        ui->m_qcheckboxOpeningBalance->setChecked(isOpenBalancingAccount);
        if (isOpenBalancingAccount) {
          // let only allow state change if no transactions are assigned to this account
          bool hasTransactions = MyMoneyFile::instance()->transactionCount(m_account.id()) != 0;
          ui->m_qcheckboxOpeningBalance->setEnabled(!hasTransactions);
          if (hasTransactions)
            ui->m_qcheckboxOpeningBalance->setToolTip(i18n("Option has been disabled because there are transactions assigned to this account"));
        }
      } else {
        ui->m_qcheckboxOpeningBalance->setChecked(false);
        ui->m_qcheckboxOpeningBalance->setEnabled(false);
        ui->m_qcheckboxOpeningBalance->setToolTip(i18n("Option has been disabled because there is another account flagged to be an opening balance account for this currency"));
      }
    } else {
      ui->m_qcheckboxOpeningBalance->setVisible(false);
    }
  }

  KNewAccountDlg*             q_ptr;
  Ui::KNewAccountDlg*         ui;
  MyMoneyAccount              m_account;
  MyMoneyAccount              m_parentAccount;
  AccountsProxyModel*         m_filterProxyModel;

  bool m_categoryEditor;
  bool m_isEditing;
};

KNewAccountDlg::KNewAccountDlg(const MyMoneyAccount& account, bool isEditing, bool categoryEditor, QWidget *parent, const QString& title) :
  QDialog(parent),
  d_ptr(new KNewAccountDlgPrivate(this))
{
  Q_D(KNewAccountDlg);
  d->m_account = account;
  d->m_categoryEditor = categoryEditor;
  d->m_isEditing = isEditing;
  d->init();
  if (!title.isEmpty())
    setWindowTitle(title);
}

MyMoneyMoney KNewAccountDlg::openingBalance() const
{
  Q_D(const KNewAccountDlg);
  return d->ui->m_openingBalanceEdit->value();
}

void KNewAccountDlg::setOpeningBalance(const MyMoneyMoney& balance)
{
  Q_D(KNewAccountDlg);
  d->ui->m_openingBalanceEdit->setValue(balance);
}

void KNewAccountDlg::setOpeningBalanceShown(bool shown)
{
  Q_D(KNewAccountDlg);
  d->ui->m_openingBalanceLabel->setVisible(shown);
  d->ui->m_openingBalanceEdit->setVisible(shown);
}

void KNewAccountDlg::setOpeningDateShown(bool shown)
{
  Q_D(KNewAccountDlg);
  d->ui->m_openingDateLabel->setVisible(shown);
  d->ui->m_openingDateEdit->setVisible(shown);
}

void KNewAccountDlg::okClicked()
{
  Q_D(KNewAccountDlg);
  auto file = MyMoneyFile::instance();

  QString accountNameText = d->ui->accountNameEdit->text();
  if (accountNameText.isEmpty()) {
    KMessageBox::error(this, i18n("You have not specified a name.\nPlease fill in this field."));
    d->ui->accountNameEdit->setFocus();
    return;
  }

  MyMoneyAccount parent = parentAccount();
  if (parent.name().length() == 0) {
    KMessageBox::error(this, i18n("Please select a parent account."));
    return;
  }

  if (!d->m_categoryEditor) {
    QString institutionNameText = d->ui->m_qcomboboxInstitutions->currentText();
    if (institutionNameText != i18n("(No Institution)")) {
      try {
        QList<MyMoneyInstitution> list = file->institutionList();
        QList<MyMoneyInstitution>::ConstIterator institutionIterator;
        for (institutionIterator = list.constBegin(); institutionIterator != list.constEnd(); ++institutionIterator) {
          if ((*institutionIterator).name() == institutionNameText)
            d->m_account.setInstitutionId((*institutionIterator).id());
        }
      } catch (const MyMoneyException &e) {
        qDebug("Exception in account institution set: %s", e.what());
      }
    } else {
      d->m_account.setInstitutionId(QString());
    }
  }

  d->m_account.setName(accountNameText);
  d->m_account.setNumber(d->ui->accountNoEdit->text());
  d->storeKVP("iban", d->ui->ibanEdit);
  d->storeKVP("minBalanceAbsolute", d->ui->m_minBalanceAbsoluteEdit);
  d->storeKVP("minBalanceEarly", d->ui->m_minBalanceEarlyEdit);

  // the figures for credit line with reversed sign
  if (!d->ui->m_maxCreditAbsoluteEdit->text().isEmpty())
    d->ui->m_maxCreditAbsoluteEdit->setValue(d->ui->m_maxCreditAbsoluteEdit->value()*MyMoneyMoney::MINUS_ONE);
  if (!d->ui->m_maxCreditEarlyEdit->text().isEmpty())
    d->ui->m_maxCreditEarlyEdit->setValue(d->ui->m_maxCreditEarlyEdit->value()*MyMoneyMoney::MINUS_ONE);
  d->storeKVP("maxCreditAbsolute", d->ui->m_maxCreditAbsoluteEdit);
  d->storeKVP("maxCreditEarly", d->ui->m_maxCreditEarlyEdit);
  if (!d->ui->m_maxCreditAbsoluteEdit->text().isEmpty())
    d->ui->m_maxCreditAbsoluteEdit->setValue(d->ui->m_maxCreditAbsoluteEdit->value()*MyMoneyMoney::MINUS_ONE);
  if (!d->ui->m_maxCreditEarlyEdit->text().isEmpty())
    d->ui->m_maxCreditEarlyEdit->setValue(d->ui->m_maxCreditEarlyEdit->value()*MyMoneyMoney::MINUS_ONE);

  d->storeKVP("lastNumberUsed", d->ui->m_lastCheckNumberUsed);
  // delete a previous version of the minimumbalance information
  d->storeKVP("minimumBalance", QString(), QString());

  Account::Type acctype;
  if (!d->m_categoryEditor) {
    acctype = static_cast<Account::Type>(d->ui->typeCombo->currentData().toInt());
    // If it's a loan, check if the parent is asset or liability. In
    // case of asset, we change the account type to be AssetLoan
    if (acctype == Account::Type::Loan
        && parent.accountGroup() == Account::Type::Asset)
      acctype = Account::Type::AssetLoan;
  } else {
    acctype = parent.accountGroup();
    QString newName;
    if (!MyMoneyFile::instance()->isStandardAccount(parent.id())) {
      newName = MyMoneyFile::instance()->accountToCategory(parent.id()) + MyMoneyFile::AccountSeparator;
    }
    newName += accountNameText;
    if (!file->categoryToAccount(newName, acctype).isEmpty()
        && (file->categoryToAccount(newName, acctype) != d->m_account.id())) {
      KMessageBox::error(this, QString("<qt>") + i18n("A category named <b>%1</b> already exists. You cannot create a second category with the same name.", newName) + QString("</qt>"));
      return;
    }
  }
  d->m_account.setAccountType(acctype);

  d->m_account.setDescription(d->ui->descriptionEdit->toPlainText());

  d->m_account.setOpeningDate(d->ui->m_openingDateEdit->date());

  if (!d->m_categoryEditor) {
    d->m_account.setCurrencyId(d->ui->m_currency->security().id());

    d->storeKVP("PreferredAccount", d->ui->m_qcheckboxPreferred);
    d->storeKVP("NoVat", d->ui->m_qcheckboxNoVat);

    if (d->ui->m_minBalanceAbsoluteEdit->isVisible()) {
      d->m_account.setValue("minimumBalance", d->ui->m_minBalanceAbsoluteEdit->value().toString());
    }
  } else {
    if (KMyMoneySettings::hideUnusedCategory() && !d->m_isEditing) {
      KMessageBox::information(this, i18n("You have selected to suppress the display of unused categories in the KMyMoney configuration dialog. The category you just created will therefore only be shown if it is used. Otherwise, it will be hidden in the accounts/categories view."), i18n("Hidden categories"), "NewHiddenCategory");
    }
    d->m_account.setCostCenterRequired(d->ui->m_costCenterRequiredCheckBox->isChecked());
  }

  d->storeKVP("Tax", d->ui->m_qcheckboxTax);

  if (d->ui->m_qcheckboxOpeningBalance->isChecked())
    d->m_account.setValue("OpeningBalanceAccount", "Yes");
  else
    d->m_account.deletePair("OpeningBalanceAccount");

  d->m_account.deletePair("VatAccount");
  d->m_account.deletePair("VatAmount");
  d->m_account.deletePair("VatRate");

  if (d->ui->m_vatCategory->isChecked()) {
    d->m_account.setValue("VatRate", (d->ui->m_vatRate->value().abs() / MyMoneyMoney(100, 1)).toString());
  } else {
    if (d->ui->m_vatAssignment->isChecked() && !d->ui->m_vatAccount->selectedItems().isEmpty()) {
      d->m_account.setValue("VatAccount", d->ui->m_vatAccount->selectedItems().first());
      if (d->ui->m_netAmount->isChecked())
        d->m_account.setValue("VatAmount", "Net");
    }
  }

  accept();
}


MyMoneyAccount KNewAccountDlg::account()
{
  Q_D(KNewAccountDlg);
  // assign the right currency to the account
  d->m_account.setCurrencyId(d->ui->m_currency->security().id());

  // and the price mode
  switch (d->ui->m_priceMode->currentItem()) {
    case 0:
      d->m_account.deletePair("priceMode");
      break;
    case 1:
    case 2:
      d->m_account.setValue("priceMode", QString("%1").arg(d->ui->m_priceMode->currentItem()));
      break;
  }

  return d->m_account;
}

MyMoneyAccount KNewAccountDlg::parentAccount() const
{
  Q_D(const KNewAccountDlg);
  return d->m_parentAccount;
}

void KNewAccountDlg::slotSelectionChanged(const QItemSelection &current, const QItemSelection &previous)
{
  Q_UNUSED(previous)
  Q_D(KNewAccountDlg);
  if (!current.indexes().empty()) {
    auto baseIdx = MyMoneyFile::baseModel()->mapToBaseSource(current.indexes().front());
    if (baseIdx.isValid()) {
      d->m_parentAccount = MyMoneyFile::instance()->accountsModel()->itemByIndex(baseIdx);
      d->ui->m_subAccountLabel->setText(i18n("Is a sub account of %1", d->m_parentAccount.name()));
    }
  }
}

void KNewAccountDlg::slotLoadInstitutions(const QString& name)
{
  Q_D(KNewAccountDlg);
  d->ui->m_qcomboboxInstitutions->model()->deleteLater();

  auto model = new QStringListModel(this);
  auto list = MyMoneyFile::instance()->institutionList();
  QStringList names;

  d->ui->m_bicValue->setText(" ");
  d->ui->ibanEdit->setEnabled(false);
  d->ui->accountNoEdit->setEnabled(false);

  QString search(i18n("(No Institution)"));

  for (const auto institution : qAsConst(list)) {
    names << institution.name();
    if (institution.name() == name) {
      d->ui->ibanEdit->setEnabled(true);
      d->ui->accountNoEdit->setEnabled(true);
      d->ui->m_bicValue->setText(institution.value("bic"));
      search = name;
    }
  }
  model->setStringList(names);
  model->sort(0);
  model->insertRow(0);
  QModelIndex idx = model->index(0, 0);
  model->setData(idx, i18n("(No Institution)"), Qt::DisplayRole);

  d->ui->m_qcomboboxInstitutions->setModel(model);

  auto i = d->ui->m_qcomboboxInstitutions->findText(search);
  if (i == -1)
    i = 0;
  d->ui->m_qcomboboxInstitutions->setCurrentIndex(i);
}

void KNewAccountDlg::slotNewClicked()
{
  MyMoneyInstitution institution;

  QPointer<KNewInstitutionDlg> dlg = new KNewInstitutionDlg(institution, this);
  if (dlg->exec()) {
    MyMoneyFileTransaction ft;
    try {
      auto file = MyMoneyFile::instance();

      institution = dlg->institution();
      file->addInstitution(institution);
      ft.commit();
      slotLoadInstitutions(institution.name());
    } catch (const MyMoneyException &) {
      KMessageBox::information(this, i18n("Cannot add institution"));
    }
  }
  delete dlg;
}

void KNewAccountDlg::slotAccountTypeChanged(int index)
{
  Q_D(KNewAccountDlg);
  Account::Type oldType;

  auto type = d->ui->typeCombo->itemData(index).value<Account::Type>();
  try {
    oldType = d->m_account.accountType();
    if (oldType != type) {
      d->m_account.setAccountType(type);
      // update the account group displayed in the accounts hierarchy
      d->m_filterProxyModel->clear();
      d->m_filterProxyModel->addAccountGroup(QVector<Account::Type> {d->m_account.accountGroup()});
    }
  } catch (const MyMoneyException &) {
    qWarning("Unexpected exception in KNewAccountDlg::slotAccountTypeChanged()");
  }
}

void KNewAccountDlg::slotCheckFinished()
{
  Q_D(KNewAccountDlg);
  auto showButton = true;

  if (d->ui->accountNameEdit->text().length() == 0) {
    showButton = false;
  }

  if (d->ui->m_vatCategory->isChecked() && d->ui->m_vatRate->value() <= MyMoneyMoney()) {
    showButton = false;
  } else {
    if (d->ui->m_vatAssignment->isChecked() && d->ui->m_vatAccount->selectedItems().isEmpty())
      showButton = false;
  }
  d->ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(showButton);
}

void KNewAccountDlg::slotVatChanged(bool state)
{
  Q_D(KNewAccountDlg);
  if (state) {
    d->ui->m_vatCategoryFrame->show();
    d->ui->m_vatAssignmentFrame->hide();
  } else {
    d->ui->m_vatCategoryFrame->hide();
    if (!d->m_account.isAssetLiability()) {
      d->ui->m_vatAssignmentFrame->show();
    }
  }
}

void KNewAccountDlg::slotVatAssignmentChanged(bool state)
{
  Q_D(KNewAccountDlg);
  d->ui->m_vatAccount->setEnabled(state);
  d->ui->m_amountGroupBox->setEnabled(state);
}

void KNewAccountDlg::slotAdjustMinBalanceAbsoluteEdit(const QString&)
{
  Q_D(KNewAccountDlg);
  d->adjustEditWidgets(d->ui->m_minBalanceAbsoluteEdit, d->ui->m_minBalanceEarlyEdit, '<', -1);
}

void KNewAccountDlg::slotAdjustMinBalanceEarlyEdit(const QString&)
{
  Q_D(KNewAccountDlg);
  d->adjustEditWidgets(d->ui->m_minBalanceEarlyEdit, d->ui->m_minBalanceAbsoluteEdit, '>', -1);
}

void KNewAccountDlg::slotAdjustMaxCreditAbsoluteEdit(const QString&)
{
  Q_D(KNewAccountDlg);
  d->adjustEditWidgets(d->ui->m_maxCreditAbsoluteEdit, d->ui->m_maxCreditEarlyEdit, '>', 1);
}

void KNewAccountDlg::slotAdjustMaxCreditEarlyEdit(const QString&)
{
  Q_D(KNewAccountDlg);
  d->adjustEditWidgets(d->ui->m_maxCreditEarlyEdit, d->ui->m_maxCreditAbsoluteEdit, '<', 1);
}

void KNewAccountDlg::slotCheckCurrency(int index)
{
  Q_D(KNewAccountDlg);
  Q_UNUSED(index)
  d->handleOpeningBalanceCheckbox(d->ui->m_currency->security().id());
}

void KNewAccountDlg::addTab(QWidget* w, const QString& name)
{
  Q_D(KNewAccountDlg);
  if (w) {
    w->setParent(d->ui->m_tab);
    d->ui->m_tab->addTab(w, name);
  }
}

void KNewAccountDlg::newCategory(MyMoneyAccount& account, const MyMoneyAccount& parent)
{
  if (KMessageBox::questionYesNo(nullptr,
                                 QString::fromLatin1("<qt>%1</qt>").arg(i18n("<p>The category <b>%1</b> currently does not exist. Do you want to create it?</p><p><i>The parent account will default to <b>%2</b> but can be changed in the following dialog</i>.</p>", account.name(), parent.name())), i18n("Create category"),
                                 KStandardGuiItem::yes(), KStandardGuiItem::no(), "CreateNewCategories") == KMessageBox::Yes) {
    KNewAccountDlg::createCategory(account, parent);
  } else {
    // we should not keep the 'no' setting because that can confuse people like
    // I have seen in some usability tests. So we just delete it right away.
    KSharedConfigPtr kconfig = KSharedConfig::openConfig();
    if (kconfig) {
      kconfig->group(QLatin1String("Notification Messages")).deleteEntry(QLatin1String("CreateNewCategories"));
    }
  }
}

void KNewAccountDlg::createCategory(MyMoneyAccount& account, const MyMoneyAccount& parent)
{
  if (!parent.id().isEmpty()) {
    try {
      // make sure parent account exists
      MyMoneyFile::instance()->account(parent.id());
      account.setParentAccountId(parent.id());
      account.setAccountType(parent.accountType());
    } catch (const MyMoneyException &) {
    }
  }

  QPointer<KNewAccountDlg> dialog =
    new KNewAccountDlg(account, false, true, 0, i18n("Create a new Category"));

  dialog->setOpeningBalanceShown(false);
  dialog->setOpeningDateShown(false);

  if (dialog->exec() == QDialog::Accepted && dialog != 0) {
    MyMoneyAccount parentAccount, brokerageAccount;
    account = dialog->account();
    parentAccount = dialog->parentAccount();

    MyMoneyFile::instance()->createAccount(account, parentAccount, brokerageAccount, MyMoneyMoney());
  }
  delete dialog;
}
