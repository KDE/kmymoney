/*******************************************************************************
*                               redefinedlg.cpp
*                             ------------------
* begin                       : Sat Jan 01 2010
* copyright                   : (C) 2010 by Allan Anderson
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

#include "redefinedlg.h"

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
#include <KIconLoader>
#include <KGlobal>

RedefineDlg::RedefineDlg()
{
  m_accountName.clear();
  m_amountColumn = 0;
  m_columnTotalWidth = 0;
  m_maxWidth = 0;
  m_maxCol = 0;
  m_mainHeight = 0;
  m_mainWidth = 0;
  m_priceColumn = 0;
  m_quantityColumn = 0;
  m_ret = 0;
  m_typeColumn = 0;
  m_symbolColumn = 0;
  m_detailColumn = 0;

  m_price = 0;
  m_quantity = 0;
  m_amount = 0;

  m_typesList << "buy" << "sell" << "divx" << "reinvdiv" << "shrsin" << "shrsout" << "intinc";

  m_iconYes = QPixmap(KIconLoader::global()->loadIcon("dialog-ok", KIconLoader::Small, KIconLoader::DefaultState));
  m_iconNo = QPixmap(KIconLoader::global()->loadIcon("dialog-cancel", KIconLoader::Small, KIconLoader::DefaultState));

  m_widget = new RedefineDlgDecl();
  setMainWidget(m_widget);

  m_widget->tableWidget->setToolTip(i18n("Results table"));
  m_widget->tableWidget->setRowCount(2);
  m_mainWidth = m_widget->tableWidget->size().width();
  m_mainHeight = m_widget->tableWidget->size().height();

  this->enableButtonOk(false);
  m_widget->kcombobox_Actions->setCurrentIndex(-1);

  connect(m_widget->kcombobox_Actions, SIGNAL(activated(int)), this, SLOT(slotNewActionSelected(int)));
  connect(this, SIGNAL(okClicked()), this, SLOT(slotAccepted()));
  connect(this, SIGNAL(cancelClicked()), this, SLOT(slotRejected()));
}

RedefineDlg::~RedefineDlg()
{
  delete m_widget;
}

void RedefineDlg::displayLine(const QString& info)
{
  this->enableButtonOk(false);
  QString txt;
  txt.setNum(m_typeColumn + 1);
  m_widget->label_actionCol->setText(i18n("Column ") + txt);
  m_widget->label_info->setText(info);
  m_maxCol = m_columnList.count();
  m_widget->tableWidget->setColumnCount(m_maxCol);

  QBrush brush;
  QColor colr;
  colr.setRgb(255, 0, 127, 100);
  brush.setColor(colr);
  brush.setStyle(Qt::SolidPattern);
  int row;
  m_columnTotalWidth = 0;
  m_maxWidth = 0;
  m_widget->tableWidget->setRowCount(2);
  for (int col = 0; col < m_maxCol; col++) {
    row = 1;
    txt = m_columnList[col];
    txt = txt.remove('"');

    QTableWidgetItem *item = new QTableWidgetItem;//  add items to UI
    item->setText(txt);
    m_widget->tableWidget->setItem(row, col, item);  // add items to UI here
    if (m_typeColumn == col) {
      item->setBackground(brush);
    }
    row = 0;
    if (col == m_quantityColumn) {
      QTableWidgetItem *item = new QTableWidgetItem;//        add items to UI
      item->setText(i18n("Quantity"));
      m_widget->tableWidget->setItem(row, col, item);
    } else if (col == m_priceColumn) {
      QTableWidgetItem *item = new QTableWidgetItem;//        add items to UI
      item->setText(i18n("Price"));
      m_widget->tableWidget->setItem(row, col, item);
    } else if (col == m_amountColumn) {
      QTableWidgetItem *item = new QTableWidgetItem;//        add items to UI
      item->setText(i18n("Amount"));
      m_widget->tableWidget->setItem(row, col, item);
    } else if (col == m_typeColumn) {
      QTableWidgetItem *item = new QTableWidgetItem;//        add items to UI
      item->setText(i18n("Type"));
      m_widget->tableWidget->setItem(row, col, item);
    } else if (col == m_detailColumn) {
      QTableWidgetItem *item = new QTableWidgetItem;//        add items to UI
      item->setText(i18n("Detail"));
      m_widget->tableWidget->setItem(row, col, item);
    }
  }
  m_widget->tableWidget->resizeColumnsToContents();
  for (int col = 0; col < m_maxCol; col++) {
    m_columnTotalWidth += m_widget->tableWidget->columnWidth(col);
  }
  if (m_columnTotalWidth > m_maxWidth) {
    m_maxWidth = m_columnTotalWidth;
  }
  updateWindow();
}

void RedefineDlg::slotAccepted()
{
  m_ret = KMessageBox::Ok;
  m_columnList[m_typeColumn] = m_newType;
  m_inBuffer = m_columnList.join(",");
  emit changedType(m_newType);
  m_widget->kcombobox_Actions->setCurrentIndex(-1);
  accept();
}

void RedefineDlg::slotNewActionSelected(const int& index)
{
  m_newType = m_typesList[index];
  if (m_okTypeList.contains(m_newType)) {
    QTableWidgetItem *item = new QTableWidgetItem;//        add new type to UI
    item->setText(m_newType);
    m_widget->tableWidget->setItem(1, m_typeColumn, item);
    this->enableButtonOk(true);
  }
}

void RedefineDlg::slotRejected()
{
  KMessageBox::information(nullptr, i18n("<center>No valid activity type found for this transaction.</center>"
                                   "<center>Please check the parameters supplied.</center>"));
  m_ret = KMessageBox::Cancel;
  reject();
}


int RedefineDlg::checkValid(const QString& type, QString info)
{
  int ret = -1;
  m_okTypeList.clear();
  m_maxCol = m_columnList.count();
  this->enableButtonOk(false);
  convertValues();
  if ((m_priceColumn < 1) || (m_priceColumn >= m_maxCol) ||
      (m_quantityColumn < 1) || (m_quantityColumn >= m_maxCol) ||
      (m_amountColumn < 1) || (m_amountColumn >= m_maxCol)) {
    info = i18n("There is a problem with the columns selected\nfor 'Price', 'Quantity and 'Amount'.\n\
You will need to reselect those columns.");
    ret = suspectType(info);
    return ret;
  }
  if ((type == "reinvdiv") || (type == "buy") || (type == "sell")) {
    m_widget->label_info->setText("OK");
    if ((m_quantity.isPositive()) && (m_price.isPositive()) && (!m_amount.isZero())) {
      m_okTypeList << "reinvdiv" << "buy" << "sell";
      if ((m_accountName.isEmpty()) && (type != "reinvdiv")) {
        m_accountName =  inputParameter(i18n("Enter the name of the Brokerage or Checking Account used for the transfer of funds:"));
        if (m_accountName.isEmpty())
          return KMessageBox::Cancel;
      }
      m_newType = type;
      this->enableButtonOk(true);
      return KMessageBox::Ok;
    }
    ret = suspectType(info);
    return ret;
  } else if ((type.toLower() == "divx") || (type.toLower() == "intinc")) {
    m_widget->label_info->setText("OK");
    if ((m_quantity.isZero()) && (m_price.isZero()) && (!m_amount.isZero())) {
      m_okTypeList << "divx" << "intinc";
      if (m_accountName.isEmpty())
        m_accountName =  inputParameter(i18n("Enter the name of the Brokerage or Checking Account used for the transfer of funds:"));
      if (m_accountName.isEmpty())
        return KMessageBox::Cancel;
      m_newType = type;
      this->enableButtonOk(true);
      return KMessageBox::Ok;
    }
    //                    validity suspect
    ret = suspectType(info);
    return ret;
  } else if ((type == "shrsin") || (type == "shrsout")) {
    m_widget->label_info->setText("OK");
    if ((m_quantity.isPositive()) && (m_price.isZero()) && (m_amount.isZero())) {
      m_okTypeList << "shrsin" << "shrsout";
      m_newType = type;
      this->enableButtonOk(true);
      return KMessageBox::Ok;
    }
    m_okTypeList.clear();
    ret = suspectType(info);
    return ret;
  }
  return KMessageBox::Cancel;
}

int RedefineDlg::suspectType(const QString& info)
{
  displayLine(info);
  buildOkTypeList();
  for (int i = 0; i < m_typesList.count() ; i++) {  //  m_okTypeList.count()
    if (m_okTypeList.contains(m_typesList[i])) {
      m_widget->kcombobox_Actions->setItemIcon(i, m_iconYes);
    } else {
      m_widget->kcombobox_Actions->setItemIcon(i, m_iconNo);
    }
  }

  int ret = exec();
  if (ret == QDialog::Rejected)
    ret = KMessageBox::Cancel;
  return ret;
}

void RedefineDlg::buildOkTypeList()
{
  convertValues();

  m_okTypeList.clear();
  MyMoneyMoney zero = MyMoneyMoney();
  if ((m_quantity > zero) && (m_price > zero) && (m_amount != zero))
    m_okTypeList << "reinvdiv" << "buy" << "sell";
  else if ((m_quantity == zero) && (m_price == zero) && (m_amount != zero)) {
    m_okTypeList << "divx" << "intinc";
  } else if ((m_quantity > zero) && (m_price == zero) && (m_amount == zero)) {
    m_okTypeList << "shrsin" << "shrsout";
  } else {
    m_okTypeList.clear();
    KMessageBox::sorry(this, i18n("The values in the columns you have selected\ndo not match any expected investment type.\nPlease check the fields in the current transaction,\nand also your selections.")
                       , i18n("CSV import"));
  }
}

QString RedefineDlg::inputParameter(const QString& aName)
{
  bool ok;
  static QString accntName;
  accntName = KInputDialog::getText(i18n("Enter Account Name"), aName, QString(), &ok, 0, 0, 0);

  if (ok && !accntName.isEmpty())
    return accntName;
  else return "";
}

void RedefineDlg::resizeEvent(QResizeEvent * event)
{
  event->accept();

  updateWindow();
}

void RedefineDlg::updateWindow()
{
  int hght = 6 + (m_widget->tableWidget->rowHeight(0) * 2);
  hght += m_widget->tableWidget->horizontalHeader()->height();  //      frig factor for horiz. headers?
  if (m_maxWidth > (m_mainWidth - 22)) {  //                            exclude borders
    hght += 15;  //                                                     allow for hor. scroll bar
  }
  m_widget->tableWidget->setFixedHeight(hght);
}

void RedefineDlg::convertValues()
{
  QString txt;
  QString txt1;
  if (m_priceColumn < m_columnList.count())     //         Ensure this is valid column
    m_price = m_columnList[m_priceColumn].remove('"');
  if (m_quantityColumn <  m_columnList.count())     //     Ensure this is valid column
    m_quantity = m_columnList[m_quantityColumn].remove(QRegExp("[\"-]"));  //  Remove unwanted -ve sign in quantity.
  if (m_amountColumn <  m_columnList.count())     //       Ensure this is valid column
    txt = m_columnList[m_amountColumn];
  if ((txt.startsWith('"')) && (!txt.endsWith('"')))  {
    txt1 = m_columnList[m_amountColumn + 1];
    txt += txt1;
  }
  txt = txt.remove('"');

  if (txt.contains(')')) {       //          replace negative ( ) with '-'
    txt = '-' + txt.remove(QRegExp("[(),]"));
  }
  m_amount = txt;
}

void RedefineDlg::setAmountColumn(int col)
{
  m_amountColumn = col;
}

void RedefineDlg::setPriceColumn(int col)
{
  m_priceColumn = col;
}

void RedefineDlg::setQuantityColumn(int col)
{
  m_quantityColumn = col;
}

void RedefineDlg::setTypeColumn(int col)
{
  m_typeColumn = col;
}

void RedefineDlg::setSymbolColumn(int col)
{
  m_symbolColumn = col;
}

void RedefineDlg::setDetailColumn(int col)
{
  m_detailColumn = col;
}

QString RedefineDlg::accountName()
{
  return m_accountName;
}

void RedefineDlg::clearAccountName()
{
  m_accountName.clear();
}

void RedefineDlg::setAccountName(const QString& val)
{
  m_accountName = val;
}

void RedefineDlg::setInBuffer(const QString& val)
{
  m_inBuffer = val;
}

void RedefineDlg::setColumnList(const QStringList& list)
{
  m_columnList = list;
}
