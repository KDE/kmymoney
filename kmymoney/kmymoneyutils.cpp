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

// ----------------------------------------------------------------------------
// KDE Headers

#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <KGlobalSettings>
#include <KColorScheme>
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

  if(tmp == i18n("Checking").toLower())
    rc = MyMoneyAccount::Checkings;
  else if(tmp == i18n("Savings").toLower())
    rc = MyMoneyAccount::Savings;
  else if(tmp == i18n("Credit Card").toLower())
    rc = MyMoneyAccount::CreditCard;
  else if(tmp == i18n("Cash").toLower())
    rc = MyMoneyAccount::Cash;
  else if(tmp == i18n("Loan").toLower())
    rc = MyMoneyAccount::Loan;
  else if(tmp == i18n("Certificate of Deposit").toLower())
    rc = MyMoneyAccount::CertificateDep;
  else if(tmp == i18n("Investment").toLower())
    rc = MyMoneyAccount::Investment;
  else if(tmp == i18n("Money Market").toLower())
    rc = MyMoneyAccount::MoneyMarket;
  else if(tmp == i18n("Asset").toLower())
    rc = MyMoneyAccount::Asset;
  else if(tmp == i18n("Liability").toLower())
    rc = MyMoneyAccount::Liability;
  else if(tmp == i18n("Currency").toLower())
    rc = MyMoneyAccount::Currency;
  else if(tmp == i18n("Income").toLower())
    rc = MyMoneyAccount::Income;
  else if(tmp == i18n("Expense").toLower())
    rc = MyMoneyAccount::Expense;
  else if(tmp == i18n("Investment Loan").toLower())
    rc = MyMoneyAccount::AssetLoan;
  else if(tmp == i18n("Stock").toLower())
    rc = MyMoneyAccount::Stock;
  else if(tmp == i18n("Equity").toLower())
    rc = MyMoneyAccount::Equity;

  return rc;
}

MyMoneySecurity::eSECURITYTYPE KMyMoneyUtils::stringToSecurity(const QString& txt)
{
  MyMoneySecurity::eSECURITYTYPE rc = MyMoneySecurity::SECURITY_NONE;
  QString tmp = txt.toLower();

  if(tmp == i18n("Stock").toLower())
    rc = MyMoneySecurity::SECURITY_STOCK;
  else if(tmp == i18n("Mutual Fund").toLower())
    rc = MyMoneySecurity::SECURITY_MUTUALFUND;
  else if(tmp == i18n("Bond").toLower())
    rc = MyMoneySecurity::SECURITY_BOND;
  else if(tmp == i18n("Currency").toLower())
    rc = MyMoneySecurity::SECURITY_CURRENCY;

  return rc;
}

const QString KMyMoneyUtils::securityTypeToString(const MyMoneySecurity::eSECURITYTYPE securityType)
{
  return i18n(MyMoneySecurity::securityTypeToString(securityType).toLatin1());
}

const QString KMyMoneyUtils::occurrenceToString(const MyMoneySchedule::occurrenceE occurrence)
{
  return i18n(MyMoneySchedule::occurrenceToString(occurrence).toLatin1());
}

const QString KMyMoneyUtils::paymentMethodToString(MyMoneySchedule::paymentTypeE paymentType)
{
  return i18n(MyMoneySchedule::paymentMethodToString(paymentType).toLatin1());
}

const QString KMyMoneyUtils::weekendOptionToString(MyMoneySchedule::weekendOptionE weekendOption)
{
  return i18n(MyMoneySchedule::weekendOptionToString(weekendOption).toLatin1());
}

const QString KMyMoneyUtils::scheduleTypeToString(MyMoneySchedule::typeE type)
{
  return i18n(MyMoneySchedule::scheduleTypeToString(type).toLatin1());
}

KGuiItem KMyMoneyUtils::scheduleNewGuiItem(void)
{
  KGuiItem splitGuiItem(  i18n("&New Schedule..."),
                          KIcon("document-new"),
                          i18n("Create a new schedule."),
                          i18n("Use this to create a new schedule."));

  return splitGuiItem;
}

