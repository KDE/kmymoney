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
#include <QScrollBar>
#include <QInputDialog>
#include <QDesktopWidget>

// ----------------------------------------------------------------------------
// KDE Headers

#include <KLocalizedString>
#include <KMessageBox>
#include <KIconLoader>
#include <KConfigGroup>
#include <KColorScheme>

RedefineDlg::RedefineDlg()
{
  m_ret = 0;

  m_colorBrush = KColorScheme(QPalette::Normal).background(KColorScheme::PositiveBackground);
  m_colorBrushText = KColorScheme(QPalette::Normal).foreground(KColorScheme::PositiveText);
  m_errorBrush = KColorScheme(QPalette::Normal).background(KColorScheme::NegativeBackground);
  m_errorBrushText = KColorScheme(QPalette::Normal).foreground(KColorScheme::NegativeText);

  m_typesList << MyMoneyStatement::Transaction::eaBuy <<
                 MyMoneyStatement::Transaction::eaSell <<
                 MyMoneyStatement::Transaction::eaCashDividend <<
                 MyMoneyStatement::Transaction::eaReinvestDividend <<
                 MyMoneyStatement::Transaction::eaShrsin <<
                 MyMoneyStatement::Transaction::eaShrsout <<
                 MyMoneyStatement::Transaction::eaInterest;

  m_iconYes = QPixmap(KIconLoader::global()->loadIcon("dialog-ok", KIconLoader::Small, KIconLoader::DefaultState));
  m_iconNo = QPixmap(KIconLoader::global()->loadIcon("dialog-cancel", KIconLoader::Small, KIconLoader::DefaultState));

  m_widget = new RedefineDlgDecl();
  QVBoxLayout *mainLayout = new QVBoxLayout;
  setLayout(mainLayout);
  mainLayout->addWidget(m_widget);

  m_widget->tableWidget->setToolTip(i18n("Results table"));
  m_widget->tableWidget->setRowCount(2);

  m_buttonOK = m_widget->buttonBox->button(QDialogButtonBox::Ok);
  m_buttonCancel = m_widget->buttonBox->button(QDialogButtonBox::Cancel);

  m_buttonOK->setEnabled(false);
  m_widget->kcombobox_Actions->setCurrentIndex(-1);

  connect(m_widget->kcombobox_Actions, SIGNAL(activated(int)), this, SLOT(slotNewActionSelected(int)));
  connect(m_buttonOK, SIGNAL(clicked()), this, SLOT(slotAccepted()));
  connect(m_buttonCancel, SIGNAL(clicked()), this, SLOT(slotRejected()));
}

RedefineDlg::~RedefineDlg()
{
  delete m_widget;
}

void RedefineDlg::displayLine(const QString& info)
{
  m_buttonOK->setEnabled(false);
  QString txt;
  txt.setNum(m_colTypeNum.value(InvestProcessing::ColumnType) + 1);
  m_widget->label_actionCol->setText(i18n("Column ") + txt);
  m_widget->label_info->setText(info);
  m_maxCol = m_columnList.count();
  m_widget->tableWidget->setColumnCount(m_maxCol);
  int row;
  m_widget->tableWidget->setRowCount(2);
  for (int col = 0; col < m_maxCol; col++) {
    row = 1;
    txt = m_columnList[col];
    txt = txt.remove('"');

    QTableWidgetItem *item = new QTableWidgetItem;//  add items to UI
    item->setText(txt);
    m_widget->tableWidget->setItem(row, col, item);  // add items to UI here
    if (m_colTypeNum.value(InvestProcessing::ColumnType) == col) {
      item->setBackground(m_errorBrush);
      item->setForeground(m_errorBrushText);
    }
    row = 0;
    item = new QTableWidgetItem;//        add items to UI
    if (m_colTypeNum.value(InvestProcessing::ColumnQuantity) == col)
      item->setText(m_colTypeName.value(InvestProcessing::ColumnQuantity));
    else if (m_colTypeNum.value(InvestProcessing::ColumnPrice) == col)
      item->setText(m_colTypeName.value(InvestProcessing::ColumnPrice));
    else if (m_colTypeNum.value(InvestProcessing::ColumnAmount) == col)
      item->setText(m_colTypeName.value(InvestProcessing::ColumnAmount));
    else if (m_colTypeNum.value(InvestProcessing::ColumnType) == col)
      item->setText(m_colTypeName.value(InvestProcessing::ColumnType));
    else if (m_colTypeNum.value(InvestProcessing::ColumnName) == col)
      item->setText(m_colTypeName.value(InvestProcessing::ColumnName));
    m_widget->tableWidget->setItem(row, col, item);
  }
}

void RedefineDlg::slotAccepted()
{
  m_ret = KMessageBox::Ok;
  m_columnList[m_colTypeNum.value(InvestProcessing::ColumnType)] = m_newType;
  m_widget->kcombobox_Actions->setCurrentIndex(-1);
  accept();
}

void RedefineDlg::slotNewActionSelected(const int& index)
{
  m_newType = m_typesList[index];
  QTableWidgetItem *item = m_widget->tableWidget->item(1, m_colTypeNum.value(InvestProcessing::ColumnType));
  if (m_validActionTypes.contains(m_newType)) {
    item->setBackground(m_colorBrush);
    item->setForeground(m_colorBrushText);
    m_buttonOK->setEnabled(true);
  } else {
    item->setBackground(m_errorBrush);
    item->setForeground(m_errorBrushText);
    m_buttonOK->setEnabled(false);
  }
}

void RedefineDlg::slotRejected()
{
  KMessageBox::information(0, i18n("<center>No valid activity type found for this transaction.</center>"
                                   "<center>Please check the parameters supplied.</center>"));
  m_ret = KMessageBox::Cancel;
  reject();
}

MyMoneyStatement::Transaction::EAction RedefineDlg::askActionType(const QString& info)
{
  displayLine(info);
  for (int i = 0; i < m_typesList.count() ; i++) {  //  m_validActionTypes.count()
    if (m_validActionTypes.contains(m_typesList[i]))
      m_widget->kcombobox_Actions->setItemIcon(i, m_iconYes);
    else
      m_widget->kcombobox_Actions->setItemIcon(i, m_iconNo);
  }

  int ret = exec();
  if (ret == QDialog::Accepted)
    return m_newType;
  return MyMoneyStatement::Transaction::eaNone;
}

void RedefineDlg::setColumnTypeName(QMap<InvestProcessing::columnTypeE, QString> &colTypeName)
{
  m_colTypeName = colTypeName;
}

void RedefineDlg::setColumnTypeNumber(QMap<InvestProcessing::columnTypeE, int> &colTypeNum)
{
  m_colTypeNum = colTypeNum;
}

void RedefineDlg::setValidActionTypes(const QList<MyMoneyStatement::Transaction::EAction> &validActionTypes)
{
  m_validActionTypes = validActionTypes;
}

void RedefineDlg::setColumnList(const QStringList& list)
{
  m_columnList = list;
}
