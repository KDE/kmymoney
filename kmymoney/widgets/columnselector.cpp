/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "columnselector.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDebug>
#include <QEvent>
#include <QHeaderView>
#include <QList>
#include <QMenu>
#include <QPoint>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KSharedConfig>
#include <KConfigGroup>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyenums.h"

/// @todo add feature to group multiple column selectors for the same view
/// in different instances by group name.

class ColumnSelectorPrivate
{
    Q_DECLARE_PUBLIC(ColumnSelector)

public:
    ColumnSelectorPrivate(ColumnSelector* qq)
        : q_ptr(qq)
        , treeView(nullptr)
        , tableView(nullptr)
        , headerView(nullptr)
        , model(nullptr)
        , storageOffset(0)
        , isInit(false)
        , columnSelectionEnabled(true)
    {
    }

    void setColumnHidden(int col, bool state)
    {
        if (treeView) {
            treeView->setColumnHidden(col, state);
        }
        else if (tableView) {
            tableView->setColumnHidden(col, state);
        }
    }

    bool isColumnHidden(int col)
    {
        if (treeView) {
            return treeView->isColumnHidden(col);
        }
        else if (tableView) {
            return tableView->isColumnHidden(col);
        }
        return false;
    }

    void init(const QString& _configGroupName)
    {
        Q_Q(ColumnSelector);

        configGroupName = _configGroupName;

        if ((treeView || tableView) && model && headerView) {
            const auto maxColumn = model->columnCount();
            QList<int> visibleColumns;

            if (!configGroupName.isEmpty()) {
                // restore the headers when we have a group name
                const auto grp = KSharedConfig::openConfig()->group(configGroupName);

                // NOTE: Temporarily disabled restoreState() due to crashes when the saved
                // state is incompatible with the current model structure (e.g., different
                // number of columns). This happens when editing splits from loan transactions.
                // TODO: Find a way to validate compatibility before restoring, or use different
                // config group names for different contexts.
                // if (!isInit) {
                //     const auto columnNames = grp.readEntry("HeaderState", QByteArray());
                //     if (!columnNames.isEmpty() && headerView->model()) {
                //         headerView->restoreState(columnNames);
                //     }
                // }

                visibleColumns = grp.readEntry<int>("ColumnsSelection", QList<int>());
                // add the storage offset during loading operation
                for (auto &column : visibleColumns) {
                    if (applyStorageOffsetColumns.contains(column + storageOffset)) {
                        column += storageOffset;
                    }
                }
            }

            if (visibleColumns.isEmpty()) {
                // in case no column was marked as visible so far, all should
                for (int col = 0; col < maxColumn; ++col) {
                    visibleColumns += col;
                }
            }

            // now setup the initial visibility of the columns
            for (int col = 0; col < maxColumn; ++col) {
                if (alwaysHiddenColumns.contains(col)) {
                    setColumnHidden(col, true);
                } else if (alwaysVisibleColumns.contains(col) || visibleColumns.contains(col)) {
                    setColumnHidden(col, false);
                } else {
                    setColumnHidden(col, true);
                }
            }

            // allow context menu to be opened on tree header for columns selection
            // and allow resizing of columns
            headerView->setContextMenuPolicy(Qt::CustomContextMenu);
            headerView->setSectionResizeMode(QHeaderView::Interactive);

            if (!isInit) {
                q->connect(headerView, &QWidget::customContextMenuRequested, q, &ColumnSelector::slotColumnsMenu);
                headerView->installEventFilter(q);
                isInit = true;
            }

        } else if (!(treeView || tableView)) {
            qDebug() << "WARNING: You must not create a ColumnSelector without a view";
        }
    }

    void updateHeaderState()
    {
        if (headerView && !configGroupName.isEmpty()) {
            auto grp = KSharedConfig::openConfig()->group(configGroupName);
            grp.writeEntry("HeaderState", headerView->saveState());
            grp.sync();
        }
    }

    ColumnSelector*       q_ptr;

    QTreeView*            treeView;
    QTableView*           tableView;
    QHeaderView*          headerView;
    QAbstractItemModel*   model;

    QVector<int>          selectableColumns;
    QVector<int>          alwaysHiddenColumns;
    QVector<int>          alwaysVisibleColumns;
    QVector<int>          applyStorageOffsetColumns;
    QString               configGroupName;

    int                   storageOffset;
    bool                  isInit;
    bool columnSelectionEnabled;
};

ColumnSelector::ColumnSelector(QTableView* view, const QString& configGroupName, int offset, const QVector<int>& columns)
    : QObject(view)
    , d_ptr(new ColumnSelectorPrivate(this))
{
    Q_D(ColumnSelector);
    d->tableView = view;
    d->headerView = view->horizontalHeader();
    d->model = view->model();
    d->storageOffset = offset;
    d->applyStorageOffsetColumns = columns;
    d->init(configGroupName);
}

ColumnSelector::ColumnSelector(QTreeView* view, const QString& configGroupName, int offset, const QVector<int>& columns)
    : QObject(view)
    , d_ptr(new ColumnSelectorPrivate(this))
{
    Q_D(ColumnSelector);
    d->treeView = view;
    d->headerView = view->header();
    d->model = view->model();
    d->storageOffset = offset;
    d->applyStorageOffsetColumns = columns;
    d->init(configGroupName);

    // make sure to keep the header state just
    // before the header view is deleted
    connect(d->headerView, &QObject::destroyed, this, [&]() {
        Q_D(ColumnSelector);
        d->updateHeaderState();
        d->headerView = nullptr;
    });
}

