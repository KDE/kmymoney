/*******************************************************************************
*                              investprocessing.cpp
*                              --------------------
* begin                       : Sat Jan 01 2010
* copyright                   : (C) 2010 by Allan Anderson
* email                       : agander93@gmail.com
* copyright                   : (C) 2016 by Łukasz Wojniłowicz
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

#include "investprocessing.h"

// ----------------------------------------------------------------------------
// QT Headers

#include <QScrollBar>
#include <QDesktopWidget>
#include <QCloseEvent>
#include <QLineEdit>

#include <QtCore/QFile>
#include <QFileDialog>
#include <QInputDialog>
#include <QtCore/QTextCodec>
#include <QtCore/QTextStream>
#include <QtCore/QDebug>
#include <QUrl>
#include <QFileDialog>

// ----------------------------------------------------------------------------
// KDE Headers

#include <KSharedConfig>
#include <kmessagebox.h>
#include <KAboutData>
#include <KAboutApplicationDialog>
#include <QVBoxLayout>
#include <QStandardPaths>
#include <QHBoxLayout>
#include <KLocalizedString>
#include <KConfigGroup>
#include <kjobwidgets.h>
#include <kio/job.h>

// ----------------------------------------------------------------------------
// Project Headers

#include "mymoneyfile.h"
#include "kmymoney.h"

#include "convdate.h"
#include "csvutil.h"
#include "csvwizard.h"

#include "mymoneystatement.h"
#include "mymoneystatementreader.h"
#include "mymoneymoney.h"
#include "redefinedlg.h"

#include "ui_introwizardpage.h"
#include "ui_separatorwizardpage.h"
#include "ui_bankingwizardpage.h"
#include "ui_lines-datewizardpage.h"
#include "ui_completionwizardpage.h"
#include "ui_investmentwizardpage.h"
#include "ui_csvwizard.h"
#include "symboltabledlg.h"

InvestProcessing::InvestProcessing()
{
  m_completer = 0;
  m_symbolRow = 0;
  m_securityName.clear();

  csvSplit m_csvSplit;

  m_redefine = new RedefineDlg;
  connect(m_redefine, SIGNAL(changedType(QString)), this, SLOT(changedType(QString)));
}

InvestProcessing::~InvestProcessing()
{
  delete m_symbolTableDlg;
  delete m_completer;
  delete m_redefine;
}

void InvestProcessing::init()
{
  m_symbolTableDlg  = new SymbolTableDlg;
  m_symbolTableDlg->m_investProcessing = this;

  m_brokerBuff.clear();
  m_accountName.clear();

  clearSelectedFlags();

  m_securityName = m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->currentText();

  QLineEdit* securityLineEdit = m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->lineEdit();//krazy:exclude=<qclasses>

  m_completer = new QCompleter(m_securityList, this);
  m_completer->setCaseSensitivity(Qt::CaseInsensitive);
  securityLineEdit->setCompleter(m_completer);
  connect(securityLineEdit, SIGNAL(editingFinished()), this, SLOT(securityNameEdited()));
  connect(this, SIGNAL(statementReady(MyMoneyStatement&)), m_wiz->m_plugin, SLOT(slotGetStatement(MyMoneyStatement&)));

  //  The following string list strings are descriptions of possible investment
  //  activity types.  Each of the lists may also contain alternative descriptions,
  //  added by the user to the resource file, to suit his needs.

  QString text = "Type of operation as in financial statement";
  m_buyList += i18nc("%1", "buy", "%1", text);  //                       some basic entries in case rc file missing
  m_sellList << i18nc("%1", "sell", text)
             << i18nc("%1", "repurchase", text);
  m_divXList += i18nc("%1", "dividend", text);
  m_intIncList << i18nc("%1", "interest", text)
               << i18nc("%1", "income", text);
  m_reinvdivList << i18nc("%1", "reinvest", text)
                 << i18nc("%1", "reinv", text)
                 << i18nc("%1", "re-inv", text);
  m_shrsinList << i18nc("%1", "add", text)
               << i18nc("%1", "stock dividend", text)
               << i18nc("%1", "divd reinv", text)
               << i18nc("%1", "transfer in", text)
               << i18nc("%1", "re-registration in", text)
               << i18nc("%1", "journal entry", text);
  m_removeList += i18nc("%1", "remove", text);
  text = "Brokerage type";
  m_brokerageList << i18nc("%1", "check", text)
                  << i18nc("%1", "payment", text)
                  << i18nc("%1", "bill payment", text)
                  << i18nc("%1", "dividend", text)
                  << i18nc("%1", "interest", text)
                  << i18nc("%1", "qualified div", text)
                  << i18nc("%1", "foreign tax paid", text)
                  << i18nc("%1", "adr mgmt fee", text);
}

void InvestProcessing::changedType(const QString& newType)
{
  if ((newType == "buy") || (newType == "sell") || (newType == "divx") || (newType == "reinvdiv") ||
      (newType == "shrsin") || (newType == "shrsout")  || (newType == "intinc")) {
    m_trInvestData.type = newType;
  }
}

void InvestProcessing::slotFileDialogClicked()
{
  if ((m_wiz->m_fileType != "Invest") || (m_wiz->m_profileName.isEmpty()))
    return;
  m_wiz->m_skipSetup = m_wiz->m_pageIntro->ui->checkBoxSkipSetup->isChecked();
  m_columnTypeList.clear();//  Needs to be here in case user selects new profile after cancelling prior one.clearColumnTypes()
  m_wiz->m_inFileName.clear();
  m_url.clear();
  m_wiz->m_columnsNotSet = true;  //  Don't check columns until they've been selected.
  m_symbolTableScanned = false;
  m_listSecurities.clear();
  m_wiz->m_accept = false;
  m_importNow = false;//                        Avoid attempting date formatting on headers
  m_wiz->m_acceptAllInvalid = false;  //  Don't accept further invalid values.

  readSettings();

  if (m_invPath.isEmpty()) {
    m_invPath = QDir::home().absolutePath();
  }

  if(m_invPath.startsWith("~/"))  //expand Linux home directory
    m_invPath.replace(0, 1, QDir::home().absolutePath());

  QPointer<QFileDialog> dialog = new QFileDialog(m_wiz->m_wizard, QString(), m_invPath,
                                                 i18n("*.csv *.PRN *.txt | CSV Files\n *|All files"));
  dialog->setOption(QFileDialog::DontUseNativeDialog, true);  //otherwise we cannot add custom QComboBox
  dialog->setFileMode(QFileDialog::ExistingFile);
  QLabel* label = new QLabel(i18n("Encoding"));
  dialog->layout()->addWidget(label);
  //    Add encoding selection to FileDialog
  m_wiz->m_comboBoxEncode = new QComboBox();
  m_wiz->setCodecList(m_wiz->m_codecs);
  m_wiz->m_comboBoxEncode->setCurrentIndex(m_wiz->m_encodeIndex);
  connect(m_wiz->m_comboBoxEncode, SIGNAL(activated(int)), this, SLOT(encodingChanged(int)));
  dialog->layout()->addWidget(m_wiz->m_comboBoxEncode);
  if(dialog->exec() == QDialog::Accepted) {
    m_url = dialog->selectedUrls().first();
  }
  delete dialog;

  if (m_url.isEmpty()) {
    return;
  } else if (m_url.isLocalFile()) {
    m_wiz->m_inFileName = m_url.toLocalFile();
  } else {
    m_wiz->m_inFileName = QDir::tempPath();
    if(!m_wiz->m_inFileName.endsWith(QDir::separator()))
      m_wiz->m_inFileName += QDir::separator();
    m_wiz->m_inFileName += m_url.fileName();
    qDebug() << "Source:" << m_url.toDisplayString() << "Destination:" << m_wiz->m_inFileName;
    KIO::FileCopyJob *job = KIO::file_copy(m_url, QUrl::fromUserInput(m_wiz->m_inFileName), -1,KIO::Overwrite);
    KJobWidgets::setWindow(job, m_wiz->m_wizard);
    job->exec();
    if (job->error()) {
      KMessageBox::detailedError(0, i18n("Error while loading file '%1'.", m_url.toDisplayString()),
                                 job->errorString(),
                                 i18n("File access error"));
      return;
    }
  }

  if (m_wiz->m_inFileName.isEmpty())
    return;

  m_accountName.clear();
  m_redefine->clearAccountName();
  m_brokerageItems = false;

  m_wiz->readFile(m_wiz->m_inFileName);
  m_wiz->displayLines(m_wiz->m_lineList, m_wiz->m_parse);
  enableInputs();

  calculateFee();

  m_wiz->updateWindowSize();
  m_wiz->m_wizard->next();  //go to separator page

  if (m_wiz->m_skipSetup)
    for (int i = 0; i < 4; i++) //programmaticaly go through separator-, investment-, linesdate-, completionpage
      m_wiz->m_wizard->next();
}

void InvestProcessing::saveSettings()
{
  if ((m_wiz->m_fileType != "Invest") || (m_wiz->m_inFileName.isEmpty())) {  // don't save if no file loaded
    return;
  }
  QString str;
  KSharedConfigPtr config = KSharedConfig::openConfig(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + "csvimporterrc");

  KConfigGroup mainGroup(config, "MainWindow");
  mainGroup.writeEntry("Height", m_wiz->height());
  mainGroup.writeEntry("Width", m_wiz->width());
  mainGroup.config()->sync();

  KConfigGroup bankProfilesGroup(config, "BankProfiles");

  bankProfilesGroup.writeEntry("BankNames", m_wiz->m_profileList);
  int indx = m_wiz->m_pageIntro->ui->combobox_source->findText(m_wiz->m_priorInvProfile, Qt::MatchExactly);
  if (indx > 0) {
    str = m_wiz->m_priorInvProfile;
  }
  bankProfilesGroup.writeEntry("PriorInvProfile", str);
  bankProfilesGroup.config()->sync();

  for (int i = 0; i < m_wiz->m_profileList.count(); i++) {
    if (m_wiz->m_profileList[i] != m_wiz->m_profileName) {
      continue;
    }

    QString txt = "Profiles-" + m_wiz->m_profileList[i];

    KConfigGroup profilesGroup(config, txt);
    profilesGroup.writeEntry("FileType", m_wiz->m_fileType);
    profilesGroup.writeEntry("DateFormat", m_wiz->m_pageLinesDate->ui->comboBox_dateFormat->currentIndex());
    profilesGroup.writeEntry("FieldDelimiter", m_wiz->m_pageSeparator->ui->comboBox_fieldDelimiter->currentIndex());
    profilesGroup.writeEntry("DecimalSymbol", m_wiz->m_pageCompletion->ui->comboBox_decimalSymbol->currentIndex());
    profilesGroup.writeEntry("ProfileName", m_wiz->m_profileName);
    profilesGroup.writeEntry("PriceFraction", m_wiz->m_pageInvestment->ui->comboBoxInv_priceFraction->currentIndex());
    profilesGroup.writeEntry("StartLine", m_wiz->m_pageLinesDate->ui->spinBox_skip->value() - 1);
    profilesGroup.writeEntry("SecurityName", m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->currentIndex());
    profilesGroup.writeEntry("TrailerLines", m_wiz->m_pageLinesDate->m_trailerLines);
    //    The strings in these resource file lists may be edited,
    //    or expanded in the file by the user, to suit his needs.

    profilesGroup.writeEntry("ShrsinParam", m_shrsinList);
    profilesGroup.writeEntry("DivXParam", m_divXList);
    profilesGroup.writeEntry("IntIncParam", m_intIncList);
    profilesGroup.writeEntry("BrokerageParam", m_brokerageList);
    profilesGroup.writeEntry("ReinvdivParam", m_reinvdivList);
    profilesGroup.writeEntry("BuyParam", m_buyList);
    profilesGroup.writeEntry("SellParam", m_sellList);
    profilesGroup.writeEntry("RemoveParam", m_removeList);

    str = m_wiz->m_pageInvestment->ui->lineEdit_filter->text();
    if (str.endsWith(' ')) {
      str.append('#');  //  Terminate trailing blank
    }
    profilesGroup.writeEntry("Filter", str);
    m_invPath = m_wiz->m_inFileName;
    int posn = m_invPath.lastIndexOf("/");
    m_invPath.truncate(posn + 1);   //           keep last "/"
    QString pth = "~/" + invPath().section('/', 3);
    profilesGroup.writeEntry("InvDirectory", pth);
    profilesGroup.writeEntry("Encoding", m_wiz->m_encodeIndex);
    profilesGroup.writeEntry("DateCol", m_wiz->m_pageInvestment->ui->comboBoxInv_dateCol->currentIndex());
    profilesGroup.writeEntry("PayeeCol", m_wiz->m_pageInvestment->ui->comboBoxInv_typeCol->currentIndex());

    QList<int> list = m_wiz->m_memoColList;
    posn = 0;
    if ((posn = list.indexOf(-1)) > -1) {
      list.removeOne(-1);
    }
    profilesGroup.writeEntry("MemoCol", list);
    profilesGroup.writeEntry("QuantityCol", m_wiz->m_pageInvestment->ui->comboBoxInv_quantityCol->currentIndex());
    profilesGroup.writeEntry("AmountCol", m_wiz->m_pageInvestment->ui->comboBoxInv_amountCol->currentIndex());
    profilesGroup.writeEntry("PriceCol", m_wiz->m_pageInvestment->ui->comboBoxInv_priceCol->currentIndex());
    profilesGroup.writeEntry("FeeCol", m_wiz->m_pageInvestment->ui->comboBoxInv_feeCol->currentIndex());
    profilesGroup.writeEntry("SymbolCol", m_wiz->m_pageInvestment->ui->comboBoxInv_symbolCol->currentIndex());
    profilesGroup.writeEntry("NameCol", m_wiz->m_pageInvestment->ui->comboBoxInv_nameCol->currentIndex());
    profilesGroup.writeEntry("FeeIsPercentage", int(m_wiz->m_pageInvestment->ui->checkBoxInv_feeIsPercentage->isChecked()));
    profilesGroup.writeEntry("FeeRate", m_wiz->m_pageInvestment->ui->lineEdit_feeRate->text());
    profilesGroup.writeEntry("MinFee", m_wiz->m_pageInvestment->ui->lineEdit_minFee->text());
    profilesGroup.config()->sync();

    KConfigGroup securitiesGroup(config, "Securities");
    securitiesGroup.writeEntry("SecurityNameList", securityList());
    securitiesGroup.config()->sync();
  }
}

void InvestProcessing::enableInputs()
{
  m_wiz->m_pageInvestment->ui->comboBoxInv_amountCol->setEnabled(true);
  m_wiz->m_pageInvestment->ui->comboBoxInv_dateCol->setEnabled(true);
  m_wiz->m_pageInvestment->ui->lineEdit_feeRate->setEnabled(true);
  m_wiz->m_pageInvestment->ui->lineEdit_minFee->setEnabled(true);
  m_wiz->m_pageInvestment->ui->comboBoxInv_feeCol->setEnabled(true);
  m_wiz->m_pageSeparator->ui->comboBox_fieldDelimiter->setEnabled(true);
  m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol->setEnabled(true);
  m_wiz->m_pageInvestment->ui->comboBoxInv_priceCol->setEnabled(true);
  m_wiz->m_pageInvestment->ui->comboBoxInv_priceFraction->setEnabled(true);
  m_wiz->m_pageInvestment->ui->comboBoxInv_quantityCol->setEnabled(true);
  m_wiz->m_pageInvestment->ui->comboBoxInv_typeCol->setEnabled(true);
  m_wiz->m_pageInvestment->ui->button_clear->setEnabled(true);
  m_wiz->m_pageInvestment->ui->buttonInv_clearFee->setEnabled(true);
  m_wiz->m_pageLinesDate->ui->spinBox_skipToLast->setEnabled(true);
  m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->setEnabled(true);
  m_wiz->m_pageInvestment->ui->checkBoxInv_feeIsPercentage->setEnabled(true);
}

void InvestProcessing::clearFeesSelected()
{
  int i;
  if (!m_wiz->m_pageInvestment->ui->lineEdit_feeRate->text().isEmpty() && !(m_feeColumn<0)) //delete fee colum, but only if it was generated
  {
    m_wiz->m_maxColumnCount--;
    m_wiz->m_endColumn = m_wiz->m_maxColumnCount;
    m_wiz->ui->tableWidget->setColumnCount(m_wiz->m_maxColumnCount);
    i=m_wiz->m_pageInvestment->ui->comboBoxInv_feeCol->currentIndex();
    m_wiz->m_pageInvestment->ui->comboBoxInv_feeCol->setCurrentIndex(-1);
    m_feeSelected = false;
    m_wiz->m_pageInvestment->ui->comboBoxInv_feeCol->removeItem(i);
  }
  m_wiz->m_pageInvestment->ui->comboBoxInv_feeCol->setEnabled(true);
  m_wiz->m_pageInvestment->ui->lineEdit_feeRate->setEnabled(true);
  m_wiz->m_pageInvestment->ui->lineEdit_minFee->setEnabled(true);
  m_wiz->m_pageInvestment->ui->checkBoxInv_feeIsPercentage->setEnabled(true);
  m_wiz->m_pageInvestment->ui->buttonInv_calculateFee->setEnabled(false);
  m_wiz->m_pageInvestment->ui->comboBoxInv_feeCol->setCurrentIndex(-1);
  m_wiz->m_pageInvestment->ui->lineEdit_feeRate->clear();
  m_wiz->m_pageInvestment->ui->lineEdit_minFee->clear();
  m_wiz->m_pageInvestment->ui->checkBoxInv_feeIsPercentage->setChecked(false);
  m_wiz->updateWindowSize();
}

void InvestProcessing::clearColumnsSelected()
{
  clearSelectedFlags();
  clearColumnNumbers();
  clearComboBoxText();
  m_wiz->m_memoColList.clear();
  for (int i = 0; i < m_columnTypeList.count(); i++)
    m_columnTypeList[i].clear();
}

void InvestProcessing::clearSelectedFlags()
{
  m_amountSelected = false;
  m_dateSelected = false;
  m_priceSelected = false;
  m_quantitySelected = false;
  m_memoSelected = false;
  m_typeSelected = false;
  m_feeSelected = false;
  m_nameSelected = false;
  m_symbolSelected = false;
}

void InvestProcessing::clearColumnNumbers()
{
  m_wiz->m_pageInvestment->ui->comboBoxInv_amountCol->setCurrentIndex(-1);
  m_wiz->m_pageInvestment->ui->comboBoxInv_dateCol->setCurrentIndex(-1);
  m_wiz->m_pageInvestment->ui->comboBoxInv_priceCol->setCurrentIndex(-1);
  m_wiz->m_pageInvestment->ui->comboBoxInv_quantityCol->setCurrentIndex(-1);
  m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol->setCurrentIndex(-1);
  m_wiz->m_pageInvestment->ui->comboBoxInv_typeCol->setCurrentIndex(-1);
  m_wiz->m_pageInvestment->ui->comboBoxInv_nameCol->setCurrentIndex(-1);
  m_wiz->m_pageInvestment->ui->comboBoxInv_symbolCol->setCurrentIndex(-1);
  m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->setCurrentIndex(-1);
}

void InvestProcessing::clearComboBoxText()
{
  for (int i = 0; i < m_wiz->m_maxColumnCount; i++)
    m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol->setItemText(i, QString().setNum(i + 1));
}

void InvestProcessing::encodingChanged(int index)
{
  m_wiz->m_encodeIndex = index;
}

void InvestProcessing::dateColumnSelected(int col)
{
  QString type = "date";
  m_wiz->m_dateColumn = col;
  if (col < 0) {      //                              it is unset
    return;
  }
  // A new column has been selected for this field so clear old one
  if ((m_columnTypeList[m_wiz->m_dateColumn] == type)  && (m_wiz->m_dateColumn != col)) {
    m_columnTypeList[m_wiz->m_dateColumn].clear();
  }
  int ret = validateNewColumn(col, type);

  if (ret == KMessageBox::Ok) {
    m_wiz->m_pageInvestment->ui->comboBoxInv_dateCol->setCurrentIndex(col);  // accept new column
    m_dateSelected = true;
    if (m_wiz->m_dateColumn != -1) {
//          if a previous date column is detected, but in a different column...
      if ((m_columnTypeList[m_wiz->m_dateColumn] == type)  && (m_wiz->m_dateColumn != col)) {
        m_columnTypeList[m_wiz->m_dateColumn].clear();//   clear it
      }
    }
    m_wiz->m_dateColumn = col;
    m_columnTypeList[m_wiz->m_dateColumn] = type;
    return;
  }
  if (ret == KMessageBox::No) {
    m_wiz->m_pageInvestment->ui->comboBoxInv_dateCol->setCurrentIndex(-1);
  }
}

int InvestProcessing::validateNewColumn(const int& col, const QString& type)
{
  //  Prevent check of column settings until user sees them.
  //  Then check if selection is in range
  if ((col < 0) || (col >= m_wiz->m_endColumn) || (m_wiz->m_columnsNotSet)) {
    return KMessageBox::No;
  }
  //  selection was in range
  //  ...but does it clash?
  if ((!m_columnTypeList[col].isEmpty())  && (m_columnTypeList[col] != type) && (m_wiz->m_pageInvestment->m_investPageInitialized)) {  // column is already in use
    KMessageBox::information(0, i18n("The '<b>%1</b>' field already has this column selected. <center>Please reselect both entries as necessary.</center>", m_columnTypeList[col]));
    resetComboBox(m_columnTypeList[col], col);  //      clash,  so reset ..
    resetComboBox(type, col);  //                   ... both comboboxes
    m_previousType.clear();
    m_columnTypeList[col].clear();
    feeInputsChanged();
    return KMessageBox::Cancel;
  }
  //                                                is this type already in use
  for (int i = 0; i < m_wiz->m_maxColumnCount; i++) {  //  check each column
    if (m_columnTypeList[i] == type) {  //          this type already in use
      m_columnTypeList[i].clear();//                ...so clear it
    }//  end this col

  }// end all columns checked                       type not in use
  m_columnTypeList[col] = type;  //                 accept new type
  m_previousType = type;
  return KMessageBox::Ok; //                        accept new type
}

void InvestProcessing::feeInputsChanged()
{
  m_wiz->m_pageInvestment->ui->buttonInv_calculateFee->setEnabled(false);
  if(m_wiz->m_pageInvestment->ui->lineEdit_feeRate->text().isEmpty())
    {
      m_wiz->m_pageInvestment->ui->comboBoxInv_feeCol->setEnabled(true);
      m_wiz->m_pageInvestment->ui->checkBoxInv_feeIsPercentage->setEnabled(true);
      m_wiz->m_pageInvestment->ui->lineEdit_minFee->setEnabled(false);
    }
  else
    {
      m_wiz->m_pageInvestment->ui->comboBoxInv_feeCol->setEnabled(false);
      m_wiz->m_pageInvestment->ui->checkBoxInv_feeIsPercentage->setEnabled(false);
      m_wiz->m_pageInvestment->ui->checkBoxInv_feeIsPercentage->setChecked(true);
      m_wiz->m_pageInvestment->ui->lineEdit_minFee->setEnabled(true);
      if (m_amountColumn!=-1)
        m_wiz->m_pageInvestment->ui->buttonInv_calculateFee->setEnabled(true);
    }
}

void InvestProcessing::feeColumnSelected(int col)
{
  QString type = "fee";
  m_feeColumn = col;
  if (col < 0) {      //                              it is unset
    return;
  }
  // A new column has been selected for this field so clear old one
  if ((m_columnTypeList[m_feeColumn] == type)  && (m_feeColumn != col)) {
    m_columnTypeList[m_feeColumn].clear();
  }
  int ret = validateNewColumn(col, type);
  if (ret == KMessageBox::Ok) {
    m_wiz->m_pageInvestment->ui->comboBoxInv_feeCol->setCurrentIndex(col);  // accept new column
    m_feeSelected = true;
    if (m_feeColumn != -1) {
//          if a previous fee column is detected, but in a different column...
      if ((m_columnTypeList[m_feeColumn] == type)  && (m_feeColumn != col)) {
        m_columnTypeList[m_feeColumn].clear();//    ..clear it
      }
    }
    m_feeColumn = col;
    m_columnTypeList[m_feeColumn] = type;
  }
  if (ret == KMessageBox::No) {
    m_wiz->m_pageInvestment->ui->comboBoxInv_feeCol->setCurrentIndex(-1);
  }
  feeInputsChanged();
}

void InvestProcessing::typeColumnSelected(int col)
{
  QString type = "type";
  m_typeColumn = col;
  if (col < 0) {      //                              it is unset
    return;
  }
// A new column has been selected for this field so clear old one
  if ((m_columnTypeList[m_typeColumn] == type)  && (m_typeColumn != col)) {
    m_columnTypeList[m_typeColumn].clear();
  }
  int ret = validateNewColumn(col, type);

  if (ret == KMessageBox::Ok) {
    m_wiz->m_pageInvestment->ui->comboBoxInv_typeCol->setCurrentIndex(col);  // accept new column
    m_typeSelected = true;
    if (m_typeColumn != -1) {
//          if a previous type column is detected, but in a different column...
      if ((m_columnTypeList[m_typeColumn] == type)  && (m_typeColumn != col)) {
        m_columnTypeList[m_typeColumn].clear();//   ...clear it
      }
    }
    m_typeColumn = col;
    m_columnTypeList[m_typeColumn] = type;
    return;
  }
  if (ret == KMessageBox::No) {
    m_wiz->m_pageInvestment->ui->comboBoxInv_typeCol->setCurrentIndex(-1);
  }
}

void InvestProcessing::memoColumnSelected(int col)
{
  //  Prevent check of column settings until user sees them.
  if ((col < 0) || (col >= m_wiz->m_endColumn) || (m_wiz->m_columnsNotSet)) {      //  out of range so...
    m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol->setCurrentIndex(-1);  //  ..clear selection
    return;
  }
  QString type = "memo";
  if (m_columnTypeList[col].isEmpty()) {      //      accept new  entry
    m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol->setItemText(col, QString().setNum(col + 1) + '*');
    m_columnTypeList[col] = type;
    m_wiz->m_memoColumn = col;
    if (m_wiz->m_memoColList.contains(col)) {
      //  Restore the '*' as column might have been cleared.
      m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol->setItemText(col, QString().setNum(col + 1) + '*');
    } else {
      m_wiz->m_memoColList << col;
    }
    m_memoSelected = true;
    return;
  } else if (m_columnTypeList[col] == type) {  //     nothing changed
    return;
  } else if ((m_columnTypeList[col] == "type") || (m_columnTypeList[col] == "name")) {
    int rc = KMessageBox::Yes;
    if (m_wiz->m_pageInvestment->isVisible()) {
      rc = KMessageBox::questionYesNo(0, i18n("<center>The '<b>%1</b>' field already has this column selected.</center>"
                                              "<center>If you wish to copy that data to the memo field, click 'Yes'.</center>",
                                              m_columnTypeList[col]));
    }
    if (rc == KMessageBox::Yes) {
      m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol->setItemText(col, QString().setNum(col + 1) + '*');
      if (!m_wiz->m_memoColList.contains(col))
        m_wiz->m_memoColList << col;
      m_memoSelected = true;
    } else if (m_wiz->m_memoColList.contains(col)) {
      m_wiz->m_memoColList.removeOne(col);
      m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol->setItemText(col, QString().setNum(col + 1));
      if (m_wiz->m_memoColList.count() == 0 && m_wiz->m_memoColumn == -1)
        m_memoSelected = false;
    }
    return;
  } else {  //  m_columnTypeList[col] != "type"or "name"
    //                                           clashes with prior selection
    m_memoSelected = false;//                    clear incorrect selection
    KMessageBox::information(0, i18n("The '<b>%1</b>' field already has this column selected. <center>Please reselect both entries as necessary.</center>", m_columnTypeList[col]));
    m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol->setCurrentIndex(-1);
    resetComboBox(m_columnTypeList[col], col);  //      clash,  so reset ..
    resetComboBox(type, col);  //                   ... both comboboxes
    m_previousType.clear();
    m_columnTypeList[col].clear();
    m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol->setItemText(col, QString().setNum(col + 1));   //  reset the '*'
    if (m_wiz->m_memoColList.contains(m_wiz->m_memoColumn))
      m_wiz->m_memoColList.removeOne(m_wiz->m_memoColumn);
    m_wiz->m_memoColumn = -1;
  }
}

void InvestProcessing::quantityColumnSelected(int col)
{
  QString type = "quantity";
  m_quantityColumn = col;
  if (col < 0) {      //                              it is unset
    return;
  }
  m_redefine->setQuantityColumn(col);
// A new column has been selected for this field so clear old one
  if ((m_columnTypeList[m_quantityColumn] == type)  && (m_quantityColumn != col)) {
    m_columnTypeList[m_quantityColumn].clear();
  }
  int ret = validateNewColumn(col, type);
  if (ret == KMessageBox::Ok) {
    m_wiz->m_pageInvestment->ui->comboBoxInv_quantityCol->setCurrentIndex(col);  // accept new column
    m_quantitySelected = true;
    if (m_quantityColumn != -1) {
      //  if a previous quantity column is detected, but in a different column...
      if ((m_columnTypeList[m_quantityColumn] == type)  && (m_quantityColumn != col)) {
        m_columnTypeList[m_quantityColumn].clear();// ...clear it
      }
    }
    m_quantityColumn = col;
    m_columnTypeList[m_quantityColumn] = type;
    return;
  }
  if (ret == KMessageBox::No) {
    m_wiz->m_pageInvestment->ui->comboBoxInv_quantityCol->setCurrentIndex(-1);
  }
}

void InvestProcessing::priceColumnSelected(int col)
{
  QString type = "price";
  m_priceColumn = col;
  if (col < 0) {      //                              it is unset
    return;
  }

  // A new column has been selected for this field so clear old one
  if ((m_columnTypeList[m_priceColumn] == type)  && (m_priceColumn != col)) {
    m_columnTypeList[m_priceColumn].clear();
  }
  int ret = validateNewColumn(col, type);
  if (ret == KMessageBox::Ok) {
    m_wiz->m_pageInvestment->ui->comboBoxInv_priceCol->setCurrentIndex(col);  // accept new column
    m_priceSelected = true;
    if (m_priceColumn != -1) {
      //          if a previous price column is detected, but in a different column...
      if ((m_columnTypeList[m_priceColumn] == type)  && (m_priceColumn != col)) {
        m_columnTypeList[m_priceColumn].clear();//  ...clear it
      }
    }
    m_priceColumn = col;
    m_columnTypeList[m_priceColumn] = type;
    m_redefine->setPriceColumn(col);
    return;
  }
  if (ret == KMessageBox::No) {
    m_wiz->m_pageInvestment->ui->comboBoxInv_priceCol->setCurrentIndex(-1);
  }
}

void InvestProcessing::amountColumnSelected(int col)
{
  QString type = "amount";
  m_amountColumn = col;
  if (col < 0) {      //                              it is unset
    return;
  }
  m_redefine->setAmountColumn(col);
  // A new column has been selected for this field so clear old one
  if ((m_columnTypeList[m_amountColumn] == type)  && (m_amountColumn != col)) {
    m_columnTypeList[m_amountColumn].clear();
  }
  int ret = validateNewColumn(col, type);
  if (ret == KMessageBox::Ok) {
    m_wiz->m_pageInvestment->ui->comboBoxInv_amountCol->setCurrentIndex(col);  // accept new column
    m_amountSelected = true;
    if (m_amountColumn != -1) {
      //          if a previous amount column is detected, but in a different column...
      if ((m_columnTypeList[m_amountColumn] == type)  && (m_amountColumn != col)) {
        m_columnTypeList[m_amountColumn].clear();// ...clear it
      }
    }
    m_amountColumn = col;
    m_columnTypeList[m_amountColumn] = type;
  }
  if (ret == KMessageBox::No) {
    m_wiz->m_pageInvestment->ui->comboBoxInv_amountCol->setCurrentIndex(-1);
  }
  feeInputsChanged();
}

void InvestProcessing::symbolColumnSelected(int col)
{
  QString type = "symbol";
  m_symbolColumn = col;
  if (col < 0) {
    //  it is not set so remove any prior settings
    int indx = m_columnTypeList.indexOf(type);
    m_symbolSelected = false;
    if (indx > -1) {
      m_columnTypeList[indx].clear();
    }
    return;
  }
  m_redefine->setSymbolColumn(col);
  // A new column has been selected for this field so clear old one
  if ((m_columnTypeList[m_symbolColumn] == type)  && (m_symbolColumn != col)) {
    m_columnTypeList[m_symbolColumn].clear();
  }
  int ret = validateNewColumn(col, type);
  if (ret == KMessageBox::Ok) {
    m_wiz->m_pageInvestment->ui->comboBoxInv_symbolCol->setCurrentIndex(col);  // accept new column
    m_symbolSelected = true;
    if (m_symbolColumn != -1) {
      //          if a previous symbol column is detected, but in a different column...
      if ((m_columnTypeList[m_symbolColumn] == type)  && (m_symbolColumn != col)) {
        m_columnTypeList[m_symbolColumn].clear();// ...clear it
      }
    }
    m_symbolColumn = col;
    m_columnTypeList[m_symbolColumn] = type;
    return;
  }
  if (ret == KMessageBox::No) {
    m_wiz->m_pageInvestment->ui->comboBoxInv_symbolCol->setCurrentIndex(-1);
  }
}

void InvestProcessing::nameColumnSelected(int col)
{
  QString type = "name";
  m_nameColumn = col;
  if (col < 0) {
    //  it is not set so remove any prior settings
    int indx = m_columnTypeList.indexOf(type);
    m_nameSelected = false;
    if (indx > -1) {
      m_columnTypeList[indx].clear();
    }
    return;
  }
  m_redefine->setNameColumn(col);
  // A new column has been selected for this field so clear old one
  if ((m_columnTypeList[m_nameColumn] == type)  && (m_nameColumn != col)) {
    m_columnTypeList[m_nameColumn].clear();
  }
  int ret = validateNewColumn(col, type);
  if (ret == KMessageBox::Ok) {
    m_wiz->m_pageInvestment->ui->comboBoxInv_nameCol->setCurrentIndex(col);  // accept new column
    m_nameSelected = true;
    if (m_nameColumn != -1) {
      //          if a previous name column is detected, but in a different column...
      if ((m_columnTypeList[m_nameSelected] == type)  && (m_nameColumn != col)) {
        m_columnTypeList[m_nameColumn].clear();// ...clear it
      }
    }
    m_nameColumn = col;
    m_columnTypeList[m_nameColumn] = type;
    return;
  }
  if (ret == KMessageBox::No) {
    m_wiz->m_pageInvestment->ui->comboBoxInv_nameCol->setCurrentIndex(-1);
  }
}

void InvestProcessing::feeIsPercentageCheckBoxClicked(bool checked)
{
  m_feeIsPercentage = checked;
}

void InvestProcessing::createStatement()
{
  if (!m_wiz->m_importNow)
    return;

  m_brokerBuff.clear();
  m_outBuffer = "!Type:Invst\n";
  MyMoneyStatement st = MyMoneyStatement();
  MyMoneyStatement stBrokerage = MyMoneyStatement();
  calculateFee();
  m_wiz->createMemoField(m_columnTypeList);

  reloadUISettings();
  for (int line = m_wiz->m_startLine - 1; line < m_wiz->m_endLine; line++) {
    m_columnList = m_wiz->m_parse->parseLine(m_wiz->m_lineList[line]); // split line into fields
    m_redefine->setColumnList(m_columnList);
    int ret = processInvestLine(m_inBuffer); // parse fields
    if (ret == KMessageBox::Ok) {
      if (m_brokerage)
        investCsvImport(stBrokerage);  //              add non-investment transaction to Brokerage statement
      else
        investCsvImport(st);  //                       add investment transaction to statement
    } else {
      m_wiz->m_importNow = false;
      m_wiz->m_wizard->back();  //               have another try at the import
      break;
    }
  }

  if (!m_wiz->m_importNow)
    return;

  emit statementReady(st);  // investment statement ready
  if (m_brokerageItems)
    emit statementReady(stBrokerage);  //   brokerage statement ready
  m_wiz->m_importNow = false;
  if (!m_symbolTableScanned)
    m_listSecurities.clear();
}

int InvestProcessing::processInvestLine(const QString& inBuffer)
{
  QString newTxt;
  //                                      validate all columns
  int neededFieldsCount = 0;//            ensure essential fields are present

  QString memo;
  QString payee;
  QString txt;
  QString type;

  m_trInvestData.memo.clear();//          initialise in case not overwritten by new data
  m_trInvestData.price = 0;
  m_trInvestData.quantity = 0;
  m_trInvestData.amount = 0;
  m_trInvestData.fee = 0;
  m_trInvestData.payee.clear();
  m_trInvestData.security.clear();
  m_trInvestData.brokerageAccnt.clear();
  m_trInvestData.type.clear();
  m_trInvestData.date = QDate();

  MyMoneyMoney zero = MyMoneyMoney();

  m_brokerage = false;
  memo.clear();

  if (m_columnList.count() < m_wiz->m_endColumn) {
    if (!m_wiz->m_accept) {
      QString row = QString::number(m_wiz->m_row);
      int ret = KMessageBox::questionYesNoCancel(0, i18n("<center>Row number %1 does not have the expected number of columns.</center>"
                "<center>This might not be a problem, but it may be a header line.</center>"
                "<center>You may accept all similar items, or just this one, or cancel.</center>",
                row), i18n("CSV import"),
                KGuiItem(i18n("Accept All")),
                KGuiItem(i18n("Accept This")),
                KGuiItem(i18n("Cancel")));
      if (ret == KMessageBox::Cancel) {
        return ret;
      }
      if (ret == KMessageBox::Yes) {
        m_wiz->m_accept = true;
      }
    }
  }

  for (int i = 0; i < m_columnList.count(); i++) {
    //  Use actual column count for this line instead of m_endColumn, which could be greater.
    if (m_columnTypeList[i] == "date") {      //                    Date Col
      ++neededFieldsCount;
      txt = m_columnList[i];
      txt = txt.remove('"');
      QDate dat = m_wiz->m_convertDate->convertDate(txt);
      if (dat == QDate()) {
        KMessageBox::sorry(0, i18n("<center>An invalid date has been detected during import.</center>"
                                   "<center><b>'%1'</b></center>"
                                   "Please check that you have set the correct date format,\n"
                                   "<center>and start and end lines.</center>"
                                   , txt), i18n("CSV import"));
        m_wiz->m_importError = true;
        return KMessageBox::Cancel;
      }
      QString qifDate = dat.toString(m_wiz->m_dateFormats[m_wiz->m_dateFormatIndex]);
      m_tempBuffer = 'D' + qifDate + '\n';
      m_trInvestData.date = dat;
    }

    else if (m_columnTypeList[i] == "type") {      //               Type Col
      type = m_columnList[i];
      m_redefine->setTypeColumn(i);
      QString str = m_columnList[i].trimmed();
      if (str.isEmpty()) {     //                                No Type specified...
        QString txt = m_nameFilter;//             ...but may be one buried in 'name' col. See if there is a filter
        if (!txt.isEmpty()) {     //                             Filter present
          int lngth = m_columnList[m_nameColumn].indexOf(txt);
          if (lngth > -1) {     //                               Position of filter.
            lngth = lngth + txt.length();//                      Length of name.
            QString tmp = m_columnList[m_nameColumn].remove('"');
            tmp = tmp.remove(0, lngth).toLower();  //            Remove all but new type.
            type = tmp;
            m_columnList[i] = type;
          }
        }
      } else {
        m_trInvestData.type = str.remove('"');  //               There is a type.
      }
      ++neededFieldsCount;
      int ret = processActionType(type);
      if (ret == KMessageBox::Cancel) {
        return KMessageBox::Cancel;
      }

      if (m_brokerage) {     //                                  Brokerage
        QStringList::const_iterator it;

        QString payee = type.toLower();
        QString typ = m_nameFilter;
        if (!typ.isEmpty()) {
          int lngth = m_columnList[m_payeeColumn].indexOf(typ);
          if (lngth > -1) {     //                               Found buried type.
            lngth = lngth + typ.length();
            QString tmp = m_columnList[m_payeeColumn];
            tmp = tmp.remove(0, lngth).toLower();
            payee = tmp;
          }
        }
        //
        //  Was brokerage but we might now have genuine investment type.
        //
        for (it = m_brokerageList.constBegin(); it != m_brokerageList.constEnd(); ++it) {      //Brokerage
          if ((payee).contains(*it, Qt::CaseInsensitive)) {
            if (payee.contains("reinv"))  {
              m_trInvestData.type = "reinvdiv";
              m_brokerage = true;
              m_csvSplit.m_strCategoryName = payee;
              break;
            } else if (payee.contains("div"))  {
              m_trInvestData.type = "divx";
              m_brokerage = true;
              m_csvSplit.m_strCategoryName = payee;
              break;
            } else if (payee.contains("interest"))  {
              m_trInvestData.type = "intinc";
              m_brokerage = true;
              m_csvSplit.m_strCategoryName = payee;
              break;
            } else {
              m_trInvestData.type = '0';//  No, so still brokerage.

            }
            m_tempBuffer += 'L' + str + '\n';
            m_tempBuffer += 'P' + m_trInvestData.payee + '\n';
          }
        }//  end of brokerage list 'for loop'
        QString tmp = m_csvSplit.m_strCategoryName;
        m_csvSplit.m_strCategoryName = payee;
      }//  end of brokerage
    }//  end of type col

    else if (m_columnTypeList[i] == "memo") {      //         could be more than one
      txt = m_columnList[i];
      if (!memo.isEmpty()) {
        memo += '\n';//                                       separator for multiple memos
      }
      memo += txt;//                                          next memo
    }//end of memo field

    else if (m_columnTypeList[i] == "quantity") {      //           Quantity Col
      ++neededFieldsCount;
      txt = m_columnList[i].remove('-');  //  Remove unwanted -ve sign in quantity.
      newTxt = m_wiz->m_parse->possiblyReplaceSymbol(txt);
      m_trInvestData.quantity = MyMoneyMoney(newTxt);
      m_tempBuffer += 'Q' + newTxt + '\n';
    }

    else if (m_columnTypeList[i] == "price") {      //              Price Col
      ++neededFieldsCount;
      txt = m_wiz->m_pageInvestment->ui->comboBoxInv_priceFraction->currentText(); //fraction
      txt = txt.replace(m_wiz->m_decimalSymbol, QLocale().decimalPoint());
      MyMoneyMoney fraction = MyMoneyMoney(txt);
      txt = m_columnList[i].remove('"');  //                     price
      newTxt = m_wiz->m_parse->possiblyReplaceSymbol(txt);
      MyMoneyMoney price = MyMoneyMoney(newTxt);
      price = price * fraction;
      double val = price.toDouble();
      newTxt.setNum(val, 'f', 6);
      m_trInvestData.price = price;
      m_tempBuffer +=  'I' + newTxt + '\n';//                 price column
    }

    else if (m_columnTypeList[i] == "amount") {
      ++neededFieldsCount;
      txt = m_columnList[i];
      txt = txt.remove('"');
      if (txt.contains(')')) {
        txt = '-' + txt.remove(QRegExp("[()]"));   //            Mark as -ve
      }
      newTxt = m_wiz->m_parse->possiblyReplaceSymbol(txt);
      MyMoneyMoney amount = MyMoneyMoney(newTxt);
      m_trInvestData.amount = amount;
      m_csvSplit.m_amount = newTxt;
      m_tempBuffer +=  'T' + newTxt + '\n';//                 amount column
    }

    else if (m_columnTypeList[i] == "fee") {      //                Fee Col
      txt = m_columnList[i];
      txt = txt.remove('"');
      if (txt.contains(')')) {
        txt = '-' + txt.remove(QRegExp("[()]"));   //            Mark as -ve
      }
      newTxt = m_wiz->m_parse->possiblyReplaceSymbol(txt);
      MyMoneyMoney fee = MyMoneyMoney(newTxt);
      if (m_feeIsPercentage && fee.toDouble() > 0.00 &&
          m_wiz->m_pageInvestment->ui->lineEdit_feeRate->text().isEmpty()) {      //   fee is percent
        txt = m_columnList[m_amountColumn];
        txt = txt.remove('"');
        if (txt.contains(')')) {
          txt = '-' +  txt.remove(QRegExp("[()]"));   //            Mark as -ve
        }
        newTxt = m_wiz->m_parse->possiblyReplaceSymbol(txt);
        MyMoneyMoney amount = MyMoneyMoney(newTxt);
        fee *= amount / MyMoneyMoney(100) ;//               as percentage
      }
      fee.abs();
      m_trInvestData.fee =  fee;
      txt.setNum(fee.toDouble(), 'f', 4);
      newTxt = m_wiz->m_parse->possiblyReplaceSymbol(txt);
      m_tempBuffer +=  'O' + newTxt + '\n';//                  fee amount
    }

    else if (m_columnTypeList[i] == "symbol") { //                Symbol Col
      txt = m_columnList[i];
      QString name;
      if (m_symbolColumn == -1) {
        return KMessageBox::Cancel;
      }
      QString symbol = m_columnList[m_symbolColumn].toUpper().trimmed();
      if (!symbol.isEmpty()) {
        name = m_map.value(symbol);
        m_columnList[i] = symbol;
      } else {
        name = m_columnList[m_nameColumn].trimmed();
      }

      m_trInvestData.symbol = symbol;
      m_trInvestData.security = name;
    }

    else if (m_columnTypeList[i] == "name") { //                Name Col
      QString str = m_nameFilter;
      QString name;
      QString symbol;
      txt = m_columnList[i];
      if (m_symbolTableDlg->m_widget->tableWidget->item(m_symbolRow, 2) != 0) {   //  If this item exists...
        m_trInvestData.security = m_symbolTableDlg->m_widget->tableWidget->item(m_symbolRow++, 2)->text() ;  //  Get 'edited' name.
      }
      QStringList list;
      if (!m_nameFilter.isEmpty()) {    //          If filter exists...
        list = txt.split(m_nameFilter);  //      ...split the name
      } else {
        list << txt;
      }
      m_columnList[m_nameColumn] = list[0].trimmed();

      if (m_symbolColumn > -1)
        symbol = m_columnList[m_symbolColumn].toUpper().trimmed();
      if (!symbol.isEmpty()) {
        name = m_map.value(symbol);
      } else {
        name = m_columnList[m_nameColumn];
      }

      if (list.count() > 1) {
        m_columnList[m_typeColumn] = list[1];//               This is the 'type' we found.
        if ((m_symbolColumn > -1) && (!m_columnList[m_symbolColumn].trimmed().isEmpty()) && (!m_brokerage)) {    //  If there is a symbol & not brokerage...
          if (m_trInvestData.type.isEmpty()) {  //            If no investment type already...
            m_trInvestData.type = list[1];//                  ...this is investment type.
          }
        } else {
          m_csvSplit.m_strCategoryName = list[1];  //         ...else use as the category.
        }
      }
      if (!txt.isEmpty()) {
        int index = txt.indexOf(str);
        if (index > -1) {
          int lngth = str.length();
          txt = txt.remove(index, lngth).toLower();  //         If there is filter, drop the 'type' from name...
        } else {
          txt = txt.toLower();
        }
      }
      m_trInvestData.payee = txt;//                           ... and use rest as payee.
    }
  }   //end of col loop

  m_redefine->setInBuffer(inBuffer);
  if (m_trInvestData.type != "0") {       //                       Don't need to do this check on checking items.
    int ret = (m_redefine->checkValid(m_trInvestData.type, i18n("The quantity, price and amount parameters in the\ncurrent transaction do not match with the action type.\nPlease select another action type\n")));
    if (ret == KMessageBox::Cancel) {
      return ret;
    }
  }
  //
  //  A brokerage type could have been changed in m_redefine->checkValid() above, so no longer brokerage.
  //
  if ((m_trInvestData.type == "buy") || (m_trInvestData.type == "sell") ||
      (m_trInvestData.type == "divx") || (m_trInvestData.type == "intinc")) {
    m_trInvestData.brokerageAccnt = m_redefine->accountName();
    m_tempBuffer +=  "L[" + m_redefine->accountName() + ']' + '\n';
    m_brokerage = false;
  } else if ((m_trInvestData.type == "reinvdiv") || (m_trInvestData.type == "shrsin") || (m_trInvestData.type == "shrsout")) {
    m_brokerage = false;
  }

  if (m_brokerage) {     //                                        brokerage items
    if (m_brokerBuff.isEmpty()) {      //                          start building data
      m_brokerBuff = "!Account\n";
      m_brokerBuff += 'N' + m_redefine->accountName() + '\n';
      m_brokerBuff += "TBank\n^\n";
      m_brokerBuff += "!Type:Bank\n";
    }
    m_trInvestData.brokerageAccnt = m_redefine->accountName();
    m_brokerBuff += m_tempBuffer;
    if (!memo.isEmpty())
      m_brokerBuff += 'M' + memo + '\n';
    m_brokerBuff += "^\n";
    m_brokerBuff = m_brokerBuff.remove('"');
  } else {//                                                  non-brokerage items
    m_tempBuffer += 'N' + m_trInvestData.type + '\n';
    m_outBuffer += m_tempBuffer;
    m_trInvestData.memo = memo;
    if ((m_trInvestData.security.isEmpty()) && (!m_securityName.isEmpty())) {
      m_trInvestData.security = m_securityName;
    }
    m_outBuffer = m_outBuffer + 'Y' + m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->currentText() + '\n';

    if (!memo.isEmpty()) {
      m_outBuffer = m_outBuffer + 'M' + memo + '\n';
    }
    m_outBuffer += "^\n";
    m_outBuffer = m_outBuffer.remove('"');
  }
  if (neededFieldsCount > 3) {
    return KMessageBox::Ok;
  } else {
    KMessageBox::sorry(0, i18n("<center>The columns selected are invalid.\n</center>"
                               "There must an amount or quantity fields, symbol or security name, plus date and type field."
                               "<center>You possibly need to check the start and end line settings, or reset 'Skip setup'.</center>"),
                       i18n("CSV import"));
    return KMessageBox::Cancel;
  }
  return KMessageBox::Ok;
}

int InvestProcessing::processActionType(QString& type)
{
  QStringList::const_iterator it;
  QString memo;
  QString payee;
  QStringList typesList;
  typesList << "buy" << "sell" << "div" << "reinv" << "shrsin" << "shrsout" << "intinc";

  if (m_buyList.isEmpty()) {
    KMessageBox::information(0, i18n("<center>buyList of transaction types was not found.</center>"
                                     "<center>Check existence of correct resource file - 'csvimporterrc'.</center>"));
    return KMessageBox::Cancel;
  }
  bool typeFound = false;
  bool brokerFound = false;

  for (it = typesList.constBegin(); it != typesList.constEnd(); ++it) {    //  Check for possible invest type.
    if (type.contains(*it, Qt::CaseInsensitive)) {
      typeFound = true;
    }
  }
  if (!typeFound) {
    for (it = m_brokerageList.constBegin(); it != m_brokerageList.constEnd(); ++it) {    //  If not, check for Brokerage type.
      if (type.contains(*it, Qt::CaseInsensitive)) {
        brokerFound = true;
      }
    }
  }
  if ((brokerFound) || (type.isEmpty()))  {      //                No investment type, but may still be...
    m_brokerage = true;//                                     ...but assume these are non-investment items, until later.
    m_tempBuffer +=  "L[" + m_redefine->accountName() + ']' + '\n';

    if (m_payeeColumn < 0) {
      m_payeeColumn = columnNumber(i18n("<center>For a brokerage item, enter the column</center>"
                                        "<center>containing the Payee or Name:</center>")) - 1;//payee column
    }
    if (m_payeeColumn == 0) {
      KMessageBox::sorry(0, i18n("An invalid column was entered.\n"
                                 "Must be between 1 and %1.", m_wiz->m_endColumn), i18n("CSV import"));
      return KMessageBox::Cancel;
    } else if (m_payeeColumn == -1) {
      return KMessageBox::Cancel;
    }
    if (m_nameColumn > -1) {
      m_columnTypeList[m_nameColumn] = "name";
      m_trInvestData.type = '0';
      m_csvSplit.m_strCategoryName = m_columnList[m_payeeColumn];
      return KMessageBox::Ok;
    } else if (m_securityName.isEmpty()) {
      KMessageBox::information(0, i18n("<center>No Name field specified</center>"
                                       "<center>and no security name supplied.</center>"
                                       "<center>(Please check the parameters given)</center>"));
      return KMessageBox::Cancel;
    }
  }
  //
  //  If not brokerage, look for genuine investment type.
  //
  for (it = m_shrsinList.constBegin(); it != m_shrsinList.constEnd(); ++it) {    //       Shrsin
    if (type.contains(*it, Qt::CaseInsensitive)) {
      type = "shrsin";
      m_trInvestData.type = "shrsin";
      return KMessageBox::Ok;
    }
  }
  //                            Needs to be before DivX
  //          because of "ReInvestorContract Buy         : ReInvested Units"
  for (it = m_reinvdivList.constBegin(); it != m_reinvdivList.constEnd(); ++it) {    //   Reinvdiv

    QString txt = (*it);
    if (type.contains(*it, Qt::CaseInsensitive)) {
      type = "reinvdiv";
      m_trInvestData.type = (type);
      return KMessageBox::Ok;
    }
  }

  //                            Needs to be after Reinvdiv
  for (it = m_divXList.constBegin(); it != m_divXList.constEnd(); ++it) {      //         DivX
    if (type.contains(*it, Qt::CaseInsensitive)) {
      type = "divx";
      m_trInvestData.type = type;
      m_csvSplit.m_strCategoryName = "dividend";
      return KMessageBox::Ok;
    }
  }

  for (it = m_buyList.constBegin(); it != m_buyList.constEnd(); ++it) {     //            Buy
    if (type.contains(*it, Qt::CaseInsensitive)) {
      type = "buy";
      m_trInvestData.type = type;
      m_csvSplit.m_strCategoryName.clear();
      return KMessageBox::Ok;
    }
  }

  for (it = m_sellList.constBegin(); it != m_sellList.constEnd(); ++it) {     //          Sell
    if (type.contains(*it, Qt::CaseInsensitive)) {
      type = "sell";
      m_trInvestData.type = type;
      m_csvSplit.m_strCategoryName.clear();
      return KMessageBox::Ok;
    }
  }

  for (it = m_removeList.constBegin(); it != m_removeList.constEnd(); ++it) {     //      shrsout
    if (type.contains(*it, Qt::CaseInsensitive)) {
      type = "shrsout";
      m_trInvestData.type = type;
      return KMessageBox::Ok;
    }
  }

  for (it = m_intIncList.constBegin(); it != m_intIncList.constEnd(); ++it) {     //      intinc
    if (type.contains(*it, Qt::CaseInsensitive)) {
      type = "intinc";
      m_trInvestData.type = type;
      m_csvSplit.m_strCategoryName = "interest";
      return KMessageBox::Ok;
    }
  }
  //   no valid type found
  m_redefine->setInBuffer(m_inBuffer);  //                      Ask user to choose valid type.
  int ret = m_redefine->suspectType(i18n("<center>The transaction below has an unrecognised type or action.</center>"
                                         "<center>Please select an appropriate entry, if available.</center>"
                                         "<center>Otherwise, click Cancel to abort.</center>"));
  //   remember the selection for further transactions
  if (!type.isEmpty() && !m_trInvestData.type.isEmpty() && type.compare(m_trInvestData.type,Qt::CaseInsensitive) != 0) {
      if (m_trInvestData.type == "shrsin") {
        m_shrsinList << type;
      } else if (m_trInvestData.type == "reinvdiv") {
        m_reinvdivList << type;
      } else if (m_trInvestData.type == "divx") {
        m_divXList << type;
      } else if (m_trInvestData.type == "buy") {
        m_buyList << type;
      } else if (m_trInvestData.type == "sell") {
        m_sellList << type;
      } else if (m_trInvestData.type == "shrsout") {
        m_removeList << type;
      } else if (m_trInvestData.type == "intinc") {
        m_intIncList << type;
      }
  }
  return ret;
}//   end of Type Col

void InvestProcessing::investCsvImport(MyMoneyStatement& st)
{
  MyMoneyStatement::Transaction::EAction convType;
  convertType(m_trInvestData.type, convType);
  MyMoneyStatement::Split s1;
  MyMoneyStatement::Transaction tr;
  QString tmp;
  QString payee = m_trInvestData.payee;//                       extractLine('P')
  //
  // Process the securities
  //

  QList<MyMoneyStatement::Security>::const_iterator it_s = m_listSecurities.constBegin();
  while (it_s != m_listSecurities.constEnd()) {
    st.m_listSecurities << (*it_s);
    ++it_s;
  }

  // Process transaction data

  if (m_brokerage) {
    m_brokerageItems = true;
    st.m_eType = MyMoneyStatement::etCheckings;
  } else
    st.m_eType = MyMoneyStatement::etInvestment;
  tr.m_datePosted = m_trInvestData.date;
  if (!m_trInvestData.date.isValid()) {
    int rc = KMessageBox::warningContinueCancel(0, i18n("The date entry \"%1\" read from the file cannot be interpreted through the current date format setting of \"%2.\"\n\n"
             "Pressing \'Continue\' will assign today's date to the transaction. Pressing \'Cancel\'' will abort the import operation. You can then restart the import and select a different date format.",
             m_trInvestData.date.toString(m_wiz->m_dateFormats[m_wiz->m_dateFormatIndex]),
             m_wiz->m_dateFormats[m_wiz->m_dateFormatIndex]), i18n("Invalid date format"));
    switch (rc) {
      case KMessageBox::Continue:
        tr.m_datePosted = (QDate::currentDate());
        break;
      case KMessageBox::Cancel:
        m_importNow = false;//                             Don't process statement
        st = MyMoneyStatement();
        return;
    }
  }
  tr.m_strMemo = m_trInvestData.memo;
  s1.m_strMemo = tr.m_strMemo;
  tr.m_eAction = convType;
  tr.m_amount = m_trInvestData.amount;
  s1.m_amount = tr.m_amount;
  MyMoneyStatement::Split s2 = s1;
  s2.m_amount = MyMoneyMoney(-s1.m_amount);
  tr.m_strInterestCategory = m_csvSplit.m_strCategoryName;
  tr.m_strSecurity = m_trInvestData.security;
  tr.m_strSymbol = m_trInvestData.symbol;

  s2.m_accountId = m_wiz->m_csvUtil->checkCategory(m_csvSplit.m_strCategoryName, s1.m_amount, s2.m_amount);
  tr.m_strBrokerageAccount = m_trInvestData.brokerageAccnt;
  if ((tr.m_eAction == (MyMoneyStatement::Transaction::eaCashDividend)) ||
      (tr.m_eAction == (MyMoneyStatement::Transaction::eaSell)) ||
      (tr.m_eAction == (MyMoneyStatement::Transaction::eaInterest))) {
    /*
     *  need to deduct fees here
     */
    tr.m_amount = tr.m_amount - m_trInvestData.fee.abs();
  }

  else if (tr.m_eAction == (MyMoneyStatement::Transaction::eaBuy)) {
      if (tr.m_amount.isPositive())
          tr.m_amount = -tr.m_amount; //if broker doesn't use minus sings for buy transactions, set it manually here
      tr.m_amount = tr.m_amount - m_trInvestData.fee.abs();
  }

  else if (tr.m_eAction == (MyMoneyStatement::Transaction::eaNone)) {
    tr.m_listSplits += s2;
  }

  tr.m_shares = m_trInvestData.quantity;//                 extractLine('T'));
  if (!payee.isEmpty()) {
    tr.m_strPayee = m_trInvestData.payee;
  }

  tr.m_price = m_trInvestData.price;
  tr.m_fees = m_trInvestData.fee;

  // Add the transaction to the statement

  st.m_listTransactions += tr;
  QList<MyMoneyStatement>   statements;
  if ((st.m_listTransactions.count()) > 0) {
    statements += st;//          this not used
    qDebug("Statement with %d transactions ready", st.m_listTransactions.count());
  }
  m_wiz->m_importError = false;  //  Clear error as this import was OK
  // Import the statements
  return;
}

