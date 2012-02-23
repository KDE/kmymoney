/*******************************************************************************
*                               symboltabledlg.cpp
*                               ------------------
* begin                       : Sun Sept 11 2011
* copyright                   : (C) 2011 by Allan Anderson
* email                       : agander93@gmail.com
********************************************************************************/

/*******************************************************************************
*                                                                              *
*   This program is free software; you can redistribute it and/or modify       *
*   it under the terms of the GNU General Public License as published by       *
*   the Free Software Foundation; either version 2 of the License, or          *
*   (at your option) any later version.                                        *
*                                                                              *
********************************************************************************/

#include "symboltabledlg.h"
#include "csvdialog.h"
#include "investprocessing.h"
// ----------------------------------------------------------------------------
// QT Headers

#include <QtCore/QDebug>
#include <QtCore/QPointer>
#include <QtGui/QResizeEvent>
#include <QtGui/QScrollBar>

// ----------------------------------------------------------------------------
// KDE Headers

#include <KInputDialog>
#include <KLocale>
#include <KMessageBox>
#include <KGlobal>
#include <KPushButton>


SymbolTableDlg::SymbolTableDlg()
{
  m_widget = new SymbolTableDlgDecl;
  setMainWidget(m_widget);

  m_widget->tableWidget->setToolTip(i18n("Symbols and Security Names present"));
  m_mainWidth = m_widget->tableWidget->size().width();

  setButtons(KDialog::Cancel | KDialog::Ok);
  setButtonsOrientation(Qt::Horizontal);
  enableButtonOk(true);

  connect(this, SIGNAL(cancelClicked()), this, SLOT(slotRejected()));
  connect(this, SIGNAL(okClicked()), this, SLOT(slotEditSecurityClicked()));
  connect(this->m_widget->tableWidget,  SIGNAL(itemChanged(QTableWidgetItem*)), this,  SLOT(slotItemChanged(QTableWidgetItem*)));
}

SymbolTableDlg::~SymbolTableDlg()
{
  delete m_widget;
}

void SymbolTableDlg::displayLine(int& row, QString& symbol, const QString& name, bool& exists)
{
  QTableWidgetItem* item = new QTableWidgetItem;         //symbol for UI
  item->setText(symbol);
  QTableWidgetItem* item1 = new QTableWidgetItem;        //exist flag for UI
  item1->setSizeHint(QSize(60, 30));
  if (exists) {
    item1->setText(i18nc("Confirm", "Yes"));
  } else {
    item1->setText(QString());
  }
  QTableWidgetItem* item2 = new QTableWidgetItem;        //security name for UI
  item2->setText(name);
  item->setTextAlignment(Qt::AlignLeft);
  item1->setTextAlignment(Qt::AlignLeft);
  m_widget->tableWidget->setRowCount(row + 1);
  m_widget->tableWidget->setItem(row, 0, item);          //add items to UI here
  m_widget->tableWidget->setItem(row, 1, item1);
  m_widget->tableWidget->setItem(row, 2, item2);
  m_widget->tableWidget->resizeColumnsToContents();
}

void SymbolTableDlg::slotAccepted()
{
  connect(this, SIGNAL(namesEdited()), m_csvDialog, SLOT(slotNamesEdited()));

  emit namesEdited();
  accept();
}

void SymbolTableDlg::slotRejected()
{
  reject();
}

void SymbolTableDlg::slotItemChanged(QTableWidgetItem* item)
{
  if (item->column() < 2) {     //  Only edit names.
    return;
  }
  QString name = item->text();
  m_selectedItems = m_widget->tableWidget->selectedItems();

  if (m_selectedItems.count() > 1) {
    foreach (QTableWidgetItem *  selectItem, m_selectedItems) {
      selectItem->setText(item->text());
    }
  }
  if (m_selectedItems.count() == 1) {
    QString symbol = m_widget->tableWidget->item(item->row(), 0)->text();
    for (int i = 0; i < m_widget->tableWidget->rowCount(); i ++) {
      if (m_widget->tableWidget->item(i, 0)->text() == symbol) {
        //      ...and edit their names too.
        m_widget->tableWidget->item(i, 2)->setText(name);
      }
    }
  }
}

void SymbolTableDlg::slotEditSecurityClicked()
{
  MyMoneyStatement::Security security;

  for (int i = 0; i < m_widget->tableWidget->rowCount(); i++) {
    QString name = m_widget->tableWidget->item(i, 2)->text();

    security.m_strName = name;
    m_securityName = name;
    security.m_strSymbol = m_widget->tableWidget->item(i, 0)->text();
    m_csvDialog->m_investProcessing->m_listSecurities << security;
  }
  slotAccepted();
  return;
}
