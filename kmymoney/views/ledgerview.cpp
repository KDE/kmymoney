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
#include <QScrollBar>
#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageWidget>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "mymoneymoney.h"
#include "mymoneyfile.h"
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
#include "transactioneditorbase.h"

Q_GLOBAL_STATIC(LedgerView*, s_globalEditView);

class LedgerView::Private
{
public:
  Private(LedgerView* p)
  : q(p)
  , delegateProxy(new DelegateProxy(q))
  , adjustableColumn(JournalModel::Column::Detail)
  , adjustingColumn(false)
  , showValuesInverted(false)
  , newTransactionPresent(false)
  , columnSelector(nullptr)
  , infoMessage(new KMessageWidget(q))
  {
    infoMessage->hide();

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

  void ensureEditorFullyVisible(const QModelIndex& idx)
  {
    const auto viewportHeight = q->viewport()->height();
    const auto verticalOffset = q->verticalHeader()->offset();
    const auto verticalPosition = q->verticalHeader()->sectionPosition(idx.row());
    const auto cellHeight = q->verticalHeader()->sectionSize(idx.row());

    // in case the idx is displayed passed the viewport
    // adjust the position of the scroll area
    if (verticalPosition - verticalOffset + cellHeight > viewportHeight) {
      q->verticalScrollBar()->setValue(q->verticalScrollBar()->maximum());
    }
  }

  bool haveGlobalEditor()
  {
    return *s_globalEditView() != nullptr;
  }

  void registerGlobalEditor()
  {
    if (!haveGlobalEditor()) {
      *s_globalEditView() = q;
    }
  }

  void unregisterGlobalEditor()
  {
    *s_globalEditView() = nullptr;
  }

  LedgerView*                     q;
  DelegateProxy*                  delegateProxy;
  QHash<const QAbstractItemModel*, QStyledItemDelegate*>   delegates;
  int                             adjustableColumn;
  bool                            adjustingColumn;
  bool                            showValuesInverted;
  bool                            newTransactionPresent;
  ColumnSelector*                 columnSelector;
  KMessageWidget*                 infoMessage;
  QString                         accountId;
  QString                         groupName;
  QPersistentModelIndex           editIndex;
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
  connect(horizontalHeader(), &QHeaderView::sectionResized, this, [&]() { adjustDetailColumn(viewport()->width()); } );

  // we don't need autoscroll as we do not support drag/drop
  setAutoScroll(false);

  setAlternatingRowColors(true);

  setSelectionBehavior(SelectRows);

  setTabKeyNavigation(false);
}

LedgerView::~LedgerView()
{
  delete d;
}

void LedgerView::setColumnSelectorGroupName(const QString& groupName)
{
  if (!d->columnSelector) {
    d->groupName = groupName;
  } else {
    qWarning() << "LedgerView::setColumnSelectorGroupName must be called before model assignment";
  }
}

void LedgerView::setModel(QAbstractItemModel* model)
{
  if (!d->columnSelector) {
    d->columnSelector = new ColumnSelector(this, d->groupName);
  }
  QTableView::setModel(model);

  d->columnSelector->setModel(model);
  horizontalHeader()->setSectionResizeMode(JournalModel::Column::Reconciliation, QHeaderView::ResizeToContents);
}

void LedgerView::setAccountId(const QString& id)
{
  d->accountId = id;
}

const QString& LedgerView::accountId() const
{
  return d->accountId;
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
  bool suppressDuplicateEditorStart = false;

  switch(trigger) {
    case QAbstractItemView::DoubleClicked:
    case QAbstractItemView::EditKeyPressed:
        suppressDuplicateEditorStart = true;
      break;
    default:
      break;
  }

  if(d->haveGlobalEditor() && suppressDuplicateEditorStart) {
    if (!d->infoMessage->isVisible() && !d->infoMessage->isShowAnimationRunning()) {
      d->infoMessage->resize(viewport()->width(), d->infoMessage->height());
      d->infoMessage->setText(i18n("You are already editing a transaction in another view. KMyMoney does not support editing two transactions in parallel."));
      d->infoMessage->setMessageType(KMessageWidget::Warning);
      d->infoMessage->animatedShow();
    }

  } else {
    bool rc = QTableView::edit(index, trigger, event);

    if(rc) {
      // editing started, but we need the editor to cover all columns
      // so we close it, set the span to have a single row and recreate
      // the editor in that single cell
      closeEditor(indexWidget(index), QAbstractItemDelegate::NoHint);

      d->registerGlobalEditor();
      d->infoMessage->animatedHide();

      emit aboutToStartEdit();
      setSpan(index.row(), 0, 1, horizontalHeader()->count());
      d->editIndex = model()->index(index.row(), 0);

      rc = QTableView::edit(d->editIndex, trigger, event);

      // make sure that the row gets resized according to the requirements of the editor
      // and is completely visible
      const auto editor = qobject_cast<TransactionEditorBase*>(indexWidget(d->editIndex));
      connect(editor, &TransactionEditorBase::editorLayoutChanged, this, &LedgerView::resizeEditorRow);

      resizeEditorRow();
    }
    return rc;
  }
  return false;
}

void LedgerView::resizeEditorRow()
{
  resizeRowToContents(d->editIndex.row());
  d->ensureEditorFullyVisible(d->editIndex);
  QMetaObject::invokeMethod(this, "ensureCurrentItemIsVisible", Qt::QueuedConnection);
}

void LedgerView::closeEditor(QWidget* editor, QAbstractItemDelegate::EndEditHint hint)
{
  QTableView::closeEditor(editor, hint);
  clearSpans();

  d->unregisterGlobalEditor();

  // we need to resize the row that contained the editor.
  resizeRowsToContents();

  emit aboutToFinishEdit();

  d->editIndex = QModelIndex();
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

void LedgerView::keyPressEvent(QKeyEvent* kev)
{
  if ((d->infoMessage->isVisible()) && kev->matches(QKeySequence::Cancel)) {
    kev->accept();
    d->infoMessage->animatedHide();
  } else {
    QTableView::keyPressEvent(kev);
  }
}

void LedgerView::currentChanged(const QModelIndex& current, const QModelIndex& previous)
{
  QTableView::currentChanged(current, previous);

  if(current.isValid()) {
    QModelIndex idx = current.model()->index(current.row(), 0);
    QString id = idx.data(eMyMoney::Model::IdRole).toString();
    // For a new transaction the id is completely empty, for a split view the transaction
    // part is filled but the split id is empty and the string ends with a dash
    if(id.isEmpty() || id.endsWith('-')) {
      // the next two lines prevent an endless recursive call of this method
      if (idx == previous) {
        return;
      }
      // check for an empty account being opened. we can detect
      // that by an invalid previous index and don't start
      // editing right away.
      if (!previous.isValid()) {
        selectRow(idx.row());
        return;
      }
      selectionModel()->clearSelection();
      setCurrentIndex(idx);
      selectRow(idx.row());
      scrollTo(idx, QAbstractItemView::PositionAtBottom);
      edit(idx);
    } else {
      scrollTo(idx, EnsureVisible);
      emit transactionSelected(MyMoneyFile::baseModel()->mapToBaseSource(idx));
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
  d->infoMessage->resize(viewport()->width(), d->infoMessage->height());
  d->infoMessage->setWordWrap(false);
  d->infoMessage->setWordWrap(true);
  d->infoMessage->setText(d->infoMessage->text());
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
      if (MyMoneyFile::baseModel()->baseModel(idx) == journalModel) {
        setCurrentIndex(idx);
        selectRow(idx.row());
        scrollTo(idx, QAbstractItemView::PositionAtBottom);
        break;
      }
      row--;
    }
  }
}

QStringList LedgerView::selectedTransactions() const
{
  QStringList selection;

  QString id;
  for (const auto& idx : selectionModel()->selectedIndexes()) {
    id = idx.data(eMyMoney::Model::JournalTransactionIdRole).toString();
    if (!selection.contains(id)) {
      selection.append(id);
    }
  }
  return selection;
}

void LedgerView::setSelectedTransactions(const QStringList& transactionIds)
{
  QItemSelection selection;
  const auto journalModel = MyMoneyFile::instance()->journalModel();
  const auto lastColumn = model()->columnCount()-1;
  int startRow = -1;
  int lastRow = -1;
  QModelIndex currentIdx;

  auto createSelectionRange = [&]() {
    if (startRow != -1) {
      selection.select(model()->index(startRow, 0), model()->index(lastRow, lastColumn));
      startRow = -1;
    }
  };

  for (const auto& id : transactionIds) {
    if (id.isEmpty())
      continue;
    const auto indexes = journalModel->indexesByTransactionId(id);
    int row = -1;
    for (const auto baseIdx : indexes) {
      row = journalModel->mapFromBaseSource(model(), baseIdx).row();
      if (row != -1) {
        break;
      }
    }
    if (row == -1) {
      qDebug() << "transaction" << id << "not found anymore for selection. skipped";
      continue;
    }

    if (startRow == -1) {
      startRow = row;
      lastRow = row;
      // use the first as the current index
      if (!currentIdx.isValid()) {
        currentIdx = model()->index(startRow, 0);
      }
    } else {
      if (row == lastRow+1) {
        lastRow = row;
      } else {
        // a new range start, so we take care of it
        createSelectionRange();
      }
    }
  }

  // if no selection has been setup but we have
  // transactions in the ledger, we select the
  // last. The very last entry is the empty line,
  // so we have to skip that.
  if ((lastRow == -1) && (model()->rowCount() > 1)) {
    // find the last 'real' transaction
    startRow = model()->rowCount()-1;
    do {
      --startRow;
      currentIdx = model()->index(startRow, 0);
    } while (startRow > 0 && journalModel->baseModel(currentIdx) != journalModel);
    lastRow = startRow;
  }

  // add a possibly dangling range
  createSelectionRange();

  selectionModel()->clearSelection();
  selectionModel()->select(selection, QItemSelectionModel::Select);
  setCurrentIndex(currentIdx);
}