void InvestProcessing::convertType(const QString& type, MyMoneyStatement::Transaction::EAction& convType)
{
  if (type == "buy")
    convType = MyMoneyStatement::Transaction::eaBuy;
  else if (type == "sell")
    convType = MyMoneyStatement::Transaction::eaSell;
  else if (type == "divx")
    convType = MyMoneyStatement::Transaction::eaCashDividend;
  else if (type == "reinvdiv")
    convType = MyMoneyStatement::Transaction::eaReinvestDividend;
  else if (type == "shrsin")
    convType = MyMoneyStatement::Transaction::eaShrsin;
  else if (type == "shrsout")
    convType = MyMoneyStatement::Transaction::eaShrsout;
  else if (type == "intinc")
    convType = MyMoneyStatement::Transaction::eaInterest;
  else convType = MyMoneyStatement::Transaction::eaNone;
  return;
}

void InvestProcessing::slotImportClicked()
{
  m_wiz->m_importError = false;
  if (m_wiz->m_fileType != "Invest") {
    return;
  }

  if (m_wiz->m_decimalSymbol.isEmpty()) {
    KMessageBox::sorry(0, i18n("<center>Please select the decimal symbol used in your file.\n</center>"), i18n("Investment import"));
    m_wiz->m_importError = true;
    return;
  }

  m_securityName = m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->currentText();
  if (m_securityName.isEmpty()) {
    m_securityName = m_symbolTableDlg->m_securityName;
  } else if (m_securityName.isEmpty()) {
    m_securityName = m_trInvestData.security;
  }

  if ((m_securityName.isEmpty()) && (m_symbolColumn < 1) && (m_nameColumn < 1)) {
    KMessageBox::sorry(0, i18n("<center>Please enter a name or symbol for the security.\n</center>"), i18n("CSV import"));
    m_wiz->m_importError = true;
    return;
  }

  bool securitySelected = true;
  if (!m_securityList.contains(m_securityName)) {
    m_securityList << m_securityName;
  }

  m_dateSelected = (m_wiz->m_pageInvestment->ui->comboBoxInv_dateCol->currentIndex() >= 0);
  m_typeSelected = (m_wiz->m_pageInvestment->ui->comboBoxInv_typeCol->currentIndex() >= 0);
  m_priceSelected = (m_wiz->m_pageInvestment->ui->comboBoxInv_priceCol->currentIndex() >= 0);
  m_quantitySelected = (m_wiz->m_pageInvestment->ui->comboBoxInv_quantityCol->currentIndex() >= 0);
  m_amountSelected = (m_wiz->m_pageInvestment->ui->comboBoxInv_amountCol->currentIndex() >= 0);

  if (m_dateSelected && m_typeSelected && securitySelected && m_quantitySelected && m_priceSelected && m_amountSelected) {
    m_importNow = true;

    //  all necessary data is present

    m_wiz->m_endLine = m_wiz->m_pageLinesDate->ui->spinBox_skipToLast->value();
    int skp = m_wiz->m_pageLinesDate->ui->spinBox_skip->value(); //         skip all headers
    if (skp > m_wiz->m_endLine) {
      KMessageBox::sorry(0, i18n("<center>The start line is greater than the end line.\n</center>"
                                 "<center>Please correct your settings.</center>"), i18n("CSV import"));
      m_wiz->m_importError = true;
      return;
    }

    createStatement();
    m_wiz->markUnwantedRows();
  } else {
    KMessageBox::information(0, i18n("The Security Name, and Date and Type columns are needed.<center>Also, the Price, Quantity and Amount columns.</center><center>Please try again.</center>"));
    m_wiz->m_importError = true;
    return;
  }
  m_importNow = false;
}

