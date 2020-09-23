/*
 * Copyright 2010-2014  Cristian Oneț <onet.cristian@gmail.com>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 * Copyright 2019       Thomas Baumgart <tbaumgart@kde.org>
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

#ifndef KMYMONEYACCOUNTTREEVIEW_H
#define KMYMONEYACCOUNTTREEVIEW_H

#include "kmm_base_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QTreeView>
#include <QScopedPointer>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyObject;
class AccountsProxyModel;
class KMyMoneyAccountTreeViewPrivate;

namespace eAccountsModel { enum class Column; }
namespace eView { enum class Intent; }
enum class View;

/**
  * This view was created to handle the actions that could be performed with the accounts.
  */
class KMM_BASE_WIDGETS_EXPORT KMyMoneyAccountTreeView : public QTreeView
{
  Q_OBJECT
  Q_DISABLE_COPY(KMyMoneyAccountTreeView)

public:
  explicit KMyMoneyAccountTreeView(QWidget* parent = nullptr);
  ~KMyMoneyAccountTreeView();

  AccountsProxyModel* proxyModel() const;

  /**
   * This method attaches the @a model to the view while
   * inserting the @sa proxyModel() in between them.
   */
  void setModel(QAbstractItemModel* model) override;

  /**
   * This method replaces the existing proxy model with @a model.
   * @a model will be reparented to the this object.
   *
   * @note It is advisable to replace this soon after construction
   * and not during operation.
   */
  void setProxyModel(AccountsProxyModel* model);

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
  const QScopedPointer<KMyMoneyAccountTreeViewPrivate> d_ptr;
  Q_DECLARE_PRIVATE(KMyMoneyAccountTreeView)
};

#endif // KMYMONEYACCOUNTTREEVIEW_H
