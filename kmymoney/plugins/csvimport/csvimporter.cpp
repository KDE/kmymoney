/***************************************************************************
                            csvimporter.cpp
                             -------------------
    begin                : Sun May 21 2017
    copyright            : (C) 2010 by Allan Anderson
    email                : agander93@gmail.com
    copyright            : (C) 2016-2017 by Łukasz Wojniłowicz
    email                : lukasz.wojnilowicz@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "csvimporter.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QFile>
#include <QFileInfo>
#include <QTextCodec>
#include <QTextStream>
#include <QMessageBox>
#include <QDir>
#include <QFileDialog>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KActionCollection>
#include <KMessageBox>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneystatementreader.h"
#include "mymoneyfile.h"

const QMap<profileTypeE, QString> CSVImporter::m_profileConfPrefix = QMap<profileTypeE, QString>{
  {ProfileBank, QStringLiteral("Bank")},
  {ProfileInvest, QStringLiteral("Invest")},
  {ProfileCurrencyPrices, QStringLiteral("CPrices")},
  {ProfileStockPrices, QStringLiteral("SPrices")}
};

const QMap<columnTypeE, QString> CSVImporter::m_colTypeConfName = QMap<columnTypeE, QString>{
  {ColumnDate, QStringLiteral("DateCol")},
  {ColumnMemo, QStringLiteral("MemoCol")},
  {ColumnNumber, QStringLiteral("NumberCol")},
  {ColumnPayee, QStringLiteral("PayeeCol")},
  {ColumnAmount, QStringLiteral("AmountCol")},
  {ColumnCredit, QStringLiteral("CreditCol")},
  {ColumnDebit, QStringLiteral("DebitCol")},
  {ColumnCategory, QStringLiteral("CategoryCol")},
  {ColumnType, QStringLiteral("TypeCol")},
  {ColumnPrice, QStringLiteral("PriceCol")},
  {ColumnQuantity, QStringLiteral("QuantityCol")},
  {ColumnFee, QStringLiteral("FeeCol")},
  {ColumnSymbol, QStringLiteral("SymbolCol")},
  {ColumnName, QStringLiteral("NameCol")},
};

const QMap<miscSettingsE, QString> CSVImporter::m_miscSettingsConfName = QMap<miscSettingsE, QString>{
  {ConfDirectory, QStringLiteral("Directory")},
  {ConfEncoding, QStringLiteral("Encoding")},
  {ConfDateFormat, QStringLiteral("DateFormat")},
  {ConfFieldDelimiter, QStringLiteral("FieldDelimiter")},
  {ConfTextDeimiter, QStringLiteral("TextDelimiter")},
  {ConfDecimalSymbol, QStringLiteral("DecimalSymbol")},
  {ConfStartLine, QStringLiteral("StartLine")},
  {ConfTrailerLines, QStringLiteral("TrailerLines")},
  {ConfOppositeSigns, QStringLiteral("OppositeSigns")},
  {ConfFeeIsPercentage, QStringLiteral("FeeIsPercentage")},
  {ConfFeeRate, QStringLiteral("FeeRate")},
  {ConfMinFee, QStringLiteral("MinFee")},
  {ConfSecurityName, QStringLiteral("SecurityName")},
  {ConfSecuritySymbol, QStringLiteral("SecuritySymbol")},
  {ConfCurrencySymbol, QStringLiteral("CurrencySymbol")},
  {ConfPriceFraction, QStringLiteral("PriceFraction")},
  {ConfDontAsk, QStringLiteral("DontAsk")},
  {ConfHeight, QStringLiteral("Height")},
  {ConfWidth, QStringLiteral("Width")}
};

const QMap<MyMoneyStatement::Transaction::EAction, QString> CSVImporter::m_transactionConfName = QMap<MyMoneyStatement::Transaction::EAction, QString>{
  {MyMoneyStatement::Transaction::eaBuy, QStringLiteral("BuyParam")},
  {MyMoneyStatement::Transaction::eaSell, QStringLiteral("SellParam")},
  {MyMoneyStatement::Transaction::eaReinvestDividend, QStringLiteral("ReinvdivParam")},
  {MyMoneyStatement::Transaction::eaCashDividend, QStringLiteral("DivXParam")},
  {MyMoneyStatement::Transaction::eaInterest, QStringLiteral("IntIncParam")},
  {MyMoneyStatement::Transaction::eaShrsin, QStringLiteral("ShrsinParam")},
  {MyMoneyStatement::Transaction::eaShrsout, QStringLiteral("ShrsoutParam")}
};

const QString CSVImporter::m_confProfileNames = QStringLiteral("ProfileNames");
const QString CSVImporter::m_confPriorName = QStringLiteral("Prior");
const QString CSVImporter::m_confMiscName = QStringLiteral("Misc");

CSVImporter::CSVImporter()
{
  m_convertDate = new ConvertDate;
  m_file = new CSVFile;

  m_priceFractions << MyMoneyMoney(0.01) << MyMoneyMoney(0.1) << MyMoneyMoney::ONE << MyMoneyMoney(10) << MyMoneyMoney(100);

  validateConfigFile();
  readMiscSettings();
}
CSVImporter::~CSVImporter()
{
  delete m_convertDate;
  delete m_file;
}

MyMoneyStatement CSVImporter::unattendedPricesImport(const QString &filename, PricesProfile *profile)
{
  MyMoneyStatement st;
  m_profile = profile;

  if (m_file->getInFileName(filename)) {
    m_file->readFile(m_profile);
    m_file->setupParser(m_profile);

    if (!createStatement(st))
      st = MyMoneyStatement();
  }

  return st;
}

KSharedConfigPtr CSVImporter::configFile()
{
  return KSharedConfig::openConfig(QDir(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation))
                                   .filePath(QStringLiteral("csvimporterrc")));
}

void CSVImporter::profileFactory(const profileTypeE type, const QString &name)
{
  if (!m_profile)
    delete m_profile;

  switch (type) {
    default:
    case ProfileInvest:
      m_profile = new InvestmentProfile;
      break;
    case ProfileBank:
      m_profile = new BankingProfile;
      break;
    case ProfileCurrencyPrices:
    case ProfileStockPrices:
      m_profile = new PricesProfile(type);
      break;
  }
  m_profile->m_profileName = name;
}

void CSVImporter::readMiscSettings() {
  KConfigGroup miscGroup(configFile(), m_confMiscName);
  m_autodetect.clear();
  m_autodetect.insert(AutoFieldDelimiter, miscGroup.readEntry(QStringLiteral("AutoFieldDelimiter"), true));
  m_autodetect.insert(AutoDecimalSymbol, miscGroup.readEntry(QStringLiteral("AutoDecimalSymbol"), true));
  m_autodetect.insert(AutoDateFormat, miscGroup.readEntry(QStringLiteral("AutoDateFormat"), true));
  m_autodetect.insert(AutoAccountInvest, miscGroup.readEntry(QStringLiteral("AutoAccountInvest"), true));
  m_autodetect.insert(AutoAccountBank, miscGroup.readEntry(QStringLiteral("AutoAccountBank"), true));
}

void CSVImporter::validateConfigFile()
{
  const KSharedConfigPtr config = configFile();
  KConfigGroup profileNamesGroup(config, m_confProfileNames);
  if (!profileNamesGroup.exists()) {
    profileNamesGroup.writeEntry(m_profileConfPrefix.value(ProfileBank), QStringList());
    profileNamesGroup.writeEntry(m_profileConfPrefix.value(ProfileInvest), QStringList());
    profileNamesGroup.writeEntry(m_profileConfPrefix.value(ProfileCurrencyPrices), QStringList());
    profileNamesGroup.writeEntry(m_profileConfPrefix.value(ProfileStockPrices), QStringList());
    profileNamesGroup.writeEntry(m_confPriorName + m_profileConfPrefix.value(ProfileBank), int());
    profileNamesGroup.writeEntry(m_confPriorName + m_profileConfPrefix.value(ProfileInvest), int());
    profileNamesGroup.writeEntry(m_confPriorName + m_profileConfPrefix.value(ProfileCurrencyPrices), int());
    profileNamesGroup.writeEntry(m_confPriorName + m_profileConfPrefix.value(ProfileStockPrices), int());
    profileNamesGroup.sync();
  }

  KConfigGroup miscGroup(config, m_confMiscName);
  if (!miscGroup.exists()) {
    miscGroup.writeEntry(CSVImporter::m_miscSettingsConfName.value(ConfHeight), "400");
    miscGroup.writeEntry(CSVImporter::m_miscSettingsConfName.value(ConfWidth), "800");
    miscGroup.sync();
  }

  QList<int> confVer = miscGroup.readEntry("KMMVer", QList<int> {0, 0, 0});
  if (updateConfigFile(confVer)) // write kmmVer only if there were no errors
    miscGroup.writeEntry("KMMVer", confVer);
}

bool CSVImporter::updateConfigFile(QList<int> &confVer)
{
  bool ret = true;

  QList<int> kmmVer = QList<int> {5, 0, 0};
  int kmmVersion = kmmVer.at(0) * 100 + kmmVer.at(1) * 10 + kmmVer.at(2);
  int confVersion = confVer.at(0) * 100 + confVer.at(1) * 10 + confVer.at(2);
  if (confVersion > kmmVersion) {
    KMessageBox::information(0,
                             i18n("Version of your CSV config file is %1.%2.%3 and is newer than supported version %4.%5.%6. Expect troubles.",
                                  confVer.at(0), confVer.at(1), confVer.at(2),
                                  kmmVer.at(0), kmmVer.at(1), kmmVer.at(2)));
    ret = false;
    return ret;
  } else if (confVersion == kmmVersion)
    return true;
  confVer = kmmVer;

  const KSharedConfigPtr config = configFile();
  QString configFilePath = config.constData()->name();
  QFile::copy(configFilePath, configFilePath + QLatin1String(".bak"));

  KConfigGroup profileNamesGroup(config, m_confProfileNames);
  QStringList bankProfiles = profileNamesGroup.readEntry(m_profileConfPrefix.value(ProfileBank), QStringList());
  QStringList investProfiles = profileNamesGroup.readEntry(m_profileConfPrefix.value(ProfileInvest), QStringList());
  QStringList invalidBankProfiles = profileNamesGroup.readEntry(QLatin1String("Invalid") + m_profileConfPrefix.value(ProfileBank), QStringList());     // get profiles that was marked invalid during last update
  QStringList invalidInvestProfiles = profileNamesGroup.readEntry(QLatin1String("Invalid") + m_profileConfPrefix.value(ProfileInvest), QStringList());
  QString bankPrefix = m_profileConfPrefix.value(ProfileBank) + QLatin1Char('-');
  QString investPrefix = m_profileConfPrefix.value(ProfileInvest) + QLatin1Char('-');

  // for kmm < 5.0.0 change 'BankNames' to 'ProfileNames' and remove 'MainWindow' group
  if (confVersion < 500 && bankProfiles.isEmpty()) {
    KConfigGroup oldProfileNamesGroup(config, "BankProfiles");
    bankProfiles = oldProfileNamesGroup.readEntry("BankNames", QStringList()); // profile names are under 'BankNames' entry for kmm < 5.0.0
    bankPrefix = QLatin1String("Profiles-");   // needed to remove non-existent profiles in first run
    oldProfileNamesGroup.deleteGroup();
    KConfigGroup oldMainWindowGroup(config, "MainWindow");
    oldMainWindowGroup.deleteGroup();
    KConfigGroup oldSecuritiesGroup(config, "Securities");
    oldSecuritiesGroup.deleteGroup();
  }

  bool firstTry = false;
  if (invalidBankProfiles.isEmpty() && invalidInvestProfiles.isEmpty())  // if there is no invalid profiles then this might be first update try
    firstTry = true;

  int invalidProfileResponse = QMessageBox::No;

  for (auto profileName = bankProfiles.begin(); profileName != bankProfiles.end();) {
    KConfigGroup bankProfile(config, bankPrefix + *profileName);
    if (!bankProfile.exists() && !invalidBankProfiles.contains(*profileName)) { // if there is reference to profile but no profile then remove this reference
      profileName = bankProfiles.erase(profileName);
      continue;
    }

    // for kmm < 5.0.0 remove 'FileType' and 'ProfileName' and assign them to either "Bank=" or "Invest="
    if (confVersion < 500) {
      QString lastUsedDirectory;
      KConfigGroup oldBankProfile(config, QLatin1String("Profiles-") + *profileName);  // if half of configuration is updated and the other one untouched this is needed
      QString oldProfileType = oldBankProfile.readEntry("FileType", QString());
      KConfigGroup newProfile;
      if (oldProfileType == QLatin1String("Invest")) {
        oldBankProfile.deleteEntry("BrokerageParam");
        oldBankProfile.writeEntry(m_colTypeConfName.value(ColumnType), oldBankProfile.readEntry("PayeeCol"));
        oldBankProfile.deleteEntry("PayeeCol");
        oldBankProfile.deleteEntry("Filter");
        oldBankProfile.deleteEntry("SecurityName");

        lastUsedDirectory = oldBankProfile.readEntry("InvDirectory");
        newProfile = KConfigGroup(config, m_profileConfPrefix.value(ProfileInvest) + QLatin1Char('-') + *profileName);
        investProfiles.append(*profileName);
        profileName = bankProfiles.erase(profileName);
      } else if (oldProfileType == QLatin1String("Banking")) {
        lastUsedDirectory = oldBankProfile.readEntry("CsvDirectory");
        newProfile = KConfigGroup(config, m_profileConfPrefix.value(ProfileBank) + QLatin1Char('-') + *profileName);
        ++profileName;
      } else {
        if (invalidProfileResponse != QMessageBox::YesToAll && invalidProfileResponse != QMessageBox::NoToAll) {
          if (!firstTry &&
              !invalidBankProfiles.contains(*profileName)) { // if it isn't first update run and profile isn't on the list of invalid ones then don't bother
            ++profileName;
            continue;
          }
        invalidProfileResponse = QMessageBox::warning(0, i18n("CSV import"),
                                       i18n("<center>During update of <b>%1</b><br>"
                                            "the profile type for <b>%2</b> could not be recognized.<br>"
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
          break;
        case QMessageBox::NoToAll:
        case QMessageBox::No:
          if (!invalidBankProfiles.contains(*profileName))  // on user request: don't delete profile but keep eye on it
            invalidBankProfiles.append(*profileName);
          ret = false;
          ++profileName;
          break;
        }
        continue;
      }
      oldBankProfile.deleteEntry("FileType");
      oldBankProfile.deleteEntry("ProfileName");
      oldBankProfile.deleteEntry("DebitFlag");
      oldBankProfile.deleteEntry("InvDirectory");
      oldBankProfile.deleteEntry("CsvDirectory");
      oldBankProfile.sync();
      oldBankProfile.copyTo(&newProfile);
      oldBankProfile.deleteGroup();
      newProfile.writeEntry(m_miscSettingsConfName.value(ConfDirectory), lastUsedDirectory);
      newProfile.writeEntry(m_miscSettingsConfName.value(ConfEncoding), "106" /*UTF-8*/ ); // in 4.8 encoding wasn't supported well so set it to utf8 by default
      newProfile.sync();
    }
  }

  for (auto profileName = investProfiles.begin(); profileName != investProfiles.end();) {
    KConfigGroup investProfile(config, investPrefix + *profileName);
    if (!investProfile.exists() && !invalidInvestProfiles.contains(*profileName)) { // if there is reference to profile but no profile then remove this reference
      profileName = investProfiles.erase(profileName);
      continue;
    }
    ++profileName;
  }

  profileNamesGroup.writeEntry(m_profileConfPrefix.value(ProfileBank), bankProfiles); // update profile names as some of them might have been changed
  profileNamesGroup.writeEntry(m_profileConfPrefix.value(ProfileInvest), investProfiles);

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

