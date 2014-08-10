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
#include <QPushButton>
#include <ui_csvdialog.h>


SymbolTableDlg::SymbolTableDlg()
{
  m_widget = new SymbolTableDlgDecl;
  setMainWidget(m_widget);
  m_widget->tableWidget->setToolTip(i18n("Symbols and Security Names present"));
  m_firstPass = true;

  setButtons(KDialog::Cancel | KDialog::Ok);
  setButtonsOrientation(Qt::Horizontal);
  enableButtonOk(true);

  connect(this, SIGNAL(cancelClicked()), this, SLOT(slotRejected()));
  connect(this, SIGNAL(okClicked()), this, SLOT(slotEditSecurityCompleted()));
  connect(this->m_widget->tableWidget,  SIGNAL(itemChanged(QTableWidgetItem*)), this,  SLOT(slotItemChanged(QTableWidgetItem*)));
  connect(this->m_widget->tableWidget,  SIGNAL(itemClicked(QTableWidgetItem*)), this,  SLOT(slotItemClicked(QTableWidgetItem*)));
}

SymbolTableDlg::~SymbolTableDlg()
{
  delete m_widget;
}

void SymbolTableDlg::displayLine(int& row, QString& symbol, const QString& name, bool& exists)
{
  int thisRow = row;
  int basicWindowHeight = 150;
  int height = 0;
  if (thisRow > 9) {
    thisRow = 9;
  }
  height = (thisRow += 1) * m_widget->tableWidget->rowHeight(0);
  this->resize(width(), height + basicWindowHeight); // + labels + footer
  QTableWidgetItem* item = new QTableWidgetItem;  //    symbol for UI
  item->setText(symbol);
  QTableWidgetItem* item1 = new QTableWidgetItem;  //   exists flag for UI
  item1->setFlags(Qt::NoItemFlags);
  item1->setSizeHint(QSize(60, 30));
  QTableWidgetItem* item2 = new QTableWidgetItem;  //   security name for UI
  item2->setText(name);
  if (exists) {  //                                     already exists in KMM
    item1->setText(i18nc("Confirm", "Yes"));
    item->setFlags(Qt::NoItemFlags);  //                ...so no editing allowed
    item2->setFlags(Qt::NoItemFlags);
  } else {
    item1->setText(QString());
  }
  item->setTextAlignment(Qt::AlignLeft);
  item1->setTextAlignment(Qt::AlignLeft);
  m_widget->tableWidget->setRowCount(row + 1);
  m_widget->tableWidget->setItem(row, 0, item);  //     add symbol to UI here
  m_widget->tableWidget->setItem(row, 1, item1);  //    add 'exists'
  m_widget->tableWidget->setItem(row, 2, item2);  //    add security name
  m_widget->tableWidget->resizeColumnsToContents();
}

void SymbolTableDlg::slotAccepted()
{
  accept();
  connect(this, SIGNAL(namesEdited()), m_csvDialog, SLOT(slotNamesEdited()));

  emit namesEdited();
}

void SymbolTableDlg::slotRejected()
{
  reject();
}

void SymbolTableDlg::slotItemChanged(QTableWidgetItem* item)
{
  QString name;
  QString symbol;

  switch (item->column()) {
    case 0:
      symbol = item->text();  //  Edit the ticker symbol
      break;
    case 1:
      return;
    case 2:
      name = item->text();  //    Edit the security name
      break;
  }

  m_selectedItems = m_widget->tableWidget->selectedItems();
  if ((m_selectedItems.count() > 1) && (item->column() == 0)) {
    //  Allow multiple selections only in symbol column
    foreach (QTableWidgetItem *  selectItem, m_selectedItems) {
      //  Each item clicked causes a pass through here
      selectItem->setText(symbol);
    }
  }
  if ((m_selectedItems.count() == 1)  && (!name.isEmpty())) {
    QString symbol = m_widget->tableWidget->item(item->row(), 0)->text();
    for (int i = 0; i < m_widget->tableWidget->rowCount(); i ++) {
      //    Look for entry with same symbol as selected item
      if ((!symbol.isEmpty()) && (m_widget->tableWidget->item(i, 0)->text()) == symbol) {
        //  and apply the same security name
        m_widget->tableWidget->item(i, 2)->setText(name);
      }
    }
  }
}

void SymbolTableDlg::slotItemClicked(QTableWidgetItem* item)
{
  QString symbol;
  if (item->column() != 0) {
    //  Only allow single selection of names
    m_widget->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    return;
  }
  //  Allow several selections in symbol col
  m_widget->tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
  symbol = item->text();
  //  Get multiple selections successively
  m_selectedItems = m_widget->tableWidget->selectedItems();
  if (m_selectedItems.count() > 1) {
    //  Copy symbol to selected cells
    foreach (QTableWidgetItem *  selectItem, m_selectedItems) {
      selectItem->setText(symbol);
    }
  }
}

void SymbolTableDlg::slotEditSecurityCompleted()
{
  MyMoneyStatement::Security security;
  //  OK clicked
  for (int row = 0; row < m_widget->tableWidget->rowCount(); row++) {
    QString symbol = m_widget->tableWidget->item(row, 0)->text();
    if (symbol.isEmpty()) {
      continue;
    }
    //  Build list of securities
    QString name = m_widget->tableWidget->item(row, 2)->text();
    security.m_strName = name;
    m_securityName = name;
    security.m_strSymbol = symbol;
    m_csvDialog->m_investProcessing->m_listSecurities << security;
    QTableWidgetItem *item = new QTableWidgetItem;
    item->setText(symbol);
    item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    //  Apply any added symbols to main UI
    m_csvDialog->ui->tableWidget->setItem(m_csvDialog->m_investProcessing->m_startLine + row - 1, m_csvDialog->m_investProcessing->symbolColumn(), item);
  }
  slotAccepted();
  return;
}
