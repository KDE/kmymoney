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

class MyMoneyObject;
class AccountsViewProxyModel;

namespace eAccountsModel { enum class Column; }
namespace eView { enum class Intent; }
enum class View;

/**
  * This view was created to handle the actions that could be performed with the accounts.
  */
class KMyMoneyAccountTreeViewPrivate;
class KMyMoneyAccountTreeView : public QTreeView
{
  Q_OBJECT
  Q_DISABLE_COPY(KMyMoneyAccountTreeView)

public:
  explicit KMyMoneyAccountTreeView(QWidget* parent = nullptr);
  ~KMyMoneyAccountTreeView();

  AccountsViewProxyModel *init(View view);

protected:
  void mouseDoubleClickEvent(QMouseEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;

protected Q_SLOTS:
  void customContextMenuRequested(const QPoint);
  void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;

Q_SIGNALS:
  void selectByObject(const MyMoneyObject&, eView::Intent);
  void selectByVariant(const QVariantList&, eView::Intent);

private:
  KMyMoneyAccountTreeViewPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KMyMoneyAccountTreeView)

private Q_SLOTS:
  void slotColumnToggled(const eAccountsModel::Column column, const bool show);
};

#endif // KMYMONEYACCOUNTTREEVIEW_H