KGuiItem KMyMoneyUtils::accountsFilterGuiItem(void)
{
  KGuiItem splitGuiItem(  i18n("&Filter"),
                          KIcon("view-filter"),
                          i18n("Filter out accounts"),
                          i18n("Use this to filter out accounts"));

  return splitGuiItem;
}

QPixmap KMyMoneyUtils::billScheduleIcon(int size)
{
  KIconLoader *ic = KIconLoader::global();
  return ic->loadIcon("billschedule", KIconLoader::User, size);
}

QPixmap KMyMoneyUtils::depositScheduleIcon(int size)
{
  KIconLoader *ic = KIconLoader::global();
  return ic->loadIcon("depositschedule", KIconLoader::User, size);
}

QPixmap KMyMoneyUtils::transferScheduleIcon(int size)
{
  KIconLoader *ic = KIconLoader::global();
  return ic->loadIcon("transferschedule", KIconLoader::User, size);
}

QPixmap KMyMoneyUtils::scheduleIcon(int size)
{
  KIconLoader *ic = KIconLoader::global();
  return ic->loadIcon("schedule", KIconLoader::User, size);
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
  if(abs(idx) > 0 && abs(idx) < static_cast<int>(sizeof(homePageItems)/sizeof(homePageItems[0]))) {
    rc = i18n(homePageItems[abs(idx-1)]);
  }
  return rc;
}

int KMyMoneyUtils::stringToHomePageItem(const QString& txt)
{
  int idx = 0;
  for(idx = 0; homePageItems[idx] != 0; ++idx) {
    if(txt == i18n(homePageItems[idx]))
      return idx+1;
  }
  return 0;
}

