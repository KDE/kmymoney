/***************************************************************************
                             hierarchyfilterproxymodel.cpp
                             -------------------
    begin                : Tue Sep 25 2006
    copyright            : (C) 2007 Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
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

#include "hierarchyfilterproxymodel.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "accountsmodel.h"
#include "accountsproxymodel.h"
#include "modelenums.h"

namespace NewAccountWizard
{
  HierarchyFilterProxyModel::HierarchyFilterProxyModel(QObject *parent)
    : AccountsProxyModel(parent)
  {
  }

  /**
  * Filter the favorites accounts group.
  */
  bool HierarchyFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
  {
    if (!source_parent.isValid()) {
        auto accCol = m_mdlColumns->indexOf(eAccountsModel::Column::Account);
        QVariant data = sourceModel()->index(source_row, accCol, source_parent).data((int)eAccountsModel::Role::ID);
        if (data.isValid() && data.toString() == AccountsModel::favoritesAccountId)
          return false;
      }
    return AccountsProxyModel::filterAcceptsRow(source_row, source_parent);
  }

  /**
  * Filter all but the first column.
  */
  bool HierarchyFilterProxyModel::filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const
  {
    Q_UNUSED(source_parent)
    if (source_column == 0)
      return true;
    return false;
  }
}