bool CSVImporter::profilesAction(const profileTypeE type, const profilesActionE action, const QString &name, const QString &newname)
{
  bool ret = false;
  const KSharedConfigPtr config = configFile();
  KConfigGroup profileNamesGroup(config, m_confProfileNames);
  QString profileTypeStr = m_profileConfPrefix.value(type);
  QStringList profiles = profileNamesGroup.readEntry(profileTypeStr, QStringList());

  KConfigGroup profileName(config, profileTypeStr + QLatin1Char('-') + name);
  switch (action) {
    case ProfilesUpdateLastUsed:
      profileNamesGroup.writeEntry(m_confPriorName + profileTypeStr, profiles.indexOf(name));
      break;
    case ProfilesAdd:
      if (!profiles.contains(newname)) {
        profiles.append(newname);
        ret = true;
      }
      break;
    case ProfilesRemove:
    {
      profiles.removeOne(name);
      profileName.deleteGroup();
      profileName.sync();
      ret = true;
      break;
    }
    case ProfilesRename:
    {
      if (!newname.isEmpty() && name != newname) {
        int idx = profiles.indexOf(name);
        if (idx != -1) {
          profiles[idx] = newname;
          KConfigGroup newProfileName(config, profileTypeStr + QLatin1Char('-') + newname);
          if (profileName.exists() && !newProfileName.exists()) {
            profileName.copyTo(&newProfileName);
            profileName.deleteGroup();
            profileName.sync();
            newProfileName.sync();
            ret = true;
          }
        }
      }
      break;
    }
  }
  profileNamesGroup.writeEntry(profileTypeStr, profiles);
  profileNamesGroup.sync();
  return ret;
}

bool CSVImporter::validateDateFormat(const int col)
{
  bool isOK = true;
  for (int row = m_profile->m_startLine; row <= m_profile->m_endLine; ++row) {
      QStandardItem* item = m_file->m_model->item(row, col);
      QDate dat = m_convertDate->convertDate(item->text());
      if (dat == QDate()) {
        isOK = false;
        break;
      }
  }
  return isOK;
}

bool CSVImporter::validateDecimalSymbols(const QList<int> &columns)
{
  bool isOK = true;
  foreach (const auto column, columns) {
    m_file->m_parse->setDecimalSymbol(m_decimalSymbolIndexMap.value(column));
    m_file->m_parse->setThousandsSeparator(m_decimalSymbolIndexMap.value(column));

    for (int row = m_profile->m_startLine; row <= m_profile->m_endLine; ++row) {
      QStandardItem *item = m_file->m_model->item(row, column);
      QString rawNumber = item->text();
      m_file->m_parse->possiblyReplaceSymbol(rawNumber);
      if (m_file->m_parse->invalidConversion() &&
          !rawNumber.isEmpty()) {                   // empty strings are welcome
        isOK = false;
        break;
      }
    }

  }
  return isOK;
}

bool CSVImporter::validateCurrencies(const PricesProfile *profile)
{
  if (profile->m_securitySymbol.isEmpty() ||
      profile->m_currencySymbol.isEmpty())
    return false;
  return true;
}

bool CSVImporter::validateSecurity(const PricesProfile *profile)
{
  if (profile->m_securitySymbol.isEmpty() ||
      profile->m_securityName.isEmpty())
    return false;
  return true;
}

bool CSVImporter::validateSecurity(const InvestmentProfile *profile)
{
  if (profile->m_securitySymbol.isEmpty() ||
      profile->m_securityName.isEmpty())
    return false;
  return true;
}

bool CSVImporter::validateSecurities()
{
    QSet<QString> onlySymbols;
    QSet<QString> onlyNames;
    sortSecurities(onlySymbols, onlyNames, m_mapSymbolName);

    if (!onlySymbols.isEmpty() || !onlyNames.isEmpty())
      return false;
    return true;
}

