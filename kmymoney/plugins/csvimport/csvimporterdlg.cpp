/****************************************************************************
*                             csvimporterdlg.cpp
*                             ---------------------
*  begin: Sat Jan 01 2010
*  copyright: (C) 2010 by Allan Anderson
*  email: agander93@gmail.com
****************************************************************************/

/***************************************************************************
*  *
*   This program is free software; you can redistribute it and/or modify   *
*   it under the terms of the GNU General Public License as published  *
*   by the Free Software Foundation; either version 2 of the License,  *
*   or  (at your option) any later version.*
*  *
****************************************************************************/

#include "csvimporterdlg.h"

// ----------------------------------------------------------------------------
// QT Headers

#include <QtGui/QScrollBar>
#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>
#include <QtGui/QCloseEvent>
#include <QtCore/QDebug>
#include <QtCore/QTextCodec>
#include <QtCore/QTextStream>
#include <QtCore/QPointer>
#include <QtCore/QFile>
#include <QTableWidgetItem>

// ----------------------------------------------------------------------------
// KDE Headers

#include <KAction>
#include <KSharedConfig>
#include <KMessageBox>
#include <KFileDialog>
#include <KStandardDirs>
#include <KLocale>
#include <KIO/NetAccess>
#include "KAboutApplicationDialog"
#include <KAboutData>

// ----------------------------------------------------------------------------
// Project Headers

#include "convdate.h"
#include "csvutil.h"
#include "investmentdlg.h"
#include "investprocessing.h"
#include "mymoneystatement.h"
#include "mymoneyaccount.h"

class CsvImporterPlugin;
class ConvertDate;
class InvestmentDlg;
class InvestProcessing;

