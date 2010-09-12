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

  m_amountColumn = -1;
  m_debitColumn = -1;
  m_creditColumn = -1;

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
  connect(comboBox_fieldDelim, SIGNAL(currentIndexChanged(int)), m_csvprocessing, SLOT(fieldDelimiterChanged()));
  connect(comboBox_memoCol, SIGNAL(currentIndexChanged(int)), this, SLOT(memoColumnSelected(int)));
  connect(comboBox_dateFormat, SIGNAL(currentIndexChanged(int)), m_csvprocessing, SLOT(dateFormatSelected(int)));
  connect(comboBox_dateFormat, SIGNAL(currentIndexChanged(int)), m_convertDate, SLOT(dateFormatSelected(int)));
  connect(comboBox_numberCol, SIGNAL(currentIndexChanged(int)), this, SLOT(numberColumnChanged(int)));
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
    radio_debCred->setEnabled(false);
    comboBox_debitCol->setEnabled(false);
    label_debits->setEnabled(false);
    comboBox_creditCol->setEnabled(false);
    label_credits->setEnabled(false);
    m_debitColumn = -1;
    m_creditColumn = -1;
  }
}

void CsvImporterDlg::amountColumnSelected(int val)
{
  if (radio_amount->isChecked()) {
    m_amountColumn = val;
    if ((m_amountColumn < 0) || (m_amountColumn >= m_csvprocessing->m_lastColumn)) {//invalid col.
      comboBox_amountCol->setCurrentIndex(-1);
      return;
    }
    // if this column not already chosen & amount not already chosen
    if ((m_columnType[m_amountColumn].isEmpty()) && (m_amountSelected == false)) {
      m_columnType[m_amountColumn] = "amount";// set this col. to "amount"
      m_amountSelected = true;

    } else {
      m_amountSelected = false;
      comboBox_amountCol->setCurrentIndex(-1);
      KMessageBox::information(this,
                               i18n("That column, or an 'amount', is already selected!\
                             <center>Please select a different column.</center>"));
    }
  }
}

void CsvImporterDlg::debitCreditRadioClicked(bool checked)
{
  if (checked) {//
    comboBox_debitCol->setEnabled(true);// if 'debit/credit' selected
    label_debits->setEnabled(true);
    comboBox_creditCol->setEnabled(true);
    label_credits->setEnabled(true);
    radio_amount->setEnabled(false);//  disable 'amount' ui choices
    comboBox_amountCol->setEnabled(false);
    label_amount->setEnabled(false);
    m_amountColumn = -1;
  }
}

void CsvImporterDlg::creditColumnSelected(int val)
{
  m_creditColumn = val;
  if ((m_creditColumn < 0) || (m_creditColumn >= m_csvprocessing->m_lastColumn)) {
    comboBox_creditCol->setCurrentIndex(-1);
    return;
  }
  if ((m_columnType[m_creditColumn].isEmpty()) && (m_creditSelected == false)) {
    m_columnType[m_creditColumn] = "credit";
    m_creditSelected = true;
  } else {
    m_creditSelected = false;
    comboBox_creditCol->setCurrentIndex(-1);
    KMessageBox::information(this, i18n("That column, or the credit field, is selected already!\
    <center>Please select a different column or field.</center>"));
  }
}

void CsvImporterDlg::debitColumnSelected(int val)
{
  m_debitColumn = val;
  if ((m_debitColumn < 0) || (m_debitColumn >= m_csvprocessing->m_lastColumn)) {
    comboBox_debitCol->setCurrentIndex(-1);
    return;
  }
  if ((m_columnType[m_debitColumn].isEmpty()) && (m_debitSelected == false)) {
    m_columnType[m_debitColumn] = "debit";
    m_debitSelected = true;
  } else {
    m_debitSelected = false;
    comboBox_debitCol->setCurrentIndex(-1);
    KMessageBox::information(this, i18n("That column, or the debit field, is selected already!\
    <center>Please select a different column or field.</center>"));
  }
}

void CsvImporterDlg::dateColumnSelected(int col)
{
  m_dateColumn = col;
  if ((m_dateColumn < 0) || (m_dateColumn >= m_csvprocessing->m_lastColumn)) {
    comboBox_dateCol->setCurrentIndex(-1);
    return;
  }

  if ((m_columnType[m_dateColumn].isEmpty()) && (m_dateSelected == false)) {
    m_columnType[m_dateColumn] = "date";
    m_dateSelected = true;

  } else {
    m_dateSelected = false;
    comboBox_dateCol->setCurrentIndex(-1);
    KMessageBox::information(0, i18n("That column, or the date field, is selected already!\
    <center>Please select a different column or field.</center>"));
  }
}

void CsvImporterDlg::memoColumnSelected(int index)
{
  m_memoColumn = index;
  if ((m_memoColumn < 0) || (m_memoColumn >= m_csvprocessing->m_lastColumn)) {
    comboBox_memoCol->setCurrentIndex(-1);
    return;
  }
  if (m_columnType[m_memoColumn].isEmpty()) {
    m_columnType[m_memoColumn] = "memo";
    m_memoSelected = true;
  } else {
    KMessageBox::information(0, i18n("That column is selected already!\
    <center>Please select a different column or field.</center>"));
  }
}

void CsvImporterDlg::payeeColumnSelected(int col)
{
  m_payeeColumn = col;
  if ((m_payeeColumn < 0) || (m_payeeColumn >= m_csvprocessing->m_lastColumn)) {
    comboBox_payeeCol->setCurrentIndex(-1);
    return;
  }
  if ((m_columnType[m_payeeColumn].isEmpty()) && (m_payeeSelected == false)) {
    m_columnType[m_payeeColumn] = "payee";
    m_payeeSelected = true;
  } else {
    m_payeeSelected = false;
    comboBox_payeeCol->setCurrentIndex(-1);
    KMessageBox::information(0, i18n("That column, or the payee field, is selected already!\
    <center>Please select a different column or field. </center>"));
  }
}

void CsvImporterDlg::numberColumnChanged(int num)
{
  m_numberColumn = num;
  if ((m_numberColumn < 0) || (m_numberColumn >= m_csvprocessing->m_lastColumn)) {//invalid col.
    comboBox_numberCol->setCurrentIndex(-1);
    return;
  }
  if ((m_columnType[m_numberColumn].isEmpty()) && (m_numberSelected == false)) {
    m_columnType[m_numberColumn] = "number";
    m_numberSelected = true;
  } else {
    comboBox_numberCol->setCurrentIndex(-1);
    KMessageBox::information(0, i18n("That column, or the number field, is selected already!\
    <center>Please select a different column or field.</center>"));
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
  profileGroup.writeEntry("FieldDelimiter", m_csvprocessing->m_fieldDelimiterIndex);
  profileGroup.writeEntry("StartLine", spinBox_skip->value() - 1);

  QString str = "$HOME/" + m_csvprocessing->m_csvPath.section('/', 3);
  profileGroup.writeEntry("CsvDirectory", str);

  profileGroup.config()->sync();

  if (!m_csvprocessing->m_inFileName.isEmpty()) { //  don't save column numbers if no file loaded
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
  if (!m_csvprocessing->m_inFileName.isEmpty())
    m_csvprocessing->updateScreen();
}