MyMoneyStatement::Transaction::EAction CSVImporter::processActionTypeField(const InvestmentProfile *profile, const int row, const int col)
{
  if (col == -1)
    return MyMoneyStatement::Transaction::eaNone;

  QString type = m_file->m_model->item(row, col)->text();
  QList<MyMoneyStatement::Transaction::EAction> actions;
  actions << MyMoneyStatement::Transaction::eaBuy << MyMoneyStatement::Transaction::eaSell <<                       // first and second most frequent action
             MyMoneyStatement::Transaction::eaReinvestDividend << MyMoneyStatement::Transaction::eaCashDividend <<  // we don't want "reinv-dividend" to be accidentaly caught by "dividend"
             MyMoneyStatement::Transaction::eaInterest <<
             MyMoneyStatement::Transaction::eaShrsin << MyMoneyStatement::Transaction::eaShrsout;

  foreach (const auto action, actions) {
    if (profile->m_transactionNames.value(action).contains(type, Qt::CaseInsensitive))
      return action;
  }

  return MyMoneyStatement::Transaction::eaNone;
}

validationResultE CSVImporter::validateActionType(MyMoneyStatement::Transaction &tr)
{
  validationResultE ret = ValidActionType;
  QList<MyMoneyStatement::Transaction::EAction> validActionTypes = createValidActionTypes(tr);
  if (validActionTypes.isEmpty())
    ret = InvalidActionValues;
  else if (!validActionTypes.contains(tr.m_eAction))
    ret = NoActionType;
  return ret;
}

bool CSVImporter::calculateFee()
{
  InvestmentProfile *profile = dynamic_cast<InvestmentProfile *>(m_profile);
  if (!profile)
    return false;
  if ((profile->m_feeRate.isEmpty() ||                  // check whether feeRate...
       profile->m_colTypeNum.value(ColumnAmount) == -1)) // ...and amount is in place
    return false;

  QString decimalSymbol;
  if (profile->m_decimalSymbolIndex == 2 ||
      profile->m_decimalSymbolIndex == -1) {
    int detectedSymbol = detectDecimalSymbol(profile->m_colTypeNum.value(ColumnAmount), QString());
    if (detectedSymbol == -1)
      return false;
    m_file->m_parse->setDecimalSymbol(detectedSymbol);
    m_file->m_parse->setThousandsSeparator(detectedSymbol); // separator list is in reverse so it's ok
    decimalSymbol = m_file->m_parse->decimalSymbol(detectedSymbol);
  } else
    decimalSymbol = m_file->m_parse->decimalSymbol(profile->m_decimalSymbolIndex);


  MyMoneyMoney feePercent(m_file->m_parse->possiblyReplaceSymbol(profile->m_feeRate)); // convert 0.67% ...
  feePercent /= MyMoneyMoney(100);                                                     // ... to 0.0067

  if (profile->m_minFee.isEmpty())
    profile->m_minFee = QString::number(0.00, 'f', 2);

  MyMoneyMoney minFee(m_file->m_parse->possiblyReplaceSymbol(profile->m_minFee));

  QList<QStandardItem *> items;
  for (int row = 0; row < profile->m_startLine; ++row) // fill rows above with whitespace for nice effect with markUnwantedRows
    items.append(new QStandardItem(QString()));

  for (int row = profile->m_startLine; row <= profile->m_endLine; ++row) {
    QString txt, numbers;
    bool ok = false;
    numbers = txt = m_file->m_model->item(row, profile->m_colTypeNum.value(ColumnAmount))->text();
    numbers.remove(QRegularExpression(QStringLiteral("[,. ]"))).toInt(&ok);
    if (!ok) {                                      // check if it's numerical string...
      items.append(new QStandardItem(QString()));
      continue;                                     // ...and skip if not (TODO: allow currency symbols and IDs)
    }

    if (txt.startsWith(QLatin1Char('('))) {
      txt.remove(QRegularExpression(QStringLiteral("[()]")));
      txt.prepend(QLatin1Char('-'));
    }
    txt = m_file->m_parse->possiblyReplaceSymbol(txt);
    MyMoneyMoney fee(txt);
    fee *= feePercent;
    if (fee < minFee)
      fee = minFee;
    txt.setNum(fee.toDouble(), 'f', 4);
    txt.replace(QLatin1Char('.'), decimalSymbol); //make sure decimal symbol is uniform in whole line
    items.append(new QStandardItem(txt));
  }

  for (int row = profile->m_endLine + 1; row < m_file->m_rowCount; ++row) // fill rows below with whitespace for nice effect with markUnwantedRows
    items.append(new QStandardItem(QString()));
  int col = profile->m_colTypeNum.value(ColumnFee);
  if (col == -1) {                                          // fee column isn't present
    m_file->m_model->appendColumn(items);
    ++m_file->m_columnCount;
  } else if (col >= m_file->m_columnCount) {    // column number must have been stored in profile
    m_file->m_model->appendColumn(items);
    ++m_file->m_columnCount;
  } else {                                                  // fee column is present and has been recalculated
    m_file->m_model->removeColumn(m_file->m_columnCount - 1);
    m_file->m_model->appendColumn(items);
  }
  profile->m_colTypeNum[ColumnFee] = m_file->m_columnCount - 1;
  return true;
}

int CSVImporter::detectDecimalSymbol(const int col, const QString &exclude)
{
  int detectedSymbol = -1;
  QString pattern;

  QRegularExpression re("^[\\(+-]?\\d+[\\)]?$"); // matches '0' ; '+12' ; '-345' ; '(6789)'

  bool dotIsDecimalSeparator = false;
  bool commaIsDecimalSeparator = false;
  for (int row = m_profile->m_startLine; row <= m_profile->m_endLine; ++row) {
    QString txt = m_file->m_model->item(row, col)->text();
    if (txt.isEmpty())  // nothing to process, so go to next row
      continue;
    int dotPos = txt.lastIndexOf(QLatin1Char('.'));   // get last positions of decimal/thousand separator...
    int commaPos = txt.lastIndexOf(QLatin1Char(',')); // ...to be able to determine which one is the last

    if (dotPos != -1 && commaPos != -1) {
      if (dotPos > commaPos && commaIsDecimalSeparator == false)    // follwing case 1,234.56
        dotIsDecimalSeparator = true;
      else if (dotPos < commaPos && dotIsDecimalSeparator == false) // follwing case 1.234,56
        commaIsDecimalSeparator = true;
      else                                                          // follwing case 1.234,56 and somwhere earlier there was 1,234.56 so unresolvable conflict
        return detectedSymbol;
    } else if (dotPos != -1) {                 // follwing case 1.23
      if (dotIsDecimalSeparator)               // it's already know that dotIsDecimalSeparator
        continue;
      if (!commaIsDecimalSeparator)            // if there is no conflict with comma as decimal separator
        dotIsDecimalSeparator = true;
      else {
        if (txt.count(QLatin1Char('.')) > 1)                // follwing case 1.234.567 so OK
          continue;
        else if (txt.length() - 4 == dotPos)   // follwing case 1.234 and somwhere earlier there was 1.234,56 so OK
          continue;
        else                                   // follwing case 1.23 and somwhere earlier there was 1,23 so unresolvable conflict
          return detectedSymbol;
      }
    } else if (commaPos != -1) {               // follwing case 1,23
      if (commaIsDecimalSeparator)             // it's already know that commaIsDecimalSeparator
        continue;
      else if (!dotIsDecimalSeparator)         // if there is no conflict with dot as decimal separator
        commaIsDecimalSeparator = true;
      else {
        if (txt.count(QLatin1Char(',')) > 1)                // follwing case 1,234,567 so OK
          continue;
        else if (txt.length() - 4 == commaPos) // follwing case 1,234 and somwhere earlier there was 1,234.56 so OK
          continue;
        else                                   // follwing case 1,23 and somwhere earlier there was 1.23 so unresolvable conflict
          return detectedSymbol;
      }

    } else {                                   // follwing case 123
      if (pattern.isEmpty()) {

      }

      txt.remove(QRegularExpression(QLatin1String("[ ") + QRegularExpression::escape(exclude) + QLatin1String("]")));
      QRegularExpressionMatch match = re.match(txt);
      if (match.hasMatch()) // if string is pure numerical then go forward...
        continue;
      else    // ...if not then it's non-numerical garbage
        return detectedSymbol;
    }
  }

  if (dotIsDecimalSeparator)
    detectedSymbol = 0;
  else if (commaIsDecimalSeparator)
    detectedSymbol = 1;
  else {  // whole column was empty, but we don't want to fail so take OS's decimal symbol
    if (QLocale().decimalPoint() == QLatin1Char('.'))
      detectedSymbol = 0;
    else
      detectedSymbol = 1;
  }
  return detectedSymbol;
}

int CSVImporter::detectDecimalSymbols(const QList<int> &columns)
{
  int ret = -2;

  // get list of used currencies to remove them from col
  QList<MyMoneyAccount> accounts;
  MyMoneyFile *file = MyMoneyFile::instance();
  file->accountList(accounts);

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
  foreach (const auto account, accounts) {
    if (accountTypes.contains(account.accountType())) {                             // account must actually have currency property
      currencySymbols.insert(account.currencyId());                                 // add currency id
      currencySymbols.insert(file->currency(account.currencyId()).tradingSymbol()); // add currency symbol
    }
  }
  QString filteredCurrencies = QStringList(currencySymbols.values()).join("");
  QString pattern = QString(QLatin1String("%1%2")).arg(QLocale().currencySymbol()).arg(filteredCurrencies);

  foreach (const auto column, columns) {
    int detectedSymbol = detectDecimalSymbol(column, pattern);
    if (detectedSymbol == -1) {
      ret = column;
      return ret;
    }
    m_decimalSymbolIndexMap.insert(column, detectedSymbol);
  }
  return ret;
}

