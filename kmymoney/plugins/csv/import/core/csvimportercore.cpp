/*
    SPDX-FileCopyrightText: 2010 Allan Anderson <agander93@gmail.com>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "csvimportercore.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QTextCodec>
#include <QTextStream>
#include <QFileDialog>
#include <QRegularExpression>
#include <QStandardItem>
#include <QPointer>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KMessageBox>
#include <KConfigGroup>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneysecurity.h"
#include "mymoneytransaction.h"
#include "csvutil.h"
#include "convdate.h"
#include "mymoneyenums.h"

const QHash<Profile, QString> CSVImporterCore::m_profileConfPrefix {
    {Profile::Banking, QStringLiteral("Bank")},
    {Profile::Investment, QStringLiteral("Invest")},
    {Profile::CurrencyPrices, QStringLiteral("CPrices")},
    {Profile::StockPrices, QStringLiteral("SPrices")}
};

const QHash<Column, QString> CSVImporterCore::m_colTypeConfName {
    {Column::Date, QStringLiteral("DateCol")},
    {Column::Memo, QStringLiteral("MemoCol")},
    {Column::Number, QStringLiteral("NumberCol")},
    {Column::Payee, QStringLiteral("PayeeCol")},
    {Column::Amount, QStringLiteral("AmountCol")},
    {Column::Credit, QStringLiteral("CreditCol")},
    {Column::Debit, QStringLiteral("DebitCol")},
    {Column::Category, QStringLiteral("CategoryCol")},
    {Column::Type, QStringLiteral("TypeCol")},
    {Column::Price, QStringLiteral("PriceCol")},
    {Column::Quantity, QStringLiteral("QuantityCol")},
    {Column::Fee, QStringLiteral("FeeCol")},
    {Column::Symbol, QStringLiteral("SymbolCol")},
    {Column::Name, QStringLiteral("NameCol")},
    {Column::CreditDebitIndicator, QStringLiteral("CreditDebitIndicatorCol")},
    {Column::Balance, QStringLiteral("BalanceCol")},
};

const QHash<miscSettingsE, QString> CSVImporterCore::m_miscSettingsConfName{{ConfDirectory, QStringLiteral("Directory")},
                                                                            {ConfEncoding, QStringLiteral("Encoding")},
                                                                            {ConfDateFormat, QStringLiteral("DateFormat")},
                                                                            {ConfFieldDelimiter, QStringLiteral("FieldDelimiter")},
                                                                            {ConfTextDelimiter, QStringLiteral("TextDelimiter")},
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
                                                                            {ConfWidth, QStringLiteral("Width")},
                                                                            {ConfCreditIndicator, QStringLiteral("CreditIndicator")},
                                                                            {ConfDebitIndicator, QStringLiteral("DebitIndicator")},
                                                                            {ConfAutoAccountName, QStringLiteral("AutoAccountName")}};

const QHash<eMyMoney::Transaction::Action, QString> CSVImporterCore::m_transactionConfName {
    {eMyMoney::Transaction::Action::Buy, QStringLiteral("BuyParam")},
    {eMyMoney::Transaction::Action::Sell, QStringLiteral("SellParam")},
    {eMyMoney::Transaction::Action::ReinvestDividend, QStringLiteral("ReinvdivParam")},
    {eMyMoney::Transaction::Action::CashDividend, QStringLiteral("DivXParam")},
    {eMyMoney::Transaction::Action::Interest, QStringLiteral("IntIncParam")},
    {eMyMoney::Transaction::Action::Shrsin, QStringLiteral("ShrsinParam")},
    {eMyMoney::Transaction::Action::Shrsout, QStringLiteral("ShrsoutParam")},
};

const QString CSVImporterCore::m_confProfileNames = QStringLiteral("ProfileNames");
const QString CSVImporterCore::m_confPriorName = QStringLiteral("Prior");
const QString CSVImporterCore::m_confMiscName = QStringLiteral("Misc");

CSVImporterCore::CSVImporterCore() :
    m_profile(0),
    m_isActionTypeValidated(false)
{
    m_convertDate = new ConvertDate;
    m_file = new CSVFile;

    m_priceFractions << MyMoneyMoney(0.01) << MyMoneyMoney(0.1) << MyMoneyMoney::ONE << MyMoneyMoney(10) << MyMoneyMoney(100);

    validateConfigFile();
    readMiscSettings();
}
CSVImporterCore::~CSVImporterCore()
{
    delete m_convertDate;
    delete m_file;
}

MyMoneyStatement CSVImporterCore::unattendedImport(const QString &filename, CSVProfile *profile)
{
    MyMoneyStatement st;
    m_profile = profile;
    m_convertDate->setDateFormatIndex(m_profile->m_dateFormat);

    if (m_file->getInFileName(filename)) {
        m_file->readFile(m_profile);
        m_file->setupParser(m_profile);

        if (profile->m_decimalSymbol == DecimalSymbol::Auto) {
            auto columns = getNumericalColumns();
            if (detectDecimalSymbols(columns) != -2)
                return st;
        }

        if (!createStatement(st))
            st = MyMoneyStatement();
    }
    return st;
}

KSharedConfigPtr CSVImporterCore::configFile()
{
    return KSharedConfig::openConfig(QStringLiteral("kmymoney/csvimporterrc"));
}

void CSVImporterCore::profileFactory(const Profile type, const QString &name)
{
    // delete current profile
    if (m_profile) {
        delete m_profile;
        m_profile = nullptr;
    }

    switch (type) {
    default:
    case Profile::Investment:
        m_profile = new InvestmentProfile;
        break;
    case Profile::Banking:
        m_profile = new BankingProfile;
        break;
    case Profile::CurrencyPrices:
    case Profile::StockPrices:
        m_profile = new PricesProfile(type);
        break;
    }
    m_profile->m_profileName = name;
}

void CSVImporterCore::readMiscSettings() {
    KConfigGroup miscGroup(configFile(), m_confMiscName);
    m_autodetect.clear();
    m_autodetect.insert(AutoFieldDelimiter, miscGroup.readEntry(QStringLiteral("AutoFieldDelimiter"), true));
    m_autodetect.insert(AutoDecimalSymbol, miscGroup.readEntry(QStringLiteral("AutoDecimalSymbol"), true));
    m_autodetect.insert(AutoDateFormat, miscGroup.readEntry(QStringLiteral("AutoDateFormat"), true));
}

void CSVImporterCore::validateConfigFile()
{
    const KSharedConfigPtr config = configFile();
    KConfigGroup profileNamesGroup(config, m_confProfileNames);
    if (!profileNamesGroup.exists()) {
        profileNamesGroup.writeEntry(m_profileConfPrefix.value(Profile::Banking), QStringList());
        profileNamesGroup.writeEntry(m_profileConfPrefix.value(Profile::Investment), QStringList());
        profileNamesGroup.writeEntry(m_profileConfPrefix.value(Profile::CurrencyPrices), QStringList());
        profileNamesGroup.writeEntry(m_profileConfPrefix.value(Profile::StockPrices), QStringList());
        profileNamesGroup.writeEntry(m_confPriorName + m_profileConfPrefix.value(Profile::Banking), int());
        profileNamesGroup.writeEntry(m_confPriorName + m_profileConfPrefix.value(Profile::Investment), int());
        profileNamesGroup.writeEntry(m_confPriorName + m_profileConfPrefix.value(Profile::CurrencyPrices), int());
        profileNamesGroup.writeEntry(m_confPriorName + m_profileConfPrefix.value(Profile::StockPrices), int());
        profileNamesGroup.sync();
    }

    KConfigGroup miscGroup(config, m_confMiscName);
    if (!miscGroup.exists()) {
        miscGroup.writeEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfHeight), "400");
        miscGroup.writeEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfWidth), "800");
        miscGroup.sync();
    }

    QList<int> confVer = miscGroup.readEntry("KMMVer", QList<int> {0, 0, 0});
    if (updateConfigFile(confVer)) // write kmmVer only if there were no errors
        miscGroup.writeEntry("KMMVer", confVer);
}

bool CSVImporterCore::updateConfigFile(QList<int> &confVer)
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
    QStringList bankProfiles = profileNamesGroup.readEntry(m_profileConfPrefix.value(Profile::Banking), QStringList());
    QStringList investProfiles = profileNamesGroup.readEntry(m_profileConfPrefix.value(Profile::Investment), QStringList());
    QStringList invalidBankProfiles = profileNamesGroup.readEntry(QLatin1String("Invalid") + m_profileConfPrefix.value(Profile::Banking), QStringList());     // get profiles that was marked invalid during last update
    QStringList invalidInvestProfiles = profileNamesGroup.readEntry(QLatin1String("Invalid") + m_profileConfPrefix.value(Profile::Investment), QStringList());
    QString bankPrefix = m_profileConfPrefix.value(Profile::Banking) + QLatin1Char('-');
    QString investPrefix = m_profileConfPrefix.value(Profile::Investment) + QLatin1Char('-');

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

    int invalidProfileResponse = QDialogButtonBox::No;

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
                oldBankProfile.writeEntry(m_colTypeConfName.value(Column::Type), oldBankProfile.readEntry("PayeeCol"));
                oldBankProfile.deleteEntry("PayeeCol");
                oldBankProfile.deleteEntry("Filter");
                oldBankProfile.deleteEntry("SecurityName");

                lastUsedDirectory = oldBankProfile.readEntry("InvDirectory");
                newProfile = KConfigGroup(config, m_profileConfPrefix.value(Profile::Investment) + QLatin1Char('-') + *profileName);
                investProfiles.append(*profileName);
                profileName = bankProfiles.erase(profileName);
            } else if (oldProfileType == QLatin1String("Banking")) {
                lastUsedDirectory = oldBankProfile.readEntry("CsvDirectory");
                newProfile = KConfigGroup(config, m_profileConfPrefix.value(Profile::Banking) + QLatin1Char('-') + *profileName);
                ++profileName;
            } else {
                if (invalidProfileResponse != QDialogButtonBox::YesToAll && invalidProfileResponse != QDialogButtonBox::NoToAll) {
                    if (!firstTry &&
                            !invalidBankProfiles.contains(*profileName)) { // if it isn't first update run and profile isn't on the list of invalid ones then don't bother
                        ++profileName;
                        continue;
                    }
                    invalidProfileResponse = KMessageBox::createKMessageBox(nullptr,
                                             new QDialogButtonBox(QDialogButtonBox::Yes | QDialogButtonBox::YesToAll |
                                                     QDialogButtonBox::No | QDialogButtonBox::NoToAll),
                                             QMessageBox::Warning,
                                             i18n("<center>During update of <b>%1</b><br>"
                                                  "the profile type for <b>%2</b> could not be recognized.<br>"
                                                  "The profile cannot be used because of that.<br>"
                                                  "Do you want to delete it?</center>",
                                                  configFilePath, *profileName),
                                             QStringList(), QString(), nullptr, KMessageBox::Dangerous);
                }
                switch (invalidProfileResponse) {
                case QDialogButtonBox::YesToAll:
                case QDialogButtonBox::Yes:
                    oldBankProfile.deleteGroup();
                    invalidBankProfiles.removeOne(*profileName);
                    profileName = bankProfiles.erase(profileName);
                    break;
                case QDialogButtonBox::NoToAll:
                case QDialogButtonBox::No:
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

    profileNamesGroup.writeEntry(m_profileConfPrefix.value(Profile::Banking), bankProfiles); // update profile names as some of them might have been changed
    profileNamesGroup.writeEntry(m_profileConfPrefix.value(Profile::Investment), investProfiles);

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

bool CSVImporterCore::profilesAction(const Profile type, const ProfileAction action, const QString &name, const QString &newname)
{
    bool ret = false;
    const KSharedConfigPtr config = configFile();
    KConfigGroup profileNamesGroup(config, m_confProfileNames);
    QString profileTypeStr = m_profileConfPrefix.value(type);
    QStringList profiles = profileNamesGroup.readEntry(profileTypeStr, QStringList());

    KConfigGroup profileName(config, profileTypeStr + QLatin1Char('-') + name);
    switch (action) {
    case ProfileAction::UpdateLastUsed:
        profileNamesGroup.writeEntry(m_confPriorName + profileTypeStr, profiles.indexOf(name));
        break;
    case ProfileAction::Add:
        if (!profiles.contains(newname)) {
            profiles.append(newname);
            ret = true;
        }
        break;
    case ProfileAction::Remove:
    {
        profiles.removeOne(name);
        profileName.deleteGroup();
        profileName.sync();
        ret = true;
        break;
    }
    case ProfileAction::Rename:
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

bool CSVImporterCore::validateDateFormat(const int col)
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

bool CSVImporterCore::validateDecimalSymbols(const QList<int> &columns)
{
    bool isOK = true;
    Q_FOREACH (const auto column, columns) {
        m_file->m_parse->setDecimalSymbol(m_decimalSymbolIndexMap.value(column));

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

bool CSVImporterCore::validateCurrencies(const PricesProfile *profile)
{
    if (profile->m_securitySymbol.isEmpty() ||
            profile->m_currencySymbol.isEmpty())
        return false;
    return true;
}

bool CSVImporterCore::validateSecurity(const PricesProfile *profile)
{
    if (profile->m_securitySymbol.isEmpty() ||
            profile->m_securityName.isEmpty())
        return false;
    return true;
}

bool CSVImporterCore::validateSecurity(const InvestmentProfile *profile)
{
    if (profile->m_securitySymbol.isEmpty() ||
            profile->m_securityName.isEmpty())
        return false;
    return true;
}

bool CSVImporterCore::validateSecurities()
{
    QSet<QString> onlySymbols;
    QSet<QString> onlyNames;
    sortSecurities(onlySymbols, onlyNames, m_mapSymbolName);

    if (!onlySymbols.isEmpty() || !onlyNames.isEmpty())
        return false;
    return true;
}

eMyMoney::Transaction::Action CSVImporterCore::processActionTypeField(const InvestmentProfile *profile, const int row, const int col)
{
    if (col == -1)
        return eMyMoney::Transaction::Action::None;

    QString type = m_file->m_model->item(row, col)->text();
    QList<eMyMoney::Transaction::Action> actions;
    actions << eMyMoney::Transaction::Action::Buy << eMyMoney::Transaction::Action::Sell <<                       // first and second most frequent action
            eMyMoney::Transaction::Action::ReinvestDividend << eMyMoney::Transaction::Action::CashDividend <<  // we don't want "reinv-dividend" to be accidentally caught by "dividend"
            eMyMoney::Transaction::Action::Interest <<
            eMyMoney::Transaction::Action::Shrsin << eMyMoney::Transaction::Action::Shrsout;

    Q_FOREACH (const auto action, actions) {
        if (profile->m_transactionNames.value(action).contains(type, Qt::CaseInsensitive))
            return action;
    }

    return eMyMoney::Transaction::Action::None;
}

validationResultE CSVImporterCore::validateActionType(MyMoneyStatement::Transaction &tr)
{
    validationResultE ret = ValidActionType;
    QList<eMyMoney::Transaction::Action> validActionTypes = createValidActionTypes(tr);
    if (validActionTypes.isEmpty())
        ret = InvalidActionValues;
    else if (!validActionTypes.contains(tr.m_eAction))
        ret = NoActionType;
    return ret;
}

bool CSVImporterCore::calculateFee()
{
    auto profile = dynamic_cast<InvestmentProfile *>(m_profile);
    if (!profile)
        return false;
    if ((profile->m_feeRate.isEmpty() ||                  // check whether feeRate...
            profile->m_colTypeNum.value(Column::Amount) == -1)) // ...and amount is in place
        return false;

    QString decimalSymbol;
    if (profile->m_decimalSymbol == DecimalSymbol::Auto) {
        DecimalSymbol detectedSymbol = detectDecimalSymbol(profile->m_colTypeNum.value(Column::Amount), QString());
        if (detectedSymbol == DecimalSymbol::Auto)
            return false;
        m_file->m_parse->setDecimalSymbol(detectedSymbol);
        decimalSymbol = m_file->m_parse->decimalSymbol(detectedSymbol);
    } else
        decimalSymbol = m_file->m_parse->decimalSymbol(profile->m_decimalSymbol);


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
        numbers = txt = m_file->m_model->item(row, profile->m_colTypeNum.value(Column::Amount))->text();
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
    int col = profile->m_colTypeNum.value(Column::Fee, -1);
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
    profile->m_colTypeNum[Column::Fee] = m_file->m_columnCount - 1;
    return true;
}

DecimalSymbol CSVImporterCore::detectDecimalSymbol(const int col, const QString &exclude)
{
    DecimalSymbol detectedSymbol = DecimalSymbol::Auto;
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
            if (dotPos > commaPos && commaIsDecimalSeparator == false)    // following case 1,234.56
                dotIsDecimalSeparator = true;
            else if (dotPos < commaPos && dotIsDecimalSeparator == false) // following case 1.234,56
                commaIsDecimalSeparator = true;
            else                                                          // following case 1.234,56 and somewhere earlier there was 1,234.56 so unresolvable conflict
                return detectedSymbol;
        } else if (dotPos != -1) {                 // following case 1.23
            if (dotIsDecimalSeparator)               // it's already know that dotIsDecimalSeparator
                continue;
            if (!commaIsDecimalSeparator)            // if there is no conflict with comma as decimal separator
                dotIsDecimalSeparator = true;
            else {
                if (txt.count(QLatin1Char('.')) > 1)                // following case 1.234.567 so OK
                    continue;
                else if (txt.length() - 4 == dotPos)   // following case 1.234 and somewhere earlier there was 1.234,56 so OK
                    continue;
                else                                   // following case 1.23 and somewhere earlier there was 1,23 so unresolvable conflict
                    return detectedSymbol;
            }
        } else if (commaPos != -1) {               // following case 1,23
            if (commaIsDecimalSeparator)             // it's already know that commaIsDecimalSeparator
                continue;
            else if (!dotIsDecimalSeparator)         // if there is no conflict with dot as decimal separator
                commaIsDecimalSeparator = true;
            else {
                if (txt.count(QLatin1Char(',')) > 1)                // following case 1,234,567 so OK
                    continue;
                else if (txt.length() - 4 == commaPos) // following case 1,234 and somewhere earlier there was 1,234.56 so OK
                    continue;
                else                                   // following case 1,23 and somewhere earlier there was 1.23 so unresolvable conflict
                    return detectedSymbol;
            }

        } else {                                   // following case 123
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
        detectedSymbol = DecimalSymbol::Dot;
    else if (commaIsDecimalSeparator)
        detectedSymbol = DecimalSymbol::Comma;
    else {  // whole column was empty, but we don't want to fail so take OS's decimal symbol
        if (QLocale().decimalPoint() == QLatin1Char('.'))
            detectedSymbol = DecimalSymbol::Dot;
        else
            detectedSymbol = DecimalSymbol::Comma;
    }
    return detectedSymbol;
}

int CSVImporterCore::detectDecimalSymbols(const QList<int> &columns)
{
    int ret = -2;

    // get list of used currencies to remove them from col
    QList<MyMoneyAccount> accounts;
    MyMoneyFile *file = MyMoneyFile::instance();
    file->accountList(accounts);

    QList<eMyMoney::Account::Type> accountTypes{
        eMyMoney::Account::Type::Checkings,
        eMyMoney::Account::Type::Savings,
        eMyMoney::Account::Type::Liability,
        eMyMoney::Account::Type::Checkings,
        eMyMoney::Account::Type::Savings,
        eMyMoney::Account::Type::Cash,
        eMyMoney::Account::Type::CreditCard,
        eMyMoney::Account::Type::Loan,
        eMyMoney::Account::Type::Asset,
        eMyMoney::Account::Type::Liability,
    };

    QSet<QString> currencySymbols;
    Q_FOREACH (const auto account, accounts) {
        if (accountTypes.contains(account.accountType())) {                             // account must actually have currency property
            currencySymbols.insert(account.currencyId());                                 // add currency id
            currencySymbols.insert(file->currency(account.currencyId()).tradingSymbol()); // add currency symbol
        }
    }
    QString filteredCurrencies = QStringList(currencySymbols.values()).join("");
    QString pattern = QString::fromLatin1("%1%2").arg(QLocale().currencySymbol()).arg(filteredCurrencies);

    Q_FOREACH (const auto column, columns) {
        DecimalSymbol detectedSymbol = detectDecimalSymbol(column, pattern);
        if (detectedSymbol == DecimalSymbol::Auto) {
            ret = column;
            return ret;
        }
        m_decimalSymbolIndexMap.insert(column, detectedSymbol);
    }
    return ret;
}

QList<MyMoneyAccount> CSVImporterCore::findAccounts(const QList<eMyMoney::Account::Type> &accountTypes, const QString &statementHeader)
{
    QList<MyMoneyAccount> accountList;
    QList<MyMoneyAccount> filteredTypes;
    QMultiMap<int, MyMoneyAccount> accountsMatchingNumber;
    QMultiMap<int, MyMoneyAccount> accountsMatchingName;
    QMultiMap<int, MyMoneyAccount> accountsMatchingBoth;
    QList<MyMoneyAccount> accountsMatching;
    QRegularExpression filterOutChars(QStringLiteral("[-., ]"));

    MyMoneyFile* file = MyMoneyFile::instance();
    file->accountList(accountList);

    Q_FOREACH (const auto account, accountList) {
        if (accountTypes.contains(account.accountType()) && !(account).isClosed())
            filteredTypes.append(account);
    }

    // filter out accounts with matching numbers, names, and both
    Q_FOREACH (const auto account, filteredTypes) {
        bool matchedNumber = false;
        bool matchedName = false;

        auto number = account.number();
        number.remove(filterOutChars);
        if (number.length() > 2 && statementHeader.contains(number, Qt::CaseInsensitive)) {
            accountsMatchingNumber.insert(number.length(), account);
            matchedNumber = true;
        }

        auto name = account.name();
        name.remove(filterOutChars);
        if (name.length() > 2 && statementHeader.contains(name, Qt::CaseInsensitive)) {
            accountsMatchingName.insert(name.length(), account);
            matchedName = true;
        }

        if (matchedNumber && matchedName)
            accountsMatchingBoth.insert(number.length() + name.length(), account);
    }

    // the logic is:
    // if a single account matching number is found, return that
    if (accountsMatchingNumber.count() == 1)
        return accountsMatchingNumber.values();
    // if more than one account matching number is found
    else if (accountsMatchingNumber.count() > 1) {
        // see if there are any accounts that matched both number and name
        // if one matching both was found, return it
        if (accountsMatchingBoth.count() == 1)
            return accountsMatchingBoth.values();
        // if more than one was found still, return ones that have the longest sum(name, account)
        else if (accountsMatchingBoth.count() > 1)
            accountsMatching = accountsMatchingBoth.values(accountsMatchingBoth.lastKey());
        else
            // if neither account matched both name and number, return the ones that have the longest number
            accountsMatching = accountsMatchingNumber.values(accountsMatchingNumber.lastKey());
    }
    // if none of the accounts matched a number, try with accounts that matched a name
    else {
        // if one account matched name, return it
        if (accountsMatchingName.count() == 1)
            return accountsMatchingName.values();
        // if more than one account matched name, return the ones that have the longest name
        else if (accountsMatchingName.count() > 1)
            accountsMatching = accountsMatchingName.values(accountsMatchingName.lastKey());
    }

    // otherwise, return accountsMatching, which may be empty if none of the accounts matched
    return accountsMatching;
}

bool CSVImporterCore::detectAccount(MyMoneyStatement &st)
{
    QString statementHeader;
    for (int row = 0; row < m_profile->m_startLine; ++row) // concatenate header for better search
        for (int col = 0; col < m_file->m_columnCount; ++col)
            statementHeader.append(m_file->m_model->item(row, col)->text());

    statementHeader.remove(QRegularExpression(QStringLiteral("[-., ]")));

    QList<MyMoneyAccount> accounts;
    QList<eMyMoney::Account::Type> accountTypes;

    switch(m_profile->type()) {
    default:
    case Profile::Banking:
        accountTypes << QList<eMyMoney::Account::Type>{
            eMyMoney::Account::Type::Checkings,
            eMyMoney::Account::Type::Savings,
            eMyMoney::Account::Type::Liability,
            eMyMoney::Account::Type::Checkings,
            eMyMoney::Account::Type::Savings,
            eMyMoney::Account::Type::Cash,
            eMyMoney::Account::Type::CreditCard,
            eMyMoney::Account::Type::Loan,
            eMyMoney::Account::Type::Asset,
            eMyMoney::Account::Type::Liability,
        };
        accounts = findAccounts(accountTypes, statementHeader);
        break;
    case Profile::Investment:
        accountTypes << eMyMoney::Account::Type::Investment; // take investment accounts...
        accounts = findAccounts(accountTypes, statementHeader); //...and search them in statement header
        break;
    }

    if (accounts.count() == 1) { // set account in statement, if it was the only one match
        st.m_strAccountName = accounts.first().name();
        st.m_strAccountNumber = accounts.first().number();
        st.m_accountId = accounts.first().id();

        switch (accounts.first().accountType()) {
        case eMyMoney::Account::Type::Checkings:
            st.m_eType = eMyMoney::Statement::Type::Checkings;
            break;
        case eMyMoney::Account::Type::Savings:
            st.m_eType = eMyMoney::Statement::Type::Savings;
            break;
        case eMyMoney::Account::Type::Investment:
            st.m_eType = eMyMoney::Statement::Type::Investment;
            break;
        case eMyMoney::Account::Type::CreditCard:
            st.m_eType = eMyMoney::Statement::Type::CreditCard;
            break;
        default:
            st.m_eType = eMyMoney::Statement::Type::None;
        }
        return true;
    }
    return false;
}

bool CSVImporterCore::processBankRow(MyMoneyStatement &st, const BankingProfile *profile, const int row)
{
    MyMoneyStatement::Transaction tr;
    QString memo;
    QString txt;

    if (!profile)
        return false;

    // process date field
    int col = profile->m_colTypeNum.value(Column::Date, -1);
    tr.m_datePosted = processDateField(row, col);
    if (tr.m_datePosted == QDate())
        return false;

    // process number field
    col = profile->m_colTypeNum.value(Column::Number, -1);
    if (col != -1)
        tr.m_strNumber = m_file->m_model->item(row, col)->text();

    // process payee field
    col = profile->m_colTypeNum.value(Column::Payee, -1);
    if (col != -1)
        tr.m_strPayee = m_file->m_model->item(row, col)->text();

    // process memo field
    col = profile->m_colTypeNum.value(Column::Memo, -1);
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
    // remove unnecessary line endings
    while (memo.endsWith(QLatin1Char('\n'))) {
        memo.resize(memo.length()-1);
    }
    tr.m_strMemo = memo;

    // process amount field
    col = profile->m_colTypeNum.value(Column::Amount, -1);
    if (col != -1) {
        tr.m_amount = processAmountField(profile, row, col);
        col = profile->m_colTypeNum.value(Column::CreditDebitIndicator, -1);
        if (col != -1) {
            const auto indicator = m_file->m_model->item(row, col)->text();

            QRegularExpression exp;
            exp.setPatternOptions(QRegularExpression::CaseInsensitiveOption);

            auto pattern = QRegularExpression::wildcardToRegularExpression(profile->m_creditIndicator);
            exp.setPattern(pattern);

            if (exp.match(indicator).hasMatch()) {
                tr.m_amount = tr.m_amount.abs();
            } else {
                exp.setPattern(profile->m_debitIndicator);
                if (exp.match(indicator).hasMatch()) {
                    tr.m_amount = -(tr.m_amount.abs());
                }
            }
        } else {
            if (profile->m_oppositeSigns) // change signs to opposite if requested by user
                tr.m_amount = -tr.m_amount;
        }
    }
    // process credit/debit field
    if (profile->m_colTypeNum.value(Column::Credit, -1) != -1 &&
            profile->m_colTypeNum.value(Column::Debit, -1) != -1) {
        QString credit = m_file->m_model->item(row, profile->m_colTypeNum.value(Column::Credit))->text();
        QString debit = m_file->m_model->item(row, profile->m_colTypeNum.value(Column::Debit))->text();
        tr.m_amount = processCreditDebit(credit, debit);
        if (!credit.isEmpty() && !debit.isEmpty())
            return false;
    }

    MyMoneyStatement::Split s1;
    s1.m_amount = tr.m_amount;
    s1.m_strMemo = tr.m_strMemo;
    MyMoneyStatement::Split s2 = s1;
    s2.m_reconcile = tr.m_reconcile;
    s2.m_amount = -s1.m_amount;

    // process category field
    col = profile->m_colTypeNum.value(Column::Category, -1);
    if (col != -1) {
        txt = m_file->m_model->item(row, col)->text();
        QString accountId = MyMoneyFile::instance()->checkCategory(txt, s1.m_amount, s2.m_amount);

        if (!accountId.isEmpty()) {
            s2.m_accountId = accountId;
            s2.m_strCategoryName = txt;
            tr.m_listSplits.append(s2);
        }
    }

    // process balance field
    col = profile->m_colTypeNum.value(Column::Balance, -1);
    if (col != -1) {
        // prior date than the one we have? Adjust it
        if (!st.m_dateBegin.isValid() || st.m_dateBegin > tr.m_datePosted) {
            st.m_dateBegin = tr.m_datePosted;
        }
        // later or equal date, adjust it and the closing balance
        if (!st.m_dateEnd.isValid() || st.m_dateEnd <= tr.m_datePosted) {
            st.m_dateEnd = tr.m_datePosted;
            st.m_closingBalance = processAmountField(profile, row, col);
        }
    }

    // calculate hash
    txt.clear();
    for (int i = 0; i < m_file->m_columnCount; ++i)
        txt.append(m_file->m_model->item(row, i)->text());
    QString hashBase = QString::fromLatin1("%1-%2")
                       .arg(tr.m_datePosted.toString(Qt::ISODate))
                       .arg(MyMoneyTransaction::hash(txt));
    QString hash;
    for (uchar idx = 0; idx < 0xFF; ++idx) {  // assuming there will be no more than 256 transactions with the same hashBase
        hash = QString::fromLatin1("%1-%2").arg(hashBase).arg(idx);
        QSet<QString>::const_iterator it = m_hashSet.constFind(hash);
        if (it == m_hashSet.constEnd())
            break;
    }
    m_hashSet.insert(hash);
    tr.m_strBankID = hash;

    st.m_listTransactions.append(tr); // Add the MyMoneyStatement::Transaction to the statement
    return true;
}

bool CSVImporterCore::processInvestRow(MyMoneyStatement &st, const InvestmentProfile *profile, const int row)
{
    MyMoneyStatement::Transaction tr;

    if (!profile)
        return false;

    QString memo;
    QString txt;
    // process date field
    int col = profile->m_colTypeNum.value(Column::Date, -1);
    tr.m_datePosted = processDateField(row, col);
    if (tr.m_datePosted == QDate())
        return false;

    // process quantity field
    col = profile->m_colTypeNum.value(Column::Quantity, -1);
    tr.m_shares = processQuantityField(profile, row, col);

    // process price field
    col = profile->m_colTypeNum.value(Column::Price, -1);
    tr.m_price = processPriceField(profile, row, col);

    // process amount field
    col = profile->m_colTypeNum.value(Column::Amount, -1);
    tr.m_amount = processAmountField(profile, row, col);

    // process type field
    col = profile->m_colTypeNum.value(Column::Type, -1);
    tr.m_eAction = processActionTypeField(profile, row, col);
    if (!m_isActionTypeValidated && col != -1 &&   // if action type wasn't validated in wizard then...
            validateActionType(tr) != ValidActionType) // ...check if price, amount, quantity is appropriate
        return false;

    // process fee field
    col = profile->m_colTypeNum.value(Column::Fee, -1);
    if (col != -1) {
        if (profile->m_decimalSymbol == DecimalSymbol::Auto) {
            DecimalSymbol decimalSymbol = m_decimalSymbolIndexMap.value(col);
            m_file->m_parse->setDecimalSymbol(decimalSymbol);
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
    col = profile->m_colTypeNum.value(Column::Symbol, -1);
    if (col != -1)
        tr.m_strSymbol = m_file->m_model->item(row, col)->text();
    col = profile->m_colTypeNum.value(Column::Name, -1);
    if (col != -1 &&
            tr.m_strSymbol.isEmpty()) { // case in which symbol field is empty
        txt = m_file->m_model->item(row, col)->text();
        tr.m_strSymbol = m_mapSymbolName.key(txt);   // it's all about getting the right symbol
    } else if (!profile->m_securitySymbol.isEmpty())
        tr.m_strSymbol = profile->m_securitySymbol;
    else if (tr.m_strSymbol.isEmpty())
        return false;
    tr.m_strSecurity = m_mapSymbolName.value(tr.m_strSymbol); // take name from prepared names to avoid potential name mismatch

    // process memo field
    col = profile->m_colTypeNum.value(Column::Memo, -1);
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
    // remove unnecessary line endings
    while (memo.endsWith(QLatin1Char('\n'))) {
        memo.resize(memo.length()-1);
    }
    tr.m_strMemo = memo;

    tr.m_strInterestCategory.clear(); // no special category
    tr.m_strBrokerageAccount.clear(); // no brokerage account auto-detection

    MyMoneyStatement::Split s1;
    s1.m_amount = tr.m_amount;
    s1.m_strMemo = tr.m_strMemo;
    MyMoneyStatement::Split s2 = s1;
    s2.m_amount = -s1.m_amount;
    s2.m_accountId = MyMoneyFile::instance()->checkCategory(tr.m_strInterestCategory, s1.m_amount, s2.m_amount);

    // deduct fees from amount
    if (tr.m_eAction == eMyMoney::Transaction::Action::CashDividend ||
            tr.m_eAction == eMyMoney::Transaction::Action::Sell ||
            tr.m_eAction == eMyMoney::Transaction::Action::Interest)
        tr.m_amount -= tr.m_fees;

    else if (tr.m_eAction == eMyMoney::Transaction::Action::Buy) {
        if (tr.m_amount.isPositive())
            tr.m_amount = -tr.m_amount; //if broker doesn't use minus sings for buy transactions, set it manually here
        tr.m_amount -= tr.m_fees;
    } else if (tr.m_eAction == eMyMoney::Transaction::Action::None)
        tr.m_listSplits.append(s2);

    st.m_listTransactions.append(tr); // Add the MyMoneyStatement::Transaction to the statement
    return true;
}

bool CSVImporterCore::processPriceRow(MyMoneyStatement &st, const PricesProfile *profile, const int row)
{
    MyMoneyStatement::Price pr;

    if (!profile)
        return false;

    // process date field
    int col = profile->m_colTypeNum.value(Column::Date, -1);
    pr.m_date = processDateField(row, col);
    if (pr.m_date == QDate())
        return false;

    // process price field
    col = profile->m_colTypeNum.value(Column::Price, -1);
    pr.m_amount = processPriceField(profile, row, col);

    switch (profile->type()) {
    case Profile::CurrencyPrices:
        if (profile->m_securitySymbol.isEmpty() || profile->m_currencySymbol.isEmpty())
            return false;
        pr.m_strSecurity = profile->m_securitySymbol;
        pr.m_strCurrency = profile->m_currencySymbol;
        break;
    case Profile::StockPrices:
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

QDate CSVImporterCore::processDateField(const int row, const int col)
{
    QDate date;
    if (col != -1) {
        QString txt = m_file->m_model->item(row, col)->text();
        date = m_convertDate->convertDate(txt);      //  Date column
    }
    return date;
}

MyMoneyMoney CSVImporterCore::processCreditDebit(QString &credit, QString &debit)
{
    MyMoneyMoney amount;
    if (m_profile->m_decimalSymbol == DecimalSymbol::Auto)
        setupFieldDecimalSymbol(m_profile->m_colTypeNum.value(Column::Credit));

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


MyMoneyMoney CSVImporterCore::processQuantityField(const CSVProfile *profile, const int row, const int col)
{
    MyMoneyMoney shares;
    if (col != -1) {
        if (profile->m_decimalSymbol == DecimalSymbol::Auto)
            setupFieldDecimalSymbol(col);

        QString txt = m_file->m_model->item(row, col)->text();
        txt.remove(QRegularExpression(QStringLiteral("-+"))); // remove unwanted sings in quantity

        if (!txt.isEmpty())
            shares = MyMoneyMoney(m_file->m_parse->possiblyReplaceSymbol(txt));
    }
    return shares;
}

MyMoneyMoney CSVImporterCore::processAmountField(const CSVProfile *profile, const int row, const int col)
{
    MyMoneyMoney amount;
    if (col != -1) {
        if (profile->m_decimalSymbol == DecimalSymbol::Auto)
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

MyMoneyMoney CSVImporterCore::processPriceField(const InvestmentProfile *profile, const int row, const int col)
{
    MyMoneyMoney price;
    if (col != -1) {
        if (profile->m_decimalSymbol == DecimalSymbol::Auto)
            setupFieldDecimalSymbol(col);

        QString txt = m_file->m_model->item(row, col)->text();
        if (!txt.isEmpty()) {
            price = MyMoneyMoney(m_file->m_parse->possiblyReplaceSymbol(txt));
            price *= m_priceFractions.at(profile->m_priceFraction);
        }
    }
    return price;
}

MyMoneyMoney CSVImporterCore::processPriceField(const PricesProfile *profile, const int row, const int col)
{
    MyMoneyMoney price;
    if (col != -1) {
        if (profile->m_decimalSymbol == DecimalSymbol::Auto)
            setupFieldDecimalSymbol(col);

        QString txt = m_file->m_model->item(row, col)->text();
        if (!txt.isEmpty()) {
            price = MyMoneyMoney(m_file->m_parse->possiblyReplaceSymbol(txt));
            price *= m_priceFractions.at(profile->m_priceFraction);
        }
    }
    return price;
}


QList<eMyMoney::Transaction::Action> CSVImporterCore::createValidActionTypes(MyMoneyStatement::Transaction &tr)
{
    QList<eMyMoney::Transaction::Action> validActionTypes;
    if (tr.m_shares.isPositive() &&
            tr.m_price.isPositive() &&
            !tr.m_amount.isZero())
        validActionTypes << eMyMoney::Transaction::Action::ReinvestDividend <<
                         eMyMoney::Transaction::Action::Buy <<
                         eMyMoney::Transaction::Action::Sell;
    else if (tr.m_shares.isZero() &&
             tr.m_price.isZero() &&
             !tr.m_amount.isZero())
        validActionTypes << eMyMoney::Transaction::Action::CashDividend <<
                         eMyMoney::Transaction::Action::Interest;
    else if (tr.m_shares.isPositive() &&
             tr.m_price.isZero() &&
             tr.m_amount.isZero())
        validActionTypes << eMyMoney::Transaction::Action::Shrsin <<
                         eMyMoney::Transaction::Action::Shrsout;
    return validActionTypes;
}


bool CSVImporterCore::sortSecurities(QSet<QString>& onlySymbols, QSet<QString>& onlyNames, QMap<QString, QString>& mapSymbolName)
{
    QList<MyMoneySecurity> securityList = MyMoneyFile::instance()->securityList();
    int symbolCol = m_profile->m_colTypeNum.value(Column::Symbol, -1);
    int nameCol = m_profile->m_colTypeNum.value(Column::Name, -1);

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
        Q_FOREACH (const auto sec, securityList) {
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
        Q_FOREACH (const auto sec, securityList) {
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

void CSVImporterCore::setupFieldDecimalSymbol(int col) {
    m_file->m_parse->setDecimalSymbol(m_decimalSymbolIndexMap.value(col));
}

QList<int> CSVImporterCore::getNumericalColumns()
{
    QList<int> columns;
    switch(m_profile->type()) {
    case Profile::Banking:
        if (m_profile->m_colTypeNum.value(Column::Amount, -1) != -1) {
            columns << m_profile->m_colTypeNum.value(Column::Amount);
        } else {
            columns << m_profile->m_colTypeNum.value(Column::Debit);
            columns << m_profile->m_colTypeNum.value(Column::Credit);
        }
        break;
    case Profile::Investment:
        columns << m_profile->m_colTypeNum.value(Column::Amount);
        columns << m_profile->m_colTypeNum.value(Column::Price);
        columns << m_profile->m_colTypeNum.value(Column::Quantity);
        if (m_profile->m_colTypeNum.value(Column::Fee, -1) != -1)
            columns << m_profile->m_colTypeNum.value(Column::Fee);
        break;
    case Profile::CurrencyPrices:
    case Profile::StockPrices:
        columns << m_profile->m_colTypeNum.value(Column::Price);
        break;
    default:
        break;
    }
    return columns;
}

bool CSVImporterCore::createStatement(MyMoneyStatement &st)
{
    switch (m_profile->type()) {
    case Profile::Banking:
    {
        if (!st.m_listTransactions.isEmpty()) // don't create statement if there is one
            return true;
        st.m_eType = eMyMoney::Statement::Type::None;

        m_hashSet.clear();
        BankingProfile *profile = dynamic_cast<BankingProfile *>(m_profile);
        if (profile->m_autoAccountName)
            detectAccount(st);

        for (int row = m_profile->m_startLine; row <= m_profile->m_endLine; ++row)
            if (!processBankRow(st, profile, row)) { // parse fields
                st = MyMoneyStatement();
                return false;
            }
        return true;
        break;
    }
    case Profile::Investment:
    {
        if (!st.m_listTransactions.isEmpty()) // don't create statement if there is one
            return true;
        st.m_eType = eMyMoney::Statement::Type::Investment;

        auto profile = dynamic_cast<InvestmentProfile *>(m_profile);
        if (profile->m_autoAccountName)
            detectAccount(st);

        if ((m_profile->m_colTypeNum.value(Column::Fee, -1) == -1 ||
                m_profile->m_colTypeNum.value(Column::Fee, -1) >= m_file->m_columnCount) &&
                profile && !profile->m_feeRate.isEmpty()) // fee column has not been calculated so do it now
            calculateFee();

        if (profile) {
            for (int row = m_profile->m_startLine; row <= m_profile->m_endLine; ++row)
                if (!processInvestRow(st, profile, row)) { // parse fields
                    st = MyMoneyStatement();
                    return false;
                }
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
    case Profile::CurrencyPrices:
    case Profile::StockPrices:
    {
        if (!st.m_listPrices.isEmpty()) // don't create statement if there is one
            return true;
        st.m_eType = eMyMoney::Statement::Type::None;

        if (auto profile = dynamic_cast<PricesProfile *>(m_profile)) {
            for (int row = m_profile->m_startLine; row <= m_profile->m_endLine; ++row)
                if (!processPriceRow(st, profile, row)) { // parse fields
                    st = MyMoneyStatement();
                    return false;
                }
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
    m_lastUsedDirectory = profilesGroup.readEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfDirectory), QString());
    m_startLine = profilesGroup.readEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfStartLine), 0);
    m_trailerLines = profilesGroup.readEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfTrailerLines), 0);
    m_encodingMIBEnum = profilesGroup.readEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfEncoding), 106 /* UTF-8 */);

    m_dateFormat = static_cast<DateFormat>(profilesGroup.readEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfDateFormat), (int)DateFormat::YearMonthDay));
    m_textDelimiter = static_cast<TextDelimiter>(profilesGroup.readEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfTextDelimiter), (int)TextDelimiter::DoubleQuote));
    m_fieldDelimiter = static_cast<FieldDelimiter>(profilesGroup.readEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfFieldDelimiter), (int)FieldDelimiter::Auto));
    m_decimalSymbol = static_cast<DecimalSymbol>(profilesGroup.readEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfDecimalSymbol), (int)DecimalSymbol::Auto));
    initColNumType();
}

