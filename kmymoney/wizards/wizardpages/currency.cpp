/*
    SPDX-FileCopyrightText: 2007 Thomas Baumgart <Thomas Baumgart <ipwizard@users.sourceforge.net>>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
