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
#include <amountedit.h>
#include <creditdebitedit.h>

// ----------------------------------------------------------------------------
// KDE Headers

#include <KColorScheme>
#include <KGuiItem>
#include <KIO/StatJob>
#include <KIO/StoredTransferJob>
#include <KLocalizedString>
#include <KMessageBox>
#include <KStandardGuiItem>
#include <KXmlGuiWindow>
#include <kio_version.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymoney.h"
#include "mymoneyexception.h"
#include "mymoneytransactionfilter.h"
#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneysecurity.h"
#include "mymoneyschedule.h"
#include "mymoneypayee.h"
#include "mymoneytag.h"
#include "mymoneyprice.h"
#include "mymoneystatement.h"
#include "mymoneyforecast.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "kmymoneysettings.h"
#include "icons.h"
#include "storageenums.h"
#include "mymoneyenums.h"
#include "kmymoneyplugin.h"
#include "statusmodel.h"
#include "journalmodel.h"
#include "splitmodel.h"
#include "accountsmodel.h"

using namespace Icons;

const QString KMyMoneyUtils::occurrenceToString(const eMyMoney::Schedule::Occurrence occurrence)
{
    return i18n(MyMoneySchedule::occurrenceToString(occurrence));
}

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
    I18N_NOOP("Payments"),
    I18N_NOOP("Preferred accounts"),
    I18N_NOOP("Payment accounts"),
    I18N_NOOP("Favorite reports"),
    I18N_NOOP("Forecast (schedule)"),
    I18N_NOOP("Net worth forecast"),
    I18N_NOOP("Forecast (history)"),        // unused, s.a. KSettingsHome::slotLoadItems()
    I18N_NOOP("Assets and Liabilities"),
    I18N_NOOP("Budget"),
    I18N_NOOP("CashFlow"),
    // insert new items above this comment
    0,
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
    for (idx = 0; homePageItems[idx] != 0; ++idx) {
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

QString KMyMoneyUtils::variableCSS()
{
    QColor tcolor = KColorScheme(QPalette::Active).foreground(KColorScheme::NormalText).color();
    QColor link = KColorScheme(QPalette::Active).foreground(KColorScheme::LinkText).color();

    QString css;
    css += QString(".row-even, .item0 { background-color: %1; color: %2 }\n")
           .arg(KMyMoneySettings::schemeColor(SchemeColor::ListBackground1).name()).arg(tcolor.name());
    css += QString(".row-odd, .item1  { background-color: %1; color: %2 }\n")
           .arg(KMyMoneySettings::schemeColor(SchemeColor::ListBackground2).name()).arg(tcolor.name());
    css += QString("a { color: %1 }\n").arg(link.name());
    return css;
}

const MyMoneySplit KMyMoneyUtils::stockSplit(const MyMoneyTransaction& t)
{
    MyMoneySplit investmentAccountSplit;
    foreach (const auto split, t.splits()) {
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
        KMessageBox::detailedError(0, i18n("Unable to load schedule details"), QString::fromLatin1(e.what()));
    }
}

QString KMyMoneyUtils::nextCheckNumber(const MyMoneyAccount& acc)
{
    return getAdjacentNumber(acc.value("lastNumberUsed"), 1);
}

QString KMyMoneyUtils::nextFreeCheckNumber(const MyMoneyAccount& acc)
{
    auto file = MyMoneyFile::instance();
    auto num = acc.value("lastNumberUsed");

    if (num.isEmpty())
        num = QStringLiteral("1");

    // now check if this number has been used already
    if (file->checkNoUsed(acc.id(), num)) {
        // if a number has been entered which is immediately prior to
        // an existing number, the next new number produced would clash
        // so need to look ahead for free next number
        // we limit that to a number of tries which depends on the
        // number of splits in that account (we can't have more)
        MyMoneyTransactionFilter filter(acc.id());
        QList<MyMoneyTransaction> transactions;
        file->transactionList(transactions, filter);
        const int maxNumber = transactions.count();
        for (int i = 0; i < maxNumber; i++) {
            if (file->checkNoUsed(acc.id(), num)) {
                //  increment and try again
                num = getAdjacentNumber(num);
            } else {
                //  found a free number
                break;
            }
        }
    }
    return num;
}

QString KMyMoneyUtils::getAdjacentNumber(const QString& number, int offset)
{
    // make sure the offset is either -1 or 1
    offset = (offset >= 0) ? 1 : -1;

    QString num = number;
    //                   +-#1--+ +#2++-#3-++-#4--+
    QRegExp exp(QString("(.*\\D)?(0*)(\\d+)(\\D.*)?"));
    if (exp.indexIn(num) != -1) {
        QString arg1 = exp.cap(1);
        QString arg2 = exp.cap(2);
        QString arg3 = QString::number(exp.cap(3).toULong() + offset);
        QString arg4 = exp.cap(4);
        num = QString("%1%2%3%4").arg(arg1, arg2, arg3, arg4);
    } else {
        num = QStringLiteral("1");
    }
    return num;
}

quint64 KMyMoneyUtils::numericPart(const QString & num)
{
    quint64 num64 = 0;
    QRegExp exp(QString("(.*\\D)?(0*)(\\d+)(\\D.*)?"));
    if (exp.indexIn(num) != -1) {
        // QString arg1 = exp.cap(1);
        QString arg2 = exp.cap(2);
        QString arg3 = QString::number(exp.cap(3).toULongLong());
        // QString arg4 = exp.cap(4);
        num64 = QString("%2%3").arg(arg2, arg3).toULongLong();
    }
    return num64;
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
    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
        KXmlGuiWindow* result = dynamic_cast<KXmlGuiWindow*>(widget);
        if (result)
            return result;
    }
    return 0;
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
    if (KMessageBox::questionYesNo(parent, msg, i18n("Delete security"), KStandardGuiItem::yes(), KStandardGuiItem::no(), dontAsk) == KMessageBox::Yes) {
        MyMoneyFileTransaction ft;
        auto file = MyMoneyFile::instance();

        QBitArray skip((int)eStorage::Reference::Count);
        skip.fill(true);
        skip.clearBit((int)eStorage::Reference::Price);
        if (file->isReferenced(security, skip)) {
            if (KMessageBox::questionYesNo(parent, msg2, i18n("Delete prices"), KStandardGuiItem::yes(), KStandardGuiItem::no(), dontAsk2) == KMessageBox::Yes) {
                try {
                    QString secID = security.id();
                    foreach (auto priceEntry, file->priceList()) {
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
            auto statjob = KIO::statDetails(url, KIO::StatJob::SourceSide, KIO::StatNoDetails);
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
    file.open();
    file.write(transferjob->data());
    filename = file.fileName();
    file.close();
    return filename;
}

bool KMyMoneyUtils::newPayee(const QString& newnameBase, QString& id)
{
    bool doit = true;

    if (newnameBase != i18n("New Payee")) {
        // Ask the user if that is what he intended to do?
        const auto msg = i18n("<qt>Do you want to add <b>%1</b> as payer/receiver?</qt>", newnameBase);

        if (KMessageBox::questionYesNo(nullptr, msg, i18n("New payee/receiver"), KStandardGuiItem::yes(), KStandardGuiItem::no(), "NewPayee") == KMessageBox::No) {
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
            MyMoneyFile::instance()->addPayee(p);
            id = p.id();
            ft.commit();
        } catch (const MyMoneyException &e) {
            KMessageBox::detailedSorry(nullptr, i18n("Unable to add payee"), QString::fromLatin1(e.what()));
            doit = false;
        }
    }
    return doit;
}

void KMyMoneyUtils::newTag(const QString& newnameBase, QString& id)
{
    bool doit = true;

    if (newnameBase != i18n("New Tag")) {
        // Ask the user if that is what he intended to do?
        const auto msg = i18n("<qt>Do you want to add <b>%1</b> as tag?</qt>", newnameBase);

        if (KMessageBox::questionYesNo(nullptr, msg, i18n("New tag"), KStandardGuiItem::yes(), KStandardGuiItem::no(), "NewTag") == KMessageBox::No) {
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
            KMessageBox::detailedSorry(nullptr, i18n("Unable to add tag"), QString::fromLatin1(e.what()));
        }
    }
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
    auto it_p = pPlugins.online.constEnd();
    for (it_a = accList.constBegin(); (it_p == pPlugins.online.constEnd()) && (it_a != accList.constEnd()); ++it_a) {
        if ((*it_a).hasOnlineMapping()) {
            // check if provider is available
            it_p = pPlugins.online.constFind((*it_a).onlineBankingSettings().value("provider").toLower());
            if (it_p != pPlugins.online.constEnd()) {
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
    return loc.toString(val, f, prec)
           .remove(loc.groupSeparator())
           .remove(QRegularExpression("0+$"))
           .remove(QRegularExpression("\\" + loc.decimalPoint() + "$"));
}

QStringList KMyMoneyUtils::tabOrder(const QString& name, const QStringList& defaultTabOrder)
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup grp = config->group(QLatin1String("TabOrder"));
    return grp.readEntry(name, defaultTabOrder);
}

#if 0
static QString widgetName(QWidget* w)
{
    return w->objectName().isEmpty() ? w->metaObject()->className() : w->objectName();
}

static void dumpFocusChain(QWidget* w, QWidget* end, int additionalTabstops = 0, bool forward = true)
{
    QString txt;
    int loopValid = 80;
    int trailing = -1;
    do {
        const auto policy = w->focusPolicy();
        if (policy == Qt::TabFocus || policy == Qt::StrongFocus ) {
            if (!txt.isEmpty()) {
                txt += forward ? QLatin1String(" -> ") : QLatin1String(" <- ");
            }
            txt += widgetName(w);
            --loopValid;
            w = forward ? w->nextInFocusChain() : w->previousInFocusChain();
            if (w == end && (trailing < 0)) {
                trailing = additionalTabstops+1;
                loopValid = trailing;
            }
        }
        --trailing;
    } while ((loopValid > 0) && (trailing != 0));

    if (end) {
        txt += forward ? QLatin1String(" -> ") : QLatin1String(" <- ");
        txt += widgetName(w);
    }

    qDebug() << txt;
}
#endif

void KMyMoneyUtils::setupTabOrder(QWidget* parent, const QStringList& tabOrder)
{
    const auto widgetCount = tabOrder.count();
    if (widgetCount > 0) {
        auto prev = parent->findChild<QWidget*>(tabOrder.at(0));
        for (int i = 1; (prev != nullptr) && (i < widgetCount); ++i) {
            const auto next = parent->findChild<QWidget*>(tabOrder.at(i));
            if (next) {
                parent->setTabOrder(prev, next);
                prev = next;
            } else {
                qDebug() << tabOrder.at(i) << "not found :(";
            }
        }
    }
}

void KMyMoneyUtils::storeTabOrder(const QString& name, const QStringList& tabOrder)
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup grp = config->group(QLatin1String("TabOrder"));
    grp.writeEntry(name, tabOrder);
}

bool KMyMoneyUtils::tabFocusHelper(QWidget* topLevelWidget, bool next)
{
    const auto reason = next ? Qt::TabFocusReason : Qt::BacktabFocusReason;
    const auto tabOrder = topLevelWidget->property("kmm_currenttaborder").toStringList();

    if (tabOrder.isEmpty())
        return false;

    auto focusWidget = topLevelWidget->focusWidget();

    enum firstOrLastVisible {
        FirstVisible,
        LastVisible,
    };

    auto findFirstOrLastVisible = [&](firstOrLastVisible type) {
        const int ofs = (type == FirstVisible) ? 1 : -1;
        int idx = (type == FirstVisible) ? 0 : tabOrder.count() - 1;
        for (; idx >= 0 && idx < tabOrder.count(); idx += ofs) {
            auto w = topLevelWidget->findChild<QWidget*>(tabOrder.at(idx));
            // in case of embedded transaction editors, we may search
            // for a widget that is known to the parent
            if (!w) {
                auto parent = topLevelWidget->parentWidget();
                while (true) {
                    w = parent->findChild<QWidget*>(tabOrder.at(idx));
                    if (!w && qobject_cast<QGroupBox*>(parent)) {
                        parent = parent->parentWidget();
                        continue;
                    }
                    break;
                }
            }
            if (w && w->isVisible() && w->isEnabled()) {
                return w;
            }
        }
        return static_cast<QWidget*>(nullptr);
    };

    auto selectWidget = [&](QWidget* w) {
        if (w) {
            // if we point to a contructed widget (e.g. ButtonBox) we
            // need to select the last widget if going backward
            if (reason == Qt::BacktabFocusReason && !w->findChildren<QWidget*>().isEmpty()) {
                auto parent = w;
                while (w->nextInFocusChain()->parentWidget() == parent) {
                    w = w->nextInFocusChain();
                }
            }
            w->setFocus(reason);
        }
    };

    auto adjustToContainer = [&](const char* containerClass, const char* widgetClass) {
        if (focusWidget->qt_metacast(widgetClass) && focusWidget->parentWidget()->qt_metacast(containerClass) && (reason == Qt::BacktabFocusReason)) {
            if (focusWidget->previousInFocusChain() == focusWidget->parentWidget()) {
                focusWidget = focusWidget->parentWidget();
            }
        }
    };

    // In case of a CreditDebitEdit widget and we leave from the left backwards,
    // we need to adjust the widget to point to the container widget.
    adjustToContainer("CreditDebitEdit", "AmountEdit");
    // In case of a QDialogButtonBox widget and we leave from the left backwards,
    // we need to adjust the widget to point to the container widget.
    // adjustToContainer("QDialogButtonBox", "QPushButton");

    if ((reason == Qt::BacktabFocusReason) && (findFirstOrLastVisible(FirstVisible) == focusWidget)) {
        selectWidget(findFirstOrLastVisible(LastVisible));
        return true;

    } else if ((reason == Qt::TabFocusReason) && (findFirstOrLastVisible(LastVisible) == focusWidget)) {
        selectWidget(findFirstOrLastVisible(FirstVisible));
        return true;
    }
    return false;
}