void CSVProfile::writeSettings(KConfigGroup &profilesGroup)
{
    QFileInfo fileInfo (m_lastUsedDirectory);
    if (fileInfo.isFile())
        m_lastUsedDirectory = fileInfo.absolutePath();

    if (m_lastUsedDirectory.startsWith(QDir::homePath())) // replace /home/user with ~/ for brevity
        m_lastUsedDirectory.replace(0, QDir::homePath().length(), QLatin1Char('~'));

    profilesGroup.writeEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfDirectory), m_lastUsedDirectory);
    profilesGroup.writeEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfEncoding), m_encodingMIBEnum);
    profilesGroup.writeEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfDateFormat), (int)m_dateFormat);
    profilesGroup.writeEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfFieldDelimiter), (int)m_fieldDelimiter);
    profilesGroup.writeEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfTextDelimiter), (int)m_textDelimiter);
    profilesGroup.writeEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfDecimalSymbol), (int)m_decimalSymbol);
    profilesGroup.writeEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfStartLine), m_startLine);
    profilesGroup.writeEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfTrailerLines), m_trailerLines);
}

bool BankingProfile::readSettings(const KSharedConfigPtr &config)
{
    bool exists = true;
    KConfigGroup profilesGroup(config, CSVImporterCore::m_profileConfPrefix.value(type()) + QLatin1Char('-') + m_profileName);
    if (!profilesGroup.exists())
        exists = false;

    m_colTypeNum[Column::Payee] = profilesGroup.readEntry(CSVImporterCore::m_colTypeConfName.value(Column::Payee), -1);
    m_colTypeNum[Column::Number] = profilesGroup.readEntry(CSVImporterCore::m_colTypeConfName.value(Column::Number), -1);
    m_colTypeNum[Column::Amount] = profilesGroup.readEntry(CSVImporterCore::m_colTypeConfName.value(Column::Amount), -1);
    m_colTypeNum[Column::Debit] = profilesGroup.readEntry(CSVImporterCore::m_colTypeConfName.value(Column::Debit), -1);
    m_colTypeNum[Column::Credit] = profilesGroup.readEntry(CSVImporterCore::m_colTypeConfName.value(Column::Credit), -1);
    m_colTypeNum[Column::Date] = profilesGroup.readEntry(CSVImporterCore::m_colTypeConfName.value(Column::Date), -1);
    m_colTypeNum[Column::Category] = profilesGroup.readEntry(CSVImporterCore::m_colTypeConfName.value(Column::Category), -1);
    m_colTypeNum[Column::CreditDebitIndicator] = profilesGroup.readEntry(CSVImporterCore::m_colTypeConfName.value(Column::CreditDebitIndicator), -1);
    m_colTypeNum[Column::Balance] = profilesGroup.readEntry(CSVImporterCore::m_colTypeConfName.value(Column::Balance), -1);
    m_colTypeNum[Column::Memo] = -1; // initialize, otherwise random data may go here
    m_oppositeSigns = profilesGroup.readEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfOppositeSigns), false);
    m_creditIndicator = profilesGroup.readEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfCreditIndicator), QString());
    m_debitIndicator = profilesGroup.readEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfDebitIndicator), QString());
    m_autoAccountName = profilesGroup.readEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfAutoAccountName), false);
    m_memoColList = profilesGroup.readEntry(CSVImporterCore::m_colTypeConfName.value(Column::Memo), QList<int>());

    CSVProfile::readSettings(profilesGroup);
    return exists;
}

