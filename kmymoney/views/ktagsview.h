/***************************************************************************
                          ktagsview.h
                          -------------
    begin                : Sat Oct 13 2012
    copyright            : (C) 2012 by Alessandro Russo <axela74@yahoo.it>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
  ~KTagsView();

  void setDefaultFocus() override;
  void refresh() override;

  void showEvent(QShowEvent* event);

public slots:
  void slotSelectTagAndTransaction(const QString& tagId, const QString& accountId, const QString& transactionId);
  void slotSelectTagAndTransaction(const QString& tagId);
  void slotStartRename(QListWidgetItem*);
  void slotRenameButtonCliked();
  void slotHelp();

protected:
  void loadTags();
  void selectedTags(QList<MyMoneyTag>& tagsList) const;
  void ensureTagVisible(const QString& id);
  void clearItemData();

protected slots:
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
  void slotRenameTag(QListWidgetItem *ta);

  /**
    * Updates the tag data in m_tag from the information in the
    * tag information widget.
    */
  void slotUpdateTag();

  void slotSelectTransaction();

  void slotChangeFilter(int index);

private slots:
  /**
    * This slot receives the signal from the listview control that an item was right-clicked,
    * If @p points to a real tag item, emits openContextMenu().
    *
    * @param p position of the pointer device
    */
  void slotOpenContextMenu(const QPoint& p);

signals:
  void transactionSelected(const QString& accountId, const QString& transactionId);
  void openContextMenu(const MyMoneyObject& obj);
  void selectObjects(const QList<MyMoneyTag>& tags);
  void tagNewClicked();
  void tagDeleteClicked();

private:
  Q_DISABLE_COPY(KTagsView)
  Q_DECLARE_PRIVATE(KTagsView)
};

#endif
