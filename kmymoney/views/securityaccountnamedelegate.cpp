/*
    SPDX-FileCopyrightText: 2023 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "securityaccountnamedelegate.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QApplication>
#include <QPainter>
#include <QScrollBar>
#include <QSortFilterProxyModel>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KColorScheme>

// ----------------------------------------------------------------------------
// Project Includes

#include "journalmodel.h"
#include "ledgerview.h"
#include "mymoneyfile.h"
#include "mymoneyutils.h"

class SecurityAccountNameDelegate::Private
{
public:
    Private()
    {
    }

    ~Private()
    {
    }
};

SecurityAccountNameDelegate::SecurityAccountNameDelegate(LedgerView* parent)
    : KMMStyledItemDelegate(parent)
    , d(nullptr)
{
}

SecurityAccountNameDelegate::~SecurityAccountNameDelegate()
{
    delete d;
}

void SecurityAccountNameDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
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

    KColorScheme::adjustBackground(opt.palette, KColorScheme::PositiveBackground, QPalette::Base, KColorScheme::View, KSharedConfigPtr());
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
        painter->drawText(opt.rect, Qt::AlignCenter, opt.text);
        break;
    }

    painter->restore();
}

QSize SecurityAccountNameDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    QStyle* style = opt.widget ? opt.widget->style() : QApplication::style();
    const auto margin = style->pixelMetric(QStyle::PM_FocusFrameHMargin);
    const auto lineHeight = opt.fontMetrics.lineSpacing();

    QSize size(10, lineHeight + 2 * margin);

    return size;
}

/**
 * This eventfilter seems to do nothing but it prevents that selecting a
 * different row with the mouse closes the editor
 */
bool SecurityAccountNameDelegate::eventFilter(QObject* o, QEvent* event)
{
    return QAbstractItemDelegate::eventFilter(o, event);
}