void BankingProfile::writeSettings(const KSharedConfigPtr &config)
{
    KConfigGroup profilesGroup(config, CSVImporterCore::m_profileConfPrefix.value(type()) + QLatin1Char('-') + m_profileName);
    CSVProfile::writeSettings(profilesGroup);

    profilesGroup.writeEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfOppositeSigns), m_oppositeSigns);
    profilesGroup.writeEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfCreditIndicator), m_creditIndicator);
    profilesGroup.writeEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfDebitIndicator), m_debitIndicator);
    profilesGroup.writeEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfAutoAccountName), m_autoAccountName);
    profilesGroup.writeEntry(CSVImporterCore::m_colTypeConfName.value(Column::Payee),
                             m_colTypeNum.value(Column::Payee));
    profilesGroup.writeEntry(CSVImporterCore::m_colTypeConfName.value(Column::Number),
                             m_colTypeNum.value(Column::Number));
    profilesGroup.writeEntry(CSVImporterCore::m_colTypeConfName.value(Column::Amount),
                             m_colTypeNum.value(Column::Amount));
    profilesGroup.writeEntry(CSVImporterCore::m_colTypeConfName.value(Column::Debit),
                             m_colTypeNum.value(Column::Debit));
    profilesGroup.writeEntry(CSVImporterCore::m_colTypeConfName.value(Column::Credit),
                             m_colTypeNum.value(Column::Credit));
    profilesGroup.writeEntry(CSVImporterCore::m_colTypeConfName.value(Column::Date),
                             m_colTypeNum.value(Column::Date));
    profilesGroup.writeEntry(CSVImporterCore::m_colTypeConfName.value(Column::Category),
                             m_colTypeNum.value(Column::Category));
    profilesGroup.writeEntry(CSVImporterCore::m_colTypeConfName.value(Column::CreditDebitIndicator),
                             m_colTypeNum.value(Column::CreditDebitIndicator));
    profilesGroup.writeEntry(CSVImporterCore::m_colTypeConfName.value(Column::Balance),
                             m_colTypeNum.value(Column::Balance));
    profilesGroup.writeEntry(CSVImporterCore::m_colTypeConfName.value(Column::Memo),
                             m_memoColList);
    profilesGroup.config()->sync();
}

