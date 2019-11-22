/***************************************************************************
                          newtransactioneditor.cpp
                             -------------------
    begin                : Sat Aug 8 2015
    copyright            : (C) 2015 by Thomas Baumgart
    email                : Thomas Baumgart <tbaumgart@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "newtransactioneditor.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QCompleter>
#include <QSortFilterProxyModel>
#include <QStringList>
#include <QDebug>
#include <QGlobalStatic>
#include <QStandardItemModel>
#include <QAbstractItemView>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "creditdebithelper.h"
#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneyexception.h"
#include "kmymoneyutils.h"
#include "kmymoneyaccountcombo.h"
#include "models.h"
#include "accountsmodel.h"
#include "costcentermodel.h"
#include "ledgermodel.h"
#include "splitmodel.h"
#include "payeesmodel.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "ui_newtransactioneditor.h"
#include "splitdialog.h"
#include "widgethintframe.h"
#include "icons/icons.h"
#include "modelenums.h"
#include "mymoneyenums.h"

using namespace Icons;

Q_GLOBAL_STATIC(QDate, lastUsedPostDate)

class NewTransactionEditor::Private
{
public:
  Private(NewTransactionEditor* parent)
  : ui(new Ui_NewTransactionEditor)
  , accountsModel(new AccountNamesFilterProxyModel(parent))
  , costCenterModel(new QSortFilterProxyModel(parent))
  , payeesModel(new QSortFilterProxyModel(parent))
  , accepted(false)
  , costCenterRequired(false)
  , amountHelper(nullptr)
  {
    accountsModel->setObjectName("NewTransactionEditor::accountsModel");
    costCenterModel->setObjectName("SortedCostCenterModel");
    payeesModel->setObjectName("SortedPayeesModel");
    statusModel.setObjectName("StatusModel");
    splitModel.setObjectName("SplitModel");

    costCenterModel->setSortLocaleAware(true);
    costCenterModel->setSortCaseSensitivity(Qt::CaseInsensitive);

    payeesModel->setSortLocaleAware(true);
    payeesModel->setSortCaseSensitivity(Qt::CaseInsensitive);

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
  void updateWidgetState();
  bool checkForValidTransaction(bool doUserInteraction = true);
  bool isDatePostOpeningDate(const QDate& date, const QString& accountId);

  bool postdateChanged(const QDate& date);
  bool costCenterChanged(int costCenterIndex);
  bool categoryChanged(const QString& accountId);
  bool numberChanged(const QString& newNumber);
  bool valueChanged(CreditDebitHelper* valueHelper);

  Ui_NewTransactionEditor*      ui;
  AccountNamesFilterProxyModel* accountsModel;
  QSortFilterProxyModel*        costCenterModel;
  QSortFilterProxyModel*        payeesModel;
  bool                          accepted;
  bool                          costCenterRequired;
  SplitModel                    splitModel;
  QStandardItemModel            statusModel;
  QString                       transactionSplitId;
  MyMoneyAccount                m_account;
  MyMoneyTransaction            transaction;
  MyMoneySplit                  split;
  CreditDebitHelper*            amountHelper;
};

void NewTransactionEditor::Private::createStatusEntry(eMyMoney::Split::State status)
{
  QStandardItem* p = new QStandardItem(KMyMoneyUtils::reconcileStateToString(status, true));
  p->setData((int)status);
  statusModel.appendRow(p);
}

void NewTransactionEditor::Private::updateWidgetState()
{
  // just in case it is disabled we turn it on
  ui->costCenterCombo->setEnabled(true);

  // setup the category/account combo box. If we have a split transaction, we disable the
  // combo box altogether. Changes can only be made via the split dialog editor
  bool blocked = false;
  QModelIndex index;

  // update the category combo box
  ui->accountCombo->setEnabled(true);
  switch(splitModel.rowCount()) {
    case 0:
      ui->accountCombo->setSelected(QString());
      break;
    case 1:
      index = splitModel.index(0, 0);
      ui->accountCombo->setSelected(splitModel.data(index, (int)eLedgerModel::Role::AccountId).toString());
      break;
    default:
      index = splitModel.index(0, 0);
      blocked = ui->accountCombo->lineEdit()->blockSignals(true);
      ui->accountCombo->lineEdit()->setText(i18n("Split transaction"));
      ui->accountCombo->setDisabled(true);
      ui->accountCombo->lineEdit()->blockSignals(blocked);
      ui->costCenterCombo->setDisabled(true);
      ui->costCenterLabel->setDisabled(true);
      break;
  }
  ui->accountCombo->hidePopup();

  // update the costcenter combo box
  if(ui->costCenterCombo->isEnabled()) {
    // extract the cost center
    index = splitModel.index(0, 0);
    QModelIndexList ccList = costCenterModel->match(costCenterModel->index(0, 0), CostCenterModel::CostCenterIdRole,
                                                    splitModel.data(index, (int)eLedgerModel::Role::CostCenterId),
                                      1,
                                      Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive | Qt::MatchRecursive));
    if (ccList.count() > 0) {
      index = ccList.front();
      ui->costCenterCombo->setCurrentIndex(index.row());
    }
  }
}

bool NewTransactionEditor::Private::checkForValidTransaction(bool doUserInteraction)
{
  QStringList infos;
  bool rc = true;
  if(!postdateChanged(ui->dateEdit->date())) {
    infos << ui->dateEdit->toolTip();
    rc = false;
  }

  if(!costCenterChanged(ui->costCenterCombo->currentIndex())) {
    infos << ui->costCenterCombo->toolTip();
    rc = false;
  }

  if(doUserInteraction) {
    /// @todo add dialog here that shows the @a infos about the problem
  }
  return rc;
}

bool NewTransactionEditor::Private::isDatePostOpeningDate(const QDate& date, const QString& accountId)
{
  bool rc = true;

  try {
    MyMoneyAccount account = MyMoneyFile::instance()->account(accountId);
    const bool isIncomeExpense = account.isIncomeExpense();

    // we don't check for categories
    if(!isIncomeExpense) {
      if(date < account.openingDate())
        rc = false;
    }
  } catch (MyMoneyException &e) {
    qDebug() << "Ooops: invalid account id" << accountId << "in" << Q_FUNC_INFO;
  }
  return rc;
}

bool NewTransactionEditor::Private::postdateChanged(const QDate& date)
{
  bool rc = true;
  WidgetHintFrame::hide(ui->dateEdit, i18n("The posting date of the transaction."));

  // collect all account ids
  QStringList accountIds;
  accountIds << m_account.id();
  for(int row = 0; row < splitModel.rowCount(); ++row) {
    QModelIndex index = splitModel.index(row, 0);
    accountIds << splitModel.data(index, (int)eLedgerModel::Role::AccountId).toString();;
  }

  Q_FOREACH(QString accountId, accountIds) {
    if(!isDatePostOpeningDate(date, accountId)) {
      MyMoneyAccount account = MyMoneyFile::instance()->account(accountId);
      WidgetHintFrame::show(ui->dateEdit, i18n("The posting date is prior to the opening date of account <b>%1</b>.", account.name()));
      rc = false;
      break;
    }
  }
  return rc;
}


bool NewTransactionEditor::Private::costCenterChanged(int costCenterIndex)
{
  bool rc = true;
  WidgetHintFrame::hide(ui->costCenterCombo, i18n("The cost center this transaction should be assigned to."));
  if(costCenterIndex != -1) {
    if(costCenterRequired && ui->costCenterCombo->currentText().isEmpty()) {
      WidgetHintFrame::show(ui->costCenterCombo, i18n("A cost center assignment is required for a transaction in the selected category."));
      rc = false;
    }
    if(rc == true && splitModel.rowCount() == 1) {
      QModelIndex index = costCenterModel->index(costCenterIndex, 0);
      QString costCenterId = costCenterModel->data(index, CostCenterModel::CostCenterIdRole).toString();
      index = splitModel.index(0, 0);
      splitModel.setData(index, costCenterId, (int)eLedgerModel::Role::CostCenterId);
    }
  }

  return rc;
}

bool NewTransactionEditor::Private::categoryChanged(const QString& accountId)
{
  bool rc = true;
  if(!accountId.isEmpty() && splitModel.rowCount() <= 1) {
    try {
      MyMoneyAccount category = MyMoneyFile::instance()->account(accountId);
      const bool isIncomeExpense = category.isIncomeExpense();
      ui->costCenterCombo->setEnabled(isIncomeExpense);
      ui->costCenterLabel->setEnabled(isIncomeExpense);
      costCenterRequired = category.isCostCenterRequired();
      rc &= costCenterChanged(ui->costCenterCombo->currentIndex());
      rc &= postdateChanged(ui->dateEdit->date());

      // make sure we have a split in the model
      bool newSplit = false;
      if(splitModel.rowCount() == 0) {
        splitModel.addEmptySplitEntry();
        newSplit = true;
      }

      const QModelIndex index = splitModel.index(0, 0);
      splitModel.setData(index, accountId, (int)eLedgerModel::Role::AccountId);
      if(newSplit) {
        costCenterChanged(ui->costCenterCombo->currentIndex());

        if(amountHelper->haveValue()) {
          splitModel.setData(index, QVariant::fromValue<MyMoneyMoney>(-amountHelper->value()), (int)eLedgerModel::Role::SplitValue);

          /// @todo make sure to convert initial value to shares according to price information

          splitModel.setData(index, QVariant::fromValue<MyMoneyMoney>(-amountHelper->value()), (int)eLedgerModel::Role::SplitShares);
        }
      }

      /// @todo we need to make sure to support multiple currencies here


    } catch (MyMoneyException &e) {
      qDebug() << "Ooops: invalid account id" << accountId << "in" << Q_FUNC_INFO;
    }
  }
  return rc;
}

bool NewTransactionEditor::Private::numberChanged(const QString& newNumber)
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
      if(model->data(index, (int)eLedgerModel::Role::AccountId) == m_account.id()
        && model->data(index, (int)eLedgerModel::Role::TransactionSplitId) != transactionSplitId) {
        WidgetHintFrame::show(ui->numberEdit, i18n("The check number <b>%1</b> has already been used in this account.", newNumber));
        rc = false;
        break;
      }
    }
  }
  return rc;
}

bool NewTransactionEditor::Private::valueChanged(CreditDebitHelper* valueHelper)
{
  bool rc = true;
  if(valueHelper->haveValue()  && splitModel.rowCount() <= 1) {
    rc = false;
    try {
      MyMoneyMoney shares;
      if(splitModel.rowCount() == 1) {
        const QModelIndex index = splitModel.index(0, 0);
        splitModel.setData(index, QVariant::fromValue<MyMoneyMoney>(-amountHelper->value()), (int)eLedgerModel::Role::SplitValue);

        /// @todo make sure to support multiple currencies

        splitModel.setData(index, QVariant::fromValue<MyMoneyMoney>(-amountHelper->value()), (int)eLedgerModel::Role::SplitShares);
      } else {
        /// @todo ask what to do: if the rest of the splits is the same amount we could simply reverse the sign
        /// of all splits, otherwise we could ask if the user wants to start the split editor or anything else.
      }
      rc = true;

    } catch (MyMoneyException &e) {
      qDebug() << "Ooops: something went wrong in" << Q_FUNC_INFO;
    }
  }
  return rc;
}


NewTransactionEditor::NewTransactionEditor(QWidget* parent, const QString& accountId)
  : QFrame(parent, Qt::FramelessWindowHint /* | Qt::X11BypassWindowManagerHint */)
  , d(new Private(this))
{
  auto const model = Models::instance()->accountsModel();
  // extract account information from model
  const auto index = model->accountById(accountId);
  d->m_account = model->data(index, (int)eAccountsModel::Role::Account).value<MyMoneyAccount>();

  d->ui->setupUi(this);

  d->accountsModel->addAccountGroup(QVector<eMyMoney::Account::Type> {eMyMoney::Account::Type::Asset, eMyMoney::Account::Type::Liability, eMyMoney::Account::Type::Income, eMyMoney::Account::Type::Expense, eMyMoney::Account::Type::Equity});
  d->accountsModel->setHideEquityAccounts(false);
  d->accountsModel->setSourceModel(model);
  d->accountsModel->setSourceColumns(model->getColumns());
  d->accountsModel->sort((int)eAccountsModel::Column::Account);
  d->ui->accountCombo->setModel(d->accountsModel);

  d->costCenterModel->setSortRole(Qt::DisplayRole);
  d->costCenterModel->setSourceModel(Models::instance()->costCenterModel());
  d->costCenterModel->sort(0);

  d->ui->costCenterCombo->setEditable(true);
  d->ui->costCenterCombo->setModel(d->costCenterModel);
  d->ui->costCenterCombo->setModelColumn(0);
  d->ui->costCenterCombo->completer()->setFilterMode(Qt::MatchContains);

  d->payeesModel->setSortRole(Qt::DisplayRole);
  d->payeesModel->setSourceModel(Models::instance()->payeesModel());
  d->payeesModel->sort(0);

  d->ui->payeeEdit->setEditable(true);
  d->ui->payeeEdit->setModel(d->payeesModel);
  d->ui->payeeEdit->setModelColumn(0);
  d->ui->payeeEdit->completer()->setFilterMode(Qt::MatchContains);

  d->ui->enterButton->setIcon(Icons::get(Icon::DialogOK));
  d->ui->cancelButton->setIcon(Icons::get(Icon::DialogCancel));

  d->ui->statusCombo->setModel(&d->statusModel);

  d->ui->dateEdit->setDisplayFormat(QLocale().dateFormat(QLocale::ShortFormat));

  d->ui->amountEditCredit->setAllowEmpty(true);
  d->ui->amountEditDebit->setAllowEmpty(true);
  d->amountHelper = new CreditDebitHelper(this, d->ui->amountEditCredit, d->ui->amountEditDebit);

  WidgetHintFrameCollection* frameCollection = new WidgetHintFrameCollection(this);
  frameCollection->addFrame(new WidgetHintFrame(d->ui->dateEdit));
  frameCollection->addFrame(new WidgetHintFrame(d->ui->costCenterCombo));
  frameCollection->addFrame(new WidgetHintFrame(d->ui->numberEdit, WidgetHintFrame::Warning));
  frameCollection->addWidget(d->ui->enterButton);

  connect(d->ui->numberEdit, SIGNAL(textChanged(QString)), this, SLOT(numberChanged(QString)));
  connect(d->ui->costCenterCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(costCenterChanged(int)));
  connect(d->ui->accountCombo, SIGNAL(accountSelected(QString)), this, SLOT(categoryChanged(QString)));
  connect(d->ui->dateEdit, SIGNAL(dateChanged(QDate)), this, SLOT(postdateChanged(QDate)));
  connect(d->amountHelper, SIGNAL(valueChanged()), this, SLOT(valueChanged()));

  connect(d->ui->cancelButton, SIGNAL(clicked(bool)), this, SLOT(reject()));
  connect(d->ui->enterButton, SIGNAL(clicked(bool)), this, SLOT(acceptEdit()));
  connect(d->ui->splitEditorButton, SIGNAL(clicked(bool)), this, SLOT(editSplits()));

  // handle some events in certain conditions different from default
  d->ui->payeeEdit->installEventFilter(this);
  d->ui->costCenterCombo->installEventFilter(this);
  d->ui->tagComboBox->installEventFilter(this);
  d->ui->statusCombo->installEventFilter(this);

  // setup tooltip

  // setWindowFlags(Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
}