QList<MyMoneyAccount> CSVImporter::findAccounts(const QList<MyMoneyAccount::accountTypeE> &accountTypes, const QString &statementHeader)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  QList<MyMoneyAccount> accountList;
  file->accountList(accountList);
  QList<MyMoneyAccount> filteredTypes;
  QList<MyMoneyAccount> filteredAccounts;
  QList<MyMoneyAccount>::iterator account;
  QRegularExpression filterOutChars = QRegularExpression(QStringLiteral("-., "));

  foreach (const auto account, accountList) {
    if (accountTypes.contains(account.accountType()) && !(account).isClosed())
        filteredTypes.append(account);
  }

  // filter out accounts whose names aren't in statements header
  foreach (const auto account, filteredTypes) {
    QString txt = account.name();
    txt.remove(filterOutChars);
    if (statementHeader.contains(txt, Qt::CaseInsensitive))
      filteredAccounts.append(account);
  }

  // if filtering returned more results, filter out accounts whose numbers aren't in statements header
  if (filteredAccounts.count() > 1) {
    for (account = filteredAccounts.begin(); account != filteredAccounts.end();) {
      QString txt = (*account).number();
      txt.remove(filterOutChars);
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
    foreach (const auto account, filteredTypes) {
      QString txt = account.number();
      txt.remove(filterOutChars);
      if (statementHeader.contains(txt, Qt::CaseInsensitive))
        filteredAccounts.append(account);
    }
  }
  return filteredAccounts;
}

bool CSVImporter::detectAccount(MyMoneyStatement &st)
{
  QString statementHeader;
  for (int row = 0; row < m_profile->m_startLine; ++row) // concatenate header for better search
    for (int col = 0; col < m_file->m_columnCount; ++col)
      statementHeader.append(m_file->m_model->item(row, col)->text());

  statementHeader.remove(QRegularExpression(QStringLiteral("-., ")));

  QList<MyMoneyAccount> accounts;
  QList<MyMoneyAccount::accountTypeE> accountTypes;

  switch(m_profile->type()) {
    default:
    case ProfileBank:
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
      break;
    case ProfileInvest:
      accountTypes << MyMoneyAccount::Investment; // take investment accounts...
      accounts = findAccounts(accountTypes, statementHeader); //...and search them in statement header
      break;
  }

  if (accounts.count() == 1) { // set account in statement, if it was the only one match
    st.m_strAccountName = accounts.first().name();
    st.m_strAccountNumber = accounts.first().number();
    st.m_accountId = accounts.first().id();

    switch (accounts.first().accountType()) {
      case MyMoneyAccount::Checkings:
        st.m_eType = MyMoneyStatement::etCheckings;
        break;
      case MyMoneyAccount::Savings:
        st.m_eType = MyMoneyStatement::etSavings;
        break;
      case MyMoneyAccount::Investment:
        st.m_eType = MyMoneyStatement::etInvestment;
        break;
      case MyMoneyAccount::CreditCard:
        st.m_eType = MyMoneyStatement::etCreditCard;
        break;
      default:
        st.m_eType = MyMoneyStatement::etNone;
    }
    return true;
  }
  return false;
}

bool CSVImporter::processBankRow(MyMoneyStatement &st, const BankingProfile *profile, const int row)
{
  MyMoneyStatement::Transaction tr;
  QString memo;
  QString txt;

  // process number field
  if (profile->m_colTypeNum.value(ColumnNumber) != -1)
    tr.m_strNumber = txt;

  // process date field
  int col = profile->m_colTypeNum.value(ColumnDate);
  tr.m_datePosted = processDateField(row, col);
  if (tr.m_datePosted == QDate())
    return false;

  // process payee field
  col = profile->m_colTypeNum.value(ColumnPayee);
  if (col != -1)
    tr.m_strPayee = m_file->m_model->item(row, col)->text();

  // process memo field
  col = profile->m_colTypeNum.value(ColumnMemo);
  if (col != -1)
    memo.append(m_file->m_model->item(row, col)->text());

  for (int i = 0; i < profile->m_memoColList.count(); ++i) {
    if (profile->m_memoColList.at(i) != col) {
      if (!memo.isEmpty())
        memo.append(QLatin1Char('\n'));
      if (profile->m_memoColList.at(i) < m_file->m_columnCount)
        memo.append(m_file->m_model->item(row, profile->m_memoColList.at(i))->text());
    }
  }
  tr.m_strMemo = memo;

  // process amount field
  col = profile->m_colTypeNum.value(ColumnAmount);
  tr.m_amount = processAmountField(profile, row, col);
  if (col != -1 && profile->m_oppositeSigns) // change signs to opposite if requested by user
    tr.m_amount *= MyMoneyMoney(-1);

  // process credit/debit field
  if (profile->m_colTypeNum.value(ColumnCredit) != -1 &&
      profile->m_colTypeNum.value(ColumnDebit) != -1) {
    QString credit = m_file->m_model->item(row, profile->m_colTypeNum.value(ColumnCredit))->text();
    QString debit = m_file->m_model->item(row, profile->m_colTypeNum.value(ColumnDebit))->text();
    tr.m_amount = processCreditDebit(credit, debit);
    if (!credit.isEmpty() && !debit.isEmpty())
      return false;
  }

  MyMoneyStatement::Split s1;
  s1.m_amount = tr.m_amount;
  s1.m_strMemo = tr.m_strMemo;
  MyMoneyStatement::Split s2 = s1;
  s2.m_reconcile = tr.m_reconcile;
  s2.m_amount = (-s1.m_amount);

  // process category field
  col = profile->m_colTypeNum.value(ColumnCategory);
  if (col != -1) {
    txt = m_file->m_model->item(row, col)->text();
    QString accountId = m_file->m_csvUtil->checkCategory(txt, s1.m_amount, s2.m_amount);

    if (!accountId.isEmpty()) {
      s2.m_accountId = accountId;
      s2.m_strCategoryName = txt;
      tr.m_listSplits.append(s2);
    }
  }

  // calculate hash
  txt.clear();
  for (int i = 0; i < m_file->m_columnCount; ++i)
    txt.append(m_file->m_model->item(row, i)->text());
  QString hashBase = QString(QLatin1String("%1-%2"))
      .arg(tr.m_datePosted.toString(Qt::ISODate))
      .arg(MyMoneyTransaction::hash(txt));
  QString hash;
  for (uchar idx = 0; idx < 0xFF; ++idx) {  // assuming threre will be no more than 256 transactions with the same hashBase
    hash = QString(QLatin1String("%1-%2")).arg(hashBase).arg(idx);
    QSet<QString>::const_iterator it = m_hashSet.constFind(hash);
    if (it == m_hashSet.constEnd())
      break;
  }
  m_hashSet.insert(hash);
  tr.m_strBankID = hash;

  st.m_listTransactions.append(tr); // Add the MyMoneyStatement::Transaction to the statement
  return true;
}

bool CSVImporter::processInvestRow(MyMoneyStatement &st, const InvestmentProfile *profile, const int row)
{
  MyMoneyStatement::Transaction tr;

  QString memo;
  QString txt;
  // process date field
  int col = profile->m_colTypeNum.value(ColumnDate);
  tr.m_datePosted = processDateField(row, col);
  if (tr.m_datePosted == QDate())
    return false;

  // process quantity field
  col = profile->m_colTypeNum.value(ColumnQuantity);
  tr.m_shares = processQuantityField(profile, row, col);

  // process price field
  col = profile->m_colTypeNum.value(ColumnPrice);
  tr.m_price = processPriceField(profile, row, col);

  // process amount field
  col = profile->m_colTypeNum.value(ColumnAmount);
  tr.m_amount = processAmountField(profile, row, col);

  // process type field
  col = profile->m_colTypeNum.value(ColumnType);
  tr.m_eAction = processActionTypeField(profile, row, col);
  if (!m_isActionTypeValidated && col != -1 &&   // if action type wasn't validated in wizard then...
      validateActionType(tr) != ValidActionType) // ...check if price, amount, quantity is appropriate
    return false;

  // process fee field
  col = profile->m_colTypeNum.value(ColumnFee);
  if (col != -1) {
    if (profile->m_decimalSymbolIndex == 2) {
      int decimalSymbolIndex = m_decimalSymbolIndexMap.value(col);
      m_file->m_parse->setDecimalSymbol(decimalSymbolIndex);
      m_file->m_parse->setThousandsSeparator(decimalSymbolIndex);
    }

    txt = m_file->m_model->item(row, col)->text();
    if (txt.startsWith(QLatin1Char('('))) // check if brackets notation is used for negative numbers
      txt.remove(QRegularExpression(QStringLiteral("[()]")));

    if (txt.isEmpty())
      tr.m_fees = MyMoneyMoney();
    else {
      MyMoneyMoney fee(m_file->m_parse->possiblyReplaceSymbol(txt));
      if (profile->m_feeIsPercentage && profile->m_feeRate.isEmpty())      //   fee is percent
        fee *= tr.m_amount / MyMoneyMoney(100); // as percentage
      fee.abs();
      tr.m_fees = fee;
    }
  }

  // process symbol and name field
  col = profile->m_colTypeNum.value(ColumnSymbol);
  if (col != -1)
    tr.m_strSymbol = m_file->m_model->item(row, col)->text();
  col = profile->m_colTypeNum.value(ColumnName);
  if (col != -1 &&
      tr.m_strSymbol.isEmpty()) { // case in which symbol field is empty
    txt = m_file->m_model->item(row, col)->text();
    tr.m_strSymbol = m_mapSymbolName.key(txt);   // it's all about getting the right symbol
  } else if (!profile->m_securitySymbol.isEmpty())
    tr.m_strSymbol = profile->m_securitySymbol;
  else
    return false;
  tr.m_strSecurity = m_mapSymbolName.value(tr.m_strSymbol); // take name from prepared names to avoid potential name mismatch

  // process memo field
  col = profile->m_colTypeNum.value(ColumnMemo);
  if (col != -1)
    memo.append(m_file->m_model->item(row, col)->text());

  for (int i = 0; i < profile->m_memoColList.count(); ++i) {
    if (profile->m_memoColList.at(i) != col) {
      if (!memo.isEmpty())
        memo.append(QLatin1Char('\n'));
      if (profile->m_memoColList.at(i) < m_file->m_columnCount)
        memo.append(m_file->m_model->item(row, profile->m_memoColList.at(i))->text());
    }
  }
  tr.m_strMemo = memo;

  tr.m_strInterestCategory.clear(); // no special category
  tr.m_strBrokerageAccount.clear(); // no brokerage account auto-detection

  MyMoneyStatement::Split s1;
  s1.m_amount = tr.m_amount;
  s1.m_strMemo = tr.m_strMemo;
  MyMoneyStatement::Split s2 = s1;
  s2.m_amount = MyMoneyMoney(-s1.m_amount);
  s2.m_accountId = m_file->m_csvUtil->checkCategory(tr.m_strInterestCategory, s1.m_amount, s2.m_amount);

  // deduct fees from amount
  if (tr.m_eAction == MyMoneyStatement::Transaction::eaCashDividend ||
      tr.m_eAction == MyMoneyStatement::Transaction::eaSell ||
      tr.m_eAction == MyMoneyStatement::Transaction::eaInterest)
    tr.m_amount -= tr.m_fees;

  else if (tr.m_eAction == MyMoneyStatement::Transaction::eaBuy) {
    if (tr.m_amount.isPositive())
      tr.m_amount = -tr.m_amount; //if broker doesn't use minus sings for buy transactions, set it manually here
    tr.m_amount -= tr.m_fees;
  } else if (tr.m_eAction == MyMoneyStatement::Transaction::eaNone)
    tr.m_listSplits.append(s2);

  st.m_listTransactions.append(tr); // Add the MyMoneyStatement::Transaction to the statement
  return true;
}