bool InvestmentProfile::readSettings(const KSharedConfigPtr &config)
{
    bool exists = true;
    KConfigGroup profilesGroup(config, CSVImporterCore::m_profileConfPrefix.value(type()) + QLatin1Char('-') + m_profileName);
    if (!profilesGroup.exists())
        exists = false;

    m_transactionNames[eMyMoney::Transaction::Action::Buy] = profilesGroup.readEntry(CSVImporterCore::m_transactionConfName.value(eMyMoney::Transaction::Action::Buy),
            QString(i18nc("Type of operation as in financial statement", "buy")).split(',', Qt::SkipEmptyParts));
    m_transactionNames[eMyMoney::Transaction::Action::Sell] = profilesGroup.readEntry(CSVImporterCore::m_transactionConfName.value(eMyMoney::Transaction::Action::Sell),
            QString(i18nc("Type of operation as in financial statement", "sell,repurchase")).split(',', Qt::SkipEmptyParts));
    m_transactionNames[eMyMoney::Transaction::Action::ReinvestDividend] = profilesGroup.readEntry(CSVImporterCore::m_transactionConfName.value(eMyMoney::Transaction::Action::ReinvestDividend),
            QString(i18nc("Type of operation as in financial statement", "reinvest,reinv,re-inv")).split(',', Qt::SkipEmptyParts));
    m_transactionNames[eMyMoney::Transaction::Action::CashDividend] = profilesGroup.readEntry(CSVImporterCore::m_transactionConfName.value(eMyMoney::Transaction::Action::CashDividend),
            QString(i18nc("Type of operation as in financial statement", "dividend")).split(',', Qt::SkipEmptyParts));
    m_transactionNames[eMyMoney::Transaction::Action::Interest] = profilesGroup.readEntry(CSVImporterCore::m_transactionConfName.value(eMyMoney::Transaction::Action::Interest),
            QString(i18nc("Type of operation as in financial statement", "interest,income")).split(',', Qt::SkipEmptyParts));
    m_transactionNames[eMyMoney::Transaction::Action::Shrsin] = profilesGroup.readEntry(CSVImporterCore::m_transactionConfName.value(eMyMoney::Transaction::Action::Shrsin),
            QString(i18nc("Type of operation as in financial statement", "add,stock dividend,divd reinv,transfer in,re-registration in,journal entry")).split(',', Qt::SkipEmptyParts));
    m_transactionNames[eMyMoney::Transaction::Action::Shrsout] = profilesGroup.readEntry(CSVImporterCore::m_transactionConfName.value(eMyMoney::Transaction::Action::Shrsout),
            QString(i18nc("Type of operation as in financial statement", "remove")).split(',', Qt::SkipEmptyParts));

    m_colTypeNum[Column::Date] = profilesGroup.readEntry(CSVImporterCore::m_colTypeConfName.value(Column::Date), -1);
    m_colTypeNum[Column::Type] = profilesGroup.readEntry(CSVImporterCore::m_colTypeConfName.value(Column::Type), -1);  //use for type col.
    m_colTypeNum[Column::Price] = profilesGroup.readEntry(CSVImporterCore::m_colTypeConfName.value(Column::Price), -1);
    m_colTypeNum[Column::Quantity] = profilesGroup.readEntry(CSVImporterCore::m_colTypeConfName.value(Column::Quantity), -1);
    m_colTypeNum[Column::Amount] = profilesGroup.readEntry(CSVImporterCore::m_colTypeConfName.value(Column::Amount), -1);
    m_colTypeNum[Column::Name] = profilesGroup.readEntry(CSVImporterCore::m_colTypeConfName.value(Column::Name), -1);
    m_colTypeNum[Column::Fee] = profilesGroup.readEntry(CSVImporterCore::m_colTypeConfName.value(Column::Fee), -1);
    m_colTypeNum[Column::Symbol] = profilesGroup.readEntry(CSVImporterCore::m_colTypeConfName.value(Column::Symbol), -1);
    m_colTypeNum[Column::Memo] = -1; // initialize, otherwise random data may go here
    m_feeIsPercentage = profilesGroup.readEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfFeeIsPercentage), false);
    m_feeRate = profilesGroup.readEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfFeeRate), QString());
    m_minFee = profilesGroup.readEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfMinFee), QString());

    m_memoColList = profilesGroup.readEntry(CSVImporterCore::m_colTypeConfName.value(Column::Memo), QList<int>());
    m_securityName = profilesGroup.readEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfSecurityName), QString());
    m_securitySymbol = profilesGroup.readEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfSecuritySymbol), QString());
    m_dontAsk = profilesGroup.readEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfDontAsk), 0);
    m_priceFraction = profilesGroup.readEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfPriceFraction), 2);

    CSVProfile::readSettings(profilesGroup);
    return exists;
}