void InvestProcessing::saveAs()
{
  if (m_wiz->m_fileType == "Invest") {
    QStringList outFile = m_wiz->m_inFileName .split('.');
    const QString &name = QString((outFile.isEmpty() ? "InvestProcessing" : outFile[0]) + ".qif");

    QString outFileName = QFileDialog::getSaveFileName(0, i18n("Save QIF"), name, QString::fromLatin1("*.qif | %1").arg(i18n("QIF Files")));
    QFile oFile(outFileName);
    oFile.open(QIODevice::WriteOnly);
    QTextStream out(&oFile);
    out << m_outBuffer;//                 output investments to qif file
    out << m_brokerBuff;//                ...also broker type items
    oFile.close();
  }
}

int InvestProcessing::columnNumber(const QString& column)
{
  bool ok;
  static int ret;
  ret = QInputDialog::getInt(0, i18n("Brokerage Item"), column, 0, 1, m_wiz->m_endColumn, 1, &ok);
  if (ok && ret > 0)
    return ret;
  return 0;
}

QString InvestProcessing::accountName(const QString& aName)
{
  bool ok;
  static QString accntName;
  accntName = QInputDialog::getText(0, i18n("Parameters"), aName, QLineEdit::Normal, QString(), &ok);
  if (ok && !accntName.isEmpty())
    return accntName;
  else return "";
}

