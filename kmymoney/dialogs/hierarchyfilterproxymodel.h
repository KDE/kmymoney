/***************************************************************************
                          hierarchyfilterproxymodel.h
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes <mte@users.sourceforge.net>
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

#ifndef HIERARCHYFILTERPROXYMODEL_H
#define HIERARCHYFILTERPROXYMODEL_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>

// ----------------------------------------------------------------------------
// KDE Headers

// ----------------------------------------------------------------------------
// Project Includes

#include "accountsproxymodel.h"

class HierarchyFilterProxyModel : public AccountsProxyModel
{
  Q_OBJECT

public:
  explicit HierarchyFilterProxyModel(QObject *parent = nullptr);

  Qt::ItemFlags flags(const QModelIndex &index) const override;

  void setCurrentAccountId(const QString &selectedAccountId);
  QModelIndex getSelectedParentAccountIndex() const;

protected:
  bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
  bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const override;

private:
  QString m_currentAccountId;
};

#endif

