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

#include <QApplication>
#include <QList>
#include <QPixmap>
#include <QWizard>
#include <QAbstractButton>
#include <QPixmapCache>
#include <QIcon>

// ----------------------------------------------------------------------------
// KDE Headers

#include <kmessagebox.h>
#include <kconfig.h>
#include <KColorScheme>
#include <KLocalizedString>
#include <KXmlGuiWindow>

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyfile.h>
#include <mymoneyforecast.h>
#include <kmymoneyglobalsettings.h>
#include <investtransactioneditor.h>

KMyMoneyUtils::KMyMoneyUtils()
{
}

KMyMoneyUtils::~KMyMoneyUtils()
{
}

const QString KMyMoneyUtils::accountTypeToString(const MyMoneyAccount::accountTypeE accountType)
{
  return MyMoneyAccount::accountTypeToString(accountType);
}

MyMoneyAccount::accountTypeE KMyMoneyUtils::stringToAccountType(const QString& type)
{
  MyMoneyAccount::accountTypeE rc = MyMoneyAccount::UnknownAccountType;
  QString tmp = type.toLower();

  if (tmp == i18n("Checking").toLower())
    rc = MyMoneyAccount::Checkings;
  else if (tmp == i18n("Savings").toLower())
    rc = MyMoneyAccount::Savings;
  else if (tmp == i18n("Credit Card").toLower())
    rc = MyMoneyAccount::CreditCard;
  else if (tmp == i18n("Cash").toLower())
    rc = MyMoneyAccount::Cash;
  else if (tmp == i18n("Loan").toLower())
    rc = MyMoneyAccount::Loan;
  else if (tmp == i18n("Certificate of Deposit").toLower())
    rc = MyMoneyAccount::CertificateDep;
  else if (tmp == i18n("Investment").toLower())
    rc = MyMoneyAccount::Investment;
  else if (tmp == i18n("Money Market").toLower())
    rc = MyMoneyAccount::MoneyMarket;
  else if (tmp == i18n("Asset").toLower())
    rc = MyMoneyAccount::Asset;
  else if (tmp == i18n("Liability").toLower())
    rc = MyMoneyAccount::Liability;
  else if (tmp == i18n("Currency").toLower())
    rc = MyMoneyAccount::Currency;
  else if (tmp == i18n("Income").toLower())
    rc = MyMoneyAccount::Income;
  else if (tmp == i18n("Expense").toLower())
    rc = MyMoneyAccount::Expense;
  else if (tmp == i18n("Investment Loan").toLower())
    rc = MyMoneyAccount::AssetLoan;
  else if (tmp == i18n("Stock").toLower())
    rc = MyMoneyAccount::Stock;
  else if (tmp == i18n("Equity").toLower())
    rc = MyMoneyAccount::Equity;

  return rc;
}

MyMoneySecurity::eSECURITYTYPE KMyMoneyUtils::stringToSecurity(const QString& txt)
{
  MyMoneySecurity::eSECURITYTYPE rc = MyMoneySecurity::SECURITY_NONE;
  QString tmp = txt.toLower();

  if (tmp == i18n("Stock").toLower())
    rc = MyMoneySecurity::SECURITY_STOCK;
  else if (tmp == i18n("Mutual Fund").toLower())
    rc = MyMoneySecurity::SECURITY_MUTUALFUND;
  else if (tmp == i18n("Bond").toLower())
    rc = MyMoneySecurity::SECURITY_BOND;
  else if (tmp == i18n("Currency").toLower())
    rc = MyMoneySecurity::SECURITY_CURRENCY;

  return rc;
}

const QString KMyMoneyUtils::securityTypeToString(const MyMoneySecurity::eSECURITYTYPE securityType)
{
  return i18n(MyMoneySecurity::securityTypeToString(securityType).toLatin1());
}

