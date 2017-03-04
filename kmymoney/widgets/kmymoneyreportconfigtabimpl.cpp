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
#include "kmymoneyglobalsettings.h"

#include <klocalizedstring.h>
#include <daterangedlg.h>

#include "ui_kmymoneyreportconfigtab1decl.h"
#include "ui_kmymoneyreportconfigtab2decl.h"
#include "ui_kmymoneyreportconfigtab3decl.h"
#include "ui_kmymoneyreportconfigtabchartdecl.h"
#include "ui_kmymoneyreportconfigtabrangedecl.h"
#include "dialogs/ui_daterangedlgdecl.h"

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

  ui->m_comboType->addItem(i18nc("type of graphic chart", "Line"), MyMoneyReport::eChartLine);
  ui->m_comboType->addItem(i18nc("type of graphic chart", "Bar"), MyMoneyReport::eChartBar);
  ui->m_comboType->addItem(i18nc("type of graphic chart", "Stacked Bar"), MyMoneyReport::eChartStackedBar);
  ui->m_comboType->addItem(i18nc("type of graphic chart", "Pie"), MyMoneyReport::eChartPie);
  ui->m_comboType->addItem(i18nc("type of graphic chart", "Ring"), MyMoneyReport::eChartRing);
  connect(ui->m_comboType, SIGNAL(currentIndexChanged(int)), this, SLOT(slotChartTypeChanged(int)));
  emit ui->m_comboType->currentIndexChanged(ui->m_comboType->currentIndex());
}

kMyMoneyReportConfigTabChartDecl::~kMyMoneyReportConfigTabChartDecl()
{
  delete ui;
}

void kMyMoneyReportConfigTabChartDecl::slotChartTypeChanged(int index)
{
  if (index == MyMoneyReport::EChartType::eChartPie ||
      index == MyMoneyReport::EChartType::eChartRing) {
    ui->m_checkCHGridLines->setText(i18n("Show circular grid lines"));
    ui->m_checkSVGridLines->setText(i18n("Show sagittal grid lines"));
    ui->m_logYaxis->setChecked(false);
    ui->m_logYaxis->setEnabled(false);
  } else {
    ui->m_checkCHGridLines->setText(i18n("Show horizontal grid lines"));
    ui->m_checkSVGridLines->setText(i18n("Show vertical grid lines"));
    ui->m_logYaxis->setEnabled(true);
  }
}

kMyMoneyReportConfigTabRangeDecl::kMyMoneyReportConfigTabRangeDecl(QWidget *parent)
    : QWidget(parent)
{
  ui = new Ui::kMyMoneyReportConfigTabRangeDecl;
  ui->setupUi(this);
  m_dateRange = new DateRangeDlg(this->parentWidget());
  ui->dateRangeGrid->addWidget(m_dateRange,0,0,1,2);
  MyDoubleValidator *dblVal = new MyDoubleValidator(KMyMoneyGlobalSettings::pricePrecision());
  ui->m_dataRangeStart->setValidator(dblVal);
  ui->m_dataRangeEnd->setValidator(dblVal);
  ui->m_dataMajorTick->setValidator(dblVal);
  ui->m_dataMinorTick->setValidator(dblVal);
  connect(ui->m_dataRangeStart, SIGNAL(editingFinished()), this, SLOT(slotEditingFinishedStart()));
  connect(ui->m_dataRangeEnd, SIGNAL(editingFinished()), this, SLOT(slotEditingFinishedEnd()));
  connect(ui->m_dataMajorTick, SIGNAL(editingFinished()), this, SLOT(slotEditingFinishedMajor()));
  connect(ui->m_dataMinorTick, SIGNAL(editingFinished()), this, SLOT(slotEditingFinishedMinor()));
  connect(ui->m_dataLock, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDataLockChanged(int)));
  emit ui->m_dataLock->currentIndexChanged(ui->m_dataLock->currentIndex());
}

kMyMoneyReportConfigTabRangeDecl::~kMyMoneyReportConfigTabRangeDecl()
{
  delete ui;
}

void kMyMoneyReportConfigTabRangeDecl::setRangeLogarythmic(bool set)
{
  // major and minor tick have no influence if axis is logarithmic so hide them
  if (set) {
    ui->lblDataMajorTick->hide();
    ui->lblDataMinorTick->hide();
    ui->m_dataMajorTick->hide();
    ui->m_dataMinorTick->hide();
  } else {
    ui->lblDataMajorTick->show();
    ui->lblDataMinorTick->show();
    ui->m_dataMajorTick->show();
    ui->m_dataMinorTick->show();
  }
}

