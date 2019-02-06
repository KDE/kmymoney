/*******************************************************************************
*                                 csvwizard.cpp
*                              --------------------
* begin                       : Thur Jan 01 2015
* copyright                   : (C) 2015 by Allan Anderson
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

#include "csvwizard.h"

#include <QtGui/QWizard>
#include <QtGui/QWizardPage>
#include <QTimer>
#include <QDebug>
#include <QDesktopWidget>

#include <KMessageBox>
#include <KStandardDirs>

#include "csvdialog.h"
#include "convdate.h"
#include "csvutil.h"
#include "investmentdlg.h"
#include "investprocessing.h"
#include "symboltabledlg.h"

#include "mymoneyfile.h"

#include "ui_csvdialog.h"
#include "ui_csvwizard.h"
#include "ui_introwizardpage.h"
#include "ui_separatorwizardpage.h"
#include "ui_bankingwizardpage.h"
#include "ui_lines-datewizardpage.h"
#include "ui_completionwizardpage.h"
#include "ui_investmentwizardpage.h"

CSVWizard::CSVWizard() :
    ui(new Ui::CSVWizard),
    m_investProcessing(0),
    m_pageIntro(0),
    m_pageSeparator(0),
    m_pageBanking(0),
    m_pageInvestment(0),
    m_pageLinesDate(0),
    m_pageCompletion(0),
    m_csvDialog(0)
{
  ui->setupUi(this);

  m_curId = -1;
  m_lastId = -1;

  m_wizard = new QWizard;
  m_wizard->setWizardStyle(QWizard::ClassicStyle);
  ui->horizontalLayout->addWidget(m_wizard, 100);

  m_iconBack = QPixmap(KIconLoader::global()->loadIcon("go-previous", KIconLoader::Small, KIconLoader::DefaultState));
  m_iconFinish = QPixmap(KIconLoader::global()->loadIcon("dialog-ok-apply", KIconLoader::Small, KIconLoader::DefaultState));
  m_iconCancel = QPixmap(KIconLoader::global()->loadIcon("dialog-cancel", KIconLoader::Small, KIconLoader::DefaultState));
  m_iconCSV = QPixmap(KIconLoader::global()->loadIcon("kmymoney", KIconLoader::Small, KIconLoader::DefaultState));
  m_iconImport = QPixmap(KIconLoader::global()->loadIcon("system-file-manager.", KIconLoader::Small, KIconLoader::DefaultState));
  m_iconQIF = QPixmap(KIconLoader::global()->loadIcon("invest-applet", KIconLoader::Small, KIconLoader::DefaultState));

  m_wizard->button(QWizard::BackButton)->setIcon(m_iconBack);
  m_wizard->button(QWizard::CancelButton)->setIcon(m_iconCancel);
  m_wizard->button(QWizard::CustomButton2)->setIcon(m_iconCSV);
  m_wizard->button(QWizard::FinishButton)->setIcon(m_iconFinish);
  m_wizard->button(QWizard::CustomButton1)->setIcon(m_iconImport);
  m_wizard->button(QWizard::CustomButton3)->setIcon(m_iconQIF);
  m_wizard->button(QWizard::NextButton)->setIcon(KStandardGuiItem::forward(KStandardGuiItem::UseRTL).icon());

  m_wizard->setDefaultProperty("QComboBox", "source", SIGNAL(currentIndexChanged(int)));
  m_wizard->setDefaultProperty("QComboBox", "symbolCol", SIGNAL(currentIndexChanged(int)));
  m_wizard->setDefaultProperty("KComboBox", "dateCol", SIGNAL(currentIndexChanged(int)));
  m_wizard->setDefaultProperty("QComboBox", "dateCol", SIGNAL(currentIndexChanged(int)));
}

void CSVWizard::init()
{
  installEventFilter(this);
  m_pageIntro = new IntroPage;
  m_wizard->setPage(Page_Intro, m_pageIntro);
  m_pageIntro->setParent(this);

  m_pageSeparator = new SeparatorPage;
  m_wizard->setPage(Page_Separator, m_pageSeparator);
  m_pageSeparator->setParent(this);

  m_pageBanking = new BankingPage;
  m_wizard->setPage(Page_Banking, m_pageBanking);
  m_pageBanking->setParent(this);

  m_pageInvestment = new InvestmentPage;
  m_wizard->setPage(Page_Investment, m_pageInvestment);
  m_pageInvestment->setParent(this);

  m_pageLinesDate = new LinesDatePage;
  m_wizard->setPage(Page_LinesDate, m_pageLinesDate);
  m_pageLinesDate->setParent(this);

  m_pageCompletion = new CompletionPage;
  m_wizard->setPage(Page_Completion, m_pageCompletion);
  m_pageCompletion->setParent(this);

  m_stageLabels << ui->label_intro << ui->label_separator << ui->label_banking << ui->label_investing << ui->label_lines << ui->label_finish;

  this->setAttribute(Qt::WA_DeleteOnClose, true);

  connect(m_wizard->button(QWizard::CancelButton), SIGNAL(clicked()), this, SLOT(slotClose()));

  connect(m_pageIntro->ui->radioButton_bank, SIGNAL(clicked()), m_pageIntro, SLOT(slotRadioButton_bankClicked()));
  connect(m_pageIntro->ui->radioButton_invest, SIGNAL(clicked()), m_pageIntro, SLOT(slotRadioButton_investClicked()));

  connect(m_pageBanking->ui->radioBnk_amount, SIGNAL(clicked(bool)), this, SLOT(amountRadioClicked(bool)));
  connect(m_pageBanking->ui->radioBnk_debCred, SIGNAL(clicked(bool)), this, SLOT(debitCreditRadioClicked(bool)));
  connect(m_pageBanking->ui->button_clear, SIGNAL(clicked()), this, SLOT(clearColumnsSelected()));

  connect(m_pageSeparator->ui->comboBox_fieldDelimiter, SIGNAL(currentIndexChanged(int)), m_csvDialog->m_investProcessing, SLOT(fieldDelimiterChanged()));
  connect(m_pageSeparator->ui->comboBox_fieldDelimiter, SIGNAL(activated(int)), m_csvDialog->m_investProcessing, SLOT(fieldDelimiterChanged()));
  connect(m_pageSeparator->ui->comboBox_fieldDelimiter, SIGNAL(activated(int)), m_pageSeparator, SLOT(delimiterActivated()));
  connect(m_pageSeparator->ui->comboBox_fieldDelimiter, SIGNAL(activated(int)), m_csvDialog, SLOT(delimiterActivated()));
  connect(m_pageSeparator->ui->comboBox_fieldDelimiter, SIGNAL(currentIndexChanged(int)), m_csvDialog, SLOT(delimiterChanged()));

  connect(m_pageInvestment->ui->comboBoxInv_securityName, SIGNAL(activated(int)), m_pageInvestment, SLOT(slotsecurityNameChanged(int)));
  connect(m_pageInvestment->ui->button_clear, SIGNAL(clicked()), m_csvDialog->m_investProcessing, SLOT(clearColumnsSelected()));

  connect(m_pageLinesDate->ui->spinBox_skipToLast, SIGNAL(valueChanged(int)), m_csvDialog, SLOT(endLineChanged(int)));
  connect(m_pageLinesDate->ui->comboBox_dateFormat, SIGNAL(currentIndexChanged(int)), m_csvDialog, SLOT(dateFormatSelected(int)));
  connect(m_pageLinesDate->ui->comboBox_dateFormat, SIGNAL(currentIndexChanged(int)), m_csvDialog->m_investProcessing, SLOT(dateFormatSelected(int)));
  connect(m_pageLinesDate->ui->comboBox_dateFormat, SIGNAL(currentIndexChanged(int)), m_csvDialog->m_convertDate, SLOT(dateFormatSelected(int)));

  connect(m_pageCompletion->ui->comboBox_decimalSymbol, SIGNAL(currentIndexChanged(int)), m_csvDialog->m_parse, SLOT(decimalSymbolSelected(int)));
  connect(m_pageCompletion->ui->comboBox_decimalSymbol, SIGNAL(activated(int)), m_csvDialog, SLOT(decimalSymbolSelected(int)));
  connect(m_pageCompletion, SIGNAL(importBanking()), m_csvDialog, SLOT(slotImportClicked()));
  connect(m_pageCompletion, SIGNAL(importInvestment()), m_csvDialog->m_investProcessing, SLOT(slotImportClicked()));
  connect(m_pageCompletion, SIGNAL(completeChanged()), this, SLOT(slotClose()));

  connect(m_wizard->button(QWizard::CustomButton1), SIGNAL(clicked()), m_csvDialog, SLOT(slotFileDialogClicked()));
  connect(m_wizard->button(QWizard::BackButton), SIGNAL(clicked()), m_csvDialog, SLOT(slotBackButtonClicked()));
  connect(m_wizard->button(QWizard::CustomButton2), SIGNAL(clicked()), m_pageCompletion, SLOT(slotImportClicked()));
  connect(m_wizard->button(QWizard::CustomButton3), SIGNAL(clicked()), m_csvDialog, SLOT(slotSaveAsQIF()));
  connect(m_wizard->button(QWizard::FinishButton), SIGNAL(clicked()), this, SLOT(slotClose()));
  connect(m_wizard, SIGNAL(currentIdChanged(int)), this, SLOT(slotIdChanged(int)));

  connect(m_csvDialog, SIGNAL(isImportable()), m_pageCompletion, SLOT(slotImportValid()));

  int y = (QApplication::desktop()->height() - m_csvDialog->height()) / 2;
  int x = (QApplication::desktop()->width() - m_csvDialog->width()) / 2;
  m_csvDialog->move(x, y);
  m_csvDialog->show();

  y = (QApplication::desktop()->height() - this->height()) / 2;
  x = (QApplication::desktop()->width() - this->width()) / 2;
  move(x, y);
}

CSVWizard::~CSVWizard()
{
  delete ui;
}

void CSVWizard::showStage()
{
  QString str = ui->label_intro->text();
  ui->label_intro->setText("<b>" + str + "</b>");
}

void CSVWizard::slotIdChanged(int id)
{
  QString txt;
  m_lastId = m_curId;
  m_curId = id;
  if ((m_lastId == -1) || (m_curId == -1)) {
    return;
  }
  txt = m_stageLabels[m_lastId]->text();
  txt.remove(QRegExp("[<b>/]"));
  m_stageLabels[m_lastId]->setText(txt);

  txt = m_stageLabels[m_curId]->text();
  txt = "<b>" + txt + "</b>";
  m_stageLabels[m_curId]->setText(txt);
}

void CSVWizard::amountRadioClicked(bool checked)
{
  if (checked) {
    m_pageBanking->ui->comboBoxBnk_amountCol->setEnabled(true);  //  disable credit & debit ui choices
    m_pageBanking->ui->labelBnk_amount->setEnabled(true);
    m_pageBanking->ui->labelBnk_credits->setEnabled(false);
    m_pageBanking->ui->labelBnk_debits->setEnabled(false);

    //   the 'm_creditColumn/m_debitColumn' could just have been reassigned, so ensure
    //   ...they == "credit or debit" before clearing them
    if ((m_csvDialog->creditColumn() >= 0) && (m_csvDialog->m_columnTypeList.indexOf("credit") != -1)) {
      m_csvDialog->m_columnTypeList.replace(m_csvDialog->m_columnTypeList.indexOf("credit"), QString());//       because amount col chosen...
    }
    if ((m_csvDialog->debitColumn() >= 0) && (m_csvDialog->m_columnTypeList.indexOf("debit") != -1)) {
      m_csvDialog->m_columnTypeList.replace(m_csvDialog->m_columnTypeList.indexOf("debit"), QString());//        ...drop any credit & debit
    }
    m_csvDialog->setDebitColumn(-1);
    m_csvDialog->setCreditColumn(-1);
    m_pageBanking->ui->comboBoxBnk_debitCol->setEnabled(false);
    m_pageBanking->ui->comboBoxBnk_debitCol->setCurrentIndex(-1);
    m_pageBanking->ui->comboBoxBnk_creditCol->setEnabled(false);
    m_pageBanking->ui->comboBoxBnk_creditCol->setCurrentIndex(-1);
  }
}

void CSVWizard::amountColumnSelected(int col)
{
  if (col < 0) {      //                                 it is unset
    m_wizard->button(QWizard::NextButton)->setEnabled(false);
    return;
  }
  QString type = "amount";
  m_csvDialog->setAmountColumn(col);

  // if a previous amount field is detected, but in a different column...
  if ((m_csvDialog->amountColumn() != -1) && (m_csvDialog->m_columnTypeList[m_csvDialog->amountColumn()] == type)  && (m_csvDialog->amountColumn() != col)) {
    m_csvDialog->m_columnTypeList[m_csvDialog->amountColumn()].clear();
  }
  int ret = m_csvDialog->validateColumn(col, type);
  if (ret == KMessageBox::Ok) {
    m_pageBanking->ui->comboBoxBnk_amountCol->setCurrentIndex(col);  //    accept new column
    m_csvDialog->m_amountSelected = true;
    m_csvDialog->setAmountColumn(col);
    m_csvDialog->m_columnTypeList[m_csvDialog->amountColumn()] = type;
    return;
  }
  if (ret == KMessageBox::No) {
    m_pageBanking->ui->comboBoxBnk_amountCol->setCurrentIndex(-1);
  }
}

void CSVWizard::debitCreditRadioClicked(bool checked)
{
  if (checked) {
    m_pageBanking->ui->comboBoxBnk_debitCol->setEnabled(true);  //         if 'debit/credit' selected
    m_pageBanking->ui->labelBnk_debits->setEnabled(true);
    m_pageBanking->ui->comboBoxBnk_creditCol->setEnabled(true);
    m_pageBanking->ui->labelBnk_credits->setEnabled(true);
    //   the 'm_amountColumn' could just have been reassigned, so ensure
    //   ...m_columnTypeList[m_amountColumn] == "amount" before clearing it
    if ((m_csvDialog->amountColumn() >= 0) && (m_csvDialog->m_columnTypeList.indexOf("amount") != -1)) {
      m_csvDialog->m_columnTypeList.replace(m_csvDialog->m_columnTypeList.indexOf("amount"), QString());// ...drop any amount choice
      m_csvDialog->setAmountColumn(-1);
    }
    m_pageBanking->ui->comboBoxBnk_amountCol->setEnabled(false);  //       disable 'amount' ui choices
    m_pageBanking->ui->comboBoxBnk_amountCol->setCurrentIndex(-1);  //     as credit/debit chosen
    m_pageBanking->ui->labelBnk_amount->setEnabled(false);
  }
}

void CSVWizard::creditColumnSelected(int col)
{
  if (col < 0) {      //                                                   it is unset
    m_wizard->button(QWizard::NextButton)->setEnabled(false);
    return;
  }
  QString type = "credit";
  m_csvDialog->setCreditColumn(col);

  // if a previous credit field is detected, but in a different column...
  if ((m_csvDialog->creditColumn() != -1) && (m_csvDialog->m_columnTypeList[m_csvDialog->creditColumn()] == type)  && (m_csvDialog->creditColumn() != col)) {
    m_csvDialog->m_columnTypeList[m_csvDialog->creditColumn()].clear();
  }
  int ret = m_csvDialog->validateColumn(col, type);

  if (ret == KMessageBox::Ok) {
    m_pageBanking->ui->comboBoxBnk_creditCol->setCurrentIndex(col);  //    accept new column
    m_csvDialog->m_creditSelected = true;
    m_csvDialog->setCreditColumn(col);
    m_csvDialog->m_columnTypeList[m_csvDialog->creditColumn()] = type;
    return;
  } else if (ret == KMessageBox::No) {
    m_pageBanking->ui->comboBoxBnk_creditCol->setCurrentIndex(-1);
  }
}

void CSVWizard::debitColumnSelected(int col)
{
  if (col < 0) {      //  it is unset
    m_wizard->button(QWizard::NextButton)->setEnabled(false);
    return;
  }
  QString type = "debit";
  m_csvDialog->setDebitColumn(col);

  // A new column has been selected for this field so clear old one
  if ((m_csvDialog->debitColumn() != -1) && (m_csvDialog->m_columnTypeList[m_csvDialog->debitColumn()] == type)  && (m_csvDialog->debitColumn() != col)) {
    m_csvDialog->m_columnTypeList[m_csvDialog->debitColumn()].clear();
  }
  int ret = m_csvDialog->validateColumn(col, type);

  if (ret == KMessageBox::Ok) {
    m_pageBanking->ui->comboBoxBnk_debitCol->setCurrentIndex(col);  //     accept new column
    m_csvDialog->m_debitSelected = true;
    m_csvDialog->setDebitColumn(col);
    m_csvDialog->m_columnTypeList[m_csvDialog->debitColumn()] = type;
    return;
  } else if (ret == KMessageBox::No) {
    m_pageBanking->ui->comboBoxBnk_debitCol->setCurrentIndex(-1);
  }
}

void CSVWizard::dateColumnSelected(int col)
{
  if (col < 0) {      //  it is unset
    m_wizard->button(QWizard::NextButton)->setEnabled(false);
    return;
  }
  QString type = "date";
  m_csvDialog->setDateColumn(col);

  // A new column has been selected for this field so clear old one
  if ((m_csvDialog->dateColumn() != -1) && (m_csvDialog->m_columnTypeList[m_csvDialog->dateColumn()] == type)  && (m_csvDialog->dateColumn() != col)) {
    m_csvDialog->m_columnTypeList[m_csvDialog->dateColumn()].clear();
  }
  int ret = m_csvDialog->validateColumn(col, type);

  if (ret == KMessageBox::Ok) {
    m_pageBanking->ui->comboBoxBnk_dateCol->setCurrentIndex(col);  // accept new column
    m_csvDialog->m_dateSelected = true;
    m_csvDialog->setDateColumn(col);
    m_csvDialog->m_columnTypeList[col] = type;
    return;
  } else if (ret == KMessageBox::No) {
    m_pageBanking->ui->comboBoxBnk_dateCol->setCurrentIndex(-1);
  }
}

void CSVWizard::memoColumnSelected(int col)
{
  //  Prevent check of column settings until user sees them.
  if ((col < 0) || (col >= m_csvDialog->m_endColumn) || (m_csvDialog->m_columnsNotSet)) {   //  out of range so...
    m_pageBanking->ui->comboBoxBnk_memoCol->setCurrentIndex(-1);  //  ..clear selection
    return;
  }
  QString type = "memo";
  m_csvDialog->setMemoColumn(col);

  if (m_csvDialog->m_columnTypeList[col].isEmpty()) {  //  accept new  entry
    m_pageBanking->ui->comboBoxBnk_memoCol->setItemText(col, QString().setNum(col + 1) + '*');
    m_csvDialog->m_columnTypeList[col] = type;
    m_csvDialog->setMemoColumn(col);
    if (m_csvDialog->m_memoColList.contains(col)) {
      //  Restore the '*' as column might have been cleared.
      m_pageBanking->ui->comboBoxBnk_memoCol->setItemText(col, QString().setNum(col + 1) + '*');
    } else {
      m_csvDialog->m_memoColList << col;
    }
    m_csvDialog->m_memoSelected = true;
    return;
  } else if (m_csvDialog->m_columnTypeList[col] == type) {  //  nothing changed
    return;
  } else if (m_csvDialog->m_columnTypeList[col] == "payee") {
    if ((m_csvDialog->m_memoColList.contains(col)) && (m_csvDialog->m_payeeColAdded)) {
      return;  //                          This copypayee column has been added already, probably from resource file
    }
    int rc = KMessageBox::Yes;
    if (m_pageBanking->isVisible()) {  //  Don't show msg. if we got here from resource file load
      rc = KMessageBox::questionYesNo(nullptr, i18n("<center>The '<b>%1</b>' field already has this column selected.</center>"
                                              "<center>If you wish to copy the Payee data to the memo field, click 'Yes'.</center>",
                                              m_csvDialog->m_columnTypeList[col]));
    }
    if (rc == KMessageBox::Yes) {
      m_csvDialog->m_payeeColCopied = true;
      m_csvDialog->m_payeeColAdded = true;  //  Indicate that extra col has been added already
      m_pageBanking->ui->comboBoxBnk_memoCol->setItemText(col, QString().setNum(col + 1) + '*');

      if (!m_csvDialog->m_memoColList.contains(col)) {
        m_csvDialog->m_memoColList << col;
      }
      m_csvDialog->m_columnTypeList << "memo";
      if (m_csvDialog->m_columnList.count() < m_csvDialog->m_columnTypeList.count()) {
        m_csvDialog->m_columnList << "";
        m_csvDialog->m_maxColumnCount ++;
        m_csvDialog->m_endColumn ++;
      }
      m_csvDialog->setMemoColumn(m_csvDialog->m_endColumn);
      m_csvDialog->m_memoSelected = true;
      m_csvDialog->m_columnCountList << m_csvDialog->m_maxColumnCount + 1;
      return;
    }
  } else {
    //  clashes with prior selection
    m_csvDialog->m_memoSelected = false;  //  clear incorrect selection
    m_csvDialog->m_payeeColCopied = false;
    if (m_pageBanking->isVisible()) {
      KMessageBox::information(nullptr, i18n("The '<b>%1</b>' field already has this column selected. <center>Please reselect both entries as necessary.</center>"
                                       , m_csvDialog->m_columnTypeList[col]));
      m_pageBanking->ui->comboBoxBnk_memoCol->setCurrentIndex(-1);
      m_csvDialog->setPreviousColumn(-1);
      resetComboBox(m_csvDialog->m_columnTypeList[col], col);  //  clash,  so reset ..
      resetComboBox(type, col);  //                                ... both comboboxes
      m_csvDialog->clearPreviousColumn();
      m_csvDialog->m_columnTypeList[col].clear();
      m_pageBanking->ui->comboBoxBnk_memoCol->setItemText(col, QString().setNum(col + 1));   //  reset the '*'
    }
  }
}

void CSVWizard::payeeColumnSelected(int col)
{
  if (col < 0) {  //  it is unset
    m_wizard->button(QWizard::NextButton)->setEnabled(false);
    return;
  }
  QString type = "payee";
  // if a previous payee field is detected, but in a different column...
  if ((m_csvDialog->payeeColumn() != -1) && (m_csvDialog->m_columnTypeList[m_csvDialog->payeeColumn()] == type)  && (m_csvDialog->payeeColumn() != col)) {
    m_csvDialog->m_columnTypeList[m_csvDialog->payeeColumn()].clear();
  }
  m_csvDialog->setPayeeColumn(col);

  int ret = m_csvDialog->validateColumn(col, type);
  if (ret == KMessageBox::Ok) {
    m_pageBanking->ui->comboBoxBnk_payeeCol->setCurrentIndex(col);  // accept new column
    m_csvDialog->m_payeeSelected = true;
    m_csvDialog->setPayeeColumn(col);
    m_csvDialog->m_columnTypeList[m_csvDialog->payeeColumn()] = type;
    return;
  } else if (ret == KMessageBox::No) {
    m_pageBanking->ui->comboBoxBnk_payeeCol->setCurrentIndex(-1);
  }
}

void CSVWizard::numberColumnSelected(int col)
{
  if (col < 0) {      //  it is unset
    return;
  }
  QString type = "number";
  m_csvDialog->setNumberColumn(col);

  // if a previous number field is detected, but in a different column...
  if ((m_csvDialog->numberColumn() != -1) && (m_csvDialog->m_columnTypeList[m_csvDialog->numberColumn()] == type)  && (m_csvDialog->numberColumn() != col)) {
    m_csvDialog->m_columnTypeList[m_csvDialog->numberColumn()].clear();
  }
  int ret = m_csvDialog->validateColumn(col, type);

  if (ret == KMessageBox::Ok) {
    m_pageBanking->ui->comboBoxBnk_numberCol->setCurrentIndex(col);  // accept new column
    m_csvDialog->m_numberSelected = true;
    m_csvDialog->setNumberColumn(col);
    m_csvDialog->m_columnTypeList[m_csvDialog->numberColumn()] = type;
    return;
  } else if (ret == KMessageBox::No) {
    m_pageBanking->ui->comboBoxBnk_numberCol->setCurrentIndex(-1);
  }
}

void CSVWizard::categoryColumnSelected(int col)
{
  if (col < 0) {  //  it is unset
    m_wizard->button(QWizard::NextButton)->setEnabled(false);
    return;
  }
  QString type = "category";
  // if a previous payee field is detected, but in a different column...
  if ((m_csvDialog->categoryColumn() != -1) && (m_csvDialog->m_columnTypeList[m_csvDialog->categoryColumn()] == type)  && (m_csvDialog->categoryColumn() != col)) {
    m_csvDialog->m_columnTypeList[m_csvDialog->categoryColumn()].clear();
  }
  m_csvDialog->setCategoryColumn(col);

  int ret = m_csvDialog->validateColumn(col, type);
  if (ret == KMessageBox::Ok) {
    m_pageBanking->ui->comboBoxBnk_categoryCol->setCurrentIndex(col);  // accept new column
    m_csvDialog->m_categorySelected = true;
    m_csvDialog->setCategoryColumn(col);
    m_csvDialog->m_columnTypeList[m_csvDialog->categoryColumn()] = type;
    return;
  } else if (ret == KMessageBox::No) {
    m_pageBanking->ui->comboBoxBnk_categoryCol->setCurrentIndex(-1);
  }
}

void CSVWizard::clearColumnsSelected()
{
  //  User has clicked clear button
  if (m_csvDialog->m_fileType == "Banking") {
    m_csvDialog->clearPreviousColumn();
    m_csvDialog->clearSelectedFlags();
    m_csvDialog->clearColumnNumbers();
    m_csvDialog->clearComboBoxText();
    m_csvDialog->m_memoColCopied = false;
    m_csvDialog->m_payeeColCopied = false;
    m_csvDialog->m_memoColList.clear();
  }
}

void CSVWizard::slotClose()
{
  if (!m_csvDialog->m_closing) {
    m_csvDialog->saveSettings();
    m_csvDialog->m_investmentDlg->saveSettings();
    m_csvDialog->m_wiz = 0;
    m_csvDialog->close();
  }
  close();
}

void CSVWizard::resetComboBox(const QString& comboBox, const int& col)
{
  QStringList fieldType;
  fieldType << "amount" << "credit" << "date" << "debit" << "memo" << "number" << "payee" << "category";
  int index = fieldType.indexOf(comboBox);
  switch (index) {
    case 0://  amount
      m_pageBanking->ui->comboBoxBnk_amountCol->setCurrentIndex(-1);
      m_csvDialog->m_amountSelected = false;
      break;
    case 1://  credit
      m_pageBanking->ui->comboBoxBnk_creditCol->setCurrentIndex(-1);
      m_csvDialog->m_creditSelected = false;
      break;
    case 2://  date
      m_pageBanking->ui->comboBoxBnk_dateCol->setCurrentIndex(-1);
      m_csvDialog->m_dateSelected = false;
      break;
    case 3://  debit
      m_pageBanking->ui->comboBoxBnk_debitCol->setCurrentIndex(-1);
      m_csvDialog->m_debitSelected = false;
      break;
    case 4://  memo
      m_pageBanking->ui->comboBoxBnk_memoCol->setCurrentIndex(-1);
      m_pageBanking->ui->comboBoxBnk_memoCol->setItemText(col, QString().setNum(col + 1));   //  reset the '*'
      m_csvDialog->m_memoSelected = false;
      break;
    case 5://  number
      m_pageBanking->ui->comboBoxBnk_numberCol->setCurrentIndex(-1);
      m_csvDialog->m_numberSelected = false;
      break;
    case 6://  payee
      m_pageBanking->ui->comboBoxBnk_payeeCol->setCurrentIndex(-1);
      m_csvDialog->m_payeeSelected = false;
      break;
    case 7://  category
      m_pageBanking->ui->comboBoxBnk_categoryCol->setCurrentIndex(-1);
      m_csvDialog->m_categorySelected = false;
      break;
    default:
      KMessageBox::sorry(this, i18n("<center>Field name not recognised.</center> <center>'<b>%1</b>'</center> Please re-enter your column selections."
                                    , comboBox), i18n("CSV import"));
  }
  m_csvDialog->m_columnTypeList[col].clear();
  return;
}

bool CSVWizard::eventFilter(QObject *object, QEvent *event)
{
  // prevent the wizard from closing on escape leaving the importer empty
  if (object == m_csvDialog->m_wiz && event->type() == QEvent::KeyPress) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    if (keyEvent->key() == Qt::Key_Escape) {
      close();
    }
    return true;
  } else if (event->spontaneous()) {
    if (event->type() == 19) {
      slotClose();
    }
  }
  return false;
}

void CSVWizard::resizeEvent(QResizeEvent* ev)
{
  if (ev->spontaneous()) {
    ev->ignore();
    return;
  }
}

//-------------------------------------------------------------------------------------------------------
IntroPage::IntroPage(QWidget *parent) :
    CSVWizardPage(parent),
    ui(new Ui::IntroPage),
    m_pageLayout(0),
    m_firstEdit(false),
    m_editAccepted(false)
{
  ui->setupUi(this);
  m_priorIndex = 0;
  m_priorName = QString();
  m_addRequested = false;
  m_lastRadioButton.clear();
  m_firstLineEdit = true;
  m_messageBoxJustCancelled = false;
  registerField("source", ui->combobox_source, "currentIndex", SIGNAL(currentIndexChanged()));
  disconnect(ui->combobox_source, 0, 0, 0);

  m_index = 1;

  ui->radioButton_bank->show();
  ui->radioButton_invest->show();
}

IntroPage::~IntroPage()
{
  delete ui;
}

void IntroPage::setParent(CSVWizard* dlg)
{
  CSVWizardPage::setParent(dlg);

  if (QApplication::desktop()->fontInfo().pixelSize() < 20) {
    m_wizDlg->resize(m_wizDlg->width() - 100, m_wizDlg->height() - 80);
  }
  registerField("csvdialog", m_wizDlg, "m_set", SIGNAL(isSet()));
  m_wizDlg->showStage();

  wizard()->button(QWizard::CustomButton1)->setEnabled(false);
}

void IntroPage::slotComboEditTextChanged(QString txt)
{
  if ((ui->combobox_source->isHidden()) || (m_messageBoxJustCancelled) || (field("source").toInt() < 0)) {
    return;
  }
  m_index = field("source").toInt();
  m_messageBoxJustCancelled = false;
  if ((field("source").toInt() == 0) && (txt.isEmpty())) {
    ui->combobox_source->setCurrentIndex(-1);
    return;
  }
  if (m_priorName.isEmpty()) {
    m_priorName = m_wizDlg->m_csvDialog->m_profileName;
  }
  if (txt == m_priorName) {
    int indx = ui->combobox_source->findText(txt);
    ui->combobox_source->setCurrentIndex(indx);
    m_messageBoxJustCancelled = false;
    return;
  }
  if ((ui->combobox_source->count() == m_index) && (!txt.isEmpty())) {
    //  Not finished entering text.
    return;
  }
  if (m_firstLineEdit) {
    m_firstLineEdit = false;
    connect(ui->combobox_source->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotLineEditingFinished()));
  }
  if ((txt.isEmpty()) && (!m_priorName.isEmpty()) && (m_messageBoxJustCancelled == false) && (ui->combobox_source->lineEdit()->text().isEmpty())) {
    //
    //  The disconnects are to avoid another messagebox appearing before the response for this one is processed.
    //
    disconnect(ui->combobox_source, SIGNAL(editTextChanged(QString)), this, SLOT(slotComboEditTextChanged(QString)));
    disconnect(ui->combobox_source->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotLineEditingFinished()));
    m_messageBoxJustCancelled = false;

    int rc = KMessageBox::warningYesNo(nullptr, i18n("<center>You have cleared the profile name '%1'.</center>\n"
                                       "<center>If you wish to delete the entry, click 'Delete'.</center>\n"
                                       "<center>Otherwise, click 'Keep'.</center>", m_wizDlg->m_csvDialog->m_profileName),
                                       i18n("Delete or Edit Profile Name"),
                                       KGuiItem(i18n("Delete")),
                                       KGuiItem(i18n("Keep")), "");
    if (rc == KMessageBox::No) {
      //
      //  Keep
      //
      ui->combobox_source->setCurrentItem(m_priorName);
      connect(ui->combobox_source, SIGNAL(editTextChanged(QString)), this, SLOT(slotComboEditTextChanged(QString)));
      connect(ui->combobox_source->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotLineEditingFinished()));
      return;
    }
    //
    //  Delete, just clear to allow new text entry.
    //
    m_wizDlg->m_csvDialog->m_profileList.removeOne(m_wizDlg->m_csvDialog->m_profileName);
    int indx = ui->combobox_source->findText(m_wizDlg->m_csvDialog->m_profileName);
    ui->combobox_source->removeItem(indx);
    m_map.take(m_wizDlg->m_csvDialog->m_profileName);
    ui->combobox_source->setCurrentIndex(-1);
    m_priorName.clear();
    KSharedConfigPtr config = KSharedConfig::openConfig(KStandardDirs::locateLocal("config", "csvimporterrc"));
    KConfigGroup bankProfilesGroup(config, "BankProfiles");
    KConfigGroup deletedProfilesGroup(config, "Profiles-" + m_wizDlg->m_csvDialog->m_profileName);
    m_wizDlg->m_csvDialog->m_profileName.clear();
    deletedProfilesGroup.deleteGroup();
    if (m_wizDlg->m_csvDialog->m_fileType == "Banking") {
      m_wizDlg->m_csvDialog->m_priorCsvProfile.clear();
      bankProfilesGroup.writeEntry("PriorCsvProfile", m_wizDlg->m_csvDialog->m_priorCsvProfile);
    } else {
      m_wizDlg->m_csvDialog->m_priorInvProfile.clear();
      bankProfilesGroup.writeEntry("PriorInvProfile", m_wizDlg->m_csvDialog->m_priorInvProfile);
    }
    bankProfilesGroup.writeEntry("BankNames", m_wizDlg->m_csvDialog->m_profileList);
    bankProfilesGroup.config()->sync();
    connect(ui->combobox_source, SIGNAL(editTextChanged(QString)), this, SLOT(slotComboEditTextChanged(QString)));
    connect(ui->combobox_source->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotLineEditingFinished()));
    return;
  }
  connect(ui->combobox_source->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotLineEditingFinished()));
}

void IntroPage::slotComboSourceClicked(int index)
{
  m_messageBoxJustCancelled = false;
  connect(ui->combobox_source->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotLineEditingFinished()));
  switch (index) {   //  Add New Profile selected.
    case 0:
      ui->combobox_source->setCurrentIndex(-1);
      m_action = "add";
      m_addRequested = true;
      break;
    default:
      wizard()->button(QWizard::CustomButton1)->setEnabled(true);
      if (m_action == "add") {
        m_action.clear();
        QString txt = ui->combobox_source->currentText();
        if ((txt.isEmpty())) {
          return;
        }
        if (addItem(txt) == -1) {    //  Name already known.
          m_wizDlg->m_csvDialog->m_profileName = ui->combobox_source->currentText();
          if (m_wizDlg->m_csvDialog->m_fileType == "Banking") {
            m_wizDlg->m_csvDialog->m_priorCsvProfile = m_wizDlg->m_csvDialog->m_profileName;
          } else {
            m_wizDlg->m_csvDialog->m_priorInvProfile = m_wizDlg->m_csvDialog->m_profileName;
          }
          m_priorName = m_wizDlg->m_csvDialog->m_profileName;
          return;
        }
        //
        //  Adding new profile.
        //
        m_addRequested = false;
        addProfileName();
        return;
      } else {
        //
        //  Not adding so must be editing name, or selecting an existing profile.
        //
        QString txt = ui->combobox_source->currentText();
        m_priorName = m_wizDlg->m_csvDialog->m_profileName;
        m_priorIndex = m_index;
        if (!m_wizDlg->m_csvDialog->m_profileList.contains(txt)) {
          //  But this profile name does not exist.
          int indx = ui->combobox_source->findText(txt);
          if (m_priorName.isEmpty()) {
            disconnect(ui->combobox_source->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotLineEditingFinished()));
            //  Add it perhaps.
            QString question = i18n("<center>The name you have entered does not exist,</center>"
                                    "<center>but you have not elected to add a new profile</center>"
                                    "<center>If you wish to add '%1' as a new profile,</center>"
                                    "<center> click 'Yes'.  Otherwise, click 'No'</center>", txt);
            if (KMessageBox::questionYesNo(nullptr, question, i18n("Adding profile name.")) == KMessageBox::Yes) {
              addProfileName();
              m_index = indx;
              m_priorIndex = indx;
              connect(ui->combobox_source->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotLineEditingFinished()));
            } else {
              ui->combobox_source->removeItem(indx);
              ui->combobox_source->setCurrentIndex(-1);
              connect(ui->combobox_source->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotLineEditingFinished()));
              return;
            }
          }
          //  Otherwise maybe edit the present name.
          int ret = editProfileName(m_priorName, txt);
          if (ret == KMessageBox::No) {
            txt = m_priorName;
          } else {
            //  Use new name.
          }
          int index = ui->combobox_source->findText(txt, Qt::MatchExactly);
          ui->combobox_source->setCurrentIndex(index);
          return;
        }
        //  This profile is known so use it.
        m_priorName = ui->combobox_source->currentText();
        m_priorIndex = ui->combobox_source->currentIndex();

        m_wizDlg->m_csvDialog->m_profileName = ui->combobox_source->currentText();
        if (m_wizDlg->m_csvDialog->m_fileType == "Banking") {
          m_wizDlg->m_csvDialog->m_priorCsvProfile = m_wizDlg->m_csvDialog->m_profileName;
        } else {
          m_wizDlg->m_csvDialog->m_priorInvProfile = m_wizDlg->m_csvDialog->m_profileName;
        }
        if (m_wizDlg->m_csvDialog->m_profileList.contains(m_wizDlg->m_csvDialog->m_profileName)) {
          return;
        }
        editProfileName(m_priorName, m_wizDlg->m_csvDialog->m_profileName);
      }
  }
}

void  IntroPage::addProfileName()
{
  m_wizDlg->m_csvDialog->m_profileName = ui->combobox_source->currentText();
  if (m_wizDlg->m_csvDialog->m_fileType == "Banking") {
    m_wizDlg->m_csvDialog->m_priorCsvProfile = m_wizDlg->m_csvDialog->m_profileName;
  } else {
    m_wizDlg->m_csvDialog->m_priorInvProfile = m_wizDlg->m_csvDialog->m_profileName;
  }
  m_priorName = m_wizDlg->m_csvDialog->m_profileName;
  m_mapFileType.insert(m_wizDlg->m_csvDialog->m_profileName, m_wizDlg->m_csvDialog->m_fileType);
  m_wizDlg->m_csvDialog->m_profileList << m_wizDlg->m_csvDialog->m_profileName;
  m_wizDlg->m_csvDialog->createProfile(m_wizDlg->m_csvDialog->m_profileName);
  int indx = ui->combobox_source->findText(m_wizDlg->m_csvDialog->m_profileName);
  if (indx == -1) {
    ui->combobox_source->addItem(m_wizDlg->m_csvDialog->m_profileName);
  }
  indx = ui->combobox_source->findText(m_wizDlg->m_csvDialog->m_profileName);
  setField("source", indx);
}

int  IntroPage::editProfileName(QString& fromName, QString& toName)
{
  QString from = fromName;
  if (from == toName) {
    return  KMessageBox::No;
  }
  if (from.isEmpty()) {
    return  KMessageBox::Yes;
  }
  m_editAccepted = true;

  disconnect(ui->combobox_source->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotLineEditingFinished()));
  int fromIndx = ui->combobox_source->findText(from, Qt::MatchExactly);
  if (fromIndx != -1) {//  From name is in combobox.
    QString question = i18n("<center>You have edited the name of a profile</center>"
                            "<center>from '%1' to '%2'.</center>"
                            "<center>If you wish to accept the new name, click 'Yes'.</center>"
                            "<center>Otherwise, click 'No'</center>", from, toName);
    if (KMessageBox::questionYesNo(nullptr, question, i18n("Edit a profile name or create new one.")) == KMessageBox::Yes) {
      disconnect(ui->combobox_source, SIGNAL(editTextChanged(QString)), this, SLOT(slotComboEditTextChanged(QString)));
      //  Accept new name.
      m_map.take(from);
      m_wizDlg->m_csvDialog->m_profileList.removeOne(from);
      ui->combobox_source->removeItem(ui->combobox_source->findText(from, Qt::MatchExactly));
      int toIndx = ui->combobox_source->findText(toName, Qt::MatchExactly);
      if ((toIndx == -1) && (m_messageBoxJustCancelled == false)) {
        ui->combobox_source->addItem(toName);
      }
      m_index = ui->combobox_source->findText(toName, Qt::MatchExactly);
      m_wizDlg->m_csvDialog->m_profileName = toName;
      if (m_wizDlg->m_csvDialog->m_fileType == "Banking") {
        m_wizDlg->m_csvDialog->m_priorCsvProfile = m_wizDlg->m_csvDialog->m_profileName;
      } else {
        m_wizDlg->m_csvDialog->m_priorInvProfile = m_wizDlg->m_csvDialog->m_profileName;
      }
      m_wizDlg->m_csvDialog->createProfile(m_wizDlg->m_csvDialog->m_profileName);
      m_editAccepted = true;
      m_wizDlg->m_csvDialog->m_profileList << toName;
      m_priorName = toName;
      m_priorIndex = m_index;
      m_messageBoxJustCancelled = false;
      connect(ui->combobox_source->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotLineEditingFinished()));
      connect(ui->combobox_source, SIGNAL(editTextChanged(QString)), this, SLOT(slotComboEditTextChanged(QString)));
      return KMessageBox::Yes;
    } else {
      //  Don't accept new name so remove.
      int indx = ui->combobox_source->findText(toName);
      ui->combobox_source->removeItem(indx);
      m_wizDlg->m_csvDialog->m_profileList.removeOne(toName);
      if (m_wizDlg->m_csvDialog->m_fileType == "Banking") {
        m_wizDlg->m_csvDialog->m_priorCsvProfile = from;
      } else {
        m_wizDlg->m_csvDialog->m_priorInvProfile = from;
      }
      m_wizDlg->m_csvDialog->m_profileName = from;
      ui->combobox_source->setCurrentItem(from);
      m_editAccepted = false;
      connect(ui->combobox_source->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotLineEditingFinished()));
      connect(ui->combobox_source, SIGNAL(editTextChanged(QString)), this, SLOT(slotComboEditTextChanged(QString)));
      return KMessageBox::No;
    }
  } else {//  Old entry was deleted from combobox, and we then accepted new name.
    return KMessageBox::Yes;
  }
}

void IntroPage::slotRadioButton_bankClicked()
{
  if ((m_lastRadioButton != "Bank") && (!m_lastRadioButton.isEmpty())) {
    int rc = KMessageBox::warningContinueCancel(this, i18n("<center>If you continue, you will lose any recent profile edits.</center>"
             "<center>Continue or Cancel?</center>"), i18n("Radio button Banking clicked"), KStandardGuiItem::cont(),
             KStandardGuiItem::cancel());
    if (rc == KMessageBox::Cancel) {
      ui->radioButton_invest->setChecked(true);
      return;
    }
  }
  m_wizDlg->m_csvDialog->m_fileType = "Banking";
  ui->combobox_source->setEnabled(true);
  ui->combobox_source->show();

  m_wizDlg->m_csvDialog->readSettingsInit();
  m_priorName.clear();

  if ((!ui->combobox_source->currentText().isEmpty()) && (ui->combobox_source->currentIndex() >= 0)) {
    wizard()->button(QWizard::CustomButton1)->setEnabled(true);
  }
  ui->checkBoxSkipSetup->setEnabled(true);
  m_lastRadioButton = "Bank";
  //
  //  This below looks strange, but is necessary (I think, anyway), because if the alternate radio button
  //  is checked, multiple connects occur.  So, disconnect any existing connection then re-enable.
  //
  disconnect(ui->combobox_source, SIGNAL(editTextChanged(QString)), this, SLOT(slotComboEditTextChanged(QString)));
  connect(ui->combobox_source, SIGNAL(editTextChanged(QString)), this, SLOT(slotComboEditTextChanged(QString)));
}

void IntroPage::slotRadioButton_investClicked()
{
  if ((m_lastRadioButton != "Invest") && (!m_lastRadioButton.isEmpty())) {
    int rc = KMessageBox::warningContinueCancel(this, i18n("<center>If you continue, you will lose any recent profile edits.</center>"
             "<center>Continue or Cancel?</center>"), i18n("Radio button Investment clicked"), KStandardGuiItem::cont(),
             KStandardGuiItem::cancel());
    if (rc == KMessageBox::Cancel) {
      ui->radioButton_bank->setChecked(true);
      return;
    }
  }
  m_wizDlg->m_csvDialog->m_fileType = "Invest";
  ui->combobox_source->setEnabled(true);
  ui->combobox_source->show();

  m_wizDlg->m_csvDialog->readSettingsInit();
  m_priorName.clear();

  if ((!ui->combobox_source->currentText().isEmpty()) && (ui->combobox_source->currentIndex() >= 0)) {
    wizard()->button(QWizard::CustomButton1)->setEnabled(true);
  }
  ui->checkBoxSkipSetup->setEnabled(true);
  m_lastRadioButton = "Invest";
  //
  //  This below looks strange, but is necessary (I think, anyway), because if the alternate radio button
  //  is checked, multiple connects occur.  So, disconnect any existing connection then re-enable.
  //
  disconnect(ui->combobox_source, SIGNAL(editTextChanged(QString)), this, SLOT(slotComboEditTextChanged(QString)));
  connect(ui->combobox_source, SIGNAL(editTextChanged(QString)), this, SLOT(slotComboEditTextChanged(QString)));
}

int IntroPage::addItem(QString txt)
{
  if (txt.isEmpty()) {
    return -1;
  }
  disconnect(ui->combobox_source->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotLineEditingFinished()));
  int ret = -1;
  int indx = ui->combobox_source->findText(txt);

  QString question1 = i18n("<center>The name you have entered does not exist,</center>"
                           "<center>but you have not elected to add a new profile.</center>");
  QString question2 = i18n("<center>If you wish to add '%1'as a new profile,</center>"
                           "<center> click 'Yes'.  Otherwise, click 'No'</center>", txt);
  if (indx == -1) {
    //  Not found.

    if (!m_addRequested) {
      question2 = question1 + question2;
      if (KMessageBox::questionYesNo(nullptr, question2, i18n("Adding profile name.")) == KMessageBox::No) {
        ui->combobox_source->lineEdit()->clear();
        connect(ui->combobox_source->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotLineEditingFinished()));
        return ret;
      }
    }
    m_addRequested = false;
    ui->combobox_source->setCurrentItem(txt, true);
    indx = ui->combobox_source->findText(txt);
    m_index = indx;
    ret = 0;
  }  else {  //  Already exists.

    if ((!m_addRequested) && (!m_editAccepted)) {
      if (KMessageBox::questionYesNo(nullptr, question2, i18n("Adding profile name.")) == KMessageBox::No) {
        int indx = ui->combobox_source->findText(txt);
        ui->combobox_source->removeItem(indx);
        return ret;
      }
      m_index = indx;
    }
    if (!m_wizDlg->m_csvDialog->m_profileList.contains(txt)) {
      m_wizDlg->m_csvDialog->m_profileList << txt;
      m_wizDlg->m_csvDialog->createProfile(txt);
    }
    m_addRequested = false;
  }
  m_wizDlg->m_csvDialog->m_profileName = txt;
  connect(ui->combobox_source->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotLineEditingFinished()));
  return ret;
}

void IntroPage::initializePage()
{
  m_wizDlg->m_pageInvestment->m_investPageInitialized = false;
  m_wizDlg->m_pageBanking->m_bankingPageInitialized = false;
  //  ensure starting size is correct
  if (QApplication::desktop()->fontInfo().pixelSize() < 20) {
    QSize sizeLow(840, 320);
    m_wizDlg->resize(sizeLow);
  } else {
    QSize sizeHigh(900, 390);
    m_wizDlg->resize(sizeHigh);
  }
  QList<QWizard::WizardButton> layout;
  layout << QWizard::Stretch << QWizard::BackButton << QWizard::CustomButton1
  <<  QWizard::CancelButton;
  wizard()->setButtonText(QWizard::CustomButton1, i18n("Select File"));
  wizard()->setOption(QWizard::HaveCustomButton1, false);
  wizard()->setButtonLayout(layout);
  wizard()->button(QWizard::CustomButton1)->setToolTip(i18n("A profile must be selected before selecting a file."));
  m_firstEdit = false;
  m_editAccepted = false;
  m_newProfileCreated  = QString();

  m_wizDlg->m_csvDialog->m_importError = false;
  if (m_wizDlg->m_csvDialog->m_profileName.isEmpty() || m_wizDlg->m_csvDialog->m_profileName == "Add New Profile") {
    wizard()->button(QWizard::CustomButton1)->setEnabled(false);  // disable 'Select file' if no profile selected
  } else {
    wizard()->button(QWizard::CustomButton1)->setEnabled(true);  //  enable 'Select file' when profile selected
  }
  connect(ui->combobox_source, SIGNAL(activated(int)), this, SLOT(slotComboSourceClicked(int)));
  connect(ui->combobox_source->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotLineEditingFinished()));
}

bool IntroPage::validatePage()
{
  m_wizDlg->m_csvDialog->m_firstPass = false;
  if (!m_newProfileCreated.isEmpty()) {
    m_wizDlg->m_csvDialog->createProfile(m_newProfileCreated);
  }
  return true;
}

void IntroPage::slotLineEditingFinished()
{
  if ((ui->combobox_source->currentIndex() == -1) && (m_firstEdit == true)) {
    m_firstEdit = false;
  }
  QString newName = ui->combobox_source->lineEdit()->text();
  if ((newName.isEmpty()) || (newName == m_priorName)) {
    return;
  }
  m_priorName = m_wizDlg->m_csvDialog->m_profileName;
  m_priorIndex = m_index;
  m_wizDlg->m_csvDialog->m_profileName = newName;
  if (m_wizDlg->m_csvDialog->m_fileType == "Banking") {
    m_wizDlg->m_csvDialog->m_priorCsvProfile = m_wizDlg->m_csvDialog->m_profileName;
  } else {
    m_wizDlg->m_csvDialog->m_priorInvProfile = m_wizDlg->m_csvDialog->m_profileName;
  }
  if (ui->combobox_source->currentIndex() < 1) {
    m_action = "add";
    if ((newName == "Add New Profile") || (newName.isEmpty())) {
      return;
    }
  }
  if ((ui->combobox_source->currentIndex() == m_priorIndex) && (m_action != "add")) {  //  Editing current selection.
    int rc = editProfileName(m_priorName, newName);
    if (rc == KMessageBox::No) {
      ui->combobox_source->setCurrentIndex(m_priorIndex);
      return;
    } else {
      ui->combobox_source->setCurrentItem(newName);
    }
  }
  m_index = ui->combobox_source->currentIndex();
  m_priorIndex = m_index;
  if ((m_messageBoxJustCancelled == false) && (m_firstEdit == true) && (m_editAccepted == true)) {
    m_firstEdit = true;
    return;
  }
  m_firstEdit = true;
  int itmIndx = 0;
  itmIndx = addItem(newName);
  if (itmIndx == -1) {//  Already exists.
    m_priorName = newName;
    return;
  }
  //
  //  Adding new profile.
  //
  setField("source", m_index);
  if (m_wizDlg->m_csvDialog->m_profileList.contains(newName)) {
    return;
  }
  if (m_action != "add") {
    editProfileName(m_priorName, newName);
  }
  m_wizDlg->m_csvDialog->m_profileName = newName;
  if (m_wizDlg->m_csvDialog->m_fileType == "Banking") {
    m_wizDlg->m_csvDialog->m_priorCsvProfile = m_wizDlg->m_csvDialog->m_profileName;
  } else {
    m_wizDlg->m_csvDialog->m_priorInvProfile = m_wizDlg->m_csvDialog->m_profileName;
  }
  m_wizDlg->m_csvDialog->m_profileList.append(m_wizDlg->m_csvDialog->m_profileName);
  m_wizDlg->m_csvDialog->createProfile(m_wizDlg->m_csvDialog->m_profileName);
  m_newProfileCreated = m_wizDlg->m_csvDialog->m_profileName;
  m_priorName = m_wizDlg->m_csvDialog->m_profileName;
  m_mapFileType.insert(m_wizDlg->m_csvDialog->m_profileName, m_wizDlg->m_csvDialog->m_fileType);
  m_priorIndex = ui->combobox_source->findText(m_wizDlg->m_csvDialog->m_profileName);
  if (m_priorIndex == -1) {
    ui->combobox_source->addItem(m_wizDlg->m_csvDialog->m_profileName);
  }
  m_priorIndex = ui->combobox_source->findText(m_wizDlg->m_csvDialog->m_profileName);
  ui->combobox_source->setCurrentItem(m_wizDlg->m_csvDialog->m_profileName, false);
  m_action.clear();
}

SeparatorPage::SeparatorPage(QWidget *parent) :
    CSVWizardPage(parent),
    ui(new Ui::SeparatorPage)
{
  ui->setupUi(this);

  m_pageLayout = new QVBoxLayout;
  ui->horizontalLayout->insertLayout(0, m_pageLayout);
}

SeparatorPage::~SeparatorPage()
{
  delete ui;
}

void SeparatorPage::initializePage()
{
  ui->horizontalLayout->setStretch(1, 2);
  ui->horizontalLayout->setStretch(2, 50);
  QSize sizeLow(m_wizDlg->width() - 100, m_wizDlg->height());
  QSize sizeHigh(m_wizDlg->width(), m_wizDlg->height() - 30);
  //  resize the wizard,
  //  depending on DPI setting
  if (QApplication::desktop()->fontInfo().pixelSize() < 20) {
    ui->comboBox_fieldDelimiter->setMaximumWidth(120);
    ui->horizontalLayout->setStretch(0, 2);
    if (m_wizDlg->m_pageInvestment->m_investPageInitialized) {
      sizeLow.setWidth(sizeLow.width() - 200);
      sizeLow.setHeight(sizeLow.height() - 150);
    }
    m_wizDlg->resize(sizeLow);
  } else {
    ui->horizontalLayout->setStretch(0, 20);
    if (m_wizDlg->m_pageInvestment->m_investPageInitialized  || m_wizDlg->m_pageBanking->m_bankingPageInitialized) {
      sizeHigh.setWidth(sizeHigh.width() - 50);
      sizeHigh.setHeight(sizeHigh.height() - 100);
    }
    ui->comboBox_fieldDelimiter->setMaximumWidth(200);
    m_wizDlg->resize(sizeHigh);
  }
  //  ...and need to adjust preview table for the file now loaded
  int x, y;
  y = (QApplication::desktop()->height() - m_wizDlg->m_csvDialog->height()) / 2;
  x = (QApplication::desktop()->width() - m_wizDlg->m_csvDialog->width()) / 2;
  if (x < 0) {
    x = 0;
  }
  m_wizDlg->m_csvDialog->resize(m_wizDlg->m_csvDialog->width(), m_wizDlg->m_csvDialog->height() + 2);
  m_wizDlg->m_csvDialog->move(x, y);

  //  above move() leaves 'shadow' so
  m_wizDlg->m_csvDialog->hide();
  m_wizDlg->m_csvDialog->show();
  //  and ensure wizard is visible
  m_wizDlg->hide();
  m_wizDlg->show();

  QList<QWizard::WizardButton> layout;
  layout << QWizard::Stretch << QWizard::BackButton << QWizard::NextButton << QWizard::CancelButton;
  wizard()->setButtonLayout(layout);
  connect(ui->comboBox_fieldDelimiter, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_csvDialog->m_investProcessing, SLOT(fieldDelimiterChanged()));
  connect(ui->comboBox_fieldDelimiter, SIGNAL(activated(int)), m_wizDlg->m_csvDialog->m_investProcessing, SLOT(fieldDelimiterChanged()));
  if (m_wizDlg->m_investProcessing->m_importCompleted) {
    wizard()->button(QWizard::NextButton)->setEnabled(false);
  }
}

void SeparatorPage::delimiterActivated()
{
  emit completeChanged();
  if ((m_wizDlg->m_csvDialog->m_delimiterError) && (m_wizDlg->m_csvDialog->m_fileType == "Invest")) {
    m_wizDlg->m_investProcessing->fieldDelimiterChanged();
  }
}

bool SeparatorPage::isComplete() const
{
  //
  //  Check for presence of needed columns.
  //  This is not foolproof, but can help detect wrong delimiter choice.
  //
  bool ret1;
  bool ret2;
  bool ret3;
  bool ret = false;

  if (m_wizDlg->m_csvDialog->m_fileType == "Banking") {
    ret1 = ((m_wizDlg->m_csvDialog->m_endColumn > 2) && (!m_wizDlg->m_csvDialog->m_importError));
    ret2 = ((field("dateColumn").toInt() > -1) && (field("payeeColumn").toInt() > -1)  &&
            ((field("amountColumn").toInt() > -1) || ((field("debitColumn").toInt() > -1)  && (field("creditColumn").toInt() > -1))));
    ret3 = m_wizDlg->m_pageBanking->m_bankingPageInitialized;
    ret = (ret1 || (ret2 && ret3));
  } else if (m_wizDlg->m_csvDialog->m_fileType == "Invest") {
    ret1 = (m_wizDlg->m_investProcessing->m_endColumn > 3);
    ret2 = ((field("dateCol").toInt() > -1)  && ((field("amountCol").toInt() > -1) || ((field("quantityCol").toInt() > -1)))  &&
            ((field("symbolCol").toInt() > -1) || (field("securityNameIndex").toInt() > -1)));
    ret3 = m_wizDlg->m_pageInvestment->m_investPageInitialized;
    ret = (ret1 || (ret2 && ret3));
  }
  if (!ret) {
    wizard()->button(QWizard::NextButton)->setToolTip(i18n("Incorrect number or type of fields.  Check the field delimiter."));
  } else {
    wizard()->button(QWizard::NextButton)->setToolTip(QString());
  }
  return ret;
}

bool SeparatorPage::validatePage()
{
  m_wizDlg->m_csvDialog->m_firstPass = false;
  return true;
}

void SeparatorPage::cleanupPage()
{
  //  On completion with error force use of 'Back' button.
  //  ...to allow resetting of 'Skip setup'

  m_wizDlg->m_pageIntro->initializePage();  //  Need to show button(QWizard::CustomButton1) not 'NextButton'
}

int SeparatorPage::nextId() const
{
  int ret;
  if (m_wizDlg->m_csvDialog->m_fileType == "Banking") {
    ret = CSVWizard::Page_Banking;
  } else {
    ret = CSVWizard::Page_Investment;
  }
  return ret;
}

BankingPage::BankingPage(QWidget *parent) :
    CSVWizardPage(parent),
    ui(new Ui::BankingPage),
    m_bankingPageInitialized(false)
{
  ui->setupUi(this);
  m_pageLayout = new QVBoxLayout;
  ui->horizontalLayout->insertLayout(0, m_pageLayout);

  ui->comboBoxBnk_numberCol->setMaxVisibleItems(12);
  ui->comboBoxBnk_dateCol->setMaxVisibleItems(12);
  ui->comboBoxBnk_payeeCol->setMaxVisibleItems(12);
  ui->comboBoxBnk_memoCol->setMaxVisibleItems(12);
  ui->comboBoxBnk_amountCol->setMaxVisibleItems(12);
  ui->comboBoxBnk_creditCol->setMaxVisibleItems(12);
  ui->comboBoxBnk_debitCol->setMaxVisibleItems(12);
  ui->comboBoxBnk_categoryCol->setMaxVisibleItems(12);

  registerField("dateColumn", ui->comboBoxBnk_dateCol, "currentIndex", SIGNAL(currentIndexChanged()));
  registerField("payeeColumn", ui->comboBoxBnk_payeeCol, "currentIndex", SIGNAL(currentIndexChanged()));
  registerField("amountColumn", ui->comboBoxBnk_amountCol, "currentIndex", SIGNAL(currentIndexChanged()));
  registerField("debitColumn", ui->comboBoxBnk_debitCol, "currentIndex", SIGNAL(currentIndexChanged()));
  registerField("creditColumn", ui->comboBoxBnk_creditCol, "currentIndex", SIGNAL(currentIndexChanged()));
  registerField("categoryColumn", ui->comboBoxBnk_categoryCol, "currentIndex", SIGNAL(currentIndexChanged()));

  connect(ui->comboBoxBnk_dateCol, SIGNAL(activated(int)), this, SLOT(slotDateColChanged(int)));
  connect(ui->comboBoxBnk_amountCol, SIGNAL(activated(int)), this, SLOT(slotAmountColChanged(int)));
  connect(ui->comboBoxBnk_payeeCol, SIGNAL(activated(int)), this, SLOT(slotPayeeColChanged(int)));
  connect(ui->comboBoxBnk_debitCol, SIGNAL(activated(int)), this, SLOT(slotDebitColChanged(int)));
  connect(ui->comboBoxBnk_creditCol, SIGNAL(activated(int)), this, SLOT(slotCreditColChanged(int)));
  connect(ui->comboBoxBnk_categoryCol, SIGNAL(activated(int)), this, SLOT(slotCategoryColChanged(int)));
}

BankingPage::~BankingPage()
{
  delete ui;
}

void BankingPage::initializePage()
{
  QSize sizeLow(m_wizDlg->width() - 100, m_wizDlg->height() - 80);
  QSize sizeHigh(m_wizDlg->width() + 100, m_wizDlg->height() + 30);
  //  resize the wizard,
  //  depending on DPI setting
  if (QApplication::desktop()->fontInfo().pixelSize() < 20) {
    m_wizDlg->resize(sizeLow);
  } else {
    m_wizDlg->resize(sizeHigh);
  }

  connect(m_wizDlg->m_pageLinesDate->ui->spinBox_skip, SIGNAL(valueChanged(int)), m_wizDlg->m_csvDialog, SLOT(startLineChanged(int)));
  int index = m_wizDlg->m_pageIntro->ui->combobox_source->currentIndex();
  setField("source", index);
  m_wizDlg->m_csvDialog->m_fileType = "Banking";
  m_bankingPageInitialized = true;  //            Allow checking of columns now.
  m_wizDlg->m_pageLinesDate->ui->spinBox_skipToLast->setMaximum(m_wizDlg->m_csvDialog->fileLastLine());
}

int BankingPage::nextId() const
{
  return CSVWizard::Page_LinesDate;
}

void BankingPage::cleanupPage()
{
  //  Need to keep this or lose settings on backing out
  if (QApplication::desktop()->fontInfo().pixelSize() < 20) {
    m_wizDlg->resize(m_wizDlg->width() - 70, m_wizDlg->height() - 100);
  }
  m_wizDlg->m_pageSeparator->initializePage();
}

bool BankingPage::isComplete() const
{
  bool ret = ((field("dateColumn").toInt() > -1)  && (field("payeeColumn").toInt() > -1)  && ((field("amountColumn").toInt() > -1) || ((field("debitColumn").toInt() > -1)  && (field("creditColumn").toInt() > -1))));
  return ret;
}

void BankingPage::slotDateColChanged(int col)
{
  if (col < 0) return;
  setField("dateColumn", col);
  emit completeChanged();
}

void BankingPage::slotPayeeColChanged(int col)
{
  setField("payeeColumn", col);
  emit completeChanged();
}

void BankingPage::slotDebitColChanged(int col)
{
  setField("debitColumn", col);
  emit completeChanged();
}

void BankingPage::slotCreditColChanged(int col)
{
  setField("creditColumn", col);
  emit completeChanged();
}

void BankingPage::slotAmountColChanged(int col)
{
  setField("amountColumn", col);
  emit completeChanged();
}

void BankingPage::slotCategoryColChanged(int col)
{
  setField("categoryColumn", col);
  emit completeChanged();
}

InvestmentPage::InvestmentPage(QWidget *parent) :
    CSVWizardPage(parent),
    ui(new Ui::InvestmentPage),
    m_investPageInitialized(false)
{
  ui->setupUi(this);

  m_pageLayout = new QVBoxLayout;
  ui->horizontalLayout->insertLayout(0, m_pageLayout);

  registerField("dateCol", ui->comboBoxInv_dateCol, "currentIndex", SIGNAL(currentIndexChanged()));
  registerField("typeCol", ui->comboBoxInv_typeCol, "currentIndex", SIGNAL(currentIndexChanged()));
  registerField("quantityCol", ui->comboBoxInv_quantityCol, "currentIndex", SIGNAL(currentIndexChanged()));
  registerField("priceCol", ui->comboBoxInv_priceCol, "currentIndex", SIGNAL(currentIndexChanged()));
  registerField("amountCol", ui->comboBoxInv_amountCol, "currentIndex", SIGNAL(currentIndexChanged()));

  registerField("symbolCol", ui->comboBoxInv_symbolCol, "currentIndex", SIGNAL(currentIndexChanged()));
  registerField("detailCol", ui->comboBoxInv_detailCol, "currentIndex", SIGNAL(currentIndexChanged()));
  registerField("securityNameIndex", ui->comboBoxInv_securityName, "currentIndex", SIGNAL(currentIndexChanged()));

  connect(ui->comboBoxInv_dateCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDateColChanged(int)));
  connect(ui->comboBoxInv_typeCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotTypeColChanged(int)));
  connect(ui->comboBoxInv_quantityCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotQuantityColChanged(int)));
  connect(ui->comboBoxInv_priceCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotPriceColChanged(int)));
  connect(ui->comboBoxInv_amountCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotAmountColChanged(int)));
  connect(ui->comboBoxInv_symbolCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSymbolColChanged(int)));
  connect(ui->comboBoxInv_detailCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDetailColChanged(int)));

  connect(ui->lineEdit_filter, SIGNAL(returnPressed()), this, SLOT(slotFilterEditingFinished()));
  connect(ui->lineEdit_filter, SIGNAL(editingFinished()), this, SLOT(slotFilterEditingFinished()));
}

InvestmentPage::~InvestmentPage()
{
  delete ui;
}

void InvestmentPage::initializePage()
{
  QSize sizeLow(m_wizDlg->width() + 200, m_wizDlg->height() + 50);
  QSize sizeHigh(m_wizDlg->width() + 255, m_wizDlg->height() + 60);
  //  resize the wizard,
  //  depending on DPI setting
  if (QApplication::desktop()->fontInfo().pixelSize() < 20) {
    m_wizDlg->resize(sizeLow);
  } else {
    m_wizDlg->resize(sizeHigh);
  }

  int index = m_wizDlg->m_pageIntro->ui->combobox_source->currentIndex();
  setField("source", index);
  m_wizDlg->m_csvDialog->m_fileType = "Invest";

  m_investPageInitialized = true;
  connect(m_wizDlg->m_pageLinesDate->ui->spinBox_skip, SIGNAL(valueChanged(int)), m_wizDlg->m_investProcessing, SLOT(startLineChanged(int)));
  wizard()->button(QWizard::NextButton)->setEnabled(false);
  connect(ui->comboBoxInv_securityName, SIGNAL(currentIndexChanged(int)), this, SLOT(slotsecurityNameChanged(int)));
  connect(ui->buttonInv_hideSecurity, SIGNAL(clicked()), m_wizDlg->m_investProcessing, SLOT(hideSecurity()));
  m_wizDlg->m_csvDialog->m_isTableTrimmed = false;
  m_wizDlg->m_csvDialog->m_detailFilter = ui->lineEdit_filter->text();//    Load setting from config file.
}

void InvestmentPage::cleanupPage()
{
  //  Need to keep this or lose settings on backing out
  m_wizDlg->resize(m_wizDlg->width() - 140, m_wizDlg->height());
  m_wizDlg->m_pageSeparator->initializePage();
}

void InvestmentPage::slotDateColChanged(int col)
{
  setField("dateCol", col);
  emit completeChanged();
}

void InvestmentPage::slotTypeColChanged(int col)
{
  setField("typeCol", col);
  emit completeChanged();
}

void InvestmentPage::slotQuantityColChanged(int col)
{
  setField("quantityCol", col);
  emit completeChanged();
}

void InvestmentPage::slotPriceColChanged(int col)
{
  setField("priceCol", col);
  emit completeChanged();
}

void InvestmentPage::slotAmountColChanged(int col)
{
  setField("amountCol", col);
  emit completeChanged();
}

void InvestmentPage::slotSymbolColChanged(int col)
{
  setField("symbolCol", col);
  if (col != -1) {
    setField("securityNameIndex", -1);
    ui->comboBoxInv_securityName->setCurrentIndex(-1);
  }
  emit completeChanged();
}

void InvestmentPage::slotDetailColChanged(int col)
{
  setField("detailCol", col);
  if (col != -1) {
    setField("securityNameIndex", -1);
    ui->comboBoxInv_securityName->setCurrentIndex(-1);
  }
  emit completeChanged();
}

void InvestmentPage::slotsecurityNameChanged(int index)
{
  setField("securityNameIndex", index);
  int symbolCol = ui->comboBoxInv_symbolCol->currentIndex();
  int detailCol = ui->comboBoxInv_detailCol->currentIndex();
  if (index != -1) {  //  There is a security name
    setField("symbolCol", -1);
    setField("detailCol", -1);
    ui->comboBoxInv_symbolCol->setCurrentIndex(-1);
    ui->comboBoxInv_detailCol->setCurrentIndex(-1);
    if ((symbolCol != -1) && (detailCol != -1)) {
      m_wizDlg->m_csvDialog->m_investProcessing->clearColumnType(symbolCol);
      m_wizDlg->m_csvDialog->m_investProcessing->clearColumnType(detailCol);
    }
  }
  emit completeChanged();
}

void InvestmentPage::slotFilterEditingFinished()
{
  m_wizDlg->m_csvDialog->m_detailFilter = ui->lineEdit_filter->text();
}

bool InvestmentPage::isComplete() const
{
  bool ret = (((field("symbolCol").toInt() > -1) && (field("detailCol").toInt() > -1)) || ((field("securityNameIndex").toInt()) > -1)) &&
             (field("dateCol").toInt() > -1) && (field("typeCol").toInt() > -1) &&
             (field("quantityCol").toInt() > -1) && (field("priceCol").toInt() > -1) && (field("amountCol").toInt() > -1);
  return ret;
}

LinesDatePage::LinesDatePage(QWidget *parent) :
    CSVWizardPage(parent),
    ui(new Ui::LinesDatePage),
    m_isColumnSelectionComplete(false)
{
  ui->setupUi(this);

  m_pageLayout = new QVBoxLayout;
  ui->horizontalLayout->insertLayout(0, m_pageLayout);
  m_trailerLines = 0;
}

LinesDatePage::~LinesDatePage()
{
  delete ui;
}

void LinesDatePage::initializePage()
{
  if (QApplication::desktop()->fontInfo().pixelSize() < 20) {
    m_wizDlg->resize(m_wizDlg->width() - 180, m_wizDlg->height() - 100);
  } else {
    m_wizDlg->resize(m_wizDlg->width() - 240, m_wizDlg->height() - 50);
  }

  m_wizDlg->m_csvDialog->markUnwantedRows();
  m_wizDlg->m_csvDialog->m_goBack = false;
  QList<QWizard::WizardButton> layout;
  layout << QWizard::Stretch << QWizard::BackButton <<  QWizard::NextButton <<  QWizard::CancelButton;
  wizard()->setButtonLayout(layout);
  m_isColumnSelectionComplete = true;

  if (m_wizDlg->m_pageIntro->ui->checkBoxSkipSetup->isChecked()) {
    validatePage();
  }
  if (m_wizDlg->m_csvDialog->m_fileType == "Banking") {
    m_wizDlg->m_pageLinesDate->ui->spinBox_skipToLast->setMinimum(m_wizDlg->m_investProcessing->m_startLine);
  } else if (m_wizDlg->m_csvDialog->m_fileType == "Invest") {
    m_wizDlg->m_pageLinesDate->ui->spinBox_skipToLast->setMinimum(m_wizDlg->m_investProcessing->m_startLine);
    m_wizDlg->m_pageLinesDate->ui->spinBox_skipToLast->setValue(m_wizDlg->m_investProcessing->m_endLine);
  }
}

bool LinesDatePage::validatePage()
{
  bool ok;
  QString value;
  QString pattern = QString("[%1(), $]").arg(KGlobal::locale()->currencySymbol());
  //
  //  Ensure numeric columns do contain valid numeric values
  //
  if (m_wizDlg->m_csvDialog->m_fileType == "Banking") {
    for (int row = m_wizDlg->m_csvDialog->startLine() - 1; row < m_wizDlg->m_csvDialog->lastLine(); row++) {
      for (int col = 0; col < m_wizDlg->m_csvDialog->ui->tableWidget->columnCount(); col++) {
        if (m_wizDlg->m_csvDialog->ui->tableWidget->item(row, col) == 0) {  //  Does cell exist?
          break;  //  No.
        }
        if ((m_wizDlg->m_csvDialog->columnType(col) == "amount") || (m_wizDlg->m_csvDialog->columnType(col) == "debit") || (m_wizDlg->m_csvDialog->columnType(col) == "credit")) {
          value = m_wizDlg->m_csvDialog->ui->tableWidget->item(row, col)->text().remove(QRegExp(pattern));
          if (value.isEmpty()) {  //  An empty cell is OK, probably.
            continue;
          }
          value.toDouble(&ok); // Test validity.
          if ((!ok) && (!m_wizDlg->m_csvDialog->m_acceptAllInvalid)) {
            QString str = KGlobal::locale()->currencySymbol();
            int rc = KMessageBox::questionYesNoCancel(this, i18n("<center>An invalid value has been detected in column %1 on row %2.</center>"
                     "Please check that you have selected the correct columns."
                     "<center>You may accept all similar items, or just this one, or cancel.</center>",
                     col + 1, row + 1), i18n("CSV import"),
                     KGuiItem(i18n("Accept All")),
                     KGuiItem(i18n("Accept This")),
                     KGuiItem(i18n("Cancel")));
            switch (rc) {
              case KMessageBox::Yes:  // Accept All
                m_wizDlg->m_csvDialog->m_acceptAllInvalid = true;
                continue;

              case KMessageBox::No:  // Accept This
                m_wizDlg->m_csvDialog->m_acceptAllInvalid = false;
                continue;

              case KMessageBox::Cancel:
                return false;
            }
          }
        }
      }
    }
  } else {  //  "Invest"
    for (int row = m_wizDlg->m_investProcessing->m_startLine - 1; row < m_wizDlg->m_csvDialog->m_investProcessing->m_endLine; row++) {
      for (int col = 0; col < m_wizDlg->m_csvDialog->ui->tableWidget->columnCount(); col++) {
        if ((m_wizDlg->m_investProcessing->columnType(col) == "amount") || (m_wizDlg->m_investProcessing->columnType(col) == "quantity") || (m_wizDlg->m_csvDialog->m_investProcessing->columnType(col) == "price")) {
          if (m_wizDlg->m_csvDialog->ui->tableWidget->item(row, col) == 0) {  //  Does cell exist?
            break;  //  No.
          }
          value = m_wizDlg->m_csvDialog->ui->tableWidget->item(row, col)->text().remove(QRegExp(pattern));
          value = value.remove("--");  //  Possible blank marker.
          if (value.isEmpty()) {  //       An empty cell is OK, probably.
            continue;
          }
          value.toDouble(&ok); // Test validity.
          if ((!ok) && (!m_wizDlg->m_csvDialog->m_acceptAllInvalid)) {
            QString str = KGlobal::locale()->currencySymbol();
            int rc = KMessageBox::questionYesNoCancel(this, i18n("<center>An invalid value has been detected in column %1 on row %2.</center>"
                     "Please check that you have selected the correct columns."
                     "<center>You may accept all similar items, or just this one, or cancel.</center>",
                     col + 1, row + 1), i18n("CSV import"),
                     KGuiItem(i18n("Accept All")),
                     KGuiItem(i18n("Accept This")),
                     KGuiItem(i18n("Cancel")));
            switch (rc) {
              case KMessageBox::Yes:  //  = "Accept All"
                m_wizDlg->m_csvDialog->m_acceptAllInvalid = true;
                continue;

              case KMessageBox::No:  //  "Accept This"
                m_wizDlg->m_csvDialog->m_acceptAllInvalid = false;
                continue;

              case KMessageBox::Cancel:
                return false;
            }
          }
        }
      }
    }
  }

  int symTableRow = -1;
  if ((m_wizDlg->m_csvDialog->m_fileType == "Banking") || (field("symbolCol").toInt() == -1)) {  //  Only check symbols if that field is set, and not Banking.
    if ((m_wizDlg->m_pageIntro->ui->checkBoxSkipSetup->isChecked())) {
      if (m_wizDlg->m_csvDialog->m_importError) {
        wizard()->next();
      } else {
        m_wizDlg->m_pageCompletion->slotImportClicked();
      }
    }
    return true;
  }
  if (m_wizDlg->m_csvDialog->m_investProcessing->m_symbolTableScanned) {
    return true;
  }
  disconnect(m_wizDlg->m_csvDialog->m_symbolTableDlg->m_widget->tableWidget, SIGNAL(cellChanged(int,int)), 0, 0);

  MyMoneyStatement::Security security;
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneySecurity sec;
  QList<MyMoneySecurity> list = file->securityList();

  //  No security name chosen so scan entries...if not already checked,
  //  to save user having to re-edit security names if having to re-import.
  if ((field("securityNameIndex").toInt() == -1)  && (!m_wizDlg->m_csvDialog->m_investProcessing->m_symbolTableScanned)) {
    QString symbl;
    QString securityName;
    for (int row = m_wizDlg->m_csvDialog->m_investProcessing->m_startLine - 1; row < m_wizDlg->m_csvDialog->ui->tableWidget->rowCount(); row++) {
      if (row >= m_wizDlg->m_csvDialog->m_investProcessing->m_endLine) {  //  No need to scan further lines
        break;
      }
      int col = m_wizDlg->m_pageInvestment->ui->comboBoxInv_symbolCol->currentIndex();
      if (m_wizDlg->m_csvDialog->ui->tableWidget->item(row, col) == 0) {  //  This cell does not exist
        continue;
      }
      symbl = m_wizDlg->m_csvDialog->ui->tableWidget->item(row, col)->text().toUpper().trimmed();
      int detail = m_wizDlg->m_pageInvestment->ui->comboBoxInv_detailCol->currentIndex();
      securityName = m_wizDlg->m_csvDialog->ui->tableWidget->item(row, detail)->text().trimmed();
      // Check if we already have the security on file.
      // Just use the symbol for matching, because the security name
      // field is unstandardised and very variable.
      bool exists = false;
      QString name;
      QList<MyMoneySecurity>::ConstIterator it = list.constBegin();
      while (it != list.constEnd()) {
        if (!symbl.isEmpty())  {     //  symbol already exists
          sec = *it;
          name.clear();
          if (symbl.compare(sec.tradingSymbol(), Qt::CaseInsensitive) == 0) {
            exists = true;
            name = sec.name();
            break;
          }
        }
        ++it;
      }
      if (!exists) {
        name = securityName;
      }
      symTableRow ++;
      m_wizDlg->m_csvDialog->m_symbolTableDlg->displayLine(symTableRow, symbl, name, exists);
      m_wizDlg->m_investProcessing->m_symbolsList << symbl;
      m_wizDlg->m_investProcessing->m_map.insert(symbl, name);
    }
    if (symTableRow > -1) {
      m_wizDlg->m_investProcessing->m_symbolTableScanned = true;
      int ret = m_wizDlg->m_csvDialog->m_symbolTableDlg->exec();
      if (ret == QDialog::Rejected) {
        m_wizDlg->m_csvDialog->m_importIsValid = false;
        m_wizDlg->m_csvDialog->m_importError = true;
        return false;
      }
    }
  }
  connect(m_wizDlg->m_csvDialog->m_symbolTableDlg->m_widget->tableWidget,  SIGNAL(itemChanged(QTableWidgetItem*)), m_wizDlg->m_csvDialog->m_symbolTableDlg,  SLOT(slotItemChanged(QTableWidgetItem*)));

  return true;
}

void LinesDatePage::cleanupPage()
{
  if (m_wizDlg->m_csvDialog->m_fileType == "Banking") {
    m_wizDlg->resize(m_wizDlg->width() + 50, m_wizDlg->height() + 20);
    m_wizDlg->m_pageBanking->initializePage();
  } else {
    m_wizDlg->resize(m_wizDlg->width() + 50, m_wizDlg->height() + 20);
    m_wizDlg->m_pageInvestment->initializePage();
  }
}

int LinesDatePage::nextId() const
{
  m_wizDlg->m_csvDialog->m_importError = false;
  m_wizDlg->m_csvDialog->m_accept = false;
  return CSVWizard::Page_Completion;
}

CompletionPage::CompletionPage(QWidget* parent) : CSVWizardPage(parent), ui(new Ui::CompletionPage)
{
  ui->setupUi(this);

  m_pageLayout = new QVBoxLayout;
  ui->horizontalLayout->insertLayout(0, m_pageLayout);
}

CompletionPage::~CompletionPage()
{
  delete ui;
}

void CompletionPage::initializePage()
{
  if (QApplication::desktop()->fontInfo().pixelSize() < 20) {
    m_wizDlg->resize(m_wizDlg->width() - 180, m_wizDlg->height() - 100);
  } else {
    m_wizDlg->resize(m_wizDlg->width() + 90, m_wizDlg->height());
  }
  m_wizDlg->m_csvDialog->m_firstPass = false;  //  Needs to be here when skipping setup.
  QList<QWizard::WizardButton> layout;
  if (m_wizDlg->m_csvDialog->m_importError) {
    layout << QWizard::Stretch << QWizard::BackButton << QWizard::CancelButton;
    wizard()->setButtonLayout(layout);
    return;
  }
  if (!m_wizDlg->m_pageIntro->ui->checkBoxSkipSetup->isChecked()) {
    layout.clear();
    layout << QWizard::Stretch << QWizard::CustomButton3 << QWizard::CustomButton2 << QWizard::BackButton
    <<  QWizard::FinishButton <<  QWizard::CancelButton;
    wizard()->setOption(QWizard::HaveCustomButton2, true);
    wizard()->setButtonText(QWizard::CustomButton2, i18n("Import CSV"));
    wizard()->setOption(QWizard::HaveCustomButton3, false);
    wizard()->setButtonText(QWizard::CustomButton3, i18n("Make QIF File"));
    wizard()->button(QWizard::CustomButton3)->setEnabled(false);
    wizard()->setButtonLayout(layout);
  }
  m_wizDlg->m_csvDialog->m_isTableTrimmed = true;
  if (m_wizDlg->m_pageIntro->ui->checkBoxSkipSetup->isChecked()) {
    m_wizDlg->m_csvDialog->m_detailFilter = m_wizDlg->m_pageInvestment->ui->lineEdit_filter->text();//  Load setting from config file.
    m_wizDlg->m_pageLinesDate->validatePage();  //  Need to validate amounts

    if (!m_wizDlg->m_investProcessing->m_importCompleted) {
      if (m_wizDlg->m_csvDialog->m_importIsValid) {
        slotImportClicked();
      }
    }
  }
  //  use saved value of index to trigger validity test
  QTimer::singleShot(200, m_wizDlg->m_csvDialog, SLOT(decimalSymbolSelected()));
}

void CompletionPage::slotImportValid()
{
  m_wizDlg->m_csvDialog->m_importIsValid = true;
  QList<QWizard::WizardButton> layout;
  if (!m_wizDlg->m_pageIntro->ui->checkBoxSkipSetup->isChecked()) {
    layout << QWizard::Stretch << QWizard:: CustomButton2 << QWizard::BackButton << QWizard::FinishButton << QWizard::CancelButton;
    wizard()->setOption(QWizard::HaveCustomButton2, true);
    wizard()->setButtonText(QWizard::CustomButton2, i18n("Import  CSV"));
    wizard()->setButtonText(QWizard::FinishButton, i18n("Exit"));
    wizard()->setButtonLayout(layout);
  }  else {
    initializePage();
  }
}

void CompletionPage::slotImportClicked()
{
  QList<QWizard::WizardButton> layout;
  if (!m_wizDlg->m_pageIntro->ui->checkBoxSkipSetup->isChecked()) {
    layout << QWizard::Stretch << QWizard::CustomButton3 << QWizard::CustomButton2 << QWizard::BackButton
    <<  QWizard::FinishButton <<  QWizard::CancelButton;
    wizard()->setOption(QWizard::HaveCustomButton2, true);
    wizard()->setButtonText(QWizard::CustomButton2, i18n("Import CSV"));
    wizard()->setOption(QWizard::HaveCustomButton3, true);
    wizard()->setButtonText(QWizard::CustomButton3, i18n("Make QIF File"));
    wizard()->button(QWizard::CustomButton3)->setEnabled(true);
  } else {
    wizard()->next();
    layout.clear();
    layout << QWizard::Stretch << QWizard::BackButton << QWizard::NextButton <<  QWizard::CancelButton;
  }
  wizard()->setButtonLayout(layout);

  m_wizDlg->m_csvDialog->m_isTableTrimmed = true;
  if (m_wizDlg->m_csvDialog->m_fileType == "Banking") {
    emit importBanking();
    setFinalPage(true);
  }  else {
    emit importInvestment();
    setFinalPage(true);
  }
}

void CompletionPage::cleanupPage()
{
  QList<QWizard::WizardButton> layout;
  layout << QWizard::Stretch << QWizard::BackButton << QWizard::NextButton <<  QWizard::CancelButton;
  wizard()->setButtonLayout(layout);
  if (QApplication::desktop()->fontInfo().pixelSize() < 20) {
    //
  } else {
    m_wizDlg->resize(m_wizDlg->width() + 150, m_wizDlg->height());
  }
  m_wizDlg->m_pageLinesDate->initializePage();
}

bool CompletionPage::validatePage()
{
  emit completeChanged();
  return true;
}