bool KMyMoneyUtils::appendCorrectFileExt(QString& str, const QString& strExtToUse)
{
  bool rc = false;

  if(!str.isEmpty()) {
    //find last . delminator
    int nLoc = str.lastIndexOf('.');
    if(nLoc != -1) {
      QString strExt, strTemp;
      strTemp = str.left(nLoc + 1);
      strExt = str.right(str.length() - (nLoc + 1));
      if(strExt.indexOf(strExtToUse, 0, Qt::CaseInsensitive) == -1) {
        // if the extension given contains a period, we remove our's
        if(strExtToUse.indexOf('.') != -1)
          strTemp = strTemp.left(strTemp.length()-1);
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

void KMyMoneyUtils::checkConstants(void)
{
  Q_ASSERT(static_cast<int>(KLocale::ParensAround) == static_cast<int>(MyMoneyMoney::ParensAround));
  Q_ASSERT(static_cast<int>(KLocale::BeforeQuantityMoney) == static_cast<int>(MyMoneyMoney::BeforeQuantityMoney));
  Q_ASSERT(static_cast<int>(KLocale::AfterQuantityMoney) == static_cast<int>(MyMoneyMoney::AfterQuantityMoney));
  Q_ASSERT(static_cast<int>(KLocale::BeforeMoney) == static_cast<int>(MyMoneyMoney::BeforeMoney));
  Q_ASSERT(static_cast<int>(KLocale::AfterMoney) == static_cast<int>(MyMoneyMoney::AfterMoney));
}

QString KMyMoneyUtils::variableCSS(void)
{
  QColor tcolor = KColorScheme(QPalette::Active).foreground(KColorScheme::NormalText).color();

  QString css;
  css += "<style type=\"text/css\">\n<!--\n";
  css += QString(".row-even, .item0 { background-color: %1; color: %2 }\n")
    .arg((KMyMoneyGlobalSettings::listBGColor()).name()).arg(tcolor.name());
  css += QString(".row-odd, .item1  { background-color: %1; color: %2 }\n")
    .arg((KMyMoneyGlobalSettings::listColor()).name()).arg(tcolor.name());
  css += "-->\n</style>\n";
  return css;
}

QString KMyMoneyUtils::findResource(const char* type, const QString& filename)
{
  QString language = KGlobal::locale()->language();
  QString country = KGlobal::locale()->country();
  QString rc, mask;

  // check that the placeholder is present
  if(!filename.indexOf("%1")) {
    qWarning("%%1 not found in '%s'", qPrintable(filename));
    return filename;
  }

  // search the given resource
  mask = filename.arg("_%1.%2");
  rc = KGlobal::dirs()->findResource(type, mask.arg(country).arg(language));
  if(rc.isEmpty()) {
    mask = filename.arg("_%1");
    rc = KGlobal::dirs()->findResource(type, mask.arg(language));
  }
  if(rc.isEmpty()) {
    // qDebug(QString("html/home_%1.html not found").arg(country).toLatin1());
    rc = KGlobal::dirs()->findResource(type, mask.arg(country));
  }
  if(rc.isEmpty()) {
    rc = KGlobal::dirs()->findResource(type, filename.arg(""));
  }

  if(rc.isEmpty()) {
    qWarning("No resource found for (%s,%s)", type, qPrintable(filename));
  }
  return rc;
}

const MyMoneySplit KMyMoneyUtils::stockSplit(const MyMoneyTransaction& t)
{
  QList<MyMoneySplit>::ConstIterator it_s;
  MyMoneySplit investmentAccountSplit;
  for(it_s = t.splits().begin(); it_s != t.splits().end(); ++it_s) {
    if(!(*it_s).accountId().isEmpty()) {
      MyMoneyAccount acc = MyMoneyFile::instance()->account((*it_s).accountId());
      if(acc.isInvest()) {
        return *it_s;
      }
      // if we have a reference to an investment account, we remember it here
      if(acc.accountType() == MyMoneyAccount::Investment)
        investmentAccountSplit = *it_s;
    }
  }
  // if we haven't found a stock split, we see if we've seen
  // an investment account on the way. If so, we return it.
  if(!investmentAccountSplit.id().isEmpty())
    return investmentAccountSplit;

  // if none was found, we return an empty split.
  return MyMoneySplit();
}

KMyMoneyUtils::transactionTypeE KMyMoneyUtils::transactionType(const MyMoneyTransaction& t)
{
  if(!stockSplit(t).id().isEmpty())
    return InvestmentTransaction;

  if(t.splitCount() < 2) {
    return Unknown;
  } else if(t.splitCount() > 2) {
    // FIXME check for loan transaction here
    return SplitTransaction;
  }
  QString ida, idb;
  if (t.splits().size() > 0)
    ida = t.splits()[0].accountId();
  if (t.splits().size() > 1)
    idb = t.splits()[1].accountId();
  if(ida.isEmpty() || idb.isEmpty())
    return Unknown;

  MyMoneyAccount a, b;
  a = MyMoneyFile::instance()->account(ida);
  b = MyMoneyFile::instance()->account(idb);
  if((a.accountGroup() == MyMoneyAccount::Asset
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
  } catch (MyMoneyException* e) {
    KMessageBox::detailedError(0, i18n("Unable to load schedule details"), e->what());
    delete e;
  }
}

QString KMyMoneyUtils::nextCheckNumber(const MyMoneyAccount& acc)
{
  // determine next check number
  QString number;
  QRegExp exp(QString("(.*\\D)?(\\d+)(\\D.*)?"));
  if(exp.indexIn(acc.value("lastNumberUsed")) != -1) {
    number = QString("%1%2%3").arg(exp.cap(1)).arg(exp.cap(2).toULongLong() + 1).arg(exp.cap(3));
  } else {
    number = '1';
  }
  return number;
}

QString KMyMoneyUtils::reconcileStateToString(MyMoneySplit::reconcileFlagE flag, bool text)
{
  QString txt;
  if(text) {
    switch(flag) {
      case MyMoneySplit::NotReconciled:
        txt = i18nc("Reconcile state 'Not reconciled'", "Not reconciled");
        break;
      case MyMoneySplit::Cleared:
        txt = i18nc("Reconcile state 'Cleared'", "Cleared");
        break;
      case MyMoneySplit::Reconciled:
        txt = i18nc("Reconcile state 'Reconciled'", "Reconciled");
        break;
      case MyMoneySplit::Frozen:
        txt = i18nc("Reconcile state 'Frozen'", "Frozen");
        break;
      default:
        txt = i18nc("Unknown reconciliation state", "Unknown");
        break;
    }
  } else {
    switch(flag) {
      case MyMoneySplit::NotReconciled:
        break;
      case MyMoneySplit::Cleared:
        txt = i18nc("Reconcile flag C", "C");
        break;
      case MyMoneySplit::Reconciled:
        txt = i18nc("Reconcile flag R", "R");
        break;
      case MyMoneySplit::Frozen:
        txt = i18nc("Reconcile flag F", "F");
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
  } catch (MyMoneyException* e) {
    qDebug("Unable to load schedule details for '%s' during transaction match: %s", qPrintable(schedule.name()), qPrintable(e->what()));
    delete e;
  }

  t.clearId();
  t.setEntryDate(QDate());
  return t;
}

void KMyMoneyUtils::previouslyUsedCategories(const QString& investmentAccount, QString& feesId, QString& interestId)
{
  feesId = interestId = QString();
  MyMoneyFile* file = MyMoneyFile::instance();
  try {
    MyMoneyAccount acc = file->account(investmentAccount);
    MyMoneyTransactionFilter filter(investmentAccount);
    filter.setReportAllSplits(false);
    // since we assume an investment account here, we need to collect the stock accounts as well
    filter.addAccount(acc.accountList());
    QList< QPair<MyMoneyTransaction, MyMoneySplit> > list;
    file->transactionList(list, filter);
    QList< QPair<MyMoneyTransaction, MyMoneySplit> >::const_iterator it_t;
    for(it_t = list.constBegin(); it_t != list.constEnd(); ++it_t) {
      const MyMoneyTransaction& t = (*it_t).first;
      const MyMoneySplit&s = (*it_t).second;
      MyMoneySplit assetAccountSplit;
      QList<MyMoneySplit> feeSplits;
      QList<MyMoneySplit> interestSplits;
      MyMoneySecurity security;
      MyMoneySecurity currency;
      MyMoneySplit::investTransactionTypeE transactionType;
      InvestTransactionEditor::dissectTransaction(t, s, assetAccountSplit, feeSplits, interestSplits, security, currency, transactionType);
      if(feeSplits.count() == 1) {
        feesId = feeSplits.first().accountId();
      }
      if(interestSplits.count() == 1) {
        interestId = interestSplits.first().accountId();
      }
    }
  } catch(MyMoneyException *e) {
    delete e;
  }

}

QPixmap KMyMoneyUtils::accountGroupPixmap(const MyMoneyAccount& account, bool reconcileFlag)
{
  QString icon;
  switch (account.accountGroup())
  {
    case MyMoneyAccount::Income:
      icon = "account-types_income";
      break;
    case MyMoneyAccount::Expense:
      icon = "account-types_expense";
      break;
    case MyMoneyAccount::Liability:
      icon = "account-types_liability";
      break;
    case MyMoneyAccount::Asset:
      icon = "account-types_asset";
      break;
    default:
      icon = "account";
      break;
  }
  QPixmap result = QPixmap(KGlobal::dirs()->findResource("appdata", QString( "icons/hicolor/22x22/actions/%1.png").arg(icon)));
  if(account.isClosed()) {
    QPixmap overlay = QPixmap(KGlobal::dirs()->findResource("appdata", QString( "icons/hicolor/22x22/actions/account-types_closed.png")));
    QPainter pixmapPainter(&result);
    pixmapPainter.drawPixmap(0, 0, overlay, 0, 0, overlay.width(), overlay.height());
  } else if(reconcileFlag) {
    QPixmap overlay = QPixmap(KGlobal::dirs()->findResource("appdata", QString( "icons/hicolor/22x22/actions/account-types_reconciled.png")));
    QPainter pixmapPainter(&result);
    pixmapPainter.drawPixmap(0, 0, overlay, 0, 0, overlay.width(), overlay.height());
  } else if(!account.onlineBankingSettings().value("provider").isEmpty()) {
    QPixmap overlay = QPixmap(KGlobal::dirs()->findResource("appdata", QString( "icons/hicolor/22x22/actions/account-types_online.png")));
    QPainter pixmapPainter(&result);
    pixmapPainter.drawPixmap(0, 0, overlay, 0, 0, overlay.width(), overlay.height());
  }
  return result;
}

KXmlGuiWindow* KMyMoneyUtils::mainWindow() {
  foreach (QWidget *widget, QApplication::topLevelWidgets()) {
    KXmlGuiWindow* result = dynamic_cast<KXmlGuiWindow*>(widget);
    if (result)
      return result;
  }
  return 0;
}
