/***************************************************************************
                             registeritemdelegate.cpp  -  description
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

#include "registeritemdelegate.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "register.h"
#include "registeritem.h"

using namespace KMyMoneyRegister;

RegisterItemDelegate::RegisterItemDelegate(Register *parent) :
  QStyledItemDelegate(parent),
  m_register(parent)
{
}

RegisterItemDelegate::~RegisterItemDelegate()
{
}

void RegisterItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  RegisterItem* const item = m_register->itemAtRow(index.row());
  if (item && m_register->updatesEnabled()) {
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    item->paintRegisterCell(painter, opt, index);
  }
}
