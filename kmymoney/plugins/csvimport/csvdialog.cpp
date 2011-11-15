/*******************************************************************************
*                                 csvdialog.cpp
*                              --------------------
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

#include "csvdialog.h"
#include "convdate.h"
#include "csvutil.h"
#include "investmentdlg.h"
#include "investprocessing.h"
#include "symboltabledlg.h"

#include "ui_csvdialog.h"
#include "ui_introwizardpage.h"
#include "ui_separatorwizardpage.h"
#include "ui_bankingwizardpage.h"
#include "ui_lines-datewizardpage.h"
#include "ui_completionwizardpage.h"
#include "ui_investmentwizardpage.h"

#include <QtGui/QWizard>
#include <QtGui/QWizardPage>
#include <QCloseEvent>
#include <QDebug>
#include <QDesktopWidget>
#include <QVBoxLayout>
#include <QFrame>
#include <QGridLayout>
#include <QTableWidget>
#include <QWizard>
#include <QWizardPage>
#include <QTextCodec>

// ----------------------------------------------------------------------------
// KDE Headers

#include <kdeversion.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>
#include <kvbox.h>
#include <KAction>
#include <KSharedConfig>
#include <KMessageBox>
#include <KInputDialog>
#include <KFileDialog>
#include <KFileWidget>
#include <KStandardDirs>
#include <KLocale>
#include <KIO/NetAccess>
#include "KAboutApplicationDialog"
#include <KAboutData>
#include "mymoneyfile.h"

// ----------------------------------------------------------------------------

CSVDialog::CSVDialog(QWidget *parent) : QWidget(parent), ui(new Ui::CSVDialog)
{
  ui->setupUi(this);
  resize(800, 600);

  m_amountSelected = false;
  m_creditSelected = false;
  m_debitSelected = false;
  m_dateSelected = false;
  m_payeeSelected = false;
  m_memoSelected = false;
  m_numberSelected = false;
  m_duplicate = false;

  m_amountColumn = -1;
  m_creditColumn = -1;
  m_dateColumn = 0;
  m_debitColumn = -1;
  m_memoColumn = 0;
  m_numberColumn = 0;
  m_payeeColumn = 0;
  m_previousColumn = 0;
  m_maxColumnCount = 0;
  m_endLine = 0;
  m_startLine = 1;

  m_curId = -1;
  m_lastId = -1;

  m_decimalSymbol.clear();
  m_previousType.clear();
  m_thousandsSeparator = ',';

  m_iconBack = QPixmap(KIconLoader::global()->loadIcon("go-previous", KIconLoader::Small, KIconLoader::DefaultState));
  m_iconFinish = QPixmap(KIconLoader::global()->loadIcon("dialog-ok-apply", KIconLoader::Small, KIconLoader::DefaultState));
  m_iconCancel = QPixmap(KIconLoader::global()->loadIcon("dialog-cancel", KIconLoader::Small, KIconLoader::DefaultState));
  m_iconCSV = QPixmap(KIconLoader::global()->loadIcon("kmymoney", KIconLoader::Small, KIconLoader::DefaultState));
  m_iconImport = QPixmap(KIconLoader::global()->loadIcon("system-file-manager.", KIconLoader::Small, KIconLoader::DefaultState));
  m_iconQIF = QPixmap(KIconLoader::global()->loadIcon("invest-applet", KIconLoader::Small, KIconLoader::DefaultState));
}

void CSVDialog::init()
{
  m_investProcessing = new InvestProcessing;
  m_investProcessing->m_csvDialog = this;


  m_wizard = new QWizard;
  ui->horizontalLayout->insertWidget(2, m_wizard, 0);
  ui->horizontalLayout->setStretch(0, 150);
  ui->horizontalLayout->setStretch(1, 1);
  ui->horizontalLayout->setStretch(2, 350);
  ui->horizontalLayout->setStretch(3, 1);

  this->setAutoFillBackground(true);

  ui->frame_stage->setPalette(QPalette(Qt::gray));

  QPalette pal = ui->frame_stage->palette();
  pal.setColor(ui->frame_stage->backgroundRole(), QColor(40, 40, 40, 0));
  ui->frame_stage->setPalette(pal);
  ui->frame_stage->setAutoFillBackground(true);


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

  m_parse = new Parse;
  m_parse->m_csvDialog = this;
  m_convertDate = new ConvertDate;

  m_pageIntro = new IntroPage;
  m_wizard->setPage(Page_Intro, m_pageIntro);
  m_pageIntro->setParent(this);


  m_investmentDlg = new InvestmentDlg;
  m_investmentDlg->m_investProcessing = m_investProcessing;
  m_investmentDlg->m_csvDialog = this;
  m_investProcessing->m_convertDat = m_convertDate;

  m_symbolTableDlg  = new SymbolTableDlg;
  m_symbolTableDlg->m_csvDialog = this;

  m_investProcessing->m_parse = m_parse;

  m_pageSeparator = new SeparatorPage;
  m_wizard->setPage(Page_Separator, m_pageSeparator);
  m_pageSeparator->setParent(this);

  m_pageBanking = new BankingPage;
  m_wizard->setPage(Page_Banking, m_pageBanking);

  m_pageInvestment = new InvestmentPage;
  m_wizard->setPage(Page_Investment, m_pageInvestment);
  m_pageInvestment->setParent(this);

  m_pageLinesDate = new LinesDatePage;
  m_wizard->setPage(Page_LinesDate, m_pageLinesDate);
  m_pageLinesDate->setParent(this);

  m_pageCompletion = new CompletionPage;
  m_wizard->setPage(Page_Completion, m_pageCompletion);
  m_pageCompletion->setParent(this);

  this->setAttribute(Qt::WA_DeleteOnClose, true);

  ui->tableWidget->setWordWrap(false);
  m_pageCompletion->ui->comboBox_decimalSymbol->setCurrentIndex(-1);
  m_pageCompletion->ui->comboBox_thousandsDelimiter->setEnabled(false);

  m_pageInvestment->ui->comboBoxInv_feeCol->setCurrentIndex(-1);//  This col might not get selected, so clear it
  m_pageInvestment->ui->comboBoxInv_memoCol->setCurrentIndex(-1);// ditto
  m_pageSeparator->ui->comboBox_fieldDelimiter->setEnabled(false);

  m_pageInvestment->ui->comboBoxInv_securityName->setCurrentIndex(-1);
  m_pageInvestment->ui->comboBoxInv_securityName->setInsertPolicy(QComboBox::InsertAlphabetically);
  m_pageInvestment->ui->comboBoxInv_securityName->setDuplicatesEnabled(false);

  m_setColor.setRgb(0, 255, 127, 100);
  m_errorColor.setRgb(255, 0, 127, 100);
  m_clearColor.setRgb(255, 255, 255, 255);
  m_colorBrush.setColor(m_setColor);
  m_clearBrush.setColor(m_clearColor);
  m_colorBrush.setStyle(Qt::SolidPattern);
  m_clearBrush.setStyle(Qt::SolidPattern);
  m_errorBrush.setColor(m_errorColor);
  m_errorBrush.setStyle(Qt::SolidPattern);

  for(int i = 0; i < MAXCOL; i++) {  //  populate comboboxes with col # values
    QString t;
    t.setNum(i + 1);
    m_pageBanking->ui->comboBoxBnk_numberCol->addItem(t) ;
    m_pageBanking->ui->comboBoxBnk_dateCol->addItem(t) ;
    m_pageBanking->ui->comboBoxBnk_payeeCol->addItem(t) ;
    m_pageBanking->ui->comboBoxBnk_memoCol->addItem(t) ;
    m_pageBanking->ui->comboBoxBnk_amountCol->addItem(t) ;
    m_pageBanking->ui->comboBoxBnk_creditCol->addItem(t) ;
    m_pageBanking->ui->comboBoxBnk_debitCol->addItem(t) ;

    m_columnType[i].clear();//          clear all column types
  }

  m_pageBanking->ui->comboBoxBnk_numberCol->setMaxVisibleItems(12);
  m_pageBanking->ui->comboBoxBnk_dateCol->setMaxVisibleItems(12);
  m_pageBanking->ui->comboBoxBnk_payeeCol->setMaxVisibleItems(12);
  m_pageBanking->ui->comboBoxBnk_memoCol->setMaxVisibleItems(12);
  m_pageBanking->ui->comboBoxBnk_amountCol->setMaxVisibleItems(12);
  m_pageBanking->ui->comboBoxBnk_creditCol->setMaxVisibleItems(12);
  m_pageBanking->ui->comboBoxBnk_debitCol->setMaxVisibleItems(12);


  int screenWidth = QApplication::desktop()->width();
  int screenHeight = QApplication::desktop()->height();
  int x = (screenWidth - width()) / 2;
  int y = (screenHeight - height()) / 2;

  this->move(x, y);

  m_dateFormats << "yyyy/MM/dd" << "MM/dd/yyyy" << "dd/MM/yyyy";

  m_stageLabels << ui->label_intro << ui->label_separator << ui->label_banking << ui->label_investing << ui->label_lines << ui->label_finish;

  m_endColumn = MAXCOL;
  clearSelectedFlags();

  m_dateFormatIndex = m_pageLinesDate->ui->comboBox_dateFormat->currentIndex();
  m_date = m_dateFormats[m_dateFormatIndex];
  m_dateFormatIndex = m_dateFormatIndex;

  findCodecs();//                             returns m_codecs = codecMap.values();

  connect(m_pageIntro->ui->combobox_source, SIGNAL(activated(int)), m_pageIntro, SLOT(slotComboSourceClicked(int)));

  connect(m_wizard->button(QWizard::CustomButton1), SIGNAL(clicked()), this, SLOT(slotFileDialogClicked()));
  connect(m_wizard->button(QWizard::BackButton), SIGNAL(clicked()), this, SLOT(slotBackButtonClicked()));

  connect(m_pageBanking->ui->comboBoxBnk_amountCol, SIGNAL(currentIndexChanged(int)), this, SLOT(amountColumnSelected(int)));
  connect(m_pageBanking->ui->comboBoxBnk_amountCol, SIGNAL(activated(int)), this, SLOT(amountColumnSelected(int)));
  connect(m_pageBanking->ui->comboBoxBnk_debitCol, SIGNAL(currentIndexChanged(int)), this, SLOT(debitColumnSelected(int)));
  connect(m_pageBanking->ui->comboBoxBnk_debitCol, SIGNAL(activated(int)), this, SLOT(debitColumnSelected(int)));
  connect(m_pageBanking->ui->comboBoxBnk_creditCol, SIGNAL(currentIndexChanged(int)), this, SLOT(creditColumnSelected(int)));
  connect(m_pageBanking->ui->comboBoxBnk_creditCol, SIGNAL(activated(int)), this, SLOT(creditColumnSelected(int)));
  connect(m_pageBanking->ui->comboBoxBnk_memoCol, SIGNAL(currentIndexChanged(int)), this, SLOT(memoColumnSelected(int)));
  connect(m_pageBanking->ui->comboBoxBnk_numberCol, SIGNAL(currentIndexChanged(int)), this, SLOT(numberColumnSelected(int)));
  connect(m_pageBanking->ui->comboBoxBnk_dateCol, SIGNAL(currentIndexChanged(int)), this, SLOT(dateColumnSelected(int)));
  connect(m_pageBanking->ui->comboBoxBnk_payeeCol, SIGNAL(currentIndexChanged(int)), this, SLOT(payeeColumnSelected(int)));
  connect(m_pageBanking->ui->radioBnk_amount, SIGNAL(clicked(bool)), this, SLOT(amountRadioClicked(bool)));
  connect(m_pageBanking->ui->radioBnk_debCred, SIGNAL(clicked(bool)), this, SLOT(debitCreditRadioClicked(bool)));
  connect(m_pageBanking->ui->button_clear, SIGNAL(clicked()), this, SLOT(clearColumnsSelected()));

  connect(m_pageSeparator->ui->comboBox_fieldDelimiter, SIGNAL(currentIndexChanged(int)), this, SLOT(delimiterChanged()));
  connect(m_pageSeparator->ui->comboBox_fieldDelimiter, SIGNAL(currentIndexChanged(int)), m_investProcessing, SLOT(fieldDelimiterChanged()));
  connect(m_pageSeparator->ui->comboBox_textDelimiter, SIGNAL(currentIndexChanged(int)), this, SLOT(delimiterChanged()));
  connect(m_pageSeparator, SIGNAL(initialiseIntroPage()), this, SLOT(setupIntroPage()));

  connect(m_pageLinesDate->ui->spinBox_skip, SIGNAL(valueChanged(int)), this, SLOT(startLineChanged(int)));
  connect(m_pageLinesDate->ui->spinBox_skip, SIGNAL(valueChanged(int)), m_investProcessing, SLOT(startLineChanged(int)));
  connect(m_pageLinesDate->ui->spinBox_skip, SIGNAL(editingFinished()), m_investProcessing, SLOT(startLineChanged()));
  connect(m_pageLinesDate->ui->spinBox_skipToLast, SIGNAL(valueChanged(int)), this, SLOT(endLineChanged(int)));
  connect(m_pageLinesDate->ui->spinBox_skipToLast, SIGNAL(editingFinished()), this, SLOT(endLineChanged()));
  connect(m_pageLinesDate->ui->spinBox_skipToLast, SIGNAL(valueChanged(int)), m_investProcessing, SLOT(endLineChanged(int)));
  connect(m_pageLinesDate->ui->spinBox_skipToLast, SIGNAL(editingFinished()), m_investProcessing, SLOT(endLineChanged()));
  connect(m_pageLinesDate->ui->comboBox_dateFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(dateFormatSelected(int)));
  connect(m_pageLinesDate->ui->comboBox_dateFormat, SIGNAL(currentIndexChanged(int)), m_convertDate, SLOT(dateFormatSelected(int)));

  connect(m_pageCompletion->ui->comboBox_decimalSymbol, SIGNAL(activated(int)), m_parse, SLOT(decimalSymbolSelected(int)));
  connect(m_pageCompletion->ui->comboBox_decimalSymbol, SIGNAL(activated(int)), this, SLOT(decimalSymbolSelected(int)));
  connect(m_pageCompletion, SIGNAL(importBanking()), this, SLOT(slotImportClicked()));
  connect(m_pageCompletion, SIGNAL(importInvestment()), m_investProcessing, SLOT(slotImportClicked()));
  connect(m_pageCompletion, SIGNAL(completeChanged()), this, SLOT(slotClose()));

  connect(m_wizard->button(QWizard::CustomButton2), SIGNAL(clicked()), m_pageCompletion, SLOT(slotImportClicked()));
  connect(m_wizard->button(QWizard::CustomButton3), SIGNAL(clicked()), this, SLOT(slotSaveAsQIF()));
  connect(m_wizard->button(QWizard::CancelButton), SIGNAL(clicked()), this, SLOT(slotClose()));
  connect(m_wizard, SIGNAL(currentIdChanged(int)), this, SLOT(slotIdChanged(int)));

  connect(this, SIGNAL(isImportable()), m_pageCompletion, SLOT(slotImportValid()));
  connect(this, SIGNAL(namesEdited()), this, SLOT(slotNamesEdited()));

  m_investmentDlg->init();

}//  CSVDialog

CSVDialog::~CSVDialog()
{
  delete ui;
}


void CSVDialog::readSettings()
{
  int tmp;
  QString txt;
  KSharedConfigPtr config = KSharedConfig::openConfig(KStandardDirs::locateLocal("config", "csvimporterrc"));

  KConfigGroup profileGroup(config, "Profile");

  txt = profileGroup.readEntry("FileType", QString());

  m_dateFormatIndex = profileGroup.readEntry("DateFormat", QString()).toInt();
  m_pageLinesDate->ui->comboBox_dateFormat->setCurrentIndex(m_dateFormatIndex);

  m_encodeIndex = profileGroup.readEntry("Encoding", QString()).toInt();

  m_fieldDelimiterIndex = profileGroup.readEntry("FieldDelimiter", QString()).toInt();
  m_pageSeparator->ui->comboBox_fieldDelimiter->setCurrentIndex(m_fieldDelimiterIndex);

  m_textDelimiterIndex = profileGroup.readEntry("TextDelimiter", QString()).toInt();
  m_pageSeparator->ui->comboBox_textDelimiter->setCurrentIndex(m_textDelimiterIndex);
  m_pageCompletion->ui->comboBox_decimalSymbol->setCurrentIndex(-1);
  m_pageCompletion->ui->comboBox_thousandsDelimiter->setCurrentIndex(-1);

  m_csvPath = profileGroup.readEntry("CsvDirectory", QString());

  m_debitFlag = profileGroup.readEntry("DebitFlag", QString().toInt());

  KConfigGroup columnsGroup(config, "Columns");

  if(columnsGroup.exists()) {
    tmp = columnsGroup.readEntry("DateCol", QString()).toInt();
    m_pageBanking->ui->comboBoxBnk_dateCol->setCurrentIndex(-1);
    m_pageBanking->ui->comboBoxBnk_dateCol->setCurrentIndex(tmp);

    tmp = columnsGroup.readEntry("PayeeCol", QString()).toInt();
    m_pageBanking->ui->comboBoxBnk_payeeCol->setCurrentIndex(-1);
    m_pageBanking->ui->comboBoxBnk_payeeCol->setCurrentIndex(tmp);

    tmp = columnsGroup.readEntry("AmountCol", QString()).toInt();
    if(tmp >= 0) {  //                            If amount previously selected, set check radio_amount
      m_pageBanking->ui->radioBnk_amount->setChecked(true);
      m_pageBanking->ui->labelBnk_amount->setEnabled(true);
      m_pageBanking->ui->labelBnk_credits->setEnabled(false);
      m_pageBanking->ui->labelBnk_debits->setEnabled(false);
    } else {//                                   ....else set check radio_debCred to clear amount col
      m_pageBanking->ui->radioBnk_debCred->setChecked(true);
      m_pageBanking->ui->labelBnk_credits->setEnabled(true);
      m_pageBanking->ui->labelBnk_debits->setEnabled(true);
      m_pageBanking->ui->labelBnk_amount->setEnabled(false);
    }
    m_pageBanking->ui->comboBoxBnk_amountCol->setCurrentIndex(-1);
    m_pageBanking->ui->comboBoxBnk_amountCol->setCurrentIndex(tmp);
    m_amountColumn = tmp;

    tmp = columnsGroup.readEntry("DebitCol", QString()).toInt();
    m_pageBanking->ui->comboBoxBnk_debitCol->setCurrentIndex(-1);
    m_pageBanking->ui->comboBoxBnk_debitCol->setCurrentIndex(tmp);

    tmp = columnsGroup.readEntry("CreditCol", QString()).toInt();
    m_pageBanking->ui->comboBoxBnk_creditCol->setCurrentIndex(-1);
    m_pageBanking->ui->comboBoxBnk_creditCol->setCurrentIndex(tmp);

    tmp = columnsGroup.readEntry("NumberCol", QString()).toInt();
    m_pageBanking->ui->comboBoxBnk_numberCol->setCurrentIndex(-1);
    m_pageBanking->ui->comboBoxBnk_numberCol->setCurrentIndex(tmp);
    m_pageBanking->ui->comboBoxBnk_memoCol->setCurrentIndex(-1);
  } else {
    m_pageBanking->ui->comboBoxBnk_dateCol->setCurrentIndex(-1);
    m_pageBanking->ui->comboBoxBnk_payeeCol->setCurrentIndex(-1);
    m_pageBanking->ui->comboBoxBnk_amountCol->setCurrentIndex(-1);
    m_pageBanking->ui->comboBoxBnk_debitCol->setCurrentIndex(-1);
    m_pageBanking->ui->comboBoxBnk_numberCol->setCurrentIndex(-1);
    m_pageBanking->ui->comboBoxBnk_numberCol->setCurrentIndex(-1);
    m_pageBanking->ui->comboBoxBnk_memoCol->setCurrentIndex(-1);
  }
}

void CSVDialog::slotFileDialogClicked()
{
  if(m_fileType != "Banking") {
    return;
  }
  KSharedConfigPtr config =
    KSharedConfig::openConfig(KStandardDirs::locateLocal("config", "csvimporterrc"));
  KConfigGroup profileGroup(config, "Profile");

  //  The "DebitFlag" setting is used to indicate whether or not to allow the user,
  //  via a dialog, to specify a column which contains a flag to indicate if the
  //  amount field is a debit ('a' or 'af'), a credit ('bij') (ING - Netherlands),
  //   or ignore ('-1').

  m_debitFlag = profileGroup.readEntry("DebitFlag", QString().toInt());
  m_pageCompletion->ui->comboBox_decimalSymbol->setEnabled(true);

  m_endLine = 0;
  m_flagCol = -1;
  m_decimalSymbolChanged = false;
  int posn;
  if(m_csvPath.isEmpty()) {
    m_csvPath = "~/";
  }
  QPointer<KFileDialog> dialog =
    new KFileDialog(KUrl("kfiledialog:///kmymoney-csvbank"),
                    i18n("*.csv *.PRN *.txt | CSV Files\n *|All files"), 0);

//  Add encoding selection to FileDialog
  KHBox* encodeBox = new KHBox();
  m_comboBoxEncode = new KComboBox(encodeBox);
  m_comboBoxEncode->setCurrentIndex(m_encodeIndex);
  setCodecList(m_codecs);
  connect(m_comboBoxEncode, SIGNAL(activated(int)), this, SLOT(encodingChanged(int)));

  dialog->fileWidget()->setCustomWidget("Encoding", m_comboBoxEncode);
  m_comboBoxEncode->setCurrentIndex(m_encodeIndex);
  dialog->setMode(KFile::File | KFile::ExistingOnly);
  if(dialog->exec() == QDialog::Accepted) {
    m_url = dialog->selectedUrl();
  }
  delete dialog;

  if(m_url.isEmpty())
    return;
  m_inFileName.clear();

  if(!KIO::NetAccess::download(m_url, m_inFileName, 0)) {
    KMessageBox::detailedError(0,
                               i18n("Error while loading file '%1'.", m_url.prettyUrl()),
                               KIO::NetAccess::lastErrorString(),
                               i18n("File access error"));
    return;
  }
  if(m_inFileName.isEmpty()) {
    return;
  }
  m_importNow = false;//                       Avoid attempting date formatting on headers
  clearComboBoxText();//                       to clear any '*' in memo combo text

  for(int i = 0; i < MAXCOL; i++)
    if(columnType(i) == "memo") {
      clearColumnType(i);   //    ensure no memo entries remain
    }

  //  set large table height to ensure resizing sees all lines in new file

  QRect rect = ui->tableWidget->geometry();
  rect.setHeight(9999);
  ui->tableWidget->setGeometry(rect);
  m_parse->setSymbolFound(false);

  readFile(m_inFileName, 0);
  m_csvPath = m_inFileName;
  posn = m_csvPath.lastIndexOf("/");
  m_csvPath.truncate(posn + 1);   //   keep last "/"

  QString str = "$HOME/" + m_csvPath.section('/', 3);
  profileGroup.writeEntry("CsvDirectory", str);//          save selected path
  profileGroup.writeEntry("Encoding", m_encodeIndex);//    ..and encoding
  profileGroup.config()->sync();
  enableInputs();

  //The following two items do not *Require* an entry so old values must be cleared.
  m_trData.number.clear();// this needs to be cleared or gets added to next transaction
  m_trData.memo.clear();//   this too, as neither might be overwritten by new data.

  setupNextPage();
}

void CSVDialog::setupNextPage()
{
  m_wizard->setOption(QWizard::HaveCustomButton1, false);
  QList<QWizard::WizardButton> layout;
  layout << QWizard::Stretch << QWizard::BackButton << QWizard::NextButton
         <<  QWizard::CancelButton;
  m_wizard->setButtonText(QWizard::CustomButton1, "Select File");
  m_wizard->setButtonLayout(layout);
  m_wizard->next();
  m_wizard->nextId();
}

void CSVDialog::setupIntroPage()
{
  QList<QWizard::WizardButton> layout;
  layout << QWizard::Stretch << QWizard::BackButton << QWizard::CustomButton1
         <<  QWizard::CancelButton;
  m_wizard->setButtonText(QWizard::CustomButton1, "Select Pile");
  m_wizard->setOption(QWizard::HaveCustomButton1, true);
  m_wizard->setButtonLayout(layout);
}

void CSVDialog::readFile(const QString& fname, int skipLines)
{
  MyMoneyStatement st = MyMoneyStatement();
  if(!fname.isEmpty()) {
    m_inFileName = fname;
  }
  ui->tableWidget->clear();//         including vert headers
  m_inBuffer.clear();
  m_outBuffer.clear();

  m_qifBuffer = "!Type:Bank\n";
  m_row = 0;
  m_maxColumnCount = 0;

  m_fieldDelimiterIndex = m_pageSeparator->ui->comboBox_fieldDelimiter->currentIndex();
  m_parse->setFieldDelimiterIndex(m_fieldDelimiterIndex);
  m_fieldDelimiterCharacter = m_parse->fieldDelimiterCharacter(m_fieldDelimiterIndex);
  m_textDelimiterIndex = m_pageSeparator->ui->comboBox_textDelimiter->currentIndex();
  m_parse->setTextDelimiterIndex(m_textDelimiterIndex);
  m_textDelimiterCharacter = m_parse->textDelimiterCharacter(m_textDelimiterIndex);

  QFile  m_inFile(m_inFileName);
  m_inFile.open(QIODevice::ReadOnly | QIODevice::Text);

  QTextStream inStream(&m_inFile);
  QTextCodec* codec = QTextCodec::codecForMib(m_encodeIndex);
  inStream.setCodec(codec);

  QString buf = inStream.readAll();

  //  Parse the buffer

  QStringList lineList = m_parse->parseFile(buf, skipLines, m_endLine);
  m_pageLinesDate->ui->spinBox_skipToLast->setValue(m_parse->lastLine());
  ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
  m_screenUpdated = false;
  m_endLine = m_parse->lastLine();

  //  Display the buffer

  for(int i = 0; i < lineList.count(); i++) {
    m_inBuffer = lineList[i];

    displayLine(m_inBuffer);

    if(m_importNow) {  //                        user now ready to continue
      int ret = (processQifLine(m_inBuffer));//  parse a line
      if(ret == KMessageBox::Ok) {
        csvImportTransaction(st);
      } else
        m_importNow = false;
    }
  }//                                            reached end of buffer

  //  Adjust table size (drop header lines)

  updateScreen();//
  ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
  m_pageLinesDate->ui->labelSet_skip->setEnabled(true);
  m_pageLinesDate->ui->spinBox_skip->setEnabled(true);
  m_endColumn = m_maxColumnCount;

  //  Export statement

  if(m_importNow) {
    emit statementReady(st);//  via CsvImporterPlugin::slotGetStatement(MyMoneyStatement& s)
    m_screenUpdated = true;
    m_importNow = false;
  }
  m_inFile.close();
}


void CSVDialog::displayLine(const QString& data)
{
  if(m_importNow) {
    if(m_pageBanking->ui->radioBnk_amount->isChecked()) {
      m_amountColumn = m_pageBanking->ui->comboBoxBnk_amountCol->currentIndex();// setAmountColumn
      m_debitColumn = -1;// setDebitColumn
      m_creditColumn = -1;
    } else {
      m_amountColumn = -1;
      m_debitColumn = m_pageBanking->ui->comboBoxBnk_debitCol->currentIndex();
      m_creditColumn = m_pageBanking->ui->comboBoxBnk_creditCol->currentIndex();
    }
  }
  int col = 0;

  m_parse->setFieldDelimiterIndex(m_pageSeparator->ui->comboBox_fieldDelimiter->currentIndex());
  m_fieldDelimiterCharacter = m_parse->fieldDelimiterCharacter(m_fieldDelimiterIndex);
  m_parse->setTextDelimiterIndex(m_pageSeparator->ui->comboBox_textDelimiter->currentIndex());
  m_textDelimiterCharacter = m_parse->textDelimiterCharacter(m_textDelimiterIndex);

  m_columnList = m_parse->parseLine(data);//                 split data into fields
  int columnCount = m_columnList.count();
  if(columnCount > m_maxColumnCount) //             maxColumnCount()
    m_maxColumnCount = columnCount;//               find maximum column count
  else
    columnCount = m_maxColumnCount;
  ui->tableWidget->setColumnCount(columnCount);
  m_pageBanking->ui->comboBoxBnk_dateCol->setMaxVisibleItems(m_maxColumnCount);
  m_inBuffer.clear();
  QStringList::const_iterator constIterator;
  QString txt;

  for(constIterator = m_columnList.constBegin(); constIterator != m_columnList.constEnd();
      ++constIterator) {
    txt = (*constIterator);

    QTableWidgetItem *item = new QTableWidgetItem;//             new item for UI
    item->setText(txt);
    ui->tableWidget->setRowCount(m_row + 1);
    ui->tableWidget->setItem(m_row, col, item);//       add items to UI here
    ui->tableWidget->resizeColumnToContents(col);
    m_inBuffer += txt + m_fieldDelimiterCharacter;
    col ++;
  }

  //  if last char. of last column added to UI (txt string) is not '"', ie an unterminated string
  //  remove the unwanted trailing m_fieldDelimiterCharacter
  if(!txt.endsWith('"')) {
    m_inBuffer = m_inBuffer.remove(-1, 1);
  }
  ++m_row;
}


int CSVDialog::processQifLine(QString& iBuff)//   parse input line
{
  QString newTxt;

  if(m_columnList.count() < m_endColumn) {
    if(!m_accept) {
      QString row = QString::number(m_row);
      int ret = KMessageBox::questionYesNoCancel(this, i18n("<center>Row number %1 does not have the expected number of columns.</center>"
                "<center>This might not be a problem, but it may be a header line.</center>"
                "<center>You may accept all similar items, or just this one, or cancel.</center>",
                row), i18n("CSV import"),
                KGuiItem(i18n("Accept All")),
                KGuiItem(i18n("Accept This")),
                KGuiItem(i18n("Cancel")));
      if(ret == KMessageBox::Cancel) {
        return ret;
      }
      if(ret == KMessageBox::Yes) {
        m_accept = true;
      }
    }
  }
  int neededFieldsCount = 0;//                        ensure essential fields are present
  QString memo;
  QString txt;
  iBuff = iBuff.remove(m_textDelimiterCharacter);
  memo.clear();//                                     memo & number may not have been used
  m_trData.number.clear();//                          .. so need to clear prior contents
  for(int i = 0; i < m_endColumn; i++) { //            check each column
    if(columnType(i) == "number") {
      txt = m_columnList[i];
      m_trData.number = txt;
      m_qifBuffer = m_qifBuffer + 'N' + txt + '\n';//     Number column
    }

    else if(columnType(i) == "date") {
      ++neededFieldsCount;
      txt = m_columnList[i];
      txt = txt.remove(m_textDelimiterCharacter);//      "16/09/2009
      QDate dat = m_convertDate->convertDate(txt);//     Date column
      if(dat == QDate()) {
        qDebug() << i18n("date ERROR");

        KMessageBox::sorry(this, i18n("<center>An invalid date has been detected during import.</center>"
                                      "<center><b>%1</b></center>"
                                      "Please check that you have set the correct date format."
                                      , txt), i18n("CSV import"));
        return KMessageBox::Cancel;
      }
      QString qifDate = dat.toString(m_dateFormats[m_dateFormatIndex]);
      m_qifBuffer = m_qifBuffer + 'D' + qifDate + '\n';
      m_trData.date = dat;
    }

    else if(columnType(i) == "payee") {
      ++neededFieldsCount;
      txt = m_columnList[i];
      txt.remove('~');//                              replace NL which was substituted
      txt = txt.remove('\'');
      m_trData.payee = txt;
      m_qifBuffer = m_qifBuffer + 'P' + txt + '\n';// Detail column
    }

    else if(columnType(i) == "amount") {  // Is this Amount column
      ++neededFieldsCount;

      //  For a file which uses a flag field value to indicate if amount is a debit or a credit.
      //  Resource file DebitFlag setting of -1 means 'ignore/notused'.
      //  DebitFlag setting of >=0 indicates the column containing the flag.

      if(m_flagCol == -1) {  //                        it's a new file
        switch(m_debitFlag) {  //                      Flag if amount is debit or credit
          case -1://                                  Ignore flag
            m_flagCol = 0;//                          ...and continue
            break;
          case  0://                                  Ask for column no.of flag
            m_flagCol = columnNumber(i18n("Enter debit flag column number"));
            if(m_flagCol == 0) {  //                  0 means Cancel was pressed
              return KMessageBox::Cancel;//           ... so exit
            }
            break;
          default : m_flagCol = m_debitFlag;//        Contains flag/column no.
        }
      }
      if((m_flagCol < 0) || (m_flagCol > m_endColumn)) {  // shouldn't get here
        KMessageBox::sorry(0, i18n("An invalid column was entered.\n"
                                   "Must be between 1 and %1.", m_endColumn), i18n("CSV import"));
        return KMessageBox::Cancel;
      }
      QString flag;//                                 m_flagCol == valid column (or zero)
      if(m_flagCol > 0) {
        flag = m_columnList[m_flagCol - 1];//         indicates if amount is debit or credit
      }//                                             if flagCol == 0, flag is empty

      txt = m_columnList[i];//                        amount column value
      if((m_amountColumn == i) &&
          (((txt.contains("("))) || (flag.startsWith('A')))) {//  "(" or "Af" = debit
        txt = txt.remove(QRegExp("[()]"));
        txt = '-' + txt;  //                          Mark as -ve
      } else if(m_debitColumn == i) {
        txt = '-' + txt;  //                          Mark as -ve
      }
      newTxt = m_parse->possiblyReplaceSymbol(txt);
      m_trData.amount = newTxt;
      m_qifBuffer = m_qifBuffer + 'T' + newTxt + '\n';
    }

    else if((columnType(i) == "debit") || (columnType(i) == "credit")) {  //  Credit or debit?
      ++neededFieldsCount;
      txt = m_columnList[i];
      if(!txt.isEmpty()) {
        if(m_debitColumn == i)
          txt = '-' + txt;//                          Mark as -ve
        if((m_debitColumn == i) || (m_creditColumn == i)) {
          newTxt = m_parse->possiblyReplaceSymbol(txt);
          m_trData.amount = newTxt;
          m_qifBuffer = m_qifBuffer + 'T' + newTxt + '\n';
        }
      }
    }

    else if(columnType(i) == "memo") {  // could be more than one
      txt = m_columnList[i];
      txt.replace('~', "\n");//                       replace NL which was substituted
      if(!memo.isEmpty())
        memo += '\n';//                               separator for multiple memos
      memo += txt;//                                  next memo
    }//end of memo field
  }//end of col loop
  m_trData.memo = memo;
  m_qifBuffer = m_qifBuffer + 'M' + memo + '\n' + "^\n";
  if(neededFieldsCount > 2) {
    return KMessageBox::Ok;
  } else {
    KMessageBox::sorry(0, i18n("<center>The columns selected are invalid.\n</center>"
                               "There must an amount or debit and credit fields, plus date and payee fields."), i18n("CSV import"));
    return KMessageBox::Cancel;
  }
}


void CSVDialog::csvImportTransaction(MyMoneyStatement& st)
{
  MyMoneyStatement::Transaction tr;
  QString tmp;
  QString payee = m_trData.payee;//                              extractLine('P')

  // Process transaction data

  char result[100];

  int rand = qrand();
  sprintf(result, "%d", rand);
  tr.m_strBankID = result;
  st.m_eType = MyMoneyStatement::etCheckings;
  tr.m_datePosted = m_trData.date;
  if(!tr.m_datePosted.isValid()) {
    int rc = KMessageBox::warningContinueCancel(0, i18n("The date entry \"%1\" read from the file cannot be interpreted through the current "
             "date format setting of \"%2.\"" "\n\nPressing \'Continue\' will "
             "assign today's date to the transaction. Pressing \'Cancel\'' will abort "
             "the import operation. You can then restart the import and select a different "
             "date format.", m_trData.date.toString(m_date), m_dateFormats[m_dateFormatIndex]), i18n("Invalid date format"));
    switch(rc) {
      case KMessageBox::Continue:
        tr.m_datePosted = (QDate::currentDate());
        break;

      case KMessageBox::Cancel:
        m_importNow = false;//                             Don't process statement
        st = MyMoneyStatement();
        return ;
    }
  }
  tr.m_amount = m_trData.amount;
  tr.m_shares = m_trData.amount;

  tmp = m_trData.number;
  tr.m_strNumber = tmp;

  if(!payee.isEmpty()) {
    tr.m_strPayee = m_trData.payee;
  }

  tr.m_strMemo = m_trData.memo;
  // Add the transaction to the statement
  st.m_listTransactions += tr;
  if((st.m_listTransactions.count()) > 0) {
    statements += st;// this not used
    qDebug("Statement with %d transactions ready.",
           st.m_listTransactions.count());
  }
  // Now to import the statements
  return;
}


void CSVDialog::slotImportClicked()
{
  if(m_fileType != "Banking") {
    return;
  }
  // The following two fields are optional so must be cleared
  // ...of any prior choices in UI

  m_pageBanking->ui->comboBoxBnk_memoCol->setCurrentIndex(-1);
  m_pageBanking->ui->comboBoxBnk_numberCol->setCurrentIndex(-1);

  if((m_dateSelected) && (m_payeeSelected) &&
      ((m_amountSelected || (m_debitSelected && m_creditSelected)))) {
    m_importNow = true; //                          all necessary data is present

    int skp = m_pageLinesDate->ui->spinBox_skip->value();
    if(skp > m_endLine) {
      KMessageBox::sorry(0, i18n("<center>The start line is greater than the end line.\n</center>"
                                 "<center>Please correct your settings.</center>"), i18n("CSV import"));
      return;
    }
    m_parse->setSymbolFound(false);
    readFile(m_inFileName, skp);   //               skip all headers

    //--- create the (revised) vertical (row) headers ---

    QStringList vertHeaders;
    for(int i = skp; i < ui->tableWidget->rowCount() + skp; i++) {
      QString hdr = (QString::number(i + 1));
      vertHeaders += hdr;
    }
    //  verticalHeader()->width() varies with its content so....

    ui->tableWidget->setVerticalHeaderLabels(vertHeaders);
    ui->tableWidget->hide();//             to ensure....
    ui->tableWidget->show();//             ..vertical header width redraws
  } else {
    KMessageBox::information(0, i18n("<center>An Amount-type column, and Date and Payee columns are needed!</center> <center>Please try again.</center>"));
  }
}

void CSVDialog::slotSaveAsQIF()
{
  if(m_fileType == QLatin1String("Banking")) {
    QStringList outFile = m_inFileName.split('.');
    const KUrl& name = QString((outFile.isEmpty() ? "CsvProcessing" : outFile[0]) + ".qif");

    QString outFileName = KFileDialog::getSaveFileName(name, QString::fromLatin1("*.qif | %1").arg(i18n("QIF Files")), 0, i18n("Save QIF")
#if KDE_IS_VERSION(4,4,0)
                          , KFileDialog::ConfirmOverwrite
#endif
                                                      );

    QFile oFile(outFileName);
    oFile.open(QIODevice::WriteOnly);
    QTextStream out(&oFile);
    out << m_qifBuffer;// output qif file
    oFile.close();
  }//else
}


void CSVDialog::setCodecList(const QList<QTextCodec *> &list)
{
  m_comboBoxEncode->clear();
  foreach (QTextCodec * codec, list) {
    m_comboBoxEncode->addItem(codec->name(), codec->mibEnum());
  }
}


int CSVDialog::columnNumber(const QString& msg)
{
  //  This dialog box is for use with the debit/credit flag resource file entry,
  //  indicating the sign of the value column. ie a debit or a credit.
  bool ok;
  static int ret;
  ret = KInputDialog::getInteger(i18n("Enter column number of debit/credit code"), msg, 0, 1, m_endColumn, 1, 10, &ok);
  if(ok && ret > 0)
    return ret;
  return 0;
}

void CSVDialog::clearColumnsSelected()
{
  if(m_fileType == "Banking") {
    clearPreviousColumn();
    clearSelectedFlags();
    clearColumnNumbers();
    clearComboBoxText();
  }
}

void CSVDialog::clearSelectedFlags()
{
  for(int i = 0; i < MAXCOL; i++)
    clearColumnType(i);   //   set to all empty

  m_dateSelected = false;
  m_payeeSelected = false;
  m_amountSelected = false;
  m_debitSelected = false;
  m_creditSelected = false;
  m_memoSelected = false;
  m_numberSelected = false;
  m_pageBanking->ui->radioBnk_amount->setEnabled(true);
  m_pageBanking->ui->radioBnk_debCred->setEnabled(true);
}

void CSVDialog::clearColumnNumbers()
{
  m_pageBanking->ui->comboBoxBnk_dateCol->setCurrentIndex(-1);
  m_pageBanking->ui->comboBoxBnk_payeeCol->setCurrentIndex(-1);
  m_pageBanking->ui->comboBoxBnk_memoCol->setCurrentIndex(-1);
  m_pageBanking->ui->comboBoxBnk_numberCol->setCurrentIndex(-1);
  m_pageBanking->ui->comboBoxBnk_amountCol->setCurrentIndex(-1);
  m_pageBanking->ui->comboBoxBnk_debitCol->setCurrentIndex(-1);
  m_pageBanking->ui->comboBoxBnk_creditCol->setCurrentIndex(-1);
}

void CSVDialog::clearComboBoxText()
{
  for(int i = 0; i < MAXCOL; i++) {
    m_pageBanking->ui->comboBoxBnk_memoCol->setItemText(i, QString().setNum(i + 1));
  }
}

void CSVDialog::clearColumnTypes()
{
  for(int i = 0; i < MAXCOL; i++) {
    clearColumnType(i);
  }
}

void CSVDialog::encodingChanged(int index)
{
  m_encodeIndex = index;
  if(!m_inFileName.isEmpty())
    readFile(m_inFileName, 0);
}

void CSVDialog::findCodecs()
{
  QMap<QString, QTextCodec *> codecMap;
  QRegExp iso8859RegExp("ISO[- ]8859-([0-9]+).*");

  foreach (int mib, QTextCodec::availableMibs()) {
    QTextCodec *codec = QTextCodec::codecForMib(mib);

    QString sortKey = codec->name().toUpper();
    int rank;

    if(sortKey.startsWith("UTF-8")) {         // krazy:exclude=strings
      rank = 1;
    } else if(sortKey.startsWith("UTF-16")) {        // krazy:exclude=strings
      rank = 2;
    } else if(iso8859RegExp.exactMatch(sortKey)) {
      if(iso8859RegExp.cap(1).size() == 1)
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


void CSVDialog::enableInputs()
{
  m_pageLinesDate->ui->spinBox_skip->setEnabled(true);
  m_pageBanking->ui->comboBoxBnk_numberCol->setEnabled(true);
  m_pageBanking->ui->comboBoxBnk_dateCol->setEnabled(true);
  m_pageBanking->ui->comboBoxBnk_payeeCol->setEnabled(true);
  m_pageBanking->ui->comboBoxBnk_memoCol->setEnabled(true);
  m_pageBanking->ui->button_clear->setEnabled(true);
  m_pageBanking->ui->gridLayout_2->columnStretch(2);
  m_pageLinesDate->ui->spinBox_skipToLast->setEnabled(true);
  m_pageSeparator->ui->comboBox_fieldDelimiter->setEnabled(true);

  if(m_pageBanking->ui->radioBnk_amount->isChecked()) {
    m_pageBanking->ui->comboBoxBnk_amountCol->setEnabled(true);
    m_pageBanking->ui->comboBoxBnk_debitCol->setEnabled(false);
    m_pageBanking->ui->comboBoxBnk_creditCol->setEnabled(false);
  } else {
    m_pageBanking->ui->comboBoxBnk_amountCol->setEnabled(false);
    m_pageBanking->ui->comboBoxBnk_debitCol->setEnabled(true);
    m_pageBanking->ui->comboBoxBnk_creditCol->setEnabled(true);
  }
}


void CSVDialog::updateScreen()
{
  QRect tableRect = ui->tableWidget->geometry();//     need table height
  int hght = ui->tableWidget->horizontalHeader()->height() + 8;// find data height
  hght += (ui->tableWidget->rowHeight(m_row - 1)) * m_row;
  int ht = (hght < ui->frame_main->height() ? hght : ui->frame_main->height() - 10);// rect.height() reduce height if > frame
  tableRect.setHeight(ht);//ht                                    set table height

  ui->tableWidget->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);//ResizeToContents
  ui->tableWidget->setGeometry(tableRect);//           resize now ***************
  ui->tableWidget->setFocus();
}

void CSVDialog::slotClose()
{
  saveSettings();
  m_investmentDlg->saveSettings();
  m_plugin->m_action->setEnabled(true);
  close();
}

void CSVDialog::saveSettings()
{
  if(m_fileType != "Banking") {
    return;
  }

  QString str;

  if(!m_inFileName.isEmpty()) {  //  don't save column numbers if no file loaded
    KSharedConfigPtr config = KSharedConfig::openConfig(KStandardDirs::locateLocal("config", "csvimporterrc"));

    KConfigGroup mainGroup(config, "MainWindow");
    mainGroup.writeEntry("Height", height());
    mainGroup.config()->sync();

    KConfigGroup profileGroup(config, "Profile");
    profileGroup.writeEntry("CurrentUI", m_currentUI);
    QString pth = "$HOME/" + m_csvPath.section('/', 3);
    profileGroup.writeEntry("CsvDirectory", pth);
    profileGroup.writeEntry("DateFormat", m_pageLinesDate->ui->comboBox_dateFormat->currentIndex());
    profileGroup.writeEntry("FieldDelimiter", m_fieldDelimiterIndex);
    profileGroup.writeEntry("TextDelimiter", m_textDelimiterIndex);

    if(str == "Banking") {
      profileGroup.writeEntry("FileType", str);
      profileGroup.writeEntry("StartLine", m_pageLinesDate->ui->spinBox_skip->value() - 1);
    }
    profileGroup.config()->sync();

    KConfigGroup columnsGroup(config, "Columns");
    columnsGroup.writeEntry("DateCol", m_pageBanking->ui->comboBoxBnk_dateCol->currentIndex());
    columnsGroup.writeEntry("PayeeCol", m_pageBanking->ui->comboBoxBnk_payeeCol->currentIndex());
    columnsGroup.writeEntry("NumberCol", m_pageBanking->ui->comboBoxBnk_numberCol->currentIndex());
    columnsGroup.writeEntry("AmountCol", m_pageBanking->ui->comboBoxBnk_amountCol->currentIndex());
    columnsGroup.writeEntry("DebitCol", m_pageBanking->ui->comboBoxBnk_debitCol->currentIndex());
    columnsGroup.writeEntry("CreditCol", m_pageBanking->ui->comboBoxBnk_creditCol->currentIndex());
    columnsGroup.config()->sync();

    m_inFileName.clear();
  }
  ui->tableWidget->clear();//     in case later reopening window, clear old contents now
}


int CSVDialog::validateColumn(const int& col, const QString& type)
{
  //  First check if selection is in range
  if((col < 0) || (col >= m_endColumn)) {
    return KMessageBox::No;
  }//                                               selection was in range

  if((!m_columnType[col].isEmpty())  && (m_columnType[col] != type)) {
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
  for(int i = 0; i < m_endColumn; i++) {
    //  check each column
    if(m_columnType[i] == type) {  //               this type already in use
      m_columnType[i].clear();//                   ...so clear it
    }//  end this col

  }// end all columns checked                      type not in use
  m_columnType[col] = type;//                      accept new type
  if(m_previousColumn != -1) {
    m_previousColumn = col;
  }
  m_previousType = type;
  return KMessageBox::Ok; //                       accept new type
}


void CSVDialog::amountColumnSelected(int col)
{
  QString type = "amount";
  if(col < 0) {  //                                 it is unset
    return;
  }
// if a previous amount field is detected, but in a different column...
  if((m_amountColumn != -1) && (m_columnType[m_amountColumn] == type)  && (m_amountColumn != col)) {
    m_columnType[m_amountColumn].clear();
  }
  int ret = validateColumn(col, type);
  if(ret == KMessageBox::Ok) {
    m_pageBanking->ui->comboBoxBnk_amountCol->setCurrentIndex(col);//    accept new column
    m_amountSelected = true;
    m_amountColumn = col;
    m_columnType[m_amountColumn] = type;
    return;
  }
  if(ret == KMessageBox::No) {
    m_pageBanking->ui->comboBoxBnk_amountCol->setCurrentIndex(-1);
  }
}

void CSVDialog::debitCreditRadioClicked(bool checked)
{
  if(checked) {
    m_pageBanking->ui->comboBoxBnk_debitCol->setEnabled(true);//         if 'debit/credit' selected
    m_pageBanking->ui->labelBnk_debits->setEnabled(true);
    m_pageBanking->ui->comboBoxBnk_creditCol->setEnabled(true);
    m_pageBanking->ui->labelBnk_credits->setEnabled(true);
    m_pageBanking->ui->comboBoxBnk_amountCol->setEnabled(false);//       disable 'amount' ui choices
    m_pageBanking->ui->comboBoxBnk_amountCol->setCurrentIndex(-1);//     as credit/debit chosen

    //   the 'm_amountColumn' could just have been reassigned, so ensure
    //   ...m_columnType[m_amountColumn] == "amount" before clearing it
    if((m_amountColumn >= 0) && (m_columnType[m_amountColumn] == "amount")) {
      m_columnType[m_amountColumn].clear();//          ...drop any amount choice
      m_amountColumn = -1;
    }
    m_pageBanking->ui->labelBnk_amount->setEnabled(false);
  }
}

void CSVDialog::creditColumnSelected(int col)
{
  QString type = "credit";
  if(col < 0) {  //                                    it is unset
    return;
  }
// if a previous credit field is detected, but in a different column...
  if((m_creditColumn != -1) && (m_columnType[m_creditColumn] == type)  && (m_creditColumn != col)) {
    m_columnType[m_creditColumn].clear();
  }
  int ret = validateColumn(col, type);

  if(ret == KMessageBox::Ok) {
    m_pageBanking->ui->comboBoxBnk_creditCol->setCurrentIndex(col);//    accept new column
    m_creditSelected = true;
    m_creditColumn = col;
    m_columnType[m_creditColumn] = type;
    return;
  }
  if(ret == KMessageBox::No) {
    m_pageBanking->ui->comboBoxBnk_creditCol->setCurrentIndex(-1);
  }
}

void CSVDialog::debitColumnSelected(int col)
{
  QString type = "debit";
  if(col < 0) {  //                                    it is unset
    return;
  }
// A new column has been selected for this field so clear old one
  if((m_debitColumn != -1) && (m_columnType[m_debitColumn] == type)  && (m_debitColumn != col)) {
    m_columnType[m_debitColumn].clear();
  }
  int ret = validateColumn(col, type);

  if(ret == KMessageBox::Ok) {
    m_pageBanking->ui->comboBoxBnk_debitCol->setCurrentIndex(col);//     accept new column
    m_debitSelected = true;
    m_debitColumn = col;
    m_columnType[m_debitColumn] = type;
    return;
  }
  if(ret == KMessageBox::No) {
    m_pageBanking->ui->comboBoxBnk_debitCol->setCurrentIndex(-1);
  }
}

void CSVDialog::dateColumnSelected(int col)
{
  QString type = "date";
  if(col < 0) {  //                                 it is unset
    return;
  }
// A new column has been selected for this field so clear old one
  if((m_dateColumn != -1) && (m_columnType[m_dateColumn] == type)  && (m_dateColumn != col)) {
    m_columnType[m_dateColumn].clear();
  }
  int ret = validateColumn(col, type);

  if(ret == KMessageBox::Ok) {
    m_pageBanking->ui->comboBoxBnk_dateCol->setCurrentIndex(col);//      accept new column
    m_dateSelected = true;
    m_dateColumn = col;
    m_columnType[m_dateColumn] = type;
    return;
  }
  if(ret == KMessageBox::No) {
    m_pageBanking->ui->comboBoxBnk_dateCol->setCurrentIndex(-1);
  }
}

void CSVDialog::memoColumnSelected(int col)
{
  QString type = "memo";
  if((col < 0) || (col >= m_endColumn)) {  // out of range so...
    m_pageBanking->ui->comboBoxBnk_memoCol->setCurrentIndex(-1);// ..clear selection
    return;
  }
  if(m_columnType[col].isEmpty()) {  //             accept new  entry
    m_pageBanking->ui->comboBoxBnk_memoCol->setItemText(col, QString().setNum(col + 1) + '*');
    m_columnType[col] = type;
    m_memoColumn = col;
    m_memoSelected = true;
    return;
  } else {//                                       clashes with prior selection
    if(m_columnType[col] == type) {  //               nothing changed
      return;
    }
    m_memoSelected = false;//                      clear incorrect selection
    KMessageBox::information(0, i18n("The '<b>%1</b>' field already has this column selected. <center>Please reselect both entries as necessary.</center>"
                                     , m_columnType[col]));
    m_pageBanking->ui->comboBoxBnk_memoCol->setCurrentIndex(-1);
    m_previousColumn = -1;
    resetComboBox(m_columnType[col], col);//       clash,  so reset ..
    resetComboBox(type, col);//                    ... both comboboxes
    m_previousType.clear();
    m_columnType[col].clear();
    m_pageBanking->ui->comboBoxBnk_memoCol->setItemText(col, QString().setNum(col + 1));//  reset the '*'
  }
}

void CSVDialog::payeeColumnSelected(int col)
{
  QString type = "payee";
  if(col < 0) {  //                              it is unset
    return;
  }
// if a previous payee field is detected, but in a different column...
  if((m_payeeColumn != -1) && (m_columnType[m_payeeColumn] == type)  && (m_payeeColumn != col)) {
    m_columnType[m_payeeColumn].clear();
  }
  int ret = validateColumn(col, type);
  if(ret == KMessageBox::Ok) {
    m_pageBanking->ui->comboBoxBnk_payeeCol->setCurrentIndex(col);// accept new column
    m_payeeSelected = true;
    m_payeeColumn = col;
    m_columnType[m_payeeColumn] = type;
    return;
  }
  if(ret == KMessageBox::No) {
    m_pageBanking->ui->comboBoxBnk_payeeCol->setCurrentIndex(-1);
  }
}

void CSVDialog::numberColumnSelected(int col)
{
  QString type = "number";
  if(col < 0) {  //                              it is unset
    return;
  }
// if a previous number field is detected, but in a different column...
  if((m_numberColumn != -1) && (m_columnType[m_numberColumn] == type)  && (m_numberColumn != col)) {
    m_columnType[m_numberColumn].clear();
  }
  int ret = validateColumn(col, type);

  if(ret == KMessageBox::Ok) {
    m_pageBanking->ui->comboBoxBnk_numberCol->setCurrentIndex(col);// accept new column
    m_numberSelected = true;
    m_numberColumn = col;
    m_columnType[m_numberColumn] = type;
    return;
  }
  if(ret == KMessageBox::No) {
    m_pageBanking->ui->comboBoxBnk_numberCol->setCurrentIndex(-1);
  }
}

void CSVDialog::closeEvent(QCloseEvent *event)
{
  slotClose();
  event->accept();
}

void CSVDialog::resizeEvent(QResizeEvent * event)
{
  event->accept();
}

QString CSVDialog::columnType(int column)
{
  return  m_columnType[column];
}

void CSVDialog::clearColumnType(int column)
{
  m_columnType[column].clear();
}

void CSVDialog::clearPreviousColumn()
{
  m_previousType.clear();
}

void CSVDialog::resetComboBox(const QString& comboBox, const int& col)
{
  QStringList fieldType;
  fieldType << "amount" << "credit" << "date" << "debit" << "memo" << "number" << "payee";
  int index = fieldType.indexOf(comboBox);
  switch(index) {
    case 0://  amount
      m_pageBanking->ui->comboBoxBnk_amountCol->setCurrentIndex(-1);
      m_amountSelected = false;
      break;
    case 1://  credit
      m_pageBanking->ui->comboBoxBnk_creditCol->setCurrentIndex(-1);
      m_creditSelected = false;
      break;
    case 2://  date
      m_pageBanking->ui->comboBoxBnk_dateCol->setCurrentIndex(-1);
      m_dateSelected = false;
      break;
    case 3://  debit
      m_pageBanking->ui->comboBoxBnk_debitCol->setCurrentIndex(-1);
      m_debitSelected = false;
      break;
    case 4://  memo
      m_pageBanking->ui->comboBoxBnk_memoCol->setCurrentIndex(-1);
      m_pageBanking->ui->comboBoxBnk_memoCol->setItemText(col, QString().setNum(col + 1));//  reset the '*'
      m_memoSelected = false;
      break;
    case 5://  number
      m_pageBanking->ui->comboBoxBnk_numberCol->setCurrentIndex(-1);
      m_numberSelected = false;
      break;
    case 6://  payee
      m_pageBanking->ui->comboBoxBnk_payeeCol->setCurrentIndex(-1);
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

void CSVDialog::updateDecimalSymbol(const QString& type, int col)
{
  QString txt;
  bool symbolFound = false;
  bool invalidResult = false;
  int last = 0;
  int first = 0;
  //  Clear background

  if(m_fileType == "Banking") {
    last = m_endLine;
    first = m_startLine;
  } else {
    last = m_investProcessing->m_endLine;
    first = m_investProcessing->m_startLine;
  }

  for(int row = 0; row < m_endLine; row++) {
    if(ui->tableWidget->item(row, col) != 0) {
      ui->tableWidget->item(row, col)->setBackground(m_clearBrush);
    }
  }

  if(type == "amount" || type == "credit" || type == "debit" || type == "price" || type == "quantity") {

    //  Set first and last rows

    m_parse->setSymbolFound(false);

    QString newTxt;
    int errorRow = 0;
    QTableWidgetItem* errorItem(0);
    //  Check if this col contains empty cells
    int row = 0;
    for(row = first - 1; row < last; row++) {
      if(ui->tableWidget->item(row, col) == 0) {  //       empty cell
        if((m_fileType == "Banking") && (importNow())) {
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
        if(ret == KMessageBox::Continue) {
          continue;
        }
        return;//                                     empty cell
      } else {

        //  Check if this col contains decimal symbol

        txt = ui->tableWidget->item(row, col)->text();//  get data

        newTxt = m_parse->possiblyReplaceSymbol(txt);//  update data
        ui->tableWidget->item(row, col)->setText(newTxt);//  highlight selection
        ui->tableWidget->item(row, col)->setBackground(m_colorBrush);
        if(m_parse->invalidConversion()) {
          invalidResult = true;
          errorItem = ui->tableWidget->item(row, col);
          errorItem->setBackground(m_errorBrush);
          ui->tableWidget->scrollToItem(errorItem, QAbstractItemView::EnsureVisible);
          if(errorRow == 0) {
            errorRow = row;
          }
        }
        if(m_parse->symbolFound()) {
          symbolFound = true;
        }
        if(newTxt == txt) {  //                        no matching symbol found
          continue;
        }
      }
      if(!symbolFound) {
        errorItem = ui->tableWidget->item(row, col);
        errorItem->setBackground(m_errorBrush);
      }
    }//  last row

    if(!symbolFound) { //                            no symbol found
      ui->tableWidget->scrollToItem(errorItem, QAbstractItemView::EnsureVisible);
      KMessageBox::sorry(this, i18n("<center>The selected decimal symbol was not present in column %1,</center>"
                                    "<center>- but may now have been added.</center>"
                                    "<center>If the <b>decimal</b> symbol displayed does not match your system setting</center>"
                                    "<center>your data is unlikely to import correctly.</center>"
                                    "<center>Please check your selection.</center>",
                                    col + 1), i18n("CSV import"));
      return;
    }

    if(invalidResult) {
      KMessageBox::sorry(0, i18n("<center>The selected decimal symbol/thousands separator</center>"
                                 "<center>have produced invalid results in row %1, and possibly more.</center>"
                                 "<center>Please try again.</center>", errorRow + 1), i18n("Invalid Conversion"));
      if(m_fileType == "Banking") {
        readFile("", 0);
      }
    }
    emit isImportable();
  }
}

void CSVDialog::dateFormatSelected(int dF)
{
  if(dF == -1) {
    return;
  }
  m_dateFormatIndex = dF;
  m_date = m_dateFormats[m_dateFormatIndex];
}

void CSVDialog::decimalSymbolSelected(int index)
{
  restoreBackground();//                              remove selection highlighting

  if(index < 0) {
    return;
  }

  if(m_startLine > m_endLine) {
    KMessageBox::sorry(0, i18n("<center>The start line is greater than the end line.\n</center>"
                               "<center>Please correct your settings.</center>"), i18n("CSV import"));
    return;
  }

  if(m_decimalSymbolChanged) {
    if(m_fileType == "Banking") {
      readFile("", 0);
    } else {
      m_investProcessing->readFile("", 0);
    }
  }

  //  Save new decimal symbol and thousands separator

  m_decimalSymbolIndex = index;
  m_parse->setDecimalSymbolIndex(index);
  m_decimalSymbol = m_parse->decimalSymbol(index);
  m_pageCompletion->ui->comboBox_thousandsDelimiter->setCurrentIndex(index);
  thousandsSeparatorChanged();

  //  Update the UI

  if(m_fileType == "Banking") {
    if((!m_inFileName.isEmpty()) && ((m_amountColumn >= 0) || ((m_debitColumn >= 0) && (m_creditColumn >= 0)))) {
      if(m_amountColumn >= 0) {
        updateDecimalSymbol("amount", m_amountColumn);
      } else {
        updateDecimalSymbol("debit", m_debitColumn);
        updateDecimalSymbol("credit", m_creditColumn);
      }
      m_decimalSymbolChanged = true;
    }
  } else {
    if(m_fileType == "Invest") {
      if(!m_investProcessing->inFileName().isEmpty()) {
        updateDecimalSymbol("amount", m_investProcessing->amountColumn());
        updateDecimalSymbol("price", m_investProcessing->priceColumn());
        updateDecimalSymbol("quantity", m_investProcessing->quantityColumn());
      }
    }
    m_decimalSymbolChanged = true;
  }
}

QString CSVDialog::decimalSymbol()
{
  return m_decimalSymbol;
}

void CSVDialog::thousandsSeparatorChanged()
{
  m_thousandsSeparator = m_parse->thousandsSeparator();
}

void CSVDialog::delimiterChanged()
{
  if(m_fileType != "Banking") {
    return;
  }
  if(!m_inFileName.isEmpty())
    readFile(m_inFileName, 0);
}

void CSVDialog::startLineChanged(int val)
{
  if(m_fileType != "Banking") {
    return;
  }
  m_startLine = val;
}

void CSVDialog::startLineChanged()
{
  int val = m_pageLinesDate->ui->spinBox_skip->value();
  if(val < 1) {
    return;
  }
  m_startLine = val - 1;
}

void CSVDialog::endLineChanged(int val)
{
  m_endLine = val + 1;
}

void CSVDialog::endLineChanged()
{
  m_endLine = m_pageLinesDate->ui->spinBox_skipToLast->value();
}

void CSVDialog::restoreBackground()
{
  int lastRow;
  int lastCol;
  if(m_fileType == "Banking") {
    lastRow = m_row;
    lastCol = m_endColumn;
  } else {
    lastRow = m_investProcessing->m_row;
    lastCol = m_investProcessing->m_endColumn;
  }

  for(int row = 0; row < lastRow; row++) {
    for(int col = 0; col < lastCol; col++) {
      if(ui->tableWidget->item(row, col) != 0) {
        ui->tableWidget->item(row, col)->setBackground(m_clearBrush);
      }
    }
  }
}

void CSVDialog::amountRadioClicked(bool checked)
{
  if(checked) {
    m_pageBanking->ui->comboBoxBnk_amountCol->setEnabled(true);//  disable credit & debit ui choices
    m_pageBanking->ui->labelBnk_amount->setEnabled(true);
    m_pageBanking->ui->comboBoxBnk_debitCol->setEnabled(false);
    m_pageBanking->ui->comboBoxBnk_debitCol->setCurrentIndex(-1);
    m_pageBanking->ui->labelBnk_debits->setEnabled(false);
    m_pageBanking->ui->comboBoxBnk_creditCol->setEnabled(false);
    m_pageBanking->ui->comboBoxBnk_creditCol->setCurrentIndex(-1);
    m_pageBanking->ui->labelBnk_credits->setEnabled(false);

    //   the 'm_creditColumn/m_debitColumn' could just have been reassigned, so ensure
    //   ...they == "credit or debit" before clearing them
    if((m_creditColumn >= 0) && (m_columnType[m_creditColumn] == "credit")) {
      m_columnType[m_creditColumn].clear();//       because amount col chosen...
    }
    if((m_debitColumn >= 0) && (m_columnType[m_debitColumn] == "debit")) {
      m_columnType[m_debitColumn].clear();//        ...drop any credit & debit
    }
    m_debitColumn = -1;
    m_creditColumn = -1;
  }
}


QString CSVDialog::currentUI()
{
  return m_currentUI;
}

void CSVDialog::setCurrentUI(QString val)
{
  m_currentUI = val;
}

bool CSVDialog::importNow()
{
  return m_importNow;
}

void CSVDialog::showStage()
{
  QString str = ui->label_intro->text();
  ui->label_intro->setText("<b>" + str + "</b>");
}

void CSVDialog::slotIdChanged(int id)
{
  QString txt;
  m_lastId = m_curId;
  m_curId = id;
  if((m_lastId == -1) || (m_curId == -1)) {
    return;
  }
  txt = m_stageLabels[m_lastId]->text();
  txt.remove(QRegExp("[<b>/]"));
  m_stageLabels[m_lastId]->setText(txt);

  txt = m_stageLabels[m_curId]->text();
  txt = "<b>" + txt + "</b>";
  m_stageLabels[m_curId]->setText(txt);

  if(id == -1) {
    return;
  }
}

void CSVDialog::slotNamesEdited()
{
  QString str;
  int row = 0;
  int symTableRow = 0;
  int strtLine = m_investProcessing->m_startLine - 1;
  int endLine = m_pageLinesDate->ui->spinBox_skipToLast->value() - 1;

  for(row = strtLine; row <= endLine; row ++) { // <= needed for single line in merrill
    str = ui->tableWidget->item(row, m_investProcessing->symbolColumn())->text().trimmed();
    if(ui->tableWidget->item(row, m_investProcessing->symbolColumn())->text().trimmed().isEmpty()) {
      continue;
    }
    //  Replace detail with edited security name.
    str = m_symbolTableDlg->m_widget->tableWidget->item(symTableRow, 2)->text();

    ui->tableWidget->item(row, m_investProcessing->detailColumn())->setText(str);
    m_investProcessing->m_map.insert(m_symbolTableDlg->m_widget->tableWidget->item(symTableRow, 0)->text(), m_symbolTableDlg->m_widget->tableWidget->item(symTableRow, 2)->text());
    symTableRow ++;
  }
}

void CSVDialog::slotBackButtonClicked()
{
  m_goBack = true;
}

//-----------------------------------------------------------------------------------------------------------------

IntroPage::IntroPage(QWidget *parent) : QWizardPage(parent), ui(new Ui::IntroPage)
{
  ui->setupUi(this);
  m_pageLayout = new QVBoxLayout;
  ui->horizontalLayout->insertLayout(0, m_pageLayout, 1);

  registerField("source", ui->combobox_source, "currentIndex", SIGNAL(currentIndexChanged()));

  m_sourceList << "banking" << "investing";
}

IntroPage::~IntroPage()
{
  delete ui;
}

void IntroPage::setParent(CSVDialog* dlg)
{
  m_dlg = dlg;
  m_set = true;
  registerField("csvdialog", m_dlg, "m_set", SIGNAL(isSet()));

  m_dlg->showStage();
}

void IntroPage::slotComboSourceClicked(int index)
{
  if(index >= 4) {
    KMessageBox::sorry(this, i18n("<center>Sorry, only four or fewer sources allowed.</center>"), i18n("CSV import"));
    ui->combobox_source->removeItem(4);
    return;
  }

  QString name = m_dlg->m_pageIntro->ui->combobox_source->currentText();
  if(index >= m_sourceList.count()) {
    m_sourceList << name;
    m_dlg->m_investProcessing->m_brokerList << name;
  }
  m_dlg->m_activityType = index;

  wizard()->button(QWizard::CustomButton1)->setEnabled(true);//  Enable fileSelect button
  switch(index) {
    case 0:
      setField("source", index);
      m_dlg->m_fileType = "Banking";
      m_dlg->readSettings();
      break;
    case 1:
      setField("source", index);
      m_dlg->m_fileType = "Invest";
      m_dlg->m_investProcessing->readSettings();
      break;
    default:
      setField("source", index);
      m_dlg->m_fileType = "Invest";
      m_dlg->m_activityType = index;
      QString str = ui->combobox_source->currentText() + QString::number(index - 1);

      m_dlg->m_investProcessing->readSettings(index);
      break;
  }
}

void IntroPage::initializePage()
{
  QList<QWizard::WizardButton> layout;
  layout << QWizard::Stretch << QWizard::BackButton << QWizard::CustomButton1
         <<  QWizard::CancelButton;
  wizard()->setButtonText(QWizard::CustomButton1, "Select File");
  wizard()->setOption(QWizard::HaveCustomButton1, true);
  wizard()->setButtonLayout(layout);
  wizard()->button(QWizard::CustomButton1)->setEnabled(false);

  ui->combobox_source->addItems(m_dlg->m_investProcessing->m_brokerList);
  m_sourceList += m_dlg->m_investProcessing->m_brokerList;
  ui->combobox_source->setCurrentIndex(-1);
}

bool IntroPage::validatePage()
{
  return true;
}

void IntroPage::slotSourceNameEdited()
{
  QString name = m_dlg->m_pageIntro->ui->combobox_source->lineEdit()->text();

  if(name.isEmpty()) {
    return;
  }

  if(!m_sourceList.contains(name)) {
    int rc = KMessageBox::warningContinueCancel(0, i18n("<center>Do you want to add a new broker</center>\n"
             "<center>%1 </center>\n"
             "<center>to the selection list?</center>\n"
             "<center>Click \'Continue\' to add the name.</center>\n"
             "<center>Otherwise, click \'Cancel\'.</center>",
             name), i18n("Add Security Name"));
    if(rc == KMessageBox::Cancel) {
      m_dlg->m_pageIntro->ui->combobox_source->clearEditText();
      m_dlg->m_pageIntro->ui->combobox_source->setCurrentIndex(-1);
      return;
    }
  }
  m_sourceList << name;
  m_dlg->m_investProcessing->m_brokerList << name;
  m_dlg->m_pageIntro->ui->combobox_source->addItem(name);

  int index = m_sourceList.count() - 1;
  if(index < 2) {
    return;
  }

  if(index >= m_sourceList.count()) {
    m_sourceList << name;
  }
  m_sourceList << name;
  m_sourceList.removeDuplicates();

  switch(index) {
    case 0:
      setField("source", index);
      m_dlg->m_fileType = "Banking";
      m_dlg->readSettings();
      break;
    case 1:
      setField("source", index);
      m_dlg->m_fileType = "Invest";
      m_dlg->m_investProcessing->readSettings();
      break;
    default:
      setField("source", index);
      m_dlg->m_fileType = "Invest";
      m_dlg->m_activityType = index;
      QString str = ui->combobox_source->currentText() + QString::number(index - 1);
      m_dlg->m_investProcessing->readSettings(index);
      break;
  }
}


SeparatorPage::SeparatorPage(QWidget *parent) : QWizardPage(parent), ui(new Ui::SeparatorPage)
{
  ui->setupUi(this);

  m_pageLayout = new QVBoxLayout;

  ui->horizontalLayout->insertLayout(0, m_pageLayout, 1);
}

SeparatorPage::~SeparatorPage()
{
  delete ui;
}

void SeparatorPage::setParent(CSVDialog* dlg)
{
  m_dlg = dlg;
}

void SeparatorPage::cleanupPage()
{
  QList<QWizard::WizardButton> layout;
  layout << QWizard::Stretch << QWizard::BackButton << QWizard::CustomButton1
         <<  QWizard::CancelButton;
  wizard()->setButtonText(QWizard::CustomButton1, "Select File");
  wizard()->setOption(QWizard::HaveCustomButton1, true);
  wizard()->button(QWizard::CustomButton1)->setEnabled(false);
  wizard()->setButtonLayout(layout);
}

int SeparatorPage::nextId() const
{
  int ret;
  switch(field("source").toInt()) {
    case 0:
      ret = CSVDialog::Page_Banking;
      break;
    default:
      ret = CSVDialog::Page_Investment;
      break;
  }
  return ret;
}

BankingPage::BankingPage(QWidget *parent) : QWizardPage(parent), ui(new Ui::BankingPage)
{
  ui->setupUi(this);

  m_pageLayout = new QVBoxLayout;

  ui->horizontalLayout->insertLayout(0, m_pageLayout, 1);

  ui->comboBoxBnk_numberCol->setMaxVisibleItems(12);
  ui->comboBoxBnk_dateCol->setMaxVisibleItems(12);
  ui->comboBoxBnk_payeeCol->setMaxVisibleItems(12);
  ui->comboBoxBnk_memoCol->setMaxVisibleItems(12);
  ui->comboBoxBnk_amountCol->setMaxVisibleItems(12);
  ui->comboBoxBnk_creditCol->setMaxVisibleItems(12);
  ui->comboBoxBnk_debitCol->setMaxVisibleItems(12);
}

BankingPage::~BankingPage()
{
  delete ui;
}

int BankingPage::nextId() const
{
  return CSVDialog::Page_LinesDate;
}

InvestmentPage::InvestmentPage(QWidget *parent) : QWizardPage(parent), ui(new Ui::InvestmentPage)
{
  ui->setupUi(this);
  resize(638, 600);
  m_pageLayout = new QVBoxLayout;

  ui->horizontalLayout->insertLayout(0, m_pageLayout, 1);

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
  wizard()->button(QWizard::NextButton)->setEnabled(false);
  connect(ui->comboBoxInv_securityName, SIGNAL(currentIndexChanged(int)), this, SLOT(slotsecurityNameChanged(int)));
  connect(ui->buttonInv_hideSecurity, SIGNAL(clicked()), m_dlg->m_investProcessing, SLOT(hideSecurity()));
  m_dlg->m_isTableTrimmed = false;
  m_dlg->m_detailFilter = ui->lineEdit_filter->text();//  Load setting from config file.
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
  if(col != -1) {
    setField("securityNameIndex", -1);
    ui->comboBoxInv_securityName->setCurrentIndex(-1);
  }
  emit completeChanged();
}

void InvestmentPage::slotDetailColChanged(int col)
{
  setField("detailCol", col);
  if(col != -1) {
    setField("securityNameIndex", -1);
    ui->comboBoxInv_securityName->setCurrentIndex(-1);
  }
  emit completeChanged();
}

void InvestmentPage::slotsecurityNameChanged(int index)
{
  setField("securityNameIndex", index);

  if(index != -1) {
    setField("symbolCol", -1);
    setField("detailCol", -1);
    ui->comboBoxInv_symbolCol->setCurrentIndex(-1);
    ui->comboBoxInv_detailCol->setCurrentIndex(-1);
  }
  emit completeChanged();
}

void InvestmentPage::slotFilterEditingFinished()
{
  m_dlg->m_detailFilter = ui->lineEdit_filter->text();
}

void InvestmentPage::setParent(CSVDialog* dlg)
{
  m_dlg = dlg;
  connect(ui->button_clear, SIGNAL(clicked()), m_dlg->m_investProcessing, SLOT(clearColumnsSelected()));
}

bool InvestmentPage::isComplete() const
{
  bool ret = (((field("symbolCol").toInt() > -1) && (field("detailCol").toInt() > -1)) || ((field("securityNameIndex").toInt()) > -1)) &&
             (field("dateCol").toInt() > -1) && (field("typeCol").toInt() > -1) &&
             (field("quantityCol").toInt() > -1) && (field("priceCol").toInt() > -1) && (field("amountCol").toInt() > -1);
  return ret;
}

LinesDatePage::LinesDatePage(QWidget *parent) : QWizardPage(parent), ui(new Ui::LinesDatePage)
{
  ui->setupUi(this);

  m_pageLayout = new QVBoxLayout;

  ui->horizontalLayout->insertLayout(0, m_pageLayout, 1);
}

LinesDatePage::~LinesDatePage()
{
  delete ui;
}

void LinesDatePage::initializePage()
{
  m_dlg->m_goBack = false;
}

void LinesDatePage::setParent(CSVDialog* dlg)
{
  m_dlg = dlg;
}


bool LinesDatePage::validatePage()
{
  int symTableRow = -1;
  if((m_dlg->m_fileType == "Banking") || (field("symbolCol").toInt() == -1)) { //  Only check symbols if that field is set, and not Banking.
    return true;
  }

  disconnect(m_dlg->m_symbolTableDlg->m_widget->tableWidget, SIGNAL(cellChanged(int, int)), 0, 0);

  MyMoneyStatement::Security security;

  if(field("securityNameIndex").toInt() == -1) { //  No security name chosen so scan entries
    QString symbl;
    QString securityName;
    int startLine = ui->spinBox_skip->value() - 1;
    int endLine = ui->spinBox_skipToLast->value() - 1;

    for(int row = startLine; row <= endLine; row++) {
      symbl = m_dlg->ui->tableWidget->item(row, field("symbolCol").toInt())->text().toLower().trimmed();

      if(symbl.trimmed().isEmpty()) { //  no ticker
        continue;
      }

      securityName = m_dlg->ui->tableWidget->item(row, field("detailCol").toInt())->text().toLower();
      bool done = false;
      MyMoneyFile* file = MyMoneyFile::instance();

      // Check if we already have the security on file.
      // Just use the symbol for matching, because the security name
      // field is unstandardised and very variable.
      MyMoneySecurity sec;
      QList<MyMoneySecurity> list = file->securityList();
      bool exists;
      QString name;
      QList<MyMoneySecurity>::ConstIterator it = list.constBegin();
      while(it != list.constEnd()) {
        exists = false;
        if(!symbl.isEmpty())  { //  symbol already exists
          sec = *it;
          name.clear();
          if(sec.tradingSymbol() == symbl) {
            exists = true;
            name = sec.name();
            break;
          }
        }
        ++it;
      }
      if(!exists) {
        name = securityName;
      }
      if(symbl.isEmpty()) {
        break;
      }
      symTableRow ++;
      m_dlg->m_symbolTableDlg->displayLine(symTableRow, symbl, name, exists);
      m_dlg->m_investProcessing->m_symbolsList << symbl;
      m_dlg->m_investProcessing->m_map.insert(symbl, name);
      if(done) {
        break;
      }
    }
  }
  if(symTableRow > -1) {
    m_dlg->m_symbolTableDlg->show();
  }
  connect(m_dlg->m_symbolTableDlg->m_widget->tableWidget,  SIGNAL(itemChanged(QTableWidgetItem*)), m_dlg->m_symbolTableDlg,  SLOT(slotItemChanged(QTableWidgetItem*)));

  return true;
}

CompletionPage::CompletionPage(QWidget* parent) : QWizardPage(parent), ui(new Ui::CompletionPage)
{
  ui->setupUi(this);

  m_pageLayout = new QVBoxLayout;

  ui->horizontalLayout->insertLayout(0, m_pageLayout, 1);
}

CompletionPage::~CompletionPage()
{
  delete ui;
}

void CompletionPage::setParent(CSVDialog* dlg)
{
  m_dlg = dlg;
}

void CompletionPage::initializePage()
{
  if((m_dlg->m_fileType == "Banking") || (field("symbolCol").toInt() == -1)) { //  Only show symbols dialog if that field is set, and not Banking.
    return;
  }
  m_dlg->m_importIsValid = false;
}

void CompletionPage::slotImportValid()
{
  QList<QWizard::WizardButton> layout;

  layout << QWizard::Stretch << QWizard::BackButton << QWizard::CustomButton2 <<  QWizard::CancelButton;
  wizard()->setOption(QWizard::HaveCustomButton2, true);
  wizard()->setButtonText(QWizard::CustomButton2, "Import CSV");
  wizard()->setButtonLayout(layout);
  m_dlg->m_importIsValid = true;
}

void CompletionPage::slotImportClicked()
{
  QList<QWizard::WizardButton> layout;
  layout << QWizard::Stretch << QWizard::BackButton << QWizard::CustomButton2 << QWizard::CustomButton3
         <<  QWizard::FinishButton <<  QWizard::CancelButton;
  wizard()->setOption(QWizard::HaveCustomButton2, true);
  wizard()->setButtonText(QWizard::CustomButton2, "Import CSV");
  wizard()->setOption(QWizard::HaveCustomButton3, false);
  wizard()->setButtonText(QWizard::CustomButton3, "Make QIF File");
  wizard()->button(QWizard::CustomButton3)->setEnabled(true);
  m_dlg->m_isTableTrimmed = true;

  wizard()->setButtonLayout(layout);

  switch(field("source").toInt()) {
    case 0:
      emit importBanking();
      break;
    default:
      emit importInvestment();
      break;
  }
}

void CompletionPage::cleanupPage()
{
  QList<QWizard::WizardButton> layout;
  layout << QWizard::Stretch << QWizard::BackButton << QWizard::NextButton <<  QWizard::CancelButton;
  wizard()->setButtonLayout(layout);
  m_dlg->m_investProcessing->reloadUI();
}

bool CompletionPage::validatePage()
{
  emit completeChanged();
  return true;
}
