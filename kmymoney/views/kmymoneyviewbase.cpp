/***************************************************************************
                          kmymoneyviewbase.cpp
                             -------------------
    copyright            : (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
