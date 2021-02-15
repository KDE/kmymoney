/*
    SPDX-FileCopyrightText: 2012 Alessandro Russo <axela74@yahoo.it>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2020 Thomas Baugart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTAGSVIEW_H
#define KTAGSVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

class QItemSelection;

// ----------------------------------------------------------------------------
// KDE Includes

class KPageWidgetItem;

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyviewbase.h"

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
  void slotHelp();

  void updateActions(const SelectedObjects& selections) override;

protected:
  void showEvent(QShowEvent* event) override;
  void aboutToShow() override;
  void aboutToHide() override;

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
  void slotNewTag();
  void slotRenameTag();
  void slotDeleteTag();

  void slotModelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
};

#endif
