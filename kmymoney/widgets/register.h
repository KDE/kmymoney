/***************************************************************************
                             register.h
                             ----------
    begin                : Fri Mar 10 2006
    copyright            : (C) 2006 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef REGISTER_H
#define REGISTER_H


// ----------------------------------------------------------------------------
// QT Includes

#include <QVector>
#include <QWidget>
#include <QMap>
#include <QList>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyaccount.h"
#include "registeritem.h"
#include "transaction.h"
#include "transactioneditorcontainer.h"
#include "selectedtransaction.h"

class MyMoneyTransaction;

namespace KMyMoneyRegister
{

typedef enum {
  UnknownSort = 0,      ///< unknown sort criteria
  PostDateSort = 1,     ///< sort by post date
  EntryDateSort,        ///< sort by entry date
  PayeeSort,            ///< sort by payee name
  ValueSort,            ///< sort by value
  NoSort,               ///< sort by number field
  EntryOrderSort,       ///< sort by entry order
  TypeSort,             ///< sort by CashFlowDirection
  CategorySort,         ///< sort by Category
  ReconcileStateSort,   ///< sort by reconciliation state
  SecuritySort,         ///< sort by security (only useful for investment accounts)
  // insert new values in front of this line
  MaxSortFields
} TransactionSortField;

typedef enum {
  AscendingOrder = 0,   ///< sort in ascending order
  DescendingOrder       ///< sort in descending order
} SortDirection;

typedef enum {
  PayeeFirst = 0,       ///< show the payee on the first row of the transaction in the details column and the account on the second
  AccountFirst          ///< show the account on the first row of the transaction in the details column and the payee on the second
} DetailsColumnType;

class Register;
class RegisterItem;
class ItemPtrVector;

const QString sortOrderToText(TransactionSortField idx);
TransactionSortField textToSortOrder(const QString& text);


class QWidgetContainer : public QMap<QString, QWidget*>
{
public:
  QWidgetContainer() {}

  QWidget* haveWidget(const QString& name) const {
    QWidgetContainer::const_iterator it_w;
    it_w = find(name);
    if (it_w != end())
      return *it_w;
    return 0;
  }

  void removeOrphans() {
    QWidgetContainer::iterator it_w;
    for (it_w = begin(); it_w != end();) {
      if ((*it_w) && (*it_w)->parent())
        ++it_w;
      else {
        delete(*it_w);
        remove(it_w.key());
        it_w = begin();
      }
    }
  }

};

class GroupMarker : public RegisterItem
{
public:
  explicit GroupMarker(Register* parent, const QString& txt = QString());
  ~GroupMarker();
  void setText(const QString& txt) {
    m_txt = txt;
  }
  const QString& text() const {
    return m_txt;
  }
  bool isSelectable() const {
    return false;
  }
  bool canHaveFocus() const {
    return false;
  }
  int numRows() const {
    return 1;
  }

  virtual const char* className() {
    return "GroupMarker";
  }

  bool isErroneous() const {
    return m_erroneous;
  }

  void paintRegisterCell(QPainter *painter, QStyleOptionViewItem &option, const QModelIndex &index);
  void paintFormCell(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index);

  int rowHeightHint() const;

  bool matches(const RegisterFilter&) const {
    return true;
  }
  virtual int sortSamePostDate() const {
    return 0;
  }

  void setErroneous(bool condition = true) {
    m_erroneous = condition;
  }

protected:
  QString                  m_txt;
  bool                     m_showDate;

  static QPixmap*          m_bg;
  static int               m_bgRefCnt;

  bool                     m_erroneous;
};


class FancyDateGroupMarker : public GroupMarker
{
public:
  FancyDateGroupMarker(Register* parent, const QDate& date, const QString& txt);

  QDate sortPostDate() const override {
    return m_date;
  }
  QDate sortEntryDate() const override {
    return m_date;
  }
  virtual const char* className() {
    return "FancyDateGroupMarker";
  }
private:
  QDate                    m_date;
};

class StatementGroupMarker : public FancyDateGroupMarker
{
public:
  StatementGroupMarker(Register* parent, CashFlowDirection dir, const QDate& date, const QString& txt);
  CashFlowDirection sortType() const {
    return m_dir;
  }
  virtual int sortSamePostDate() const {
    return 3;
  }
private:
  CashFlowDirection        m_dir;
};

class SimpleDateGroupMarker : public FancyDateGroupMarker
{
public:
  SimpleDateGroupMarker(Register* parent, const QDate& date, const QString& txt);
  void paintRegisterCell(QPainter *painter, QStyleOptionViewItem &option, const QModelIndex &index);
  int rowHeightHint() const;
  virtual const char* className() {
    return "SimpleDateGroupMarker";
  }
};

class TypeGroupMarker : public GroupMarker
{
public:
  TypeGroupMarker(Register* parent, CashFlowDirection dir, eMyMoney::Account accType);
  CashFlowDirection sortType() const {
    return m_dir;
  }
private:
  CashFlowDirection        m_dir;
};

class FiscalYearGroupMarker : public FancyDateGroupMarker
{
public:
  FiscalYearGroupMarker(Register* parent, const QDate& date, const QString& txt);
  virtual const char* className() {
    return "FiscalYearGroupMarker";
  }
  virtual int sortSamePostDate() const {
    return 1;
  }
};

class PayeeGroupMarker : public GroupMarker
{
public:
  PayeeGroupMarker(Register* parent, const QString& name);
  const QString& sortPayee() const {
    return m_txt;
  }
};

class CategoryGroupMarker : public GroupMarker
{
public:
  CategoryGroupMarker(Register* parent, const QString& category);
  const QString& sortCategory() const {
    return m_txt;
  }
  const QString sortSecurity() const {
    return m_txt;
  }

  virtual const char* className() {
    return "CategoryGroupMarker";
  }
};

class ReconcileGroupMarker : public GroupMarker
{
public:
  ReconcileGroupMarker(Register* parent, eMyMoney::Split::State state);
  virtual eMyMoney::Split::State sortReconcileState() const {
    return m_state;
  }
private:
  eMyMoney::Split::State  m_state;
};


class ItemPtrVector : public QVector<RegisterItem *>
{
public:
  ItemPtrVector() {}

  void sort();

protected:
  /**
    * sorter's compare routine. Returns true if i1 < i2
    */
  static bool item_cmp(RegisterItem* i1, RegisterItem* i2);
};

class RegisterItemDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:
  explicit RegisterItemDelegate(Register *parent);
  ~RegisterItemDelegate();

  void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;


private:
  Register *m_register;
};

class Register : public TransactionEditorContainer
{
  Q_OBJECT

  friend class Transaction;
  friend class StdTransaction;
  friend class InvestTransaction;

public:
  Register(QWidget *parent = 0);
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
  void endEdit(int row, int col, bool accept, bool replace);

  RegisterItem* focusItem() const {
    return m_focusItem;
  }
  RegisterItem* anchorItem() const {
    return m_selectAnchor;
  }

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
  void setupRegister(const MyMoneyAccount& account, const QList<Column>& cols);

  void setSortOrder(const QString& order);
  const QList<TransactionSortField>& sortOrder() const {
    return m_sortOrder;
  }
  TransactionSortField primarySortKey() const;
  void sortItems();

  /**
    * This member returns the last visible column that is used by the register
    * after it has been setup using setupRegister().
    *
    * @return last actively used column (base 0)
    */
  Column lastCol() const {
    return m_lastCol;
  }

  RegisterItem* firstItem() const;
  RegisterItem* firstVisibleItem() const;
  RegisterItem* nextItem(RegisterItem*) const;
  RegisterItem* lastItem() const;
  RegisterItem* lastVisibleItem() const;
  RegisterItem* prevItem(RegisterItem*) const;
  RegisterItem* itemAtRow(int row) const;