CsvImporterDlg::CsvImporterDlg(QWidget* parent) :
    CsvImporterDlgDecl(parent)
{
  m_amountSelected = false;
  m_creditSelected = false;
  m_debitSelected = false;
  m_dateSelected = false;
  m_payeeSelected = false;
  m_memoSelected = false;
  m_numberSelected = false;
  m_duplicate = false;

  m_amountColumn = 0;
  m_creditColumn = 0;
  m_dateColumn = 0;
  m_debitColumn = 0;
  m_memoColumn = 0;
  m_numberColumn = 0;
  m_payeeColumn = 0;
  m_previousColumn = 0;
  m_maxColumnCount = 0;
  m_endLine = 0;
  m_startLine = 1;

  m_decimalSymbol.clear();
  m_previousType.clear();
  m_thousandsSeparator = ',';

  tableWidget->setWordWrap(false);
  comboBox_decimalSymbol->setCurrentIndex(-1);
  comboBox_thousandsDelimiter->setEnabled(false);

  m_setColor.setRgb(0, 255, 127, 100);
  m_errorColor.setRgb(255, 0, 127, 100);
  m_clearColor.setRgb(255, 255, 255, 255);
  m_colorBrush.setColor(m_setColor);
  m_clearBrush.setColor(m_clearColor);
  m_colorBrush.setStyle(Qt::SolidPattern);
  m_clearBrush.setStyle(Qt::SolidPattern);
  m_errorBrush.setColor(m_errorColor);
  m_errorBrush.setStyle(Qt::SolidPattern);

  for (int i = 0; i < MAXCOL; i++) { //  populate comboboxes with col # values
    QString t;
    t.setNum(i + 1);
    comboBoxBnk_numberCol->addItem(t) ;
    comboBoxBnk_dateCol->addItem(t) ;
    comboBoxBnk_payeeCol->addItem(t) ;
    comboBoxBnk_memoCol->addItem(t) ;
    comboBoxBnk_amountCol->addItem(t) ;
    comboBoxBnk_creditCol->addItem(t) ;
    comboBoxBnk_debitCol->addItem(t) ;

    m_columnType[i].clear();//          clear all column types
  }

  int screenWidth = QApplication::desktop()->width();
  int screenHeight = QApplication::desktop()->height();
  int x = (screenWidth - width()) / 2;
  int y = (screenHeight - height()) / 2;

  this->move(x, y);

  m_debitColumn = -1;
  m_creditColumn = -1;
  m_amountColumn = -1;

  m_convertDate = new ConvertDate;
  m_csvprocessing = new CsvProcessing;
  m_csvprocessing->m_csvDialog = this;
  m_investProcessing = new InvestProcessing;
  m_investProcessing->m_csvDialog = this;
  m_investmentDlg = new InvestmentDlg;
  m_investmentDlg->m_investProcessing = m_investProcessing;
  m_investmentDlg->m_csvDialog = this;
  m_investProcessing->m_convertDat = m_convertDate;
  m_investmentDlg->init();

  m_parse = new Parse;
  m_parse->m_csvDialog = this;
  m_csvprocessing->m_parse = m_parse;
  m_investProcessing->m_parse = m_parse;

  this->setAttribute(Qt::WA_DeleteOnClose, true);

  connect(radioBnk_amount, SIGNAL(clicked(bool)), this, SLOT(amountRadioClicked(bool)));
  connect(radioBnk_debCred, SIGNAL(clicked(bool)), this, SLOT(debitCreditRadioClicked(bool)));

  connect(button_clear, SIGNAL(clicked()), m_csvprocessing, SLOT(clearColumnsSelected()));
  connect(button_import, SIGNAL(clicked()), m_csvprocessing, SLOT(importClicked()));
  connect(button_import, SIGNAL(clicked()), m_investProcessing, SLOT(importClicked()));
  connect(button_quit, SIGNAL(clicked()), this, SLOT(slotClose()));
  connect(button_quit, SIGNAL(clicked()), m_investmentDlg, SLOT(slotClose()));
  connect(button_open, SIGNAL(clicked()), m_csvprocessing, SLOT(fileDialog()));
  connect(button_saveAs, SIGNAL(clicked()), m_csvprocessing, SLOT(saveAs()));
  connect(button_saveAs, SIGNAL(clicked()), m_investProcessing, SLOT(saveAs()));
  connect(buttonInv_hideSecurity, SIGNAL(clicked()), m_investProcessing, SLOT(hideSecurity()));

  connect(comboBoxBnk_amountCol, SIGNAL(currentIndexChanged(int)), this, SLOT(amountColumnSelected(int)));
  connect(comboBoxBnk_amountCol, SIGNAL(activated(int)), this, SLOT(amountColumnSelected(int)));
  connect(comboBoxBnk_debitCol, SIGNAL(currentIndexChanged(int)), this, SLOT(debitColumnSelected(int)));
  connect(comboBoxBnk_debitCol, SIGNAL(activated(int)), this, SLOT(debitColumnSelected(int)));
  connect(comboBoxBnk_creditCol, SIGNAL(currentIndexChanged(int)), this, SLOT(creditColumnSelected(int)));
  connect(comboBoxBnk_creditCol, SIGNAL(activated(int)), this, SLOT(creditColumnSelected(int)));
  connect(comboBox_decimalSymbol, SIGNAL(activated(int)), m_parse, SLOT(decimalSymbolSelected(int)));
  connect(comboBox_decimalSymbol, SIGNAL(activated(int)), this, SLOT(decimalSymbolSelected(int)));
  connect(comboBox_fieldDelimiter, SIGNAL(currentIndexChanged(int)), m_csvprocessing, SLOT(delimiterChanged()));
  connect(comboBox_fieldDelimiter, SIGNAL(currentIndexChanged(int)), m_investmentDlg->m_investProcessing, SLOT(fieldDelimiterChanged()));
  connect(comboBox_textDelimiter, SIGNAL(currentIndexChanged(int)), m_csvprocessing, SLOT(delimiterChanged()));
  connect(comboBoxBnk_memoCol, SIGNAL(currentIndexChanged(int)), this, SLOT(memoColumnSelected(int)));
  connect(comboBox_dateFormat, SIGNAL(currentIndexChanged(int)), m_csvprocessing, SLOT(dateFormatSelected(int)));
  connect(comboBox_dateFormat, SIGNAL(currentIndexChanged(int)), m_convertDate, SLOT(dateFormatSelected(int)));
  connect(comboBox_dateFormat, SIGNAL(currentIndexChanged(int)), m_investProcessing, SLOT(dateFormatSelected(int)));
  connect(comboBoxBnk_numberCol, SIGNAL(currentIndexChanged(int)), this, SLOT(numberColumnSelected(int)));
  connect(comboBoxBnk_dateCol, SIGNAL(currentIndexChanged(int)), this, SLOT(dateColumnSelected(int)));
  connect(comboBoxBnk_payeeCol, SIGNAL(currentIndexChanged(int)), this, SLOT(payeeColumnSelected(int)));
  connect(comboBoxInv_securityName, SIGNAL(activated(QString)), m_investProcessing, SLOT(securityNameSelected(QString)));

  connect(spinBox_skip, SIGNAL(valueChanged(int)), this, SLOT(startLineChanged(int)));
  connect(spinBox_skip, SIGNAL(valueChanged(int)), m_csvprocessing, SLOT(startLineChanged()));
  connect(spinBox_skipToLast, SIGNAL(valueChanged(int)), this, SLOT(endLineChanged(int)));
  connect(spinBox_skipToLast, SIGNAL(editingFinished()), m_csvprocessing, SLOT(endLineChanged()));
  connect(spinBox_skipToLast, SIGNAL(editingFinished()), m_investProcessing, SLOT(endLineChanged()));
  connect(m_csvprocessing, SIGNAL(statementReady(MyMoneyStatement&)), this, SIGNAL(statementReady(MyMoneyStatement&)));
  connect(m_investmentDlg, SIGNAL(statementReady(MyMoneyStatement&)), this, SIGNAL(statementReady(MyMoneyStatement&)));

  connect(tabWidget_Main, SIGNAL(currentChanged(int)), this, SLOT(tabSelected(int)));

  m_csvprocessing->init();
}

