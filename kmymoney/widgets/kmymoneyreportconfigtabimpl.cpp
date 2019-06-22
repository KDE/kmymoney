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

#include "ui_kmymoneyreportconfigtab1decl.h"
#include "ui_kmymoneyreportconfigtab2decl.h"
#include "ui_kmymoneyreportconfigtab3decl.h"
#include "ui_kmymoneyreportconfigtabchartdecl.h"

#include "mymoney/mymoneyreport.h"

kMyMoneyReportConfigTab1Decl::kMyMoneyReportConfigTab1Decl(QWidget *parent)
    : QWidget(parent)
{
  ui = new Ui::kMyMoneyReportConfigTab1Decl;
  ui->setupUi(this);
}

kMyMoneyReportConfigTab1Decl::~kMyMoneyReportConfigTab1Decl()
{
  delete ui;
}

kMyMoneyReportConfigTab2Decl::kMyMoneyReportConfigTab2Decl(QWidget *parent)
    : QWidget(parent)
{
  ui = new Ui::kMyMoneyReportConfigTab2Decl;
  ui->setupUi(this);
}

kMyMoneyReportConfigTab2Decl::~kMyMoneyReportConfigTab2Decl()
{
  delete ui;
}

kMyMoneyReportConfigTab3Decl::kMyMoneyReportConfigTab3Decl(QWidget *parent)
    : QWidget(parent)
{
  ui = new Ui::kMyMoneyReportConfigTab3Decl;
  ui->setupUi(this);
  ui->buttonGroup1->setExclusive(false);
  ui->buttonGroup1->setId(ui->m_checkMemo, 0);
  ui->buttonGroup1->setId(ui->m_checkShares, 1);
  ui->buttonGroup1->setId(ui->m_checkPrice, 2);
  ui->buttonGroup1->setId(ui->m_checkReconciled, 3);
  ui->buttonGroup1->setId(ui->m_checkAccount, 4);
  ui->buttonGroup1->setId(ui->m_checkNumber, 5);
  ui->buttonGroup1->setId(ui->m_checkPayee, 6);
  ui->buttonGroup1->setId(ui->m_checkCategory, 7);
  ui->buttonGroup1->setId(ui->m_checkAction, 8);
  ui->buttonGroup1->setId(ui->m_checkBalance, 9);
}

kMyMoneyReportConfigTab3Decl::~kMyMoneyReportConfigTab3Decl()
{
  delete ui;
}

kMyMoneyReportConfigTabChartDecl::kMyMoneyReportConfigTabChartDecl(QWidget *parent)
    : QWidget(parent)
{
  ui = new Ui::kMyMoneyReportConfigTabChartDecl;
  ui->setupUi(this);

  ui->m_comboType->addItem(i18nc("type of graphic chart", "Line"), MyMoneyReport::Chart::Line);
  ui->m_comboType->addItem(i18nc("type of graphic chart", "Bar"), MyMoneyReport::Chart::Bar);
  ui->m_comboType->addItem(i18nc("type of graphic chart", "Stacked Bar"), MyMoneyReport::Chart::StackedBar);
  ui->m_comboType->addItem(i18nc("type of graphic chart", "Pie"), MyMoneyReport::Chart::Pie);
  ui->m_comboType->addItem(i18nc("type of graphic chart", "Ring"), MyMoneyReport::Chart::Ring);

  ui->m_chartPalette->addItem(i18nc("chart palette", "Use application setting"), MyMoneyReport::ChartPalette::Application);
  ui->m_chartPalette->addItem(i18nc("chart palette", "Default"), MyMoneyReport::ChartPalette::Default);
  ui->m_chartPalette->addItem(i18nc("chart palette", "Rainbowed"), MyMoneyReport::ChartPalette::Rainbow);
  ui->m_chartPalette->addItem(i18nc("chart palette", "Subdued"), MyMoneyReport::ChartPalette::Subdued);
}

kMyMoneyReportConfigTabChartDecl::~kMyMoneyReportConfigTabChartDecl()
{
  delete ui;
}