  void resize(int col, bool force = false);

  void forceUpdateLists() {
    m_listsDirty = true;
  }

  void ensureItemVisible(RegisterItem* item);

  void arrangeEditWidgets(QMap<QString, QWidget*>& editWidgets, Transaction* t);
  void removeEditWidgets(QMap<QString, QWidget*>& editWidgets);
  void tabOrder(QWidgetList& tabOrderWidgets, KMyMoneyRegister::Transaction* t) const;

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

  const MyMoneyAccount& account() const {
    return m_account;
  }

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

  void setLedgerLensForced(bool forced = true) {
    m_ledgerLensForced = forced;
  }

  /**
    * Sets the selection mode to @a mode. Supported modes are QTable::Single and
    * QTable::Multi. QTable::Multi is the default when the object is created.
    */
  void setSelectionMode(SelectionMode mode) {
    m_selectionMode = mode;
  }

  /**
    * This method sets a hint that the register instance will be used
    * with a transaction editor. This information is used while the column
    * sizes are being auto adjusted. If a transaction editor is used then
    * it's possible that it will need some extra space.
    */
  void setUsedWithEditor(bool value) {
    m_usedWithEditor = value;
  }

  DetailsColumnType getDetailsColumnType() const;
  void setDetailsColumnType(DetailsColumnType detailsColumnType);

protected:

  void mouseReleaseEvent(QMouseEvent *e);
  void contextMenuEvent(QContextMenuEvent *e);

  void unselectItems(int from = -1, int to = -1) {
    doSelectItems(from, to, false);
  }
  void selectItems(int from, int to) {
    doSelectItems(from, to, true);
  }
  void doSelectItems(int from, int to, bool selected);
  int selectedItemsCount() const;

  bool event(QEvent*);
  void focusOutEvent(QFocusEvent*);
  void focusInEvent(QFocusEvent*);
  void keyPressEvent(QKeyEvent*);
  virtual void resizeEvent(QResizeEvent* ev);

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
  bool focusNextPrevChild(bool next);

  bool eventFilter(QObject* o, QEvent* e);

  void handleItemChange(RegisterItem* old, bool shift, bool control);

  void selectRange(RegisterItem* from, RegisterItem* to, bool invert, bool includeFirst, bool clearSel);

  /**
    * Returns the minimum column width based on the data in the header and the transactions.
    */
  int minimumColumnWidth(int col);

protected slots:
  void resize();

  void selectItem(int row, int col);
  void slotEnsureItemVisible();
  void slotDoubleClicked(int row, int);

signals:
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

protected:
  ItemPtrVector                m_items;
  QVector<RegisterItem*>       m_itemIndex;
  RegisterItem*                m_selectAnchor;
  RegisterItem*                m_focusItem;
  RegisterItem*                m_ensureVisibleItem;
  RegisterItem*                m_firstItem;
  RegisterItem*                m_lastItem;
  RegisterItem*                m_firstErroneous;
  RegisterItem*                m_lastErroneous;

  int                          m_markErroneousTransactions;
  int                          m_rowHeightHint;

  MyMoneyAccount               m_account;

  bool                         m_ledgerLensForced;
  SelectionMode                m_selectionMode;

private:
  bool                         m_needResize;
  bool                         m_listsDirty;
  bool                         m_ignoreNextButtonRelease;
  bool                         m_needInitialColumnResize;
  bool                         m_usedWithEditor;
  Qt::MouseButtons             m_mouseButton;
  Qt::KeyboardModifiers        m_modifiers;
  Column                       m_lastCol;
  QList<TransactionSortField>  m_sortOrder;
  QRect                        m_lastRepaintRect;
  DetailsColumnType            m_detailsColumnType;
};

} // namespace

#endif
