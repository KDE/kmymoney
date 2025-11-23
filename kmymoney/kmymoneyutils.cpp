/*
    SPDX-FileCopyrightText: 2000-2003 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2003 Javier Campos Morales <javi_c@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2003 Felix Rodriguez <frodriguez@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2003 John C <thetacoturtle@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2003 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2003 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kmymoneyutils.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QAbstractButton>
#include <QApplication>
#include <QBitArray>
#include <QFileInfo>
#include <QGroupBox>
#include <QIcon>
#include <QList>
#include <QPainter>
#include <QPixmap>
#include <QPixmapCache>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QTemporaryFile>
#include <QWidget>
#include <QWizard>

// ----------------------------------------------------------------------------
// KDE Headers

#include <KColorScheme>
#include <KGuiItem>
#include <KIO/StatJob>
#include <KIO/StoredTransferJob>
#include <KLazyLocalizedString>
#include <KLocalizedString>
#include <KMessageBox>
#include <KStandardGuiItem>
#include <KXmlGuiWindow>
#include <kio_version.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "accountsmodel.h"
#include "amountedit.h"
#include "creditdebitedit.h"
#include "icons.h"
#include "journalmodel.h"
#include "kmymoneyplugin.h"
#include "kmymoneysettings.h"
#include "mymoneyaccount.h"
#include "mymoneyenums.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneyforecast.h"
#include "mymoneymoney.h"
#include "mymoneypayee.h"
#include "mymoneyprice.h"
#include "mymoneyreport.h"
#include "mymoneyschedule.h"
#include "mymoneysecurity.h"
#include "mymoneysplit.h"
#include "mymoneystatement.h"
#include "mymoneytag.h"
#include "mymoneytransaction.h"
#include "mymoneytransactionfilter.h"
#include "splitmodel.h"
#include "statusmodel.h"
#include "storageenums.h"
#include "widgethintframe.h"

#include "kmmyesno.h"

using namespace Icons;

const QString KMyMoneyUtils::paymentMethodToString(eMyMoney::Schedule::PaymentType paymentType)
{
    return i18n(MyMoneySchedule::paymentMethodToString(paymentType));
}

const QString KMyMoneyUtils::weekendOptionToString(eMyMoney::Schedule::WeekendOption weekendOption)
{
    return i18n(MyMoneySchedule::weekendOptionToString(weekendOption).toLatin1());
}

const QString KMyMoneyUtils::scheduleTypeToString(eMyMoney::Schedule::Type type)
{
    return i18nc("Scheduled transaction type", MyMoneySchedule::scheduleTypeToString(type).toLatin1());
}

KGuiItem KMyMoneyUtils::scheduleNewGuiItem()
{
    KGuiItem splitGuiItem(i18n("&New Schedule..."),
                          Icons::get(Icon::DocumentNew),
                          i18n("Create a new schedule."),
                          i18n("Use this to create a new schedule."));

    return splitGuiItem;
}

KGuiItem KMyMoneyUtils::accountsFilterGuiItem()
{
    KGuiItem splitGuiItem(i18n("&Filter"),
                          Icons::get(Icon::Filter),
                          i18n("Filter out accounts"),
                          i18n("Use this to filter out accounts"));

    return splitGuiItem;
}

const char* homePageItems[] = {
    kli18n("Scheduled payments").untranslatedText(),
    kli18n("Preferred accounts").untranslatedText(),
    kli18n("Payment accounts").untranslatedText(),
    kli18n("Favorite reports").untranslatedText(),
    kli18n("Forecast (schedule)").untranslatedText(),
    kli18n("Net worth forecast").untranslatedText(),
    kli18n("Forecast (history)").untranslatedText(), // unused, s.a. KSettingsHome::slotLoadItems()
    kli18n("Assets and Liabilities").untranslatedText(),
    kli18n("Budget").untranslatedText(),
    kli18n("Cash Flow").untranslatedText(),
    // insert new items above this comment
    nullptr,
};

const QString KMyMoneyUtils::homePageItemToString(const int idx)
{
    QString rc;
    if (abs(idx) > 0 && abs(idx) < static_cast<int>(sizeof(homePageItems) / sizeof(homePageItems[0]))) {
        rc = i18n(homePageItems[abs(idx-1)]);
    }
    return rc;
}

int KMyMoneyUtils::stringToHomePageItem(const QString& txt)
{
    int idx = 0;
    for (idx = 0; homePageItems[idx] != nullptr; ++idx) {
        if (txt == i18n(homePageItems[idx]))
            return idx + 1;
    }
    return 0;
}

bool KMyMoneyUtils::appendCorrectFileExt(QString& str, const QString& strExtToUse)
{
    bool rc = false;

    if (!str.isEmpty()) {
        //find last . deliminator
        int nLoc = str.lastIndexOf('.');
        if (nLoc != -1) {
            QString strExt, strTemp;
            strTemp = str.left(nLoc + 1);
            strExt = str.right(str.length() - (nLoc + 1));
            if (strExt.indexOf(strExtToUse, 0, Qt::CaseInsensitive) == -1) {
                // if the extension given contains a period, we remove ours
                if (strExtToUse.indexOf('.') != -1)
                    strTemp = strTemp.left(strTemp.length() - 1);
                //append extension to make complete file name
                strTemp.append(strExtToUse);
                str = strTemp;
                rc = true;
            }
        } else {
            str.append(QLatin1Char('.'));
            str.append(strExtToUse);
            rc = true;
        }
    }
    return rc;
}

void KMyMoneyUtils::checkConstants()
{
    // TODO: port to kf5
#if 0
    Q_ASSERT(static_cast<int>(KLocale::ParensAround) == static_cast<int>(MyMoneyMoney::ParensAround));
    Q_ASSERT(static_cast<int>(KLocale::BeforeQuantityMoney) == static_cast<int>(MyMoneyMoney::BeforeQuantityMoney));
    Q_ASSERT(static_cast<int>(KLocale::AfterQuantityMoney) == static_cast<int>(MyMoneyMoney::AfterQuantityMoney));
    Q_ASSERT(static_cast<int>(KLocale::BeforeMoney) == static_cast<int>(MyMoneyMoney::BeforeMoney));
    Q_ASSERT(static_cast<int>(KLocale::AfterMoney) == static_cast<int>(MyMoneyMoney::AfterMoney));
#endif
}

QString KMyMoneyUtils::getStylesheet(QString baseStylesheet)
{
    if (baseStylesheet.isEmpty())
        baseStylesheet = QStandardPaths::locate(QStandardPaths::AppConfigLocation, "html/kmymoney.css");

    QColor tcolor = KColorScheme(QPalette::Active).foreground(KColorScheme::NormalText).color();
    QColor link = KColorScheme(QPalette::Active).foreground(KColorScheme::LinkText).color();

    QString css;
    css += QString("@media screen {\n");
    css += QString(".row-even, .item0 { background-color: %1; color: %2 }\n")
               .arg(KMyMoneySettings::schemeColor(SchemeColor::ListBackground1).name(), tcolor.name());
    css += QString(".row-odd, .item1  { background-color: %1; color: %2 }\n")
               .arg(KMyMoneySettings::schemeColor(SchemeColor::ListBackground2).name(), tcolor.name());
    css += QString(".negativetext  { color: %1; }\n").arg(KMyMoneySettings::schemeColor(SchemeColor::Negative).name());
    css += QString("a { color: %1; }\n").arg(link.name());
    css += QString("}\n");

    QFile cssFile(baseStylesheet);
    if (cssFile.open(QIODevice::ReadOnly)) {
        QTextStream cssStream(&cssFile);
        css += cssStream.readAll();
        cssFile.close();
    }

    return css;
}

const MyMoneySplit KMyMoneyUtils::stockSplit(const MyMoneyTransaction& t)
{
    MyMoneySplit investmentAccountSplit;
    const auto splits = t.splits();
    for (const auto& split : splits) {
        if (!split.accountId().isEmpty()) {
            auto acc = MyMoneyFile::instance()->account(split.accountId());
            if (acc.isInvest()) {
                return split;
            }
            // if we have a reference to an investment account, we remember it here
            if (acc.accountType() == eMyMoney::Account::Type::Investment)
                investmentAccountSplit = split;
        }
    }
    // if we haven't found a stock split, we see if we've seen
    // an investment account on the way. If so, we return it.
    if (!investmentAccountSplit.id().isEmpty())
        return investmentAccountSplit;

    // if none was found, we return an empty split.
    return MyMoneySplit();
}

KMyMoneyUtils::transactionTypeE KMyMoneyUtils::transactionType(const MyMoneyTransaction& t)
{
    if (!stockSplit(t).id().isEmpty())
        return InvestmentTransaction;

    if (t.splitCount() < 2) {
        return Unknown;
    } else if (t.splitCount() > 2) {
        // FIXME check for loan transaction here
        return SplitTransaction;
    }
    QString ida, idb;
    const auto & splits = t.splits();
    if (splits.size() > 0)
        ida = splits[0].accountId();
    if (splits.size() > 1)
        idb = splits[1].accountId();
    if (ida.isEmpty() || idb.isEmpty())
        return Unknown;

    MyMoneyAccount a, b;
    a = MyMoneyFile::instance()->account(ida);
    b = MyMoneyFile::instance()->account(idb);
    if ((a.accountGroup() == eMyMoney::Account::Type::Asset
            || a.accountGroup() == eMyMoney::Account::Type::Liability)
            && (b.accountGroup() == eMyMoney::Account::Type::Asset
                || b.accountGroup() == eMyMoney::Account::Type::Liability))
        return Transfer;
    return Normal;
}

void KMyMoneyUtils::calculateAutoLoan(const MyMoneySchedule& schedule, MyMoneyTransaction& transaction, const QMap<QString, MyMoneyMoney>& balances)
{
    try {
        MyMoneyForecast::calculateAutoLoan(schedule, transaction, balances);
    } catch (const MyMoneyException &e) {
        KMessageBox::detailedError(nullptr, i18n("Unable to load schedule details"), QString::fromLatin1(e.what()));
    }
}

QString KMyMoneyUtils::reconcileStateToString(eMyMoney::Split::State flag, bool text)
{
    QString txt;
    const QModelIndex idx = MyMoneyFile::instance()->statusModel()->index(static_cast<int>(flag), 0);
    if (idx.isValid()) {
        txt = idx.data(text ? eMyMoney::Model::SplitReconcileStatusRole : eMyMoney::Model::SplitReconcileFlagRole).toString();
    }
    return txt;
}

MyMoneyTransaction KMyMoneyUtils::scheduledTransaction(const MyMoneySchedule& schedule)
{
    MyMoneyTransaction t = schedule.transaction();

    try {
        if (schedule.type() == eMyMoney::Schedule::Type::LoanPayment) {
            calculateAutoLoan(schedule, t, QMap<QString, MyMoneyMoney>());
        }
    } catch (const MyMoneyException &e) {
        qDebug("Unable to load schedule details for '%s' during transaction match: %s", qPrintable(schedule.name()), e.what());
    }

    t.clearId();
    t.setEntryDate(QDate());
    return t;
}

KXmlGuiWindow* KMyMoneyUtils::mainWindow()
{
    const auto widgetList = QApplication::topLevelWidgets();
    for (QWidget* widget : qAsConst(widgetList)) {
        KXmlGuiWindow* result = dynamic_cast<KXmlGuiWindow*>(widget);
        if (result)
            return result;
    }
    return nullptr;
}

void KMyMoneyUtils::updateWizardButtons(QWizard* wizard)
{
    // setup text on buttons
    wizard->setButtonText(QWizard::NextButton, i18nc("Go to next page of the wizard", "&Next"));
    wizard->setButtonText(QWizard::BackButton, KStandardGuiItem::back().text());

    // setup icons
    wizard->button(QWizard::FinishButton)->setIcon(KStandardGuiItem::ok().icon());
    wizard->button(QWizard::CancelButton)->setIcon(KStandardGuiItem::cancel().icon());
    wizard->button(QWizard::NextButton)->setIcon(KStandardGuiItem::forward(KStandardGuiItem::UseRTL).icon());
    wizard->button(QWizard::BackButton)->setIcon(KStandardGuiItem::back(KStandardGuiItem::UseRTL).icon());
}

void KMyMoneyUtils::dissectInvestmentTransaction(const QModelIndex &investSplitIdx, QModelIndex &assetAccountSplitIdx, SplitModel* feeSplitModel, SplitModel* interestSplitModel, MyMoneySecurity &security, MyMoneySecurity &currency, eMyMoney::Split::InvestmentTransactionType &transactionType)
{
    // clear split models
    feeSplitModel->unload();
    interestSplitModel->unload();

    assetAccountSplitIdx = QModelIndex(); // set to none to check later if it was assigned
    const auto file = MyMoneyFile::instance();

    // collect the splits. split references the stock account and should already
    // be set up. assetAccountSplit references the corresponding asset account (maybe
    // empty), feeSplits is the list of all expenses and interestSplits
    // the list of all incomes
    auto idx = MyMoneyFile::baseModel()->mapToBaseSource(investSplitIdx);
    const auto list = idx.model()->match(idx.model()->index(0, 0), eMyMoney::Model::JournalTransactionIdRole,
                                         idx.data(eMyMoney::Model::JournalTransactionIdRole),
                                         -1,                         // all splits
                                         Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive | Qt::MatchRecursive));
    for (const auto& splitIdx : list) {
        auto accIdx = file->accountsModel()->indexById(splitIdx.data(eMyMoney::Model::SplitAccountIdRole).toString());
        const auto accountGroup = accIdx.data(eMyMoney::Model::AccountGroupRole).value<eMyMoney::Account::Type>();
        if (splitIdx.row() == idx.row()) {
            security = file->security(accIdx.data(eMyMoney::Model::AccountCurrencyIdRole).toString());
        } else if (accountGroup == eMyMoney::Account::Type::Expense) {
            feeSplitModel->appendSplit(file->journalModel()->itemByIndex(splitIdx).split());
        } else if (accountGroup == eMyMoney::Account::Type::Income) {
            interestSplitModel->appendSplit(file->journalModel()->itemByIndex(splitIdx).split());
        } else {
            if (!assetAccountSplitIdx.isValid()) { // first asset Account should be our requested brokerage account
                assetAccountSplitIdx = splitIdx;
            } else if (idx.data(eMyMoney::Model::SplitValueRole).value<MyMoneyMoney>().isNegative()) { // the rest (if present) is handled as fee or interest
                feeSplitModel->appendSplit(file->journalModel()->itemByIndex(splitIdx).split());
            } else if (idx.data(eMyMoney::Model::SplitValueRole).value<MyMoneyMoney>().isPositive()) {
                interestSplitModel->appendSplit(file->journalModel()->itemByIndex(splitIdx).split());
            }
        }
    }

    // determine transaction type
    transactionType = idx.data(eMyMoney::Model::TransactionInvestementType).value<eMyMoney::Split::InvestmentTransactionType>();

    currency.setTradingSymbol("???");
    try {
        currency = file->security(file->journalModel()->itemByIndex(idx).transaction().commodity());
    } catch (const MyMoneyException &) {
    }
}

void KMyMoneyUtils::processPriceList(const MyMoneyStatement &st)
{
    auto file = MyMoneyFile::instance();
    QHash<QString, MyMoneySecurity> secBySymbol;
    QHash<QString, MyMoneySecurity> secByName;

    const auto securityList = file->securityList();
    for (const auto& sec : securityList) {
        secBySymbol[sec.tradingSymbol()] = sec;
        secByName[sec.name()] = sec;
    }

    for (const auto& stPrice : st.m_listPrices) {
        auto currency = file->baseCurrency().id();
        QString security;

        if (!stPrice.m_strCurrency.isEmpty()) {
            security = stPrice.m_strSecurity;
            currency = stPrice.m_strCurrency;
        } else if (secBySymbol.contains(stPrice.m_strSecurity)) {
            security = secBySymbol[stPrice.m_strSecurity].id();
            currency = file->security(file->security(security).tradingCurrency()).id();
        } else if (secByName.contains(stPrice.m_strSecurity)) {
            security = secByName[stPrice.m_strSecurity].id();
            currency = file->security(file->security(security).tradingCurrency()).id();
        } else
            return;

        MyMoneyPrice price(security,
                           currency,
                           stPrice.m_date,
                           stPrice.m_amount, stPrice.m_sourceName.isEmpty() ? i18n("Prices Importer") : stPrice.m_sourceName);
        file->addPrice(price);
    }
}

void KMyMoneyUtils::deleteSecurity(const MyMoneySecurity& security, QWidget* parent)
{
    QString msg, msg2;
    QString dontAsk, dontAsk2;
    if (security.isCurrency()) {
        msg = i18n("<p>Do you really want to remove the currency <b>%1</b> from the file?</p>", security.name());
        msg2 = i18n("<p>All exchange rates for currency <b>%1</b> will be lost.</p><p>Do you still want to continue?</p>", security.name());
        dontAsk = "DeleteCurrency";
        dontAsk2 = "DeleteCurrencyRates";
    } else {
        msg = i18n("<p>Do you really want to remove the %1 <b>%2</b> from the file?</p>", MyMoneySecurity::securityTypeToString(security.securityType()), security.name());
        msg2 = i18n("<p>All price quotes for %1 <b>%2</b> will be lost.</p><p>Do you still want to continue?</p>", MyMoneySecurity::securityTypeToString(security.securityType()), security.name());
        dontAsk = "DeleteSecurity";
        dontAsk2 = "DeleteSecurityPrices";
    }
    if (KMessageBox::questionTwoActions(parent, msg, i18n("Delete security"), KMMYesNo::yes(), KMMYesNo::no(), dontAsk) == KMessageBox::PrimaryAction) {
        MyMoneyFileTransaction ft;
        auto file = MyMoneyFile::instance();

        QBitArray skip((int)eStorage::Reference::Count);
        skip.fill(true);
        skip.clearBit((int)eStorage::Reference::Price);
        if (file->isReferenced(security, skip)) {
            if (KMessageBox::questionTwoActions(parent, msg2, i18n("Delete prices"), KMMYesNo::yes(), KMMYesNo::no(), dontAsk2) == KMessageBox::PrimaryAction) {
                try {
                    QString secID = security.id();
                    const auto priceList = file->priceList();
                    for (const auto& priceEntry : priceList) {
                        const MyMoneyPrice& price = priceEntry.first();
                        if (price.from() == secID || price.to() == secID)
                            file->removePrice(price);
                    }
                    ft.commit();
                    ft.restart();
                } catch (const MyMoneyException &) {
                    qDebug("Cannot delete price");
                    return;
                }
            } else
                return;
        }
        try {
            if (security.isCurrency())
                file->removeCurrency(security);
            else
                file->removeSecurity(security);
            ft.commit();
        } catch (const MyMoneyException &) {
        }
    }
}

bool KMyMoneyUtils::fileExists(const QUrl &url)
{
    bool fileExists = false;
    if (url.isValid()) {
        if (url.isLocalFile() || url.scheme().isEmpty()) {
            QFileInfo check_file(url.toLocalFile());
            fileExists = check_file.exists() && check_file.isFile();

        } else {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            auto statjob = KIO::stat(url, KIO::StatJob::SourceSide, KIO::StatNoDetails);
#else
            auto statjob = KIO::statDetails(url, KIO::StatJob::SourceSide, KIO::StatNoDetails);
#endif
            bool noerror = statjob->exec();
            if (noerror) {
                // We want a file
                fileExists = !statjob->statResult().isDir();
            }
            statjob->kill();
        }
    }
    return fileExists;
}

QString KMyMoneyUtils::downloadFile(const QUrl &url)
{
    QString filename;
    KIO::StoredTransferJob *transferjob = KIO::storedGet (url);
//  KJobWidgets::setWindow(transferjob, this);
    if (! transferjob->exec()) {
        KMessageBox::detailedError(nullptr,
                                   i18n("Error while loading file '%1'.", url.url()),
                                   transferjob->errorString(),
                                   i18n("File access error"));
        return filename;
    }

    QTemporaryFile file;
    file.setAutoRemove(false);
    if (file.open()) {
        file.write(transferjob->data());
        filename = file.fileName();
        file.close();
    } else {
        qDebug() << "Cannot write" << file.fileName();
    }
    return filename;
}

std::tuple<bool, QString> KMyMoneyUtils::newPayee(const QString& newnameBase)
{
    bool doit = true;
    QString id;

    if (newnameBase != i18n("New Payee")) {
        // Ask the user if that is what he intended to do?
        const auto msg = i18n("<qt>Do you want to add <b>%1</b> as payee/receiver?</qt>", newnameBase);

        if (KMessageBox::questionTwoActions(nullptr, msg, i18n("New payee/receiver"), KMMYesNo::yes(), KMMYesNo::no(), "NewPayee")
            == KMessageBox::SecondaryAction) {
            doit = false;
            // we should not keep the 'no' setting because that can confuse people like
            // I have seen in some usability tests. So we just delete it right away.
            KSharedConfigPtr kconfig = KSharedConfig::openConfig();
            if (kconfig) {
                kconfig->group(QLatin1String("Notification Messages")).deleteEntry(QLatin1String("NewPayee"));
            }
        }
    }

    if (doit) {
        MyMoneyFileTransaction ft;
        try {
            QString newname(newnameBase);
            // adjust name until a unique name has been created
            int count = 0;

            for (;;) {
                try {
                    const auto payee = MyMoneyFile::instance()->payeeByName(newname);
                    if (payee.id().isEmpty())
                        break;
                    newname = QString::fromLatin1("%1 [%2]").arg(newnameBase).arg(++count);
                } catch (const MyMoneyException &) {
                    break;
                }
            }

            MyMoneyPayee p;
            p.setName(newname);
            p.setMatchData(eMyMoney::Payee::MatchType::NameExact, true, QStringList());
            MyMoneyFile::instance()->addPayee(p);
            id = p.id();
            ft.commit();
        } catch (const MyMoneyException &e) {
            KMessageBox::detailedError(nullptr, i18n("Unable to add payee"), QString::fromLatin1(e.what()));
            doit = false;
        }
    }
    return std::make_tuple(doit, id);
}

std::tuple<bool, QString> KMyMoneyUtils::newTag(const QString& newnameBase)
{
    bool doit = true;
    QString id;

    if (newnameBase != i18n("New Tag")) {
        // Ask the user if that is what he intended to do?
        const auto msg = i18n("<qt>Do you want to add <b>%1</b> as tag?</qt>", newnameBase);

        if (KMessageBox::questionTwoActions(nullptr, msg, i18n("New tag"), KMMYesNo::yes(), KMMYesNo::no(), "NewTag") == KMessageBox::SecondaryAction) {
            doit = false;
            // we should not keep the 'no' setting because that can confuse people like
            // I have seen in some usability tests. So we just delete it right away.
            KSharedConfigPtr kconfig = KSharedConfig::openConfig();
            if (kconfig) {
                kconfig->group(QLatin1String("Notification Messages")).deleteEntry(QLatin1String("NewTag"));
            }
        }
    }

    if (doit) {
        MyMoneyFileTransaction ft;
        try {
            QString newname(newnameBase);
            // adjust name until a unique name has been created
            int count = 0;
            for (;;) {
                try {
                    if (MyMoneyFile::instance()->tagByName(newname).id().isEmpty()) {
                        break;
                    }
                    newname = QString::fromLatin1("%1 [%2]").arg(newnameBase).arg(++count);
                } catch (const MyMoneyException &) {
                    break;
                }
            }

            MyMoneyTag ta;
            ta.setName(newname);
            MyMoneyFile::instance()->addTag(ta);
            id = ta.id();
            ft.commit();
        } catch (const MyMoneyException &e) {
            KMessageBox::detailedError(nullptr, i18n("Unable to add tag"), QString::fromLatin1(e.what()));
        }
    }
    return std::make_tuple(doit, id);
}

void KMyMoneyUtils::newInstitution(MyMoneyInstitution& institution)
{
    auto file = MyMoneyFile::instance();

    MyMoneyFileTransaction ft;

    try {
        file->addInstitution(institution);
        ft.commit();

    } catch (const MyMoneyException &e) {
        KMessageBox::information(nullptr, i18n("Cannot add institution: %1", QString::fromLatin1(e.what())));
    }
}

QDebug KMyMoneyUtils::debug()
{
    return qDebug() << QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss.zzz"));
}

MyMoneyForecast KMyMoneyUtils::forecast()
{
    MyMoneyForecast forecast;

    // override object defaults with those of the application
    forecast.setForecastCycles(KMyMoneySettings::forecastCycles());
    forecast.setAccountsCycle(KMyMoneySettings::forecastAccountCycle());
    forecast.setHistoryStartDate(QDate::currentDate().addDays(-forecast.forecastCycles()*forecast.accountsCycle()));
    forecast.setHistoryEndDate(QDate::currentDate().addDays(-1));
    forecast.setForecastDays(KMyMoneySettings::forecastDays());
    forecast.setBeginForecastDay(KMyMoneySettings::beginForecastDay());
    forecast.setForecastMethod(KMyMoneySettings::forecastMethod());
    forecast.setHistoryMethod(KMyMoneySettings::historyMethod());
    forecast.setIncludeFutureTransactions(KMyMoneySettings::includeFutureTransactions());
    forecast.setIncludeScheduledTransactions(KMyMoneySettings::includeScheduledTransactions());

    return forecast;
}

bool KMyMoneyUtils::canUpdateAllAccounts()
{
    const auto file = MyMoneyFile::instance();
    auto rc = false;

    QList<MyMoneyAccount> accList;
    file->accountList(accList);
    QList<MyMoneyAccount>::const_iterator it_a;
    auto it_p = pPlugins.online.cend();
    for (it_a = accList.cbegin(); (it_p == pPlugins.online.cend()) && (it_a != accList.cend()); ++it_a) {
        if ((*it_a).hasOnlineMapping()) {
            // check if provider is available
            it_p = pPlugins.online.constFind((*it_a).onlineBankingSettings().value("provider").toLower());
            if (it_p != pPlugins.online.cend()) {
                QStringList protocols;
                (*it_p)->protocols(protocols);
                if (!protocols.isEmpty()) {
                    rc = true;
                    break;
                }
            }
        }
    }
    return rc;
}

void KMyMoneyUtils::showStatementImportResult(const QStringList& resultMessages, uint statementCount)
{
    KMessageBox::informationList(nullptr,
                                 i18np("One statement has been processed with the following results:",
                                       "%1 statements have been processed with the following results:",
                                       statementCount),
                                 !resultMessages.isEmpty() ?
                                 resultMessages :
                                 QStringList { i18np("No new transaction has been imported.", "No new transactions have been imported.", statementCount) },
                                 i18n("Statement import statistics"));
}

QString KMyMoneyUtils::normalizeNumericString(const qreal& val, const QLocale& loc, const char f, const int prec)
{
    static const QRegularExpression trailingZeroesRegex("0+$");
    return loc.toString(val, f, prec).remove(loc.groupSeparator()).remove(trailingZeroesRegex).remove(QRegularExpression("\\" + loc.decimalPoint() + "$"));
}