ColumnSelector::~ColumnSelector()
{
    Q_D(ColumnSelector);
    d->updateHeaderState();
    delete d;
}

void ColumnSelector::slotColumnsMenu(const QPoint)
{
    Q_D(ColumnSelector);

    if (!d->columnSelectionEnabled) {
        return;
    }

    // create actions menu
    QList<QAction *> actions;
    const auto maxColumn = d->headerView->count();

    for (int col = 0; col < maxColumn; ++col) {
        if (d->alwaysHiddenColumns.contains(col))
            continue;
        if (d->alwaysVisibleColumns.contains(col))
            continue;
        if (!d->selectableColumns.isEmpty()) {
            if (!d->selectableColumns.contains(col)) {
                continue;
            }
        }
        auto a = new QAction(nullptr);
        a->setObjectName(QString::number(col));
        a->setText(d->model->headerData(col, Qt::Horizontal, eMyMoney::Model::LongDisplayRole).toString());
        a->setCheckable(true);
        a->setChecked(!d->isColumnHidden(col));
        actions.append(a);
    }

    // if there are any actions present menu
    if (!actions.isEmpty()) {
        QMenu menu(i18n("Displayed columns"));
        menu.addActions(actions);

        // execute menu and get result
        const auto retAction = menu.exec(QCursor::pos());
        if (retAction) {
            d->setColumnHidden(retAction->objectName().toInt(), !retAction->isChecked());

            QList<int> visibleColumns;

            for (int col = 0; col < maxColumn; ++col) {
                if (d->alwaysHiddenColumns.contains(col))
                    continue;
                // in case a column is always visible, we
                // add it to the list for backward compatibility
                if (!d->alwaysVisibleColumns.contains(col)) {
                    if (!d->selectableColumns.isEmpty() && !d->selectableColumns.contains(col))
                        continue;
                }
                if (!d->isColumnHidden(col)) {
                    visibleColumns.append(col);
                }
            }
            if (!d->configGroupName.isEmpty()) {
                auto grp = KSharedConfig::openConfig()->group(d->configGroupName);
                // subtract offset during storage operation
                for (auto &column : visibleColumns) {
                    if (d->applyStorageOffsetColumns.contains(column)) {
                        column -= d->storageOffset;
                    }
                }
                grp.writeEntry("ColumnsSelection", visibleColumns);
                grp.sync();
            }

            Q_EMIT columnsChanged();
        }
    }
}

void ColumnSelector::setAlwaysHidden(QVector<int> columns)
{
    Q_D(ColumnSelector);

    d->alwaysHiddenColumns = columns;
    for (int i = 0; i < columns.count(); ++i) {
        auto col = columns.at(i);
        d->setColumnHidden(col, true);
        d->alwaysVisibleColumns.removeAll(col);
    }
}

void ColumnSelector::setAlwaysVisible(QVector<int> columns)
{
    Q_D(ColumnSelector);

    d->alwaysVisibleColumns = columns;
    for (int i = 0; i < columns.count(); ++i) {
        auto col = columns.at(i);
        d->setColumnHidden(col, false);
        d->alwaysHiddenColumns.removeAll(col);
    }
}

void ColumnSelector::setSelectable(QVector<int> columns)
{
    Q_D(ColumnSelector);

    d->selectableColumns = columns;
}

void ColumnSelector::setModel(QAbstractItemModel* model)
{
    Q_D(ColumnSelector);
    d->model = model;
    d->init(d->configGroupName);
}

QVector<int> ColumnSelector::columns() const
{
    Q_D(const ColumnSelector);

    QVector<int>    columns;
    const auto maxColumn = d->headerView->count();

    for (int col = 0; col < maxColumn; ++col) {
        columns.append(col);
    }
    return columns;
}

const QString& ColumnSelector::configGroupName() const
{
    Q_D(const ColumnSelector);
    return d->configGroupName;
}

void ColumnSelector::setColumnSelectionEnabled()
{
    Q_D(ColumnSelector);
    d->columnSelectionEnabled = true;
}

void ColumnSelector::setColumnSelectionDisabled()
{
    Q_D(ColumnSelector);
    d->columnSelectionEnabled = false;
}

bool ColumnSelector::eventFilter(QObject* o, QEvent* e)
{
    Q_D(ColumnSelector);
    if (e->type() == QEvent::Show) {
        // It has been noticed, that if the application's configuration is lost, the
        // initial setup hidden columns are not filled with data but they are visible
        // in the view. Calling the hide/showSection again when the header view is
        // about to be displayed solves the issue and removes them completely.
        // Turns out that on Qt6 one needs to call showColumn before the hideColumn
        // call has an effect. While we're doing that, we make sure that we don't
        // save the configuration on the fly
        const auto maxColumn = d->model->columnCount();
        for (int col = 0; col < maxColumn; ++col) {
            const auto hidden = d->isColumnHidden(col);
            d->headerView->showSection(col);
            if (hidden) {
                d->headerView->hideSection(col);
            }
        }
    }
    return QObject::eventFilter(o, e);
}