bool CSVImporter::processPriceRow(MyMoneyStatement &st, const PricesProfile *profile, const int row)
{
  MyMoneyStatement::Price pr;

  // process date field
  int col = profile->m_colTypeNum.value(ColumnDate);
  pr.m_date = processDateField(row, col);
  if (pr.m_date == QDate())
    return false;

  // process price field
  col = profile->m_colTypeNum.value(ColumnPrice);
  pr.m_amount = processPriceField(profile, row, col);

  switch (profile->type()) {
    case ProfileCurrencyPrices:
      if (profile->m_securitySymbol.isEmpty() || profile->m_currencySymbol.isEmpty())
        return false;
      pr.m_strSecurity = profile->m_securitySymbol;
      pr.m_strCurrency = profile->m_currencySymbol;
      break;
    case ProfileStockPrices:
      if (profile->m_securityName.isEmpty())
        return false;
      pr.m_strSecurity = profile->m_securityName;
      break;
    default:
      return false;
  }

  pr.m_sourceName = profile->m_profileName;
  st.m_listPrices.append(pr); // Add price to the statement
  return true;
}

QDate CSVImporter::processDateField(const int row, const int col)
{
  QDate date;
  if (col != -1) {
    QString txt = m_file->m_model->item(row, col)->text();
    date = m_convertDate->convertDate(txt);      //  Date column
  }
  return date;
}

MyMoneyMoney CSVImporter::processCreditDebit(QString &credit, QString &debit)
{
  MyMoneyMoney amount;
  if (m_profile->m_decimalSymbolIndex == 2)
    setupFieldDecimalSymbol(m_profile->m_colTypeNum.value(ColumnCredit));

  if (credit.startsWith(QLatin1Char('('))) { // check if brackets notation is used for negative numbers
    credit.remove(QRegularExpression(QStringLiteral("[()]")));
    credit.prepend(QLatin1Char('-'));
  }
  if (debit.startsWith(QLatin1Char('('))) { // check if brackets notation is used for negative numbers
    debit.remove(QRegularExpression(QStringLiteral("[()]")));
    debit.prepend(QLatin1Char('-'));
  }

  if (!credit.isEmpty() && !debit.isEmpty()) {  // we do not expect both fields to be non-zero
    if (MyMoneyMoney(credit).isZero())
      credit = QString();
    if (MyMoneyMoney(debit).isZero())
      debit = QString();
  }

  if (!debit.startsWith(QLatin1Char('-')) && !debit.isEmpty()) // ensure debit field is negative
    debit.prepend(QLatin1Char('-'));

  if (!credit.isEmpty() && debit.isEmpty())
    amount = MyMoneyMoney(m_file->m_parse->possiblyReplaceSymbol(credit));
  else if (credit.isEmpty() && !debit.isEmpty())
    amount = MyMoneyMoney(m_file->m_parse->possiblyReplaceSymbol(debit));
  else if (!credit.isEmpty() && !debit.isEmpty()) { // both fields are non-empty and non-zero so let user decide
    return amount;

  } else
    amount = MyMoneyMoney();    // both fields are empty and zero so set amount to zero

  return amount;
}


MyMoneyMoney CSVImporter::processQuantityField(const CSVProfile *profile, const int row, const int col)
{
  MyMoneyMoney shares;
  if (col != -1) {
    if (profile->m_decimalSymbolIndex == 2)
      setupFieldDecimalSymbol(col);

    QString txt = m_file->m_model->item(row, col)->text();
    txt.remove(QRegularExpression(QStringLiteral("+-"))); // remove unwanted sings in quantity

    if (!txt.isEmpty())
      shares = MyMoneyMoney(m_file->m_parse->possiblyReplaceSymbol(txt));
  }
  return shares;
}

MyMoneyMoney CSVImporter::processAmountField(const CSVProfile *profile, const int row, const int col)
{
  MyMoneyMoney amount;
  if (col != -1) {
    if (profile->m_decimalSymbolIndex == 2)
      setupFieldDecimalSymbol(col);

    QString txt = m_file->m_model->item(row, col)->text();
    if (txt.startsWith(QLatin1Char('('))) { // check if brackets notation is used for negative numbers
      txt.remove(QRegularExpression(QStringLiteral("[()]")));
      txt.prepend(QLatin1Char('-'));
    }

    if (!txt.isEmpty())
      amount = MyMoneyMoney(m_file->m_parse->possiblyReplaceSymbol(txt));
  }
  return amount;
}

MyMoneyMoney CSVImporter::processPriceField(const InvestmentProfile *profile, const int row, const int col)
{
  MyMoneyMoney price;
  if (col != -1) {
    if (profile->m_decimalSymbolIndex == 2)
      setupFieldDecimalSymbol(col);

    QString txt = m_file->m_model->item(row, col)->text();
    if (!txt.isEmpty()) {
      price = MyMoneyMoney(m_file->m_parse->possiblyReplaceSymbol(txt));
      price *= m_priceFractions.at(profile->m_priceFraction);
    }
  }
  return price;
}

MyMoneyMoney CSVImporter::processPriceField(const PricesProfile *profile, const int row, const int col)
{
  MyMoneyMoney price;
  if (col != -1) {
    if (profile->m_decimalSymbolIndex == 2)
      setupFieldDecimalSymbol(col);

    QString txt = m_file->m_model->item(row, col)->text();
    if (!txt.isEmpty()) {
      price = MyMoneyMoney(m_file->m_parse->possiblyReplaceSymbol(txt));
      price *= m_priceFractions.at(profile->m_priceFraction);
    }
  }
  return price;
}


QList<MyMoneyStatement::Transaction::EAction> CSVImporter::createValidActionTypes(MyMoneyStatement::Transaction &tr)
{
  QList<MyMoneyStatement::Transaction::EAction> validActionTypes;
  if (tr.m_shares.isPositive() &&
      tr.m_price.isPositive() &&
      !tr.m_amount.isZero())
    validActionTypes << MyMoneyStatement::Transaction::eaReinvestDividend <<
                        MyMoneyStatement::Transaction::eaBuy <<
                        MyMoneyStatement::Transaction::eaSell;
  else if (tr.m_shares.isZero() &&
           tr.m_price.isZero() &&
           !tr.m_amount.isZero())
    validActionTypes << MyMoneyStatement::Transaction::eaCashDividend <<
                        MyMoneyStatement::Transaction::eaInterest;
  else if (tr.m_shares.isPositive() &&
           tr.m_price.isZero() &&
           tr.m_amount.isZero())
    validActionTypes << MyMoneyStatement::Transaction::eaShrsin <<
                        MyMoneyStatement::Transaction::eaShrsout;
  return validActionTypes;
}


bool CSVImporter::sortSecurities(QSet<QString>& onlySymbols, QSet<QString>& onlyNames, QMap<QString, QString>& mapSymbolName)
{
  QList<MyMoneySecurity> securityList = MyMoneyFile::instance()->securityList();
  int symbolCol = m_profile->m_colTypeNum.value(ColumnSymbol);
  int nameCol = m_profile->m_colTypeNum.value(ColumnName);

  // sort by availability of symbol and name
  for (int row = m_profile->m_startLine; row <= m_profile->m_endLine; ++row) {
    QString symbol;
    QString name;
    if (symbolCol != -1)
      symbol = m_file->m_model->item(row, symbolCol)->text().trimmed();
    if (nameCol != -1)
      name = m_file->m_model->item(row, nameCol)->text().trimmed();

    if (!symbol.isEmpty() && !name.isEmpty())
      mapSymbolName.insert(symbol, name);
    else if (!symbol.isEmpty())
      onlySymbols.insert(symbol);
    else if (!name.isEmpty())
      onlyNames.insert(name);
    else
      return false;
  }

  // try to find names for symbols
  for (QSet<QString>::iterator symbol = onlySymbols.begin(); symbol != onlySymbols.end();) {
    QList<MyMoneySecurity> filteredSecurities;
    foreach (const auto sec, securityList) {
      if ((*symbol).compare(sec.tradingSymbol(), Qt::CaseInsensitive) == 0)
        filteredSecurities.append(sec);      // gather all securities that by matched by symbol
    }

    if (filteredSecurities.count() == 1) {                                  // single security matched by the symbol so...
      mapSymbolName.insert(*symbol, filteredSecurities.first().name());
      symbol = onlySymbols.erase(symbol);                                       // ...it's no longer unknown
    } else if (!filteredSecurities.isEmpty()) {                             // multiple securities matched by the symbol
      // TODO: Ask user which security should we match to
      mapSymbolName.insert(*symbol, filteredSecurities.first().name());
      symbol = onlySymbols.erase(symbol);
    } else                                                                  // no security matched, so leave it as unknown
      ++symbol;
  }

  // try to find symbols for names
  for (QSet<QString>::iterator name = onlyNames.begin(); name != onlyNames.end();) {
    QList<MyMoneySecurity> filteredSecurities;
    foreach (const auto sec, securityList) {
      if ((*name).compare(sec.name(), Qt::CaseInsensitive) == 0)
        filteredSecurities.append(sec);      // gather all securities that by matched by name
    }

    if (filteredSecurities.count() == 1) {                                  // single security matched by the name so...
      mapSymbolName.insert(filteredSecurities.first().tradingSymbol(), *name);
      name = onlyNames.erase(name);                                       // ...it's no longer unknown
    } else if (!filteredSecurities.isEmpty()) {                             // multiple securities matched by the name
      // TODO: Ask user which security should we match to
      mapSymbolName.insert(filteredSecurities.first().tradingSymbol(), *name);
      name = onlySymbols.erase(name);
    } else                                                                  // no security matched, so leave it as unknown
      ++name;
  }
  return true;
}

