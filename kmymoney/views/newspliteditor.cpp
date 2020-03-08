/*
 * Copyright 2016-2019  Thomas Baumgart <tbaumgart@kde.org>
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


#include "newspliteditor.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QCompleter>
#include <QSortFilterProxyModel>
#include <QStringList>
#include <QDate>
#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KConcatenateRowsProxyModel>

// ----------------------------------------------------------------------------
// Project Includes

#include "creditdebithelper.h"
#include "kmymoneyutils.h"
#include "kmymoneyaccountcombo.h"
#include "mymoneyfile.h"
#include "accountsmodel.h"
#include "costcentermodel.h"
#include "ledgermodel.h"
#include "splitmodel.h"
#include "payeesmodel.h"
#include "mymoneyaccount.h"
#include "mymoneyexception.h"
#include "ui_newspliteditor.h"
#include "widgethintframe.h"
#include "splitview.h"
#include "icons/icons.h"
#include "mymoneyenums.h"
#include "modelenums.h"
#include "mymoneysecurity.h"
#include "kcurrencycalculator.h"
#include "amounteditcurrencyhelper.h"

using namespace Icons;

struct NewSplitEditor::Private
{
  Private(NewSplitEditor* parent)
  : q(parent)
  , ui(new Ui_NewSplitEditor)
  , accountsModel(new AccountNamesFilterProxyModel(parent))
  , payeesModel(new QSortFilterProxyModel(parent))
  , costCenterModel(new QSortFilterProxyModel(parent))
  , splitModel(0)
  , accepted(false)
  , costCenterRequired(false)
  , showValuesInverted(false)
  , haveShares(false)
  , loadingSplit(false)
  , postDate(QDate::currentDate())
  , amountHelper(nullptr)
  {
    accountsModel->setObjectName("AccountNamesFilterProxyModel");
    costCenterModel->setObjectName("SortedCostCenterModel");
    payeesModel->setObjectName("SortedPayeesModel");

    costCenterModel->setSortLocaleAware(true);
    costCenterModel->setSortCaseSensitivity(Qt::CaseInsensitive);

    payeesModel->setSortLocaleAware(true);
    payeesModel->setSortCaseSensitivity(Qt::CaseInsensitive);
  }

  ~Private()
  {
    delete ui;
  }

  bool checkForValidSplit(bool doUserInteraction = true);

  bool costCenterChanged(int costCenterIndex);
  bool categoryChanged(const QString& accountId);
  bool numberChanged(const QString& newNumber);
  bool amountChanged(CreditDebitHelper* valueHelper);

  void checkMultiCurrency();

  NewSplitEditor*               q;
  Ui_NewSplitEditor*            ui;
  AccountNamesFilterProxyModel* accountsModel;
  QSortFilterProxyModel*        payeesModel;
  QSortFilterProxyModel*        costCenterModel;
  SplitModel*                   splitModel;
  bool                          accepted;
  bool                          costCenterRequired;
  bool                          showValuesInverted;
  bool                          haveShares;
  bool                          loadingSplit;
  MyMoneyAccount                counterAccount;
  MyMoneyAccount                category;
  MyMoneySecurity               commodity;
  MyMoneyMoney                  shares;
  MyMoneyMoney                  price;
  QDate                         postDate;
  CreditDebitHelper*            amountHelper;
};

bool NewSplitEditor::Private::checkForValidSplit(bool doUserInteraction)
{
  QStringList infos;
  bool rc = true;
  if(!costCenterChanged(ui->costCenterCombo->currentIndex())) {
    infos << ui->costCenterCombo->toolTip();
    rc = false;
  }

  if(doUserInteraction) {
    /// @todo add dialog here that shows the @a infos
  }
  return rc;
}

bool NewSplitEditor::Private::costCenterChanged(int costCenterIndex)
{
  bool rc = true;
  WidgetHintFrame::hide(ui->costCenterCombo, i18n("The cost center this transaction should be assigned to."));
  if(costCenterIndex != -1) {
    if(costCenterRequired && ui->costCenterCombo->currentText().isEmpty()) {
      WidgetHintFrame::show(ui->costCenterCombo, i18n("A cost center assignment is required for a transaction in the selected category."));
      rc = false;
    }
  }
  return rc;
}

bool NewSplitEditor::Private::categoryChanged(const QString& accountId)
{
  bool rc = true;
  if(!accountId.isEmpty()) {
    try {
      const auto model = MyMoneyFile::instance()->accountsModel();
      QModelIndex index = model->indexById(accountId);
      category = model->itemByIndex(index);
      const bool isIncomeExpense = category.isIncomeExpense();
      ui->costCenterCombo->setEnabled(isIncomeExpense);
      ui->costCenterLabel->setEnabled(isIncomeExpense);
      ui->numberEdit->setDisabled(isIncomeExpense);
      ui->numberLabel->setDisabled(isIncomeExpense);

      checkMultiCurrency();

      costCenterRequired = category.isCostCenterRequired();
      rc &= costCenterChanged(ui->costCenterCombo->currentIndex());
    } catch (MyMoneyException &e) {
      qDebug() << "Ooops: invalid account id" << accountId << "in" << Q_FUNC_INFO;
    }
  }
  return rc;
}

bool NewSplitEditor::Private::numberChanged(const QString& newNumber)
{
  bool rc = true;
  WidgetHintFrame::hide(ui->numberEdit, i18n("The check number used for this transaction."));
  if(!newNumber.isEmpty()) {
    /// @todo port to new model code
#if 0
    const LedgerModel* model = Models::instance()->ledgerModel();
    QModelIndexList list = model->match(model->index(0, 0), (int)eLedgerModel::Role::Number,
                                        QVariant(newNumber),
                                        -1,                         // all splits
                                        Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive | Qt::MatchRecursive));

    foreach(QModelIndex index, list) {
      if(model->data(index, (int)eLedgerModel::Role::AccountId) == ui->accountCombo->getSelected()
        && model->data(index, (int)eLedgerModel::Role::TransactionSplitId) != transactionSplitId) {
        WidgetHintFrame::show(ui->numberEdit, i18n("The check number <b>%1</b> has already been used in this account.", newNumber));
        rc = false;
        break;
      }
    }
#endif
  }
  return rc;
}

bool NewSplitEditor::Private::amountChanged(CreditDebitHelper* valueHelper)
{
  Q_UNUSED(valueHelper);
  if (valueHelper->haveValue()) {

  } else {

  }
  bool rc = true;
  checkMultiCurrency();
  return rc;
}

void NewSplitEditor::Private::checkMultiCurrency()
{
  // skip interaction during loading operation
  if (loadingSplit)
    return;

  const auto categoryId = q->accountId();
  auto const model = MyMoneyFile::instance()->accountsModel();
  auto account = model->itemById(categoryId);
  auto security = MyMoneyFile::instance()->security(account.currencyId());
  if (security.id() != commodity.id() && !amountHelper->value().isZero()) {
    QPointer<KCurrencyCalculator> calc =
    new KCurrencyCalculator(security,
                            commodity,
                            q->value() * price,
                            q->value(),
                            postDate,
                            security.smallestAccountFraction(),
                            q);

    if (calc->exec() == QDialog::Accepted && calc) {
      price = calc->price();
    }
    delete calc;

  } else {
    price = MyMoneyMoney::ONE;
  }
  shares = q->value() * price;
}


NewSplitEditor::NewSplitEditor(QWidget* parent, const MyMoneySecurity& commodity, const QString& counterAccountId)
  : QFrame(parent, Qt::FramelessWindowHint /* | Qt::X11BypassWindowManagerHint */)
  , d(new Private(this))
{
  d->commodity = commodity;
  auto const file = MyMoneyFile::instance();
  auto view = qobject_cast<SplitView*>(parent->parentWidget());
  Q_ASSERT(view != 0);
  d->splitModel = qobject_cast<SplitModel*>(view->model());

  auto const model = MyMoneyFile::instance()->accountsModel();
  d->counterAccount = model->itemById(counterAccountId);

  d->ui->setupUi(this);
  d->ui->enterButton->setIcon(Icons::get(Icon::DialogOK));
  d->ui->cancelButton->setIcon(Icons::get(Icon::DialogCancel));

  auto concatModel = new KConcatenateRowsProxyModel(parent);
  concatModel->addSourceModel(file->payeesModel()->emptyPayee());
  concatModel->addSourceModel(file->payeesModel());
  d->payeesModel->setSortRole(Qt::DisplayRole);
  d->payeesModel->setSourceModel(concatModel);
  d->payeesModel->sort(0);

  d->ui->payeeEdit->setEditable(true);
  d->ui->payeeEdit->setModel(d->payeesModel);
  d->ui->payeeEdit->setModelColumn(0);
  d->ui->payeeEdit->completer()->setFilterMode(Qt::MatchContains);

  d->accountsModel->addAccountGroup(QVector<eMyMoney::Account::Type> {eMyMoney::Account::Type::Asset, eMyMoney::Account::Type::Liability, eMyMoney::Account::Type::Income, eMyMoney::Account::Type::Expense, eMyMoney::Account::Type::Equity});
  d->accountsModel->setHideEquityAccounts(false);
  d->accountsModel->setSourceModel(model);
  d->accountsModel->sort(AccountsModel::Column::AccountName);
  d->ui->accountCombo->setModel(d->accountsModel);
  d->ui->accountCombo->setSplitActionVisible(false);

  d->costCenterModel->setSortRole(Qt::DisplayRole);
  d->costCenterModel->setSourceModel(MyMoneyFile::instance()->costCenterModel());
  d->costCenterModel->sort(AccountsModel::Column::AccountName);

  d->ui->costCenterCombo->setEditable(true);
  d->ui->costCenterCombo->setModel(d->costCenterModel);
  d->ui->costCenterCombo->setModelColumn(0);
  d->ui->costCenterCombo->completer()->setFilterMode(Qt::MatchContains);

  WidgetHintFrameCollection* frameCollection = new WidgetHintFrameCollection(this);
  frameCollection->addFrame(new WidgetHintFrame(d->ui->costCenterCombo));
  frameCollection->addFrame(new WidgetHintFrame(d->ui->numberEdit, WidgetHintFrame::Warning));
  frameCollection->addWidget(d->ui->enterButton);

  d->amountHelper = new CreditDebitHelper(this, d->ui->amountEditCredit, d->ui->amountEditDebit);

  new AmountEditCurrencyHelper(d->ui->accountCombo, d->amountHelper, commodity.id());

  connect(d->ui->numberEdit, SIGNAL(textChanged(QString)), this, SLOT(numberChanged(QString)));
  connect(d->ui->costCenterCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(costCenterChanged(int)));
  connect(d->ui->accountCombo, SIGNAL(accountSelected(QString)), this, SLOT(categoryChanged(QString)));
  connect(d->amountHelper, SIGNAL(valueChanged()), this, SLOT(amountChanged()));

  connect(d->ui->cancelButton, SIGNAL(clicked(bool)), this, SLOT(reject()));
  connect(d->ui->enterButton, SIGNAL(clicked(bool)), this, SLOT(acceptEdit()));
}

