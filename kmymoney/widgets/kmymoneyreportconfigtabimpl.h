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

#ifndef KMYMONEYREPORTCONFIGTABIMPL_H
#define KMYMONEYREPORTCONFIGTABIMPL_H

#include "ui_kmymoneyreportconfigtab1decl.h"
#include "ui_kmymoneyreportconfigtab2decl.h"
#include "ui_kmymoneyreportconfigtab3decl.h"
#include "ui_kmymoneyreportconfigtabchartdecl.h"



class kMyMoneyReportConfigTab1Decl : public QWidget, public Ui::kMyMoneyReportConfigTab1Decl
{
public:
  kMyMoneyReportConfigTab1Decl(QWidget *parent);
};

class kMyMoneyReportConfigTab2Decl : public QWidget, public Ui::kMyMoneyReportConfigTab2Decl
{
public:
  kMyMoneyReportConfigTab2Decl(QWidget *parent);
};

class kMyMoneyReportConfigTab3Decl : public QWidget, public Ui::kMyMoneyReportConfigTab3Decl
{
public:
  kMyMoneyReportConfigTab3Decl(QWidget *parent);
};

class kMyMoneyReportConfigTabChartDecl : public QWidget, public Ui::kMyMoneyReportConfigTabChartDecl
{
public:
  kMyMoneyReportConfigTabChartDecl(QWidget *parent);
};
#endif /* KMYMONEYREPORTCONFIGTABIMPL_H */

