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
#include <kjobwidgets.h>
#include <kio/job.h>

#include "convdate.h"
#include "csvutil.h"

#include "mymoneyfile.h"

#include "ui_csvwizard.h"
#include "ui_introwizardpage.h"
#include "ui_separatorwizardpage.h"
#include "ui_rowswizardpage.h"
#include "ui_bankingwizardpage.h"
#include "ui_formatswizardpage.h"
#include "ui_investmentwizardpage.h"

CSVWizard::CSVWizard() :
    ui(new Ui::CSVWizard),
    m_pageIntro(0),
    m_pageSeparator(0),
    m_pageBanking(0),
    m_pageInvestment(0)
{
  ui->setupUi(this);

  m_parse = new Parse;
  m_convertDate = new ConvertDate;
  m_csvUtil = new CsvUtil;

  st = MyMoneyStatement();
  m_curId = -1;
  m_lastId = -1;
  m_maxColumnCount = 0;
  m_importError = false;
  m_fileEndLine = 0;
  ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

  m_wizard = new QWizard;
  m_wizard->setWizardStyle(QWizard::ClassicStyle);
  ui->horizontalLayout->addWidget(m_wizard);
  m_wizard->installEventFilter(this); // event filter for escape key presses

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
}

void CSVWizard::init()
{
  m_pageIntro = new IntroPage;
  m_wizard->setPage(PageIntro, m_pageIntro);
  m_pageIntro->setParent(this);

  m_pageSeparator = new SeparatorPage;
  m_wizard->setPage(PageSeparator, m_pageSeparator);
  m_pageSeparator->setParent(this);

  m_pageRows = new RowsPage;
  m_wizard->setPage(PageRows, m_pageRows);
  m_pageRows->setParent(this);

  m_pageFormats = new FormatsPage;
  m_wizard->setPage(PageFormats, m_pageFormats);
  m_pageFormats->setParent(this);

  m_stageLabels << ui->label_intro << ui->label_separators << ui->label_rows << ui->label_columns << ui->label_columns << ui->label_formats;
  m_pageFormats->setFinalPage(true);

  this->setAttribute(Qt::WA_DeleteOnClose, true);

  m_profileList.clear();
  findCodecs();
  m_config = KSharedConfig::openConfig(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
                                       QDir::separator() +
                                       "csvimporterrc");
  validateConfigFile(m_config);
  readMiscSettings(m_config);

  connect(m_pageFormats, SIGNAL(statementReady(MyMoneyStatement&)), m_plugin, SLOT(slotGetStatement(MyMoneyStatement&)));

  connect(m_wizard->button(QWizard::FinishButton), SIGNAL(clicked()), this, SLOT(slotClose()));
  connect(m_wizard->button(QWizard::CancelButton), SIGNAL(clicked()), this, SLOT(close()));
  connect(m_wizard->button(QWizard::CustomButton1), SIGNAL(clicked()), this, SLOT(slotFileDialogClicked()));
  connect(m_wizard->button(QWizard::CustomButton2), SIGNAL(clicked()), m_pageFormats, SLOT(slotImportClicked()));
  connect(m_wizard->button(QWizard::CustomButton3), SIGNAL(clicked()), m_pageFormats, SLOT(slotSaveAsQIFClicked()));
  connect(m_wizard, SIGNAL(currentIdChanged(int)), this, SLOT(slotIdChanged(int)));

  ui->tableWidget->setWordWrap(false);

  m_vScrollBar = ui->tableWidget->verticalScrollBar();
  m_vScrollBar->setTracking(false);
  m_dateFormats << "yyyy/MM/dd" << "MM/dd/yyyy" << "dd/MM/yyyy";

  m_clearBrush = KColorScheme(QPalette::Normal).background(KColorScheme::NormalBackground);
  m_clearBrushText = KColorScheme(QPalette::Normal).foreground(KColorScheme::NormalText);
  m_colorBrush = KColorScheme(QPalette::Normal).background(KColorScheme::PositiveBackground);
  m_colorBrushText = KColorScheme(QPalette::Normal).foreground(KColorScheme::PositiveText);
  m_errorBrush = KColorScheme(QPalette::Normal).background(KColorScheme::NegativeBackground);
  m_errorBrushText = KColorScheme(QPalette::Normal).foreground(KColorScheme::NegativeText);

  int y = (QApplication::desktop()->height() - this->height()) / 2;
  int x = (QApplication::desktop()->width() - this->width()) / 2;
  move(x, y);
  show();
}

