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

#include <QTreeView>
#include <QCompleter>
#include <QSortFilterProxyModel>
#include <QStringList>
#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KLocale>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyaccountcombo.h"
#include "models.h"
#include "costcentermodel.h"
#include "ledgermodel.h"
#include "mymoneysplit.h"
#include "ui_newtransactioneditor.h"
#include "splitdialog.h"
#include "widgethintframe.h"

class NewTransactionEditor::Private
{
public:
  Private(NewTransactionEditor* parent)
  : ui(new Ui_NewTransactionEditor)
  , accountsModel(new AccountNamesFilterProxyModel(parent))
  , costCenterModel(new QSortFilterProxyModel(parent))
  , accepted(false)
  , costCenterRequired(false)
  , invertSign(false)
  {
    accountsModel->setObjectName("AccountNamesFilterProxyModel");
    costCenterModel->setObjectName("SortedCostCenterModel");
    statusModel.setObjectName("StatusModel");
    splitModel.setObjectName("SplitModel");

    costCenterModel->setSortLocaleAware(true);
    costCenterModel->setSortCaseSensitivity(Qt::CaseInsensitive);

    createStatusEntry(MyMoneySplit::NotReconciled);
    createStatusEntry(MyMoneySplit::Cleared);
    createStatusEntry(MyMoneySplit::Reconciled);
    // createStatusEntry(MyMoneySplit::Frozen);
  }

  void createStatusEntry(MyMoneySplit::reconcileFlagE status);
  void updateWidgetState();
  bool checkForValidTransaction(bool doUserInteraction = true);
  bool isDatePostOpeningDate(const QDate& date, const QString& accountId);

  int amountTypeCreditIndex() const { return 0; }
  int amountTypeDebitIndex() const { return 1; }

  bool postdateChanged(const QDate& date);
  bool costCenterChanged(int costCenterIndex);
  bool categoryChanged(const QString& accountId);
  bool numberChanged(const QString& newNumber);

  Ui_NewTransactionEditor*      ui;
  AccountNamesFilterProxyModel* accountsModel;
  QSortFilterProxyModel*        costCenterModel;
  bool                          accepted;
  bool                          costCenterRequired;
  bool                          costCenterOk;
  bool                          invertSign;
  SplitModel                    splitModel;
  QStandardItemModel            statusModel;
  QString                       transactionSplitId;
  QString                       accountId;
};