NewSplitEditor::~NewSplitEditor()
{
}

void NewSplitEditor::setPostDate(const QDate& date)
{
  d->postDate = date;
}

void NewSplitEditor::setShowValuesInverted(bool inverse)
{
  d->showValuesInverted = inverse;
}

bool NewSplitEditor::showValuesInverted()
{
  return d->showValuesInverted;
}

bool NewSplitEditor::accepted() const
{
  return d->accepted;
}

void NewSplitEditor::acceptEdit()
{
  if(d->checkForValidSplit()) {
    d->accepted = true;
    emit done();
  }
}

void NewSplitEditor::reject()
{
  emit done();
}

void NewSplitEditor::keyPressEvent(QKeyEvent* e)
{
  if (!e->modifiers() || (e->modifiers() & Qt::KeypadModifier && e->key() == Qt::Key_Enter)) {
    switch (e->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
      {
        if(focusWidget() == d->ui->cancelButton) {
          reject();
        } else {
          if(d->ui->enterButton->isEnabled()) {
            d->ui->enterButton->setFocus();
            d->ui->enterButton->click();
          }
          return;
        }
      }
      break;

    case Qt::Key_Escape:
      reject();
      break;

    default:
      e->ignore();
      return;
    }
  } else {
    e->ignore();
  }
}

