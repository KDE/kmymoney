/***************************************************************************
 *   Copyright 2010  Cristian Onet onet.cristian@gmail.com                 *
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

/**
  * This view was created to handle the actions that could be performed with the accounts.
  */
class KMyMoneyAccountTreeView : public QTreeView
{
  Q_OBJECT

public:
  KMyMoneyAccountTreeView(QWidget *parent = 0);
  ~KMyMoneyAccountTreeView();

protected:
  void mouseDoubleClickEvent(QMouseEvent *event);

protected slots:
  void customContextMenuRequested(const QPoint &pos);
  void currentChanged(const QModelIndex &current, const QModelIndex &previous);

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

protected:
  bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

protected slots:
  void collapsed(const QModelIndex &index);
  void expanded(const QModelIndex &index);

private:
  class Private;
  Private* const d;
};

#endif // KMYMONEYACCOUNTTREEVIEW_H