AlkValue::RoundingMethod KMyMoneyUtils::stringToRoundingMethod(const QString& txt)
{
  AlkValue::RoundingMethod rc = AlkValue::RoundNever;
  QString tmp = txt.toLower();

  if (tmp == i18n("Never").toLower())
    rc = AlkValue::RoundNever;
  else if (tmp == i18n("Floor").toLower())
    rc = AlkValue::RoundFloor;
  else if (tmp == i18n("Ceil").toLower())
    rc = AlkValue::RoundCeil;
  else if (tmp == i18n("Truncate").toLower())
    rc = AlkValue::RoundTruncate;
  else if (tmp == i18n("Promote").toLower())
    rc = AlkValue::RoundPromote;
  else if (tmp == i18n("HalfDown").toLower())
    rc = AlkValue::RoundHalfDown;
  else if (tmp == i18n("HalfUp").toLower())
    rc = AlkValue::RoundHalfUp;
  else if (tmp == i18n("Round").toLower())
    rc = AlkValue::RoundRound;

  return rc;
}

const QString KMyMoneyUtils::roundingMethodToString(const AlkValue::RoundingMethod roundingMethod)
{
  return i18n(MyMoneySecurity::roundingMethodToString(roundingMethod).toLatin1());
}

const QString KMyMoneyUtils::occurrenceToString(const MyMoneySchedule::occurrenceE occurrence)
{
  return i18nc("Frequency of schedule", MyMoneySchedule::occurrenceToString(occurrence).toLatin1());
}

const QString KMyMoneyUtils::paymentMethodToString(MyMoneySchedule::paymentTypeE paymentType)
{
  return i18nc("Scheduled Transaction payment type", MyMoneySchedule::paymentMethodToString(paymentType).toLatin1());
}

const QString KMyMoneyUtils::weekendOptionToString(MyMoneySchedule::weekendOptionE weekendOption)
{
  return i18n(MyMoneySchedule::weekendOptionToString(weekendOption).toLatin1());
}

const QString KMyMoneyUtils::scheduleTypeToString(MyMoneySchedule::typeE type)
{
  return i18nc("Scheduled transaction type", MyMoneySchedule::scheduleTypeToString(type).toLatin1());
}

KGuiItem KMyMoneyUtils::scheduleNewGuiItem()
{
  KGuiItem splitGuiItem(i18n("&New Schedule..."),
                        QIcon::fromTheme("document-new"),
                        i18n("Create a new schedule."),
                        i18n("Use this to create a new schedule."));

  return splitGuiItem;
}

