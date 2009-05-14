/***************************************************************************
                         kmymoneyaccounttreebase.h  -  description
                            -------------------
   begin                : Sat Jan 1 2005
   copyright            : (C) 2005 by Thomas Baumgart
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

#ifndef KMYMONEYACCOUNTTREEBASE_H
#define KMYMONEYACCOUNTTREEBASE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qtimer.h>
//Added by qt3to4:
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QPaintEvent>
#include <Q3ValueList>
class Q3DragObject;

// ----------------------------------------------------------------------------
// KDE Includes

#include <k3listview.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyaccount.h>
#include <mymoneyinstitution.h>
#include <mymoneyprice.h>
#include <mymoneysecurity.h>
#include <mymoneybudget.h>

class KMyMoneyAccountTreeBaseItem;

class KMyMoneyAccountTreeBase : public K3ListView
{
  friend class KMyMoneyAccountTreeBaseItem;

  Q_OBJECT
public:
  KMyMoneyAccountTreeBase(QWidget* parent = 0, const char *name = 0);
  virtual ~KMyMoneyAccountTreeBase();

  /**
    * Modify the text shown in the header of the name column.
    *
    * @param txt the text to be used in the header
    */
  void setSectionHeader(const QString& txt);

  /**
    * overridden from base class implementation to return a pointer
    * to a KMyMoneyAccountTreeBaseItem.
    *
    * @return pointer to currently selected item
    */
  KMyMoneyAccountTreeBaseItem* selectedItem(void) const;

  /**
    */
  void setBaseCurrency(const MyMoneySecurity& currency) { m_baseCurrency = currency; };

  const MyMoneySecurity& baseCurrency(void) const { return m_baseCurrency; };

  void emitValueChanged(void) { emit valueChanged(); };

  /**
    * restores the layout from the config file
    * @param group the group to be used from the config file.
    * At destruction time, we will use this group name to save
    * the layout.
    */
  void restoreLayout(const QString& group);

public slots:
  /** autoscroll support */
  void slotStartAutoScroll(void);
  void slotStopAutoScroll(void);
  void slotExpandAll(void);
  void slotCollapseAll(void);

protected:
  virtual bool acceptDrag (QDropEvent* event) const;
  virtual void startDrag(void);
  const KMyMoneyAccountTreeBaseItem* findItem(const QString& id) const;

  /**
    * This method checks, if account @p accFrom can be dropped onto
    * account @p accTo.
    *
    * @param accFrom source account
    * @param accTo new parent account for @p accFrom
    * @retval true drop is ok
    * @retval false drop is not ok (@p accTo cannot be parent of @p accFrom)
    */
  bool dropAccountOnAccount(const MyMoneyAccount& accFrom, const MyMoneyAccount& accTo) const;
  // virtual void contentsDropEvent(QDropEvent*);

  /**
    * This member counts the connects to the signals
    * newAccountParent(const MyMoneyAccount&, const MyMoneyAccount&)) and
    * newAccountParent(const MyMoneyAccount&, const MyMoneyInstitution&))
    * in m_accountConnections and m_institutionConnections.
    */
  void connectNotify(const char *);

  /**
    * This member counts the disconnects from the signals
    * newAccountParent(const MyMoneyAccount&, const MyMoneyAccount&)) and
    * newAccountParent(const MyMoneyAccount&, const MyMoneyInstitution&))
    * in m_accountConnections and m_institutionConnections.
    */
  void disconnectNotify(const char *);

  void contentsDragMoveEvent( QDragMoveEvent *e );

  /**
   * Reimplemented for internal reasons.
   * Further reimplementations should call this function or else
   * some features may not work correctly.
   *
   * The API is unaffected.
   */
  virtual void viewportPaintEvent(QPaintEvent*);

  void expandCollapseAll(bool expand);

  void queueSort(void);

protected slots:
  void slotObjectDropped(QDropEvent* event, Q3ListViewItem* parent, Q3ListViewItem* after);

  /**
    * Select the object pointed to by @p i. This slot emits selectObject signals
    * with an emtpy MyMoneyAccount and an empty MyMoneyInstitution object
    * to deselect current selections. If @p i points to a KMyMoneyAccountTreeItem
    * object, it emits selectObject() for this item.
    *
    * @param i pointer to QListViewItem of object to be selected
    */
  void slotSelectObject(Q3ListViewItem *i);

  /**
    * This slot is connected to the accout list view's contextMenu signal
    * and checks if the item pointed to by @p i is either an account or institution
    * and sends out the necessary signal openContextMenu.
    *
    * @param lv pointer to K3ListView
    * @param i pointer to QListViewItem
    * @param p position information
    */
  void slotOpenContextMenu(K3ListView* lv, Q3ListViewItem* i, const QPoint& p);

  /**
    * This slot is connected to the accout list view's executed signal
    * and checks if the item pointed to by @p i is either an account or institution
    * and sends out the necessary signal openObject.
    *
    * @param i pointer to QListViewItem
    */
  void slotOpenObject(Q3ListViewItem* i);

  void slotAutoScroll(void);

  /** Open the folder pointed to by m_dropItem */
  void slotOpenFolder(void);

  /** override K3ListView implementation */
  void cleanItemHighlighter(void);

  void slotActivateSort(void);

