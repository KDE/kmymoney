/***************************************************************************
 *   Copyright 2010  Cristian Onet onet.cristian@gmail.com                 *
 *   Copyright 2017  Łukasz Wojniłowicz lukasz.wojnilowicz@gmail.com       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>  *
 ***************************************************************************/

#ifndef KMYMONEYACCOUNTTREEVIEW_H
#define KMYMONEYACCOUNTTREEVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QTreeView>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "accountsmodel.h"
#include <kmymoneyview.h>

class AccountsViewFilterProxyModel;
/**
  * This view was created to handle the actions that could be performed with the accounts.
  */
class KMyMoneyAccountTreeView : public QTreeView
{
  Q_OBJECT

public:
  KMyMoneyAccountTreeView(QWidget *parent = 0);
  ~KMyMoneyAccountTreeView();

  void init(QAbstractItemModel *model, QList<AccountsModel::Columns> *columns);

public slots:
  /**
    * Overridden for internal reasons. @sa collapsedAll().
    */
  void collapseAll();

  /**
    * Overridden for internal reasons. @sa expandedAll().
    */
  void expandAll();

  QList<AccountsModel::Columns> *getColumns(const KMyMoneyView::View view);

protected:
  void mouseDoubleClickEvent(QMouseEvent *event);
  void keyPressEvent(QKeyEvent *event);

protected slots:
  void customContextMenuRequested(const QPoint);
  void customHeaderContextMenuRequested(const QPoint);
  void slotColumnToggled(bool);
  void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

signals:
  /**
    * This signal serves as proxy for KMyMoneyAccountTree::selectObject()
    *
    * @param obj const reference to object
    */
  void selectObject(const MyMoneyObject& obj);

  /**
    * This signal serves as proxy for
    * KMyMoneyAccountTree::openContextMenu(const MyMoneyObject&)
    *
    * @param obj const reference to object
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
    * collapsedAll() and expandedAll() are emitted when the resp.
    * collapseAll() and expandAll() slots were invoked after the
    * base functionality of QTreeView::collapseAll()/QTreeView::expandAll()
    * has been finished.
    */
  void collapsedAll();
  void expandedAll();

  void columnToggled(const AccountsModel::Columns column, const bool show);

private:
  void openIndex(const QModelIndex &index);
  static QString getConfGrpName(const KMyMoneyView::View view);

private:
  QList<AccountsModel::Columns> *m_mdlColumns;
  QList<AccountsModel::Columns> m_visColumns;
  KMyMoneyView::View m_view;
  QMenu *m_menu;
};

/**
  * This model is specialized to organize the data for the accounts tree view
  * based on the data of the @ref AccountsModel.
  */
class AccountsViewFilterProxyModel : public AccountsFilterProxyModel
{
  Q_OBJECT

public:
  AccountsViewFilterProxyModel(QObject *parent = 0);
  ~AccountsViewFilterProxyModel();

  virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

public slots:
  void collapseAll();
  void expandAll();
  void collapsed(const QModelIndex &index);
  void expanded(const QModelIndex &index);

protected:
  bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

private:
  class Private;
  Private* const d;
};

#endif // KMYMONEYACCOUNTTREEVIEW_H
