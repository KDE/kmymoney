/*
    SPDX-FileCopyrightText: 2007 Thomas Baumgart <Thomas Baumgart <ipwizard@users.sourceforge.net>>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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

  HierarchyFilterProxyModel::HierarchyFilterProxyModel(QObject *parent) :
    AccountsProxyModel(*new HierarchyFilterProxyModelPrivate, parent)
  {
  }

  HierarchyFilterProxyModel::~HierarchyFilterProxyModel()
  {
  }

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