NewTransactionEditor::~NewTransactionEditor()
{
}

bool NewTransactionEditor::accepted() const
{
  return d->accepted;
}

void NewTransactionEditor::acceptEdit()
{
  if(d->checkForValidTransaction()) {
    d->accepted = true;
    emit done();
  }
}

void NewTransactionEditor::reject()
{
  emit done();
}

void NewTransactionEditor::keyPressEvent(QKeyEvent* e)
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

void NewTransactionEditor::loadTransaction(const QString& id)
{
  const LedgerModel* model = Models::instance()->ledgerModel();
  const QString transactionId = model->transactionIdFromTransactionSplitId(id);

  if(id.isEmpty()) {
    d->transactionSplitId.clear();
    d->transaction = MyMoneyTransaction();
    if(lastUsedPostDate()->isValid()) {
      d->ui->dateEdit->setDate(*lastUsedPostDate());
    } else {
      d->ui->dateEdit->setDate(QDate::currentDate());
    }
    bool blocked = d->ui->accountCombo->lineEdit()->blockSignals(true);
    d->ui->accountCombo->lineEdit()->clear();
    d->ui->accountCombo->lineEdit()->blockSignals(blocked);

  } else {
    // find which item has this id and set is as the current item
    QModelIndexList list = model->match(model->index(0, 0), (int)eLedgerModel::Role::TransactionId,
                                        QVariant(transactionId),
                                        -1,                         // all splits
                                        Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive | Qt::MatchRecursive));

    Q_FOREACH(QModelIndex index, list) {
      // the selected split?
      const QString transactionSplitId = model->data(index, (int)eLedgerModel::Role::TransactionSplitId).toString();
      if(transactionSplitId == id) {
        d->transactionSplitId = id;
        d->transaction = model->data(index, (int)eLedgerModel::Role::Transaction).value<MyMoneyTransaction>();
        d->split = model->data(index, (int)eLedgerModel::Role::Split).value<MyMoneySplit>();
        d->ui->dateEdit->setDate(model->data(index, (int)eLedgerModel::Role::PostDate).toDate());
        d->ui->payeeEdit->lineEdit()->setText(model->data(index, (int)eLedgerModel::Role::PayeeName).toString());
        d->ui->memoEdit->clear();
        d->ui->memoEdit->insertPlainText(model->data(index, (int)eLedgerModel::Role::Memo).toString());
        d->ui->memoEdit->moveCursor(QTextCursor::Start);
        d->ui->memoEdit->ensureCursorVisible();

        // The calculator for the amount field can simply be added as an icon to the line edit widget.
        // See https://stackoverflow.com/questions/11381865/how-to-make-an-extra-icon-in-qlineedit-like-this howto do it
        d->ui->amountEditCredit->setText(model->data(model->index(index.row(), (int)eLedgerModel::Column::Payment)).toString());
        d->ui->amountEditDebit->setText(model->data(model->index(index.row(), (int)eLedgerModel::Column::Deposit)).toString());

        d->ui->numberEdit->setText(model->data(index, (int)eLedgerModel::Role::Number).toString());
        d->ui->statusCombo->setCurrentIndex(model->data(index, (int)eLedgerModel::Role::Number).toInt());

        QModelIndexList stList = d->statusModel.match(d->statusModel.index(0, 0), Qt::UserRole+1, model->data(index, (int)eLedgerModel::Role::Reconciliation).toInt());
        if(stList.count()) {
          QModelIndex stIndex = stList.front();
          d->ui->statusCombo->setCurrentIndex(stIndex.row());
        }
      } else {
        d->splitModel.addSplit(transactionSplitId);
      }
    }
    d->updateWidgetState();
  }

  // set focus to date edit once we return to event loop
  QMetaObject::invokeMethod(d->ui->dateEdit, "setFocus", Qt::QueuedConnection);
}

