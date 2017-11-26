/***************************************************************************
                             hierarchyfilterproxymodel.h
                             -------------------
    begin                : Tue Sep 25 2007
    copyright            : (C) 2007 Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef HIERARCHRYFILTERPROXYMODEL_H
#define HIERARCHRYFILTERPROXYMODEL_H

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
  class HierarchyFilterProxyModel : public AccountsProxyModel
  {
    Q_OBJECT
    Q_DISABLE_COPY(HierarchyFilterProxyModel)

  public:
    explicit HierarchyFilterProxyModel(QObject *parent = 0);

  protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
    bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const;
  };
} // namespace

#endif
