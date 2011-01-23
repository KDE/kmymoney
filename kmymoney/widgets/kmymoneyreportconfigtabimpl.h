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

#include <QWidget>

namespace Ui
{
class kMyMoneyReportConfigTab1Decl;
class kMyMoneyReportConfigTab2Decl;
class kMyMoneyReportConfigTab3Decl;
class kMyMoneyReportConfigTabChartDecl;
};

class kMyMoneyReportConfigTab1Decl : public QWidget
{
public:
  kMyMoneyReportConfigTab1Decl(QWidget *parent);
  virtual ~kMyMoneyReportConfigTab1Decl();
private:
  Ui::kMyMoneyReportConfigTab1Decl* ui;
};

class kMyMoneyReportConfigTab2Decl : public QWidget
{
public:
  kMyMoneyReportConfigTab2Decl(QWidget *parent);
  virtual ~kMyMoneyReportConfigTab2Decl();
private:
  Ui::kMyMoneyReportConfigTab2Decl* ui;
};

class kMyMoneyReportConfigTab3Decl : public QWidget
{
public:
  kMyMoneyReportConfigTab3Decl(QWidget *parent);
  virtual ~kMyMoneyReportConfigTab3Decl();
private:
  Ui::kMyMoneyReportConfigTab3Decl* ui;
};

class kMyMoneyReportConfigTabChartDecl : public QWidget
{
public:
  kMyMoneyReportConfigTabChartDecl(QWidget *parent);
  virtual ~kMyMoneyReportConfigTabChartDecl();
private:
  Ui::kMyMoneyReportConfigTabChartDecl* ui;
};

#endif /* KMYMONEYREPORTCONFIGTABIMPL_H */