void CSVImporter::setupFieldDecimalSymbol(int col) {
  int decimalSymbolIndex = m_decimalSymbolIndexMap.value(col);
  m_file->m_parse->setDecimalSymbol(decimalSymbolIndex);
  m_file->m_parse->setThousandsSeparator(decimalSymbolIndex);
}

QList<int> CSVImporter::getNumericalColumns()
{
  QList<int> columns;
  switch(m_profile->type()) {
    case ProfileBank:
      if (m_profile->m_colTypeNum.value(ColumnAmount) != -1) {
        columns << m_profile->m_colTypeNum.value(ColumnAmount);
      } else {
        columns << m_profile->m_colTypeNum.value(ColumnDebit);
        columns << m_profile->m_colTypeNum.value(ColumnCredit);
      }
      break;
    case ProfileInvest:
      columns << m_profile->m_colTypeNum.value(ColumnAmount);
      columns << m_profile->m_colTypeNum.value(ColumnPrice);
      columns << m_profile->m_colTypeNum.value(ColumnQuantity);
      if (m_profile->m_colTypeNum.value(ColumnFee) != -1)
        columns << m_profile->m_colTypeNum.value(ColumnFee);
      break;
    case ProfileCurrencyPrices:
    case ProfileStockPrices:
      columns << m_profile->m_colTypeNum.value(ColumnPrice);
      break;
    default:
      break;
  }
  return columns;
}

bool CSVImporter::createStatement(MyMoneyStatement &st)
{
  switch (m_profile->type()) {
    case ProfileBank:
    {
      if (!st.m_listTransactions.isEmpty()) // don't create statement if there is one
        return true;
      st.m_eType = MyMoneyStatement::etNone;
      if (m_autodetect.value(AutoAccountBank))
        detectAccount(st);

      m_hashSet.clear();
      BankingProfile *profile = dynamic_cast<BankingProfile *>(m_profile);
      for (int row = m_profile->m_startLine; row <= m_profile->m_endLine; ++row)
        if (!processBankRow(st, profile, row)) { // parse fields
          st = MyMoneyStatement();
          return false;
        }
      return true;
      break;
    }
    case ProfileInvest:
    {
      if (!st.m_listTransactions.isEmpty()) // don't create statement if there is one
        return true;
      st.m_eType = MyMoneyStatement::etInvestment;
      if (m_autodetect.value(AutoAccountInvest))
        detectAccount(st);

      InvestmentProfile *profile = dynamic_cast<InvestmentProfile *>(m_profile);
      if ((m_profile->m_colTypeNum.value(ColumnFee) == -1 ||
           m_profile->m_colTypeNum.value(ColumnFee) >= m_file->m_columnCount) &&
          !profile->m_feeRate.isEmpty()) // fee column has not been calculated so do it now
        calculateFee();

      for (int row = m_profile->m_startLine; row <= m_profile->m_endLine; ++row)
        if (!processInvestRow(st, profile, row)) { // parse fields
          st = MyMoneyStatement();
          return false;
        }

      for (QMap<QString, QString>::const_iterator it = m_mapSymbolName.cbegin(); it != m_mapSymbolName.cend(); ++it) {
        MyMoneyStatement::Security security;
        security.m_strSymbol = it.key();
        security.m_strName = it.value();
        st.m_listSecurities.append(security);
      }
      return true;
      break;
    }
    default:
    case ProfileCurrencyPrices:
    case ProfileStockPrices:
    {
      if (!st.m_listPrices.isEmpty()) // don't create statement if there is one
        return true;
      st.m_eType = MyMoneyStatement::etNone;

      PricesProfile *profile = dynamic_cast<PricesProfile *>(m_profile);
      for (int row = m_profile->m_startLine; row <= m_profile->m_endLine; ++row)
        if (!processPriceRow(st, profile, row)) { // parse fields
          st = MyMoneyStatement();
          return false;
        }

      for (QMap<QString, QString>::const_iterator it = m_mapSymbolName.cbegin(); it != m_mapSymbolName.cend(); ++it) {
        MyMoneyStatement::Security security;
        security.m_strSymbol = it.key();
        security.m_strName = it.value();
        st.m_listSecurities.append(security);
      }
      return true;
    }
  }
  return true;
}

void CSVProfile::readSettings(const KConfigGroup &profilesGroup)
{
  m_lastUsedDirectory = profilesGroup.readEntry(CSVImporter::m_miscSettingsConfName.value(ConfDirectory), QString());
  m_startLine = profilesGroup.readEntry(CSVImporter::m_miscSettingsConfName.value(ConfStartLine), 0);
  m_trailerLines = profilesGroup.readEntry(CSVImporter::m_miscSettingsConfName.value(ConfTrailerLines), 0);
  m_encodingMIBEnum = profilesGroup.readEntry(CSVImporter::m_miscSettingsConfName.value(ConfEncoding), 106 /* UTF-8 */);

  m_dateFormatIndex = profilesGroup.readEntry(CSVImporter::m_miscSettingsConfName.value(ConfDateFormat), 0);
  m_textDelimiterIndex = profilesGroup.readEntry(CSVImporter::m_miscSettingsConfName.value(ConfTextDeimiter), 0);
  m_fieldDelimiterIndex = profilesGroup.readEntry(CSVImporter::m_miscSettingsConfName.value(ConfFieldDelimiter), -1);
  m_decimalSymbolIndex = profilesGroup.readEntry(CSVImporter::m_miscSettingsConfName.value(ConfDecimalSymbol), 2);
  initColNumType();
}

void CSVProfile::writeSettings(KConfigGroup &profilesGroup)
{
  if (m_lastUsedDirectory.startsWith(QDir::homePath())) { // replace /home/user with ~/ for brevity
    QFileInfo fileInfo = QFileInfo(m_lastUsedDirectory);
    if (fileInfo.isFile())
      m_lastUsedDirectory = fileInfo.absolutePath();
    m_lastUsedDirectory.replace(0, QDir::homePath().length(), QLatin1Char('~'));
  }
  profilesGroup.writeEntry(CSVImporter::m_miscSettingsConfName.value(ConfDirectory), m_lastUsedDirectory);
  profilesGroup.writeEntry(CSVImporter::m_miscSettingsConfName.value(ConfEncoding), m_encodingMIBEnum);
  profilesGroup.writeEntry(CSVImporter::m_miscSettingsConfName.value(ConfDateFormat), m_dateFormatIndex);
  profilesGroup.writeEntry(CSVImporter::m_miscSettingsConfName.value(ConfFieldDelimiter), m_fieldDelimiterIndex);
  profilesGroup.writeEntry(CSVImporter::m_miscSettingsConfName.value(ConfTextDeimiter), m_textDelimiterIndex);
  profilesGroup.writeEntry(CSVImporter::m_miscSettingsConfName.value(ConfDecimalSymbol), m_decimalSymbolIndex);
  profilesGroup.writeEntry(CSVImporter::m_miscSettingsConfName.value(ConfStartLine), m_startLine);
  profilesGroup.writeEntry(CSVImporter::m_miscSettingsConfName.value(ConfTrailerLines), m_trailerLines);
}

bool BankingProfile::readSettings(const KSharedConfigPtr &config)
{
  bool exists = true;
  KConfigGroup profilesGroup(config, CSVImporter::m_profileConfPrefix.value(type()) + QLatin1Char('-') + m_profileName);
  if (!profilesGroup.exists())
    exists = false;

  m_colTypeNum[ColumnPayee] = profilesGroup.readEntry(CSVImporter::m_colTypeConfName.value(ColumnPayee), -1);
  m_colTypeNum[ColumnNumber] = profilesGroup.readEntry(CSVImporter::m_colTypeConfName.value(ColumnNumber), -1);
  m_colTypeNum[ColumnAmount] = profilesGroup.readEntry(CSVImporter::m_colTypeConfName.value(ColumnAmount), -1);
  m_colTypeNum[ColumnDebit] = profilesGroup.readEntry(CSVImporter::m_colTypeConfName.value(ColumnDebit), -1);
  m_colTypeNum[ColumnCredit] = profilesGroup.readEntry(CSVImporter::m_colTypeConfName.value(ColumnCredit), -1);
  m_colTypeNum[ColumnDate] = profilesGroup.readEntry(CSVImporter::m_colTypeConfName.value(ColumnDate), -1);
  m_colTypeNum[ColumnCategory] = profilesGroup.readEntry(CSVImporter::m_colTypeConfName.value(ColumnCategory), -1);
  m_colTypeNum[ColumnMemo] = -1; // initialize, otherwise random data may go here
  m_oppositeSigns = profilesGroup.readEntry(CSVImporter::m_miscSettingsConfName.value(ConfOppositeSigns), 0);
  m_memoColList = profilesGroup.readEntry(CSVImporter::m_colTypeConfName.value(ColumnMemo), QList<int>());

  CSVProfile::readSettings(profilesGroup);
  return exists;
}