QString NewSplitEditor::accountId() const
{
  return d->ui->accountCombo->getSelected();
}

void NewSplitEditor::setAccountId(const QString& id)
{
  d->ui->accountCombo->clearEditText();
  d->ui->accountCombo->setSelected(id);
}


QString NewSplitEditor::memo() const
{
  return d->ui->memoEdit->toPlainText();
}

void NewSplitEditor::setMemo(const QString& memo)
{
  d->ui->memoEdit->setPlainText(memo);
}

MyMoneyMoney NewSplitEditor::shares() const
{
  return d->shares;
}

void NewSplitEditor::setShares(const MyMoneyMoney& shares)
{
  d->shares = shares;
  d->haveShares = true;
}

MyMoneyMoney NewSplitEditor::value() const
{
  return d->amountHelper->value();
}

void NewSplitEditor::setValue(const MyMoneyMoney& value)
{
  d->price = MyMoneyMoney::ONE;
  if (!d->haveShares) {
    qDebug() << "NewSplitEditor::setValue(): call to setShares() missing, price invalid";
  } else if (!(d->shares.isZero() || value.isZero())) {
    d->price = d->shares/value;
  }
  d->amountHelper->setValue(value);
}


QString NewSplitEditor::costCenterId() const
{
  const int row = d->ui->costCenterCombo->currentIndex();
  QModelIndex index = d->ui->costCenterCombo->model()->index(row, 0);
  return d->ui->costCenterCombo->model()->data(index, eMyMoney::Model::Roles::IdRole).toString();
}

