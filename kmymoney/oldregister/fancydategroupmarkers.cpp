/*
    SPDX-FileCopyrightText: 2006-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fancydategroupmarkers.h"
#include "fancydategroupmarker_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QStyleOptionViewItem>
#include <QPainter>
#include <QPalette>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneysettings.h"
#include "widgetenums.h"

using namespace KMyMoneyRegister;
using namespace eMyMoney;

namespace KMyMoneyRegister {
class StatementGroupMarkerPrivate : public FancyDateGroupMarkerPrivate
{
public:
    eWidgets::eRegister::CashFlowDirection m_dir;
};
}

StatementGroupMarker::StatementGroupMarker(Register* parent, eWidgets::eRegister::CashFlowDirection dir, const QDate& date, const QString& txt) :
    FancyDateGroupMarker(*new StatementGroupMarkerPrivate, parent, date, txt)
{
    Q_D(StatementGroupMarker);
    d->m_dir = dir;
    d->m_showDate = true;
}

StatementGroupMarker::~StatementGroupMarker()
{
}

eWidgets::eRegister::CashFlowDirection StatementGroupMarker::sortType() const
{
    Q_D(const StatementGroupMarker);
    return d->m_dir;
}

int StatementGroupMarker::sortSamePostDate() const
{
    return 3;
}

FiscalYearGroupMarker::FiscalYearGroupMarker(Register* parent, const QDate& date, const QString& txt) :
    FancyDateGroupMarker(parent, date, txt)
{
}

FiscalYearGroupMarker::~FiscalYearGroupMarker()
{
}

const char* FiscalYearGroupMarker::className()
{
    return "FiscalYearGroupMarker";
}

int FiscalYearGroupMarker::sortSamePostDate() const
{
    return 1;
}

SimpleDateGroupMarker::SimpleDateGroupMarker(Register* parent, const QDate& date, const QString& txt) :
    FancyDateGroupMarker(parent, date, txt)
{
}

SimpleDateGroupMarker::~SimpleDateGroupMarker()
{
}

int SimpleDateGroupMarker::rowHeightHint() const
{
    Q_D(const FancyDateGroupMarker);
    if (!d->m_visible)
        return 0;

    return RegisterItem::rowHeightHint() / 2;
}

const char* SimpleDateGroupMarker::className()
{
    return "SimpleDateGroupMarker";
}

void SimpleDateGroupMarker::paintRegisterCell(QPainter *painter, QStyleOptionViewItem &option, const QModelIndex &index)
{
    Q_D(FancyDateGroupMarker);
    QRect cellRect = option.rect;
    painter->save();
    cellRect.setWidth(d->m_parent->viewport()->width());
    cellRect.setHeight(d->m_parent->rowHeight(index.row() + d->m_startRow));

    if (d->m_alternate)
        option.palette.setColor(QPalette::Base, KMyMoneySettings::schemeColor(SchemeColor::ListBackground2));
    else
        option.palette.setColor(QPalette::Base, KMyMoneySettings::schemeColor(SchemeColor::ListBackground1));

    QBrush backgroundBrush(option.palette.color(QPalette::Base));
    backgroundBrush.setStyle(Qt::Dense5Pattern);
    backgroundBrush.setColor(KMyMoneySettings::schemeColor(SchemeColor::ListGrid));
    painter->eraseRect(cellRect);
    painter->fillRect(cellRect, backgroundBrush);
    painter->setPen(KMyMoneySettings::schemeColor(SchemeColor::ListGrid));
    painter->restore();
}