CsvImporterDlg::~CsvImporterDlg()
{
  delete m_parse;
  delete m_convertDate;
  delete m_investmentDlg;
  delete m_csvprocessing;
}

void CsvImporterDlg::amountRadioClicked(bool checked)
{
  if (checked) {
    comboBoxBnk_amountCol->setEnabled(true);//  disable credit & debit ui choices
    labelBnk_amount->setEnabled(true);
    comboBoxBnk_debitCol->setEnabled(false);
    comboBoxBnk_debitCol->setCurrentIndex(-1);
    labelBnk_debits->setEnabled(false);
    comboBoxBnk_creditCol->setEnabled(false);
    comboBoxBnk_creditCol->setCurrentIndex(-1);
    labelBnk_credits->setEnabled(false);

    //   the 'm_creditColumn/m_debitColumn' could just have been reassigned, so ensure
    //   ...they == "credit or debit" before clearing them
    if ((m_creditColumn >= 0) && (m_columnType[m_creditColumn] == "credit")) {
      m_columnType[m_creditColumn].clear();//       because amount col chosen...
    }
    if ((m_debitColumn >= 0) && (m_columnType[m_debitColumn] == "debit")) {
      m_columnType[m_debitColumn].clear();//        ...drop any credit & debit
    }
    m_debitColumn = -1;
    m_creditColumn = -1;
  }
}

void CsvImporterDlg::endLineChanged(int val)
{
  m_endLine = val;
}

void CsvImporterDlg::startLineChanged(int val)
{
  m_startLine = val;
}

int CsvImporterDlg::validateColumn(const int& col, const QString& type)
{
  //  First check if selection is in range
  if ((col < 0) || (col >= m_csvprocessing->endColumn())) {
    return KMessageBox::No;
  }//                                               selection was in range


  if ((!m_columnType[col].isEmpty())  && (m_columnType[col] != type)) {
    //                                              BUT column is already in use

    KMessageBox::information(0, i18n("The '<b>%1</b>' field already has this column selected. <center>Please reselect both entries as necessary.</center>"
                                     , m_columnType[col]));

    m_previousColumn = -1;
    resetComboBox(m_columnType[col], col);
    resetComboBox(type, col);//                    reset this combobox
    m_previousType.clear();
    m_columnType[col].clear();
    return KMessageBox::Cancel;
  }
  //                                               is this type already in use
  for (int i = 0; i < m_csvprocessing->endColumn(); i++) {   //  check each column
    if (m_columnType[i] == type) { //               this type already in use
      m_columnType[i].clear();//                   ...so clear it
    }//  end this col

  }// end all columns checked                      type not in use
  m_columnType[col] = type;//                      accept new type
  if (m_previousColumn != -1) {
    m_previousColumn = col;
  }
  m_previousType = type;
  return KMessageBox::Ok; //                       accept new type
}

