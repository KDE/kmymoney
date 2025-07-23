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
    QModelIndex columnZeroIdx;
    initStyleOption(&opt, index);

    QTreeView* view = qobject_cast< QTreeView*>(parent());

    if (view) {
        switch(index.column()) {
        case AccountsModel::Column::Balance:
            columnZeroIdx = index.model()->index(index.row(), 0, index.parent());
            if (!view->isExpanded(columnZeroIdx) && index.model()->rowCount(index) != 0
                && columnZeroIdx.data(eMyMoney::Model::AccountCurrencyIdRole).toString() != MyMoneyFile::instance()->baseCurrency().id()) {
                opt.text.clear();
            }
            break;

        case AccountsModel::Column::TotalPostedValue:
            columnZeroIdx = index.model()->index(index.row(), 0, index.parent());
            if (index.parent().isValid() && (view->isExpanded(columnZeroIdx) || index.model()->rowCount(index) == 0)) {
                // in case the index does not have children and the account type is
                // not investment then we display the PostedValue instead of the
                // TotalPostedValue. Investments are different as the
                // subaccounts can be filtered out globally with the hide equity account
                // filter. In this case we switch to the PostedValue only if the
                // item is expanded in the view.
                if (index.data(eMyMoney::Model::AccountTypeRole).value<eMyMoney::Account::Type>() == eMyMoney::Account::Type::Investment) {
                    if (view->isExpanded(columnZeroIdx) && (columnZeroIdx.model()->rowCount(columnZeroIdx) != 0)) {
                        paint(painter, option, index.model()->index(index.row(), AccountsModel::Column::PostedValue, index.parent()));
                        return;
                    }
                } else {
                    paint(painter, option, index.model()->index(index.row(), AccountsModel::Column::PostedValue, index.parent()));
                    return;
                }
            }
            break;
        }
    }

    QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter);
}