void InvestmentProfile::writeSettings(const KSharedConfigPtr &config)
{
    KConfigGroup profilesGroup(config, CSVImporterCore::m_profileConfPrefix.value(type()) + QLatin1Char('-') + m_profileName);
    CSVProfile::writeSettings(profilesGroup);

    profilesGroup.writeEntry(CSVImporterCore::m_transactionConfName.value(eMyMoney::Transaction::Action::Buy),
                             m_transactionNames.value(eMyMoney::Transaction::Action::Buy));
    profilesGroup.writeEntry(CSVImporterCore::m_transactionConfName.value(eMyMoney::Transaction::Action::Sell),
                             m_transactionNames.value(eMyMoney::Transaction::Action::Sell));
    profilesGroup.writeEntry(CSVImporterCore::m_transactionConfName.value(eMyMoney::Transaction::Action::ReinvestDividend),
                             m_transactionNames.value(eMyMoney::Transaction::Action::ReinvestDividend));
    profilesGroup.writeEntry(CSVImporterCore::m_transactionConfName.value(eMyMoney::Transaction::Action::CashDividend),
                             m_transactionNames.value(eMyMoney::Transaction::Action::CashDividend));
    profilesGroup.writeEntry(CSVImporterCore::m_transactionConfName.value(eMyMoney::Transaction::Action::Interest),
                             m_transactionNames.value(eMyMoney::Transaction::Action::Interest));
    profilesGroup.writeEntry(CSVImporterCore::m_transactionConfName.value(eMyMoney::Transaction::Action::Shrsin),
                             m_transactionNames.value(eMyMoney::Transaction::Action::Shrsin));
    profilesGroup.writeEntry(CSVImporterCore::m_transactionConfName.value(eMyMoney::Transaction::Action::Shrsout),
                             m_transactionNames.value(eMyMoney::Transaction::Action::Shrsout));

    profilesGroup.writeEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfPriceFraction), m_priceFraction);
    profilesGroup.writeEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfFeeIsPercentage), m_feeIsPercentage);
    profilesGroup.writeEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfFeeRate), m_feeRate);
    profilesGroup.writeEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfMinFee), m_minFee);
    profilesGroup.writeEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfSecurityName), m_securityName);
    profilesGroup.writeEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfSecuritySymbol), m_securitySymbol);
    profilesGroup.writeEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfDontAsk), m_dontAsk);
    profilesGroup.writeEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfAutoAccountName), m_autoAccountName);

    profilesGroup.writeEntry(CSVImporterCore::m_colTypeConfName.value(Column::Date),
                             m_colTypeNum.value(Column::Date));
    profilesGroup.writeEntry(CSVImporterCore::m_colTypeConfName.value(Column::Type),
                             m_colTypeNum.value(Column::Type));
    profilesGroup.writeEntry(CSVImporterCore::m_colTypeConfName.value(Column::Quantity),
                             m_colTypeNum.value(Column::Quantity));
    profilesGroup.writeEntry(CSVImporterCore::m_colTypeConfName.value(Column::Amount),
                             m_colTypeNum.value(Column::Amount));
    profilesGroup.writeEntry(CSVImporterCore::m_colTypeConfName.value(Column::Price),
                             m_colTypeNum.value(Column::Price));
    profilesGroup.writeEntry(CSVImporterCore::m_colTypeConfName.value(Column::Symbol),
                             m_colTypeNum.value(Column::Symbol));
    profilesGroup.writeEntry(CSVImporterCore::m_colTypeConfName.value(Column::Name),
                             m_colTypeNum.value(Column::Name));
    profilesGroup.writeEntry(CSVImporterCore::m_colTypeConfName.value(Column::Fee),
                             m_colTypeNum.value(Column::Fee));
    profilesGroup.writeEntry(CSVImporterCore::m_colTypeConfName.value(Column::Memo),
                             m_memoColList);
    profilesGroup.config()->sync();
}

