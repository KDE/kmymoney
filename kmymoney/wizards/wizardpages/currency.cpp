/***************************************************************************
                             userinfo.cpp
                             -------------------
    begin                : Fri Jun  1 2007
    copyright            : (C) 2007 Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "currency.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QTreeWidget>
#include <QTreeWidgetItem>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_currency.h"
#include "mymoneysecurity.h"

Currency::Currency(QWidget* parent) :
  QWidget(parent),
  ui(new Ui::Currency)
{
  ui->setupUi(this);
  ui->m_currencyList->setAllColumnsShowFocus(true);
  ui->m_currencyList->setColumnWidth(0, size().width()*6 / 10);
}

Currency::~Currency()
{
  delete ui;
}

QTreeWidgetItem* Currency::insertCurrency(const MyMoneySecurity& sec)
{
  QStringList item = QStringList();
  item.append(sec.name());
  item.append(QString(sec.id()));
  item.append(sec.tradingSymbol());

  return new QTreeWidgetItem(ui->m_currencyList, item);
}

void Currency::selectCurrency(const MyMoneySecurity& sec)
{
  QList<QTreeWidgetItem*> selectedItems = ui->m_currencyList->findItems(sec.id(), Qt::MatchExactly, 1);
  QList<QTreeWidgetItem*>::iterator itemIt = selectedItems.begin();
  while (itemIt != selectedItems.end()) {
    (*itemIt)->setSelected(true);
    ui->m_currencyList->scrollToItem(*itemIt);
  }
}

QString Currency::selectedCurrency() const
{
  QString id;

  if (ui->m_currencyList->selectedItems().size() > 0) {
    id = ui->m_currencyList->selectedItems().at(0)->text(1);
  }
  return id;
}
