/*******************************************************************************
*                                 csvwizard.cpp
*                              --------------------
* begin                       : Thur Jan 01 2015
* copyright                   : (C) 2015 by Allan Anderson
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

#include "csvwizard.h"

#include <QWizard>
#include <QWizardPage>
#include <QCloseEvent>
#include <QTimer>
#include <QDebug>
#include <QDesktopWidget>
#include <QTextCodec>

#include <KMessageBox>
#include <KIconLoader>
#include <KLocalizedString>
#include <KConfigGroup>
#include <KColorScheme>

#include "csvdialog.h"
#include "convdate.h"
#include "csvutil.h"
#include "investprocessing.h"
#include "symboltabledlg.h"

#include "mymoneyfile.h"

#include "ui_csvwizard.h"
#include "ui_introwizardpage.h"
#include "ui_separatorwizardpage.h"
#include "ui_bankingwizardpage.h"
#include "ui_lines-datewizardpage.h"
#include "ui_completionwizardpage.h"
#include "ui_investmentwizardpage.h"

CSVWizard::CSVWizard() : ui(new Ui::CSVWizard)
{
  ui->setupUi(this);

  m_parse = new Parse;
  m_convertDate = new ConvertDate;
  m_csvUtil = new CsvUtil;
  m_csvDialog = new CSVDialog;
  m_csvDialog->m_wiz = this;
  m_investProcessing = new InvestProcessing;
  m_investProcessing->m_wiz = this;

  m_curId = -1;
  m_lastId = -1;
  m_initialHeight = -1;
  m_initialWidth = -1;
  m_maxColumnCount = 0;
  m_importError = false;
  m_importIsValid = false;
  m_fileEndLine = 0;
  m_thousandsSeparator = ',';
  ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

  m_wizard = new QWizard;
  m_wizard->setWizardStyle(QWizard::ClassicStyle);
  ui->horizontalLayout->addWidget(m_wizard);

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
  m_wizard->setDefaultProperty("QComboBox", "dateCol", SIGNAL(currentIndexChanged(int)));
  m_wizard->setDefaultProperty("QComboBox", "dateCol", SIGNAL(currentIndexChanged(int)));
}

void CSVWizard::init()
{
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

  m_profileList.clear();
  m_priorCsvProfile.clear();
  m_priorInvProfile.clear();
  findCodecs();
  readSettingsProfiles();
  m_csvDialog->init();
  m_investProcessing->init();

  connect(m_wizard->button(QWizard::CancelButton), SIGNAL(clicked()), this, SLOT(slotClose()));

  connect(m_pageIntro->ui->radioButton_bank, SIGNAL(clicked()), m_pageIntro, SLOT(slotRadioButton_bankClicked()));
  connect(m_pageIntro->ui->radioButton_invest, SIGNAL(clicked()), m_pageIntro, SLOT(slotRadioButton_investClicked()));

  connect(m_pageBanking->ui->radioBnk_amount, SIGNAL(toggled(bool)), this, SLOT(amountRadioClicked(bool)));
  connect(m_pageBanking->ui->radioBnk_debCred, SIGNAL(toggled(bool)), this, SLOT(debitCreditRadioClicked(bool)));
  connect(m_pageBanking->ui->checkBoxBnk_oppositeSigns, SIGNAL(clicked(bool)), this, SLOT(oppositeSignsCheckBoxClicked(bool)));
  connect(m_pageBanking->ui->button_clear, SIGNAL(clicked()), this, SLOT(clearColumnsSelected()));
  connect(m_pageInvestment->ui->button_clear, SIGNAL(clicked()), m_investProcessing, SLOT(clearColumnsSelected()));
  connect(m_pageInvestment->ui->buttonInv_clearFee, SIGNAL(clicked()), m_investProcessing, SLOT(clearFeesSelected()));
  connect(m_pageCompletion, SIGNAL(importBanking()), m_csvDialog, SLOT(slotImportClicked()));
  connect(m_pageCompletion, SIGNAL(importInvestment()), m_investProcessing, SLOT(slotImportClicked()));
  connect(m_pageCompletion, SIGNAL(completeChanged()), this, SLOT(slotClose()));

  connect(m_wizard->button(QWizard::CustomButton1), SIGNAL(clicked()), m_csvDialog, SLOT(slotFileDialogClicked()));
  connect(m_wizard->button(QWizard::CustomButton1), SIGNAL(clicked()), m_investProcessing, SLOT(slotFileDialogClicked()));
  connect(m_wizard->button(QWizard::BackButton), SIGNAL(clicked()), m_csvDialog, SLOT(slotBackButtonClicked()));
  connect(m_wizard->button(QWizard::CustomButton2), SIGNAL(clicked()), m_pageCompletion, SLOT(slotImportClicked()));
  connect(m_wizard->button(QWizard::CustomButton3), SIGNAL(clicked()), m_csvDialog, SLOT(slotSaveAsQIF()));
  connect(m_wizard->button(QWizard::FinishButton), SIGNAL(clicked()), this, SLOT(slotClose()));
  connect(m_wizard, SIGNAL(currentIdChanged(int)), this, SLOT(slotIdChanged(int)));

  ui->tableWidget->setWordWrap(false);
  m_pageCompletion->ui->comboBox_thousandsDelimiter->setEnabled(false);
  m_pageSeparator->ui->comboBox_fieldDelimiter->setEnabled(false);

  m_vScrollBar = ui->tableWidget->verticalScrollBar();
  m_vScrollBar->setTracking(false);
  m_dateFormats << "yyyy/MM/dd" << "MM/dd/yyyy" << "dd/MM/yyyy";

  m_clearBrush = KColorScheme(QPalette::Normal).background(KColorScheme::NormalBackground);
  m_clearBrushText = KColorScheme(QPalette::Normal).foreground(KColorScheme::NormalText);
  m_colorBrush = KColorScheme(QPalette::Normal).background(KColorScheme::PositiveBackground);
  m_colorBrushText = KColorScheme(QPalette::Normal).foreground(KColorScheme::PositiveText);
  m_errorBrush = KColorScheme(QPalette::Normal).background(KColorScheme::NegativeBackground);
  m_errorBrushText = KColorScheme(QPalette::Normal).foreground(KColorScheme::NegativeText);

  connect(m_csvDialog, SIGNAL(isImportable()), m_pageCompletion, SLOT(slotImportValid()));
  connect(m_investProcessing, SIGNAL(isImportable()), m_pageCompletion, SLOT(slotImportValid()));
  int y = (QApplication::desktop()->height() - this->height()) / 2;
  int x = (QApplication::desktop()->width() - this->width()) / 2;
  move(x, y);
  show();
}

CSVWizard::~CSVWizard()
{
  delete ui;
  delete m_investProcessing;
  delete m_csvDialog;
  delete m_parse;
  delete m_convertDate;
  delete m_csvUtil;
  delete m_wizard;
}

void CSVWizard::showStage()
{
  QString str = ui->label_intro->text();
  ui->label_intro->setText("<b>" + str + "</b>");
}

void CSVWizard::createProfile(QString newName)
{
  KSharedConfigPtr  config = KSharedConfig::openConfig(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + "csvimporterrc");
  KConfigGroup bankProfilesGroup(config, "BankProfiles");

  bankProfilesGroup.writeEntry("BankNames", m_profileList);
  bankProfilesGroup.config()->sync();

  KConfigGroup bankGroup(config, "BankProfiles");
  QString txt = "Profiles-" + newName;

  KConfigGroup newProfilesGroup(config, txt);
  newProfilesGroup.writeEntry("FileType", m_fileType);
  if (m_fileType == "Invest") {
    newProfilesGroup.writeEntry("ShrsinParam", m_investProcessing->m_shrsinList);
    newProfilesGroup.writeEntry("DivXParam", m_investProcessing->m_divXList);
    newProfilesGroup.writeEntry("IntIncParam", m_investProcessing->m_intIncList);
    newProfilesGroup.writeEntry("BrokerageParam", m_investProcessing->m_brokerageList);
    newProfilesGroup.writeEntry("ReinvdivParam", m_investProcessing->m_reinvdivList);
    newProfilesGroup.writeEntry("BuyParam", m_investProcessing->m_buyList);
    newProfilesGroup.writeEntry("SellParam", m_investProcessing->m_sellList);
    newProfilesGroup.writeEntry("RemoveParam", m_investProcessing->m_removeList);
  }
  newProfilesGroup.config()->sync();
}

void CSVWizard::readSettingsProfiles()
{
  KSharedConfigPtr  newConfig = KSharedConfig::openConfig(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + "csvimporterrc");
  KConfigGroup newBankGroup(newConfig, "BankProfiles");
  if (newBankGroup.exists()) {     //  If local config file exists, exit
    return;
  }
  KSharedConfigPtr  config = KSharedConfig::openConfig(QStandardPaths::locate(QStandardPaths::ConfigLocation, "csvimporterrc"));
  KConfigGroup bankGroup(config, "BankProfiles");

  QStringList lst = bankGroup.readEntry("BankNames", QStringList());
  foreach (const QString & group, lst) {
    bankGroup.copyTo(&newBankGroup);
    newBankGroup.config()->sync();

    QString txt = "Profiles-" + group;
    KConfigGroup profilesGroup(config, txt);
    KConfigGroup newProfilesGroup(newConfig, txt);
    profilesGroup.copyTo(&newProfilesGroup);
    newProfilesGroup.config()->sync();
  }
  KConfigGroup securitiesGroup(config, "Securities");
  KConfigGroup newSecuritiesGroup(newConfig, "Securities");
  securitiesGroup.copyTo(&newSecuritiesGroup);
  newSecuritiesGroup.config()->sync();
}

void CSVWizard::readSettingsInit()
{
  m_pageIntro->m_index = 0;
  KSharedConfigPtr  myconfig = KSharedConfig::openConfig(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + "csvimporterrc");
  KConfigGroup bankProfilesGroup(myconfig, "BankProfiles");

  m_profileList.clear();
  m_pageIntro->ui->combobox_source->clear();
  m_pageIntro->ui->combobox_source->addItem(i18n("Add New Profile"));
  QStringList list = bankProfilesGroup.readEntry("BankNames", QStringList());
  if (!list.isEmpty()) {
    for (int i = 0; i < list.count(); i++) {
      m_profileList.append(list[i]);
      QString txt = "Profiles-" + list[i];
      KConfigGroup profilesGroup(myconfig, txt);

      if (profilesGroup.exists()) {
        txt = profilesGroup.readEntry("FileType", QString());
        m_pageIntro->m_mapFileType.insert(list[i], txt);
        if (txt == m_fileType) {
          m_pageIntro->ui->combobox_source->addItem(list[i]);
          m_pageIntro->m_map.insert(list[i], m_pageIntro->m_index++);
        }
      }
    }
  }
  if (m_fileType == "Banking") {
    m_priorCsvProfile = bankProfilesGroup.readEntry("PriorCsvProfile", QString());
    if (m_priorCsvProfile.isEmpty()) {
      m_pageIntro->ui->combobox_source->setCurrentIndex(0);
    } else {
      m_profileName = m_priorCsvProfile;
      int indx = m_pageIntro->ui->combobox_source->findText(m_priorCsvProfile);
      m_pageIntro->ui->combobox_source->setCurrentIndex(indx);
      m_pageIntro->m_index = indx;
    }
  } else if (m_fileType == "Invest") {
    m_priorInvProfile = bankProfilesGroup.readEntry("PriorInvProfile", QString());
    if (m_priorInvProfile.isEmpty()) {
      m_pageIntro->ui->combobox_source->setCurrentIndex(0);
    } else {
      int indx = m_pageIntro->ui->combobox_source->findText(m_priorInvProfile);
      m_pageIntro->ui->combobox_source->setCurrentIndex(indx);
      m_pageIntro->m_index = indx;
      m_profileName = m_priorInvProfile;
    }
  }
  disconnect(m_pageIntro->ui->combobox_source->lineEdit(), SIGNAL(editingFinished()), m_pageIntro, SLOT(slotLineEditingFinished()));
}


void CSVWizard::setCodecList(const QList<QTextCodec *> &list)
{
  m_comboBoxEncode->clear();
  foreach (QTextCodec * codec, list)
  m_comboBoxEncode->addItem(codec->name(), codec->mibEnum());
}

void CSVWizard::findCodecs()
{
  QMap<QString, QTextCodec *> codecMap;
  QRegExp iso8859RegExp("ISO[- ]8859-([0-9]+).*");

  foreach (int mib, QTextCodec::availableMibs()) {
    QTextCodec *codec = QTextCodec::codecForMib(mib);

    QString sortKey = codec->name().toUpper();
    int rank;

    if (sortKey.startsWith("UTF-8")) {             // krazy:exclude=strings
      rank = 1;
    } else if (sortKey.startsWith("UTF-16")) {            // krazy:exclude=strings
      rank = 2;
    } else if (iso8859RegExp.exactMatch(sortKey)) {
      if (iso8859RegExp.cap(1).size() == 1)
        rank = 3;
      else
        rank = 4;
    } else {
      rank = 5;
    }
    sortKey.prepend(QChar('0' + rank));

    codecMap.insert(sortKey, codec);
  }
  m_codecs = codecMap.values();
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

    m_pageBanking->ui->comboBoxBnk_debitCol->setEnabled(false);
    m_pageBanking->ui->comboBoxBnk_debitCol->setCurrentIndex(-1);
    m_pageBanking->ui->comboBoxBnk_creditCol->setEnabled(false);
    m_pageBanking->ui->comboBoxBnk_creditCol->setCurrentIndex(-1);
  }
}

void CSVWizard::debitCreditRadioClicked(bool checked)
{
  if (checked) {
    m_pageBanking->ui->comboBoxBnk_debitCol->setEnabled(true);  //         if 'debit/credit' selected
    m_pageBanking->ui->labelBnk_debits->setEnabled(true);
    m_pageBanking->ui->comboBoxBnk_creditCol->setEnabled(true);
    m_pageBanking->ui->labelBnk_credits->setEnabled(true);

    m_pageBanking->ui->comboBoxBnk_amountCol->setEnabled(false);  //       disable 'amount' ui choices
    m_pageBanking->ui->comboBoxBnk_amountCol->setCurrentIndex(-1);  //     as credit/debit chosen
    m_pageBanking->ui->labelBnk_amount->setEnabled(false);
  }
}

void CSVWizard::oppositeSignsCheckBoxClicked(bool checked)
{
  m_csvDialog->m_oppositeSigns = checked;
}

void CSVWizard::clearColumnsSelected()
{
  //  User has clicked clear button
  if (m_fileType == "Banking") {
    m_csvDialog->clearColumnNumbers();
    m_csvDialog->clearComboBoxText();
    m_memoColList.clear();
  }
}

void CSVWizard::slotClose()
{
  if (!m_csvDialog->m_closing) {
    m_csvDialog->saveSettings();
    m_investProcessing->saveSettings();
  }
  close();
}

void CSVWizard::clearBackground()
{
  for (int row = 0; row < ui->tableWidget->rowCount(); row++) {
    for (int col = 0; col < ui->tableWidget->columnCount(); col++) {
      if (ui->tableWidget->item(row, col) != 0) {
        ui->tableWidget->item(row, col)->setBackground(m_clearBrush);
        ui->tableWidget->item(row, col)->setForeground(m_clearBrushText);
      }
    }
  }
}

void CSVWizard::markUnwantedRows()
{
  if (m_fileType == "Banking") {
    if (!m_pageBanking->m_bankingPageInitialized) {
      return;
    }
  } else if (m_fileType == "Invest") {
    if (!m_pageInvestment->m_investPageInitialized) {
      return;
    }
  }
  int first = m_startLine - 1;
  int last = m_endLine - 1;
  //
  //  highlight unwanted lines instead of not showing them.
  //
  QBrush brush;
  QBrush brushText;
  for (int row = 0; row < ui->tableWidget->rowCount(); row++) {
    if ((row < first) || (row > last)) {
      brush = m_errorBrush;
      brushText = m_errorBrushText;
    } else {
      brush = m_clearBrush;
      brushText = m_clearBrushText;
    }
    for (int col = 0; col < ui->tableWidget->columnCount(); col ++) {
      if (ui->tableWidget->item(row, col) != 0) {
        ui->tableWidget->item(row, col)->setBackground(brush);
        ui->tableWidget->item(row, col)->setForeground(brushText);
      }
    }
  }
}

void CSVWizard::decimalSymbolSelected(int index)
{
  if (index < 0)
  {
    index = 0;
    m_pageCompletion->ui->comboBox_decimalSymbol->blockSignals(true);
    m_pageCompletion->ui->comboBox_decimalSymbol->setCurrentIndex(index);
    m_pageCompletion->ui->comboBox_decimalSymbol->blockSignals(false);
  }

  //  Save new decimal symbol and thousands separator
  m_decimalSymbolIndex = index;
  m_parse->setDecimalSymbolIndex(index);
  m_decimalSymbol = m_parse->decimalSymbol(index);
  m_pageCompletion->ui->comboBox_thousandsDelimiter->setCurrentIndex(index);
  thousandsSeparatorChanged();

  if (!(m_wizard->currentId() == Page_Completion)) // do not update tableWidget unless on completion page
    return;

  if (m_fileType == "Banking") {
    if (m_csvDialog->m_colTypeNum.value(CSVDialog::ColumnAmount) >= 0)
      updateDecimalSymbol(m_csvDialog->m_colTypeNum.value(CSVDialog::ColumnAmount));
    else {
      updateDecimalSymbol(m_csvDialog->m_colTypeNum.value(CSVDialog::ColumnDebit));
      updateDecimalSymbol(m_csvDialog->m_colTypeNum.value(CSVDialog::ColumnCredit));
    }
  } else if (m_fileType == "Invest") {
    updateDecimalSymbol(m_investProcessing->amountColumn());
    updateDecimalSymbol(m_investProcessing->priceColumn());
    updateDecimalSymbol(m_investProcessing->quantityColumn());
  }

  if (!m_importError)
    m_pageCompletion->slotImportValid();
}

void CSVWizard::decimalSymbolSelected()
{
  decimalSymbolSelected(m_decimalSymbolIndex);
}

void CSVWizard::updateDecimalSymbol(int col)
{
  QString txt;
  bool symbolFound = false;
  bool invalidResult = false;
  //  Clear background

  for (int row = m_startLine; row < m_endLine; row++) {
    if (ui->tableWidget->item(row, col) != 0) {
      ui->tableWidget->item(row, col)->setBackground(m_clearBrush);
      ui->tableWidget->item(row, col)->setForeground(m_clearBrushText);
    }
  }

  int errorRow = 0;
  m_parse->setSymbolFound(false);

  QString newTxt;
  QTableWidgetItem* errorItem(0);
  //  Check if this col contains empty cells
  for (int row = m_startLine - 1; row < m_endLine; row++) {
    if (ui->tableWidget->item(row, col) == 0) {      //       empty cell
      if (m_importNow) {
        //                                                    if importing, this is error
        KMessageBox::sorry(this, i18n("Row number %1 may be a header line, as it has an incomplete set of entries."
                                      "<center>It may be that the start line is incorrectly set.</center>",
                                      row + 1), i18n("CSV import"));
        return;
      }
      //                                                      if not importing, query

      int ret = KMessageBox::warningContinueCancel(this, i18n("<center>The cell in column '%1' on row %2 is empty.</center>"
                                                              "<center>Please check your selections.</center><center>Continue or Cancel?</center>",
                                                              col + 1, row + 1), i18n("Selections Warning"), KStandardGuiItem::cont(),
                                                   KStandardGuiItem::cancel());
      if (ret == KMessageBox::Continue) {
        continue;
      }
      return;//                                               empty cell
    } else {
      //  Check if this col contains decimal symbol

      txt = ui->tableWidget->item(row, col)->text(); //       get data
      newTxt = m_parse->possiblyReplaceSymbol(txt);  //       update data
      ui->tableWidget->item(row, col)->setText(newTxt);   //  highlight selection
      ui->tableWidget->item(row, col)->setBackground(m_colorBrush);
      ui->tableWidget->item(row, col)->setForeground(m_colorBrushText);
      if (m_parse->invalidConversion()) {
        invalidResult = true;
        errorItem = ui->tableWidget->item(row, col);
        errorItem->setBackground(m_errorBrush);
        errorItem->setForeground(m_errorBrushText);
        ui->tableWidget->scrollToItem(errorItem, QAbstractItemView::EnsureVisible);
        if (errorRow == 0) {
          errorRow = row;
        }
      }
      if (m_pageIntro->isVisible() || m_pageLinesDate->isVisible()) {
        ui->tableWidget->horizontalScrollBar()->setValue(col);  //                   ensure col visible
      }
      if (m_parse->symbolFound()) {
        symbolFound = true;
      }
      if (newTxt == txt) {      //                                                 no matching symbol found
        continue;
      }
    }
    ui->tableWidget->resizeColumnToContents(col);
  }//  last row

  if (!symbolFound && !m_skipSetup && !m_errorFoundAlready) {  //  no symbol found
    ui->tableWidget->horizontalScrollBar()->setValue(col);  //                     ensure col visible
    KMessageBox::sorry(this, i18n("<center>The selected decimal symbol was not present in column %1,</center>"
                                  "<center>- but may now have been added.</center>"
                                  "<center>If the <b>decimal</b> symbol displayed does not match your system setting</center>"
                                  "<center>your data is unlikely to import correctly.</center>"
                                  "<center>Please check your selection.</center>",
                                  col + 1), i18n("CSV import"));
    m_errorFoundAlready = true;
  }

  if (invalidResult && !m_errorFoundAlready) {
    ui->tableWidget->verticalScrollBar()->setValue(errorRow - 1);  //              ensure row visible
    KMessageBox::sorry(this, i18n("<center>The selected decimal symbol ('%1') was not present</center>"
                                  "<center>or has produced invalid results in row %2, and possibly more.</center>"
                                  "<center>Please try again.</center>", m_decimalSymbol, errorRow + 1), i18n("Invalid Conversion"));
    m_importError = true;
    m_importNow = false;
    m_wizard->button(QWizard::NextButton)->hide();
    m_wizard->button(QWizard::CustomButton1)->hide();
    return;
  } else {  //  allow user to change setting and try again
    m_importError = false;
    m_importNow = true;
    m_errorFoundAlready = true;
  }
}

void CSVWizard::thousandsSeparatorChanged()
{
  m_thousandsSeparator = m_parse->thousandsSeparator();
}

void CSVWizard::delimiterChanged(int index)
{
  m_pageBanking->m_bankingPageInitialized  = false;
  m_pageInvestment->m_investPageInitialized  = false;

  m_fieldDelimiterIndex = index;
  m_maxColumnCount = getMaxColumnCount(m_lineList, m_fieldDelimiterIndex); // get column count, we get with this fieldDelimiter
  m_endColumn = m_maxColumnCount;
  if (index == -1) // set valid fieldDelimiter if it wasn't set
  {
    m_pageSeparator->ui->comboBox_fieldDelimiter->blockSignals(true);
    m_pageSeparator->ui->comboBox_fieldDelimiter->setCurrentIndex(m_fieldDelimiterIndex);
    m_pageSeparator->ui->comboBox_fieldDelimiter->blockSignals(false);
  }
  m_parse->setFieldDelimiterIndex(m_fieldDelimiterIndex);
  m_fieldDelimiterCharacter = m_parse->fieldDelimiterCharacter(m_fieldDelimiterIndex);
  displayLines(m_lineList, m_parse);  // refresh tableWidget with new fieldDelimiter set
}

bool CSVWizard::validateDateFormat(int dF)
{
  if (m_lineList.isEmpty())
    return false;

  QTableWidgetItem* tableItem(0);
  m_convertDate->setDateFormatIndex(dF);
  m_importError = false;

  for (int i = m_startLine - 1 ; i < m_endLine; i++)
  {
    QStringList columnList = m_parse->parseLine(m_lineList[i]);
    QString txt = columnList[m_dateColumn];
    txt = txt.remove(m_textDelimiterCharacter);       //   "16/09/2009
    QDate dat = m_convertDate->convertDate(txt);      //  Date column

    tableItem = ui->tableWidget->item(i, m_dateColumn);
    if (dat == QDate()) {
      tableItem->setBackground(m_errorBrush);
      tableItem->setForeground(m_errorBrushText);
      m_importError = true;
    } else {
      tableItem->setBackground(m_colorBrush);
      tableItem->setForeground(m_colorBrushText);
    }
  }
  if (m_importError)
  {
    KMessageBox::sorry(this, i18n("<center>There are invalid date formats in column '%1'.</center>"
                                  "<center>Please check your selections.</center>"
                                  ,m_dateColumn ), i18n("CSV import"));
    return false;
  }

  return true;
}

int CSVWizard::getMaxColumnCount(QStringList &lineList, int &delimiter)
{
  if (lineList.isEmpty())
    return 0;

  QList<int> delimiterIndexes;
  if (delimiter == -1)
    delimiterIndexes = QList<int>{0, 1, 2, 3}; // include all delimiters to test or ...
  else
    delimiterIndexes = QList<int>{delimiter}; // ... only the one specified

  int totalDelimiterCount[4] = {0}; //  Total in file for each delimiter
  int thisDelimiterCount[4] = {0};  //  Total in this line for each delimiter
  int colCount = 0;                 //  Total delimiters in this line
  int possibleDelimiter = 0;
  int maxColumnCount = 0;

  for (int i = 0; i < lineList.count(); i++) {
    QString data = lineList[i];

    for (QList<int>::ConstIterator it = delimiterIndexes.constBegin(); it != delimiterIndexes.constEnd(); it++) {
      m_parse->setFieldDelimiterIndex(*it);
      colCount = m_parse->parseLine(data).count(); //  parse each line using each delimiter

      if (colCount > thisDelimiterCount[*it])
        thisDelimiterCount[*it] = colCount;

      if (thisDelimiterCount[*it] > maxColumnCount)
        maxColumnCount = thisDelimiterCount[*it];

      totalDelimiterCount[*it] += colCount;
      if (totalDelimiterCount[*it] > totalDelimiterCount[possibleDelimiter])
        possibleDelimiter = *it;
    }
  }
  delimiter = possibleDelimiter;
  m_parse->setFieldDelimiterIndex(delimiter);
  return maxColumnCount;
}

void CSVWizard::readFile(const QString& fname)
{
  if (!fname.isEmpty())
    m_inFileName = fname;

  m_importError = false;
  m_errorFoundAlready = false;

  QFile  inFile(m_inFileName);
  inFile.open(QIODevice::ReadOnly);  // allow a Carriage return -// QIODevice::Text
  QTextStream inStream(&inFile);
  QTextCodec* codec = QTextCodec::codecForMib(m_encodeIndex);
  inStream.setCodec(codec);

  QString buf = inStream.readAll();
  inFile.close();
  m_lineList = m_parse->parseFile(buf, 1, 0);  // parse the buffer

  m_maxColumnCount = getMaxColumnCount(m_lineList, m_fieldDelimiterIndex); // get column count, we get with this fieldDelimiter
  m_endColumn = m_maxColumnCount;
  m_fileEndLine = m_parse->lastLine();

  if (m_fileEndLine > m_pageLinesDate->m_trailerLines)
    m_endLine = m_fileEndLine - m_pageLinesDate->m_trailerLines;
   else
    m_endLine = m_fileEndLine;  // Ignore m_trailerLines as > file length.

  if (m_startLine > m_endLine)   // Don't allow m_startLine > m_endLine
    m_startLine = m_endLine;
}

void CSVWizard::displayLines(const QStringList &lineList, Parse* parse)
{
  ui->tableWidget->clear();
  m_row = 0;
  QFont font(QApplication::font());
  ui->tableWidget->setFont(font);
  ui->tableWidget->setRowCount(lineList.count());
  ui->tableWidget->setColumnCount(m_maxColumnCount);

  for (int line = 0; line < lineList.count(); line++) {
    QStringList columnList = parse->parseLine(lineList[line]);
    for (int col = 0; col < columnList.count(); col ++) {
      QTableWidgetItem *item = new QTableWidgetItem;  // new item for tableWidget
      item->setText(columnList[col]);
      ui->tableWidget->setItem(m_row, col, item);  // add item to tableWidget
    }
    m_row ++;
  }

  for (int col = 0; col < ui->tableWidget->columnCount(); col ++)
    ui->tableWidget->resizeColumnToContents(col);
}

void CSVWizard::updateWindowSize()
{
  QTableWidget *table = this->ui->tableWidget;
  int newWidth = table->verticalHeader()->width() + table->verticalScrollBar()->width();      //take header and scrollbar into account
  int newHeight = table->horizontalHeader()->height() + table->horizontalScrollBar()->height();
  table->horizontalHeader()->setStretchLastSection(false);
  table->resizeColumnsToContents();

  QRect screen = QApplication::desktop()->availableGeometry();    //get available screen size
  QRect view = table->contentsRect();                             //get current tableview size
  QRect wizard = this->geometry();                               //get current wizard size

  for(int i = 0; i < table->columnCount(); i++ )
    newWidth += table->columnWidth(i);                            //add up required column widths

  if( this->ui->tableWidget->rowCount() > 0)
    newHeight += this->ui->tableWidget->rowCount() * table->rowHeight(0); //add up estimated row heights

  newWidth = wizard.width() + (newWidth - view.width());
  newHeight = wizard.height() + (newHeight - view.height());

  if (newWidth > screen.width())  //limit wizard size to screen size
    newWidth = screen.width();
  if (newHeight > screen.height())
    newHeight = screen.height();

  if (newWidth < this->m_initialWidth) //don't shrink wizard if required size is less than initial
  {
    table->horizontalHeader()->setStretchLastSection(true);
    newWidth = this->m_initialWidth;
  }
  if (newHeight < this->m_initialHeight)
    newHeight = this->m_initialHeight;

  wizard.setWidth(newWidth);
  wizard.setHeight(newHeight);
  this->setGeometry(wizard);
  wizard.moveTo((screen.width() - wizard.width()) / 2,
                (screen.height() - wizard.height()) / 2);
}

void CSVWizard::createMemoField(QStringList &columnTypeList)
{
  if (m_memoColList.count() == 0)
    return;

  columnTypeList << "memo";
  m_maxColumnCount ++;
  m_endColumn ++;

  for (int i = 0; i < m_lineList.count(); i++)
  {
    QString txt;
    QStringList columnList = m_parse->parseLine(m_lineList[i]);
    for (int j = 0; j < m_memoColList.count(); j++)
    {
      if (m_memoColList[j] != m_memoColumn)
        txt += columnList[m_memoColList[j]] + "\n";
    }
    if (txt.length() > 1)
      txt = txt.left(txt.length() - 1);
    m_lineList[i] = m_lineList[i] + m_fieldDelimiterCharacter + m_textDelimiterCharacter + txt + m_textDelimiterCharacter;
  }
}

void CSVWizard::resizeEvent(QResizeEvent* ev)
{
  if (ev->spontaneous()) {
    ev->ignore();
    return;
  }
}

//-------------------------------------------------------------------------------------------------------
IntroPage::IntroPage(QWidget *parent) : QWizardPage(parent), ui(new Ui::IntroPage)
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
  m_wizDlg = dlg;
  m_set = true;
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
    m_priorName = m_wizDlg->m_profileName;
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

    int rc = KMessageBox::warningYesNo(0, i18n("<center>You have cleared the profile name '%1'.</center>\n"
                                       "<center>If you wish to delete the entry, click 'Delete'.</center>\n"
                                       "<center>Otherwise, click 'Keep'.</center>", m_wizDlg->m_profileName),
                                       i18n("Delete or Edit Profile Name"),
                                       KGuiItem(i18n("Delete")),
                                       KGuiItem(i18n("Keep")), "");
    if (rc == KMessageBox::No) {
      //
      //  Keep
      //
      int indx = ui->combobox_source->findText(m_priorName);
      if (indx != -1)
        ui->combobox_source->setCurrentIndex(indx);
      connect(ui->combobox_source, SIGNAL(editTextChanged(QString)), this, SLOT(slotComboEditTextChanged(QString)));
      connect(ui->combobox_source->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotLineEditingFinished()));
      return;
    }
    //
    //  Delete, just clear to allow new text entry.
    //
    m_wizDlg->m_profileList.removeOne(m_wizDlg->m_profileName);
    int indx = ui->combobox_source->findText(m_wizDlg->m_profileName);
    ui->combobox_source->removeItem(indx);
    m_map.take(m_wizDlg->m_profileName);
    ui->combobox_source->setCurrentIndex(-1);
    m_priorName.clear();
    KSharedConfigPtr  config = KSharedConfig::openConfig(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QDir::separator() + "csvimporterrc");
    KConfigGroup bankProfilesGroup(config, "BankProfiles");
    KConfigGroup deletedProfilesGroup(config, "Profiles-" + m_wizDlg->m_profileName);
    m_wizDlg->m_profileName.clear();
    deletedProfilesGroup.deleteGroup();
    if (m_wizDlg->m_fileType == "Banking") {
      m_wizDlg->m_priorCsvProfile.clear();
      bankProfilesGroup.writeEntry("PriorCsvProfile", m_wizDlg->m_priorCsvProfile);
    } else {
      m_wizDlg->m_priorInvProfile.clear();
      bankProfilesGroup.writeEntry("PriorInvProfile", m_wizDlg->m_priorInvProfile);
    }
    bankProfilesGroup.writeEntry("BankNames", m_wizDlg->m_profileList);
    bankProfilesGroup.config()->sync();
    connect(ui->combobox_source, SIGNAL(editTextChanged(QString)), this, SLOT(slotComboEditTextChanged(QString)));
    connect(ui->combobox_source->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotLineEditingFinished()));
    return;
  }
  connect(ui->combobox_source->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotLineEditingFinished()));
  if (m_wizDlg->m_initialHeight == -1 || m_wizDlg->m_initialWidth == -1)
  {
    m_wizDlg->m_initialHeight = m_wizDlg->geometry().height();
    m_wizDlg->m_initialWidth = m_wizDlg->geometry().width();
  }

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
          m_wizDlg->m_profileName = ui->combobox_source->currentText();
          if (m_wizDlg->m_fileType == "Banking") {
            m_wizDlg->m_priorCsvProfile = m_wizDlg->m_profileName;
          } else {
            m_wizDlg->m_priorInvProfile = m_wizDlg->m_profileName;
          }
          m_priorName = m_wizDlg->m_profileName;
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
        m_priorName = m_wizDlg->m_profileName;
        m_priorIndex = m_index;
        if (!m_wizDlg->m_profileList.contains(txt)) {
          //  But this profile name does not exist.
          int indx = ui->combobox_source->findText(txt);
          if (m_priorName.isEmpty()) {
            disconnect(ui->combobox_source->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotLineEditingFinished()));
            //  Add it perhaps.
            QString question = i18n("<center>The name you have entered does not exist,</center>"
                                    "<center>but you have not elected to add a new profile</center>"
                                    "<center>If you wish to add '%1' as a new profile,</center>"
                                    "<center> click 'Yes'.  Otherwise, click 'No'</center>", txt);
            if (KMessageBox::questionYesNo(0, question, i18n("Adding profile name.")) == KMessageBox::Yes) {
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

        m_wizDlg->m_profileName = ui->combobox_source->currentText();
        if (m_wizDlg->m_fileType == "Banking") {
          m_wizDlg->m_priorCsvProfile = m_wizDlg->m_profileName;
        } else {
          m_wizDlg->m_priorInvProfile = m_wizDlg->m_profileName;
        }
        if (m_wizDlg->m_profileList.contains(m_wizDlg->m_profileName)) {
          return;
        }
        editProfileName(m_priorName, m_wizDlg->m_profileName);
      }
  }
}

void  IntroPage::addProfileName()
{
  m_wizDlg->m_profileName = ui->combobox_source->currentText();
  if (m_wizDlg->m_fileType == "Banking") {
    m_wizDlg->m_priorCsvProfile = m_wizDlg->m_profileName;
  } else {
    m_wizDlg->m_priorInvProfile = m_wizDlg->m_profileName;
  }
  m_priorName = m_wizDlg->m_profileName;
  m_mapFileType.insert(m_wizDlg->m_profileName, m_wizDlg->m_fileType);
  m_wizDlg->m_profileList << m_wizDlg->m_profileName;
  m_wizDlg->createProfile(m_wizDlg->m_profileName);
  int indx = ui->combobox_source->findText(m_wizDlg->m_profileName);
  if (indx == -1) {
    ui->combobox_source->addItem(m_wizDlg->m_profileName);
  }
  indx = ui->combobox_source->findText(m_wizDlg->m_profileName);
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
    if (KMessageBox::questionYesNo(0, question, i18n("Edit a profile name or create new one.")) == KMessageBox::Yes) {
      disconnect(ui->combobox_source, SIGNAL(editTextChanged(QString)), this, SLOT(slotComboEditTextChanged(QString)));
      //  Accept new name.
      m_map.take(from);
      m_wizDlg->m_profileList.removeOne(from);
      ui->combobox_source->removeItem(ui->combobox_source->findText(from, Qt::MatchExactly));
      int toIndx = ui->combobox_source->findText(toName, Qt::MatchExactly);
      if ((toIndx == -1) && (m_messageBoxJustCancelled == false)) {
        ui->combobox_source->addItem(toName);
      }
      m_index = ui->combobox_source->findText(toName, Qt::MatchExactly);
      m_wizDlg->m_profileName = toName;
      if (m_wizDlg->m_fileType == "Banking") {
        m_wizDlg->m_priorCsvProfile = m_wizDlg->m_profileName;
      } else {
        m_wizDlg->m_priorInvProfile = m_wizDlg->m_profileName;
      }
      m_wizDlg->createProfile(m_wizDlg->m_profileName);
      m_editAccepted = true;
      m_wizDlg->m_profileList << toName;
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
      m_wizDlg->m_profileList.removeOne(toName);
      if (m_wizDlg->m_fileType == "Banking") {
        m_wizDlg->m_priorCsvProfile = from;
      } else {
        m_wizDlg->m_priorInvProfile = from;
      }
      m_wizDlg->m_profileName = from;
      indx = ui->combobox_source->findText(from);
      if (indx != -1)
        ui->combobox_source->setCurrentIndex(indx);
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
  m_wizDlg->m_fileType = "Banking";
  ui->combobox_source->setEnabled(true);
  ui->combobox_source->show();

  m_wizDlg->readSettingsInit();
  m_priorName.clear();

  if ((!ui->combobox_source->currentText().isEmpty()) && (ui->combobox_source->currentIndex() >= 0)) {
    wizard()->button(QWizard::CustomButton1)->setEnabled(true);
  }
  ui->checkBoxSkipSetup->setEnabled(true);
  m_wizDlg->m_skipSetup = true;
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
  m_wizDlg->m_fileType = "Invest";
  ui->combobox_source->setEnabled(true);
  ui->combobox_source->show();

  m_wizDlg->readSettingsInit();
  m_priorName.clear();

  if ((!ui->combobox_source->currentText().isEmpty()) && (ui->combobox_source->currentIndex() >= 0)) {
    wizard()->button(QWizard::CustomButton1)->setEnabled(true);
  }
  ui->checkBoxSkipSetup->setEnabled(true);
  m_wizDlg->m_skipSetup = true;
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
      if (KMessageBox::questionYesNo(0, question2, i18n("Adding profile name.")) == KMessageBox::No) {
        ui->combobox_source->lineEdit()->clear();
        connect(ui->combobox_source->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotLineEditingFinished()));
        return ret;
      }
    }
    m_addRequested = false;
    ui->combobox_source->addItem(txt);
    indx = ui->combobox_source->findText(txt);
    ui->combobox_source->setCurrentIndex(indx);
    m_index = indx;
    ret = 0;
  }  else {  //  Already exists.

    if ((!m_addRequested) && (!m_editAccepted)) {
      if (KMessageBox::questionYesNo(0, question2, i18n("Adding profile name.")) == KMessageBox::No) {
        int indx = ui->combobox_source->findText(txt);
        ui->combobox_source->removeItem(indx);
        return ret;
      }
      m_index = indx;
    }
    if (!m_wizDlg->m_profileList.contains(txt)) {
      m_wizDlg->m_profileList << txt;
      m_wizDlg->createProfile(txt);
    }
    m_addRequested = false;
  }
  m_wizDlg->m_profileName = txt;
  connect(ui->combobox_source->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotLineEditingFinished()));
  return ret;
}

void IntroPage::initializePage()
{
  m_wizDlg->m_pageInvestment->m_investPageInitialized = false;
  m_wizDlg->m_pageBanking->m_bankingPageInitialized = false;
  m_wizDlg->ui->tableWidget->clear();
  m_wizDlg->ui->tableWidget->setColumnCount(0);
  m_wizDlg->ui->tableWidget->setRowCount(0);
  m_wizDlg->ui->tableWidget->verticalScrollBar()->setValue(0);
  m_wizDlg->ui->tableWidget->horizontalScrollBar()->setValue(0);

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

  m_wizDlg->m_importError = false;
  if (m_wizDlg->m_profileName.isEmpty() || m_wizDlg->m_profileName == "Add New Profile") {
    wizard()->button(QWizard::CustomButton1)->setEnabled(false);  // disable 'Select file' if no profile selected
  } else {
    wizard()->button(QWizard::CustomButton1)->setEnabled(true);  //  enable 'Select file' when profile selected
  }
  connect(ui->combobox_source, SIGNAL(activated(int)), this, SLOT(slotComboSourceClicked(int)));
  connect(ui->combobox_source->lineEdit(), SIGNAL(editingFinished()), this, SLOT(slotLineEditingFinished()));
  if (m_wizDlg->m_initialHeight == -1 || m_wizDlg->m_initialWidth == -1)
  {
    m_wizDlg->m_initialHeight = m_wizDlg->geometry().height();
    m_wizDlg->m_initialWidth = m_wizDlg->geometry().width();
  }
  //resize wizard to its initial size and center it
  m_wizDlg->setGeometry(
        QStyle::alignedRect(
          Qt::LeftToRight,
          Qt::AlignCenter,
          QSize(m_wizDlg->m_initialWidth, m_wizDlg->m_initialHeight),
          QApplication::desktop()->availableGeometry()
          )
        );
}

bool IntroPage::validatePage()
{
  if (!m_newProfileCreated.isEmpty()) {
    m_wizDlg->createProfile(m_newProfileCreated);
  }
  return true;
}

int IntroPage::nextId() const
{
  return CSVWizard::Page_Separator;
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
  m_priorName = m_wizDlg->m_profileName;
  m_priorIndex = m_index;
  m_wizDlg->m_profileName = newName;
  if (m_wizDlg->m_fileType == "Banking") {
    m_wizDlg->m_priorCsvProfile = m_wizDlg->m_profileName;
  } else {
    m_wizDlg->m_priorInvProfile = m_wizDlg->m_profileName;
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
      int indx = ui->combobox_source->findText(newName);
      ui->combobox_source->setCurrentIndex(indx);
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
  if (m_wizDlg->m_profileList.contains(newName)) {
    return;
  }
  if (m_action != "add") {
    editProfileName(m_priorName, newName);
  }
  m_wizDlg->m_profileName = newName;
  if (m_wizDlg->m_fileType == "Banking") {
    m_wizDlg->m_priorCsvProfile = m_wizDlg->m_profileName;
  } else {
    m_wizDlg->m_priorInvProfile = m_wizDlg->m_profileName;
  }
  m_wizDlg->m_profileList.append(m_wizDlg->m_profileName);
  m_wizDlg->createProfile(m_wizDlg->m_profileName);
  m_newProfileCreated = m_wizDlg->m_profileName;
  m_priorName = m_wizDlg->m_profileName;
  m_mapFileType.insert(m_wizDlg->m_profileName, m_wizDlg->m_fileType);
  m_priorIndex = ui->combobox_source->findText(m_wizDlg->m_profileName);
  if (m_priorIndex == -1) {
    ui->combobox_source->addItem(m_wizDlg->m_profileName);
  }
  m_priorIndex = ui->combobox_source->findText(m_wizDlg->m_profileName);
  if (m_priorIndex != -1)
    ui->combobox_source->setCurrentIndex(m_priorIndex);
  m_action.clear();
}

SeparatorPage::SeparatorPage(QWidget *parent) : QWizardPage(parent), ui(new Ui::SeparatorPage)
{
  ui->setupUi(this);

  m_pageLayout = new QVBoxLayout;
  ui->horizontalLayout->insertLayout(0, m_pageLayout);
}

SeparatorPage::~SeparatorPage()
{
  delete ui;
}

void SeparatorPage::setParent(CSVWizard* dlg)
{
  m_wizDlg = dlg;
}

void SeparatorPage::initializePage()
{
  disconnect(ui->comboBox_fieldDelimiter, SIGNAL(currentIndexChanged(int)), m_wizDlg, SLOT(delimiterChanged(int)));
  disconnect(ui->comboBox_fieldDelimiter, SIGNAL(activated(int)), this, SLOT(delimiterActivated()));
  ui->comboBox_fieldDelimiter->setCurrentIndex(m_wizDlg->m_fieldDelimiterIndex);
  ui->comboBox_textDelimiter->setCurrentIndex(m_wizDlg->m_textDelimiterIndex);
  connect(ui->comboBox_fieldDelimiter, SIGNAL(currentIndexChanged(int)), m_wizDlg, SLOT(delimiterChanged(int)));
  connect(ui->comboBox_fieldDelimiter, SIGNAL(activated(int)), this, SLOT(delimiterActivated()));

  QList<QWizard::WizardButton> layout;
  layout << QWizard::Stretch << QWizard::BackButton << QWizard::NextButton << QWizard::CancelButton;
  wizard()->setButtonLayout(layout);
}

void SeparatorPage::delimiterActivated()
{
  emit completeChanged();
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

  if (m_wizDlg->m_fileType == "Banking") {
    ret1 = ((m_wizDlg->m_endColumn > 2) && (!m_wizDlg->m_importError));
    ret2 = ((field("dateColumn").toInt() > -1) && (field("payeeColumn").toInt() > -1)  &&
            ((field("amountColumn").toInt() > -1) || ((field("debitColumn").toInt() > -1)  && (field("creditColumn").toInt() > -1))));
    ret3 = m_wizDlg->m_pageBanking->m_bankingPageInitialized;
    ret = (ret1 || (ret2 && ret3));
  } else if (m_wizDlg->m_fileType == "Invest") {
    ret1 = (m_wizDlg->m_endColumn > 3);
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
  return true;
}

void SeparatorPage::cleanupPage()
{
  //  On completion with error force use of 'Back' button.
  //  ...to allow resetting of 'Skip setup'
  disconnect(ui->comboBox_fieldDelimiter, SIGNAL(currentIndexChanged(int)), m_wizDlg, SLOT(delimiterChanged(int)));
  disconnect(ui->comboBox_fieldDelimiter, SIGNAL(activated(int)), this, SLOT(delimiterActivated()));
  m_wizDlg->m_pageIntro->initializePage();  //  Need to show button(QWizard::CustomButton1) not 'NextButton'
}

int SeparatorPage::nextId() const
{
  int ret;
  if (m_wizDlg->m_fileType == "Banking") {
    ret = CSVWizard::Page_Banking;
  } else {
    ret = CSVWizard::Page_Investment;
  }
  return ret;
}

BankingPage::BankingPage(QWidget *parent) : QWizardPage(parent), ui(new Ui::BankingPage)
{
  ui->setupUi(this);
  m_pageLayout = new QVBoxLayout;
  ui->horizontalLayout->insertLayout(0, m_pageLayout);

  registerField("dateColumn", ui->comboBoxBnk_dateCol, "currentIndex", SIGNAL(currentIndexChanged()));
  registerField("payeeColumn", ui->comboBoxBnk_payeeCol, "currentIndex", SIGNAL(currentIndexChanged()));
  registerField("amountColumn", ui->comboBoxBnk_amountCol, "currentIndex", SIGNAL(currentIndexChanged()));
  registerField("debitColumn", ui->comboBoxBnk_debitCol, "currentIndex", SIGNAL(currentIndexChanged()));
  registerField("creditColumn", ui->comboBoxBnk_creditCol, "currentIndex", SIGNAL(currentIndexChanged()));
  registerField("categoryColumn", ui->comboBoxBnk_categoryCol, "currentIndex", SIGNAL(currentIndexChanged()));

  connect(ui->comboBoxBnk_dateCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDateColChanged(int)));
  connect(ui->comboBoxBnk_amountCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotAmountColChanged(int)));
  connect(ui->comboBoxBnk_payeeCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotPayeeColChanged(int)));
  connect(ui->comboBoxBnk_debitCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDebitColChanged(int)));
  connect(ui->comboBoxBnk_creditCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotCreditColChanged(int)));
  connect(ui->comboBoxBnk_categoryCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotCategoryColChanged(int)));
}

BankingPage::~BankingPage()
{
  delete ui;
}

void BankingPage::setParent(CSVWizard* dlg)
{
  m_wizDlg = dlg;
}

void BankingPage::initializePage()
{
  // disable banking widgets allowing their initialization
  disconnect(ui->comboBoxBnk_amountCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_csvDialog, SLOT(amountColumnSelected(int)));
  disconnect(ui->comboBoxBnk_debitCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_csvDialog, SLOT(debitColumnSelected(int)));
  disconnect(ui->comboBoxBnk_creditCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_csvDialog, SLOT(creditColumnSelected(int)));
  disconnect(ui->comboBoxBnk_memoCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_csvDialog, SLOT(memoColumnSelected(int)));
  disconnect(ui->comboBoxBnk_numberCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_csvDialog, SLOT(numberColumnSelected(int)));
  disconnect(ui->comboBoxBnk_dateCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_csvDialog, SLOT(dateColumnSelected(int)));
  disconnect(ui->comboBoxBnk_payeeCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_csvDialog, SLOT(payeeColumnSelected(int)));
  disconnect(ui->comboBoxBnk_categoryCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_csvDialog, SLOT(categoryColumnSelected(int)));

  // clear all existing items before adding new ones
  ui->comboBoxBnk_numberCol->clear();
  ui->comboBoxBnk_dateCol->clear();
  ui->comboBoxBnk_payeeCol->clear();
  ui->comboBoxBnk_memoCol->clear();
  ui->comboBoxBnk_amountCol->clear();
  ui->comboBoxBnk_creditCol->clear();
  ui->comboBoxBnk_debitCol->clear();
  ui->comboBoxBnk_categoryCol->clear();

  QStringList columnNumbers;
  for (int i = 0; i < m_wizDlg->m_maxColumnCount; i++)
    columnNumbers << QString::number(i + 1);

  // populate comboboxes with col # values
  ui->comboBoxBnk_numberCol->addItems(columnNumbers);
  ui->comboBoxBnk_dateCol->addItems(columnNumbers);
  ui->comboBoxBnk_payeeCol->addItems(columnNumbers);
  ui->comboBoxBnk_memoCol->addItems(columnNumbers);
  ui->comboBoxBnk_amountCol->addItems(columnNumbers);
  ui->comboBoxBnk_creditCol->addItems(columnNumbers);
  ui->comboBoxBnk_debitCol->addItems(columnNumbers);
  ui->comboBoxBnk_categoryCol->addItems(columnNumbers);

  m_wizDlg->m_csvDialog->clearColumnNumbers(); // all comboboxes are set to 0 so set them to -1
  connect(ui->comboBoxBnk_amountCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_csvDialog, SLOT(amountColumnSelected(int)));
  connect(ui->comboBoxBnk_debitCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_csvDialog, SLOT(debitColumnSelected(int)));
  connect(ui->comboBoxBnk_creditCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_csvDialog, SLOT(creditColumnSelected(int)));
  connect(ui->comboBoxBnk_memoCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_csvDialog, SLOT(memoColumnSelected(int)));
  connect(ui->comboBoxBnk_numberCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_csvDialog, SLOT(numberColumnSelected(int)));
  connect(ui->comboBoxBnk_dateCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_csvDialog, SLOT(dateColumnSelected(int)));
  connect(ui->comboBoxBnk_payeeCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_csvDialog, SLOT(payeeColumnSelected(int)));
  connect(ui->comboBoxBnk_categoryCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_csvDialog, SLOT(categoryColumnSelected(int)));

  m_wizDlg->m_columnsNotSet = false;  // allow checking of columns now
  ui->comboBoxBnk_payeeCol->setCurrentIndex(m_wizDlg->m_csvDialog->m_colTypeNum.value(CSVDialog::ColumnPayee));
  ui->comboBoxBnk_numberCol->setCurrentIndex(m_wizDlg->m_csvDialog->m_colTypeNum.value(CSVDialog::ColumnNumber));
  ui->comboBoxBnk_amountCol->setCurrentIndex(m_wizDlg->m_csvDialog->m_colTypeNum.value(CSVDialog::ColumnAmount));
  ui->comboBoxBnk_debitCol->setCurrentIndex(m_wizDlg->m_csvDialog->m_colTypeNum.value(CSVDialog::ColumnDebit));
  ui->comboBoxBnk_creditCol->setCurrentIndex(m_wizDlg->m_csvDialog->m_colTypeNum.value(CSVDialog::ColumnCredit));
  ui->comboBoxBnk_dateCol->setCurrentIndex(m_wizDlg->m_csvDialog->m_colTypeNum.value(CSVDialog::ColumnDate));
  ui->comboBoxBnk_categoryCol->setCurrentIndex(m_wizDlg->m_csvDialog->m_colTypeNum.value(CSVDialog::ColumnCategory));
  ui->checkBoxBnk_oppositeSigns->setChecked(m_wizDlg->m_csvDialog->m_oppositeSigns);

  if (m_wizDlg->m_memoColList.count() > 0)
  {
    for (int i = 0; i < m_wizDlg->m_memoColList.count(); i++)
      ui->comboBoxBnk_memoCol->setCurrentIndex(m_wizDlg->m_memoColList[i]);
  } else
    ui->comboBoxBnk_memoCol->setCurrentIndex(-1);

  if (m_wizDlg->m_csvDialog->m_colTypeNum.value(CSVDialog::ColumnDebit) == -1)     // If amount previously selected, set check radio_amount
    ui->radioBnk_amount->setChecked(true);
  else                                     // ...else set check radio_debCred to clear amount col
    ui->radioBnk_debCred->setChecked(true);

  int index = m_wizDlg->m_pageIntro->ui->combobox_source->currentIndex();
  setField("source", index);
  m_wizDlg->m_fileType = "Banking";
  m_bankingPageInitialized = true;  //            Allow checking of columns now.
}

int BankingPage::nextId() const
{
  return CSVWizard::Page_LinesDate;
}

void BankingPage::cleanupPage()
{
  m_wizDlg->m_columnsNotSet = true;  // disallow checking of columns now
  disconnect(ui->comboBoxBnk_amountCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_csvDialog, SLOT(amountColumnSelected(int)));
  disconnect(ui->comboBoxBnk_debitCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_csvDialog, SLOT(debitColumnSelected(int)));
  disconnect(ui->comboBoxBnk_creditCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_csvDialog, SLOT(creditColumnSelected(int)));
  disconnect(ui->comboBoxBnk_memoCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_csvDialog, SLOT(memoColumnSelected(int)));
  disconnect(ui->comboBoxBnk_numberCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_csvDialog, SLOT(numberColumnSelected(int)));
  disconnect(ui->comboBoxBnk_dateCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_csvDialog, SLOT(dateColumnSelected(int)));
  disconnect(ui->comboBoxBnk_payeeCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_csvDialog, SLOT(payeeColumnSelected(int)));
  disconnect(ui->comboBoxBnk_categoryCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_csvDialog, SLOT(categoryColumnSelected(int)));
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

InvestmentPage::InvestmentPage(QWidget *parent) : QWizardPage(parent), ui(new Ui::InvestmentPage)
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
  registerField("nameCol", ui->comboBoxInv_nameCol, "currentIndex", SIGNAL(currentIndexChanged()));
  registerField("securityNameIndex", ui->comboBoxInv_securityName, "currentIndex", SIGNAL(currentIndexChanged()));

  connect(ui->comboBoxInv_dateCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDateColChanged(int)));
  connect(ui->comboBoxInv_typeCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotTypeColChanged(int)));
  connect(ui->comboBoxInv_quantityCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotQuantityColChanged(int)));
  connect(ui->comboBoxInv_priceCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotPriceColChanged(int)));
  connect(ui->comboBoxInv_amountCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotAmountColChanged(int)));
  connect(ui->comboBoxInv_symbolCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSymbolColChanged(int)));
  connect(ui->comboBoxInv_nameCol, SIGNAL(currentIndexChanged(int)), this, SLOT(slotNameColChanged(int)));

  connect(ui->lineEdit_filter, SIGNAL(returnPressed()), this, SLOT(slotFilterEditingFinished()));
  connect(ui->lineEdit_filter, SIGNAL(editingFinished()), this, SLOT(slotFilterEditingFinished()));
}

InvestmentPage::~InvestmentPage()
{
  delete ui;
}

void InvestmentPage::initializePage()
{
  // disable investment widgets allowing their initialization
  disconnect(ui->comboBoxInv_memoCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_investProcessing, SLOT(memoColumnSelected(int)));
  disconnect(ui->comboBoxInv_typeCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_investProcessing, SLOT(typeColumnSelected(int)));
  disconnect(ui->comboBoxInv_dateCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_investProcessing, SLOT(dateColumnSelected(int)));
  disconnect(ui->comboBoxInv_quantityCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_investProcessing, SLOT(quantityColumnSelected(int)));
  disconnect(ui->comboBoxInv_priceCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_investProcessing, SLOT(priceColumnSelected(int)));
  disconnect(ui->comboBoxInv_amountCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_investProcessing, SLOT(amountColumnSelected(int)));
  disconnect(ui->lineEdit_feeRate, SIGNAL(editingFinished()), m_wizDlg->m_investProcessing, SLOT(feeInputsChanged()));
  disconnect(ui->comboBoxInv_feeCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_investProcessing, SLOT(feeColumnSelected(int)));
  disconnect(ui->comboBoxInv_symbolCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_investProcessing, SLOT(symbolColumnSelected(int)));
  disconnect(ui->comboBoxInv_nameCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_investProcessing, SLOT(nameColumnSelected(int)));
  disconnect(ui->checkBoxInv_feeIsPercentage, SIGNAL(clicked(bool)), m_wizDlg->m_investProcessing, SLOT(feeIsPercentageCheckBoxClicked(bool)));
  disconnect(ui->comboBoxInv_securityName, SIGNAL(currentIndexChanged(int)), m_wizDlg, SLOT(slotsecurityNameChanged(int)));

  // clear all existing items before adding new ones
  ui->comboBoxInv_amountCol->clear(); // clear all existing items before adding new ones
  ui->comboBoxInv_dateCol->clear();
  ui->comboBoxInv_memoCol->clear();
  ui->comboBoxInv_priceCol->clear();
  ui->comboBoxInv_quantityCol->clear();
  ui->comboBoxInv_typeCol->clear();
  ui->lineEdit_feeRate->clear();
  ui->lineEdit_minFee->clear();
  ui->comboBoxInv_feeCol->clear();
  ui->comboBoxInv_symbolCol->clear();
  ui->comboBoxInv_nameCol->clear();
  ui->comboBoxInv_securityName->clear();

  m_wizDlg->m_investProcessing->m_columnTypeList.clear();
  QStringList columnNumbers;
  for (int i = 0; i < m_wizDlg->m_maxColumnCount; i++) {
    m_wizDlg->m_investProcessing->m_columnTypeList << QString();
    columnNumbers << QString::number(i + 1);
  }

  // populate comboboxes with col # values
  ui->comboBoxInv_amountCol->addItems(columnNumbers);
  ui->comboBoxInv_dateCol->addItems(columnNumbers);
  ui->comboBoxInv_memoCol->addItems(columnNumbers);
  ui->comboBoxInv_priceCol->addItems(columnNumbers);
  ui->comboBoxInv_quantityCol->addItems(columnNumbers);
  ui->comboBoxInv_typeCol->addItems(columnNumbers);
  ui->comboBoxInv_feeCol->addItems(columnNumbers);
  ui->comboBoxInv_symbolCol->addItems(columnNumbers);
  ui->comboBoxInv_nameCol->addItems(columnNumbers);
  ui->comboBoxInv_securityName->addItems(m_wizDlg->m_investProcessing->m_securityList);

  m_wizDlg->m_investProcessing->clearColumnsSelected(); // all comboboxes are set to 0 so set them to -1
  connect(ui->comboBoxInv_memoCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_investProcessing, SLOT(memoColumnSelected(int)));
  connect(ui->comboBoxInv_typeCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_investProcessing, SLOT(typeColumnSelected(int)));
  connect(ui->comboBoxInv_dateCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_investProcessing, SLOT(dateColumnSelected(int)));
  connect(ui->comboBoxInv_quantityCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_investProcessing, SLOT(quantityColumnSelected(int)));
  connect(ui->comboBoxInv_priceCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_investProcessing, SLOT(priceColumnSelected(int)));
  connect(ui->comboBoxInv_amountCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_investProcessing, SLOT(amountColumnSelected(int)));
  connect(ui->lineEdit_feeRate, SIGNAL(editingFinished()), m_wizDlg->m_investProcessing, SLOT(feeInputsChanged()));
  connect(ui->comboBoxInv_feeCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_investProcessing, SLOT(feeColumnSelected(int)));
  connect(ui->comboBoxInv_symbolCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_investProcessing, SLOT(symbolColumnSelected(int)));
  connect(ui->comboBoxInv_nameCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_investProcessing, SLOT(nameColumnSelected(int)));
  connect(ui->checkBoxInv_feeIsPercentage, SIGNAL(clicked(bool)), m_wizDlg->m_investProcessing, SLOT(feeIsPercentageCheckBoxClicked(bool)));
  connect(ui->comboBoxInv_securityName, SIGNAL(currentIndexChanged(int)), m_wizDlg, SLOT(slotsecurityNameChanged(int)));

  m_wizDlg->m_columnsNotSet = false;  // allow checking of columns now
  ui->comboBoxInv_dateCol->setCurrentIndex(m_wizDlg->m_dateColumn);
  ui->comboBoxInv_typeCol->setCurrentIndex(m_wizDlg->m_investProcessing->m_payeeColumn);
  ui->comboBoxInv_priceCol->setCurrentIndex(m_wizDlg->m_investProcessing->m_priceColumn);
  ui->comboBoxInv_quantityCol->setCurrentIndex(m_wizDlg->m_investProcessing->m_quantityColumn);
  ui->comboBoxInv_amountCol->setCurrentIndex(m_wizDlg->m_investProcessing->m_amountColumn);
  ui->comboBoxInv_nameCol->setCurrentIndex(m_wizDlg->m_investProcessing->m_nameColumn);
  ui->checkBoxInv_feeIsPercentage->setChecked(m_wizDlg->m_investProcessing->m_feeIsPercentage);
  ui->lineEdit_feeRate->setText(m_wizDlg->m_investProcessing->m_feeRate);
  ui->lineEdit_minFee->setText(m_wizDlg->m_investProcessing->m_minFee);
  ui->comboBoxInv_feeCol->setCurrentIndex(m_wizDlg->m_investProcessing->m_feeColumn);
  ui->comboBoxInv_securityName->setCurrentIndex(m_wizDlg->m_investProcessing->m_securityNameIndex);
  ui->lineEdit_filter->setText(QString());
  ui->lineEdit_filter->setText(m_wizDlg->m_investProcessing->m_nameFilter);
  ui->comboBoxInv_symbolCol->setCurrentIndex(m_wizDlg->m_investProcessing->m_symbolColumn);
  ui->lineEdit_feeRate->setValidator(new QRegExpValidator(QRegExp("[0-9]{1,2}[" + QLocale().decimalPoint() + "]{0,1}[0-9]{0,2}"), m_wizDlg->m_investProcessing) );
  ui->lineEdit_minFee->setValidator(new QRegExpValidator(QRegExp("[0-9]{1,}[" + QLocale().decimalPoint() + "]{0,1}[0-9]{0,}"), m_wizDlg->m_investProcessing) );
  ui->comboBoxInv_priceFraction->setCurrentIndex(m_wizDlg->m_investProcessing->m_priceFraction);

  for (int i = 0; i < m_wizDlg->m_memoColList.count(); i++) { //  Set up all memo fields...
    int tmp = m_wizDlg->m_memoColList[i];
    if (tmp < m_wizDlg->m_investProcessing->m_columnTypeList.count())
      ui->comboBoxInv_memoCol->setCurrentIndex(tmp);
  }

  int index = m_wizDlg->m_pageIntro->ui->combobox_source->currentIndex();
  setField("source", index);
  m_wizDlg->m_fileType = "Invest";

  m_investPageInitialized = true;
  m_wizDlg->m_investProcessing->feeInputsChanged();
  connect(ui->buttonInv_calculateFee, SIGNAL(clicked()), m_wizDlg->m_investProcessing, SLOT(calculateFee()));
  connect(ui->buttonInv_hideSecurity, SIGNAL(clicked()), m_wizDlg->m_investProcessing, SLOT(hideSecurity()));
}

void InvestmentPage::cleanupPage()
{
  m_wizDlg->m_columnsNotSet = true;  // disallow checking of columns now
  disconnect(ui->comboBoxInv_memoCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_investProcessing, SLOT(memoColumnSelected(int)));
  disconnect(ui->comboBoxInv_typeCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_investProcessing, SLOT(typeColumnSelected(int)));
  disconnect(ui->comboBoxInv_dateCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_investProcessing, SLOT(dateColumnSelected(int)));
  disconnect(ui->comboBoxInv_quantityCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_investProcessing, SLOT(quantityColumnSelected(int)));
  disconnect(ui->comboBoxInv_priceCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_investProcessing, SLOT(priceColumnSelected(int)));
  disconnect(ui->comboBoxInv_amountCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_investProcessing, SLOT(amountColumnSelected(int)));
  disconnect(ui->lineEdit_feeRate, SIGNAL(editingFinished()), m_wizDlg->m_investProcessing, SLOT(feeInputsChanged()));
  disconnect(ui->comboBoxInv_feeCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_investProcessing, SLOT(feeColumnSelected(int)));
  disconnect(ui->comboBoxInv_symbolCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_investProcessing, SLOT(symbolColumnSelected(int)));
  disconnect(ui->comboBoxInv_nameCol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_investProcessing, SLOT(nameColumnSelected(int)));
  disconnect(ui->checkBoxInv_feeIsPercentage, SIGNAL(clicked(bool)), m_wizDlg->m_investProcessing, SLOT(feeIsPercentageCheckBoxClicked(bool)));
  disconnect(ui->comboBoxInv_securityName, SIGNAL(currentIndexChanged(int)), m_wizDlg, SLOT(slotsecurityNameChanged(int)));

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

void InvestmentPage::slotNameColChanged(int col)
{
  setField("nameCol", col);
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
  int nameCol = ui->comboBoxInv_nameCol->currentIndex();
  if (index != -1) {  //  There is a security name
    setField("symbolCol", -1);
    setField("nameCol", -1);
    ui->comboBoxInv_symbolCol->setCurrentIndex(-1);
    ui->comboBoxInv_nameCol->setCurrentIndex(-1);
    if ((symbolCol != -1) && (nameCol != -1)) {
      m_wizDlg->m_investProcessing->clearColumnType(symbolCol);
      m_wizDlg->m_investProcessing->clearColumnType(nameCol);
    }
  }
  emit completeChanged();
}

void InvestmentPage::slotFilterEditingFinished()
{
  m_wizDlg->m_investProcessing->m_nameFilter = ui->lineEdit_filter->text();
}

void InvestmentPage::setParent(CSVWizard* dlg)
{
  m_wizDlg = dlg;
}

bool InvestmentPage::isComplete() const
{  
  bool ret = ((field("symbolCol").toInt() > -1) || (field("nameCol").toInt() > -1) || (field("securityNameIndex").toInt() > -1)) &&
             (field("dateCol").toInt() > -1) && (field("typeCol").toInt() > -1) &&
             (field("quantityCol").toInt() > -1) && (field("priceCol").toInt() > -1) && (field("amountCol").toInt() > -1);
  return ret;
}

LinesDatePage::LinesDatePage(QWidget *parent) : QWizardPage(parent), ui(new Ui::LinesDatePage)
{
  ui->setupUi(this);
  m_pageLayout = new QVBoxLayout;
  ui->horizontalLayout->insertLayout(0, m_pageLayout);

  registerField("dateFormat", ui->comboBox_dateFormat, "currentIndex", SIGNAL(currentIndexChanged()));
}

LinesDatePage::~LinesDatePage()
{
  delete ui;
}

void LinesDatePage::initializePage()
{
  disconnect(ui->spinBox_skip, SIGNAL(valueChanged(int)), this, SLOT(startLineChanged(int)));
  disconnect(ui->spinBox_skipToLast, SIGNAL(valueChanged(int)), this, SLOT(endLineChanged(int)));
  disconnect(ui->comboBox_dateFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(dateFormatSelected(int)));

  if( m_wizDlg->m_fileType == "Banking")
    m_wizDlg->m_dateColumn = m_wizDlg->m_csvDialog->m_colTypeNum.value(CSVDialog::ColumnDate);

  ui->spinBox_skip->setMaximum(m_wizDlg->m_fileEndLine);
  ui->spinBox_skipToLast->setMaximum(m_wizDlg->m_fileEndLine);
  ui->spinBox_skip->setValue(m_wizDlg->m_startLine);
  ui->spinBox_skipToLast->setValue(m_wizDlg->m_endLine);

  m_wizDlg->markUnwantedRows();

  connect(ui->spinBox_skip, SIGNAL(valueChanged(int)), this, SLOT(startLineChanged(int)));
  connect(ui->spinBox_skipToLast, SIGNAL(valueChanged(int)), this, SLOT(endLineChanged(int)));
  connect(ui->comboBox_dateFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(dateFormatSelected(int)));
  ui->comboBox_dateFormat->setCurrentIndex(m_wizDlg->m_dateFormatIndex);

  QList<QWizard::WizardButton> layout;
  layout << QWizard::Stretch << QWizard::BackButton <<  QWizard::NextButton <<  QWizard::CancelButton;
  wizard()->setButtonLayout(layout);
}

void LinesDatePage::setParent(CSVWizard* dlg)
{
  m_wizDlg = dlg;
}

bool LinesDatePage::isComplete() const
{
  bool ret = (field("dateFormat").toInt() > -1);
  return ret;
}

void LinesDatePage::dateFormatSelected(int dF)
{
  if (dF == -1)
    return;
  if (m_wizDlg->validateDateFormat(dF))
  {
    m_wizDlg->m_dateFormatIndex = dF;
    m_wizDlg->m_date = m_wizDlg->m_dateFormats[m_wizDlg->m_dateFormatIndex];
  } else {
    ui->comboBox_dateFormat->blockSignals(true);
    ui->comboBox_dateFormat->setCurrentIndex(-1);
    setField("dateFormat", -1);
    ui->comboBox_dateFormat->blockSignals(false);
  }
  emit completeChanged();
}

void LinesDatePage::startLineChanged(int val)
{
  if (val > m_wizDlg->m_fileEndLine) {
    ui->spinBox_skip->setValue(m_wizDlg->m_fileEndLine);
    return;
  }
  if (val > m_wizDlg->m_endLine) {
    ui->spinBox_skip->setValue(m_wizDlg->m_endLine);
    return;
  }
  m_wizDlg->m_startLine = val;
  if (!m_wizDlg->m_inFileName.isEmpty()) {
    m_wizDlg->m_vScrollBar->setValue(m_wizDlg->m_startLine - 1);
    m_wizDlg->markUnwantedRows();
  }
}

void LinesDatePage::endLineChanged(int val)
{
  if (val > m_wizDlg->m_fileEndLine) {
    ui->spinBox_skipToLast->setValue(m_wizDlg->m_fileEndLine);
    return;
  }
  if (val < m_wizDlg->m_startLine) {
    ui->spinBox_skipToLast->setValue(m_wizDlg->m_startLine);
    return;
  }
  m_trailerLines = m_wizDlg->m_fileEndLine - val;
  m_wizDlg->m_endLine = val;
  if (!m_wizDlg->m_inFileName.isEmpty()) {
    m_wizDlg->markUnwantedRows();
  }
}

bool LinesDatePage::validatePage()
{
  bool ok;
  QString value;
  QString pattern = QString("[%1(), $]").arg(QLocale().currencySymbol());
  //
  //  Ensure numeric columns do contain valid numeric values
  //
  if (m_wizDlg->m_fileType == "Banking") {
    for (int row = m_wizDlg->m_startLine - 1; row < m_wizDlg->m_endLine; row++) {
      for (int col = 0; col < m_wizDlg->ui->tableWidget->columnCount(); col++) {
        if (m_wizDlg->ui->tableWidget->item(row, col) == 0) {  //  Does cell exist?
          break;  //  No.
        }
        if (m_wizDlg->m_csvDialog->m_colNumType.value(col) == CSVDialog::ColumnAmount ||
            m_wizDlg->m_csvDialog->m_colNumType.value(col) == CSVDialog::ColumnDebit ||
            m_wizDlg->m_csvDialog->m_colNumType.value(col) == CSVDialog::ColumnCredit) {
          value = m_wizDlg->ui->tableWidget->item(row, col)->text().remove(QRegExp(pattern));
          if (value.isEmpty()) {  //  An empty cell is OK, probably.
            continue;
          }
          value.toDouble(&ok); // Test validity.
          if ((!ok) && (!m_wizDlg->m_acceptAllInvalid)) {
            QString str = QLocale().currencySymbol();
            int rc = KMessageBox::questionYesNoCancel(this, i18n("<center>An invalid value has been detected in column %1 on row %2.</center>"
                     "Please check that you have selected the correct columns."
                     "<center>You may accept all similar items, or just this one, or cancel.</center>",
                     col + 1, row + 1), i18n("CSV import"),
                     KGuiItem(i18n("Accept All")),
                     KGuiItem(i18n("Accept This")),
                     KGuiItem(i18n("Cancel")));
            switch (rc) {
              case KMessageBox::Yes:  // Accept All
                m_wizDlg->m_acceptAllInvalid = true;
                continue;

              case KMessageBox::No:  // Accept This
                m_wizDlg->m_acceptAllInvalid = false;
                continue;

              case KMessageBox::Cancel:
                m_wizDlg->m_importIsValid = false;
                return false;
            }
          }
        }
      }
    }
    m_wizDlg->m_importIsValid = true;
  } else {  //  "Invest"
    m_wizDlg->m_investProcessing->m_symbolTableDlg->m_validRowCount = 0;
    for (int row = m_wizDlg->m_startLine - 1; row < m_wizDlg->m_endLine; row++) {
      for (int col = 0; col < m_wizDlg->ui->tableWidget->columnCount(); col++) {
        if ((m_wizDlg->m_investProcessing->columnType(col) == "amount") || (m_wizDlg->m_investProcessing->columnType(col) == "quantity") || (m_wizDlg->m_investProcessing->columnType(col) == "price")) {
          if (m_wizDlg->ui->tableWidget->item(row, col) == 0) {  //  Does cell exist?
            break;  //  No.
          }
          value = m_wizDlg->ui->tableWidget->item(row, col)->text().remove(QRegExp(pattern));
          value = value.remove("--");  //  Possible blank marker.
          if (value.isEmpty()) {  //       An empty cell is OK, probably.
            continue;
          }
          value.toDouble(&ok); // Test validity.
          if ((!ok) && (!m_wizDlg->m_acceptAllInvalid)) {
            QString str = QLocale().currencySymbol();
            int rc = KMessageBox::questionYesNoCancel(this, i18n("<center>An invalid value has been detected in column %1 on row %2.</center>"
                     "Please check that you have selected the correct columns."
                     "<center>You may accept all similar items, or just this one, or cancel.</center>",
                     col + 1, row + 1), i18n("CSV import"),
                     KGuiItem(i18n("Accept All")),
                     KGuiItem(i18n("Accept This")),
                     KGuiItem(i18n("Cancel")));
            switch (rc) {
              case KMessageBox::Yes:  //  = "Accept All"
                m_wizDlg->m_acceptAllInvalid = true;
                continue;

              case KMessageBox::No:  //  "Accept This"
                m_wizDlg->m_acceptAllInvalid = false;
                continue;

              case KMessageBox::Cancel:
                m_wizDlg->m_importIsValid = false;
                return false;
            }
          }
        }
      }
    }
    m_wizDlg->m_importIsValid = true;
    int symTableRow = -1;
    if (m_wizDlg->m_investProcessing->m_symbolTableScanned) {
      return true;
    }
    disconnect(m_wizDlg->m_investProcessing->m_symbolTableDlg->m_widget->tableWidget, SIGNAL(cellChanged(int,int)), 0, 0);

    MyMoneyStatement::Security security;
    MyMoneyFile* file = MyMoneyFile::instance();
    MyMoneySecurity sec;
    QList<MyMoneySecurity> list = file->securityList();

    //  No security name chosen so scan entries...if not already checked,
    //  to save user having to re-edit security names if having to re-import.
    if ((field("securityNameIndex").toInt() == -1)  && (!m_wizDlg->m_investProcessing->m_symbolTableScanned)) {
      QString symbl;
      QString securityName;
      for (int row = m_wizDlg->m_startLine - 1; row < m_wizDlg->m_endLine; row++) {
        int symbolCol = m_wizDlg->m_pageInvestment->ui->comboBoxInv_symbolCol->currentIndex();
        int nameCol = m_wizDlg->m_pageInvestment->ui->comboBoxInv_nameCol->currentIndex();

        if (m_wizDlg->ui->tableWidget->item(row, symbolCol) == 0 &&
            m_wizDlg->ui->tableWidget->item(row, nameCol) == 0) {  //  This cell does not exist
          continue;
        }

        bool exists = false;
        QString name;
        QList<MyMoneySecurity>::ConstIterator it = list.constBegin();
        if (symbolCol > -1) {
          name.clear();
          symbl = m_wizDlg->ui->tableWidget->item(row, symbolCol)->text().toUpper().trimmed();
          // Check if we already have the security on file.
          if (!symbl.isEmpty())  {
            while (it != list.constEnd()) {
              sec = *it;
              if (symbl.compare(sec.tradingSymbol(), Qt::CaseInsensitive) == 0) {  // symbol already exists
                exists = true;
                name = sec.name();
                break;
              }
              ++it;
            }
          }
          if (!exists && nameCol > -1) {
            name = m_wizDlg->ui->tableWidget->item(row, nameCol)->text().trimmed();
          }
        } else if (nameCol > -1) {
          name = m_wizDlg->ui->tableWidget->item(row, nameCol)->text().trimmed();
          symbl.clear();
          // Check if we already have the security on file.
          if (!name.isEmpty())  {
            while (it != list.constEnd()) {
              sec = *it;
              if (name.compare(sec.name(), Qt::CaseInsensitive) == 0) { //  name already exists
                exists = true;
                symbl = sec.tradingSymbol();
                break;
              }
              ++it;
            }
          }
        } else
          continue;

        symTableRow ++;
        m_wizDlg->m_investProcessing->m_symbolTableDlg->displayLine(symTableRow, symbl, name, exists);
        if (!symbl.isEmpty()) {
          m_wizDlg->m_investProcessing->m_symbolsList << symbl;
          if (!name.isEmpty())
            m_wizDlg->m_investProcessing->m_map.insert(symbl, name);
        }
      }

      if (symTableRow > -1) {
        int ret = m_wizDlg->m_investProcessing->m_symbolTableDlg->exec();
        if (ret == QDialog::Rejected) {
          m_wizDlg->m_importIsValid = false;
          m_wizDlg->m_importError = true;
          m_wizDlg->m_investProcessing->m_symbolTableScanned = false;
          return false;
        } else {
          m_wizDlg->m_investProcessing->m_symbolTableScanned = true;
        }
      }
    }
    connect(m_wizDlg->m_investProcessing->m_symbolTableDlg->m_widget->tableWidget,  SIGNAL(itemChanged(QTableWidgetItem*)), m_wizDlg->m_investProcessing->m_symbolTableDlg,  SLOT(slotItemChanged(QTableWidgetItem*)));
  }

  return true;
}

void LinesDatePage::cleanupPage()
{
  disconnect(ui->spinBox_skip, SIGNAL(valueChanged(int)), this, SLOT(startLineChanged(int)));
  disconnect(ui->spinBox_skipToLast, SIGNAL(valueChanged(int)), this, SLOT(endLineChanged(int)));
  disconnect(ui->comboBox_dateFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(dateFormatSelected(int)));
  m_wizDlg->clearBackground();
  if (m_wizDlg->m_fileType == "Banking")
    m_wizDlg->m_pageBanking->initializePage();
  else
    m_wizDlg->m_pageInvestment->initializePage();
}

int LinesDatePage::nextId() const
{
  m_wizDlg->m_accept = false;
  return CSVWizard::Page_Completion;
}

CompletionPage::CompletionPage(QWidget* parent) : QWizardPage(parent), ui(new Ui::CompletionPage)
{
  ui->setupUi(this);

  m_pageLayout = new QVBoxLayout;
  ui->horizontalLayout->insertLayout(0, m_pageLayout);
}

CompletionPage::~CompletionPage()
{
  delete ui;
}

void CompletionPage::setParent(CSVWizard* dlg)
{
  m_wizDlg = dlg;
}

void CompletionPage::initializePage()
{
  connect(ui->comboBox_decimalSymbol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_parse, SLOT(decimalSymbolSelected(int)));
  connect(ui->comboBox_decimalSymbol, SIGNAL(currentIndexChanged(int)), m_wizDlg, SLOT(decimalSymbolSelected(int)));
  ui->comboBox_decimalSymbol->setCurrentIndex(m_wizDlg->m_decimalSymbolIndex);

  QList<QWizard::WizardButton> layout;
  layout << QWizard::Stretch
         << QWizard::CustomButton3
         << QWizard::CustomButton2
         << QWizard::BackButton
         << QWizard::FinishButton
         << QWizard::CancelButton;
  wizard()->setOption(QWizard::HaveCustomButton2, true);
  wizard()->setButtonText(QWizard::CustomButton2, i18n("Import CSV"));
  wizard()->setOption(QWizard::HaveCustomButton3, true);
  wizard()->setButtonText(QWizard::CustomButton3, i18n("Make QIF File"));
  wizard()->setButtonLayout(layout);
  wizard()->button(QWizard::CustomButton2)->setVisible(false);
  wizard()->button(QWizard::CustomButton3)->setVisible(false);
  wizard()->button(QWizard::FinishButton)->setVisible(false);
  m_wizDlg->decimalSymbolSelected();
  if (m_wizDlg->m_skipSetup)
      if (!m_wizDlg->m_importError)
        slotImportClicked();
}

void CompletionPage::slotImportValid()
{
  m_wizDlg->m_importIsValid = true;
  wizard()->button(QWizard::CustomButton2)->setVisible(true);
  wizard()->button(QWizard::CustomButton3)->setVisible(true);
  wizard()->button(QWizard::FinishButton)->setVisible(true);
}

void CompletionPage::slotImportClicked()
{
  m_wizDlg->hide(); //hide wizard so it will not cover accountselector
  if (m_wizDlg->m_fileType == "Banking")
    emit importBanking();
  else
    emit importInvestment();
  setFinalPage(true);
  emit completeChanged(); //close hidden window as it isn't needed anymore
}

void CompletionPage::cleanupPage()
{
  disconnect(ui->comboBox_decimalSymbol, SIGNAL(currentIndexChanged(int)), m_wizDlg->m_parse, SLOT(decimalSymbolSelected(int)));
  disconnect(ui->comboBox_decimalSymbol, SIGNAL(currentIndexChanged(int)), m_wizDlg, SLOT(decimalSymbolSelected(int)));
  m_wizDlg->clearBackground();
  QList<QWizard::WizardButton> layout;
  layout << QWizard::Stretch << QWizard::BackButton << QWizard::NextButton <<  QWizard::CancelButton;
  wizard()->setButtonLayout(layout);
  m_wizDlg->m_pageLinesDate->initializePage();
}

bool CompletionPage::validatePage()
{
  emit completeChanged();
  return true;
}

void CSVWizard::closeEvent(QCloseEvent *event)
{
  this->m_plugin->m_action->setEnabled(true);
  this->m_csvDialog->m_closing = true;
  event->accept();
}
