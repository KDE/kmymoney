/*
    SPDX-FileCopyrightText: 2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "reconciliationdelegate.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QAbstractItemView>
#include <QApplication>
#include <QDate>
#include <QPainter>
#include <QScrollBar>
#include <QSortFilterProxyModel>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KColorScheme>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "journalmodel.h"
#include "mymoneyfile.h"
#include "mymoneyutils.h"

class ReconciliationDelegate::Private
{
public:
    Private()
    {
    }

    ~Private()
    {
    }
};

ReconciliationDelegate::ReconciliationDelegate(QWidget* parent)
    : KMMStyledItemDelegate(parent)
    , d(new Private)
{
}

ReconciliationDelegate::~ReconciliationDelegate()
{
    delete d;
}

void ReconciliationDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    // never change the background of the cell the mouse is hovering over
    opt.state &= ~QStyle::State_MouseOver;

    // never show the focus
    opt.state &= ~QStyle::State_HasFocus;

    // if selected, always show as active, so that the
    // background does not change when the editor is shown
    if (opt.state & QStyle::State_Selected) {
        opt.state |= QStyle::State_Active;
    }
    // never draw it as selected but always enabled
    opt.state &= ~QStyle::State_Selected;
    opt.state |= QStyle::State_Enabled;

    painter->save();

    QAbstractItemView* view = qobject_cast<QAbstractItemView*>(parent());

    // Background
    QStyle* style = opt.widget ? opt.widget->style() : QApplication::style();
    const int margin = style->pixelMetric(QStyle::PM_FocusFrameHMargin);

    const QRect textArea = QRect(opt.rect.x() + margin, opt.rect.y() + margin, opt.rect.width() - 2 * margin, opt.rect.height() - 2 * margin);

    const auto reconciliationDate = index.data(eMyMoney::Model::TransactionPostDateRole).toDate();
    const auto reconciliationBalanceValue = index.data(eMyMoney::Model::ReconciliationAmountRole).value<MyMoneyMoney>();
    const auto accountBalance = MyMoneyFile::instance()->balance(index.data(eMyMoney::Model::SplitAccountIdRole).toString(), reconciliationDate);

    KColorScheme::BackgroundRole role = (accountBalance == reconciliationBalanceValue) ? KColorScheme::PositiveBackground : KColorScheme::NegativeBackground;

    KColorScheme::adjustBackground(opt.palette, role, QPalette::Base, KColorScheme::View, KSharedConfigPtr());
    // opt.rect.setHeight(lineHeight);
    opt.backgroundBrush = opt.palette.base();

    opt.rect.setX(opt.rect.x() - 2);
    opt.rect.setWidth(opt.rect.width() + 5);
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

    painter->setFont(opt.font);
    switch (index.column()) {
    case JournalModel::Column::Detail:
        // adjust the rect to cover all columns
        if (view && view->viewport()) {
            opt.rect.setX(0);
            opt.rect.setWidth(view->viewport()->width());
        }
        painter->setPen(opt.palette.color(QPalette::Normal, QPalette::Text));
        painter->drawText(opt.rect, Qt::AlignCenter, i18nc("Ledger marker showing a reconciliation entry", "Reconciliation"));
        break;
    case JournalModel::Column::Date:
        painter->setPen(opt.palette.color(QPalette::Normal, QPalette::Text));
        painter->drawText(textArea, opt.displayAlignment, MyMoneyUtils::formatDate(reconciliationDate));
        break;
    case JournalModel::Column::Balance:
        painter->setPen(opt.palette.color(QPalette::Normal, QPalette::Text));
        painter->drawText(textArea, opt.displayAlignment, opt.text);
        break;
    }

    painter->restore();
}

QSize ReconciliationDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    QStyle* style = opt.widget ? opt.widget->style() : QApplication::style();
    const auto margin = style->pixelMetric(QStyle::PM_FocusFrameHMargin);
    const auto lineHeight = opt.fontMetrics.lineSpacing();

    return QSize(10, lineHeight + 2 * margin);
}

/**
 * This eventfilter seems to do nothing but it prevents that selecting a
 * different row with the mouse closes the editor
 */
bool ReconciliationDelegate::eventFilter(QObject* o, QEvent* event)
{
    return QAbstractItemDelegate::eventFilter(o, event);
}
