/***************************************************************************
                          hierarchyfilterproxymodel.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes <mte@users.sourceforge.net>
                               2004 by Thomas Baumgart <ipwizard@users.sourceforge.net>
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
// KDE Headers

// ----------------------------------------------------------------------------
// Project Includes

#include "accountsmodel.h"
#include "modelenums.h"

HierarchyFilterProxyModel::HierarchyFilterProxyModel(QObject *parent)
    : AccountsProxyModel(parent)
{
}

/**
  * The current account and all it's children are not selectable because the view is used to select a possible parent account.
  */
Qt::ItemFlags HierarchyFilterProxyModel::flags(const QModelIndex &index) const
{
  Qt::ItemFlags flags = AccountsProxyModel::flags(index);
  QModelIndex currentIndex = index;
  while (currentIndex.isValid()) {
    QVariant accountId = data(currentIndex, (int)eAccountsModel::Role::ID);
    if (accountId.isValid() && accountId.toString() == m_currentAccountId) {
      flags = flags & ~Qt::ItemIsSelectable & ~Qt::ItemIsEnabled;
    }
    currentIndex = currentIndex.parent();
  }
  return flags;
}

/**
  * Set the account for which to select a parent.
  *
  * @param currentAccountId The current account.
  */
void HierarchyFilterProxyModel::setCurrentAccountId(const QString &currentAccountId)
{
  m_currentAccountId = currentAccountId;
}

/**
  * Get the index of the selected parent account.
  *
  * @return The model index of the selected parent account.
  */
QModelIndex HierarchyFilterProxyModel::getSelectedParentAccountIndex() const
{
  QModelIndexList list = match(index(0, 0), (int)eAccountsModel::Role::ID, m_currentAccountId, -1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive | Qt::MatchRecursive));
  if (!list.empty()) {
    return list.front().parent();
  }
  return QModelIndex();
}

/**
  * Filter the favorites accounts group.
  */
bool HierarchyFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
  if (!source_parent.isValid()) {
    const auto data = sourceModel()->index(source_row, (int)eAccountsModel::Column::Account, source_parent).data((int)eAccountsModel::Role::ID);
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
