/*
 * Copyright 2015-2019  Thomas Baumgart <tbaumgart@kde.org>
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

#include "mymoneyfile.h"
#include "mymoneymoney.h"
#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "accountsmodel.h"
#include "journalmodel.h"
#include "specialdatesmodel.h"
#include "columnselector.h"
#include "mymoneyenums.h"
#include "delegateproxy.h"
#include "journaldelegate.h"
#include "onlinebalancedelegate.h"
#include "specialdatedelegate.h"
#include "schedulesjournalmodel.h"

class LedgerView::Private
{
public:
  Private(LedgerView* p)
  : q(p)
  , delegateProxy(new DelegateProxy(q))
  , adjustableColumn(JournalModel::Column::Detail)
  , adjustingColumn(false)
  , showValuesInverted(false)
  , balanceCalculationPending(false)
  , newTransactionPresent(false)
  , columnSelector(nullptr)
  {
    const auto file = MyMoneyFile::instance();

    auto journalDelegate = new JournalDelegate(q);
    delegateProxy->addDelegate(file->journalModel(), journalDelegate);
    delegateProxy->addDelegate(file->journalModel()->newTransaction(), journalDelegate);
    delegateProxy->addDelegate(file->accountsModel(), new OnlineBalanceDelegate(q));
    delegateProxy->addDelegate(file->specialDatesModel(), new SpecialDateDelegate(q));
    delegateProxy->addDelegate(file->schedulesJournalModel(), journalDelegate);

    q->setItemDelegate(delegateProxy);
  }

  void setSingleLineDetailRole(eMyMoney::Model::Roles role)
  {
    for (auto delegate : delegates) {
      JournalDelegate* journalDelegate = qobject_cast<JournalDelegate*>(delegate);
      if(journalDelegate) {
        journalDelegate->setSingleLineRole(role);
      }
    }
  }

  LedgerView*                     q;
  DelegateProxy*                  delegateProxy;
  QHash<const QAbstractItemModel*, QStyledItemDelegate*>   delegates;
  MyMoneyAccount                  account;
  int                             adjustableColumn;
  bool                            adjustingColumn;
  bool                            showValuesInverted;
  bool                            balanceCalculationPending;
  bool                            newTransactionPresent;
  ColumnSelector*                 columnSelector;
};



LedgerView::LedgerView(QWidget* parent)
  : QTableView(parent)
  , d(new Private(this))
{
  // keep rows as small as possible
  verticalHeader()->setMinimumSectionSize(1);
  verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  verticalHeader()->hide();

  horizontalHeader()->setMinimumSectionSize(20);

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

  d->columnSelector = new ColumnSelector(this);
}

LedgerView::~LedgerView()
{
  delete d;
}

void LedgerView::setModel(QAbstractItemModel* model)
{
  QTableView::setModel(model);

  d->columnSelector->setModel(model);
  horizontalHeader()->setSectionResizeMode(JournalModel::Column::Reconciliation, QHeaderView::ResizeToContents);
}

bool LedgerView::showValuesInverted() const
{
  return d->showValuesInverted;
}

void LedgerView::setColumnsHidden(QVector<int> columns)
{
  d->columnSelector->setAlwaysHidden(columns);
}

void LedgerView::setColumnsShown(QVector<int> columns)
{
  d->columnSelector->setAlwaysVisible(columns);
}

bool LedgerView::edit(const QModelIndex& index, QAbstractItemView::EditTrigger trigger, QEvent* event)
{
  bool rc = QTableView::edit(index, trigger, event);

  if(rc) {
    // editing started, but we need the editor to cover all columns
    // so we close it, set the span to have a single row and recreate
    // the editor in that single cell
    closeEditor(indexWidget(index), QAbstractItemDelegate::NoHint);

    bool haveEditorInOtherView = false;
    /// @todo Here we need to make sure that only a single editor can be started at a time

    if(!haveEditorInOtherView) {
      emit aboutToStartEdit();
      setSpan(index.row(), 0, 1, horizontalHeader()->count());
      QModelIndex editIndex = model()->index(index.row(), 0);
      rc = QTableView::edit(editIndex, trigger, event);

      // make sure that the row gets resized according to the requirements of the editor
      // and is completely visible
      resizeRowToContents(index.row());
      QMetaObject::invokeMethod(this, "ensureCurrentItemIsVisible", Qt::QueuedConnection);
    } else {
      rc = false;
    }
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
  // qDebug() << "currentChanged" << current;
  QTableView::currentChanged(current, previous);

  if(current.isValid()) {
    QModelIndex idx = current.model()->index(current.row(), 0);
    QString id = idx.data(eMyMoney::Model::IdRole).toString();
    // For a new transaction the id is completely empty, for a split view the transaction
    // part is filled but the split id is empty and the string ends with a dash
    if(id.isEmpty() || id.endsWith('-')) {
      selectionModel()->clearSelection();
      setCurrentIndex(idx);
      selectRow(idx.row());
      scrollTo(idx, QAbstractItemView::PositionAtBottom);
      edit(idx);
    } else {
      scrollTo(idx, EnsureVisible);
      emit transactionSelected(MyMoneyModelBase::mapToBaseSource(idx));
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

void LedgerView::setSingleLineDetailRole(eMyMoney::Model::Roles role)
{
  d->setSingleLineDetailRole(role);
}

int LedgerView::sizeHintForColumn(int col) const
{
  if (col == JournalModel::Column::Reconciliation) {
    QStyleOptionViewItem opt;
    const QModelIndex index = model()->index(0, col);
    const auto delegate = d->delegateProxy->delegate(index);
    if (delegate) {
      int hint = delegate->sizeHint(opt, index).width();
      if(showGrid())
        hint += 1;
      return hint;
    }
  }
  return QTableView::sizeHintForColumn(col);
}

int LedgerView::sizeHintForRow(int row) const
{
  // we can optimize the sizeHintForRow() operation by asking the
  // delegate about the height. There's no need to use the std
  // method which scans over all items in a column and takes a long
  // time in large ledgers. In case the editor is open in the row, we
  // use the regular method.
  // We always ask for the detail column as this varies in height
  ensurePolished();

  const QModelIndex index = model()->index(row, JournalModel::Column::Detail);
  const auto delegate = d->delegateProxy->delegate(index);
  const auto journalDelegate = qobject_cast<const JournalDelegate*>(delegate);

  if(journalDelegate && (journalDelegate->editorRow() != row)) {
    QStyleOptionViewItem opt;
    opt.state |= (row == currentIndex().row()) ? QStyle::State_Selected : QStyle::State_None;
    int hint = delegate->sizeHint(opt, index).height();
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

void LedgerView::slotSettingsChanged()
{
#if 0

  // KMyMoneySettings::showGrid()
  // KMyMoneySettings::sortNormalView()
  // KMyMoneySettings::ledgerLens()
  // KMyMoneySettings::showRegisterDetailed()
  d->m_proxyModel->setHideClosedAccounts(KMyMoneySettings::hideClosedAccounts());
  d->m_proxyModel->setHideEquityAccounts(!KMyMoneySettings::expertMode());
  d->m_proxyModel->setHideFavoriteAccounts(true);
#endif
}

void LedgerView::selectMostRecentTransaction()
{
  if (model()->rowCount() > 0) {

    // we need to check that the last row may contain a scheduled transaction or
    // the row that is shown for new transacations or a special entry (e.g.
    // online balance or date mark).
    // in that case, we need to go back to find the actual last transaction
    int row = model()->rowCount()-1;
    const auto journalModel = MyMoneyFile::instance()->journalModel();
    while(row >= 0) {
      const QModelIndex idx = model()->index(row, 0);
      if (MyMoneyModelBase::baseModel(idx) == journalModel) {
        setCurrentIndex(idx);
        selectRow(idx.row());
        scrollTo(idx, QAbstractItemView::PositionAtBottom);
        break;
      }
      row--;
    }
  }
}