bool PricesProfile::readSettings(const KSharedConfigPtr &config)
{
    bool exists = true;
    KConfigGroup profilesGroup(config, CSVImporterCore::m_profileConfPrefix.value(type()) + QLatin1Char('-') + m_profileName);
    if (!profilesGroup.exists())
        exists = false;

    m_colTypeNum[Column::Date] = profilesGroup.readEntry(CSVImporterCore::m_colTypeConfName.value(Column::Date), -1);
    m_colTypeNum[Column::Price] = profilesGroup.readEntry(CSVImporterCore::m_colTypeConfName.value(Column::Price), -1);
    m_priceFraction = profilesGroup.readEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfPriceFraction), 2);
    m_securityName = profilesGroup.readEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfSecurityName), QString());
    m_securitySymbol = profilesGroup.readEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfSecuritySymbol), QString());
    m_currencySymbol = profilesGroup.readEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfCurrencySymbol), QString());
    m_dontAsk = profilesGroup.readEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfDontAsk), 0);

    CSVProfile::readSettings(profilesGroup);
    return exists;
}

void PricesProfile::writeSettings(const KSharedConfigPtr &config)
{
    KConfigGroup profilesGroup(config, CSVImporterCore::m_profileConfPrefix.value(type()) + QLatin1Char('-') + m_profileName);
    CSVProfile::writeSettings(profilesGroup);

    profilesGroup.writeEntry(CSVImporterCore::m_colTypeConfName.value(Column::Date),
                             m_colTypeNum.value(Column::Date));
    profilesGroup.writeEntry(CSVImporterCore::m_colTypeConfName.value(Column::Price),
                             m_colTypeNum.value(Column::Price));
    profilesGroup.writeEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfPriceFraction), m_priceFraction);
    profilesGroup.writeEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfSecurityName), m_securityName);
    profilesGroup.writeEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfSecuritySymbol), m_securitySymbol);
    profilesGroup.writeEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfCurrencySymbol), m_currencySymbol);
    profilesGroup.writeEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfDontAsk), m_dontAsk);
    profilesGroup.config()->sync();
}

