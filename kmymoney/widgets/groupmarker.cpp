/***************************************************************************
                             groupmarker.cpp  -  description
                             -------------------
    begin                : Fri Mar 10 2006
    copyright            : (C) 2006 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "groupmarker.h"
#include "groupmarker_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QPixmap>
#include <QHeaderView>
#include <QStyleOptionViewItem>
#include <QPainter>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "widgetenums.h"

using namespace KMyMoneyRegister;

QPixmap* GroupMarkerPrivate::GroupMarkerPrivate::m_bg = nullptr;
int GroupMarkerPrivate::GroupMarkerPrivate::m_bgRefCnt = 0;

GroupMarker::GroupMarker(Register *parent, const QString& txt) :
    RegisterItem(*new GroupMarkerPrivate, parent)
{
  Q_D(GroupMarker);
  d->m_txt = txt;
  d->init();
}

GroupMarker::GroupMarker(GroupMarkerPrivate &dd, Register *parent, const QString& txt) :
  RegisterItem(dd, parent)
{
  Q_D(GroupMarker);
  d->m_txt = txt;
  d->init();
}

GroupMarker::~GroupMarker()
{
  Q_D(GroupMarker);
  --d->GroupMarkerPrivate::m_bgRefCnt;
  if (!d->GroupMarkerPrivate::m_bgRefCnt) {
    delete d->GroupMarkerPrivate::m_bg;
    d->GroupMarkerPrivate::m_bg = 0;
  }
}

void GroupMarker::setText(const QString& txt)
{
  Q_D(GroupMarker);
  d->m_txt = txt;
}

QString GroupMarker::text() const
{
  Q_D(const GroupMarker);
  return d->m_txt;
}

bool GroupMarker::isSelectable() const
{
  return false;
}

bool GroupMarker::canHaveFocus() const
{
  return false;
}

int GroupMarker::numRows() const
{
  return 1;
}

const char* GroupMarker::className()
{
  return "GroupMarker";
}

bool GroupMarker::isErroneous() const
{
  Q_D(const GroupMarker);
  return d->m_erroneous;
}

void GroupMarker::paintRegisterCell(QPainter *painter, QStyleOptionViewItem &option, const QModelIndex &index)
{
  Q_D(GroupMarker);
  QRect r(option.rect);
  painter->save();

  // the group marker always uses all cols
  r.setX(d->m_parent->horizontalHeader()->sectionPosition(0));
  r.setWidth(d->m_parent->viewport()->width());
  painter->translate(r.x(), r.y());

  QRect cellRect;
  cellRect.setX(0);
  cellRect.setY(0);
  cellRect.setWidth(d->m_parent->viewport()->width());
  cellRect.setHeight(d->m_parent->rowHeight(index.row()));

  option.palette.setColor(QPalette::Base, isErroneous() ? KMyMoneySettings::schemeColor(SchemeColor::TransactionErroneous) : KMyMoneySettings::schemeColor(SchemeColor::GroupMarker));

  QBrush backgroundBrush(option.palette.color(QPalette::Base));
  painter->fillRect(cellRect, backgroundBrush);
  painter->setPen(KMyMoneySettings::schemeColor(SchemeColor::ListGrid));
  painter->drawLine(cellRect.x(), cellRect.height() - 1, cellRect.width(), cellRect.height() - 1);

  // now write the text
  painter->setPen(option.palette.color(isErroneous() ? QPalette::HighlightedText : QPalette::Text));
  QFont font = painter->font();
  font.setBold(true);
  painter->setFont(font);

  painter->drawText(cellRect, Qt::AlignVCenter | Qt::AlignCenter, d->m_txt);

  cellRect.setHeight(d->GroupMarkerPrivate::m_bg->height());

  // now it's time to draw the background
  painter->drawPixmap(cellRect, *KMyMoneyRegister::GroupMarkerPrivate::m_bg);

  // in case we need to show the date, we just paint it in col 1
  if (d->m_showDate) {
    font.setBold(false);
    cellRect.setX(d->m_parent->horizontalHeader()->sectionPosition((int)eWidgets::eTransaction::Column::Date));
    cellRect.setWidth(d->m_parent->horizontalHeader()->sectionSize((int)eWidgets::eTransaction::Column::Date));
    painter->setFont(font);
    painter->drawText(cellRect, Qt::AlignVCenter | Qt::AlignCenter, QLocale().toString(sortPostDate(), QLocale::ShortFormat));
  }

  painter->restore();
}

void GroupMarker::paintFormCell(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index)
{
  Q_UNUSED(painter);
  Q_UNUSED(option);
  Q_UNUSED(index);
}

int GroupMarker::rowHeightHint() const
{
  Q_D(const GroupMarker);
  if (!d->m_visible)
    return 0;

  return d->GroupMarkerPrivate::m_bg->height();
}

bool GroupMarker::matches(const RegisterFilter&) const
{
  return true;
}

int GroupMarker::sortSamePostDate() const
{
  return 0;
}

void GroupMarker::setErroneous(bool condition)
{
  Q_D(GroupMarker);
  d->m_erroneous = condition;
}