private:
  MyMoneySecurity     m_baseCurrency;
  bool                m_accountConnections;
  bool                m_institutionConnections;
  QTimer              m_autoopenTimer;
  QTimer              m_autoscrollTimer;
  int                 m_autoscrollTime;
  int                 m_autoscrollAccel;
  Q3ListViewItem*      m_dropItem;
  QRect               m_lastDropHighlighter;
  int                 m_queuedSort;
  int                 m_nameColumn;
  int                 m_typeColumn;
  int                 m_valueColumn;
  int                 m_balanceColumn;
  QString             m_configGroup;

public:
  int typeColumn(void) const { return m_typeColumn; }
  int nameColumn(void) const { return m_nameColumn; }
  int balanceColumn(void) const { return m_balanceColumn; }
  int valueColumn(void) const { return m_valueColumn; }

  void showType(void);
  void showValue(void);


signals:
  /**
    * This signal is emitted whenever an object in the view is selected
    *
    * @param obj reference to actual MyMoneyObject (is either
    *            MyMoneyAccount or MyMoneyInstitution depending on selected item)
    */
  void selectObject(const MyMoneyObject& obj);

  /**
    * This signal is emitted whenever the user requests the context menu for an object
    *
    * @param obj reference to actual MyMoneyObject (is either
    *            MyMoneyAccount or MyMoneyInstitution depending on selected item)
    */
  void openContextMenu(const MyMoneyObject& obj);

  /**
    * This signal is emitted whenever the user requests to open an object
    *
    * @param obj reference to actual MyMoneyObject (is either
    *            MyMoneyAccount or MyMoneyInstitution depending on selected item)
    */
  void openObject(const MyMoneyObject& obj);

  /**
    * This signal is emitted whenever the value of an object changed
    */
  void valueChanged(void);

  /**
    * This signal is emitted, when the user selected to reparent the
    * account @p acc to be a subordinate account of @p parent.
    *
    * @param acc const reference to account to be reparented
    * @param parent const reference to new parent account
    */
  void reparent(const MyMoneyAccount& acc, const MyMoneyAccount& parent);

  /**
    * This signal is emitted, when the user selected to reparent the
    * account @p acc to be a subordinate account of @p institution.
    *
    * @param acc const reference to account to be reparented
    * @param institution const reference to new institution
    */
  void reparent(const MyMoneyAccount& acc, const MyMoneyInstitution& institution);
};

class KMyMoneyAccountTreeBaseItem : public K3ListViewItem
{
public:
  typedef enum {
    Account,
    Institution
  } KMyMoneyAccountTreeItemType;

  /**
    * Constructor to be used to construct an institution entry
    * object.
    *
    * @param parent pointer to the K3ListView object this entry should be
    *               added to.
    * @param institution const reference to MyMoneyInstitution for which
    *               the K3ListView entry is constructed
    */
   KMyMoneyAccountTreeBaseItem(K3ListView *parent, const MyMoneyInstitution& institution);

  /**
    * Constructor to be used to construct a standard account entry object (e.g. Asset,
    * Liability, etc.).
    *
    * @param parent pointer to the K3ListView object this entry should be
    *               added to.
    * @param account const reference to MyMoneyAccount for which
    *               the K3ListView entry is constructed
    * @param security const reference to the security used to show the value. Usually
    *                 one should pass MyMoneyFile::baseCurrency() here.
    * @param name name of the account to be used instead of the one stored with @p account
    *               If empty, the one stored with @p account will be used. Default: empty
    */
   KMyMoneyAccountTreeBaseItem(K3ListView *parent, const MyMoneyAccount& account, const MyMoneySecurity& security = MyMoneySecurity(), const QString& name = QString());

  /**
    * Constructor to be used to construct an account entry
    * object.
    *
    * @param parent pointer to the parent KAccountListView object this entry should be
    *               added to.
    * @param account const reference to MyMoneyAccount for which
    *               the K3ListView entry is constructed
    * @param price price to be used to calculate value (defaults to 1)
    *              This is used for accounts denominated in foreign currencies or stocks
    * @param security const reference to the security used to show the value. Usually
    *                 one should pass MyMoneyFile::baseCurrency() here.
    */
  KMyMoneyAccountTreeBaseItem(KMyMoneyAccountTreeBaseItem *parent, const MyMoneyAccount& account, const Q3ValueList<MyMoneyPrice>& price = Q3ValueList<MyMoneyPrice>(), const MyMoneySecurity& security = MyMoneySecurity());

