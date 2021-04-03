/*
    SPDX-FileCopyrightText: 2015-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
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
