/*
 * Copyright 2009-2016  Cristian Oneț <onet.cristian@gmail.com>
 * Copyright 2009-2010  Alvaro Soliverez <asoliverez@gmail.com>
 * Copyright 2010-2017  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#include "kmymoneygeneralcombo.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

KMyMoneyGeneralCombo::KMyMoneyGeneralCombo(QWidget* w) :
    KComboBox(w)
{
  connect(this, static_cast<void (QComboBox::*)(int)>(&QComboBox::highlighted), this, &KMyMoneyGeneralCombo::slotChangeItem);
}

KMyMoneyGeneralCombo::~KMyMoneyGeneralCombo()
{
}

void KMyMoneyGeneralCombo::setCurrentItem(int id)
{
  setCurrentIndex(findData(QVariant(id), Qt::UserRole, Qt::MatchExactly));
}

int KMyMoneyGeneralCombo::currentItem() const
{
  return itemData(currentIndex()).toInt();
}

void KMyMoneyGeneralCombo::clear()
{
  KComboBox::clear();
}

void KMyMoneyGeneralCombo::insertItem(const QString& txt, int id, int idx)
{
  KComboBox::insertItem(idx, txt, QVariant(id));
}

void KMyMoneyGeneralCombo::removeItem(int id)
{
  KComboBox::removeItem(findData(QVariant(id), Qt::UserRole, Qt::MatchExactly));
}

void KMyMoneyGeneralCombo::slotChangeItem(int idx)
{
  emit itemSelected(itemData(idx).toInt());
}