  ~KMyMoneyAccountTreeBaseItem();

  /**
    * populates the columns. Derived classes should override this. The
    * name column is already filled and should not be changed.
    */
  virtual void fillColumns();

  /**
    * This method loads new information into the item and updates the fields
    *
    * @param forceTotalUpdate set to true to force update of total values
    *                         (used in constructor, should not be necessary to
    *                          be set by application code)
    *
    */
  void updateAccount(bool forceTotalUpdate = false);

  /**
    * This method checks, if the item contains an account or not.
    *
    * @retval true item holds an account
    * @retval false item does not hold an account
    */
  bool isAccount(void) const { return m_type == Account; };

  /**
    * This method checks, if the item contains an institution or not.
    *
    * @retval true item holds an institution
    * @retval false item does not hold an institution
    */
  bool isInstitution(void) const { return m_type == Institution; };

  /**
    * This method returns the id of the object held by this item
    *
    * @return const reference to id of object
    */
  const QString& id(void) const;

  /**
    * Helper method to show the right order
    */
  int compare(Q3ListViewItem* i, int col, bool ascending) const;

  /**
    * If o is TRUE all child items are shown initially. The user can
    * hide them by clicking the - icon to the left of the item. If
    * o is FALSE, the children of this item are initially hidden.
    * The user can show them by clicking the + icon to the left of the item.
    *
    * Overrides K3ListViewItem::setOpen() and exchanges the value field
    * with either the value of this account and its subaccounts if @p o
    * is false or the value of this account if @p o is true.
    *
    * @param o show item open (true) or closed (false)
    */
  virtual void setOpen(bool o);

  /**
    * This method is re-implemented from QListViewItem::paintCell().
    * Besides the standard implementation, the QPainter is set
    * according to the applications settings.
    */
  void paintCell(QPainter *p, const QColorGroup & cg, int column, int width, int align);

  /**
    * Convenience method to return casted pointer
    */
  KMyMoneyAccountTreeBase* listView(void) const { return dynamic_cast<KMyMoneyAccountTreeBase*>(K3ListViewItem::listView()); };

  /**
    * Return the type of entry
    *
    * @return type of this entry.
    */
  KMyMoneyAccountTreeItemType entryType(void) const { return m_type; };

  /**
    * This method returns a const reference to this item (either the m_account or m_institution)
    * depending on m_type.
    *
    * @return reference to the MyMoneyObject of this entry
    */
  const MyMoneyObject& itemObject(void) const;

  /**
    * This method returns the value of this account and all it's subordinate accounts.
    *
    * @return value of this account including all subordinate accounts
    */
  const MyMoneyMoney& totalValue(void) const { return m_totalValue; };

  /**
    * This method adjusts the current total value by @p diff.
    *
    * @param diff difference to be added to the current value to
    *             get the new value
    */
  void adjustTotalValue(const MyMoneyMoney& diff);

  /**
    * Checks whether this object is a child of the one passed
    * by @p item.
    *
    * @param item pointer to other KMyMoneyAccountTreeItem that
    *        should be checked for parent/grand-parenthood of this
    *        object
    * @retval true @p this object is a decendant of @p item
    * @retval false @p this object is no decendant of @p item
    */
  bool isChildOf(const Q3ListViewItem* const item) const;

  /**
    * Sets the whole item to be shown with negative colors
    */
  void setNegative(bool isNegative);

  /**
   * Sets the text of a given column. @param negative indicates whether it should
   * be shown as negative number or not
   */
  void setText( int column, const QString &text, const bool &negative = false );

protected:
   /**
    * Returns the current balance of this account.
    *
    * This is a pure virtual function, to allow subclasses to calculate
    * the balance in different ways.
    *
    * Parent items in the tree will only be recomputed if the balance() for
    * a child changes.
    * @param account Account to get the balance for
    * @return Balance of this account
   */
  virtual MyMoneyMoney balance() const = 0;

  /**
   * Computes and returns the current value of the account held by this item.
   * This is the same as balance() but in the currency of the view.
   * if value() changed since the item has been displayed, updateAccount()
   * will notify the parent.
   * @return value of the account held by this item
   */
  MyMoneyMoney value() const;

protected:
  MyMoneyMoney                      m_value;
  Q3ValueList<MyMoneyPrice>          m_price;
  MyMoneySecurity                   m_security;
  MyMoneyMoney                      m_totalValue;
  MyMoneyAccount                    m_account;
  QMap<int, QColor>                 m_columnsColor;
  bool                              m_negative;

private:
  MyMoneyInstitution                m_institution;
  KMyMoneyAccountTreeItemType       m_type;

  /**
   * fills the name column with text and pixmap
   */
  void setName();

};

#endif

