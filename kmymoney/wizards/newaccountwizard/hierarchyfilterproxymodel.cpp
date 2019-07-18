/***************************************************************************
                             hierarchyfilterproxymodel.cpp
                             -------------------
    begin                : Tue Sep 25 2006
    copyright            : (C) 2007 Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
                           (C) 2017-2018 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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
#include "accountsproxymodel_p.h"

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
/// @todo port to new model code
#if 0
  class HierarchyFilterProxyModelPrivate : public AccountsProxyModelPrivate
  {
    Q_DISABLE_COPY(HierarchyFilterProxyModelPrivate)

  public:
    HierarchyFilterProxyModelPrivate() :
      AccountsProxyModelPrivate()
    {
    }

    ~HierarchyFilterProxyModelPrivate() override
    {
    }
  };
#endif

  HierarchyFilterProxyModel::HierarchyFilterProxyModel(QObject *parent)
  //   : AccountsProxyModel(*new HierarchyFilterProxyModelPrivate, parent)
  {
  }

  HierarchyFilterProxyModel::~HierarchyFilterProxyModel()
  {
  }

/// @todo port to new model code
// This should be handled by a flag in the base class
#if 0
  /**
  * Filter the favorites accounts group.
  */
  bool HierarchyFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
  {
    Q_D(const HierarchyFilterProxyModel);
    if (!source_parent.isValid()) {
        auto accCol = d->m_mdlColumns->indexOf(eAccountsModel::Column::Account);
        QVariant data = sourceModel()->index(source_row, accCol, source_parent).data((int)eAccountsModel::Role::ID);
        if (data.isValid() && data.toString() == AccountsModel::favoritesAccountId)
          return false;
      }
    return AccountsProxyModel::filterAcceptsRow(source_row, source_parent);
  }
#endif

/// @todo port to new model code
/// check if we can't do this via a views setColumnHidden() method which would completely eliminate this class
#if 0
#endif
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