void CsvImporterDlg::amountColumnSelected(int col)
{
  QString type = "amount";
  if (col < 0) { //                                 it is unset
    return;
  }
// if a previous amount field is detected, but in a different column...
  if ((m_amountColumn != -1) && (m_columnType[m_amountColumn] == type)  && (m_amountColumn != col)) {
    m_columnType[m_amountColumn].clear();
  }
  int ret = validateColumn(col, type);
  if (ret == KMessageBox::Ok) {
    comboBoxBnk_amountCol->setCurrentIndex(col);//    accept new column
    m_amountSelected = true;
    m_amountColumn = col;
    m_columnType[m_amountColumn] = type;
    restoreBackground();
    return;
  }
  if (ret == KMessageBox::No) {
    comboBoxBnk_amountCol->setCurrentIndex(-1);
  }
}

void CsvImporterDlg::debitCreditRadioClicked(bool checked)
{
  if (checked) {
    comboBoxBnk_debitCol->setEnabled(true);//         if 'debit/credit' selected
    labelBnk_debits->setEnabled(true);
    comboBoxBnk_creditCol->setEnabled(true);
    labelBnk_credits->setEnabled(true);
    comboBoxBnk_amountCol->setEnabled(false);//       disable 'amount' ui choices
    comboBoxBnk_amountCol->setCurrentIndex(-1);//     as credit/debit chosen

    //   the 'm_amountColumn' could just have been reassigned, so ensure
    //   ...m_columnType[m_amountColumn] == "amount" before clearing it
    if ((m_amountColumn >= 0) && (m_columnType[m_amountColumn] == "amount")) {
      m_columnType[m_amountColumn].clear();//          ...drop any amount choice
      m_amountColumn = -1;
    }
    labelBnk_amount->setEnabled(false);
  }
}

void CsvImporterDlg::creditColumnSelected(int col)
{
  QString type = "credit";
  if (col < 0) { //                                    it is unset
    return;
  }
// if a previous credit field is detected, but in a different column...
  if ((m_creditColumn != -1) && (m_columnType[m_creditColumn] == type)  && (m_creditColumn != col)) {
    m_columnType[m_creditColumn].clear();
  }
  int ret = validateColumn(col, type);

  if (ret == KMessageBox::Ok) {
    comboBoxBnk_creditCol->setCurrentIndex(col);//    accept new column
    m_creditSelected = true;
    m_creditColumn = col;
    m_columnType[m_creditColumn] = type;
    restoreBackground();
    return;
  }
  if (ret == KMessageBox::No) {
    comboBoxBnk_creditCol->setCurrentIndex(-1);
  }
}

void CsvImporterDlg::debitColumnSelected(int col)
{
  QString type = "debit";
  if (col < 0) { //                                    it is unset
    return;
  }
// A new column has been selected for this field so clear old one
  if ((m_debitColumn != -1) && (m_columnType[m_debitColumn] == type)  && (m_debitColumn != col)) {
    m_columnType[m_debitColumn].clear();
  }
  int ret = validateColumn(col, type);

  if (ret == KMessageBox::Ok) {
    comboBoxBnk_debitCol->setCurrentIndex(col);//     accept new column
    m_debitSelected = true;
    m_debitColumn = col;
    m_columnType[m_debitColumn] = type;
    return;
  }
  if (ret == KMessageBox::No) {
    comboBoxBnk_debitCol->setCurrentIndex(-1);
  }
}

void CsvImporterDlg::dateColumnSelected(int col)
{
  QString type = "date";
  if (col < 0) { //                                 it is unset
    return;
  }
// A new column has been selected for this field so clear old one
  if ((m_dateColumn != -1) && (m_columnType[m_dateColumn] == type)  && (m_dateColumn != col)) {
    m_columnType[m_dateColumn].clear();
  }
  int ret = validateColumn(col, type);

  if (ret == KMessageBox::Ok) {
    comboBoxBnk_dateCol->setCurrentIndex(col);//      accept new column
    m_dateSelected = true;
    m_dateColumn = col;
    m_columnType[m_dateColumn] = type;
    return;
  }
  if (ret == KMessageBox::No) {
    comboBoxBnk_dateCol->setCurrentIndex(-1);
  }
}