void NewTransactionEditor::numberChanged(const QString& newNumber)
{
  d->numberChanged(newNumber);
}

void NewTransactionEditor::categoryChanged(const QString& accountId)
{
  d->categoryChanged(accountId);
}

void NewTransactionEditor::costCenterChanged(int costCenterIndex)
{
  d->costCenterChanged(costCenterIndex);
}

void NewTransactionEditor::postdateChanged(const QDate& date)
{
  d->postdateChanged(date);
}

void NewTransactionEditor::valueChanged()
{
  d->valueChanged(d->amountHelper);
}

void NewTransactionEditor::editSplits()
{
  SplitModel splitModel;

  splitModel.deepCopy(d->splitModel, true);

  // create an empty split at the end
  splitModel.addEmptySplitEntry();

  QPointer<SplitDialog> splitDialog = new SplitDialog(d->m_account, transactionAmount(), this);
  splitDialog->setModel(&splitModel);

  int rc = splitDialog->exec();

  if(splitDialog && (rc == QDialog::Accepted)) {
      // remove that empty split again before we update the splits
      splitModel.removeEmptySplitEntry();

      // copy the splits model contents
      d->splitModel.deepCopy(splitModel, true);

      // update the transaction amount
      d->amountHelper->setValue(splitDialog->transactionAmount());

      d->updateWidgetState();
      QWidget *next = d->ui->tagComboBox;
      if(d->ui->costCenterCombo->isEnabled()) {
        next = d->ui->costCenterCombo;
      }
      next->setFocus();
  }

  if(splitDialog) {
    splitDialog->deleteLater();
  }
}