CSVWizard::~CSVWizard()
{
  delete ui;
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

void CSVWizard::readMiscSettings(const KSharedConfigPtr& config) {
  KConfigGroup miscGroup(config, "Misc");
  m_initialWidth = miscGroup.readEntry("Width", 800);
  m_initialHeight = miscGroup.readEntry("Height", 400);
  m_autodetect.clear();
  m_autodetect.insert(CSVWizard::AutoFieldDelimiter, miscGroup.readEntry("AutoFieldDelimiter", true));
  m_autodetect.insert(CSVWizard::AutoDecimalSymbol, miscGroup.readEntry("AutoDecimalSymbol", true));
  m_autodetect.insert(CSVWizard::AutoDateFormat, miscGroup.readEntry("AutoDateFormat", true));
  m_autodetect.insert(CSVWizard::AutoAccountInvest, miscGroup.readEntry("AutoAccountInvest", true));
  m_autodetect.insert(CSVWizard::AutoAccountBank, miscGroup.readEntry("AutoAccountBank", true));
}

void CSVWizard::saveWindowSize(const KSharedConfigPtr& config) {
  KConfigGroup miscGroup(config, "Misc");
  m_initialHeight = this->geometry().height();
  m_initialWidth = this->geometry().width();
  miscGroup.writeEntry("Width", m_initialWidth);
  miscGroup.writeEntry("Height", m_initialHeight);
  miscGroup.sync();
}

bool CSVWizard::updateConfigFile(const KSharedConfigPtr& config, const QList<int>& kmmVer)
{
  QString configFilePath = config.constData()->name();
  QFile::copy(configFilePath, configFilePath + ".bak");

  KConfigGroup profileNamesGroup(config, "ProfileNames");
  QStringList bankProfiles = profileNamesGroup.readEntry("Bank", QStringList());
  QStringList investProfiles = profileNamesGroup.readEntry("Invest", QStringList());
  QStringList invalidBankProfiles = profileNamesGroup.readEntry("InvalidBank", QStringList());     // get profiles that was marked invalid during last update
  QStringList invalidInvestProfiles = profileNamesGroup.readEntry("InvalidInvest", QStringList());
  QString bankPrefix = "Bank-";
  QString investPrefix = "Invest-";

  uint version = kmmVer[0]*100 + kmmVer[1]*10 + kmmVer[2];

  // for kmm < 5.0.0 change 'BankNames' to 'ProfileNames' and remove 'MainWindow' group
  if (version < 500 && bankProfiles.isEmpty()) {
    KConfigGroup oldProfileNamesGroup(config, "BankProfiles");
    bankProfiles = oldProfileNamesGroup.readEntry("BankNames", QStringList()); // profile names are under 'BankNames' entry for kmm < 5.0.0
    bankPrefix = "Profiles-";   // needed to remove non-existent profiles in first run
    oldProfileNamesGroup.deleteGroup();
    KConfigGroup oldMainWindowGroup(config, "MainWindow");
    oldMainWindowGroup.deleteGroup();
  }

  bool firstTry = false;
  if (invalidBankProfiles.isEmpty() && invalidInvestProfiles.isEmpty())  // if there is no invalid profiles then this might be first update try
    firstTry = true;

  bool ret = true;
  int invalidProfileResponse = QMessageBox::No;

  for (QStringList::Iterator profileName = bankProfiles.begin(); profileName != bankProfiles.end(); ++profileName) {

    KConfigGroup bankProfile(config, bankPrefix + *profileName);
    if (!bankProfile.exists() && !invalidBankProfiles.contains(*profileName)) { // if there is reference to profile but no profile then remove this reference
      profileName = bankProfiles.erase(profileName);
      profileName--;
      continue;
    }

    // for kmm < 5.0.0 remove 'FileType' and 'ProfileName' and assign them to either "Bank=" or "Invest="
    if (version < 500) {
      KConfigGroup oldBankProfile(config, "Profiles-" + *profileName);  // if half of configuration is updated and the other one untouched this is needed
      QString oldProfileType = oldBankProfile.readEntry("FileType", QString());
      KConfigGroup newProfile;
      if (oldProfileType == "Invest") {
        newProfile = KConfigGroup(config, "Invest-" + *profileName);
        investProfiles.append(*profileName);
        profileName = bankProfiles.erase(profileName);
        profileName--;
      }
      else if (oldProfileType == "Banking")
        newProfile = KConfigGroup(config, "Bank-" + *profileName);
      else {
        if (invalidProfileResponse != QMessageBox::YesToAll && invalidProfileResponse != QMessageBox::NoToAll) {
          if (!firstTry &&
              !invalidBankProfiles.contains(*profileName)) // if it isn't first update run and profile isn't on the list of invalid ones then don't bother
            continue;
        invalidProfileResponse = QMessageBox::warning(m_wizard, i18n("CSV import"),
                                       i18n("<center>During update of <b>%1</b><br>"
                                            "the profile type for <b>%2</b> couldn't be recognized.<br>"
                                            "The profile cannot be used because of that.<br>"
                                            "Do you want to delete it?</center>",
                                            configFilePath, *profileName),
                                       QMessageBox::Yes | QMessageBox::YesToAll |
                                       QMessageBox::No | QMessageBox::NoToAll, QMessageBox::No );
        }

        switch (invalidProfileResponse) {
        case QMessageBox::YesToAll:
        case QMessageBox::Yes:
          oldBankProfile.deleteGroup();
          invalidBankProfiles.removeOne(*profileName);
          profileName = bankProfiles.erase(profileName);
          --profileName;
          break;
        case QMessageBox::NoToAll:
        case QMessageBox::No:
          if (!invalidBankProfiles.contains(*profileName))  // on user request: don't delete profile but keep eye on it
            invalidBankProfiles.append(*profileName);
          ret = false;
          break;
        }
        continue;
      }
      oldBankProfile.deleteEntry("FileType");
      oldBankProfile.deleteEntry("ProfileName");
      oldBankProfile.deleteEntry("DebitFlag");
      oldBankProfile.deleteEntry("InvDirectory");
      oldBankProfile.deleteEntry("CsvDirectory");
      oldBankProfile.copyTo(&newProfile);
      oldBankProfile.deleteGroup();
      newProfile.sync();
      oldBankProfile.sync();
    }
  }

  for (QStringList::Iterator profileName = investProfiles.begin(); profileName != investProfiles.end(); ++profileName) {

    KConfigGroup investProfile(config, investPrefix + *profileName);
    if (!investProfile.exists() && !invalidInvestProfiles.contains(*profileName)) { // if there is reference to profile but no profile then remove this reference
      investProfiles.erase(profileName);
      continue;
    }
  }

  profileNamesGroup.writeEntry("Bank", bankProfiles); // update profile names as some of them might have been changed
  profileNamesGroup.writeEntry("Invest", investProfiles);

  if (invalidBankProfiles.isEmpty())  // if no invalid profiles then we don't need this variable anymore
    profileNamesGroup.deleteEntry("InvalidBank");
  else
    profileNamesGroup.writeEntry("InvalidBank", invalidBankProfiles);

  if (invalidInvestProfiles.isEmpty())
    profileNamesGroup.deleteEntry("InvalidInvest");
  else
    profileNamesGroup.writeEntry("InvalidInvest", invalidInvestProfiles);

  if (ret)
    QFile::remove(configFilePath + ".bak"); // remove backup if all is ok

  return ret;
}

void CSVWizard::validateConfigFile(const KSharedConfigPtr& config)
{
  KConfigGroup profileNamesGroup(config, "ProfileNames");
  if (!profileNamesGroup.exists()) {
    profileNamesGroup.writeEntry("Bank", QStringList());
    profileNamesGroup.writeEntry("Invest", QStringList());
    profileNamesGroup.writeEntry("PriorBank", int());
    profileNamesGroup.writeEntry("PriorInvest", int());
    profileNamesGroup.sync();
  }

  KConfigGroup miscGroup(config, "Misc");
  if (!miscGroup.exists()) {
    miscGroup.writeEntry("Height", "400");
    miscGroup.writeEntry("Width", "800");
    miscGroup.sync();
  }

  QList<int> kmmVer = miscGroup.readEntry("KMMVer", QList<int> {0, 0, 0});
  QList<int> curKmmVer =  QList<int> {5, 0, 0};
  if (curKmmVer != kmmVer) {
    if (updateConfigFile(config, kmmVer)) // write kmmVer only if there were no errors
      miscGroup.writeEntry("KMMVer", curKmmVer);
  }

  KConfigGroup securitiesGroup(config, "Securities");
  if (!securitiesGroup.exists()) {
    securitiesGroup.writeEntry("SecurityNameList", QStringList());
    securitiesGroup.sync();
  }
}

void CSVWizard::setCodecList(const QList<QTextCodec *> &list, QComboBox* comboBoxEncode)
{
  comboBoxEncode->clear();
  foreach (QTextCodec * codec, list)
    comboBoxEncode->addItem(codec->name(), codec->mibEnum());
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

void CSVWizard::clearColumnsBackground(int col) {
  QList<int> columnList;
  columnList << col;
  clearColumnsBackground(columnList);
}

void CSVWizard::clearColumnsBackground(QList<int>& columnList)
{
  for (int row = m_startLine -1 ; row < m_endLine; ++row) {
    for (QList<int>::const_iterator col = columnList.constBegin(); col < columnList.constEnd(); ++col) {
      QTableWidgetItem* item = ui->tableWidget->item(row, *col);
      item->setBackground(m_clearBrush);
      item->setForeground(m_clearBrushText);
    }
  }
}

void CSVWizard::clearBackground()
{
  for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
    for (int col = 0; col < ui->tableWidget->columnCount(); ++col) {
      QTableWidgetItem *item = ui->tableWidget->item(row, col);
      item->setBackground(m_clearBrush);
      item->setForeground(m_clearBrushText);
    }
  }
}

void CSVWizard::markUnwantedRows()
{
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

QList<MyMoneyAccount> CSVWizard::findAccounts(QList<MyMoneyAccount::accountTypeE> &accountTypes, QString& statementHeader)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  QList<MyMoneyAccount> accountList;
  file->accountList(accountList);
  QList<MyMoneyAccount> filteredTypes;
  QList<MyMoneyAccount> filteredAccounts;
  QList<MyMoneyAccount>::iterator account;
  QRegExp filterOutChars = QRegExp("-., ");

  for (account = accountList.begin(); account != accountList.end(); ++account) {
    if (accountTypes.contains((*account).accountType()) && !(*account).isClosed())
        filteredTypes << *account;
  }

  // filter out accounts whose names aren't in statements header
  for (account = filteredTypes.begin(); account != filteredTypes.end(); ++account) {
    QString txt = (*account).name();
    txt = txt.replace(filterOutChars, "");
    if (statementHeader.contains(txt, Qt::CaseInsensitive))
      filteredAccounts << *account;
  }

  // if filtering returned more results, filter out accounts whose numbers aren't in statements header
  if (filteredAccounts.count() > 1) {
    for (account = filteredAccounts.begin(); account != filteredAccounts.end();) {
      QString txt = (*account).number();
      txt = txt.replace(filterOutChars, "");
      if (txt.isEmpty() || txt.length() < 3) {
        ++account;
        continue;
      }
      if (statementHeader.contains(txt, Qt::CaseInsensitive))
        ++account;
      else
        account = filteredAccounts.erase(account);
    }
  }

  // if filtering by name and number didn't return nothing, then try filtering by number only
  if (filteredAccounts.isEmpty()) {
    for (account = filteredTypes.begin(); account != filteredTypes.end(); ++account) {
      QString txt = (*account).number();
      txt = txt.replace(filterOutChars, "");
      if (statementHeader.contains(txt, Qt::CaseInsensitive))
        filteredAccounts << *account;
    }
  }
  return filteredAccounts;
}

bool CSVWizard::detectAccount(MyMoneyStatement& st)
{
  QString statementHeader;
  for (int row = 0; row < m_startLine - 1; ++row) // concatenate header for better search
    statementHeader +=  m_lineList.value(row);

  QRegExp filterOutChars = QRegExp("-., ");
  statementHeader.replace(filterOutChars, "");

  QList<MyMoneyAccount> accounts;
  QList<MyMoneyAccount::accountTypeE> accountTypes;

  if (m_profileType == CSVWizard::ProfileBank) {
    accountTypes << MyMoneyAccount::Checkings <<
                    MyMoneyAccount::Savings <<
                    MyMoneyAccount::Liability <<
                    MyMoneyAccount::Checkings <<
                    MyMoneyAccount::Savings <<
                    MyMoneyAccount::Cash <<
                    MyMoneyAccount::CreditCard <<
                    MyMoneyAccount::Loan <<
                    MyMoneyAccount::Asset <<
                    MyMoneyAccount::Liability;
    accounts = findAccounts(accountTypes, statementHeader);
  } else if (m_profileType == CSVWizard::ProfileInvest) {
    accountTypes << MyMoneyAccount::Investment; // take investment accounts...
    accounts = findAccounts(accountTypes, statementHeader); //...and search them in statement header
  }

  if (accounts.count() == 1) { // set account in statement, if it was the only one match
    st.m_strAccountName = accounts.first().name();
    st.m_strAccountNumber = accounts.first().number();
    st.m_accountId = accounts.first().id();

    switch (accounts.first().accountType()) {
    case MyMoneyAccount::Checkings:
      st.m_eType=MyMoneyStatement::etCheckings;
      break;
    case MyMoneyAccount::Savings:
      st.m_eType=MyMoneyStatement::etSavings;
      break;
    case MyMoneyAccount::Investment:
      st.m_eType=MyMoneyStatement::etInvestment;
      break;
    case MyMoneyAccount::CreditCard:
      st.m_eType=MyMoneyStatement::etCreditCard;
      break;
    default:
      st.m_eType=MyMoneyStatement::etNone;
    }
    return true;
  }
  return false;
}

bool CSVWizard::detectDecimalSymbol(const int col, int& symbol)
{
  if (symbol != 2)
    return true;

  // get list of used currencies to remove them from col
  QList<MyMoneyAccount> accountList;
  MyMoneyFile::instance()->accountList(accountList);

  QList<MyMoneyAccount::accountTypeE> accountTypes;
  accountTypes << MyMoneyAccount::Checkings <<
                  MyMoneyAccount::Savings <<
                  MyMoneyAccount::Liability <<
                  MyMoneyAccount::Checkings <<
                  MyMoneyAccount::Savings <<
                  MyMoneyAccount::Cash <<
                  MyMoneyAccount::CreditCard <<
                  MyMoneyAccount::Loan <<
                  MyMoneyAccount::Asset <<
                  MyMoneyAccount::Liability;

  QSet<QString> currencySymbols;
  for (QList<MyMoneyAccount>::ConstIterator account = accountList.cbegin(); account != accountList.cend(); ++account) {
    if (accountTypes.contains((*account).accountType())) {                                                // account must actually have currency property
      currencySymbols.insert((*account).currencyId());                                                    // add currency id
      currencySymbols.insert(MyMoneyFile::instance()->currency((*account).currencyId()).tradingSymbol()); // add currency symbol
    }
  }
  QString filteredCurrencies = QStringList(currencySymbols.values()).join("");
  QString pattern = QString("%1%2").arg(QLocale().currencySymbol()).arg(filteredCurrencies);
  QRegularExpression re("^[\\(+-]?\\d+[\\)]?$"); // matches '0' ; '+12' ; '-345' ; '(6789)'

  bool dotIsDecimalSeparator = false;
  bool commaIsDecimalSeparator = false;
  for (int row = m_startLine - 1; row < m_endLine; ++row) {
    QString txt = ui->tableWidget->item(row, col)->text();
    if (txt.isEmpty())  // nothing to process, so go to next row
      continue;
    int dotPos = txt.lastIndexOf(".");   // get last positions of decimal/thousand separator...
    int commaPos = txt.lastIndexOf(","); // ...to be able to determine which one is the last

    if (dotPos != -1 && commaPos != -1) {
      if (dotPos > commaPos && commaIsDecimalSeparator == false)    // follwing case 1,234.56
        dotIsDecimalSeparator = true;
      else if (dotPos < commaPos && dotIsDecimalSeparator == false) // follwing case 1.234,56
        commaIsDecimalSeparator = true;
      else                                                          // follwing case 1.234,56 and somwhere earlier there was 1,234.56 so unresolvable conflict
        return false;
    } else if (dotPos != -1) {                 // follwing case 1.23
      if (dotIsDecimalSeparator)               // it's already know that dotIsDecimalSeparator
        continue;
      if (!commaIsDecimalSeparator)            // if there is no conflict with comma as decimal separator
        dotIsDecimalSeparator = true;
      else {
        if (txt.count('.') > 1)                // follwing case 1.234.567 so OK
          continue;
        else if (txt.length() - 4 == dotPos)   // follwing case 1.234 and somwhere earlier there was 1.234,56 so OK
          continue;
        else                                   // follwing case 1.23 and somwhere earlier there was 1,23 so unresolvable conflict
          return false;
      }
    } else if (commaPos != -1) {               // follwing case 1,23
      if (commaIsDecimalSeparator)             // it's already know that commaIsDecimalSeparator
        continue;
      else if (!dotIsDecimalSeparator)         // if there is no conflict with dot as decimal separator
        commaIsDecimalSeparator = true;
      else {
        if (txt.count(',') > 1)                // follwing case 1,234,567 so OK
          continue;
        else if (txt.length() - 4 == commaPos) // follwing case 1,234 and somwhere earlier there was 1,234.56 so OK
          continue;
        else                                   // follwing case 1,23 and somwhere earlier there was 1.23 so unresolvable conflict
          return false;
      }

    } else {                                   // follwing case 123
      txt.remove(QRegularExpression("[ " + QRegularExpression::escape(pattern) + ']'));
      QRegularExpressionMatch match = re.match(txt);
      if (match.hasMatch()) // if string is pure numerical then go forward...
        continue;
      else    // ...if not then it's non-numerical garbage
        return false;
    }
  }

  if (dotIsDecimalSeparator)
    symbol = 0;
  else if (commaIsDecimalSeparator)
    symbol = 1;
  else {  // whole column was empty, but we don't want to fail so take os decimal symbol
    if (QLocale().decimalPoint() == '.')
      symbol = 0;
    else
      symbol = 1;
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

bool CSVWizard::getInFileName(QString& inFileName)
{
  if (inFileName.isEmpty())
    inFileName = QDir::homePath();

  if(inFileName.startsWith("~/"))  //expand Linux home directory
    inFileName.replace(0, 1, QDir::home().absolutePath());

  QFileInfo fileInfo = QFileInfo(inFileName);
  if (fileInfo.isFile()) {    // if it is file...
    if (fileInfo.exists())  // ...and exists...
      return true;          // ...then all is OK...
    else {                  // ...but if not...
      fileInfo.setFile(fileInfo.absolutePath()); //...then set start directory to directory of that file...
      if (!fileInfo.exists())                    //...and if it doesn't exist too...
        fileInfo.setFile(QDir::homePath());      //...then set start directory to home path
    }
  }

  QPointer<QFileDialog> dialog = new QFileDialog(this, QString(),
                                                 fileInfo.absoluteFilePath(),
                                                 i18n("CSV Files (*.csv)"));
  dialog->setOption(QFileDialog::DontUseNativeDialog, true);  //otherwise we cannot add custom QComboBox
  dialog->setFileMode(QFileDialog::ExistingFile);
  QPointer<QLabel> label = new QLabel(i18n("Encoding"));
  dialog->layout()->addWidget(label);
  //    Add encoding selection to FileDialog
  QPointer<QComboBox> comboBoxEncode = new QComboBox();
  setCodecList(m_codecs, comboBoxEncode);
  comboBoxEncode->setCurrentIndex(m_encodeIndex);
  connect(comboBoxEncode, SIGNAL(activated(int)), this, SLOT(encodingChanged(int)));
  dialog->layout()->addWidget(comboBoxEncode);
  QUrl url;
  if (dialog->exec() == QDialog::Accepted)
    url = dialog->selectedUrls().first();
  else
    url.clear();
  delete dialog;

  if (url.isEmpty())
    return false;
  else if (url.isLocalFile())
    inFileName = url.toLocalFile();
  else {
    inFileName = QDir::tempPath();
    if (!inFileName.endsWith(QDir::separator()))
      inFileName += QDir::separator();
    inFileName += url.fileName();
    qDebug() << "Source:" << url.toDisplayString() << "Destination:" << inFileName;
    KIO::FileCopyJob *job = KIO::file_copy(url, QUrl::fromUserInput(inFileName),
                                           -1, KIO::Overwrite);
    KJobWidgets::setWindow(job, this);
    job->exec();
    if (job->error()) {
      KMessageBox::detailedError(this,
                                 i18n("Error while loading file '%1'.", url.toDisplayString()),
                                 job->errorString(),
                                 i18n("File access error"));
      return false;
    }
  }

  if (inFileName.isEmpty())
    return false;
  return true;
}

void CSVWizard::encodingChanged(int index)
{
  m_encodeIndex = index;
}

void CSVWizard::readFile(const QString& fname)
{
  if (!fname.isEmpty())
    m_inFileName = fname;

  m_importError = false;

  QFile  inFile(m_inFileName);
  inFile.open(QIODevice::ReadOnly);  // allow a Carriage return -// QIODevice::Text
  QTextStream inStream(&inFile);
  QTextCodec* codec = QTextCodec::codecForMib(m_codecs.value(m_encodeIndex)->mibEnum());
  inStream.setCodec(codec);

  QString buf = inStream.readAll();
  inFile.close();
  m_lineList = m_parse->parseFile(buf, 1, 0);  // parse the buffer
  m_fileEndLine = m_parse->lastLine(); // won't work without above line

  if (m_fileEndLine > m_trailerLines)
    m_endLine = m_fileEndLine - m_trailerLines;
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

  for (int line = 0; line < lineList.count(); ++line) {
    QStringList columnList = parse->parseLine(lineList[line]);
    for (int col = 0; col < columnList.count(); ++col) {
      QTableWidgetItem *item = new QTableWidgetItem;  // new item for tableWidget
      item->setText(columnList[col]);
      ui->tableWidget->setItem(m_row, col, item);  // add item to tableWidget
    }
    if (columnList.count() < m_maxColumnCount) {  // if 'header' area has less columns than 'data' area, then fill this area with whitespaces for nice effect with markUnwantedRows
      for (int col = columnList.count(); col < m_maxColumnCount; ++col) {
        QTableWidgetItem *item = new QTableWidgetItem; // new item for tableWidget
        item->setText("");
        ui->tableWidget->setItem(m_row, col, item);  // add item to tableWidget
      }
    }
    m_row ++;
  }

  for (int col = 0; col < ui->tableWidget->columnCount(); col ++)
    ui->tableWidget->resizeColumnToContents(col);
}

void CSVWizard::updateWindowSize()
{
  QTableWidget *table = this->ui->tableWidget;
  table->resizeColumnsToContents();
  layout()->invalidate();
  layout()->activate();

  QRect screen = QApplication::desktop()->availableGeometry();    //get available screen size
  QRect wizard = this->frameGeometry();                           //get current wizard size

  int newWidth = table->verticalHeader()->width() +               //take header, margins nad scrollbar into account
                 table->contentsMargins().left() +
                 table->contentsMargins().right();
  if (table->verticalScrollBar()->isEnabled())
    newWidth += table->verticalScrollBar()->width();
  for(int i = 0; i < table->columnCount(); ++i)
    newWidth += table->columnWidth(i);                            //add up required column widths

  int newHeight = table->horizontalHeader()->height() +
                  table->horizontalScrollBar()->height() +
                  table->contentsMargins().top() +
                  table->contentsMargins().bottom();
  if (table->horizontalScrollBar()->isEnabled())
    newHeight += table->horizontalScrollBar()->height();

  if( this->ui->tableWidget->rowCount() > 0)
    newHeight += this->ui->tableWidget->rowCount() * table->rowHeight(0); //add up estimated row heights

  newWidth = wizard.width() + (newWidth - table->width());
  newHeight = wizard.height() + (newHeight - table->height());

  if (newWidth > screen.width())  //limit wizard size to screen size
    newWidth = screen.width();
  if (newHeight > screen.height())
    newHeight = screen.height();

  if (newWidth < this->m_initialWidth) //don't shrink wizard if required size is less than initial
    newWidth = this->m_initialWidth;
  if (newHeight < this->m_initialHeight)
    newHeight = this->m_initialHeight;

  newWidth -= (wizard.width() - this->geometry().width());      // remove window frame
  newHeight -= (wizard.height() - this->geometry().height());

  wizard.setWidth(newWidth);
  wizard.setHeight(newHeight);
  wizard.moveTo((screen.width() - wizard.width()) / 2,
                (screen.height() - wizard.height()) / 2);
  this->setGeometry(wizard);
}

void CSVWizard::slotFileDialogClicked()
{
  saveWindowSize(m_config);
  if (!m_pageInvestment.isNull())
    m_wizard->removePage(PageInvestment);
  delete m_pageInvestment;
  if (!m_pageBanking.isNull())
    m_wizard->removePage(PageBanking);
  delete m_pageBanking;

  m_profileName = m_pageIntro->ui->combobox_source->currentText();
  m_skipSetup = m_pageIntro->ui->checkBoxSkipSetup->isChecked();
  m_accept = false;
  m_acceptAllInvalid = false;  //  Don't accept further invalid values.
  m_memoColList.clear();

  if (m_profileType == CSVWizard::ProfileInvest) {
    m_pageInvestment = new InvestmentPage;
    m_wizard->setPage(PageInvestment, m_pageInvestment);
    m_pageInvestment->setParent(this);
    m_pageInvestment->readSettings(m_config);
  }
  else if (m_profileType == CSVWizard::ProfileBank) {
    m_pageBanking = new BankingPage;
    m_wizard->setPage(PageBanking, m_pageBanking);
    m_pageBanking->setParent(this);
    m_pageBanking->readSettings(m_config);
  }

  if (!getInFileName(m_inFileName))
    return;
  readFile(m_inFileName);

  m_wizard->next();  //go to separator page

  if (m_skipSetup)
    for (int i = 0; i < 4; i++) //programmaticaly go through separator-, rows-, investment-/bank-, formatspage
      m_wizard->next();
}

void CSVWizard::resizeEvent(QResizeEvent* ev)
{
  if (ev->spontaneous()) {
    ev->ignore();
    return;
  }
}

//-------------------------------------------------------------------------------------------------------
IntroPage::IntroPage(QDialog *parent) :
    CSVWizardPage(parent),
    ui(new Ui::IntroPage),
    m_pageLayout(0)
{
  ui->setupUi(this);
}

IntroPage::~IntroPage()
{
  delete ui;
}

void IntroPage::setParent(CSVWizard* dlg)
{
  CSVWizardPage::setParent(dlg);
  m_wizDlg->showStage();

  wizard()->button(QWizard::CustomButton1)->setEnabled(false);
}

void IntroPage::slotAddProfile()
{
  profileChanged(ProfileAdd);
}

void IntroPage::slotRemoveProfile()
{
  profileChanged(ProfileRemove);
}

void IntroPage::slotRenameProfile()
{
  profileChanged(ProfileRename);
}

void IntroPage::profileChanged(const profileActionsE& action)
{
  int cbIndex = ui->combobox_source->currentIndex();
  QString cbText = ui->combobox_source->currentText();
  if (cbText.isEmpty()) // you cannot neither add nor remove empty name profile or rename to empty name
    return;

  QString profileTypeStr;
  if (m_wizDlg->m_profileType == CSVWizard::ProfileBank)
    profileTypeStr = "Bank";
  else if (m_wizDlg->m_profileType == CSVWizard::ProfileInvest)
    profileTypeStr = "Invest";

  KConfigGroup profileNamesGroup(m_wizDlg->m_config, "ProfileNames");
  KConfigGroup currentProfileName(m_wizDlg->m_config, profileTypeStr + '-' + cbText);
  if (action == ProfileRemove) {
    if (m_wizDlg->m_profileList.value(cbIndex) != cbText)
      return;
    m_wizDlg->m_profileList.removeAt(cbIndex);
    currentProfileName.deleteGroup();
    ui->combobox_source->removeItem(cbIndex);
    KMessageBox::information(m_wizDlg->m_wizard,
                             i18n("<center>Profile <b>%1</b> has been removed.</center>",
                                  cbText));
  }

  if (action == ProfileAdd || action == ProfileRename) {
    int dupIndex = m_wizDlg->m_profileList.indexOf(cbText, Qt::CaseInsensitive);
    if (dupIndex == cbIndex && cbIndex != -1)  // if profile name wasn't changed then return
      return;
    else if (dupIndex != -1) {    // profile with the same name already exists
      ui->combobox_source->setItemText(cbIndex, m_wizDlg->m_profileList.value(cbIndex));
      KMessageBox::information(m_wizDlg->m_wizard,
                               i18n("<center>Profile <b>%1</b> already exists.<br>"
                                    "Please enter another name</center>", cbText));
      return;
    }

    if (action == ProfileAdd) {
      m_wizDlg->m_profileList.append(cbText);
      ui->combobox_source->addItem(cbText);
      ui->combobox_source->setCurrentIndex(m_wizDlg->m_profileList.count() - 1);
      currentProfileName.writeEntry("Directory", QString());
      KMessageBox::information(m_wizDlg->m_wizard,
                               i18n("<center>Profile <b>%1</b> has been added.</center>", cbText));
    } else if (action == ProfileRename) {
      KConfigGroup oldProfileName(m_wizDlg->m_config, profileTypeStr + '-' + m_wizDlg->m_profileList.value(cbIndex));
      oldProfileName.copyTo(&currentProfileName);
      oldProfileName.deleteGroup();
      ui->combobox_source->setItemText(cbIndex, cbText);
      oldProfileName.sync();
      KMessageBox::information(m_wizDlg->m_wizard,
                               i18n("<center>Profile name has been renamed from <b>%1</b> to <b>%2</b>.</center>",
                                    m_wizDlg->m_profileList.value(cbIndex), cbText));
      m_wizDlg->m_profileList[cbIndex] = cbText;
    }
  }
  currentProfileName.sync();
  profileNamesGroup.writeEntry(profileTypeStr, m_wizDlg->m_profileList);  // update profiles list
  profileNamesGroup.sync();
}

void IntroPage::slotComboSourceIndexChanged(int idx)
{
  if (idx == -1) {
    wizard()->button(QWizard::CustomButton1)->setEnabled(false);
    ui->checkBoxSkipSetup->setEnabled(false);
    ui->buttonRemove->setEnabled(false);
    ui->buttonRename->setEnabled(false);
  }
  else {
    wizard()->button(QWizard::CustomButton1)->setEnabled(true);
    ui->checkBoxSkipSetup->setEnabled(true);
    ui->buttonRemove->setEnabled(true);
    ui->buttonRename->setEnabled(true);
  }
}

void IntroPage::profileTypeChanged(const CSVWizard::profileTypeE profileType, bool toggled)
{
  if (!toggled)
    return;

  KConfigGroup profilesGroup(m_wizDlg->m_config, "ProfileNames");
  m_wizDlg->m_profileType = profileType;
  QString profileTypeStr;
  int priorProfile;
  if (m_wizDlg->m_profileType == CSVWizard::ProfileBank) {
    ui->radioButton_invest->setChecked(false);
    profileTypeStr = "Bank";
  } else if (m_wizDlg->m_profileType == CSVWizard::ProfileInvest) {
    ui->radioButton_bank->setChecked(false);
    profileTypeStr = "Invest";
  }

  m_wizDlg->m_profileList = profilesGroup.readEntry(profileTypeStr, QStringList());
  priorProfile = profilesGroup.readEntry("Prior" + profileTypeStr, 0);
  ui->combobox_source->clear();
  ui->combobox_source->addItems(m_wizDlg->m_profileList);
  ui->combobox_source->setCurrentIndex(priorProfile);
  ui->combobox_source->setEnabled(true);
  ui->buttonAdd->setEnabled(true);
}

void IntroPage::slotBankRadioToggled(bool toggled)
{
  profileTypeChanged(CSVWizard::ProfileBank, toggled);
}

void IntroPage::slotInvestRadioToggled(bool toggled)
{
  profileTypeChanged(CSVWizard::ProfileInvest, toggled);
}

void IntroPage::initializePage()
{
  m_wizDlg->ui->tableWidget->clear();
  m_wizDlg->ui->tableWidget->setColumnCount(0);
  m_wizDlg->ui->tableWidget->setRowCount(0);
  m_wizDlg->ui->tableWidget->verticalScrollBar()->setValue(0);
  m_wizDlg->ui->tableWidget->horizontalScrollBar()->setValue(0);

  wizard()->setButtonText(QWizard::CustomButton1, i18n("Select File"));
  wizard()->button(QWizard::CustomButton1)->setToolTip(i18n("A profile must be selected before selecting a file."));
  QList<QWizard::WizardButton> layout;
  layout << QWizard::Stretch <<
            QWizard::CustomButton1 <<
            QWizard::CancelButton;
  wizard()->setButtonLayout(layout);


  m_wizDlg->m_importError = false;
  ui->combobox_source->lineEdit()->setClearButtonEnabled(true);

  connect(ui->combobox_source, SIGNAL(currentIndexChanged(int)), this, SLOT(slotComboSourceIndexChanged(int)));
  connect(ui->buttonAdd, SIGNAL(clicked()), this, SLOT(slotAddProfile()));
  connect(ui->buttonRemove, SIGNAL(clicked()), this, SLOT(slotRemoveProfile()));
  connect(ui->buttonRename, SIGNAL(clicked()), this, SLOT(slotRenameProfile()));
  connect(ui->radioButton_bank, SIGNAL(toggled(bool)), this, SLOT(slotBankRadioToggled(bool)));
  connect(ui->radioButton_invest, SIGNAL(toggled(bool)), this, SLOT(slotInvestRadioToggled(bool)));
  if (m_wizDlg->m_initialHeight == -1 || m_wizDlg->m_initialWidth == -1) {
    m_wizDlg->m_initialHeight = m_wizDlg->geometry().height();
    m_wizDlg->m_initialWidth = m_wizDlg->geometry().width();
  } else {
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
}

bool IntroPage::validatePage()
{
  return true;
}

int IntroPage::nextId() const
{
  return CSVWizard::PageSeparator;
}

SeparatorPage::SeparatorPage(QDialog *parent) :
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
  ui->comboBox_fieldDelimiter->setCurrentIndex(m_wizDlg->m_fieldDelimiterIndex);
  ui->comboBox_textDelimiter->setCurrentIndex(m_wizDlg->m_textDelimiterIndex);
  connect(ui->comboBox_fieldDelimiter, SIGNAL(currentIndexChanged(int)), this, SLOT(fieldDelimiterChanged(int)));
  connect(ui->comboBox_textDelimiter, SIGNAL(currentIndexChanged(int)), this, SLOT(textDelimiterChanged(int)));
  emit ui->comboBox_fieldDelimiter->currentIndexChanged(m_wizDlg->m_fieldDelimiterIndex);
  emit ui->comboBox_textDelimiter->currentIndexChanged(m_wizDlg->m_textDelimiterIndex);

  QList<QWizard::WizardButton> layout;
  layout << QWizard::Stretch << QWizard::BackButton << QWizard::NextButton << QWizard::CancelButton;
  wizard()->setButtonLayout(layout);
}

void SeparatorPage::textDelimiterChanged(const int index)
{
  if (index < 0)
    ui->comboBox_textDelimiter->setCurrentIndex(0); // for now there is no better idea how to detect textDelimiter

  m_wizDlg->m_textDelimiterIndex = index;
  m_wizDlg->m_parse->setTextDelimiterIndex(index);
  m_wizDlg->m_parse->setTextDelimiterCharacter(index);
  m_wizDlg->m_textDelimiterCharacter = m_wizDlg->m_parse->textDelimiterCharacter(index);
}

void SeparatorPage::fieldDelimiterChanged(const int index)
{
  if (index == -1 && !m_wizDlg->m_autodetect.value(CSVWizard::AutoFieldDelimiter))
    return;
  m_wizDlg->m_fieldDelimiterIndex = index;
  m_wizDlg->m_maxColumnCount = m_wizDlg->getMaxColumnCount(m_wizDlg->m_lineList, m_wizDlg->m_fieldDelimiterIndex); // get column count, we get with this fieldDelimiter
  m_wizDlg->m_endColumn = m_wizDlg->m_maxColumnCount;
  m_wizDlg->m_parse->setFieldDelimiterIndex(m_wizDlg->m_fieldDelimiterIndex);
  m_wizDlg->m_parse->setFieldDelimiterCharacter(m_wizDlg->m_fieldDelimiterIndex);
  m_wizDlg->m_fieldDelimiterCharacter = m_wizDlg->m_parse->fieldDelimiterCharacter(m_wizDlg->m_fieldDelimiterIndex);
  if (index == -1) {
    ui->comboBox_fieldDelimiter->blockSignals(true);
    ui->comboBox_fieldDelimiter->setCurrentIndex(m_wizDlg->m_fieldDelimiterIndex);
    ui->comboBox_fieldDelimiter->blockSignals(false);
  }
  m_wizDlg->displayLines(m_wizDlg->m_lineList, m_wizDlg->m_parse);  // refresh tableWidget with new fieldDelimiter set
  m_wizDlg->updateWindowSize();
  emit completeChanged();
}

bool SeparatorPage::isComplete() const
{
  if (ui->comboBox_fieldDelimiter->currentIndex() != -1) {
    if (m_wizDlg->m_profileType == CSVWizard::ProfileBank && m_wizDlg->m_endColumn > 2)
      return true;
    else if (m_wizDlg->m_profileType == CSVWizard::ProfileInvest && m_wizDlg->m_endColumn > 3)
      return true;
  }
  return false;
}

bool SeparatorPage::validatePage()
{
  return true;
}

void SeparatorPage::cleanupPage()
{
  //  On completion with error force use of 'Back' button.
  //  ...to allow resetting of 'Skip setup'
  disconnect(ui->comboBox_fieldDelimiter, SIGNAL(currentIndexChanged(int)), this, SLOT(fieldDelimiterChanged(int)));
  disconnect(ui->comboBox_textDelimiter, SIGNAL(currentIndexChanged(int)), this, SLOT(textDelimiterChanged(int)));
  m_wizDlg->m_pageIntro->initializePage();  //  Need to show button(QWizard::CustomButton1) not 'NextButton'
}

RowsPage::RowsPage(QDialog *parent) :
    CSVWizardPage(parent),
    ui(new Ui::RowsPage)
{
  ui->setupUi(this);
  m_pageLayout = new QVBoxLayout;
  ui->horizontalLayout->insertLayout(0, m_pageLayout);
}

RowsPage::~RowsPage()
{
  delete ui;
}

void RowsPage::initializePage()
{
  disconnect(ui->spinBox_skip, SIGNAL(valueChanged(int)), this, SLOT(startRowChanged(int)));
  disconnect(ui->spinBox_skipToLast, SIGNAL(valueChanged(int)), this, SLOT(endRowChanged(int)));

  ui->spinBox_skip->setMaximum(m_wizDlg->m_fileEndLine);
  ui->spinBox_skipToLast->setMaximum(m_wizDlg->m_fileEndLine);
  ui->spinBox_skip->setValue(m_wizDlg->m_startLine);
  ui->spinBox_skipToLast->setValue(m_wizDlg->m_endLine);

  m_wizDlg->markUnwantedRows();
  m_wizDlg->m_vScrollBar->setValue(m_wizDlg->m_startLine - 1);

  connect(ui->spinBox_skip, SIGNAL(valueChanged(int)), this, SLOT(startRowChanged(int)));
  connect(ui->spinBox_skipToLast, SIGNAL(valueChanged(int)), this, SLOT(endRowChanged(int)));

  QList<QWizard::WizardButton> layout;
  layout << QWizard::Stretch <<
            QWizard::BackButton <<
            QWizard::NextButton <<
            QWizard::CancelButton;
  wizard()->setButtonLayout(layout);
}

void RowsPage::startRowChanged(int val)
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

void RowsPage::endRowChanged(int val)
{
  if (val > m_wizDlg->m_fileEndLine) {
    ui->spinBox_skipToLast->setValue(m_wizDlg->m_fileEndLine);
    return;
  }
  if (val < m_wizDlg->m_startLine) {
    ui->spinBox_skipToLast->setValue(m_wizDlg->m_startLine);
    return;
  }
  m_wizDlg->m_trailerLines = m_wizDlg->m_fileEndLine - val;
  m_wizDlg->m_endLine = val;
  if (!m_wizDlg->m_inFileName.isEmpty()) {
    m_wizDlg->markUnwantedRows();
  }
}

void RowsPage::cleanupPage()
{
  disconnect(ui->spinBox_skip, SIGNAL(valueChanged(int)), this, SLOT(startRowChanged(int)));
  disconnect(ui->spinBox_skipToLast, SIGNAL(valueChanged(int)), this, SLOT(endRowChanged(int)));
  m_wizDlg->clearBackground();
}

int RowsPage::nextId() const
{
  int ret;
  if (m_wizDlg->m_profileType == CSVWizard::ProfileBank) {
    ret = CSVWizard::PageBanking;
  } else {
    ret = CSVWizard::PageInvestment;
  }
  return ret;
}

FormatsPage::FormatsPage(QDialog *parent) :
    CSVWizardPage(parent),
    ui(new Ui::FormatsPage)
{
  ui->setupUi(this);
  m_pageLayout = new QVBoxLayout;
  ui->horizontalLayout->insertLayout(0, m_pageLayout);
}

FormatsPage::~FormatsPage()
{
  delete ui;
}

void FormatsPage::initializePage()
{
  disconnect(ui->comboBox_dateFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(dateFormatChanged(int)));
  disconnect(ui->comboBox_decimalSymbol, SIGNAL(currentIndexChanged(int)), this, SLOT(decimalSymbolChanged(int)));
  m_isDecimalSymbolOK = false;
  m_isDateFormatOK = false;
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
  wizard()->button(QWizard::CustomButton2)->setEnabled(false);
  wizard()->button(QWizard::CustomButton3)->setEnabled(false);
  wizard()->button(QWizard::FinishButton)->setEnabled(false);

  ui->comboBox_thousandsDelimiter->setEnabled(false);

  ui->comboBox_dateFormat->setCurrentIndex(m_wizDlg->m_dateFormatIndex); // put before connect to not emit update signal
  connect(ui->comboBox_dateFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(dateFormatChanged(int)));
  emit ui->comboBox_dateFormat->currentIndexChanged(m_wizDlg->m_dateFormatIndex); // emit update signal manually regardless of change to combobox

  ui->comboBox_decimalSymbol->setCurrentIndex(m_wizDlg->m_decimalSymbolIndex);    // put before connect to not emit update signal
  connect(ui->comboBox_decimalSymbol, SIGNAL(currentIndexChanged(int)), this, SLOT(decimalSymbolChanged(int)));
  emit ui->comboBox_decimalSymbol->currentIndexChanged(m_wizDlg->m_decimalSymbolIndex); // emit update signal manually regardless of change to combobox

  if (m_wizDlg->m_skipSetup &&
      wizard()->button(QWizard::CustomButton2)->isEnabled())
    slotImportClicked();
}

void FormatsPage::decimalSymbolChanged(int index)
{
  switch (index) {
  case -1:
    if (!m_wizDlg->m_autodetect.value(CSVWizard::AutoDecimalSymbol))
        return;
  case 2:
    m_wizDlg->m_decimalSymbolIndex = 2;
    m_wizDlg->m_decimalSymbol.clear();
    break;
  default:
    m_wizDlg->m_parse->setDecimalSymbol(index);
    m_wizDlg->m_parse->setDecimalSymbolIndex(index);
    m_wizDlg->m_parse->setThousandsSeparator(index);
    m_wizDlg->m_parse->setThousandsSeparatorIndex(index);

    m_wizDlg->m_decimalSymbol = m_wizDlg->m_parse->decimalSymbol(index);
    m_wizDlg->m_decimalSymbolIndex = m_wizDlg->m_parse->decimalSymbolIndex();
  }
  ui->comboBox_thousandsDelimiter->setCurrentIndex(m_wizDlg->m_decimalSymbolIndex);

  bool isOk = true;
  QList<int> columnList;
  if (m_wizDlg->m_profileType == CSVWizard::ProfileBank) {
    if (m_wizDlg->m_pageBanking->m_colTypeNum.value(BankingPage::ColumnAmount) >= 0) {
      columnList << m_wizDlg->m_pageBanking->m_colTypeNum.value(BankingPage::ColumnAmount);
    } else {
      columnList << m_wizDlg->m_pageBanking->m_colTypeNum.value(BankingPage::ColumnDebit);
      columnList << m_wizDlg->m_pageBanking->m_colTypeNum.value(BankingPage::ColumnCredit);
    }
  } else if (m_wizDlg->m_profileType == CSVWizard::ProfileInvest) {
    columnList << m_wizDlg->m_pageInvestment->m_colTypeNum.value(InvestmentPage::ColumnAmount);
    columnList << m_wizDlg->m_pageInvestment->m_colTypeNum.value(InvestmentPage::ColumnPrice);
    columnList << m_wizDlg->m_pageInvestment->m_colTypeNum.value(InvestmentPage::ColumnQuantity);
    if (m_wizDlg->m_pageInvestment->m_colTypeNum.value(InvestmentPage::ColumnFee) != -1)
      columnList << m_wizDlg->m_pageInvestment->m_colTypeNum.value(InvestmentPage::ColumnFee);
  }

  for (QList<int>::const_iterator col = columnList.constBegin(); col < columnList.constEnd(); ++col) {
    int detectedSymbol = m_wizDlg->m_decimalSymbolIndex;
    if (!m_wizDlg->detectDecimalSymbol(*col, detectedSymbol)) {
      isOk = false;
      KMessageBox::sorry(this, i18n("<center>Autodetect couldn't detect your decimal symbol in column %1.</center>"
                                    "<center>Try manual selection to see problematic cells and correct your data.</center>", *col + 1), i18n("CSV import"));
      ui->comboBox_decimalSymbol->blockSignals(true);
      ui->comboBox_decimalSymbol->setCurrentIndex(-1);
      ui->comboBox_decimalSymbol->blockSignals(false);
      ui->comboBox_thousandsDelimiter->setCurrentIndex(-1);
      break;
    }
    m_wizDlg->m_decimalSymbolIndexMap.insert(*col, detectedSymbol);
    m_wizDlg->m_parse->setDecimalSymbol(detectedSymbol);
    m_wizDlg->m_parse->setThousandsSeparator(detectedSymbol); // separator list is in reverse so it's ok

    isOk &= validateDecimalSymbol(*col);
  }

  if (index == -1 && isOk) {  // if detection went well and decimal symbol was unspeciffied then we'll be specifying it
    int prevDecimalSymbol = columnList.value(0);
    bool allSymbolsEqual = true;
    for (QList<int>::const_iterator col = columnList.constBegin(); col < columnList.constEnd(); ++col) {
      if (m_wizDlg->m_decimalSymbolIndexMap.value(*col) != prevDecimalSymbol)
        allSymbolsEqual = false;
    }
    ui->comboBox_decimalSymbol->blockSignals(true);
    if (allSymbolsEqual) {   // if symbol in all columns is equal then set it...
      ui->comboBox_decimalSymbol->setCurrentIndex(prevDecimalSymbol);
      ui->comboBox_thousandsDelimiter->setCurrentIndex(prevDecimalSymbol);
    } else {  // else set to auto
      ui->comboBox_decimalSymbol->setCurrentIndex(2);
      ui->comboBox_thousandsDelimiter->setCurrentIndex(2);
    }
    ui->comboBox_decimalSymbol->blockSignals(false);
  }

  m_isDecimalSymbolOK = isOk;
  emit completeChanged();
}

bool FormatsPage::validateDecimalSymbol(int col)
{
  m_wizDlg->clearColumnsBackground(col);

  bool isOK = true;
  for (int row = m_wizDlg->m_startLine - 1; row < m_wizDlg->m_endLine; ++row) {
      QTableWidgetItem* item = m_wizDlg->ui->tableWidget->item(row, col);
      QString rawNumber = item->text();
      m_wizDlg->m_parse->possiblyReplaceSymbol(rawNumber);

      if (!m_wizDlg->m_parse->invalidConversion() ||
          rawNumber.isEmpty()) {                   // empty strings are welcome
        item->setBackground(m_wizDlg->m_colorBrush);
        item->setForeground(m_wizDlg->m_colorBrushText);
      } else {
        isOK = false;
        m_wizDlg->ui->tableWidget->scrollToItem(item, QAbstractItemView::EnsureVisible);
        item->setBackground(m_wizDlg->m_errorBrush);
        item->setForeground(m_wizDlg->m_errorBrushText);
      }
  }
  return isOK;
}

void FormatsPage::dateFormatChanged(int index)
{
  if (index < 0)
    return;
  else {
    m_wizDlg->m_dateFormatIndex = index;
    m_wizDlg->m_date = m_wizDlg->m_dateFormats[m_wizDlg->m_dateFormatIndex];
  }

  int col;
  if (m_wizDlg->m_profileType == CSVWizard::ProfileBank)
    col = m_wizDlg->m_pageBanking->m_colTypeNum.value(BankingPage::ColumnDate);
  else
    col = m_wizDlg->m_pageInvestment->m_colTypeNum.value(InvestmentPage::ColumnDate);

  m_wizDlg->m_convertDate->setDateFormatIndex(index);
  m_isDateFormatOK = validateDateFormat(col);
  if (!m_isDateFormatOK) {
    KMessageBox::sorry(this, i18n("<center>There are invalid date formats in column '%1'.</center>"
                                  "<center>Please check your selections.</center>"
                                  , col + 1), i18n("CSV import"));
  }
  emit completeChanged();
}

bool FormatsPage::validateDateFormat(int col)
{
  m_wizDlg->clearColumnsBackground(col);

  bool isOK = true;
  for (int row = m_wizDlg->m_startLine - 1; row < m_wizDlg->m_endLine; ++row) {
      QTableWidgetItem* item = m_wizDlg->ui->tableWidget->item(row, col);

      QDate dat = m_wizDlg->m_convertDate->convertDate(item->text());

      if (dat == QDate()) {
        isOK = false;
        m_wizDlg->ui->tableWidget->scrollToItem(item, QAbstractItemView::EnsureVisible);
        item->setBackground(m_wizDlg->m_errorBrush);
        item->setForeground(m_wizDlg->m_errorBrushText);
      } else {
        item->setBackground(m_wizDlg->m_colorBrush);
        item->setForeground(m_wizDlg->m_colorBrushText);
      }
  }
  return isOK;
}

void FormatsPage::slotImportClicked()
{
  bool isOK = true;
  if (m_wizDlg->m_profileType == CSVWizard::ProfileBank)
    isOK = m_wizDlg->m_pageBanking->createStatement(m_wizDlg->st);
  else if (m_wizDlg->m_profileType == CSVWizard::ProfileInvest)
    isOK = m_wizDlg->m_pageInvestment->createStatement(m_wizDlg->st);

  if (!isOK) {
    m_wizDlg->st = MyMoneyStatement(); // statement is invalid so erase it
    return;
  }


  m_wizDlg->hide(); //hide wizard so it will not cover accountselector
  emit m_wizDlg->statementReady(m_wizDlg->st);
  m_wizDlg->slotClose(); //close hidden window as it isn't needed anymore
}

void FormatsPage::slotSaveAsQIFClicked()
{
  bool isOK = true;
  if (m_wizDlg->m_profileType == CSVWizard::ProfileBank)
    isOK = m_wizDlg->m_pageBanking->createStatement(m_wizDlg->st);
  else if (m_wizDlg->m_profileType == CSVWizard::ProfileInvest)
    isOK = m_wizDlg->m_pageInvestment->createStatement(m_wizDlg->st);

  if (!isOK || m_wizDlg->st.m_listTransactions.isEmpty())
    return;

  QString outFileName = m_wizDlg->m_inFileName;
  outFileName.truncate(m_wizDlg->m_inFileName.lastIndexOf('.'));
  outFileName += ".qif";
  outFileName = QFileDialog::getSaveFileName(this, i18n("Save QIF"), outFileName, i18n("QIF Files (*.qif)"));
  if (outFileName.isEmpty())
    return;
  QFile oFile(outFileName);
  oFile.open(QIODevice::WriteOnly);
  if (m_wizDlg->m_profileType == CSVWizard::ProfileBank)
    m_wizDlg->m_pageBanking->makeQIF(m_wizDlg->st, oFile);
  else if (m_wizDlg->m_profileType == CSVWizard::ProfileInvest)
    m_wizDlg->m_pageInvestment->makeQIF(m_wizDlg->st, oFile);
  oFile.close();
}

bool FormatsPage::isComplete() const
{
  const bool enable = m_isDecimalSymbolOK && m_isDateFormatOK;
  wizard()->button(QWizard::CustomButton2)->setEnabled(enable);
  wizard()->button(QWizard::CustomButton3)->setEnabled(enable);
  return enable;
}

void FormatsPage::cleanupPage()
{
  disconnect(ui->comboBox_dateFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(dateFormatChanged(int)));
  disconnect(ui->comboBox_decimalSymbol, SIGNAL(currentIndexChanged(int)), this, SLOT(decimalSymbolChanged(int)));

  QList<int> columnList;
  if (m_wizDlg->m_profileType == CSVWizard::ProfileBank) {
    columnList << m_wizDlg->m_pageBanking->m_colTypeNum.value(BankingPage::ColumnDate);
    if (m_wizDlg->m_pageBanking->m_colTypeNum.value(BankingPage::ColumnAmount) >= 0)
      columnList << m_wizDlg->m_pageBanking->m_colTypeNum.value(BankingPage::ColumnAmount);
    else
      columnList << m_wizDlg->m_pageBanking->m_colTypeNum.value(BankingPage::ColumnDebit) <<
                    m_wizDlg->m_pageBanking->m_colTypeNum.value(BankingPage::ColumnCredit);
  } else if (m_wizDlg->m_profileType == CSVWizard::ProfileInvest) {
    columnList << m_wizDlg->m_pageInvestment->m_colTypeNum.value(InvestmentPage::ColumnAmount) <<
                  m_wizDlg->m_pageInvestment->m_colTypeNum.value(InvestmentPage::ColumnPrice) <<
                  m_wizDlg->m_pageInvestment->m_colTypeNum.value(InvestmentPage::ColumnQuantity) <<
                  m_wizDlg->m_pageInvestment->m_colTypeNum.value(InvestmentPage::ColumnDate);
    if (m_wizDlg->m_pageInvestment->m_colTypeNum.value(InvestmentPage::ColumnFee) != -1)
      columnList << m_wizDlg->m_pageInvestment->m_colTypeNum.value(InvestmentPage::ColumnFee);
  }
  m_wizDlg->clearColumnsBackground(columnList);
  m_wizDlg->st = MyMoneyStatement();  // any change on investment/banking page invalidates created statement

  QList<QWizard::WizardButton> layout;
  layout << QWizard::Stretch <<
            QWizard::BackButton <<
            QWizard::NextButton <<
            QWizard::CancelButton;
  wizard()->setButtonLayout(layout);
}

void CSVWizard::closeEvent(QCloseEvent *event)
{
  this->m_plugin->m_action->setEnabled(true); // reenable File->Import->CSV
  event->accept();
}

bool CSVWizard::eventFilter(QObject *object, QEvent *event)
{
  // prevent the QWizard part of CSVWizard window from closing on Escape key press
  if (object == this->m_wizard) {
    if (event->type() == QEvent::KeyPress) {
      QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
      if (keyEvent->key() == Qt::Key_Escape) {
        close();
        return true;
      }
    }
  }
  return false;
}
