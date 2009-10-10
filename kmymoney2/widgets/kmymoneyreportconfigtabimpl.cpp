/*  This file is part of the KDE project
    Copyright (C) 2009 Laurent Montel <montel@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kmymoneyreportconfigtabimpl.h"

kMyMoneyReportConfigTab1Decl::kMyMoneyReportConfigTab1Decl( QWidget *parent )
    : QWidget( parent )
{
    setupUi( this );
}

kMyMoneyReportConfigTab2Decl::kMyMoneyReportConfigTab2Decl( QWidget *parent )
    : QWidget( parent )
{
    setupUi( this );
}

kMyMoneyReportConfigTab3Decl::kMyMoneyReportConfigTab3Decl( QWidget *parent )
    : QWidget( parent )
{
    setupUi( this );
    buttonGroup1->setExclusive(false);
    buttonGroup1->setId(m_checkMemo, 0);
    buttonGroup1->setId(m_checkShares, 1);
    buttonGroup1->setId(m_checkPrice, 2);
    buttonGroup1->setId(m_checkReconciled, 3);
    buttonGroup1->setId(m_checkAccount, 4);
    buttonGroup1->setId(m_checkNumber, 5);
    buttonGroup1->setId(m_checkPayee, 6);
    buttonGroup1->setId(m_checkCategory, 7);
    buttonGroup1->setId(m_checkAction, 8);
    buttonGroup1->setId(m_checkBalance, 9);
}

kMyMoneyReportConfigTabChartDecl::kMyMoneyReportConfigTabChartDecl( QWidget *parent )
    : QWidget( parent )
{
    setupUi( this );
}
