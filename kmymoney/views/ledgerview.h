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

#include "ledgerviewsettings.h"
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

    void setSingleLineDetailRole(eMyMoney::Model::Roles role);

    /**
     * Returns true if the sign of the values displayed has
     * been inverted depending on the account type.
     */
    bool showValuesInverted() const;

    void setColumnsHidden(QVector<int> columns);
    void setColumnsShown(QVector<int> columns);

    void setModel(QAbstractItemModel * model) override;

    QStringList selectedJournalEntryIds() const;

    void reselectJournalEntry(const QString& journalEntryId);

    void selectMostRecentTransaction();

    void selectAllTransactions();

    void setColumnSelectorGroupName(const QString& groupName);

    /**
     * If @a show is @c true, the payee name is shown in the
     * detail column if no payee column is present. This defaults
     * to @c true.
     */
    void setShowPayeeInDetailColumn(bool show);

    void editNewTransaction();

    QVector<eMyMoney::Model::Roles> statusRoles(const QModelIndex& idx) const;

    /**
     * New transactions will be created in account referenced by @a id.
     */
    void setAccountId(const QString& id);
    const QString& accountId() const;

    QModelIndex editIndex() const;

    /**
     * In case an editor is opened, make sure to fully show the
     * editor in the viewport. If editing is not active, this
     * acts as a nop.
     */
    void showEditor();

    void setSortOrder(LedgerSortOrder sortOrder);

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

    /**
     * This resizes the section identified by @a section
     * if this view belongs to the same configuration group
     * identified by @a configGroupName. In case @a view points
     * to itself, the method does nothing and returns.
     *
     * @param view pointer to LedgerView object
     * @param configGroupName name of the configuration group that changes
     * @param section column index
     * @param oldSize the old width of the column in pixels
     * @param newSize the new width of the column in pixels
     *
     * @note Calls adjustDetailColumn(newSize, false) in case
     * the size changes but does not inform other views about the
     * change.
     */
    void resizeSection(QWidget* view, const QString& configGroupName, int section, int oldSize, int newSize);

    /**
     * This moves the section identified by @a section
     * In case @a view points to itself, the method does nothing and returns.
     *
     * @param view pointer to LedgerView object
     * @param section section index
     * @param oldIndex the old index of the column
     * @param newIndex the new index of the column
     *
     * @note Does not inform other views about the change.
     */
    void moveSection(QWidget* view, int section, int oldIndex, int newIndex);

    void reset() override;

    void setSelectedJournalEntries(const QStringList& journalEntryIds);

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
    /**
     * Overridden to prevent moving to the end using End or PageDown to start the editor
     */
    QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) override;

protected Q_SLOTS:
    void selectionChanged ( const QItemSelection& selected, const QItemSelection& deselected ) override;
    void closeEditor(QWidget* editor, QAbstractItemDelegate::EndEditHint hint) final override;
    void currentChanged(const QModelIndex &current, const QModelIndex &previous) final override;
    void resizeEditorRow();

    /**
     * Adjust the detail column so that it takes the rest of the
     * available width. In case @a informOtherViews is @c true,
     * the sectionResized signal is emitted, if it is @c false
     * the emission will be suppressed.
     */
    virtual void adjustDetailColumn(int newViewportWidth, bool informOtherViews);

    void slotMoveToAccount(const QString& accountId);

    void reselectAfterModelReset();

Q_SIGNALS:
    void requestCustomContextMenu(eMenu::Menu type, const QPoint& pos) const;
    void transactionSelectionChanged (const SelectedObjects& selection) const;
    void transactionSelected(const QModelIndex& idx) const;
    void aboutToStartEdit() const;
    void aboutToFinishEdit() const;
    void sectionResized(QWidget* view, const QString& configGroupName, int section, int oldSize, int newSize) const;
    void sectionMoved(QWidget* view, int section, int oldIndex, int newIndex) const;
    void requestView(QWidget* viewWidget, const QString& accountId, const QString& journalEntryId);
    void settingsChanged();
    void sortOrderChanged(QList<int> sortOrder);

    void modifySortOrder();

protected:
    class Private;
    Private * const d;
};
#endif // LEDGERVIEW_H

