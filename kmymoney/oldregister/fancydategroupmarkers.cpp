/*
 * Copyright 2006-2018  Thomas Baumgart <tbaumgart@kde.org>
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
