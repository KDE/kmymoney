/*
 * Copyright 2006-2018  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#ifndef KMYMONEYSELECTOR_H
#define KMYMONEYSELECTOR_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>
#include <QTreeWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmm_widgets_export.h"

/**
  * This class implements a general selector for id based objects. It is based
  * on a tree view. Using this widget, one can select one or multiple
  * items depending on the mode of operation and the set of items
  * selected to be displayed. (see setSelectionMode() ).
  *
  * - Single selection mode\n
  *   In this mode the widget allows to select a single entry out of
  *   the set of displayed items.
  *
  * - Multi selection mode\n
  *   In this mode, the widget allows to select one or more entries
  *   out of the set of displayed items. Selection is performed
  *   by marking the item in the view.
  */
class KMyMoneySelectorPrivate;
class KMM_WIDGETS_EXPORT KMyMoneySelector : public QWidget
{
  Q_OBJECT
  Q_DISABLE_COPY(KMyMoneySelector)

  Q_PROPERTY(QStringList selectedItems READ selectedItems DESIGNABLE false STORED false)
public:
  explicit KMyMoneySelector(QWidget* parent = nullptr, Qt::WindowFlags flags = 0);
  virtual ~KMyMoneySelector();

  /**
    * This method sets the mode of operation of this widget.
    * Supported values are @p QListView::Single and @p QListView::Multi.
    *
    * @param mode @p QListView::Single selects single selection mode and
    *             @p QListView::Multi selects multi selection mode
    *
    * @note When the widget is created, it defaults to QListView::Single.
    *       Any items loaded into the widget will be cleared if the mode changes.
    *       Changing the selection mode also changes the type of the items
    *       created through newItem(). You should therefor set the selection mode
    *       before you create items.
    */
  void setSelectionMode(const QTreeWidget::SelectionMode mode);

  /**
    * returns the selection mode of the widget.
    *
    * @sa setSelectionMode()
    */
  QTreeWidget::SelectionMode selectionMode() const;

  /**
    * This method returns the list of selected item ids. If
    * no item is selected, the list is empty. The list is cleared
    * when the method is called.
    *
    * @param list reference to id list
    */
  void selectedItems(QStringList& list) const;

  /**
    * Convenience method for above method. Requires more resources.
    * Provided only for backward compatibility.
    *
    * @todo Deprecated after 1.0
    */
  QStringList selectedItems() const;

  /**
    * This method returns the list of all item ids.
    * The list is cleared when the method is called.
    *
    * @param list reference to id list
    */
  void itemList(QStringList& list) const;

  /**
    * Convenience method for above method. Requires more resources.
    * Provided only for backward compatibility.
    *
    * @todo Deprecated after 1.0
    */
  QStringList itemList() const;

  /**
    * This method returns an information if all items
    * currently shown are selected or not.
    *
    * @retval true All items shown are selected
    * @retval false Not all items are selected
    *
    * @note If the selection mode is set to Single, this
    *       method always returns false.
    */
  bool allItemsSelected() const;

  /**
    * This method sets the current selected item and marks the
    * checkbox according to @p state in multi-selection-mode.
    *
    * @param id id of account
    * @param state state of checkbox in multi-selection-mode
    *              @p true checked
    *              @p false not checked (default)
    */
  void setSelected(const QString& id, const bool state = false);

  /**
    * Return a pointer to the QTreeWidget object
    */
  QTreeWidget* listView() const;

  /**
    * This method selects/deselects all items that
    * are currently in the item list according
    * to the parameter @p state.
    *
    * @param state select items if @p true, deselect otherwise
    */
  void selectAllItems(const bool state);

  /**
    * This method selects/deselects all items that
    * are currently in this object's item list AND are present in the supplied
    * @p itemList of items to select, according to the @p state.
    *
    * @param itemList of item ids to apply @p state to
    * @param state select items if @p true, deselect otherwise
    */
  void selectItems(const QStringList& itemList, const bool state);

  /**
    * Protect an entry from selection. Protection is controlled by
    * the parameter @p protect.
    *
    * @param itemId id of item for which to modify the protection
    * @param protect if true, the entry specified by @p accId cannot be
    *                selected. If false, it can be selected. Defaults to @p true.
    */
  void protectItem(const QString& itemId, const bool protect = true);

  /**
    * This method removes an item with a given id from the list.
    *
    * @param id QString containing id of item to be removed
    */
  void removeItem(const QString& id);

  /**
    * This method creates a new top level KMyMoneyTreeWidgetItem object in the list view.
    * The type can be influenced with the @a type argument. It defaults
    * to QCheckListItem::RadioButtonController. If @a id is empty, the item is not
    * selectable. It will be shown 'opened' (see QListViewItem::setOpen())
    *
    * @return pointer to newly created object
    */
  QTreeWidgetItem* newItem(const QString& name, const QString& key, const QString& id);
  QTreeWidgetItem* newItem(const QString& name, const QString& key);
  QTreeWidgetItem* newItem(const QString& name);