MyMoneyMoney NewTransactionEditor::transactionAmount() const
{
  return d->amountHelper->value();
}

void NewTransactionEditor::saveTransaction()
{
  MyMoneyTransaction t;

  if(!d->transactionSplitId.isEmpty()) {
    t = d->transaction;
  } else {
    // we keep the date when adding a new transaction
    // for the next new one
    *lastUsedPostDate() = d->ui->dateEdit->date();
  }

  // first remove the splits that are gone
  foreach (const auto split, t.splits()) {
    if(split.id() == d->split.id()) {
      continue;
    }
    int row;
    for(row = 0; row < d->splitModel.rowCount(); ++row) {
      QModelIndex index = d->splitModel.index(row, 0);
      if(d->splitModel.data(index, (int)eLedgerModel::Role::SplitId).toString() == split.id()) {
        break;
      }
    }

    // if the split is not in the model, we get rid of it
    if(d->splitModel.rowCount() == row) {
      t.removeSplit(split);
    }
  }

  MyMoneyFileTransaction ft;
  try {
    // new we update the split we are opened for
    MyMoneySplit sp(d->split);
    sp.setNumber(d->ui->numberEdit->text());
    sp.setMemo(d->ui->memoEdit->toPlainText());
    sp.setShares(d->amountHelper->value());
    if(t.commodity().isEmpty()) {
      t.setCommodity(d->m_account.currencyId());
      sp.setValue(d->amountHelper->value());
    } else {
      /// @todo check that the transactions commodity is the same
      /// as the one of the account this split references. If
      /// that is not the case, the next statement would create
      /// a problem
      sp.setValue(d->amountHelper->value());
    }

    if(sp.reconcileFlag() != eMyMoney::Split::State::Reconciled
    && !sp.reconcileDate().isValid()
    && d->ui->statusCombo->currentIndex() == (int)eMyMoney::Split::State::Reconciled) {
      sp.setReconcileDate(QDate::currentDate());
    }
    sp.setReconcileFlag(static_cast<eMyMoney::Split::State>(d->ui->statusCombo->currentIndex()));
    // sp.setPayeeId(d->ui->payeeEdit->cu)
    if(sp.id().isEmpty()) {
      t.addSplit(sp);
    } else {
      t.modifySplit(sp);
    }
    t.setPostDate(d->ui->dateEdit->date());

    // now update and add what we have in the model
    const SplitModel * model = &d->splitModel;
    for(int row = 0; row < model->rowCount(); ++row) {
      QModelIndex index = model->index(row, 0);
      MyMoneySplit s;
      const QString splitId = model->data(index, (int)eLedgerModel::Role::SplitId).toString();
      if(!SplitModel::isNewSplitId(splitId)) {
        s = t.splitById(splitId);
      }
      s.setNumber(model->data(index, (int)eLedgerModel::Role::Number).toString());
      s.setMemo(model->data(index, (int)eLedgerModel::Role::Memo).toString());
      s.setAccountId(model->data(index, (int)eLedgerModel::Role::AccountId).toString());
      s.setShares(model->data(index, (int)eLedgerModel::Role::SplitShares).value<MyMoneyMoney>());
      s.setValue(model->data(index, (int)eLedgerModel::Role::SplitValue).value<MyMoneyMoney>());
      s.setCostCenterId(model->data(index, (int)eLedgerModel::Role::CostCenterId).toString());
      s.setPayeeId(model->data(index, (int)eLedgerModel::Role::PayeeId).toString());

      // reconcile flag and date
      if(s.id().isEmpty()) {
        t.addSplit(s);
      } else {
        t.modifySplit(s);
      }
    }

    if(t.id().isEmpty()) {
      MyMoneyFile::instance()->addTransaction(t);
    } else {
      MyMoneyFile::instance()->modifyTransaction(t);
    }
    ft.commit();

  } catch (const MyMoneyException &e) {
    qDebug() << Q_FUNC_INFO << "something went wrong" << e.what();
  }

}

bool NewTransactionEditor::eventFilter(QObject* o, QEvent* e)
{
  auto cb = qobject_cast<QComboBox*>(o);
  if (o) {
    // filter out wheel events for combo boxes if the popup view is not visible
    if ((e->type() == QEvent::Wheel) && !cb->view()->isVisible()) {
      return true;
    }
  }
  return QFrame::eventFilter(o, e);
}

// kate: space-indent on; indent-width 2; remove-trailing-space on; remove-trailing-space-save on;
