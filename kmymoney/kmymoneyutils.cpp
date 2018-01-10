/***************************************************************************
                          kmymoneyutils.cpp  -  description
                             -------------------
    begin                : Wed Feb 5 2003
    copyright            : (C) 2000-2003 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kmymoneyutils.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>
#include <QApplication>
#include <QList>
#include <QPixmap>
#include <QWizard>
#include <QAbstractButton>
#include <QPixmapCache>
#include <QIcon>
#include <QPainter>
#include <QBitArray>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

// ----------------------------------------------------------------------------
// KDE Headers

#include <KColorScheme>
#include <KLocalizedString>
#include <KGuiItem>
#include <KXmlGuiWindow>
#include <KMessageBox>
#include <KStandardGuiItem>
#include <KIO/StatJob>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymoney.h"
#include "mymoneyexception.h"
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
#include "kmymoneyglobalsettings.h"
#include "icons.h"
#include "storageenums.h"
#include "mymoneyenums.h"

using namespace Icons;

KMyMoneyUtils::KMyMoneyUtils()
{
}

KMyMoneyUtils::~KMyMoneyUtils()
{
}

const QString KMyMoneyUtils::occurrenceToString(const eMyMoney::Schedule::Occurrence occurrence)
{
  return i18nc("Frequency of schedule", MyMoneySchedule::occurrenceToString(occurrence).toLatin1());
}

const QString KMyMoneyUtils::paymentMethodToString(eMyMoney::Schedule::PaymentType paymentType)
{
  return i18nc("Scheduled Transaction payment type", MyMoneySchedule::paymentMethodToString(paymentType).toLatin1());
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
                        Icons::get(Icon::ViewFilter),
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
  I18N_NOOP("Forecast (history)"),
  I18N_NOOP("Assets and Liabilities"),
  I18N_NOOP("Budget"),
  I18N_NOOP("CashFlow"),
  // insert new items above this comment
  0
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
    //find last . delminator
    int nLoc = str.lastIndexOf('.');
    if (nLoc != -1) {
      QString strExt, strTemp;
      strTemp = str.left(nLoc + 1);
      strExt = str.right(str.length() - (nLoc + 1));
      if (strExt.indexOf(strExtToUse, 0, Qt::CaseInsensitive) == -1) {
        // if the extension given contains a period, we remove our's
        if (strExtToUse.indexOf('.') != -1)
          strTemp = strTemp.left(strTemp.length() - 1);
        //append extension to make complete file name
        strTemp.append(strExtToUse);
        str = strTemp;
        rc = true;
      }
    } else {
      str.append(".");
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
  css += "<style type=\"text/css\">\n<!--\n";
  css += QString(".row-even, .item0 { background-color: %1; color: %2 }\n")
         .arg(KMyMoneyGlobalSettings::schemeColor(SchemeColor::ListBackground1).name()).arg(tcolor.name());
  css += QString(".row-odd, .item1  { background-color: %1; color: %2 }\n")
         .arg(KMyMoneyGlobalSettings::schemeColor(SchemeColor::ListBackground2).name()).arg(tcolor.name());
  css += QString("a { color: %1 }\n").arg(link.name());
  css += "-->\n</style>\n";
  return css;
}

QString KMyMoneyUtils::findResource(QStandardPaths::StandardLocation type, const QString& filename)
{
  QLocale locale;
  QString country;
  QString localeName = locale.bcp47Name();
  QString language = localeName;

  // extract language and country from the bcp47name
  QRegularExpression regExp(QLatin1String("(\\w+)_(\\w+)"));
  QRegularExpressionMatch match = regExp.match(localeName);
  if (match.hasMatch()) {
    language = match.captured(1);
    country = match.captured(2);
  }

  QString rc;

  // check that the placeholder is present and set things up
  if (filename.indexOf("%1") != -1) {
    /// @fixme somehow I have the impression, that language and country
    ///    mappings to the filename are not correct. This certainly must
    ///    be overhauled at some point in time (ipwizard, 2017-10-22)
    QString mask = filename.arg("_%1.%2");
    rc = QStandardPaths::locate(type, mask.arg(country).arg(language));

    // search the given resource
    if (rc.isEmpty()) {
        mask = filename.arg("_%1");
        rc = QStandardPaths::locate(type, mask.arg(language));
    }
    if (rc.isEmpty()) {
        // qDebug(QString("html/home_%1.html not found").arg(country).toLatin1());
        rc = QStandardPaths::locate(type, mask.arg(country));
    }
    if (rc.isEmpty()) {
        rc = QStandardPaths::locate(type, filename.arg(""));
    }
  } else {
    rc = QStandardPaths::locate(type, filename);
  }

  if (rc.isEmpty()) {
    qWarning("No resource found for (%s,%s)", qPrintable(QStandardPaths::displayName(type)), qPrintable(filename));
  }
  return rc;
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
  if (t.splits().size() > 0)
    ida = t.splits()[0].accountId();
  if (t.splits().size() > 1)
    idb = t.splits()[1].accountId();
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
    KMessageBox::detailedError(0, i18n("Unable to load schedule details"), e.what());
  }
}

QString KMyMoneyUtils::nextCheckNumber(const MyMoneyAccount& acc)
{
  QString number;
  //                   +-#1--+ +#2++-#3-++-#4--+
  QRegExp exp(QString("(.*\\D)?(0*)(\\d+)(\\D.*)?"));
  if (exp.indexIn(acc.value("lastNumberUsed")) != -1) {
    setLastNumberUsed(acc.value("lastNumberUsed"));
    QString arg1 = exp.cap(1);
    QString arg2 = exp.cap(2);
    QString arg3 = QString::number(exp.cap(3).toULong() + 1);
    QString arg4 = exp.cap(4);
    number = QString("%1%2%3%4").arg(arg1).arg(arg2).arg(arg3).arg(arg4);

    // if new number is longer than previous one and we identified
    // preceding 0s, then remove one of the preceding zeros
    if (arg2.length() > 0 && (number.length() != acc.value("lastNumberUsed").length())) {
      arg2 = arg2.mid(1);
      number = QString("%1%2%3%4").arg(arg1).arg(arg2).arg(arg3).arg(arg4);
    }
  } else {
    number = '1';
  }
  return number;
}

void KMyMoneyUtils::updateLastNumberUsed(const MyMoneyAccount& acc, const QString& number)
{
  MyMoneyAccount accnt = acc;
  QString num = number;
  // now check if this number has been used already
  auto file = MyMoneyFile::instance();
  if (file->checkNoUsed(accnt.id(), num)) {
    // if a number has been entered which is immediately prior to
    // an existing number, the next new number produced would clash
    // so need to look ahead for free next number
    bool free = false;
    for (int i = 0; i < 10; i++) {
      // find next unused number - 10 tries (arbitrary)
      if (file->checkNoUsed(accnt.id(), num)) {
        //  increment and try again
        num = getAdjacentNumber(num);
      } else {
        //  found a free number
        free = true;
        break;
      }
    }
    if (!free) {
      qDebug() << "No free number found - set to '1'";
      num = '1';
    }
    setLastNumberUsed(getAdjacentNumber(num, - 1));
  }
}

void KMyMoneyUtils::setLastNumberUsed(const QString& num)
{
  m_lastNumberUsed = num;
}

QString KMyMoneyUtils::lastNumberUsed()
{
  return m_lastNumberUsed;
}

QString KMyMoneyUtils::getAdjacentNumber(const QString& number, int offset)
{
  QString num = number;
  //                   +-#1--+ +#2++-#3-++-#4--+
  QRegExp exp(QString("(.*\\D)?(0*)(\\d+)(\\D.*)?"));
  if (exp.indexIn(num) != -1) {
    QString arg1 = exp.cap(1);
    QString arg2 = exp.cap(2);
    QString arg3 = QString::number(exp.cap(3).toULong() + offset);
    QString arg4 = exp.cap(4);
    num = QString("%1%2%3%4").arg(arg1).arg(arg2).arg(arg3).arg(arg4);
  } else {
    num = '1';
  }  //  next free number
  return num;
}

quint64 KMyMoneyUtils::numericPart(const QString & num)
{
  quint64 num64 = 0;
  QRegExp exp(QString("(.*\\D)?(0*)(\\d+)(\\D.*)?"));
  if (exp.indexIn(num) != -1) {
    QString arg1 = exp.cap(1);
    QString arg2 = exp.cap(2);
    QString arg3 = QString::number(exp.cap(3).toULongLong());
    QString arg4 = exp.cap(4);
    num64 = QString("%2%3").arg(arg2).arg(arg3).toULongLong();
  }
  return num64;
}

QString KMyMoneyUtils::reconcileStateToString(eMyMoney::Split::State flag, bool text)
{
  QString txt;
  if (text) {
    switch (flag) {
      case eMyMoney::Split::State::NotReconciled:
        txt = i18nc("Reconciliation state 'Not reconciled'", "Not reconciled");
        break;
      case eMyMoney::Split::State::Cleared:
        txt = i18nc("Reconciliation state 'Cleared'", "Cleared");
        break;
      case eMyMoney::Split::State::Reconciled:
        txt = i18nc("Reconciliation state 'Reconciled'", "Reconciled");
        break;
      case eMyMoney::Split::State::Frozen:
        txt = i18nc("Reconciliation state 'Frozen'", "Frozen");
        break;
      default:
        txt = i18nc("Unknown reconciliation state", "Unknown");
        break;
    }
  } else {
    switch (flag) {
      case eMyMoney::Split::State::NotReconciled:
        break;
      case eMyMoney::Split::State::Cleared:
        txt = i18nc("Reconciliation flag C", "C");
        break;
      case eMyMoney::Split::State::Reconciled:
        txt = i18nc("Reconciliation flag R", "R");
        break;
      case eMyMoney::Split::State::Frozen:
        txt = i18nc("Reconciliation flag F", "F");
        break;
      default:
        txt = i18nc("Flag for unknown reconciliation state", "?");
        break;
    }
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
    qDebug("Unable to load schedule details for '%s' during transaction match: %s", qPrintable(schedule.name()), qPrintable(e.what()));
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

void KMyMoneyUtils::dissectTransaction(const MyMoneyTransaction& transaction, const MyMoneySplit& split, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency, eMyMoney::Split::InvestmentTransactionType& transactionType)
{
  // collect the splits. split references the stock account and should already
  // be set up. assetAccountSplit references the corresponding asset account (maybe
  // empty), feeSplits is the list of all expenses and interestSplits
  // the list of all incomes
  assetAccountSplit = MyMoneySplit(); // set to none to check later if it was assigned
  auto file = MyMoneyFile::instance();
  foreach (const auto tsplit, transaction.splits()) {
    auto acc = file->account(tsplit.accountId());
    if (tsplit.id() == split.id()) {
      security = file->security(acc.currencyId());
    } else if (acc.accountGroup() == eMyMoney::Account::Type::Expense) {
      feeSplits.append(tsplit);
      // feeAmount += tsplit.value();
    } else if (acc.accountGroup() == eMyMoney::Account::Type::Income) {
      interestSplits.append(tsplit);
      // interestAmount += tsplit.value();
    } else {
      if (assetAccountSplit == MyMoneySplit()) // first asset Account should be our requested brokerage account
        assetAccountSplit = tsplit;
      else if (tsplit.value().isNegative())  // the rest (if present) is handled as fee or interest
        feeSplits.append(tsplit);              // and shouldn't be allowed to override assetAccountSplit
      else if (tsplit.value().isPositive())
        interestSplits.append(tsplit);
    }
  }

  // determine transaction type
  if (split.action() == MyMoneySplit::actionName(eMyMoney::Split::Action::AddShares)) {
    transactionType = (!split.shares().isNegative()) ? eMyMoney::Split::InvestmentTransactionType::AddShares : eMyMoney::Split::InvestmentTransactionType::RemoveShares;
  } else if (split.action() == MyMoneySplit::actionName(eMyMoney::Split::Action::BuyShares)) {
    transactionType = (!split.value().isNegative()) ? eMyMoney::Split::InvestmentTransactionType::BuyShares : eMyMoney::Split::InvestmentTransactionType::SellShares;
  } else if (split.action() == MyMoneySplit::actionName(eMyMoney::Split::Action::Dividend)) {
    transactionType = eMyMoney::Split::InvestmentTransactionType::Dividend;
  } else if (split.action() == MyMoneySplit::actionName(eMyMoney::Split::Action::ReinvestDividend)) {
    transactionType = eMyMoney::Split::InvestmentTransactionType::ReinvestDividend;
  } else if (split.action() == MyMoneySplit::actionName(eMyMoney::Split::Action::Yield)) {
    transactionType = eMyMoney::Split::InvestmentTransactionType::Yield;
  } else if (split.action() == MyMoneySplit::actionName(eMyMoney::Split::Action::SplitShares)) {
    transactionType = eMyMoney::Split::InvestmentTransactionType::SplitShares;
  } else if (split.action() == MyMoneySplit::actionName(eMyMoney::Split::Action::InterestIncome)) {
    transactionType = eMyMoney::Split::InvestmentTransactionType::InterestIncome;
  } else
    transactionType = eMyMoney::Split::InvestmentTransactionType::BuyShares;

  currency.setTradingSymbol("???");
  try {
    currency = file->security(transaction.commodity());
  } catch (const MyMoneyException &) {
  }
}

void KMyMoneyUtils::processPriceList(const MyMoneyStatement &st)
{
  auto file = MyMoneyFile::instance();
  QHash<QString, MyMoneySecurity> secBySymbol;
  QHash<QString, MyMoneySecurity> secByName;

  for (const auto& sec : file->securityList()) {
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
        short int detailLevel = 0; // Lowest level: file/dir/symlink/none
        KIO::StatJob* statjob = KIO::stat(url, KIO::StatJob::SourceSide, detailLevel);
        bool noerror = statjob->exec();
        if (noerror) {
            // We want a file
            fileExists = !statjob->statResult().isDir();
        }
        statjob->kill();
    }
    return fileExists;
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
          MyMoneyFile::instance()->payeeByName(newname);
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
      KMessageBox::detailedSorry(nullptr, i18n("Unable to add payee"),
                                 i18n("%1 thrown in %2:%3", e.what(), e.file(), e.line()));
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
          MyMoneyFile::instance()->tagByName(newname);
          newname = QString::fromLatin1("%1 [%2]").arg(newnameBase, ++count);
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
      KMessageBox::detailedSorry(nullptr, i18n("Unable to add tag"),
                                 i18n("%1 thrown in %2:%3", e.what(), e.file(), e.line()));
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
    KMessageBox::information(nullptr, i18n("Cannot add institution: %1", e.what()));
  }
}

QDebug KMyMoneyUtils::debug()
{
  return qDebug() << QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss.zzz"));
}
