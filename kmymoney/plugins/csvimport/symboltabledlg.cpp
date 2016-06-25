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
#include <QResizeEvent>
#include <QScrollBar>

// ----------------------------------------------------------------------------
// KDE Headers

#include <KLocalizedString>
#include <KMessageBox>
#include <QPushButton>
#include <ui_csvdialog.h>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QVBoxLayout>


SymbolTableDlg::SymbolTableDlg()
{
  m_widget = new SymbolTableDlgDecl;
  QVBoxLayout *mainLayout = new QVBoxLayout;
  setLayout(mainLayout);
  mainLayout->addWidget(m_widget);
  m_widget->tableWidget->setToolTip(i18n("Symbols and Security Names present"));
  m_firstPass = true;
  m_validRowCount = 0;

  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
  QWidget *mainWidget = new QWidget(this);
  mainLayout->addWidget(mainWidget);
  m_buttonOK = buttonBox->button(QDialogButtonBox::Ok);
  m_buttonCancel = buttonBox->button(QDialogButtonBox::Cancel);
  m_buttonOK->setDefault(true);
  m_buttonOK->setShortcut(Qt::CTRL | Qt::Key_Return);
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  //PORTING SCRIPT: WARNING mainLayout->addWidget(buttonBox) must be last item in layout. Please move it.
  mainLayout->addWidget(buttonBox);
  setOrientation(Qt::Horizontal);
  m_buttonOK->setEnabled(false);

  connect(m_buttonCancel, SIGNAL(clicked()), this, SLOT(slotRejected()));
  connect(m_buttonOK, SIGNAL(clicked()), this, SLOT(slotEditSecurityCompleted()));
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
    item1->setText(i18nc("Security exists", "Exists"));
    item->setFlags(Qt::NoItemFlags); //                 \\ no editing of symbol allowed
    item2->setFlags(Qt::NoItemFlags); //                 \\ no editing of name allowed
  } else if (name.isEmpty()) {
    item1->setText(i18nc("Security has no name", "No name"));
    item->setFlags(Qt::NoItemFlags); //                 \\ no editing of symbol allowed
    item2->setText(QString());
    m_validRowCount += 1;
  } else if (symbol.isEmpty()) {
    item1->setText(i18nc("Security has no symbol", "No symbol"));
    item->setText(QString());
    item2->setFlags(Qt::NoItemFlags); //                 \\ no editing of name allowed
    m_validRowCount += 1;
  } else {
    item1->setText(i18n("OK"));
    item->setFlags(Qt::NoItemFlags); //                 \\ no editing of symbol allowed
    item2->setFlags(Qt::NoItemFlags); //                 \\ no editing of name allowed
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
  QString text;
  QString symbol;

  m_selectedItems = m_widget->tableWidget->selectedItems();
  switch (item->column()) {
    case 0:
      symbol = item->text();  //  Edit the ticker symbol
      if (m_selectedItems.count() > 1) {
        //  Allow multiple selections only in symbol column
        foreach (QTableWidgetItem *  selectItem, m_selectedItems) {
          //  Each item clicked causes a pass through here
          selectItem->setText(symbol);
        }
      } else if (m_selectedItems.count() == 1) {
        QString name = m_widget->tableWidget->item(item->row(), 2)->text();
        for (int i = 0; i < m_widget->tableWidget->rowCount(); i ++) {
          //    Look for entry with same name as selected item
          if ((!name.isEmpty()) && (m_widget->tableWidget->item(i, 2)->text()) == name) {
            //  and apply the same security name
            m_widget->tableWidget->item(i, 0)->setText(symbol);
            if (!symbol.isEmpty())
              m_widget->tableWidget->item(i, 1)->setText(i18n("OK"));
            else
              m_widget->tableWidget->item(i, 1)->setText(i18nc("Security has no symbol", "No symbol"));
          }
       }
      }
      break;
    case 1:
      text = item->text();
      if (text.compare(i18n("OK"),Qt::CaseInsensitive) == 0 ||
          text.compare(i18nc("Security exists", "Exists"),Qt::CaseInsensitive) == 0)
        m_validRowCount += 1;
      else
        m_validRowCount -= 1;
      if (m_validRowCount == m_widget->tableWidget->rowCount())
        m_buttonOK->setEnabled(true);
      else
        m_buttonOK->setEnabled(false);
      return;
    case 2:
      name = item->text();  //    Edit the security name
      if (m_selectedItems.count() > 1) {
        //  Allow multiple selections only in name column
        foreach (QTableWidgetItem *  selectItem, m_selectedItems) {
          //  Each item clicked causes a pass through here
          selectItem->setText(name);
        }
      } else if (m_selectedItems.count() == 1) {
        QString symbol = m_widget->tableWidget->item(item->row(), 0)->text();
        for (int i = 0; i < m_widget->tableWidget->rowCount(); i ++) {
          //    Look for entry with same symbol as selected item
          if ((!symbol.isEmpty()) && (m_widget->tableWidget->item(i, 0)->text()) == symbol) {
            //  and apply the same security name
            m_widget->tableWidget->item(i, 2)->setText(name);
            if (!name.isEmpty())
              m_widget->tableWidget->item(i, 1)->setText(i18n("OK"));
            else
              m_widget->tableWidget->item(i, 1)->setText(i18nc("Security has no name", "No name"));
          }
       }
      }
      break;
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
  }
  slotAccepted();
  return;
}
