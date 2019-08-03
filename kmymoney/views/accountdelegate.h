/*
 * Copyright 2015-2019  Thomas Baumgart <tbaumgart@kde.org>
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


#ifndef ACCOUNTDELEGATE_H
#define ACCOUNTDELEGATE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QStyledItemDelegate>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "accountsmodel.h"
#include "mymoneyenums.h"

class MyMoneyMoney;

class AccountDelegate : public QStyledItemDelegate
{
  Q_OBJECT
public:
  explicit AccountDelegate(QObject* parent = 0);
  virtual ~AccountDelegate();

  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const final override;

private:
  class Private;
  Private * const d;
};

#endif // ACCOUNTDELEGATE_H