void CsvImporterDlg::memoColumnSelected(int col)
{
  QString type = "memo";
  if ((col < 0) || (col >= m_csvprocessing->endColumn())) { // out of range so...
    comboBoxBnk_memoCol->setCurrentIndex(-1);// ..clear selection
    return;
  }
  if (m_columnType[col].isEmpty()) { //             accept new  entry
    comboBoxBnk_memoCol->setItemText(col, QString().setNum(col + 1) + '*');
    m_columnType[col] = type;
    m_memoColumn = col;
    m_memoSelected = true;
    return;
  } else {//                                       clashes with prior selection
    if (m_columnType[col] == type) { //               nothing changed
      return;
    }
    m_memoSelected = false;//                      clear incorrect selection
    KMessageBox::information(0, i18n("The '<b>%1</b>' field already has this column selected. <center>Please reselect both entries as necessary.</center>"
                                     , m_columnType[col]));
    comboBoxBnk_memoCol->setCurrentIndex(-1);
    m_previousColumn = -1;
    resetComboBox(m_columnType[col], col);//       clash,  so reset ..
    resetComboBox(type, col);//                    ... both comboboxes
    m_previousType.clear();
    m_columnType[col].clear();
    comboBoxBnk_memoCol->setItemText(col, QString().setNum(col + 1));//  reset the '*'
  }
}

void CsvImporterDlg::payeeColumnSelected(int col)
{
  QString type = "payee";
  if (col < 0) { //                              it is unset
    return;
  }
// if a previous payee field is detected, but in a different column...
  if ((m_payeeColumn != -1) && (m_columnType[m_payeeColumn] == type)  && (m_payeeColumn != col)) {
    m_columnType[m_payeeColumn].clear();
  }
  int ret = validateColumn(col, type);
  if (ret == KMessageBox::Ok) {
    comboBoxBnk_payeeCol->setCurrentIndex(col);// accept new column
    m_payeeSelected = true;
    m_payeeColumn = col;
    m_columnType[m_payeeColumn] = type;
    return;
  }
  if (ret == KMessageBox::No) {
    comboBoxBnk_payeeCol->setCurrentIndex(-1);
  }
}

void CsvImporterDlg::numberColumnSelected(int col)
{
  QString type = "number";
  if (col < 0) { //                              it is unset
    return;
  }
// if a previous number field is detected, but in a different column...
  if ((m_numberColumn != -1) && (m_columnType[m_numberColumn] == type)  && (m_numberColumn != col)) {
    m_columnType[m_numberColumn].clear();
  }
  int ret = validateColumn(col, type);

  if (ret == KMessageBox::Ok) {
    comboBoxBnk_numberCol->setCurrentIndex(col);// accept new column
    m_numberSelected = true;
    m_numberColumn = col;
    m_columnType[m_numberColumn] = type;
    return;
  }
  if (ret == KMessageBox::No) {
    comboBoxBnk_numberCol->setCurrentIndex(-1);
  }
}

void CsvImporterDlg::slotClose()
{
  saveSettings();
  m_plugin->m_action->setEnabled(true);
  CsvImporterDlg::close();
}

void CsvImporterDlg::saveSettings()
{
  if (!m_csvprocessing->inFileName().isEmpty()) { //  don't save column numbers if no file loaded
    KSharedConfigPtr config = KSharedConfig::openConfig(KStandardDirs::locateLocal("config", "csvimporterrc"));

    KConfigGroup mainGroup(config, "MainWindow");
    mainGroup.writeEntry("Height", height());
    mainGroup.config()->sync();

    KConfigGroup profileGroup(config, "Profile");
    profileGroup.writeEntry("CurrentUI", m_currentUI);
    QString str = "$HOME/" + m_csvprocessing->csvPath().section('/', 3);
    profileGroup.writeEntry("CsvDirectory", str);
    profileGroup.writeEntry("DateFormat", comboBox_dateFormat->currentIndex());
    profileGroup.writeEntry("FieldDelimiter", m_csvprocessing->fieldDelimiterIndex());
    profileGroup.writeEntry("TextDelimiter", m_csvprocessing->textDelimiterIndex());
    profileGroup.writeEntry("StartLine", spinBox_skip->value() - 1);
    profileGroup.config()->sync();

    KConfigGroup columnsGroup(config, "Columns");
    columnsGroup.writeEntry("DateCol", comboBoxBnk_dateCol->currentIndex());
    columnsGroup.writeEntry("PayeeCol", comboBoxBnk_payeeCol->currentIndex());
    columnsGroup.writeEntry("NumberCol", comboBoxBnk_numberCol->currentIndex());
    columnsGroup.writeEntry("AmountCol", comboBoxBnk_amountCol->currentIndex());
    columnsGroup.writeEntry("DebitCol", comboBoxBnk_debitCol->currentIndex());
    columnsGroup.writeEntry("CreditCol", comboBoxBnk_creditCol->currentIndex());
    columnsGroup.config()->sync();

    m_csvprocessing->inFileName().clear();
  }
  tableWidget->clear();//     in case later reopening window, clear old contents now
}

