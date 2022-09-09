/*
    SPDX-FileCopyrightText: 2009-2016 Cristian Oneț <onet.cristian@gmail.com>
    SPDX-FileCopyrightText: 2009-2010 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-FileCopyrightText: 2010-2017 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
    Q_EMIT itemSelected(itemData(idx).toInt());
}