  /**
    * Same as above, but create the item following the item pointed to by @c after.
    * If @c after is 0, then behave as previous method
    */
  QTreeWidgetItem* newItem(const QString& name, QTreeWidgetItem* after, const QString& key, const QString& id);
  QTreeWidgetItem* newItem(const QString& name, QTreeWidgetItem* after, const QString& key);
  QTreeWidgetItem* newItem(const QString& name, QTreeWidgetItem* after);

  /**
    * This method creates a new selectable object depending on the
    * selection mode. This is either a QTreeWidgetItem for single
    * selection mode or a KMyMoneyTreeWidgetItem for multi selection mode
    *
    * @note The new item will be the first one in the selection
    *
    * @param parent pointer to parent item
    * @param name the displayed name
    * @param key string to be used for completion. If empty defaults to @a name
    * @param id the id used to identify the objects
    *
    * @return pointer to newly created object
    */
  QTreeWidgetItem* newItem(QTreeWidgetItem* parent, const QString& name, const QString& key, const QString& id);

  /**
    * This method creates a new selectable object depending on the
    * selection mode. This is either a QTreeWidgetItem for single
    * selection mode or a KMyMoneyTreeWidgetItem for multi selection mode.
    * In contrast to the above method, the parent is always the view.
    *
    * @note The new item will be the first one in the selection
    *
    * @param name the displayed name
    * @param key string to be used for completion. If empty defaults to @a name
    * @param id the id used to identify the objects
    *
    * @return pointer to newly created object
    */
  QTreeWidgetItem* newTopItem(const QString& name, const QString& key, const QString& id);

  /**
    * This method checks if a given @a item matches the given regular expression @a exp.
    *
    * @param exp const reference to a regular expression object
    * @param item pointer to QListViewItem
    *
    * @retval true item matches
    * @retval false item does not match
    */
  virtual bool match(const QRegExp& exp, QTreeWidgetItem* item) const;

  /**
    * This method returns a pointer to the QListViewItem with the id @a id.
    * If such an item is not contained in the list, @a 0 will be returned.
    *
    * @param id id to be used to find a QListViewItem pointer for
    */
  QTreeWidgetItem* item(const QString& id) const;

  /**
    * This method returns, if any of the items in the selector contains
    * the text @a txt.
    *
    * @param txt const reference to string to be looked for
    * @retval true exact match found
    * @retval false no match found
    */
  virtual bool contains(const QString& txt) const;

  /**
    * Clears all items of the selector and the associated listview.
    */
  virtual void clear();

  /**
    * Sets the give item's selectable state.
    */
  void setSelectable(QTreeWidgetItem *item, bool selectable);

public Q_SLOTS:
  /**
    * This slot selects all items that are currently in
    * the item list of the widget.
    */
  void slotSelectAllItems();

  /**
    * This slot deselects all items that are currently in
    * the item list of the widget.
    */
  void slotDeselectAllItems();

Q_SIGNALS:
  void stateChanged();

  void itemSelected(const QString& id);

protected:
  /**
    * Helper method for selectedItems() to traverse the tree.
    *
    * @param list list of selected ids
    * @param item pointer to item to start with
    */
  void selectedItems(QStringList& list, QTreeWidgetItem* item) const;

  /**
    * Helper method for allItemsSelected() to traverse the tree.
    *
    * @param item pointer to item to start with
    */
  bool allItemsSelected(const QTreeWidgetItem *item) const;

  /**
    * This is a helper method for selectAllItems().
    *
    * @param item pointer to item to start with
    * @param state selection state (@a true = selected, @a false = not selected)
    */
  void selectAllSubItems(QTreeWidgetItem* item, const bool state);

  /**
    * This is a helper method for selectItems().
    *
    * @param item pointer to item to start with
    * @param itemList list of ids to be selected
    * @param state selection state (@a true = selected, @a false = not selected)
    */
  void selectSubItems(QTreeWidgetItem* item, const QStringList& itemList, const bool state);

public Q_SLOTS:
  /**
    * Hide all listview items that do not match the regular expression @a exp.
    * This method returns the number of visible items
    *
    * @param exp const reference to QRegExp that an item must match to stay visible
    *
    * @return number of visible items
    */
  int slotMakeCompletion(const QRegExp& exp);

  /**
    * This is an overloaded member function, provided for convenience. It behaves essentially like the above function.
    *
    * @param txt contains the pattern for a QRegExp
    */
  int slotMakeCompletion(const QString& txt);


protected Q_SLOTS:
  /**
    * This slot is connected to the QTreeWidget executed signal
    */
  void slotItemSelected(QTreeWidgetItem *it_v);

  /**
    * This slot processes the right mouse button press on a list view item.
    *
    * @param item pointer to the item where the mouse was pressed
    * @param col  the column of the tree (unused here)
    */
  void slotItemPressed(QTreeWidgetItem* item, int col);

protected:
  KMyMoneySelectorPrivate * const d_ptr;
  KMyMoneySelector(KMyMoneySelectorPrivate &dd, QWidget* parent = nullptr, Qt::WindowFlags flags = 0);

private:
  Q_DECLARE_PRIVATE(KMyMoneySelector)
};

#endif