void CsvImporterDlg::closeEvent(QCloseEvent *event)
{
  slotClose();
  event->accept();
}

void CsvImporterDlg::resizeEvent(QResizeEvent * event)
{
  event->accept();
}

int CsvImporterDlg::maxColumnCount()
{
  return m_maxColumnCount;
}

void CsvImporterDlg::setMaxColumnCount(int val)
{
  m_maxColumnCount = val;
}

int CsvImporterDlg::debitColumn()
{
  return  m_debitColumn;
}

void CsvImporterDlg::setDebitColumn(int column)
{
  m_debitColumn = column;
}

int CsvImporterDlg::creditColumn()
{
  return  m_creditColumn;
}

void CsvImporterDlg::setCreditColumn(int column)
{
  m_creditColumn = column;
}

int CsvImporterDlg::amountColumn()
{
  return m_amountColumn;
}

void CsvImporterDlg::setAmountColumn(int column)
{
  m_amountColumn = column;
}

void CsvImporterDlg::setNumberSelected(bool val)
{
  m_numberSelected = val;
}

void CsvImporterDlg::setMemoSelected(bool val)
{
  m_memoSelected = val;
}

bool CsvImporterDlg::payeeSelected()
{
  return m_payeeSelected;
}

void CsvImporterDlg::setPayeeSelected(bool val)
{
  m_payeeSelected = val;
}

bool CsvImporterDlg::dateSelected()
{
  return m_dateSelected;
}

void CsvImporterDlg::setDateSelected(bool val)
{
  m_dateSelected = val;
}

bool CsvImporterDlg::debitSelected()
{
  return m_debitSelected;
}

void CsvImporterDlg::setDebitSelected(bool val)
{
  m_debitSelected = val;
}

bool CsvImporterDlg::creditSelected()
{
  return m_creditSelected;
}

void CsvImporterDlg::setCreditSelected(bool val)
{
  m_creditSelected = val;
}

bool CsvImporterDlg::amountSelected()
{
  return m_amountSelected;
}

void CsvImporterDlg::setAmountSelected(bool val)
{
  m_amountSelected = val;
}

QString CsvImporterDlg::columnType(int column)
{
  return  m_columnType[column];
}

void CsvImporterDlg::clearColumnType(int column)
{
  m_columnType[column].clear();
}

void CsvImporterDlg::clearPreviousColumn()
{
  m_previousType.clear();
}

void CsvImporterDlg::resetComboBox(const QString& comboBox, const int& col)
{
  QStringList fieldType;
  fieldType << "amount" << "credit" << "date" << "debit" << "memo" << "number" << "payee";
  int index = fieldType.indexOf(comboBox);
  switch (index) {
    case 0://  amount
      comboBoxBnk_amountCol->setCurrentIndex(-1);
      m_amountSelected = false;
      break;
    case 1://  credit
      comboBoxBnk_creditCol->setCurrentIndex(-1);
      m_creditSelected = false;
      break;
    case 2://  date
      comboBoxBnk_dateCol->setCurrentIndex(-1);
      m_dateSelected = false;
      break;
    case 3://  debit
      comboBoxBnk_debitCol->setCurrentIndex(-1);
      m_debitSelected = false;
      break;
    case 4://  memo
      comboBoxBnk_memoCol->setCurrentIndex(-1);
      comboBoxBnk_memoCol->setItemText(col, QString().setNum(col + 1));//  reset the '*'
      m_memoSelected = false;
      break;
    case 5://  number
      comboBoxBnk_numberCol->setCurrentIndex(-1);
      m_numberSelected = false;
      break;
    case 6://  payee
      comboBoxBnk_payeeCol->setCurrentIndex(-1);
      m_payeeSelected = false;
      break;
    default:
      qDebug() << i18n("ERROR. Field name not recognised.") << comboBox;
      KMessageBox::sorry(this, i18n("<center>Field name not recognised.</center> <center>'<b>%1</b>'</center> Please re-enter your column selections."
                                    , comboBox), i18n("CSV import"));
  }
  m_columnType[col].clear();
  return;
}