void InvestProcessing::readSettings()
{
  KSharedConfigPtr config = KSharedConfig::openConfig(QStandardPaths::locate(QStandardPaths::ConfigLocation, "csvimporterrc"));
  KConfigGroup securitiesGroup(config, "Securities");
  m_securityList.clear();
  m_securityList = securitiesGroup.readEntry("SecurityNameList", QStringList());
  for (int i = 0; i < m_wiz->m_profileList.count(); i++) {
    if (m_wiz->m_profileList[i] != m_wiz->m_profileName)
      continue;
    KConfigGroup profilesGroup(config, "Profiles-" + m_wiz->m_profileList[i]);

    m_invPath = profilesGroup.readEntry("InvDirectory", QString());

    QStringList list = profilesGroup.readEntry("BuyParam", QStringList());
    if (!list.isEmpty())
      m_buyList = list;
    list = profilesGroup.readEntry("ShrsinParam", QStringList());
    if (!list.isEmpty())
      m_shrsinList = list;
    list = profilesGroup.readEntry("DivXParam", QStringList());
    if (!list.isEmpty())
      m_divXList = list;
    list = profilesGroup.readEntry("IntIncParam", QStringList());
    if (!list.isEmpty())
      m_intIncList = list;
    list = profilesGroup.readEntry("BrokerageParam", QStringList());
    if (!list.isEmpty())
      m_brokerageList = list;
    list = profilesGroup.readEntry("ReinvdivParam", QStringList());
    if (!list.isEmpty())
      m_reinvdivList = list;
    list = profilesGroup.readEntry("SellParam", QStringList());
    if (!list.isEmpty())
      m_sellList = list;
    list = profilesGroup.readEntry("RemoveParam", QStringList());
    if (!list.isEmpty())
      m_removeList = list;

    m_securityNameIndex = profilesGroup.readEntry("SecurityName", -1);
    m_nameFilter = profilesGroup.readEntry("Filter", QString());
    if (m_nameFilter.endsWith('#'))      //  Terminates a trailing blank
      m_nameFilter.chop(1);

    m_priceFraction = profilesGroup.readEntry("PriceFraction", 0);
    m_symbolColumn = profilesGroup.readEntry("SymbolCol", -1);
    m_wiz->m_dateColumn = profilesGroup.readEntry("DateCol", -1);
    m_payeeColumn = profilesGroup.readEntry("PayeeCol", -1);  //use for type col.
    m_priceColumn = profilesGroup.readEntry("PriceCol", -1);
    m_quantityColumn = profilesGroup.readEntry("QuantityCol", -1);
    m_amountColumn = profilesGroup.readEntry("AmountCol", -1);
    m_nameColumn = profilesGroup.readEntry("NameCol", -1);
    m_feeIsPercentage = profilesGroup.readEntry("FeeIsPercentage", 0);
    m_feeRate = profilesGroup.readEntry("FeeRate", QString());
    m_minFee = profilesGroup.readEntry("MinFee", QString());
    m_feeColumn = profilesGroup.readEntry("FeeCol", -1);
    m_symbolColumn = profilesGroup.readEntry("SymbolCol", -1);

    m_wiz->m_memoColList = profilesGroup.readEntry("MemoCol", QList<int>());
    m_wiz->m_dateFormatIndex = profilesGroup.readEntry("DateFormat", -1);
    m_wiz->m_textDelimiterIndex = profilesGroup.readEntry("TextDelimiter", 0);
    m_wiz->m_fieldDelimiterIndex = profilesGroup.readEntry("FieldDelimiter", -1);
    m_wiz->m_decimalSymbolIndex = profilesGroup.readEntry("DecimalSymbol", -1);

    if (m_wiz->m_decimalSymbolIndex == -1) { // if no decimal symbol in config, then get one from locale settings
      if (QLocale().decimalPoint() == '.')
        m_wiz->m_decimalSymbolIndex = 0;
      else
        m_wiz->m_decimalSymbolIndex = 1;
    }
    if (m_wiz->m_decimalSymbolIndex == 0)
      m_wiz->m_ThousandsSeparatorIndex = 1;
    else
      m_wiz->m_ThousandsSeparatorIndex = 0;

    m_wiz->m_parse->setDecimalSymbolIndex(m_wiz->m_decimalSymbolIndex);
    m_wiz->m_parse->setDecimalSymbol(m_wiz->m_decimalSymbolIndex);
    m_wiz->m_parse->setThousandsSeparatorIndex(m_wiz->m_decimalSymbolIndex);
    m_wiz->m_parse->setThousandsSeparator(m_wiz->m_decimalSymbolIndex);
    m_wiz->m_decimalSymbol = m_wiz->m_parse->decimalSymbol(m_wiz->m_decimalSymbolIndex);

    m_wiz->m_parse->setFieldDelimiterIndex(m_wiz->m_fieldDelimiterIndex);
    m_wiz->m_parse->setTextDelimiterIndex(m_wiz->m_textDelimiterIndex);
    m_wiz->m_fieldDelimiterCharacter = m_wiz->m_parse->fieldDelimiterCharacter(m_wiz->m_fieldDelimiterIndex);
    m_wiz->m_textDelimiterCharacter = m_wiz->m_parse->textDelimiterCharacter(m_wiz->m_textDelimiterIndex);
    m_wiz->m_startLine = profilesGroup.readEntry("StartLine", 0) + 1;
    m_wiz->m_pageLinesDate->m_trailerLines = profilesGroup.readEntry("TrailerLines", 0);
    m_wiz->m_encodeIndex = profilesGroup.readEntry("Encoding", 0);
    break;
  }
  KConfigGroup mainGroup(config, "MainWindow");
  m_wiz->m_pluginHeight = mainGroup.readEntry("Height", 640);
  m_wiz->m_pluginWidth = mainGroup.readEntry("Width", 800);
}