void BankingProfile::writeSettings(const KSharedConfigPtr &config)
{
  KConfigGroup profilesGroup(config, CSVImporter::m_profileConfPrefix.value(type()) + QLatin1Char('-') + m_profileName);
  CSVProfile::writeSettings(profilesGroup);

  profilesGroup.writeEntry(CSVImporter::m_miscSettingsConfName.value(ConfOppositeSigns), m_oppositeSigns);
  profilesGroup.writeEntry(CSVImporter::m_colTypeConfName.value(ColumnPayee),
                           m_colTypeNum.value(ColumnPayee));
  profilesGroup.writeEntry(CSVImporter::m_colTypeConfName.value(ColumnNumber),
                           m_colTypeNum.value(ColumnNumber));
  profilesGroup.writeEntry(CSVImporter::m_colTypeConfName.value(ColumnAmount),
                           m_colTypeNum.value(ColumnAmount));
  profilesGroup.writeEntry(CSVImporter::m_colTypeConfName.value(ColumnDebit),
                           m_colTypeNum.value(ColumnDebit));
  profilesGroup.writeEntry(CSVImporter::m_colTypeConfName.value(ColumnCredit),
                           m_colTypeNum.value(ColumnCredit));
  profilesGroup.writeEntry(CSVImporter::m_colTypeConfName.value(ColumnDate),
                           m_colTypeNum.value(ColumnDate));
  profilesGroup.writeEntry(CSVImporter::m_colTypeConfName.value(ColumnCategory),
                           m_colTypeNum.value(ColumnCategory));
  profilesGroup.writeEntry(CSVImporter::m_colTypeConfName.value(ColumnMemo),
                           m_memoColList);
  profilesGroup.config()->sync();
}

bool InvestmentProfile::readSettings(const KSharedConfigPtr &config)
{
  bool exists = true;
  KConfigGroup profilesGroup(config, CSVImporter::m_profileConfPrefix.value(type()) + QLatin1Char('-') + m_profileName);
  if (!profilesGroup.exists())
    exists = false;

  m_transactionNames[MyMoneyStatement::Transaction::eaBuy] = profilesGroup.readEntry(CSVImporter::m_transactionConfName.value(MyMoneyStatement::Transaction::eaBuy),
                                                                   QString(i18nc("Type of operation as in financial statement", "buy")).split(',', QString::SkipEmptyParts));
  m_transactionNames[MyMoneyStatement::Transaction::eaSell] = profilesGroup.readEntry(CSVImporter::m_transactionConfName.value(MyMoneyStatement::Transaction::eaSell),
                                                                    QString(i18nc("Type of operation as in financial statement", "sell,repurchase")).split(',', QString::SkipEmptyParts));
  m_transactionNames[MyMoneyStatement::Transaction::eaReinvestDividend] = profilesGroup.readEntry(CSVImporter::m_transactionConfName.value(MyMoneyStatement::Transaction::eaReinvestDividend),
                                                                                QString(i18nc("Type of operation as in financial statement", "reinvest,reinv,re-inv")).split(',', QString::SkipEmptyParts));
  m_transactionNames[MyMoneyStatement::Transaction::eaCashDividend] = profilesGroup.readEntry(CSVImporter::m_transactionConfName.value(MyMoneyStatement::Transaction::eaCashDividend),
                                                                            QString(i18nc("Type of operation as in financial statement", "dividend")).split(',', QString::SkipEmptyParts));
  m_transactionNames[MyMoneyStatement::Transaction::eaInterest] = profilesGroup.readEntry(CSVImporter::m_transactionConfName.value(MyMoneyStatement::Transaction::eaInterest),
                                                                        QString(i18nc("Type of operation as in financial statement", "interest,income")).split(',', QString::SkipEmptyParts));
  m_transactionNames[MyMoneyStatement::Transaction::eaShrsin] = profilesGroup.readEntry(CSVImporter::m_transactionConfName.value(MyMoneyStatement::Transaction::eaShrsin),
                                                                      QString(i18nc("Type of operation as in financial statement", "add,stock dividend,divd reinv,transfer in,re-registration in,journal entry")).split(',', QString::SkipEmptyParts));
  m_transactionNames[MyMoneyStatement::Transaction::eaShrsout] = profilesGroup.readEntry(CSVImporter::m_transactionConfName.value(MyMoneyStatement::Transaction::eaShrsout),
                                                                       QString(i18nc("Type of operation as in financial statement", "remove")).split(',', QString::SkipEmptyParts));

  m_colTypeNum[ColumnDate] = profilesGroup.readEntry(CSVImporter::m_colTypeConfName.value(ColumnDate), -1);
  m_colTypeNum[ColumnType] = profilesGroup.readEntry(CSVImporter::m_colTypeConfName.value(ColumnType), -1);  //use for type col.
  m_colTypeNum[ColumnPrice] = profilesGroup.readEntry(CSVImporter::m_colTypeConfName.value(ColumnPrice), -1);
  m_colTypeNum[ColumnQuantity] = profilesGroup.readEntry(CSVImporter::m_colTypeConfName.value(ColumnQuantity), -1);
  m_colTypeNum[ColumnAmount] = profilesGroup.readEntry(CSVImporter::m_colTypeConfName.value(ColumnAmount), -1);
  m_colTypeNum[ColumnName] = profilesGroup.readEntry(CSVImporter::m_colTypeConfName.value(ColumnName), -1);
  m_colTypeNum[ColumnFee] = profilesGroup.readEntry(CSVImporter::m_colTypeConfName.value(ColumnFee), -1);
  m_colTypeNum[ColumnSymbol] = profilesGroup.readEntry(CSVImporter::m_colTypeConfName.value(ColumnSymbol), -1);
  m_colTypeNum[ColumnMemo] = -1; // initialize, otherwise random data may go here
  m_feeIsPercentage = profilesGroup.readEntry(CSVImporter::m_miscSettingsConfName.value(ConfFeeIsPercentage), 0);
  m_feeRate = profilesGroup.readEntry(CSVImporter::m_miscSettingsConfName.value(ConfFeeRate), QString());
  m_minFee = profilesGroup.readEntry(CSVImporter::m_miscSettingsConfName.value(ConfMinFee), QString());

  m_memoColList = profilesGroup.readEntry(CSVImporter::m_colTypeConfName.value(ColumnMemo), QList<int>());
  m_securityName = profilesGroup.readEntry(CSVImporter::m_miscSettingsConfName.value(ConfSecurityName), QString());
  m_securitySymbol = profilesGroup.readEntry(CSVImporter::m_miscSettingsConfName.value(ConfSecuritySymbol), QString());
  m_dontAsk = profilesGroup.readEntry(CSVImporter::m_miscSettingsConfName.value(ConfDontAsk), 0);
  m_priceFraction = profilesGroup.readEntry(CSVImporter::m_miscSettingsConfName.value(ConfPriceFraction), 2);

  CSVProfile::readSettings(profilesGroup);
  return exists;
}

void InvestmentProfile::writeSettings(const KSharedConfigPtr &config)
{
  KConfigGroup profilesGroup(config, CSVImporter::m_profileConfPrefix.value(type()) + QLatin1Char('-') + m_profileName);
  CSVProfile::writeSettings(profilesGroup);

  profilesGroup.writeEntry(CSVImporter::m_transactionConfName.value(MyMoneyStatement::Transaction::eaBuy),
                           m_transactionNames.value(MyMoneyStatement::Transaction::eaBuy));
  profilesGroup.writeEntry(CSVImporter::m_transactionConfName.value(MyMoneyStatement::Transaction::eaSell),
                           m_transactionNames.value(MyMoneyStatement::Transaction::eaSell));
  profilesGroup.writeEntry(CSVImporter::m_transactionConfName.value(MyMoneyStatement::Transaction::eaReinvestDividend),
                           m_transactionNames.value(MyMoneyStatement::Transaction::eaReinvestDividend));
  profilesGroup.writeEntry(CSVImporter::m_transactionConfName.value(MyMoneyStatement::Transaction::eaCashDividend),
                           m_transactionNames.value(MyMoneyStatement::Transaction::eaCashDividend));
  profilesGroup.writeEntry(CSVImporter::m_transactionConfName.value(MyMoneyStatement::Transaction::eaInterest),
                           m_transactionNames.value(MyMoneyStatement::Transaction::eaInterest));
  profilesGroup.writeEntry(CSVImporter::m_transactionConfName.value(MyMoneyStatement::Transaction::eaShrsin),
                           m_transactionNames.value(MyMoneyStatement::Transaction::eaShrsin));
  profilesGroup.writeEntry(CSVImporter::m_transactionConfName.value(MyMoneyStatement::Transaction::eaShrsout),
                           m_transactionNames.value(MyMoneyStatement::Transaction::eaShrsout));

  profilesGroup.writeEntry(CSVImporter::m_miscSettingsConfName.value(ConfPriceFraction), m_priceFraction);
  profilesGroup.writeEntry(CSVImporter::m_miscSettingsConfName.value(ConfFeeIsPercentage), m_feeIsPercentage);
  profilesGroup.writeEntry(CSVImporter::m_miscSettingsConfName.value(ConfFeeRate), m_feeRate);
  profilesGroup.writeEntry(CSVImporter::m_miscSettingsConfName.value(ConfMinFee), m_minFee);
  profilesGroup.writeEntry(CSVImporter::m_miscSettingsConfName.value(ConfSecurityName), m_securityName);
  profilesGroup.writeEntry(CSVImporter::m_miscSettingsConfName.value(ConfSecuritySymbol), m_securitySymbol);
  profilesGroup.writeEntry(CSVImporter::m_miscSettingsConfName.value(ConfDontAsk), m_dontAsk);

  profilesGroup.writeEntry(CSVImporter::m_colTypeConfName.value(ColumnDate),
                           m_colTypeNum.value(ColumnDate));
  profilesGroup.writeEntry(CSVImporter::m_colTypeConfName.value(ColumnType),
                           m_colTypeNum.value(ColumnType));
  profilesGroup.writeEntry(CSVImporter::m_colTypeConfName.value(ColumnQuantity),
                           m_colTypeNum.value(ColumnQuantity));
  profilesGroup.writeEntry(CSVImporter::m_colTypeConfName.value(ColumnAmount),
                           m_colTypeNum.value(ColumnAmount));
  profilesGroup.writeEntry(CSVImporter::m_colTypeConfName.value(ColumnPrice),
                           m_colTypeNum.value(ColumnPrice));
  profilesGroup.writeEntry(CSVImporter::m_colTypeConfName.value(ColumnSymbol),
                           m_colTypeNum.value(ColumnSymbol));
  profilesGroup.writeEntry(CSVImporter::m_colTypeConfName.value(ColumnName),
                           m_colTypeNum.value(ColumnName));
  profilesGroup.writeEntry(CSVImporter::m_colTypeConfName.value(ColumnFee),
                           m_colTypeNum.value(ColumnFee));
  profilesGroup.writeEntry(CSVImporter::m_colTypeConfName.value(ColumnMemo),
                           m_memoColList);
  profilesGroup.config()->sync();
}

