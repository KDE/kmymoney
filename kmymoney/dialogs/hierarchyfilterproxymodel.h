/*
 * SPDX-FileCopyrightText: 2002-2003 Michael Edwardes <mte@users.sourceforge.net>
 * SPDX-FileCopyrightText: 2005-2018 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

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