void kMyMoneyReportConfigTabRangeDecl::slotEditingFinished(EDimension dim)
{
  qreal dataRangeStart = locale().toDouble(ui->m_dataRangeStart->text());
  qreal dataRangeEnd = locale().toDouble(ui->m_dataRangeEnd->text());
  qreal dataMajorTick = locale().toDouble(ui->m_dataMajorTick->text());
  qreal dataMinorTick = locale().toDouble(ui->m_dataMinorTick->text());
  if (dataRangeEnd < dataRangeStart) {  // end must be higher than start
    if (dim == eRangeEnd) {
      ui->m_dataRangeStart->setText(ui->m_dataRangeEnd->text());
      dataRangeStart = dataRangeEnd;
    } else {
      ui->m_dataRangeEnd->setText(ui->m_dataRangeStart->text());
      dataRangeEnd = dataRangeStart;
    }
  }
  if ((dataRangeStart != 0 || dataRangeEnd != 0)) { // if data range isn't going to be reset
    if ((dataRangeEnd - dataRangeStart) < dataMajorTick) // major tick cannot be greater than data range
      dataMajorTick = dataRangeEnd - dataRangeStart;

    if (dataMajorTick != 0 && // if major tick isn't going to be reset
        dataMajorTick < (dataRangeEnd - dataRangeStart) * 0.01) // constraint major tick to be greater or equal to 1% of data range
      dataMajorTick = (dataRangeEnd - dataRangeStart) * 0.01;   // that should produce more than 256 Y labels in KReportChartView::slotNeedUpdate

    ui->m_dataMajorTick->setText(locale().toString(dataMajorTick, 'f', KMyMoneySettings::pricePrecision()).remove(locale().groupSeparator()).remove(QRegularExpression("0+$")).remove(QRegularExpression("\\" + locale().decimalPoint() + "$")));
  }

  if (dataMajorTick < dataMinorTick) { // major tick must be higher than minor
    if (dim == eMinorTick) {
      ui->m_dataMajorTick->setText(ui->m_dataMinorTick->text());
      dataMajorTick = dataMinorTick;
    } else {
      ui->m_dataMinorTick->setText(ui->m_dataMajorTick->text());
      dataMinorTick = dataMajorTick;
    }
  }

  if (dataMinorTick < dataMajorTick * 0.1) { // constraint minor tick to be greater or equal to 10% of major tick
    dataMinorTick = dataMajorTick * 0.1;
    ui->m_dataMinorTick->setText(locale().toString(dataMinorTick, 'f', KMyMoneySettings::pricePrecision()).remove(locale().groupSeparator()).remove(QRegularExpression("0+$")).remove(QRegularExpression("\\" + locale().decimalPoint() + "$")));
  }
}

void kMyMoneyReportConfigTabRangeDecl::slotEditingFinishedStart()
{
  slotEditingFinished(eRangeStart);
}

void kMyMoneyReportConfigTabRangeDecl::slotEditingFinishedEnd()
{
  slotEditingFinished(eRangeEnd);
}

void kMyMoneyReportConfigTabRangeDecl::slotEditingFinishedMajor()
{
  slotEditingFinished(eMajorTick);
}

void kMyMoneyReportConfigTabRangeDecl::slotEditingFinishedMinor()
{
  slotEditingFinished(eMinorTick);
}

void kMyMoneyReportConfigTabRangeDecl::slotDataLockChanged(int index) {
  if (index == MyMoneyReport::dataOptionE::automatic) {
    ui->m_dataRangeStart->setText(QStringLiteral("0"));
    ui->m_dataRangeEnd->setText(QStringLiteral("0"));
    ui->m_dataMajorTick->setText(QStringLiteral("0"));
    ui->m_dataMinorTick->setText(QStringLiteral("0"));
    ui->m_dataRangeStart->setEnabled(false);
    ui->m_dataRangeEnd->setEnabled(false);
    ui->m_dataMajorTick->setEnabled(false);
    ui->m_dataMinorTick->setEnabled(false);
  } else {
    ui->m_dataRangeStart->setEnabled(true);
    ui->m_dataRangeEnd->setEnabled(true);
    ui->m_dataMajorTick->setEnabled(true);
    ui->m_dataMinorTick->setEnabled(true);
  }
}