void InvestProcessing::reloadUISettings()
{
  m_wiz->m_memoColumn = m_columnTypeList.indexOf("memo");
  m_priceColumn = m_columnTypeList.indexOf("price");
  m_quantityColumn = m_columnTypeList.indexOf("quantity");
  m_wiz->m_dateColumn = m_columnTypeList.indexOf("date");
  m_amountColumn = m_columnTypeList.indexOf("amount");
  m_feeColumn = m_columnTypeList.indexOf("fee");
  m_nameColumn = m_columnTypeList.indexOf("name");
  m_feeIsPercentage = m_wiz->m_pageInvestment->ui->checkBoxInv_feeIsPercentage->isChecked();
  m_wiz->m_startLine = m_wiz->m_pageLinesDate->ui->spinBox_skip->value();
  m_wiz->m_endLine = m_wiz->m_pageLinesDate->ui->spinBox_skipToLast->value();
}

void InvestProcessing::clearColumnType(int column)
{
  m_columnTypeList[column].clear();
}

QString InvestProcessing::columnType(int column)
{
  return  m_columnTypeList[column];
}

void InvestProcessing::setColumnType(int column, const QString& type)
{
  m_columnTypeList[column] = type;
}

QString InvestProcessing::previousType()
{
  return m_previousType;
}

