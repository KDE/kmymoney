/*
    SPDX-FileCopyrightText: 2015-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LEDGERVIEW_H
#define LEDGERVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QTableView>
#include <QVector>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyenums.h"

class MyMoneyAccount;
class SelectedObjects;

namespace eMenu {
enum class Menu;
}

class LedgerView : public QTableView
{
    Q_OBJECT
public:
    explicit LedgerView(QWidget* parent = 0);
    virtual ~LedgerView();

    /**
     * This method is used to modify the visibility of the
     * empty entry at the end of the ledger. The default
     * for the parameter @a show is @c true.
     */
    void setShowEntryForNewTransaction(bool show = true);

    void setSingleLineDetailRole(eMyMoney::Model::Roles role);

    /**
     * Returns true if the sign of the values displayed has
     * been inverted depending on the account type.
     */
    bool showValuesInverted() const;

    void setColumnsHidden(QVector<int> columns);
    void setColumnsShown(QVector<int> columns);

    void setModel(QAbstractItemModel * model) override;

    QStringList selectedJournalEntries() const;

    void setSelectedJournalEntries(const QStringList& journalEntryIds);
    void reselectJournalEntry(const QString& journalEntryId);

    void selectMostRecentTransaction();

    void selectAllTransactions();

    void setColumnSelectorGroupName(const QString& groupName);

    void editNewTransaction();

    QVector<eMyMoney::Model::Roles> statusRoles(const QModelIndex& idx) const;

    /**
     * New transactions will be created in account referenced by @a id.
     */
    void setAccountId(const QString& id);
    const QString& accountId() const;

    QModelIndex editIndex() const;

public Q_SLOTS:
    /**
     * This method scrolls the ledger so that the current item is visible
     */
    void ensureCurrentItemIsVisible();

    /**
     * Overridden for internal reasons. No change in base functionality
     */
    void edit(const QModelIndex& index) {
        QTableView::edit(index);
    }

    void slotSettingsChanged();

protected:
    bool edit(const QModelIndex& index, EditTrigger trigger, QEvent* event) final override;
    void mousePressEvent(QMouseEvent* event) final override;
    void mouseMoveEvent(QMouseEvent* event) final override;
    void mouseDoubleClickEvent(QMouseEvent* event) final override;
    void wheelEvent(QWheelEvent *event) final override;
    void moveEvent(QMoveEvent *event) final override;
    void resizeEvent(QResizeEvent* event) final override;
    void paintEvent(QPaintEvent* event) final override;
    int sizeHintForRow(int row) const final override;
    int sizeHintForColumn(int row) const final override;
    void keyPressEvent ( QKeyEvent* event ) override;
    bool viewportEvent(QEvent*) override;

protected Q_SLOTS:
    void selectionChanged ( const QItemSelection& selected, const QItemSelection& deselected ) override;
    void closeEditor(QWidget* editor, QAbstractItemDelegate::EndEditHint hint) final override;
    void currentChanged(const QModelIndex &current, const QModelIndex &previous) final override;
    void resizeEditorRow();

    virtual void adjustDetailColumn(int newViewportWidth);

    void slotMoveToAccount(const QString& accountId);

Q_SIGNALS:
    void requestCustomContextMenu(eMenu::Menu type, const QPoint& pos) const;
    void transactionSelectionChanged (const SelectedObjects& selection) const;
    void transactionSelected(const QModelIndex& idx) const;
    void aboutToStartEdit() const;
    void aboutToFinishEdit() const;

protected:
    class Private;
    Private * const d;
};
#endif // LEDGERVIEW_H