CSVFile::CSVFile() :
    m_columnCount(0),
    m_rowCount(0)
{
    m_parse = new Parse;
    m_model = new QStandardItemModel;
}

CSVFile::~CSVFile()
{
    delete m_parse;
    delete m_model;
}

void CSVFile::getStartEndRow(CSVProfile *profile)
{
    profile->m_endLine = m_rowCount - 1;

    // if trailer lines are specified then remove them here
    if (profile->m_trailerLines)
        profile->m_endLine -= profile->m_trailerLines;

    if (profile->m_startLine > profile->m_endLine)   // Don't allow m_startLine > m_endLine
        profile->m_startLine = profile->m_endLine;

    if (profile->m_startLine < 0) // Don't allow m_startLine negative
        profile->m_startLine = 0;

    if (profile->m_endLine < 0) // Don't allow m_endLine negative
        profile->m_endLine = 0;
}

void CSVFile::getColumnCount(CSVProfile *profile, const QStringList &rows)
{
    if (rows.isEmpty())
        return;

    QVector<FieldDelimiter> delimiterIndexes;
    if (profile->m_fieldDelimiter == FieldDelimiter::Auto)
        delimiterIndexes = QVector<FieldDelimiter> {FieldDelimiter::Comma, FieldDelimiter::Semicolon, FieldDelimiter::Colon, FieldDelimiter::Tab}; // include all delimiters to test or ...
    else
        delimiterIndexes = QVector<FieldDelimiter> {profile->m_fieldDelimiter};  // ... only the one specified

    QList<int> totalDelimiterCount({0, 0, 0, 0}); //  Total in file for each delimiter
    QList<int> thisDelimiterCount({0, 0, 0, 0});  //  Total in this line for each delimiter
    int colCount = 0;                             //  Total delimiters in this line
    FieldDelimiter possibleDelimiter = FieldDelimiter::Comma;
    m_columnCount = 0;

    Q_FOREACH (const auto row, rows) {
        Q_FOREACH(const auto delimiterIndex, delimiterIndexes) {
            m_parse->setFieldDelimiter(delimiterIndex);
            colCount = m_parse->parseLine(row).count(); //  parse each line using each delimiter

            if (colCount > thisDelimiterCount.at((int)delimiterIndex))
                thisDelimiterCount[(int)delimiterIndex] = colCount;

            if (thisDelimiterCount[(int)delimiterIndex] > m_columnCount)
                m_columnCount = thisDelimiterCount.at((int)delimiterIndex);

            totalDelimiterCount[(int)delimiterIndex] += colCount;
            if (totalDelimiterCount.at((int)delimiterIndex) > totalDelimiterCount.at((int)possibleDelimiter))
                possibleDelimiter = delimiterIndex;
        }
    }
    if (delimiterIndexes.count() != 1)                      // if purpose was to autodetect...
        profile->m_fieldDelimiter = possibleDelimiter;        // ... then change field delimiter
    m_parse->setFieldDelimiter(profile->m_fieldDelimiter);  // restore original field delimiter
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
            fileInfo.absoluteFilePath());
    dialog->setMimeTypeFilters({"text/csv", "text/tab-separated-values", "text/plain", "application/octet-stream"});
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
    if (profile->m_decimalSymbol != DecimalSymbol::Auto)
        m_parse->setDecimalSymbol(profile->m_decimalSymbol);
    m_parse->setFieldDelimiter(profile->m_fieldDelimiter);
    m_parse->setTextDelimiter(profile->m_textDelimiter);
}

void CSVFile::readFile(CSVProfile *profile)
{
    QFile inFile(m_inFileName);
    if (!inFile.exists())
        return;
    inFile.open(QIODevice::ReadOnly);
    QTextStream inStream(&inFile);
    QTextCodec* codec = QTextCodec::codecForMib(profile->m_encodingMIBEnum);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    inStream.setCodec(codec);
#endif

    QString buf = inStream.readAll();
    inFile.close();
    m_model->clear();
    m_parse->setTextDelimiter(profile->m_textDelimiter);
    QStringList rows = m_parse->parseFile(buf);        // parse the buffer
    m_rowCount = m_parse->lastLine();                  // won't work without above line
    getColumnCount(profile, rows);
    getStartEndRow(profile);

    // prepare model from rows having rowCount and columnCount,
    // but only if there is at least one row
    if (m_rowCount > 0) {
        for (int i = 0; i < m_rowCount; ++i) {
            QList<QStandardItem*> itemList;
            // use take instead of read from rows to preserve memory
            QStringList columns = m_parse->parseLine(rows.takeFirst());
            for (int j = 0; j < m_columnCount; ++j)
                itemList.append(new QStandardItem(columns.value(j, QString())));
            m_model->appendRow(itemList);
        }
    }
}
