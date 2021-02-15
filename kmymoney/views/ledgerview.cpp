/***************************************************************************
                          ledgerview.cpp
                             -------------------
    begin                : Sat Aug 8 2015
    copyright            : (C) 2015 by Thomas Baumgart
    email                : Thomas Baumgart <tbaumgart@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#include "ledgerview.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QHeaderView>
#include <QPainter>
#include <QResizeEvent>
#include <QDate>
#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ledgerproxymodel.h"
#include "ledgerdelegate.h"
#include "ledgermodel.h"
#include "models.h"
#include "mymoneymoney.h"
#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "accountsmodel.h"

class LedgerView::Private
{
public:
  Private(LedgerView* p)
  : q(p)
  , delegate(0)
  , filterModel(new LedgerProxyModel(p))
  , adjustableColumn((int)eLedgerModel::Column::Detail)
  , adjustingColumn(false)
  , showValuesInverted(false)
  , balanceCalculationPending(false)
  {
    filterModel->setFilterRole((int)eLedgerModel::Role::AccountId);
    filterModel->setSourceModel(Models::instance()->ledgerModel());
  }

  void setDelegate(LedgerDelegate* _delegate)
  {
    delete delegate;
    delegate = _delegate;
  }

  void setSortRole(eLedgerModel::Role role, int column)
  {
    Q_ASSERT(delegate);
    Q_ASSERT(filterModel);

    delegate->setSortRole(role);
    filterModel->setSortRole((int)role);
    filterModel->sort(column);
  }

  void recalculateBalances()
  {
    const auto start = filterModel->index(0, 0);
    const auto indexes = filterModel->match(start, (int)eLedgerModel::Role::AccountId, account.id(), -1);
    MyMoneyMoney balance;
    for(const auto &index : indexes) {
      if(showValuesInverted) {
        balance -= filterModel->data(index, (int)eLedgerModel::Role::SplitShares).value<MyMoneyMoney>();
      } else {
        balance += filterModel->data(index, (int)eLedgerModel::Role::SplitShares).value<MyMoneyMoney>();
      }
      const auto txt = balance.formatMoney(account.fraction());
      const auto dispIndex = filterModel->index(index.row(), (int)eLedgerModel::Column::Balance);
      filterModel->setData(dispIndex, txt, Qt::DisplayRole);
    }

    // filterModel->invalidate();
    const QModelIndex top = filterModel->index(0, (int)eLedgerModel::Column::Balance);
    const QModelIndex bottom = filterModel->index(filterModel->rowCount()-1, (int)eLedgerModel::Column::Balance);

    q->dataChanged(top, bottom);
    balanceCalculationPending = false;
  }

  LedgerView*                 q;
  LedgerDelegate*             delegate;
  LedgerProxyModel*           filterModel;
  MyMoneyAccount              account;
  int                         adjustableColumn;
  bool                        adjustingColumn;
  bool                        showValuesInverted;
  bool                        balanceCalculationPending;
};



LedgerView::LedgerView(QWidget* parent)
  : QTableView(parent)
  , d(new Private(this))
{
  verticalHeader()->setDefaultSectionSize(15);
  verticalHeader()->setMinimumSectionSize(15);
  verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  verticalHeader()->hide();

  // since we don't have a vertical header, it does not make sense
  // to use the first column to select all items in the view
  setCornerButtonEnabled(false);

  // This will allow the user to move the columns, but
  // the delegate cannot handle it yet and it requires to
  // reset the spans as well.
  // horizontalHeader()->setMovable(true);

  // make sure to get informed about resize operations on the columns
  connect(horizontalHeader(), SIGNAL(sectionResized(int,int,int)), this, SLOT(adjustDetailColumn()));

  // we don't need autoscroll as we do not support drag/drop
  setAutoScroll(false);

  setAlternatingRowColors(true);

  setSelectionBehavior(SelectRows);

  setTabKeyNavigation(false);

  setModel(d->filterModel);
}

LedgerView::~LedgerView()
{
  delete d;
}

bool LedgerView::showValuesInverted() const
{
  return d->showValuesInverted;
}

void LedgerView::setAccount(const MyMoneyAccount& acc)
{
  d->account = acc;
  switch(acc.accountType()) {
    case eMyMoney::Account::Type::Investment:
      break;

    default:
      setColumnHidden((int)eLedgerModel::Column::Security, true);
      setColumnHidden((int)eLedgerModel::Column::CostCenter, true);
      setColumnHidden((int)eLedgerModel::Column::Quantity, true);
      setColumnHidden((int)eLedgerModel::Column::Price, true);
      setColumnHidden((int)eLedgerModel::Column::Amount, true);
      setColumnHidden((int)eLedgerModel::Column::Value, true);

      horizontalHeader()->resizeSection((int)eLedgerModel::Column::Reconciliation, 20);

      d->setDelegate(new LedgerDelegate(this));
      setItemDelegate(d->delegate);
      break;
  }

  d->showValuesInverted = false;
  if(acc.accountGroup() == eMyMoney::Account::Type::Liability
  || acc.accountGroup() == eMyMoney::Account::Type::Income) {
    d->showValuesInverted = true;
  }

  d->filterModel->setFilterRole((int)eLedgerModel::Role::AccountId);
  d->filterModel->setFilterKeyColumn(0);
  d->filterModel->setFilterFixedString(acc.id());
  d->filterModel->setAccountType(acc.accountType());

  d->setSortRole(eLedgerModel::Role::PostDate, (int)eLedgerModel::Column::Date);

  if (acc.hasOnlineMapping()) {
    connect(Models::instance()->accountsModel(), &AccountsModel::dataChanged, this, &LedgerView::accountChanged);
  } else {
    disconnect(Models::instance()->accountsModel(), &AccountsModel::dataChanged, this, &LedgerView::accountChanged);
    d->delegate->setOnlineBalance(QDate(), MyMoneyMoney());
  }
  accountChanged();

  // if balance calculation has not been triggered, then run it immediately
  if(!d->balanceCalculationPending) {
    recalculateBalances();
  }

  if(d->filterModel->rowCount() > 0) {
    // we need to check that the last row may contain a scheduled transaction or
    // the row that is shown for new transacations.
    // in that case, we need to go back to find the actual last transaction
    int row = d->filterModel->rowCount()-1;
    while(row >= 0) {
      const QModelIndex index = d->filterModel->index(row, 0);
      if(d->filterModel->data(index, (int)eLedgerModel::Role::ScheduleId).toString().isEmpty()
      && !d->filterModel->data(index, (int)eLedgerModel::Role::TransactionSplitId).toString().isEmpty() ) {
        setCurrentIndex(index);
        selectRow(index.row());
        scrollTo(index, PositionAtBottom);
        break;
      }
      row--;
    }
  }
}

QString LedgerView::accountId() const
{
  QString id;
  if(d->filterModel->filterRole() == (int)eLedgerModel::Role::AccountId)
    id = d->account.id();
  return id;
}

void LedgerView::accountChanged()
{
  QString id = accountId();
  if(!id.isEmpty()) {
    d->account = MyMoneyFile::instance()->account(id);
    QDate onlineBalanceDate = QDate::fromString(d->account.value(QLatin1String("lastImportedTransactionDate")), Qt::ISODate);
    MyMoneyMoney amount(d->account.value(QLatin1String("lastStatementBalance")));
    if (d->showValuesInverted) {
      amount = -amount;
    }
    d->delegate->setOnlineBalance(onlineBalanceDate, amount, d->account.fraction());
  } else {
    d->delegate->setOnlineBalance(QDate(), MyMoneyMoney());
  }
  // force redraw
  d->filterModel->invalidate();
}

void LedgerView::recalculateBalances()
{
  d->recalculateBalances();
}

void LedgerView::rowsAboutToBeRemoved(const QModelIndex& index, int start, int end)
{
  QAbstractItemView::rowsAboutToBeRemoved(index, start, end);
  // make sure the balances are recalculated but trigger only once
  if(!d->balanceCalculationPending) {
    d->balanceCalculationPending = true;
    QMetaObject::invokeMethod(this, "recalculateBalances", Qt::QueuedConnection);
  }
}

void LedgerView::rowsInserted(const QModelIndex& index, int start, int end)
{
  QTableView::rowsInserted(index, start, end);
  // make sure the balances are recalculated but trigger only once
  if(!d->balanceCalculationPending) {
    d->balanceCalculationPending = true;
    QMetaObject::invokeMethod(this, "recalculateBalances", Qt::QueuedConnection);
  }
}

bool LedgerView::edit(const QModelIndex& index, QAbstractItemView::EditTrigger trigger, QEvent* event)
{
  bool rc = QTableView::edit(index, trigger, event);

  if(rc) {
    // editing started, but we need the editor to cover all columns
    // so we close it, set the span to have a single row and recreate
    // the editor in that single cell
    closeEditor(indexWidget(index), QAbstractItemDelegate::NoHint);

//    bool haveEditorInOtherView = false;
    /// @todo Here we need to make sure that only a single editor can be started at a time

//    if(!haveEditorInOtherView) {
      emit aboutToStartEdit();
      setSpan(index.row(), 0, 1, horizontalHeader()->count());
      QModelIndex editIndex = model()->index(index.row(), 0);
      rc = QTableView::edit(editIndex, trigger, event);

      // make sure that the row gets resized according to the requirements of the editor
      // and is completely visible
      resizeRowToContents(index.row());
      QMetaObject::invokeMethod(this, "ensureCurrentItemIsVisible", Qt::QueuedConnection);
//    } else {
//      rc = false;
//    }
  }

  return rc;
}

void LedgerView::closeEditor(QWidget* editor, QAbstractItemDelegate::EndEditHint hint)
{
  QTableView::closeEditor(editor, hint);
  clearSpans();

  // we need to resize the row that contained the editor.
  resizeRowsToContents();

  emit aboutToFinishEdit();

  QMetaObject::invokeMethod(this, "ensureCurrentItemIsVisible", Qt::QueuedConnection);
}

void LedgerView::mousePressEvent(QMouseEvent* event)
{
  // qDebug() << "mousePressEvent";
  if(state() != QAbstractItemView::EditingState) {
    QTableView::mousePressEvent(event);
  }
}

void LedgerView::mouseMoveEvent(QMouseEvent* event)
{
  Q_UNUSED(event);
  // qDebug() << "mouseMoveEvent";
  // QTableView::mouseMoveEvent(event);
}

void LedgerView::mouseDoubleClickEvent(QMouseEvent* event)
{
  // qDebug() << "mouseDoubleClickEvent";
  QTableView::mouseDoubleClickEvent(event);
}

void LedgerView::wheelEvent(QWheelEvent* e)
{
  // qDebug() << "wheelEvent";
  QTableView::wheelEvent(e);
}

void LedgerView::currentChanged(const QModelIndex& current, const QModelIndex& previous)
{
  // qDebug() << "currentChanged";
  QTableView::currentChanged(current, previous);

  if(current.isValid()) {
    QModelIndex index = current.model()->index(current.row(), 0);
    scrollTo(index, EnsureVisible);
    QString id = current.model()->data(index, (int)eLedgerModel::Role::TransactionSplitId).toString();
    // For a new transaction the id is completely empty, for a split view the transaction
    // part is filled but the split id is empty and the string ends with a dash
    if(id.isEmpty() || id.endsWith('-')) {
      edit(index);
    } else {
      emit transactionSelected(id);
    }
    QMetaObject::invokeMethod(this, "doItemsLayout", Qt::QueuedConnection);
  }
}

void LedgerView::moveEvent(QMoveEvent* event)
{
  // qDebug() << "moveEvent";
  QWidget::moveEvent(event);
}

void LedgerView::paintEvent(QPaintEvent* event)
{
  QTableView::paintEvent(event);

  // the base class implementation paints the regular grid in case there
  // is room below the last line and the bottom of the viewport. We check
  // here if that is the case and fill that part with the base color to
  // remove the false painted grid.

  const QHeaderView *verticalHeader = this->verticalHeader();
  if(verticalHeader->count() == 0)
    return;

  int lastVisualRow = verticalHeader->visualIndexAt(verticalHeader->viewport()->height());
  if (lastVisualRow == -1)
      lastVisualRow = model()->rowCount(QModelIndex()) - 1;

  while(lastVisualRow >= model()->rowCount(QModelIndex()))
    --lastVisualRow;

  while ((lastVisualRow > -1) && verticalHeader->isSectionHidden(verticalHeader->logicalIndex(lastVisualRow)))
    --lastVisualRow;

  int top = 0;
  if(lastVisualRow != -1)
    top = verticalHeader->sectionViewportPosition(lastVisualRow) + verticalHeader->sectionSize(lastVisualRow);

  if(top < viewport()->height()) {
    QPainter painter(viewport());
    QRect rect(0, top, viewport()->width(), viewport()->height()-top);
    painter.fillRect(rect, QBrush(palette().base()));
  }
}

int LedgerView::sizeHintForRow(int row) const
{
  // we can optimize the sizeHintForRow() operation by asking the
  // delegate about the height. There's no need to use the std
  // method which scans over all items in a column and takes a long
  // time in large ledgers. In case the editor is open in the row, we
  // use the regular method.
  QModelIndex index = d->filterModel->index(row, 0);
  if(d->delegate && (d->delegate->editorRow() != row)) {
    QStyleOptionViewItem opt;
    int hint = d->delegate->sizeHint(opt, index).height();
    if(showGrid())
      hint += 1;
    return hint;
  }

  return QTableView::sizeHintForRow(row);
}

void LedgerView::resizeEvent(QResizeEvent* event)
{
  // qDebug() << "resizeEvent, old:" << event->oldSize() << "new:" << event->size() << "viewport:" << viewport()->width();
  QTableView::resizeEvent(event);
  adjustDetailColumn(event->size().width());
}

void LedgerView::adjustDetailColumn()
{
  adjustDetailColumn(viewport()->width());
}

void LedgerView::adjustDetailColumn(int newViewportWidth)
{
  // make sure we don't get here recursively
  if(d->adjustingColumn)
    return;

  d->adjustingColumn = true;

  QHeaderView* header = horizontalHeader();

  int totalColumnWidth = 0;
  for(int i=0; i < header->count(); ++i) {
    if(header->isSectionHidden(i)) {
      continue;
    }
    totalColumnWidth += header->sectionSize(i);
  }
  const int delta = newViewportWidth - totalColumnWidth;
  const int newWidth = header->sectionSize(d->adjustableColumn) + delta;
  if(newWidth > 10) {
    header->resizeSection(d->adjustableColumn, newWidth);
  }

  // remember that we're done this time
  d->adjustingColumn = false;
}

void LedgerView::ensureCurrentItemIsVisible()
{
  scrollTo(currentIndex(), EnsureVisible);
}

void LedgerView::setShowEntryForNewTransaction(bool show)
{
  d->filterModel->setShowNewTransaction(show);
}

SplitView::SplitView(QWidget* parent)
  : LedgerView(parent)
{
}

SplitView::~SplitView()
{
}