void NewTransactionEditor::Private::createStatusEntry(MyMoneySplit::reconcileFlagE status)
{
  QStandardItem* p = new QStandardItem(KMyMoneyUtils::reconcileStateToString(status, true));
  p->setData(status);
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
  switch(splitModel.rowCount()) {
    case 0:
      ui->accountCombo->setSelected(QString());
      break;
    case 1:
      index = splitModel.index(0, 0);
      ui->accountCombo->setSelected(splitModel.data(index, SplitModel::AccountIdRole).toString());
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

  // update the costcenter combo box
  if(ui->costCenterCombo->isEnabled()) {
    // extract the cost center
    index = splitModel.index(0, 0);
    QModelIndexList ccList = costCenterModel->match(costCenterModel->index(0, 0), CostCenterModel::CostCenterIdRole,
                                      splitModel.data(index, LedgerModel::CostCenterIdRole),
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
  accountIds << accountId;
  for(int row = 0; row < splitModel.rowCount(); ++row) {
    QModelIndex index = splitModel.index(row, 0);
    accountIds << splitModel.data(index, LedgerModel::AccountIdRole).toString();;
  }

  Q_FOREACH(QString accountId, accountIds) {
    if(!isDatePostOpeningDate(date, accountId)) {
      MyMoneyAccount account = MyMoneyFile::instance()->account(accountId);
      WidgetHintFrame::show(ui->dateEdit, i18n("The posting date is prior to the opening date of account <b>%1</b>.").arg(account.name()));
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

      // update the split model

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
    QModelIndexList list = model->match(model->index(0, 0), LedgerModel::NumberRole,
                                        QVariant(newNumber),
                                        -1,                         // all splits
                                        Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive | Qt::MatchRecursive));

    foreach(QModelIndex index, list) {
      qDebug() << model->data(index, LedgerModel::AccountIdRole).toString();
      if(model->data(index, LedgerModel::AccountIdRole) == accountId
      && model->data(index, LedgerModel::TransactionSplitIdRole) != transactionSplitId) {
        WidgetHintFrame::show(ui->numberEdit, i18n("The check number <b>%1</b> has already been used in this account.").arg(newNumber));
        rc = false;
        break;
      }
    }
  }
  return rc;
}




NewTransactionEditor::NewTransactionEditor(QWidget* parent, const QString& accountId)
  : QFrame(parent, Qt::FramelessWindowHint /* | Qt::X11BypassWindowManagerHint */)
  , d(new Private(this))
{
  d->accountId = accountId;
  d->ui->setupUi(this);
  d->accountsModel->addAccountGroup(MyMoneyAccount::Asset);
  d->accountsModel->addAccountGroup(MyMoneyAccount::Liability);
  d->accountsModel->addAccountGroup(MyMoneyAccount::Income);
  d->accountsModel->addAccountGroup(MyMoneyAccount::Expense);
  d->accountsModel->addAccountGroup(MyMoneyAccount::Equity);
  d->accountsModel->setHideEquityAccounts(false);
  d->accountsModel->setSourceModel(Models::instance()->accountsModel());
  d->accountsModel->sort(0);
  d->ui->accountCombo->setModel(d->accountsModel);

  d->costCenterModel->setSortRole(Qt::DisplayRole);
  d->costCenterModel->setSourceModel(Models::instance()->costCenterModel());
  d->costCenterModel->sort(0);

  d->ui->costCenterCombo->setEditable(true);
  d->ui->costCenterCombo->setModel(d->costCenterModel);
  d->ui->costCenterCombo->setModelColumn(0);
  d->ui->costCenterCombo->completer()->setFilterMode(Qt::MatchContains);

  d->ui->statusCombo->setModel(&d->statusModel);

  d->ui->dateEdit->setDateFormat(KLocale::global()->dateFormatShort());

  WidgetHintFrameCollection* frameCollection = new WidgetHintFrameCollection(this);
  frameCollection->addFrame(new WidgetHintFrame(d->ui->dateEdit));
  frameCollection->addFrame(new WidgetHintFrame(d->ui->costCenterCombo));
  frameCollection->addFrame(new WidgetHintFrame(d->ui->numberEdit, WidgetHintFrame::Warning));
  frameCollection->addWidget(d->ui->enterButton);

  connect(d->ui->numberEdit, SIGNAL(textChanged(QString)), this, SLOT(numberChanged(QString)));
  connect(d->ui->costCenterCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(costCenterChanged(int)));
  connect(d->ui->accountCombo, SIGNAL(accountSelected(QString)), this, SLOT(categoryChanged(QString)));
  connect(d->ui->dateEdit, SIGNAL(dateChanged(QDate)), this, SLOT(postdateChanged(QDate)));

  connect(d->ui->cancelButton, SIGNAL(clicked(bool)), this, SLOT(reject()));
  connect(d->ui->enterButton, SIGNAL(clicked(bool)), this, SLOT(acceptEdit()));
  connect(d->ui->splitEditorButton, SIGNAL(clicked(bool)), this, SLOT(editSplits()));

  // setup tooltip

  // setWindowFlags(Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
}

NewTransactionEditor::~NewTransactionEditor()
{
  delete d;
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


void NewTransactionEditor::setAccountId(const QString& id)
{
  d->accountId = id;
}

void NewTransactionEditor::setTransactionId(const QString& id)
{
  const LedgerModel* model = Models::instance()->ledgerModel();
  const QString transactionId = model->transactionIdFromTransactionSplitId(id);

  if(id.isEmpty()) {
    d->ui->dateEdit->setDate(QDate::currentDate());
    bool blocked = d->ui->accountCombo->lineEdit()->blockSignals(true);
    d->ui->accountCombo->lineEdit()->clear();
    d->ui->accountCombo->lineEdit()->blockSignals(blocked);

  } else {
    // find which item has this id and set is as the current item
    QModelIndexList list = model->match(model->index(0, 0), LedgerModel::TransactionIdRole,
                                        QVariant(transactionId),
                                        -1,                         // all splits
                                        Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive | Qt::MatchRecursive));

    Q_FOREACH(QModelIndex index, list) {
      // the selected split?
      const QString transactionSplitId = model->data(index, LedgerModel::TransactionSplitIdRole).toString();
      if(transactionSplitId == id) {
        d->transactionSplitId = id;
        d->ui->dateEdit->setDate(model->data(index, LedgerModel::PostDateRole).toDate());
        d->ui->payeeEdit->lineEdit()->setText(model->data(index, LedgerModel::PayeeNameRole).toString());
        d->ui->memoEdit->clear();
        d->ui->memoEdit->insertPlainText(model->data(index, LedgerModel::MemoRole).toString());
        d->ui->memoEdit->moveCursor(QTextCursor::Start);
        d->ui->memoEdit->ensureCursorVisible();

        // The calculator for the amount field can simply be added as an icon to the line edit widget.
        // See http://stackoverflow.com/questions/11381865/how-to-make-an-extra-icon-in-qlineedit-like-this howto do it
        d->ui->amountEdit->setText(model->data(index, LedgerModel::ShareAmountRole).toString());

        d->ui->amountTypeCombo->setCurrentIndex(model->data(index, LedgerModel::ShareAmountSuffixRole).toString() == i18nc("Debit suffix", "Dr.") ? 1 : 0);

        d->ui->numberEdit->setText(model->data(index, LedgerModel::NumberRole).toString());

        d->ui->statusCombo->setCurrentIndex(0);   // not reconciled is the default
        QModelIndexList stList = d->statusModel.match(d->statusModel.index(0, 0), Qt::UserRole+1, model->data(index, LedgerModel::ReconciliationRole).toInt());
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

  // set focus to payee edit once we return to event loop
  QMetaObject::invokeMethod(d->ui->payeeEdit, "setFocus", Qt::QueuedConnection);
}


QString NewTransactionEditor::transactionId() const
{
  qDebug() << "transactionId";
  return QString();
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

void NewTransactionEditor::editSplits()
{
  const int previousSplitCount = d->splitModel.rowCount();

  const bool isDepositTransaction = d->ui->amountTypeCombo->currentIndex() == d->amountTypeDebitIndex();
  SplitModel splitModel;

  splitModel.deepCopy(d->splitModel, isDepositTransaction);

  // create an empty split at the end
  splitModel.addEmptySplitEntry();

  MyMoneyAccount account = MyMoneyFile::instance()->account(d->accountId);

  QPointer<SplitDialog> splitDialog = new SplitDialog(account, this);
  splitDialog->setModel(&splitModel);

  do {
    int rc = splitDialog->exec();

    if(splitDialog && (rc == QDialog::Accepted)) {
      // check if we have a mismatch of values
      MyMoneyMoney splitSum;
      for(int row = 0; row < splitModel.rowCount(); ++row) {
        QModelIndex index = splitModel.index(row, 0);
        splitSum += splitModel.data(index, SplitModel::SplitValueRole).value<MyMoneyMoney>();
      }
      if(!isDepositTransaction) {
        splitSum = -splitSum;
      }

      if(transactionAmount() != splitSum) {
        /// @todo we need to add that KSplitTransactionDlg logic here
        qDebug() << "Problem" << splitSum.formatMoney(100);
      }

      // remove that empty split again before we update the splits
      splitModel.removeEmptySplitEntry();

      d->splitModel.deepCopy(splitModel, isDepositTransaction);
      d->updateWidgetState();
      break;

    } else {
      break;
    }
  } while(1);

  if(splitDialog) {
    splitDialog->deleteLater();
  }
}

MyMoneyMoney NewTransactionEditor::transactionAmount() const
{
  MyMoneyMoney rc = d->ui->amountEdit->value();
  qDebug() << "transactionAmount is " << rc.formatMoney(100);
  if(d->ui->amountTypeCombo->currentIndex() == d->amountTypeCreditIndex()) {
    rc = -rc;
    qDebug() << "Amount type is credit. Value change to" << rc.formatMoney(100);
  }
  if(d->invertSign) {
    rc = -rc;
    qDebug() << "Invert sign set. Value change to" << rc.formatMoney(100);
  }
  return rc;
}

void NewTransactionEditor::setInvertSign(bool invertSign)
{
  d->invertSign = invertSign;
}