void CsvImporterDlg::tabSelected(int index)
{
  if (index == 2) {
    return; //                      Settings
  }

  switch (index) {
    case 0 ://  "Banking" selected
      if ((!m_investProcessing->inFileName().isEmpty()) && (m_currentUI == "Invest")) {
        int ret = KMessageBox::warningContinueCancel(this, i18n("<center>Are you sure you want to switch from '%1'?</center>"
                  "<center>You will lose your current settings.</center><center>Continue or Cancel?</center>",
                  m_currentUI), i18n("Changing Tab"), KStandardGuiItem::cont(),
                  KStandardGuiItem::cancel());
        if (ret == KMessageBox::Cancel) {
          return;
        }
      }
      if ((m_csvprocessing->inFileName().isEmpty()) || (m_currentUI == "Invest")) {
        m_investmentDlg->saveSettings();//            leaving "Invest" so save settings
        m_csvprocessing->readSettings();//            ...and load "Banking"
        tableWidget->reset();
        tabWidget_Main->setTabText(0, i18n("Banking") + '*');
        tabWidget_Main->setTabText(1, i18n("Investment"));
      }
      m_fileType = "Banking";
      m_currentUI = "Banking";
      break;
    case 1 ://  "Invest" selected
      if ((!m_csvprocessing->inFileName().isEmpty())  && (m_currentUI == "Banking")) {
        int ret = KMessageBox::warningContinueCancel(this, i18n("<center>Are you sure you want to switch from '%1'?</center>"
                  "<center>You will lose your current settings.</center><center>Continue or Cancel?</center>",
                  m_currentUI), i18n("Changing Tab"), KStandardGuiItem::cont(),
                  KStandardGuiItem::cancel());
        if (ret == KMessageBox::Cancel) {
          return;
        }
      }
      if ((m_investProcessing->inFileName().isEmpty()) || (m_currentUI == "Banking")) {
        saveSettings();//                             leaving "Banking" so save settings
        m_investProcessing->readSettings();//         ...and load "Invest"
        tableWidget->reset();
        tabWidget_Main->setTabText(0, i18n("Banking"));
        tabWidget_Main->setTabText(1, i18n("Investment") + '*');
      }
      m_fileType = "Invest";
      m_currentUI = "Invest";
      break;
  }
}

void CsvImporterDlg::updateDecimalSymbol(const QString& type, int col)
{
  QString txt;
  bool symbolFound = false;
  bool invalidResult = false;

  //  Clear background

  for (int row = 0; row < m_endLine; row++) {
    if (tableWidget->item(row, col) != 0) {
      tableWidget->item(row, col)->setBackground(m_clearBrush);
    }
  }

  if (type == "amount" || type == "credit" || type == "debit" || type == "price" || type == "quantity") {

    //  Set first and last rows

    int first = m_startLine;
    int last = m_endLine;
    m_parse->setSymbolFound(false);

    QString newTxt;
    int errorRow = 0;
    QTableWidgetItem* errorItem(0);
    //  Check if this col contains empty cells
    int row = 0;
    for (row = first - 1; row < last; row++) {
      if (tableWidget->item(row, col) == 0) { //       empty cell
        if (((m_fileType == "Banking") && (m_csvprocessing->importNow())) ||
            ((m_fileType == "Invest") && (m_investProcessing->importNow()))) {
          //                                     if importing, this is error
          KMessageBox::sorry(this, (i18n("Row number %1 may be a header line, as it has an incomplete set of entries."
                                         "<center>It may be that the start line is incorrectly set.</center>",
                                         row + 1), i18n("CSV import")));
          return;
        }
        //                                       if not importing, query
        int ret = KMessageBox::warningContinueCancel(this, i18n("<center>The cell in column '%1' on row %2 is empty.</center>"
                  "<center>Please check your selections.</center><center>Continue or Cancel?</center>",
                  col + 1 , row + 1), i18n("Selections Warning"), KStandardGuiItem::cont(),
                  KStandardGuiItem::cancel());
        if (ret == KMessageBox::Continue) {
          continue;
        }
        return;//                                     empty cell
      } else {

        //  Check if this col contains decimal symbol

        txt = tableWidget->item(row, col)->text();//  get data

        newTxt = m_parse->possiblyReplaceSymbol(txt);//  update data
        tableWidget->item(row, col)->setText(newTxt);//  highlight selection
        tableWidget->item(row, col)->setBackground(m_colorBrush);
        if (m_parse->invalidConversion()) {
          invalidResult = true;
          errorItem = tableWidget->item(row, col);
          errorItem->setBackground(m_errorBrush);
          tableWidget->scrollToItem(errorItem, QAbstractItemView::EnsureVisible);
          if (errorRow == 0) {
            errorRow = row;
          }
        }
        if (m_parse->symbolFound()) {
          symbolFound = true;
        }
        if (newTxt == txt) { //                        no matching symbol found
          continue;
        }
      }
      if (!symbolFound) {
        errorItem = tableWidget->item(row, col);
        errorItem->setBackground(m_errorBrush);
      }
    }//  last row

    if (!symbolFound) {//                            no symbol found
      tableWidget->scrollToItem(errorItem, QAbstractItemView::EnsureVisible);
      KMessageBox::sorry(this, i18n("<center>The selected decimal symbol was not present in column %1,</center>"
                                    "<center>- but may now have been added.</center>"
                                    "<center>If the <b>decimal</b> symbol displayed does not match your system setting</center>"
                                    "<center>your data is unlikely to import correctly.</center>"
                                    "<center>Please check your selection.</center>",
                                    col + 1), i18n("CSV import"));
      return;
    }

    if (invalidResult) {
      KMessageBox::sorry(0, i18n("<center>The selected decimal symbol/thousands separator</center>"
                                 "<center>have produced invalid results in row %1, and possibly more.</center>"
                                 "<center>Please try again.</center>", errorRow + 1), i18n("Invalid Conversion"));
      if (m_fileType == "Banking") {
        m_csvprocessing->readFile("", 0);
      } else {
        m_investProcessing->readFile("", 0);
      }
    }
  }
}

