/*******************************************************************************
*                               securitiesdlg.cpp
*                               ------------------
* begin                       : Sun Sept 11 2011
* copyright                   : (C) 2011 by Allan Anderson
* email                       : agander93@gmail.com
*  copyright                 : (C) 2016 by Łukasz Wojnilowicz
* email                       : lukasz.wojnilowicz@gmail.com
********************************************************************************/

/*******************************************************************************
*                                                                              *
*   This program is free software; you can redistribute it and/or modify       *
*   it under the terms of the GNU General Public License as published by       *
*   the Free Software Foundation; either version 2 of the License, or          *
*   (at your option) any later version.                                        *
*                                                                              *
********************************************************************************/

#include "securitiesdlg.h"

#include <QTableWidgetItem>
#include <QPushButton>

#include "ui_securitiesdlg.h"

SecuritiesDlg::SecuritiesDlg() : ui(new Ui::SecuritiesDlg)
{
  ui->setupUi(this);
  m_validRowCount = 0;
  m_buttonOK = ui->buttonBox->button(QDialogButtonBox::Ok);
  m_buttonOK->setDefault(true);
  m_buttonOK->setShortcut(Qt::CTRL | Qt::Key_Return);
  m_buttonOK->setEnabled(false);

  connect(ui->tableWidget,  SIGNAL(itemChanged(QTableWidgetItem*)), this,  SLOT(slotItemChanged(QTableWidgetItem*)));
}

SecuritiesDlg::~SecuritiesDlg()
{
  delete ui;
}

void SecuritiesDlg::displayLine(const QString symbol, const QString name)
{
  QTableWidgetItem* symbolItem = new QTableWidgetItem;
  QTableWidgetItem* statusItem = new QTableWidgetItem;
  QTableWidgetItem* nameItem = new QTableWidgetItem;

  statusItem->setFlags(Qt::NoItemFlags); // no editing of status allowed
  symbolItem->setText(symbol);
  nameItem->setText(name);
  if (name.isEmpty()) {
    statusItem->setText(i18nc("Security has no name", "No name"));
    symbolItem->setFlags(Qt::NoItemFlags);               // no editing of symbol allowed
  } else if (symbol.isEmpty()) {
    statusItem->setText(i18nc("Security has no symbol", "No symbol"));
    nameItem->setFlags(Qt::NoItemFlags);                 // no editing of name allowed
  }
  int nextRow = ui->tableWidget->rowCount();
  ui->tableWidget->setRowCount(nextRow + 1);
  ui->tableWidget->blockSignals(true); // don't run slotItemChanged on uninitialized Items
  ui->tableWidget->setItem(nextRow, ColumnStatus, statusItem);
  ui->tableWidget->setItem(nextRow, ColumnSymbol, symbolItem);
  ui->tableWidget->setItem(nextRow, ColumnName, nameItem);
  ui->tableWidget->blockSignals(false);
  ui->tableWidget->resizeColumnsToContents();
}

void SecuritiesDlg::slotItemChanged(QTableWidgetItem* item)
{
  switch (item->column()) {
  case ColumnStatus:
    if (item->text().compare(i18n("OK"), Qt::CaseInsensitive) == 0)
      m_validRowCount += 1;
    else
      m_validRowCount -= 1;
    if (m_validRowCount == ui->tableWidget->rowCount())
      m_buttonOK->setEnabled(true);
    else
      m_buttonOK->setEnabled(false);
    return;
  case ColumnSymbol:
    if (!item->text().isEmpty())
      ui->tableWidget->item(item->row(), ColumnStatus)->setText(i18n("OK"));
    else
      ui->tableWidget->item(item->row(), ColumnStatus)->setText(i18nc("Security has no symbol", "No symbol"));
    break;
  case ColumnName:
    if (!item->text().isEmpty())
      ui->tableWidget->item(item->row(), ColumnStatus)->setText(i18n("OK"));
    else
      ui->tableWidget->item(0, ColumnStatus)->setText(i18nc("Security has no name", "No name"));
    break;
  }
}
