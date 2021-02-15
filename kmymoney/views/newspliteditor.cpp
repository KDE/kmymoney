/*
    SPDX-FileCopyrightText: 2016 Thomas Baumgart <Thomas Baumgart <tbaumgart@kde.org>>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "newspliteditor.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QCompleter>
#include <QSortFilterProxyModel>
#include <QStringList>
#include <QDebug>
#include <QStandardItemModel>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "creditdebithelper.h"
#include "kmymoneyutils.h"
#include "kmymoneyaccountcombo.h"
#include "models.h"
#include "accountsmodel.h"
#include "costcentermodel.h"
#include "ledgermodel.h"
#include "splitmodel.h"
#include "mymoneyaccount.h"
#include "mymoneyexception.h"
#include "ui_newspliteditor.h"
#include "widgethintframe.h"
#include "ledgerview.h"
#include "icons/icons.h"
#include "mymoneyenums.h"
#include "modelenums.h"

using namespace Icons;

struct NewSplitEditor::Private
{
  Private(NewSplitEditor* parent)
  : ui(new Ui_NewSplitEditor)
  , accountsModel(new AccountNamesFilterProxyModel(parent))
  , costCenterModel(new QSortFilterProxyModel(parent))
  , splitModel(0)
  , accepted(false)
  , costCenterRequired(false)
  , showValuesInverted(false)
  , amountHelper(nullptr)
  {
    accountsModel->setObjectName("AccountNamesFilterProxyModel");
    costCenterModel->setObjectName("SortedCostCenterModel");
    statusModel.setObjectName("StatusModel");

    costCenterModel->setSortLocaleAware(true);
    costCenterModel->setSortCaseSensitivity(Qt::CaseInsensitive);

    createStatusEntry(eMyMoney::Split::State::NotReconciled);
    createStatusEntry(eMyMoney::Split::State::Cleared);
    createStatusEntry(eMyMoney::Split::State::Reconciled);
    // createStatusEntry(eMyMoney::Split::State::Frozen);
  }

  ~Private()
  {
    delete ui;
  }

  void createStatusEntry(eMyMoney::Split::State status);
  bool checkForValidSplit(bool doUserInteraction = true);

  bool costCenterChanged(int costCenterIndex);
  bool categoryChanged(const QString& accountId);
  bool numberChanged(const QString& newNumber);
  bool amountChanged(CreditDebitHelper* valueHelper);

  Ui_NewSplitEditor*            ui;
  AccountNamesFilterProxyModel* accountsModel;
  QSortFilterProxyModel*        costCenterModel;
  SplitModel*                   splitModel;
  bool                          accepted;
  bool                          costCenterRequired;
  bool                          showValuesInverted;
  QStandardItemModel            statusModel;
  QString                       transactionSplitId;
  MyMoneyAccount                counterAccount;
  MyMoneyAccount                category;
  CreditDebitHelper*            amountHelper;
};

void NewSplitEditor::Private::createStatusEntry(eMyMoney::Split::State status)
{
  QStandardItem* p = new QStandardItem(KMyMoneyUtils::reconcileStateToString(status, true));
  p->setData((int)status);
  statusModel.appendRow(p);
}

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
      QModelIndex index = Models::instance()->accountsModel()->accountById(accountId);
      category = Models::instance()->accountsModel()->data(index, (int)eAccountsModel::Role::Account).value<MyMoneyAccount>();
      const bool isIncomeExpense = category.isIncomeExpense();
      ui->costCenterCombo->setEnabled(isIncomeExpense);
      ui->costCenterLabel->setEnabled(isIncomeExpense);
      ui->numberEdit->setDisabled(isIncomeExpense);
      ui->numberLabel->setDisabled(isIncomeExpense);

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
  }
  return rc;
}

bool NewSplitEditor::Private::amountChanged(CreditDebitHelper* valueHelper)
{
  Q_UNUSED(valueHelper);
  bool rc = true;
  return rc;
}



NewSplitEditor::NewSplitEditor(QWidget* parent, const QString& counterAccountId)
  : QFrame(parent, Qt::FramelessWindowHint /* | Qt::X11BypassWindowManagerHint */)
  , d(new Private(this))
{
  SplitView* view = qobject_cast<SplitView*>(parent->parentWidget());
  Q_ASSERT(view != 0);
  d->splitModel = qobject_cast<SplitModel*>(view->model());

  QModelIndex index = Models::instance()->accountsModel()->accountById(counterAccountId);
  d->counterAccount = Models::instance()->accountsModel()->data(index, (int)eAccountsModel::Role::Account).value<MyMoneyAccount>();

  d->ui->setupUi(this);
  d->ui->enterButton->setIcon(Icons::get(Icon::DialogOK));
  d->ui->cancelButton->setIcon(Icons::get(Icon::DialogCancel));

  d->accountsModel->addAccountGroup(QVector<eMyMoney::Account::Type> {eMyMoney::Account::Type::Asset, eMyMoney::Account::Type::Liability, eMyMoney::Account::Type::Income, eMyMoney::Account::Type::Expense, eMyMoney::Account::Type::Equity});
  d->accountsModel->setHideEquityAccounts(false);
  auto const model = Models::instance()->accountsModel();
  d->accountsModel->setSourceModel(model);
  d->accountsModel->setSourceColumns(model->getColumns());
  d->accountsModel->sort((int)eAccountsModel::Column::Account);
  d->ui->accountCombo->setModel(d->accountsModel);

  d->costCenterModel->setSortRole(Qt::DisplayRole);
  d->costCenterModel->setSourceModel(Models::instance()->costCenterModel());
  d->costCenterModel->sort((int)eAccountsModel::Column::Account);

  d->ui->costCenterCombo->setEditable(true);
  d->ui->costCenterCombo->setModel(d->costCenterModel);
  d->ui->costCenterCombo->setModelColumn(0);
  d->ui->costCenterCombo->completer()->setFilterMode(Qt::MatchContains);

  WidgetHintFrameCollection* frameCollection = new WidgetHintFrameCollection(this);
  frameCollection->addFrame(new WidgetHintFrame(d->ui->costCenterCombo));
  frameCollection->addFrame(new WidgetHintFrame(d->ui->numberEdit, WidgetHintFrame::Warning));
  frameCollection->addWidget(d->ui->enterButton);

  d->amountHelper = new CreditDebitHelper(this, d->ui->amountEditCredit, d->ui->amountEditDebit);

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

MyMoneyMoney NewSplitEditor::amount() const
{
  return d->amountHelper->value();
}

void NewSplitEditor::setAmount(MyMoneyMoney value)
{
  d->amountHelper->setValue(value);
}

QString NewSplitEditor::costCenterId() const
{
  const int row = d->ui->costCenterCombo->currentIndex();
  QModelIndex index = d->ui->costCenterCombo->model()->index(row, 0);
  return d->ui->costCenterCombo->model()->data(index, CostCenterModel::CostCenterIdRole).toString();
}

void NewSplitEditor::setCostCenterId(const QString& id)
{
  QModelIndex index = Models::indexById(d->costCenterModel, CostCenterModel::CostCenterIdRole, id);
  if(index.isValid()) {
    d->ui->costCenterCombo->setCurrentIndex(index.row());
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


QString NewSplitEditor::splitId() const
{
  return d->transactionSplitId;
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
//  d->amountChanged(d->amountHelper); // useless call as reported by coverity scan
}
