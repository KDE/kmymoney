/*
 * Copyright 2012       Alessandro Russo <axela74@yahoo.it>
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

#ifndef KTAGSVIEW_H
#define KTAGSVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyviewbase.h"

class QListWidgetItem;
class KListWidgetSearchLine;
class MyMoneyObject;
class MyMoneyTag;
class SelectedObjects;

template <typename T> class QList;

/**
  * @author Alessandro Russo
  */

class KTagsViewPrivate;
class KTagsView : public KMyMoneyViewBase
{
  Q_OBJECT

public:
  explicit KTagsView(QWidget *parent = nullptr);
  ~KTagsView() override;

  void executeCustomAction(eView::Action action) override;

public Q_SLOTS:
  void slotSelectTagAndTransaction(const QString& tagId, const QString& accountId, const QString& transactionId);
  void slotSelectTagAndTransaction(const QString& tagId);
  void slotStartRename(QListWidgetItem*);
  void slotHelp();

  void refresh();
  void updateActions(const SelectedObjects& selections) override;

protected:
  void showEvent(QShowEvent* event) override;
  void loadTags();
  void selectedTags(QList<MyMoneyTag>& tagsList) const;
  void ensureTagVisible(const QString& id);
  void clearItemData();

protected Q_SLOTS:
  /**
    * This method loads the m_transactionList, clears
    * the m_TransactionPtrVector and rebuilds and sorts
    * it according to the current settings. Then it
    * loads the m_transactionView with the transaction data.
    */
  void showTransactions();

  /**
    * This slot is called whenever the selection in m_tagsList
    * is about to change.
    */
  void slotSelectTag(QListWidgetItem* cur, QListWidgetItem* prev);

  /**
    * This slot is called whenever the selection in m_tagsList
    * has been changed.
    */
  void slotSelectTag();

  /**
    * This slot marks the current selected tag as modified (dirty).
    */
  void slotTagDataChanged();

  /**
    * This slot is called when the name of a tag is changed inside
    * the tag list view and only a single tag is selected.
    */
  void slotRenameSingleTag(QListWidgetItem *ta);

  /**
    * Updates the tag data in m_tag from the information in the
    * tag information widget.
    */
  void slotUpdateTag();

  void slotSelectTransaction();

  void slotChangeFilter(int index);

Q_SIGNALS:
  void transactionSelected(const QString& accountId, const QString& transactionId);

private:
  Q_DISABLE_COPY(KTagsView)
  Q_DECLARE_PRIVATE(KTagsView)

private Q_SLOTS:
  /**
    * This slot receives the signal from the listview control that an item was right-clicked,
    * If @p points to a real tag item, emits openContextMenu().
    *
    * @param p position of the pointer device
    */
  void slotShowTagsMenu(const QPoint& p);

  void slotSelectTags(const QList<MyMoneyTag>& list);

  void slotNewTag();
  void slotRenameTag();
  void slotDeleteTag();
};

#endif