void InvestProcessing::clearPreviousType()
{
  m_previousType.clear();
}

void InvestProcessing::setPreviousType(const QString& type)
{
  m_previousType = type;
}

QString InvestProcessing::invPath()
{
  return m_invPath;
}

void InvestProcessing::setTrInvestDataType(const QString& val)
{
  m_trInvestData.type = val;
}

void InvestProcessing::resetComboBox(const QString& comboBox, const int& col)
{
  QStringList fieldType;
  fieldType << "amount" << "date" << "fee" << "memo" << "price" << "quantity" << "type" << "symbol" << "name";
  int index = fieldType.indexOf(comboBox);
  switch (index) {
    case 0://  amount
      m_wiz->m_pageInvestment->ui->comboBoxInv_amountCol->setCurrentIndex(-1);
      m_amountSelected = false;
      break;
    case 1://  date
      m_wiz->m_pageInvestment->ui->comboBoxInv_dateCol->setCurrentIndex(-1);
      m_dateSelected = false;
      break;
    case 2://  fee
      m_wiz->m_pageInvestment->ui->comboBoxInv_feeCol->setCurrentIndex(-1);
      m_feeSelected = false;
      break;
    case 3://  memo
      m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol->setCurrentIndex(-1);
      m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol->setItemText(col, QString().setNum(col + 1));   //  reset the '*'
      m_wiz->m_memoColList.removeOne(col);  //  We're clearing this memo col.
      m_memoSelected = false;
      break;
    case 4://  price
      m_wiz->m_pageInvestment->ui->comboBoxInv_priceCol->setCurrentIndex(-1);
      m_priceSelected = false;
      break;
    case 5://  quantity
      m_wiz->m_pageInvestment->ui->comboBoxInv_quantityCol->setCurrentIndex(-1);
      m_quantitySelected = false;
      break;
    case 6://  type
      m_wiz->m_pageInvestment->ui->comboBoxInv_typeCol->setCurrentIndex(-1);
      m_typeSelected = false;
      break;
    case 7://  symbol
      m_wiz->m_pageInvestment->ui->comboBoxInv_symbolCol->setCurrentIndex(-1);
      m_symbolSelected = false;
      break;
    case 8://  name
      m_wiz->m_pageInvestment->ui->comboBoxInv_nameCol->setCurrentIndex(-1);
      m_nameSelected = false;
      break;
    default:
      KMessageBox::sorry(0, i18n("<center>Field name not recognised.</center><center>'<b>%1</b>'</center>Please re-enter your column selections.", comboBox), i18n("CSV import"));
  }
  m_columnTypeList[col].clear();
}

