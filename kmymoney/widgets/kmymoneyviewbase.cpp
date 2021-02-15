/*
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kmymoneyviewbase.h"
#include "kmymoneyviewbase_p.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

KMyMoneyViewBase::KMyMoneyViewBase(QWidget* parent) :
    QWidget(parent), d_ptr(new KMyMoneyViewBasePrivate)
{
}

KMyMoneyViewBase::KMyMoneyViewBase(KMyMoneyViewBasePrivate &dd, QWidget *parent)
    : QWidget(parent), d_ptr(&dd)
{
}

KMyMoneyViewBase::~KMyMoneyViewBase()
{
}
