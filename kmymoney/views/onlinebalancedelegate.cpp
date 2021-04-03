/*
    SPDX-FileCopyrightText: 2015-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "onlinebalancedelegate.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QApplication>
#include <QScrollBar>
#include <QPainter>
#include <QDebug>
#include <QDate>
#include <QSortFilterProxyModel>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KColorScheme>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "mymoneyutils.h"
#include "ledgerview.h"
#include "journalmodel.h"
#include "payeesmodel.h"
#include "newtransactioneditor.h"

QColor OnlineBalanceDelegate::m_erroneousColor = QColor(Qt::red);
QColor OnlineBalanceDelegate::m_importedColor = QColor(Qt::yellow);
QColor OnlineBalanceDelegate::m_separatorColor = QColor(0xff, 0xf2, 0x9b);




class OnlineBalanceDelegate::Private
{
public:
    Private()
        : m_editor(nullptr)
        , m_view(nullptr)
        , m_editorRow(-1)
        , m_singleLineRole(eMyMoney::Model::SplitPayeeRole)
        , m_lineHeight(12)
        , m_margin(2)

    {}

    ~Private()
    {
    }

    QStringList displayString(const QModelIndex& index, const QStyleOptionViewItem& opt)
    {
        QStringList lines;
        if(index.column() == JournalModel::Column::Detail) {
            lines << index.data(m_singleLineRole).toString();
            if(opt.state & QStyle::State_Selected) {
                lines.clear();
                lines << index.data(eMyMoney::Model::Roles::SplitPayeeRole).toString();
                lines << index.data(eMyMoney::Model::Roles::TransactionCounterAccountRole).toString();
                lines << index.data(eMyMoney::Model::Roles::SplitSingleLineMemoRole).toString();

            } else {
                if(lines.at(0).isEmpty()) {
                    lines.clear();
                    lines << index.data(eMyMoney::Model::Roles::SplitSingleLineMemoRole).toString();
                }
                if(lines.at(0).isEmpty()) {
                    lines << index.data(eMyMoney::Model::Roles::TransactionCounterAccountRole).toString();
                }
            }
            lines.removeAll(QString());

        } else {
            lines << opt.text;
        }
        return lines;
    }

    NewTransactionEditor*         m_editor;
    LedgerView*                   m_view;
    int                           m_editorRow;
    eMyMoney::Model::Roles        m_singleLineRole;
    int                           m_lineHeight;
    int                           m_margin;
    QBrush                        m_backGround;
};


OnlineBalanceDelegate::OnlineBalanceDelegate(LedgerView* parent)
    : KMMStyledItemDelegate(parent)
    , d(new Private)
{
    d->m_view = parent;
}

OnlineBalanceDelegate::~OnlineBalanceDelegate()
{
    delete d;
}

void OnlineBalanceDelegate::setErroneousColor(const QColor& color)
{
    m_erroneousColor = color;
}

void OnlineBalanceDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
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

    QAbstractItemView* view = qobject_cast< QAbstractItemView* >(parent());

    // Background
    QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
    const int margin = style->pixelMetric(QStyle::PM_FocusFrameHMargin);
    const int lineHeight = opt.fontMetrics.lineSpacing() + 2;

    const QRect textArea = QRect(opt.rect.x() + margin, opt.rect.y() + margin, opt.rect.width() - 2 * margin, opt.rect.height() - 2 * margin);

    QPalette::ColorGroup cg;

    const auto onlineBalanceDate = index.data(eMyMoney::Model::AccountOnlineBalanceDateRole).toDate();
    const auto onlineBalanceValue = index.data(eMyMoney::Model::AccountOnlineBalanceValueRole).value<MyMoneyMoney>();
    const auto accountBalance = MyMoneyFile::instance()->balance(index.data(eMyMoney::Model::IdRole).toString(), onlineBalanceDate);

    KColorScheme::BackgroundRole role = (accountBalance == onlineBalanceValue) ? KColorScheme::PositiveBackground : KColorScheme::NegativeBackground;

    KColorScheme::adjustBackground(opt.palette, role, QPalette::Base, KColorScheme::View, KSharedConfigPtr());
    // opt.rect.setHeight(lineHeight);
    opt.backgroundBrush = opt.palette.base();
    d->m_backGround = opt.backgroundBrush;

    opt.rect.setX(opt.rect.x()-2);
    opt.rect.setWidth(opt.rect.width()+5);
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

    switch (index.column()) {
    case JournalModel::Column::Detail:
        // adjust the rect to cover all columns
        if(view && view->viewport()) {
            opt.rect.setX(0);
            opt.rect.setWidth(view->viewport()->width());
        }
        painter->setPen(opt.palette.color(QPalette::Normal, QPalette::Text));
        painter->drawText(opt.rect, Qt::AlignCenter, "Online Balance");
        break;
    case JournalModel::Column::Date:
        painter->setPen(opt.palette.color(QPalette::Normal, QPalette::Text));
        painter->drawText(textArea, opt.displayAlignment, MyMoneyUtils::formatDate(onlineBalanceDate));
        break;
    case JournalModel::Column::Balance:
        painter->setPen(opt.palette.color(QPalette::Normal, QPalette::Text));
        painter->drawText(textArea, opt.displayAlignment, onlineBalanceValue.formatMoney(index.data(eMyMoney::Model::AccountFractionRole).toInt()));
        break;
    }

    painter->restore();
}

QSize OnlineBalanceDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
    d->m_margin = style->pixelMetric(QStyle::PM_FocusFrameHMargin);
    d->m_lineHeight = opt.fontMetrics.lineSpacing();

    QSize size(10, d->m_lineHeight + 2 * d->m_margin);

    return size;
}

/**
 * This eventfilter seems to do nothing but it prevents that selecting a
 * different row with the mouse closes the editor
 */
bool OnlineBalanceDelegate::eventFilter(QObject* o, QEvent* event)
{
    return QAbstractItemDelegate::eventFilter(o, event);
}