void CsvImporterDlg::decimalSymbolSelected(int index)
{
  restoreBackground();//                              remove selection highlighting

  if (index < 0) {
    return;
  }

  if (m_startLine > m_endLine) {
    KMessageBox::sorry(0, i18n("<center>The start line is greater than the end line.\n</center>"
                               "<center>Please correct your settings.</center>"), i18n("CSV import"));
    return;
  }

  if (m_decimalSymbolChanged) {
    if (m_fileType == "Banking") {
      m_csvprocessing->readFile("", 0);
    } else {
      m_investProcessing->readFile("", 0);
    }
  }

  //  Save new decimal symbol and thousands separator

  m_decimalSymbolIndex = index;
  m_parse->setDecimalSymbolIndex(index);
  m_decimalSymbol = m_parse->decimalSymbol(index);
  comboBox_thousandsDelimiter->setCurrentIndex(index);
  thousandsSeparatorChanged();

  //  Update the UI

  if (m_fileType == "Banking") {
    if ((!m_csvprocessing->inFileName().isEmpty()) && ((m_amountColumn >= 0) || ((m_debitColumn >= 0) && (m_creditColumn >= 0)))) {
      if (m_amountColumn >= 0) {
        updateDecimalSymbol("amount", m_amountColumn);
      } else {
        updateDecimalSymbol("debit", m_debitColumn);
        updateDecimalSymbol("credit", m_creditColumn);
      }
      m_decimalSymbolChanged = true;
    }
  } else {
    if (m_fileType == "Invest") {
      if (!m_investProcessing->inFileName().isEmpty()) {
        updateDecimalSymbol("amount", m_investProcessing->amountColumn());
        updateDecimalSymbol("price", m_investProcessing->priceColumn());
        updateDecimalSymbol("quantity", m_investProcessing->quantityColumn());
      }
    }
    m_decimalSymbolChanged = true;
  }
}

QString CsvImporterDlg::decimalSymbol()
{
  return m_decimalSymbol;
}

void CsvImporterDlg::thousandsSeparatorChanged()
{
  m_thousandsSeparator = m_parse->thousandsSeparator();
}

void CsvImporterDlg::restoreBackground()
{
  for (int row = 0; row < m_csvprocessing->lastLine(); row++)
    for (int col = 0; col < m_csvprocessing->endColumn(); col++)
      if (tableWidget->item(row, col) != 0) {
        tableWidget->item(row, col)->setBackground(m_clearBrush);
      }
}

QString CsvImporterDlg::currentUI()
{
  return m_currentUI;
}

void CsvImporterDlg::setCurrentUI(QString val)
{
  m_currentUI = val;
}
