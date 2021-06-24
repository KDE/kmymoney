/*
    SPDX-FileCopyrightText: 2015-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "accountdelegate.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QTreeView>

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
        case AccountsModel::Column::Balance:
            baseIdx = index.model()->index(index.row(), 0, index.parent());
            if (!view->isExpanded(baseIdx) && index.model()->rowCount(index) != 0
                && baseIdx.data(eMyMoney::Model::AccountCurrencyIdRole).toString() != MyMoneyFile::instance()->baseCurrency().id()) {
                opt.text.clear();
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

    QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter);
}
