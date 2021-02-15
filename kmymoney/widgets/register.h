/*
 * SPDX-FileCopyrightText: 2006-2018 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef REGISTER_H
#define REGISTER_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "transactioneditorcontainer.h"

class MyMoneyAccount;
class MyMoneySplit;
class MyMoneyTransaction;

namespace eWidgets { enum class SortField;
                     namespace eTransaction { enum class Column; }
                     namespace eRegister { enum class DetailColumn;} }
namespace eMyMoney { namespace Account { enum class Type; } }

template <typename T> class QList;

namespace KMyMoneyRegister
{
  class RegisterItem;
  class Transaction;
  class SelectedTransactions;

  class RegisterPrivate;
  class Register : public TransactionEditorContainer
  {
    Q_OBJECT
    Q_DISABLE_COPY(Register)

    friend class Transaction;
    friend class StdTransaction;
    friend class InvestTransaction;

  public:
    explicit Register(QWidget* parent = nullptr);
    virtual ~Register();

    /**
    * add the item @a p to the register
    */
    void addItem(RegisterItem* p);

    /**
    * insert the item @a p into the register after item @a q
    */
    void insertItemAfter(RegisterItem* p, RegisterItem* q);

    /**
    * remove the item @p from the register
    */
    void removeItem(RegisterItem* p);

    /**
    * This method returns a list of pointers to all selected items
    * in the register
    *
    * @retval QList<RegisterItem*>
    */
    QList<RegisterItem*> selectedItems() const;

    /**
    * Construct a list of all currently selected transactions in the register.
    * If the current item carrying the focus (see focusItem() ) is selected
    * it will be the first one contained in the list.
    *
    * @param list reference to QList receiving the SelectedTransaction()'s
    */
    void selectedTransactions(SelectedTransactions&  list) const;

    QString text(int row, int col) const;
    QWidget* createEditor(int row, int col, bool initFromCell) const;
    void setCellContentFromEditor(int row, int col);
    void endEdit();

    RegisterItem* focusItem() const;
    RegisterItem* anchorItem() const;

    /**
    * set focus to specific item.
    * @return true if the item got focus
    */
    bool setFocusItem(RegisterItem* focusItem);

    void setAnchorItem(RegisterItem* anchorItem);

    /**
    * Set focus to the first focussable item
    * @return true if a focussable item was found
    */
    bool setFocusToTop();

    /**
    * Select @a item and unselect all others if @a dontChangeSelections
    * is @a false. If m_buttonState differs from Qt::NoButton (method is
    * called as a result of a mouse button press), then the setting of
    * @a dontChangeSelections has no effect.
    */
    void selectItem(RegisterItem* item, bool dontChangeSelections = false);

    /**
    * Clears all items in the register. All objects
    * added to the register will be deleted.
    */
    void clear();

    void updateRegister(bool forceUpdateRowHeight = false);

    /**
    * Assign all visible items an alternate background color
    */
    void updateAlternate() const;

    /**
    * make sure, we only show a single marker in a row
    * through hiding unused ones
    */
    void suppressAdjacentMarkers();

    /**
    * Adjusts column @a col so that all data fits in width.
    */
    void adjustColumn(int col);

    /**
    * Convenience method to setup the register to show the columns
    * based on the account type of @a account. If @a showAccountColumn
    * is @a true then the account column is shown independent of the
    * account type. If @a account does not have an @a id, all columns
    * will be hidden.
    */
    void setupRegister(const MyMoneyAccount& account, bool showAccountColumn = false);

    /**
    * Show the columns contained in @a cols for @a account. @a account
    * can be left empty ( MyMoneyAccount() ) e.g. for the search dialog.
    */
    void setupRegister(const MyMoneyAccount& account, const QList<eWidgets::eTransaction::Column>& cols);

    void setSortOrder(const QString& order);
    const QList<eWidgets::SortField>& sortOrder() const;
    eWidgets::SortField primarySortKey() const;
    void sortItems();

    /**
    * This member returns the last visible column that is used by the register
    * after it has been setup using setupRegister().
    *
    * @return last actively used column (base 0)
    */
    eWidgets::eTransaction::Column lastCol() const;

    RegisterItem* firstItem() const;
    RegisterItem* firstVisibleItem() const;
    RegisterItem* nextItem(RegisterItem*) const;
    RegisterItem* lastItem() const;
    RegisterItem* lastVisibleItem() const;
    RegisterItem* prevItem(RegisterItem*) const;
    RegisterItem* itemAtRow(int row) const;

    void resize(int col, bool force = false);

    void forceUpdateLists();

    void ensureItemVisible(RegisterItem* item);

    void arrangeEditWidgets(QMap<QString, QWidget*>& editWidgets, Transaction* t) override;
    void removeEditWidgets(QMap<QString, QWidget*>& editWidgets) override;
    void tabOrder(QWidgetList& tabOrderWidgets, KMyMoneyRegister::Transaction* t) const override;

    int rowHeightHint() const;

    void clearSelection();

    /**
    * This method creates a specific transaction according to the
    * transaction passed in @a transaction.
    *
    * @param parent pointer to register where the created object should be added
    * @param transaction the transaction which should be used to create the object
    * @param split the split of the transaction which should be used to create the object
    * @param uniqueId an int that will be used to construct the id of the item
    *
    * @return pointer to created object (0 upon failure)
    */
    static Transaction* transactionFactory(Register *parent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId);

    const MyMoneyAccount& account() const;

    /**
    * This method creates group marker items and adds them to the register
    */
    void addGroupMarkers();

    /**
    * This method removes all trailing group markers and in a second
    * run reduces all adjacent group markers to show only one. In that
    * case the last one will remain.
    */
    void removeUnwantedGroupMarkers();

    void setLedgerLensForced(bool forced = true);
    bool ledgerLens() const;

    /**
    * Sets the selection mode to @a mode. Supported modes are QTable::Single and
    * QTable::Multi. QTable::Multi is the default when the object is created.
    */
    void setSelectionMode(SelectionMode mode);

    /**
    * This method sets a hint that the register instance will be used
    * with a transaction editor. This information is used while the column
    * sizes are being auto adjusted. If a transaction editor is used then
    * it's possible that it will need some extra space.
    */
    void setUsedWithEditor(bool value);

    eWidgets::eRegister::DetailColumn getDetailsColumnType() const;
    void setDetailsColumnType(eWidgets::eRegister::DetailColumn detailsColumnType);

  public Q_SLOTS:
      void ensureFocusItemVisible();

  protected:

    void mouseReleaseEvent(QMouseEvent *e) override;
    void contextMenuEvent(QContextMenuEvent *e) override;

    void unselectItems(int from = -1, int to = -1);
    void selectItems(int from, int to);
    void doSelectItems(int from, int to, bool selected);
    int selectedItemsCount() const;

    bool event(QEvent*) override;
    void focusOutEvent(QFocusEvent*) override;
    void focusInEvent(QFocusEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
    void resizeEvent(QResizeEvent* ev) override;

    int rowToIndex(int row) const;
    void setupItemIndex(int rowCount);

    /**
    * This method determines the register item that is one page
    * further down or up in the ledger from the previous focus item.
    * The height to scroll is determined by visibleHeight()
    *
    * @param key Qt::Page_Up or Qt::Page_Down depending on the direction to scroll
    * @param modifiers state of Qt::ShiftModifier, Qt::ControlModifier, Qt::AltModifier and
    *              Qt::MetaModifier.
    */
    void scrollPage(int key, Qt::KeyboardModifiers modifiers);

    /**
    * This method determines the pointer to a RegisterItem
    * based on the item's @a id. If @a id is empty, this method
    * returns @a m_lastItem.
    *
    * @param id id of the item to be searched
    * @return pointer to RegisterItem or 0 if not found
    */
    RegisterItem* itemById(const QString& id) const;

    /**
    * Override logic and use standard QFrame behaviour
    */
    bool focusNextPrevChild(bool next) override;

    bool eventFilter(QObject* o, QEvent* e) override;

    void handleItemChange(RegisterItem* old, bool shift, bool control);

    void selectRange(RegisterItem* from, RegisterItem* to, bool invert, bool includeFirst, bool clearSel);

    /**
    * Returns the minimum column width based on the data in the header and the transactions.
    */
    int minimumColumnWidth(int col);

  protected Q_SLOTS:
    void resize();

    void selectItem(int row, int col);
    void slotEnsureItemVisible();
    void slotDoubleClicked(int row, int);

  Q_SIGNALS:
    void transactionsSelected(const KMyMoneyRegister::SelectedTransactions& list);
    /**
    * This signal is emitted when the focus and selection changes to @p item.
    *
    * @param item pointer to transaction that received the focus and was selected
    */
    void focusChanged(KMyMoneyRegister::Transaction* item);

    /**
    * This signal is emitted when the focus changes but the selection remains
    * the same. This usually happens when the focus is changed using the keyboard.
    */
    void focusChanged();

    /**
   * This signal is emitted when an @p item is about to be selected. The boolean
   * @p okToSelect is preset to @c true. If the @p item should not be selected
   * for whatever reason, the boolean @p okToSelect should be reset to @c false
   * by the connected slot.
   */
    void aboutToSelectItem(KMyMoneyRegister::RegisterItem* item, bool& okToSelect);

    void editTransaction();

    /**
    * This signal is sent out when the user clicks on the ReconcileStateColumn and
    * only a single transaction is selected.
    */
    void reconcileStateColumnClicked(KMyMoneyRegister::Transaction* item);

    /**
    * This signal is sent out, if an item without a transaction id has been selected.
    */
    void emptyItemSelected();

    /**
    * This signal is sent out, if the user selects an item with the right mouse button
    */
    void openContextMenu();

    /**
    * This signal is sent out when a new item has been added to the register
    */
    void itemAdded(RegisterItem* item);

  private:
    RegisterPrivate * const d_ptr;
    Q_DECLARE_PRIVATE(Register)
  };

} // namespace

#endif
