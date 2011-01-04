/****************************************************************************
*                             csvimporterdialog.cpp
*                             ---------------------
*  begin: Sat Jan 01 2010
*  copyright: (C) 2010 by Allan Anderson
*  email: aganderson@ukonline.co.uk
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
#include "investmentdlg.h"
#include "mymoneystatement.h"
#include "mymoneyaccount.h"

class CsvImporterPlugin;
class ConvertDate;
class InvestmentDlg;

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
  m_tableFrameHeight = 0;
  m_tableFrameWidth = 0;
  m_maxColumnCount = 0;

  m_previousType.clear();

  m_tableFrameHeight =  groupBox->frameGeometry().bottom() - frame_main->frameGeometry().bottom();
  m_tableFrameWidth = tableWidget->size().width();

  for (int i = 0; i < MAXCOL; i++) {//  populate comboboxes with col # values
    QString t;
    t.setNum(i + 1);
    comboBox_numberCol->addItem(t) ;
    comboBox_dateCol->addItem(t) ;
    comboBox_payeeCol->addItem(t) ;
    comboBox_memoCol->addItem(t) ;
    comboBox_amountCol->addItem(t) ;
    comboBox_creditCol->addItem(t) ;
    comboBox_debitCol->addItem(t) ;
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
  m_investmentDlg = new InvestmentDlg;
  m_investmentDlg->m_csvImportDlg = this;
  m_csvprocessing = new CsvProcessing;
  m_csvprocessing->m_csvDialog = this;

  this->setAttribute(Qt::WA_DeleteOnClose, true);

  connect(checkBox_qif, SIGNAL(clicked(bool)), m_csvprocessing, SLOT(importClicked(bool)));
  connect(radio_amount, SIGNAL(clicked(bool)), this, SLOT(amountRadioClicked(bool)));
  connect(radio_debCred, SIGNAL(clicked(bool)), this, SLOT(debitCreditRadioClicked(bool)));
  connect(comboBox_amountCol, SIGNAL(currentIndexChanged(int)), this, SLOT(amountColumnSelected(int)));
  connect(comboBox_debitCol, SIGNAL(currentIndexChanged(int)), this, SLOT(debitColumnSelected(int)));
  connect(comboBox_creditCol, SIGNAL(currentIndexChanged(int)), this, SLOT(creditColumnSelected(int)));
  connect(button_clear, SIGNAL(clicked()), m_csvprocessing, SLOT(clearColumnsSelected()));
  connect(button_close, SIGNAL(clicked()), this, SLOT(slotClose()));
  connect(button_invest, SIGNAL(clicked()), this, SLOT(investmentSelected()));
  connect(button_open, SIGNAL(clicked()), m_csvprocessing, SLOT(fileDialog()));
  connect(button_saveAs, SIGNAL(clicked()), m_csvprocessing, SLOT(saveAs()));
  connect(pushButton, SIGNAL(clicked()), m_investmentDlg, SLOT(helpSelected()));
  connect(comboBox_encoding, SIGNAL(currentIndexChanged(int)), m_csvprocessing, SLOT(encodingChanged()));
  connect(comboBox_fieldDelim, SIGNAL(currentIndexChanged(int)), m_csvprocessing, SLOT(delimiterChanged()));
  connect(comboBox_textDelimiter, SIGNAL(currentIndexChanged(int)), m_csvprocessing, SLOT(delimiterChanged()));
  connect(comboBox_memoCol, SIGNAL(currentIndexChanged(int)), this, SLOT(memoColumnSelected(int)));
  connect(comboBox_dateFormat, SIGNAL(currentIndexChanged(int)), m_csvprocessing, SLOT(dateFormatSelected(int)));
  connect(comboBox_dateFormat, SIGNAL(currentIndexChanged(int)), m_convertDate, SLOT(dateFormatSelected(int)));
  connect(comboBox_numberCol, SIGNAL(currentIndexChanged(int)), this, SLOT(numberColumnSelected(int)));
  connect(comboBox_dateCol, SIGNAL(currentIndexChanged(int)), this, SLOT(dateColumnSelected(int)));
  connect(comboBox_payeeCol, SIGNAL(currentIndexChanged(int)), this, SLOT(payeeColumnSelected(int)));
  connect(spinBox_skip, SIGNAL(valueChanged(int)), m_csvprocessing, SLOT(startLineChanged()));
  connect(spinBox_skipLast, SIGNAL(editingFinished()), m_csvprocessing, SLOT(endLineChanged()));
  connect(m_csvprocessing, SIGNAL(statementReady(MyMoneyStatement&)), this, SIGNAL(statementReady(MyMoneyStatement&)));
  connect(m_investmentDlg, SIGNAL(statementReady(MyMoneyStatement&)), this, SIGNAL(statementReady(MyMoneyStatement&)));

  m_csvprocessing->init();
}

CsvImporterDlg::~CsvImporterDlg()
{
  delete m_convertDate;
  delete m_investmentDlg;
  delete m_csvprocessing;
}

void CsvImporterDlg::amountRadioClicked(bool checked)
{
  if (checked) {
    comboBox_amountCol->setEnabled(true);//  disable credit & debit ui choices
    label_amount->setEnabled(true);
    comboBox_debitCol->setEnabled(false);
    comboBox_debitCol->setCurrentIndex(-1);
    label_debits->setEnabled(false);
    comboBox_creditCol->setEnabled(false);
    comboBox_creditCol->setCurrentIndex(-1);
    label_credits->setEnabled(false);

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


int CsvImporterDlg::validateColumn(const int& col, const QString& type)
{
  //  First check if selection is in range
  if ((col < 0) || (col >= m_csvprocessing->endColumn())) {
    return KMessageBox::No;
  }
  if ((col == m_previousColumn) && (type == m_previousType)) {
    return -1;
  }
  //                                               selection was in range

  if ((!m_columnType[col].isEmpty())  && (m_columnType[col] != type)) {
    //                         BUT column is already in use

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
    comboBox_amountCol->setCurrentIndex(col);//    accept new column
    m_amountSelected = true;
    m_amountColumn = col;
    m_columnType[m_amountColumn] = type;
    return;
  }
  if (ret == KMessageBox::No) {

    comboBox_amountCol->setCurrentIndex(-1);
  }
}

void CsvImporterDlg::debitCreditRadioClicked(bool checked)
{
  if (checked) {
    comboBox_debitCol->setEnabled(true);//         if 'debit/credit' selected
    label_debits->setEnabled(true);
    comboBox_creditCol->setEnabled(true);
    label_credits->setEnabled(true);
    comboBox_amountCol->setEnabled(false);//       disable 'amount' ui choices
    comboBox_amountCol->setCurrentIndex(-1);//     as credit/debit chosen

    //   the 'm_amountColumn' could just have been reassigned, so ensure
    //   ...m_columnType[m_amountColumn] == "amount" before clearing it
    if ((m_amountColumn >= 0) && (m_columnType[m_amountColumn] == "amount")) {
      m_columnType[m_amountColumn].clear();//      ...drop any amount choice
    }
    label_amount->setEnabled(false);
  }
}

void CsvImporterDlg::creditColumnSelected(int col)
{
  QString type = "credit";
  if (col < 0) { //                              it is unset
    return;
  }
// if a previous credit field is detected, but in a different column...
  if ((m_creditColumn != -1) && (m_columnType[m_creditColumn] == type)  && (m_creditColumn != col)) {
    m_columnType[m_creditColumn].clear();
  }
  int ret = validateColumn(col, type);

  if (ret == KMessageBox::Ok) {
    comboBox_creditCol->setCurrentIndex(col);// accept new column
    m_creditSelected = true;
    m_creditColumn = col;
    m_columnType[m_creditColumn] = type;
    return;
  }
  if (ret == KMessageBox::No) {
    comboBox_creditCol->setCurrentIndex(-1);
  }
}


void CsvImporterDlg::debitColumnSelected(int col)
{
  QString type = "debit";
  if (col < 0) { //                                 it is unset
    return;
  }
// A new column has been selected for this field so clear old one
  if ((m_debitColumn != -1) && (m_columnType[m_debitColumn] == type)  && (m_debitColumn != col)) {
    m_columnType[m_debitColumn].clear();
  }
  int ret = validateColumn(col, type);

  if (ret == KMessageBox::Ok) {
    comboBox_debitCol->setCurrentIndex(col);//     accept new column
    m_debitSelected = true;
    m_debitColumn = col;
    m_columnType[m_debitColumn] = type;
    return;
  }
  if (ret == KMessageBox::No) {
    comboBox_debitCol->setCurrentIndex(-1);
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
    comboBox_dateCol->setCurrentIndex(col);//      accept new column
    m_dateSelected = true;
    m_dateColumn = col;
    m_columnType[m_dateColumn] = type;
    return;
  }
  if (ret == KMessageBox::No) {
    comboBox_dateCol->setCurrentIndex(-1);
  }
}

void CsvImporterDlg::memoColumnSelected(int col)
{
  QString type = "memo";
  if ((col < 0) || (col >= m_csvprocessing->endColumn())) { // out of range so...
    comboBox_memoCol->setCurrentIndex(-1);// ..clear selection
    return;
  }
  if (m_columnType[col].isEmpty()) { //             accept new  entry
    comboBox_memoCol->setItemText(col, QString().setNum(col + 1) + '*');
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
    comboBox_memoCol->setCurrentIndex(-1);
    m_previousColumn = -1;
    resetComboBox(m_columnType[col], col);//       clash,  so reset ..
    resetComboBox(type, col);//                    ... both comboboxes
    m_previousType.clear();
    m_columnType[col].clear();
    comboBox_memoCol->setItemText(col, QString().setNum(col + 1));//  reset the '*'
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
    comboBox_payeeCol->setCurrentIndex(col);// accept new column
    m_payeeSelected = true;
    m_payeeColumn = col;
    m_columnType[m_payeeColumn] = type;
    return;
  }
  if (ret == KMessageBox::No) {
    comboBox_payeeCol->setCurrentIndex(-1);
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
    comboBox_numberCol->setCurrentIndex(col);// accept new column
    m_numberSelected = true;
    m_numberColumn = col;
    m_columnType[m_numberColumn] = type;
    return;
  }
  if (ret == KMessageBox::No) {
    comboBox_numberCol->setCurrentIndex(-1);
  }
}


void CsvImporterDlg::investmentSelected()
{
  m_investmentDlg->show();
  this->hide();
}

void CsvImporterDlg::slotClose()
{
  KSharedConfigPtr config = KSharedConfig::openConfig(KStandardDirs::locateLocal("config", "csvimporterrc"));

  KConfigGroup mainGroup(config, "MainWindow");
  mainGroup.writeEntry("Height", height());
  mainGroup.config()->sync();

  KConfigGroup profileGroup(config, "Profile");
  profileGroup.writeEntry("DateFormat", comboBox_dateFormat->currentIndex());
  profileGroup.writeEntry("Encoding", comboBox_encoding->currentIndex());
  profileGroup.writeEntry("FieldDelimiter", m_csvprocessing->fieldDelimiterIndex());
  profileGroup.writeEntry("StartLine", spinBox_skip->value() - 1);

  QString str = "$HOME/" + m_csvprocessing->csvPath().section('/', 3);
  profileGroup.writeEntry("CsvDirectory", str);

  profileGroup.config()->sync();

  if (!m_csvprocessing->inFileName().isEmpty()) {  //  don't save column numbers if no file loaded
    KConfigGroup columnsGroup(config, "Columns");
    columnsGroup.writeEntry("DateCol", comboBox_dateCol->currentIndex());
    columnsGroup.writeEntry("PayeeCol", comboBox_payeeCol->currentIndex());
    columnsGroup.writeEntry("MemoCol", comboBox_memoCol->currentIndex());
    columnsGroup.writeEntry("NumberCol", comboBox_numberCol->currentIndex());
    columnsGroup.writeEntry("AmountCol", comboBox_amountCol->currentIndex());
    columnsGroup.writeEntry("DebitCol", comboBox_debitCol->currentIndex());
    columnsGroup.writeEntry("CreditCol", comboBox_creditCol->currentIndex());
    columnsGroup.config()->sync();
  }
  m_plugin->m_action->setEnabled(true);
  CsvImporterDlg::close();
}

void CsvImporterDlg::closeEvent(QCloseEvent *event)
{
  slotClose();
  event->accept();
}

void CsvImporterDlg::resizeEvent(QResizeEvent * event)
{
  event->accept();
  if (!m_csvprocessing->inFileName().isEmpty())
    m_csvprocessing->updateScreen();
}

int CsvImporterDlg::maxColumnCount()
{
  return m_maxColumnCount;
}

void CsvImporterDlg::setMaxColumnCount(int val)
{
  m_maxColumnCount = val;
}

int CsvImporterDlg::tableFrameWidth()
{
  return m_tableFrameWidth;
}

void CsvImporterDlg::setTableFrameWidth(int val)
{
  m_tableFrameWidth = val;
}

int CsvImporterDlg::tableFrameHeight()
{
  return m_tableFrameHeight;
}

void CsvImporterDlg::setTableFrameHeight(int val)
{
  m_tableFrameHeight = val;
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
  switch(index) {
    case 0://  amount
      comboBox_amountCol->setCurrentIndex(-1);
      m_amountSelected = false;
      break;
    case 1://  credit
      comboBox_creditCol->setCurrentIndex(-1);
      m_creditSelected = false;
      break;
    case 2://  date
      comboBox_dateCol->setCurrentIndex(-1);
      m_dateSelected = false;
      break;
    case 3://  debit
      comboBox_debitCol->setCurrentIndex(-1);
      m_debitSelected = false;
      break;
    case 4://  memo
      comboBox_memoCol->setCurrentIndex(-1);
      comboBox_memoCol->setItemText(col, QString().setNum(col + 1));//  reset the '*'
      m_memoSelected = false;
      break;
    case 5://  number
      comboBox_numberCol->setCurrentIndex(-1);
      m_numberSelected = false;
      break;
    case 6://  payee
      comboBox_payeeCol->setCurrentIndex(-1);
      m_payeeSelected = false;
      break;
    default:
      qDebug() << i18n("588 ERROR. Field name not recognised.") << comboBox;
      KMessageBox::sorry(this, i18n("<center>Field name not recognised.</center> <center>'<b>%1</b>'</center> Please re-enter your column selections."
                                   , comboBox), i18n("CSV import"));
  }
  m_columnType[col].clear();
  return;
}