void NewSplitEditor::setCostCenterId(const QString& id)
{
  const auto baseIdx = MyMoneyFile::instance()->costCenterModel()->indexById(id);
  if (baseIdx.isValid()) {
    const auto index = CostCenterModel::mapFromBaseSource(d->costCenterModel, baseIdx);
    if(index.isValid()) {
      d->ui->costCenterCombo->setCurrentIndex(index.row());
    }
  }
}

QString NewSplitEditor::number() const
{
  return d->ui->numberEdit->text();
}

void NewSplitEditor::setNumber(const QString& number)
{
  d->ui->numberEdit->setText(number);
}

QString NewSplitEditor::payeeId() const
{
  const auto idx = d->payeesModel->index(d->ui->payeeEdit->currentIndex(), 0);
  return idx.data(eMyMoney::Model::IdRole).toString();
}

void NewSplitEditor::setPayeeId(const QString& id)
{
  QModelIndexList indexes = d->payeesModel->match(d->payeesModel->index(0, 0), eMyMoney::Model::IdRole, QVariant(id), 1, Qt::MatchFlags(Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive | Qt::MatchRecursive)));
  int row(0);
  if (!indexes.isEmpty()) {
    row = indexes.first().row();
  }
  d->ui->payeeEdit->setCurrentIndex(row);
}

void NewSplitEditor::numberChanged(const QString& newNumber)
{
  d->numberChanged(newNumber);
}

void NewSplitEditor::categoryChanged(const QString& accountId)
{
  d->categoryChanged(accountId);
}

void NewSplitEditor::costCenterChanged(int costCenterIndex)
{
  d->costCenterChanged(costCenterIndex);
}

void NewSplitEditor::amountChanged()
{
  d->amountChanged(d->amountHelper);
}

void NewSplitEditor::startLoadingSplit()
{
  d->loadingSplit = true;
}

void NewSplitEditor::finishLoadingSplit()
{
  d->loadingSplit = false;
}