KGuiItem KMyMoneyUtils::accountsFilterGuiItem()
{
  KGuiItem splitGuiItem(i18n("&Filter"),
                        QIcon::fromTheme("view-filter"),
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
         .arg((KMyMoneyGlobalSettings::listBGColor()).name()).arg(tcolor.name());
  css += QString(".row-odd, .item1  { background-color: %1; color: %2 }\n")
         .arg((KMyMoneyGlobalSettings::listColor()).name()).arg(tcolor.name());
  css += QString("a { color: %1 }\n").arg(link.name());
  css += "-->\n</style>\n";
  return css;
}

QString KMyMoneyUtils::findResource(QStandardPaths::StandardLocation type, const QString& filename)
{
  // TODO: port to kf5
  QString language = "en";//KLocale::global()->language();
  QString country = "US";//KLocale::global()->country();
  QString rc, mask;

  // check that the placeholder is present
  if (!filename.indexOf("%1")) {
    qWarning("%%1 not found in '%s'", qPrintable(filename));
    return filename;
  }

  // search the given resource
  mask = filename.arg("_%1.%2");
  rc = QStandardPaths::locate(type, mask.arg(country).arg(language));
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

  if (rc.isEmpty()) {
    qWarning("No resource found for (%s,%s)", qPrintable(QStandardPaths::displayName(type)), qPrintable(filename));
  }
  return rc;
}

const MyMoneySplit KMyMoneyUtils::stockSplit(const MyMoneyTransaction& t)
{
  QList<MyMoneySplit>::ConstIterator it_s;
  MyMoneySplit investmentAccountSplit;
  for (it_s = t.splits().begin(); it_s != t.splits().end(); ++it_s) {
    if (!(*it_s).accountId().isEmpty()) {
      MyMoneyAccount acc = MyMoneyFile::instance()->account((*it_s).accountId());
      if (acc.isInvest()) {
        return *it_s;
      }
      // if we have a reference to an investment account, we remember it here
      if (acc.accountType() == MyMoneyAccount::Investment)
        investmentAccountSplit = *it_s;
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
  if ((a.accountGroup() == MyMoneyAccount::Asset
       || a.accountGroup() == MyMoneyAccount::Liability)
      && (b.accountGroup() == MyMoneyAccount::Asset
          || b.accountGroup() == MyMoneyAccount::Liability))
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
  MyMoneyFile* file = MyMoneyFile::instance();
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

QString KMyMoneyUtils::reconcileStateToString(MyMoneySplit::reconcileFlagE flag, bool text)
{
  QString txt;
  if (text) {
    switch (flag) {
      case MyMoneySplit::NotReconciled:
        txt = i18nc("Reconciliation state 'Not reconciled'", "Not reconciled");
        break;
      case MyMoneySplit::Cleared:
        txt = i18nc("Reconciliation state 'Cleared'", "Cleared");
        break;
      case MyMoneySplit::Reconciled:
        txt = i18nc("Reconciliation state 'Reconciled'", "Reconciled");
        break;
      case MyMoneySplit::Frozen:
        txt = i18nc("Reconciliation state 'Frozen'", "Frozen");
        break;
      default:
        txt = i18nc("Unknown reconciliation state", "Unknown");
        break;
    }
  } else {
    switch (flag) {
      case MyMoneySplit::NotReconciled:
        break;
      case MyMoneySplit::Cleared:
        txt = i18nc("Reconciliation flag C", "C");
        break;
      case MyMoneySplit::Reconciled:
        txt = i18nc("Reconciliation flag R", "R");
        break;
      case MyMoneySplit::Frozen:
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
    if (schedule.type() == MyMoneySchedule::TYPE_LOANPAYMENT) {
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

QPixmap KMyMoneyUtils::overlayIcon(const QString iconName, const QString overlayName, const Qt::Corner corner, int size)
{
  int x, y;
  QPixmap result;
  QIcon icon;
  QPixmap ovly;
  QString overlaidIcon = iconName + '-' + overlayName;

  // If found in the cache, return quickly
  if (QPixmapCache::find(overlaidIcon, result)) {
    return result;
  }

  // try to retrieve the main icon from cache
  if (!QPixmapCache::find(iconName, result)) {
    icon = QIcon::fromTheme(iconName);
    if (icon.isNull()) {
      QString iconReplacement;
      if (iconName.compare(QLatin1String("view-bank-account")) == 0)
        iconReplacement = "account";
      else if (iconName.compare(QLatin1String("view-investment")) == 0)
        iconReplacement = "investment";
      else if (iconName.compare(QLatin1String("view-financial-categories")) == 0)
        iconReplacement = "categories";
      else if (iconName.compare(QLatin1String("view-financial-transfer")) == 0)
        iconReplacement = "ledger";
      else if (iconName.compare(QLatin1String("view-bank")) == 0)
        iconReplacement = "bank";
      else if (iconName.compare(QLatin1String("view-time-schedule-calculus")) == 0)
        iconReplacement = "budget";
      else if (iconName.compare(QLatin1String("merge")) == 0)
        iconReplacement = "reconcile";
      else
        iconReplacement = "unknown";
      icon = QIcon::fromTheme(iconReplacement);
      if (icon.isNull())
        icon = QIcon::fromTheme("unknown");
    }

    if (!icon.availableSizes().isEmpty())
      result = icon.pixmap(size == 0 ? icon.availableSizes().first() : QSize(size, size));
    QPixmapCache::insert(iconName, result);
  }

  QPainter pixmapPainter(&result);
  icon = QIcon::fromTheme(overlayName);
  if (icon.isNull()) {
    QString overlayReplacement;
    if (overlayName.compare(QLatin1String("document-import")) == 0)
      overlayReplacement = "format-indent-less";
    else if (overlayName.compare(QLatin1String("document-edit")) == 0)
      overlayReplacement = "text-editor";
    else if (overlayName.compare(QLatin1String("dialog-ok-apply")) == 0)
      overlayReplacement = "finish";
    else if (overlayName.compare(QLatin1String("download")) == 0)
      overlayReplacement = "go-down";
    else
      overlayReplacement = "unknown";
    icon = QIcon::fromTheme(overlayReplacement);
    if (icon.isNull())
      icon = QIcon::fromTheme("unknown");
  }

  if (!icon.availableSizes().isEmpty())
    ovly = icon.pixmap(size == 0 ? icon.availableSizes().first() : QSize(size, size));

  switch (corner) {
    case Qt::TopLeftCorner:
      x = 0;
      y = 0;
      break;
    case Qt::TopRightCorner:
      x = result.width() / 2;
      y = 0;
      break;
    case Qt::BottomLeftCorner:
      x = 0;
      y = result.height() / 2;
      break;
    case Qt::BottomRightCorner:
    default:
      x = result.width() / 2;
      y = result.height() / 2;
      break;
  }
  pixmapPainter.drawPixmap(x, y, result.width() / 2, result.height() / 2, ovly);

  //save for later use
  QPixmapCache::insert(overlaidIcon, result);

  return result;
}

void KMyMoneyUtils::dissectTransaction(const MyMoneyTransaction& transaction, const MyMoneySplit& split, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency, MyMoneySplit::investTransactionTypeE& transactionType)
{
  // collect the splits. split references the stock account and should already
  // be set up. assetAccountSplit references the corresponding asset account (maybe
  // empty), feeSplits is the list of all expenses and interestSplits
  // the list of all incomes
  assetAccountSplit = MyMoneySplit(); // set to none to check later if it was assigned
  MyMoneyFile* file = MyMoneyFile::instance();
  QList<MyMoneySplit>::ConstIterator it_s;
  for (it_s = transaction.splits().begin(); it_s != transaction.splits().end(); ++it_s) {
    MyMoneyAccount acc = file->account((*it_s).accountId());
    if ((*it_s).id() == split.id()) {
      security = file->security(acc.currencyId());
    } else if (acc.accountGroup() == MyMoneyAccount::Expense) {
      feeSplits.append(*it_s);
      // feeAmount += (*it_s).value();
    } else if (acc.accountGroup() == MyMoneyAccount::Income) {
      interestSplits.append(*it_s);
      // interestAmount += (*it_s).value();
    } else {
      if (assetAccountSplit == MyMoneySplit()) // first asset Account should be our requested brokerage account
        assetAccountSplit = *it_s;
      else if ((*it_s).value().isNegative())  // the rest (if present) is handled as fee or interest
        feeSplits.append(*it_s);              // and shouldn't be allowed to override assetAccountSplit
      else if ((*it_s).value().isPositive())
        interestSplits.append(*it_s);
    }
  }

  // determine transaction type
  if (split.action() == MyMoneySplit::ActionAddShares) {
    transactionType = (!split.shares().isNegative()) ? MyMoneySplit::AddShares : MyMoneySplit::RemoveShares;
  } else if (split.action() == MyMoneySplit::ActionBuyShares) {
    transactionType = (!split.value().isNegative()) ? MyMoneySplit::BuyShares : MyMoneySplit::SellShares;
  } else if (split.action() == MyMoneySplit::ActionDividend) {
    transactionType = MyMoneySplit::Dividend;
  } else if (split.action() == MyMoneySplit::ActionReinvestDividend) {
    transactionType = MyMoneySplit::ReinvestDividend;
  } else if (split.action() == MyMoneySplit::ActionYield) {
    transactionType = MyMoneySplit::Yield;
  } else if (split.action() == MyMoneySplit::ActionSplitShares) {
    transactionType = MyMoneySplit::SplitShares;
  } else if (split.action() == MyMoneySplit::ActionInterestIncome) {
    transactionType = MyMoneySplit::InterestIncome;
  } else
    transactionType = MyMoneySplit::BuyShares;

  currency.setTradingSymbol("???");
  try {
    currency = file->security(transaction.commodity());
  } catch (const MyMoneyException &) {
  }
}
