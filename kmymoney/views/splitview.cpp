/*
    SPDX-FileCopyrightText: 2019-2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "splitview.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>
#include <QDebug>
#include <QHeaderView>
#include <QMenu>
#include <QPainter>
#include <QResizeEvent>
#include <QScopedPointer>
#include <QScrollBar>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "accountsmodel.h"
#include "columnselector.h"
#include "icons.h"
#include "mymoneyaccount.h"
#include "mymoneyenums.h"
#include "mymoneyfile.h"
#include "mymoneymoney.h"
#include "mymoneysecurity.h"
#include "splitdelegate.h"
#include "splitmodel.h"

class SplitView::Private
{
public:
    Private(SplitView* p)
        : q(p)
        , splitDelegate(nullptr)
        , adjustableColumn(SplitModel::Column::Memo)
        , adjustingColumn(false)
        , showValuesInverted(false)
        , balanceCalculationPending(false)
        , newTransactionPresent(false)
        , firstSelectionAfterCreation(true)
        , blockEditorStart(false)
        , rightMouseButtonPress(false)
        , readOnly(false)
        , columnSelector(nullptr)
    {
    }

    void setSingleLineDetailRole(eMyMoney::Model::Roles role)
    {
        Q_UNUSED(role)
#if 0
        auto delegate = qobject_cast<SplitDelegate*>(q->itemDelegate());
        if (delegate) {
            delegate->setSingleLineDetailRole(role);
        }
#endif
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

    void executeContextMenu(const QPoint& pos)
    {
        const auto currentIdx = q->currentIndex();
        const auto currentId = currentIdx.data(eMyMoney::Model::IdRole).toString();
        const auto enableOption = !(currentId.isEmpty() || currentId.endsWith('-'));

        QScopedPointer<QMenu> menu(new QMenu(q));
        menu->addSection(i18nc("@title:menu split context menu", "Split Options"));

        menu->addAction(Icons::get(Icons::Icon::DocumentNew), i18nc("@item:inmenu Create a split", "New..."), q, [&]() {
            // search the empty split and start editing it
            const auto rows = q->model()->rowCount();
            for (int row = 0; row < rows; ++row) {
                const auto idx = q->model()->index(row, 0);
                const auto id = idx.data(eMyMoney::Model::IdRole).toString();
                if (id.isEmpty() || id.endsWith('-')) {
                    // force a call to currentChanged() to start editing
                    q->setCurrentIndex(QModelIndex());
                    q->setCurrentIndex(idx);
                    break;
                }
            }
        });

        const auto editItem = menu->addAction(Icons::get(Icons::Icon::DocumentEdit), i18nc("@item:inmenu Edit a split", "Edit..."), q, [&]() {
            q->edit(currentIdx);
        });
        editItem->setEnabled(enableOption);

        const auto duplicateItem = menu->addAction(Icons::get(Icons::Icon::EditCopy), i18nc("@item:inmenu Duplicate selected split(s)", "Duplicate"), q, [&]() {
            const auto list = q->selectionModel()->selectedRows();
            QPersistentModelIndex newCurrentIdx;
            for (const auto idx : list) {
                const auto id = idx.data(eMyMoney::Model::IdRole).toString();
                if (!(id.isEmpty() || id.endsWith('-'))) {
                    // we can use any model object for the next operation, but we have to use one
                    const auto baseIdx = MyMoneyFile::instance()->accountsModel()->mapToBaseSource(idx);
                    auto model = const_cast<SplitModel*>(qobject_cast<const SplitModel*>(baseIdx.model()));
                    if (model) {
                        // get the original data
                        const auto split = model->itemByIndex(baseIdx);

                        model->appendEmptySplit();
                        const auto newIdx = model->emptySplit();
                        // prevent update signals
                        QSignalBlocker block(model);
                        // the id of the split will be automatically assigned by the model
                        model->setData(newIdx, split.number(), eMyMoney::Model::SplitNumberRole);
                        model->setData(newIdx, split.memo(), eMyMoney::Model::SplitMemoRole);
                        model->setData(newIdx, split.accountId(), eMyMoney::Model::SplitAccountIdRole);
                        model->setData(newIdx, split.costCenterId(), eMyMoney::Model::SplitCostCenterIdRole);
                        model->setData(newIdx, split.payeeId(), eMyMoney::Model::SplitPayeeIdRole);
                        model->setData(newIdx, QVariant::fromValue<MyMoneyMoney>(split.shares()), eMyMoney::Model::SplitSharesRole);
                        // send out the dataChanged signal with the next (last) setData()
                        block.unblock();
                        model->setData(newIdx, QVariant::fromValue<MyMoneyMoney>(split.value()), eMyMoney::Model::SplitValueRole);

                        // now add a new empty split at the end
                        model->appendEmptySplit();
                        // and make the new split the current one
                        if (!newCurrentIdx.isValid()) {
                            newCurrentIdx = newIdx;
                        }
                    }
                }
            }
            if (newCurrentIdx.isValid()) {
                q->setCurrentIndex(newCurrentIdx);
            }
        });
        duplicateItem->setEnabled(enableOption);

        const auto deleteItem = menu->addAction(Icons::get(Icons::Icon::EditRemove), i18nc("@item:inmenu Delete selected split(s)", "Delete"), q, [&]() {
            emit q->deleteSelectedSplits();
        });
        deleteItem->setEnabled(enableOption);

        menu->exec(q->viewport()->mapToGlobal(pos));
    }

    SplitView* q;
    SplitDelegate* splitDelegate;
    MyMoneyAccount account;
    int adjustableColumn;
    bool adjustingColumn;
    bool showValuesInverted;
    bool balanceCalculationPending;
    bool newTransactionPresent;
    bool firstSelectionAfterCreation;
    bool blockEditorStart;
    bool rightMouseButtonPress;
    bool readOnly;
    ColumnSelector* columnSelector;
};



SplitView::SplitView(QWidget* parent)
    : QTableView(parent)
    , d(new Private(this))
{
    // keep rows as small as possible
    verticalHeader()->setMinimumSectionSize(10);
    verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    verticalHeader()->hide();

    horizontalHeader()->setMinimumSectionSize(20);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    // since we don't have a vertical header, it does not make sense
    // to use the first column to select all items in the view
    setCornerButtonEnabled(false);

    // make sure to get informed about resize operations on the columns
    connect(horizontalHeader(), &QHeaderView::sectionResized, this, [&]() {
        adjustDetailColumn(viewport()->width());
    });

    // we don't need autoscroll as we do not support drag/drop
    setAutoScroll(false);

    setAlternatingRowColors(true);

    setSelectionBehavior(SelectRows);

    // setup the context menu
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, [&](QPoint pos) {
        d->executeContextMenu(pos);
    });

    setTabKeyNavigation(false);

    d->splitDelegate = new SplitDelegate(this);
    setItemDelegate(d->splitDelegate);

    d->columnSelector = new ColumnSelector(this, QStringLiteral("SplitEditor"));
    d->columnSelector->setAlwaysVisible(QVector<int>({ SplitModel::Column::Category, SplitModel::Column::Payment, SplitModel::Column::Deposit}));
}

SplitView::~SplitView()
{
    delete d;
}

void SplitView::setModel(QAbstractItemModel* model)
{
    QTableView::setModel(model);

    d->columnSelector->setModel(model);

    // This will allow the user to move the columns
    horizontalHeader()->setSectionsMovable(true);
}

void SplitView::setCommodity(const MyMoneySecurity& commodity)
{
    if (d->splitDelegate) {
        d->splitDelegate->setCommodity(commodity);
    }
}

bool SplitView::showValuesInverted() const
{
    return d->showValuesInverted;
}

void SplitView::setColumnsHidden(QVector<int> columns)
{
    d->columnSelector->setAlwaysHidden(columns);
}

void SplitView::setColumnsShown(QVector<int> columns)
{
    d->columnSelector->setAlwaysVisible(columns);
}

bool SplitView::edit(const QModelIndex& index, QAbstractItemView::EditTrigger trigger, QEvent* event)
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
            d->ensureEditorFullyVisible(index);
            QMetaObject::invokeMethod(this, "ensureCurrentItemIsVisible", Qt::QueuedConnection);
        } else {
            rc = false;
        }
    }

    return rc;
}

void SplitView::closeEditor(QWidget* editor, QAbstractItemDelegate::EndEditHint hint)
{
    QTableView::closeEditor(editor, hint);
    clearSpans();

    // we need to resize the row that contained the editor.
    resizeRowsToContents();

    emit aboutToFinishEdit();

    QMetaObject::invokeMethod(this, "ensureCurrentItemIsVisible", Qt::QueuedConnection);
}

void SplitView::mousePressEvent(QMouseEvent* event)
{
    // qDebug() << "mousePressEvent";
    if(state() != QAbstractItemView::EditingState) {
        d->rightMouseButtonPress = (event->button() == Qt::RightButton);
        QTableView::mousePressEvent(event);
        d->rightMouseButtonPress = false;
    }
}

void SplitView::mouseMoveEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
    // qDebug() << "mouseMoveEvent";
    // QTableView::mouseMoveEvent(event);
}

void SplitView::mouseDoubleClickEvent(QMouseEvent* event)
{
    // qDebug() << "mouseDoubleClickEvent";
    QTableView::mouseDoubleClickEvent(event);
}

void SplitView::wheelEvent(QWheelEvent* e)
{
    // qDebug() << "wheelEvent";
    QTableView::wheelEvent(e);
}

void SplitView::skipStartEditing()
{
    d->firstSelectionAfterCreation = true;
}

void SplitView::blockEditorStart(bool blocked)
{
    d->blockEditorStart = blocked;
}

QModelIndex SplitView::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    QModelIndex newIndex;

    if (!(modifiers & Qt::ControlModifier)) {
        // for home and end we need to have the ControlModifier set so
        // that the base class implementation works on rows instead of
        // columns.
        switch (cursorAction) {
        case MoveHome:
        case MoveEnd:
            newIndex = QTableView::moveCursor(cursorAction, modifiers | Qt::ControlModifier);
            break;

        default:
            newIndex = QTableView::moveCursor(cursorAction, modifiers);
            break;
        }
    }

    // now make sure that moving the cursor does not hit the empty
    // transaction at the bottom or a schedule.
    for (auto row = newIndex.row(); row >= 0; --row) {
        newIndex = model()->index(row, 0);
        QString id = newIndex.data(eMyMoney::Model::IdRole).toString();
        // skip the empty transaction at the end of a ledger if
        // the movement is not the down arrow
        if ((id.isEmpty() || id.endsWith('-')) && (cursorAction != MoveDown)) {
            continue;
        }
        if ((newIndex.flags() & (Qt::ItemIsSelectable | Qt::ItemIsEnabled)) == (Qt::ItemIsSelectable | Qt::ItemIsEnabled)) {
            return newIndex;
        }
    }
    return {};
}

void SplitView::currentChanged(const QModelIndex& current, const QModelIndex& previous)
{
    // qDebug() << "currentChanged";
    QTableView::currentChanged(current, previous);

    // is it a new selection (a different row)?
    if (current.isValid() && (current.row() != previous.row())) {
        const auto idx = current.model()->index(current.row(), 0);
        const auto id = idx.data(eMyMoney::Model::IdRole).toString();
        // For a new transaction the id is completely empty, for a split view the transaction
        // part is filled but the split id is empty and the string ends with a dash
        if (!d->blockEditorStart && !d->firstSelectionAfterCreation && (id.isEmpty() || id.endsWith('-')) && !d->rightMouseButtonPress) {
            selectionModel()->clearSelection();
            setCurrentIndex(idx);
            selectRow(idx.row());
            scrollTo(idx, QAbstractItemView::PositionAtBottom);
            edit(idx);
        } else {
            scrollTo(idx, EnsureVisible);
            emit transactionSelected(MyMoneyFile::baseModel()->mapToBaseSource(idx));
        }
        d->firstSelectionAfterCreation = false;
        QMetaObject::invokeMethod(this, "doItemsLayout", Qt::QueuedConnection);
    }
}

void SplitView::moveEvent(QMoveEvent* event)
{
    // qDebug() << "moveEvent";
    QWidget::moveEvent(event);
}

void SplitView::paintEvent(QPaintEvent* event)
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

void SplitView::setSingleLineDetailRole(eMyMoney::Model::Roles role)
{
    d->setSingleLineDetailRole(role);
}

int SplitView::sizeHintForColumn(int col) const
{
#if 0
    if (col == JournalModel::Column::Reconciliation) {
        QStyleOptionViewItem opt;
        const QModelIndex index = model()->index(0, col);
        const auto delegate = itemDelegate();
        if (delegate) {
            int hint = delegate->sizeHint(opt, index).width();
            if(showGrid())
                hint += 1;
            return hint;
        }
    }
#endif
    return QTableView::sizeHintForColumn(col);
}

int SplitView::sizeHintForRow(int row) const
{
    // we can optimize the sizeHintForRow() operation by asking the
    // delegate about the height. There's no need to use the std
    // method which scans over all items in a column and takes a long
    // time in large ledgers. In case the editor is open in the row, we
    // use the regular method.
    // We always ask for the detail column as this varies in height
    ensurePolished();

    if (model()) {
        const QModelIndex index = model()->index(row, SplitModel::Column::Memo);
        const auto delegate = itemDelegate();
        const auto splitDelegate = qobject_cast<const SplitDelegate*>(delegate);

        if(splitDelegate&& (splitDelegate->editorRow() != row)) {
            QStyleOptionViewItem opt;
            opt.state |= (row == currentIndex().row()) ? QStyle::State_Selected : QStyle::State_None;
            int hint = delegate->sizeHint(opt, index).height();
            if(showGrid())
                hint += 1;
            return hint;
        }
    }

    return QTableView::sizeHintForRow(row);
}

void SplitView::resizeEvent(QResizeEvent* event)
{
    // qDebug() << "resizeEvent, old:" << event->oldSize() << "new:" << event->size() << "viewport:" << viewport()->width();
    QTableView::resizeEvent(event);
    adjustDetailColumn(event->size().width());
}

void SplitView::adjustDetailColumn(int newViewportWidth)
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

void SplitView::ensureCurrentItemIsVisible()
{
    scrollTo(currentIndex(), EnsureVisible);
}

void SplitView::slotSettingsChanged()
{
#if 0

    // KMyMoneySettings::showGrid()
    // KMyMoneySettings::sortNormalView()
    // KMyMoneySettings::ledgerLens()
    // KMyMoneySettings::showRegisterDetailed()
    d->m_proxyModel->setHideClosedAccounts(!KMyMoneySettings::showAllAccounts());
    d->m_proxyModel->setHideEquityAccounts(!KMyMoneySettings::expertMode());
    d->m_proxyModel->setHideFavoriteAccounts(true);
#endif
}

void SplitView::selectMostRecentTransaction()
{
    if (model()->rowCount() > 0) {

        // we need to check that the last row may contain a scheduled transaction or
        // the row that is shown for new transacations or a special entry (e.g.
        // online balance or date mark).
        // in that case, we need to go back to find the actual last transaction
        int row = model()->rowCount()-1;
        if(row >= 0) {
            const QModelIndex idx = model()->index(row, 0);
            setCurrentIndex(idx);
            selectRow(idx.row());
            scrollTo(idx, QAbstractItemView::PositionAtBottom);
        }
    }
}

void SplitView::setTransactionPayeeId(const QString& id)
{
    if (d->splitDelegate) {
        d->splitDelegate->setTransactionPayeeId(id);
    }
}

void SplitView::setReadOnlyMode(bool readOnly)
{
    d->readOnly = readOnly;
    d->splitDelegate->setReadOnlyMode(readOnly);
}
