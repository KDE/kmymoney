/*
 * Copyright 2002-2003  Michael Edwardes <mte@users.sourceforge.net>
 * Copyright 2005-2018  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

/// @todo cleanup - remove file

// #ifndef HIERARCHYFILTERPROXYMODEL_H
// #define HIERARCHYFILTERPROXYMODEL_H
//
// // ----------------------------------------------------------------------------
// // QT Includes
//
// #include <QString>
//
// // ----------------------------------------------------------------------------
// // KDE Headers
//
// // ----------------------------------------------------------------------------
// // Project Includes
//
// #include "accountsproxymodel.h"
//
// class HierarchyFilterProxyModel : public AccountsProxyModel
// {
//   Q_OBJECT
//
// public:
//   explicit HierarchyFilterProxyModel(QObject *parent = nullptr);
//
//   Qt::ItemFlags flags(const QModelIndex &index) const override;
//
//   void setCurrentAccountId(const QString &selectedAccountId);
//   QModelIndex getSelectedParentAccountIndex() const;
//
// protected:
//   // bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
//   bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const override;
//
// private:
//   QString m_currentAccountId;
// };
//
// #endif
//
