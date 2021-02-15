/*
    SPDX-FileCopyrightText: 2006-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "registeritemdelegate.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "register.h"
#include "registeritem.h"

/// @todo remove this class with old register code
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
