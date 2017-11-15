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

#include "mymoneyenums.h"
#include "viewenums.h"

namespace eAccountsModel {
  enum class Column;
}

class AccountsViewProxyModel;
class MyMoneyObject;

/**
  * This view was created to handle the actions that could be performed with the accounts.
  */
class KMyMoneyAccountTreeView : public QTreeView
{
  Q_OBJECT

public:
  KMyMoneyAccountTreeView(QWidget *parent = nullptr);
  ~KMyMoneyAccountTreeView();

  AccountsViewProxyModel *init(View view);

protected:
  void mouseDoubleClickEvent(QMouseEvent *event);
  void keyPressEvent(QKeyEvent *event);

protected slots:
  void customContextMenuRequested(const QPoint);
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

  void columnToggled(const eAccountsModel::Column column, const bool show);

private:
  void openIndex(const QModelIndex &index);
  static QString getConfGrpName(const View view);
  QSet<eAccountsModel::Column> readVisibleColumns(const View view);
  QVector<eMyMoney::Account> getVisibleGroups(const View view);

  AccountsViewProxyModel   *m_model;
  View                     m_view;
};

#endif // KMYMONEYACCOUNTTREEVIEW_H
