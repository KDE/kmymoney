/***************************************************************************
                             hierarchyfilterproxymodel.h
                             -------------------
    begin                : Tue Sep 25 2007
    copyright            : (C) 2007 Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
                           (C) 2017-2018 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

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