int InvestProcessing::amountColumn()
{
  return m_amountColumn;
}

int InvestProcessing::dateColumn()
{
  return m_wiz->m_dateColumn;
}

int InvestProcessing::feeColumn()
{
  return m_feeColumn;
}

int InvestProcessing::quantityColumn()
{
  return m_quantityColumn;
}

int InvestProcessing::priceColumn()
{
  return m_priceColumn;
}

int InvestProcessing::nameColumn()
{
  return m_nameColumn;
}

int InvestProcessing::feeIsPercentage()
{
  return m_feeIsPercentage;
}

int InvestProcessing::symbolColumn()
{
  return m_symbolColumn;
}

int InvestProcessing::memoColumn()
{
  return m_wiz->m_memoColumn;
}

int InvestProcessing::typeColumn()
{
  return m_typeColumn;
}

bool InvestProcessing::importNow()
{
  return m_importNow;
}

void  InvestProcessing::setSecurityName(QString name)
{
  m_securityName = name;
}

void InvestProcessing::slotNamesEdited()
{
  int row = 0;
  int symTableRow = -1;

  for (row = m_wiz->m_startLine - 1; row < m_wiz->m_endLine; row ++) {
    if (m_wiz->ui->tableWidget->item(row, symbolColumn()) == 0) {  //  Item does not exist
      continue;
    }
    symTableRow++;
    if (m_wiz->ui->tableWidget->item(row, symbolColumn())->text().trimmed().isEmpty()) {
      continue;
    }
    //  Replace detail with edited security name.
    QString securityName = m_symbolTableDlg->m_widget->tableWidget->item(symTableRow, 2)->text();
    if (nameColumn() > -1)
      m_wiz->ui->tableWidget->item(row, nameColumn())->setText(securityName);
    //  Replace symbol with edited symbol.
    QString securitySymbol = m_symbolTableDlg->m_widget->tableWidget->item(symTableRow, 0)->text();
    if (symbolColumn() > -1)
      m_wiz->ui->tableWidget->item(row, symbolColumn())->setText(securitySymbol);
    m_map.insert(securitySymbol, securityName);
  }

  emit isImportable();
}

