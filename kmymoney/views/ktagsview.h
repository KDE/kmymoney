/***************************************************************************
                          ktagsview.h
                          -------------
    begin                : Sat Oct 13 2012
    copyright            : (C) 2012 by Alessandro Russo <axela74@yahoo.it>

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

#include <QWidget>
#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_ktagsviewdecl.h"
#include "mymoneytag.h"

class KListWidgetSearchLine;

/**
  * @author Alessandro Russo
  */

/**
  * This class represents an item in the tags list view.
  */
class KTagListItem : public QListWidgetItem
{
public:
  /**
    * Constructor to be used to construct a tag entry object.
    *
    * @param parent pointer to the QListWidget object this entry should be
    *               added to.
    * @param tag    const reference to MyMoneyTag for which
    *               the QListWidget entry is constructed
    */
  KTagListItem(QListWidget *parent, const MyMoneyTag& tag);
  ~KTagListItem();

  const MyMoneyTag& tag() const {
    return m_tag;
  };

private:
  MyMoneyTag  m_tag;
};

class KTagsView : public QWidget, private Ui::KTagsViewDecl
{
  Q_OBJECT

public:
  KTagsView(QWidget *parent = 0);
  ~KTagsView();

  void setDefaultFocus();

  void showEvent(QShowEvent* event);

  enum filterTypeE {
    eAllTags = 0,
    eReferencedTags, // used tags
    eUnusedTags,     // unused tags
    eOpenedTags,     // not closed tags
    eClosedTags      // closed tags
  };

public slots:
  void slotSelectTagAndTransaction(const QString& tagId, const QString& accountId = QString(), const QString& transactionId = QString());
  void slotLoadTags();
  void slotStartRename(QListWidgetItem*);
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

  void slotTagNew();

  void slotRenameButtonCliked();

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

  /**
    * This signal is emitted whenever the view is about to be shown.
    */
  void aboutToShow();

private:
  MyMoneyTag   m_tag;
  QString      m_newName;

  /**
    * This member holds a list of all transactions
    */
  QList<QPair<MyMoneyTransaction, MyMoneySplit> > m_transactionList;


  /**
    * This member holds the state of the toggle switch used
    * to suppress updates due to MyMoney engine data changes
    */
  bool m_needReload;

  /**
    * This member holds the load state of page
    */
  bool m_needLoad;

  /**
    * Search widget for the list
    */
  KListWidgetSearchLine*  m_searchWidget;

  /**
   * Semaphore to suppress loading during selection
   */
  bool m_inSelection;

  /**
   * This signals whether a tag can be edited
   **/
  bool m_allowEditing;

  /**
    * This holds the filter type
    */
  int m_tagFilterType;

  AccountNamesFilterProxyModel *m_filterProxyModel;

  /** Initializes page and sets its load status to initialized
   */
  void init();
};

#endif
