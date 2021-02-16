/*
    SPDX-FileCopyrightText: 2007 Thomas Baumgart <Thomas Baumgart <ipwizard@users.sourceforge.net>>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HIERARCHYFILTERPROXYMODEL_H
#define HIERARCHYFILTERPROXYMODEL_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "accountsproxymodel.h"

class Wizard;
class MyMoneyInstitution;

namespace NewAccountWizard
{
  class HierarchyFilterProxyModelPrivate;
  class HierarchyFilterProxyModel : public AccountsProxyModel
  {
    Q_OBJECT
    Q_DISABLE_COPY(HierarchyFilterProxyModel)

  public:
    explicit HierarchyFilterProxyModel(QObject *parent = nullptr);
    ~HierarchyFilterProxyModel() override;

  protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const override;

  private:
    Q_DECLARE_PRIVATE(HierarchyFilterProxyModel)
  };
} // namespace

#endif