void InvestProcessing::securityNameSelected(const QString& name)
{
  if ((m_securityList.contains(name)) || (name.isEmpty())) {
    return;
  }

  m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->setInsertPolicy(QComboBox::InsertAlphabetically);
  m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->setDuplicatesEnabled(false);
  m_securityName = name;
  m_securityList << name;
  m_securityList.removeDuplicates();
  m_securityList.sort();
}

void InvestProcessing::securityNameEdited()
{
  QString name = m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->currentText();
  int index = m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->findText(name);
  if ((index >= 0) || (name.isEmpty())) {
    return;
  }
  int rc = KMessageBox::warningContinueCancel(0, i18n("<center>Do you want to add a new security</center>\n"
           "<center>%1 </center>\n"
           "<center>to the selection list?</center>\n"
           "<center>Click \'Continue\' to add the name.</center>\n"
           "<center>Otherwise, click \'Cancel\'.</center>",
           name), i18n("Add Security Name"));
  if (rc == KMessageBox::Cancel) {
    m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->clearEditText();
    m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->setCurrentIndex(-1);
  } else {
    m_securityName = name;
    m_securityList << name;
    m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->addItem(name);
    m_securityList.removeDuplicates();
    m_securityList.sort();
  }
}

QStringList InvestProcessing::securityList()
{
  return m_securityList;
}

void InvestProcessing::calculateFee()
{
  QString txt;
  QString newTxt;
  MyMoneyMoney amount;
  MyMoneyMoney fee;
  MyMoneyMoney minFee;
  MyMoneyMoney percent;
  double d;
  bool ok;

  txt = m_wiz->m_pageInvestment->ui->lineEdit_feeRate->text();
  if (txt.isEmpty() || m_amountColumn == -1) //check if required inputs are in place
    return;
  newTxt = m_wiz->m_parse->possiblyReplaceSymbol(txt);
  percent = MyMoneyMoney(newTxt);

  txt=m_wiz->m_pageInvestment->ui->lineEdit_minFee->text();
  if (txt.isEmpty())
    minFee = MyMoneyMoney(0);
  else
    {
      newTxt = m_wiz->m_parse->possiblyReplaceSymbol(txt);
      minFee = MyMoneyMoney(newTxt);
    }

  if (m_feeColumn == -1) //check if fee column is already present
    {
      if (m_wiz->m_endColumn > m_wiz->m_maxColumnCount) //shift virtual columns in m_columnTypeList to the end
      {
        for (int i = (m_wiz->m_endColumn - m_wiz->m_maxColumnCount) ; i > 0  ; i--)
          m_columnTypeList << QString();
        for (int i = (m_wiz->m_endColumn - m_wiz->m_maxColumnCount) ; i > 0  ; i--)
          m_columnTypeList[m_wiz->m_endColumn] = m_columnTypeList[m_wiz->m_endColumn - i];
      }

      m_feeColumn = m_wiz->m_maxColumnCount;
      m_wiz->m_maxColumnCount ++;
      m_wiz->m_endColumn ++;
      m_wiz->ui->tableWidget->setColumnCount(m_wiz->m_maxColumnCount);
      if (m_columnTypeList.count() <= m_feeColumn)
        m_columnTypeList << "fee";
      else
        m_columnTypeList[m_feeColumn] = "fee";
      txt.setNum(m_feeColumn + 1);
      m_wiz->m_pageInvestment->ui->comboBoxInv_feeCol->addItem(txt); //add generated column to fee combobox...
      m_wiz->m_pageInvestment->ui->comboBoxInv_feeCol->setCurrentIndex(m_feeColumn); // ...and select it by default
      m_feeSelected = true;
    }

  for (int i = m_wiz->m_startLine - 1; i < m_wiz->m_endLine; i++)
    {
      m_columnList = m_wiz->m_parse->parseLine(m_wiz->m_lineList[i]);
      txt = m_columnList[m_amountColumn];
      txt.replace(QRegExp("[,. ]"),"").toInt(&ok);
      if (!ok) //if the line is in statement's header, skip
        {
          m_wiz->m_lineList[i] = m_wiz->m_lineList[i] + m_wiz->m_fieldDelimiterCharacter;
          continue;
        }
      txt = m_columnList[m_amountColumn];
      txt = txt.remove('"');
      if (txt.contains(')')) {
          txt = '-' + txt.remove(QRegExp("[()]"));   //            Mark as -ve
        }
      newTxt = m_wiz->m_parse->possiblyReplaceSymbol(txt);
      MyMoneyMoney amount = MyMoneyMoney(newTxt);
      fee = percent * amount / MyMoneyMoney(100);
      if (fee < minFee)
        fee = minFee;
      d = fee.toDouble();
      txt.setNum(d, 'f', 4);
      txt.replace('.', m_wiz->m_decimalSymbol); //make sure decimal symbol is uniform in whole line

      if (m_wiz->m_decimalSymbol == m_wiz->m_fieldDelimiterCharacter) { //make sure fee has the same notation as the line it's being attached to
        if (m_columnList.count() == m_columnTypeList.count())
          m_wiz->m_lineList[i] = m_wiz->m_lineList[i].left(m_wiz->m_lineList[i].length() - txt.length() - 2 * m_wiz->m_textDelimiterCharacter.length() - m_wiz->m_fieldDelimiterCharacter.length());
        m_wiz->m_lineList[i] = m_wiz->m_lineList[i] + m_wiz->m_fieldDelimiterCharacter + m_wiz->m_textDelimiterCharacter + txt + m_wiz->m_textDelimiterCharacter;
      }
      else {
        if (m_columnList.count() == m_columnTypeList.count())
          m_wiz->m_lineList[i] = m_wiz->m_lineList[i].left(m_wiz->m_lineList[i].length()-txt.length() - m_wiz->m_fieldDelimiterCharacter.length() );
        m_wiz->m_lineList[i] = m_wiz->m_lineList[i] + m_wiz->m_fieldDelimiterCharacter + txt;
      }

      QTableWidgetItem *item = new QTableWidgetItem;
      item->setText(txt + "  ");
      m_wiz->ui->tableWidget->setItem(i,m_feeColumn,item);
    }
    m_wiz->updateWindowSize();
}

void InvestProcessing::hideSecurity()
{
  QString name = m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->currentText();
  if (name.isEmpty()) {
    return;
  }
  int rc = KMessageBox::warningContinueCancel(0, i18n("<center>You have selected to remove from the selection list</center>\n"
           "<center>%1. </center>\n"
           "<center>Click \'Continue\' to remove the name, or</center>\n"
           "<center>Click \'Cancel\'' to leave 'as is'.</center>",
           name), i18n("Hide Security Name"));
  if (rc == KMessageBox::Continue) {
    int index = m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->currentIndex();
    m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->removeItem(index);
    m_securityList.removeAt(index);
    m_securityName.clear();
  }
}
