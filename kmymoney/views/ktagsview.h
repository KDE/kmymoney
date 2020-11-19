/*
 * Copyright 2012       Alessandro Russo <axela74@yahoo.it>
 * Copyright 2017       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 * Copyright 2020       Thomas Baugart <tbaumgart@kde.org>
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

class QItemSelection;
class MyMoneyTag;

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

  void updateTagActions(const QList<MyMoneyTag>& tags);
  void executeCustomAction(eView::Action action) override;

public Q_SLOTS:
  void slotSelectTagAndTransaction(const QString& tagId, const QString& accountId, const QString& transactionId);
  void slotSelectTagAndTransaction(const QString& tagId);
  void slotHelp();

protected:
  void showEvent(QShowEvent* event) override;
  QList<MyMoneyTag> selectedTags() const;

protected Q_SLOTS:
  /**
    * This slot is called whenever the selection in m_tagsList
    * has been changed.
    */
  void slotTagSelectionChanged (const QItemSelection& selected, const QItemSelection& deselected);

  /**
    * This slot marks the current selected tag as modified (dirty).
    */
  void slotTagDataChanged();

  /**
    * This slot is called when the name of a tag is changed inside
    * the tag list view and only a single tag is selected.
    */
  void slotRenameSingleTag(const QModelIndex& idx, const QVariant& value);

  /**
    * Updates the tag data in m_tag from the information in the
    * tag information widget.
    */
  void slotUpdateTag();

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

  void slotNewTag();
  void slotRenameTag();
  void slotDeleteTag();

  void slotModelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
};

#endif
