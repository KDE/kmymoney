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


#include "accountdelegate.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPainter>
#include <QTreeView>
#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "accountsmodel.h"
#include "securitiesmodel.h"
#include "mymoneymoney.h"
#include "mymoneyenums.h"



class AccountDelegate::Private
{
public:
  Private()
  {}

  ~Private()
  {
  }
};


AccountDelegate::AccountDelegate(QObject* parent)
  : QStyledItemDelegate(parent)
  , d(new Private)
{
}

AccountDelegate::~AccountDelegate()
{
  delete d;
}

void AccountDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QStyleOptionViewItem opt = option;
  QModelIndex baseIdx;
  initStyleOption(&opt, index);

  QTreeView* view = qobject_cast< QTreeView*>(parent());

  if (view) {
    switch(index.column()) {
      case AccountsModel::Column::TotalBalance:
        opt.text.clear();
        baseIdx = index.model()->index(index.row(), 0, index.parent());
        if (index.parent().isValid() && (view->isExpanded(baseIdx) || index.model()->rowCount(index) == 0)) {
          if (baseIdx.data(eMyMoney::Model::AccountCurrencyIdRole).toString() != MyMoneyFile::instance()->baseCurrency().id()) {
            paint(painter, option, index.model()->index(index.row(), AccountsModel::Column::Balance, index.parent()));
            return;
          }
        }
        break;

      case AccountsModel::Column::TotalPostedValue:
        baseIdx = index.model()->index(index.row(), 0, index.parent());
        if (index.parent().isValid() && (view->isExpanded(baseIdx) || index.model()->rowCount(index) == 0)) {
          paint(painter, option, index.model()->index(index.row(), AccountsModel::Column::PostedValue, index.parent()));
          return;
        }
        break;
    }
  }

  painter->save();

  QStyledItemDelegate::paint(painter, opt, index);

  painter->restore();
}