bool PricesProfile::readSettings(const KSharedConfigPtr &config)
{
  bool exists = true;
  KConfigGroup profilesGroup(config, CSVImporter::m_profileConfPrefix.value(type()) + QLatin1Char('-') + m_profileName);
  if (!profilesGroup.exists())
    exists = false;

  m_colTypeNum[ColumnDate] = profilesGroup.readEntry(CSVImporter::m_colTypeConfName.value(ColumnDate), -1);
  m_colTypeNum[ColumnPrice] = profilesGroup.readEntry(CSVImporter::m_colTypeConfName.value(ColumnPrice), -1);
  m_priceFraction = profilesGroup.readEntry(CSVImporter::m_miscSettingsConfName.value(ConfPriceFraction), 2);
  m_securityName = profilesGroup.readEntry(CSVImporter::m_miscSettingsConfName.value(ConfSecurityName), QString());
  m_securitySymbol = profilesGroup.readEntry(CSVImporter::m_miscSettingsConfName.value(ConfSecuritySymbol), QString());
  m_currencySymbol = profilesGroup.readEntry(CSVImporter::m_miscSettingsConfName.value(ConfCurrencySymbol), QString());
  m_dontAsk = profilesGroup.readEntry(CSVImporter::m_miscSettingsConfName.value(ConfDontAsk), 0);

  CSVProfile::readSettings(profilesGroup);
  return exists;
}

void PricesProfile::writeSettings(const KSharedConfigPtr &config)
{
  KConfigGroup profilesGroup(config, CSVImporter::m_profileConfPrefix.value(type()) + QLatin1Char('-') + m_profileName);
  CSVProfile::writeSettings(profilesGroup);

  profilesGroup.writeEntry(CSVImporter::m_colTypeConfName.value(ColumnDate),
                           m_colTypeNum.value(ColumnDate));
  profilesGroup.writeEntry(CSVImporter::m_colTypeConfName.value(ColumnPrice),
                           m_colTypeNum.value(ColumnPrice));
  profilesGroup.writeEntry(CSVImporter::m_miscSettingsConfName.value(ConfPriceFraction), m_priceFraction);
  profilesGroup.writeEntry(CSVImporter::m_miscSettingsConfName.value(ConfSecurityName), m_securityName);
  profilesGroup.writeEntry(CSVImporter::m_miscSettingsConfName.value(ConfSecuritySymbol), m_securitySymbol);
  profilesGroup.writeEntry(CSVImporter::m_miscSettingsConfName.value(ConfCurrencySymbol), m_currencySymbol);
  profilesGroup.writeEntry(CSVImporter::m_miscSettingsConfName.value(ConfDontAsk), m_dontAsk);
  profilesGroup.config()->sync();
}

CSVFile::CSVFile()
{
  m_parse = new Parse;
  m_csvUtil = new CsvUtil;
  m_model = new QStandardItemModel();
}

CSVFile::~CSVFile()
{
  delete m_parse;
  delete m_csvUtil;
  delete m_model;
}

void CSVFile::getStartEndRow(CSVProfile *profile)
{
  profile->m_endLine = m_rowCount - 1;
  if (profile->m_endLine > profile->m_trailerLines)
    profile->m_endLine -= profile->m_trailerLines;

  if (profile->m_startLine > m_rowCount - 1)   // Don't allow m_startLine > m_endLine
    profile->m_startLine = m_rowCount - 1;
}

void CSVFile::getColumnCount(CSVProfile *profile, const QStringList &rows)
{
  if (rows.isEmpty())
    return;

  QList<int> delimiterIndexes;
  if (profile->m_fieldDelimiterIndex == -1)
    delimiterIndexes = QList<int>{0, 1, 2, 3};  // include all delimiters to test or ...
  else
    delimiterIndexes = QList<int>{profile->m_fieldDelimiterIndex};   // ... only the one specified

  QList<int> totalDelimiterCount({0, 0, 0, 0}); //  Total in file for each delimiter
  QList<int> thisDelimiterCount({0, 0, 0, 0});  //  Total in this line for each delimiter
  int colCount = 0;                             //  Total delimiters in this line
  int possibleDelimiter = 0;
  m_columnCount = 0;

  foreach (const auto row, rows) {
    foreach(const auto delimiterIndex, delimiterIndexes) {
      m_parse->setFieldDelimiterIndex(delimiterIndex);
      colCount = m_parse->parseLine(row).count(); //  parse each line using each delimiter

      if (colCount > thisDelimiterCount.at(delimiterIndex))
        thisDelimiterCount[delimiterIndex] = colCount;

      if (thisDelimiterCount[delimiterIndex] > m_columnCount)
        m_columnCount = thisDelimiterCount.at(delimiterIndex);

      totalDelimiterCount[delimiterIndex] += colCount;
      if (totalDelimiterCount.at(delimiterIndex) > totalDelimiterCount.at(possibleDelimiter))
        possibleDelimiter = delimiterIndex;
    }
  }
  if (delimiterIndexes.count() != 1)                    // if purpose was to autodetect...
    profile->m_fieldDelimiterIndex = possibleDelimiter; // ... then change field delimiter
  m_parse->setFieldDelimiterIndex(profile->m_fieldDelimiterIndex); // restore original field delimiter
}

bool CSVFile::getInFileName(QString inFileName)
{
  QFileInfo fileInfo;
  if (!inFileName.isEmpty()) {
    if (inFileName.startsWith(QLatin1Char('~')))
      inFileName.replace(0, 1, QDir::homePath());
    fileInfo = QFileInfo(inFileName);
    if (fileInfo.isFile()) {       // if it is file...
      if (fileInfo.exists()) {     // ...and exists...
        m_inFileName = inFileName; // ...then set as valid filename
        return true;               // ...and return success...
      } else {                     // ...but if not...
        fileInfo.setFile(fileInfo.absolutePath()); //...then set start directory to directory of that file...
        if (!fileInfo.exists())                    //...and if it doesn't exist too...
          fileInfo.setFile(QDir::homePath());      //...then set start directory to home path
      }
    } else if (fileInfo.isDir()) {
      if (fileInfo.exists())
        fileInfo = QFileInfo(inFileName);
      else
        fileInfo.setFile(QDir::homePath());
    }
  } else
    fileInfo = QFileInfo(QDir::homePath());

  QPointer<QFileDialog> dialog = new QFileDialog(nullptr, QString(),
                                                 fileInfo.absoluteFilePath(),
                                                 i18n("CSV Files (*.csv)"));
  dialog->setFileMode(QFileDialog::ExistingFile);
  QUrl url;
  if (dialog->exec() == QDialog::Accepted)
    url = dialog->selectedUrls().first();
  delete dialog;

  if (url.isEmpty()) {
    m_inFileName.clear();
    return false;
  } else
    m_inFileName = url.toDisplayString(QUrl::PreferLocalFile);

  return true;
}

void CSVFile::setupParser(CSVProfile *profile)
{
  if (profile->m_decimalSymbolIndex != 2) {
    m_parse->setDecimalSymbol(profile->m_decimalSymbolIndex);
    m_parse->setThousandsSeparator(profile->m_decimalSymbolIndex);
  }
  m_parse->setFieldDelimiterCharacter(profile->m_fieldDelimiterIndex);
  m_parse->setTextDelimiterCharacter(profile->m_textDelimiterIndex);
}

void CSVFile::readFile(CSVProfile *profile)
{
  QFile inFile(m_inFileName);
  if (!inFile.exists())
    return;
  inFile.open(QIODevice::ReadOnly);
  QTextStream inStream(&inFile);
  QTextCodec* codec = QTextCodec::codecForMib(profile->m_encodingMIBEnum);
  inStream.setCodec(codec);

  QString buf = inStream.readAll();
  inFile.close();
  m_parse->setTextDelimiterCharacter(profile->m_textDelimiterIndex);
  QStringList rows = m_parse->parseFile(buf, 1, 0);  // parse the buffer
  m_rowCount = m_parse->lastLine();                  // won't work without above line
  getColumnCount(profile, rows);
  getStartEndRow(profile);

  // prepare model from rows having rowCount and columnCount
  m_model->clear();
  for (int i = 0; i < m_rowCount; ++i) {
    QList<QStandardItem*> itemList;
    QStringList columns = m_parse->parseLine(rows.takeFirst());  // take instead of read from rows to preserve memory
    for (int j = 0; j < m_columnCount; ++j)
      itemList.append(new QStandardItem(columns.value(j, QString())));
    m_model->appendRow(itemList);
  }
}
