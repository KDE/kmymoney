/***************************************************************************
 *   Copyright 2009  Cristian Onet onet.cristian@gmail.com                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>  *
 ***************************************************************************/

#include "reconciliationreport.h"

// KDE includes
#include <KPluginFactory>
#include <KAction>
#include <KGlobal>
#include <KGlobalSettings>
#include <KLocale>
#include <KStandardDirs>
#include <KPluginInfo>
#include <KColorScheme>

// KMyMoney includes
#include "mymoneyfile.h"
#include "pluginloader.h"

#include "kreconciliationreportdlg.h"

K_PLUGIN_FACTORY(ReconciliationReportFactory, registerPlugin<KMMReconciliationReportPlugin>();)
K_EXPORT_PLUGIN(ReconciliationReportFactory("kmm_reconciliationreport", "kmymoney"))

KMMReconciliationReportPlugin::KMMReconciliationReportPlugin(QObject *parent, const QVariantList&)
    : KMyMoneyPlugin::Plugin(parent, "Reconciliation report"/*must be the same as X-KDE-PluginInfo-Name*/)
{
  // For ease announce that we have been loaded.
  qDebug("KMyMoney reconciliation report plugin loaded");

  // connect to the pluginloader's relevant signals
  connect(KMyMoneyPlugin::PluginLoader::instance(), SIGNAL(plug(KPluginInfo*)), this, SLOT(slotPlug(KPluginInfo*)));
  connect(KMyMoneyPlugin::PluginLoader::instance(), SIGNAL(unplug(KPluginInfo*)), this, SLOT(slotUnplug(KPluginInfo*)));

}

void KMMReconciliationReportPlugin::slotGenerateReconciliationReport(const MyMoneyAccount& account, const QDate& date, const MyMoneyMoney& startingBalance, const MyMoneyMoney& endingBalance, const QList<QPair<MyMoneyTransaction, MyMoneySplit> >& transactionList)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneySecurity currency = file->currency(account.currencyId());

  QString filename;
  if (!MyMoneyFile::instance()->value("reportstylesheet").isEmpty())
    filename = KGlobal::dirs()->findResource("apps/kmymoney", QString("html/%1").arg(MyMoneyFile::instance()->value("reportstylesheet")));
  if (filename.isEmpty())
    filename = KGlobal::dirs()->findResource("apps/kmymoney", "html/kmymoney.css");
  QString header = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\">\n") +
                   QString("<html><head><link rel=\"stylesheet\" type=\"text/css\" href=\"%1\">").arg(filename);

  header += "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />";

  QColor tcolor = KColorScheme(QPalette::Active).foreground(KColorScheme::NormalText).color();
  QString css;
  css += "<style type=\"text/css\">\n<!--\n"
      +  QString(".row-even, .item0 { background-color: %1; color: %2 }\n")
         .arg((KColorScheme(QPalette::Normal).background(KColorScheme::AlternateBackground).color()).name()).arg(tcolor.name())
      +  QString(".row-odd, .item1  { background-color: %1; color: %2 }\n")
         .arg((KColorScheme(QPalette::Normal).background(KColorScheme::NormalBackground).color()).name()).arg(tcolor.name())
      +  "-->\n</style>\n";
  header += css;

  header += "</head><body>\n";

  QString footer = "</body></html>\n";

  MyMoneyMoney clearedBalance = startingBalance;
  MyMoneyMoney clearedDepositAmount, clearedPaymentAmount;
  int clearedDeposits = 0;
  int clearedPayments = 0;
  MyMoneyMoney outstandingDepositAmount, outstandingPaymentAmount;
  int outstandingDeposits = 0;
  int outstandingPayments = 0;
  QList<QPair<MyMoneyTransaction, MyMoneySplit> >::const_iterator it;
  for (it = transactionList.begin(); it != transactionList.end(); ++it) {
    // if this split is a stock split, we can't just add the amount of shares
    if ((*it).second.reconcileFlag() == MyMoneySplit::NotReconciled) {
      if ((*it).second.shares().isNegative()) {
        outstandingPayments++;
        outstandingPaymentAmount += (*it).second.shares();
      } else {
        outstandingDeposits++;
        outstandingDepositAmount += (*it).second.shares();
      }
    } else {
      if ((*it).second.shares().isNegative()) {
        clearedPayments++;
        clearedPaymentAmount += (*it).second.shares();
      } else {
        clearedDeposits++;
        clearedDepositAmount += (*it).second.shares();
      }
      clearedBalance += (*it).second.shares();
    }
  }

  QString reportName = i18n("Reconciliation report of account %1", account.name());
  QString report = QString("<h2 class=\"report\">%1</h2>\n").arg(reportName);
  report += QString("<div class=\"subtitle\">");
  report += QString("%1").arg(KGlobal::locale()->formatDate(date, KLocale::ShortDate));
  report += QString("</div>\n");
  report += QString("<div class=\"gap\">&nbsp;</div>\n");
  report += QString("<div class=\"subtitle\">");
  report += i18n("All values shown in %1", file->baseCurrency().name());
  report += QString("</div>\n");
  report += QString("<div class=\"gap\">&nbsp;</div>\n");

  report += "<table class=\"report\">\n<thead><tr class=\"itemheader\">";
  report += "<th>" + i18n("Summary") + "</th>";
  report += "</tr></thead>\n";

  // row 1
  report += "<tr class=\"row-odd\"><td class=\"left\">";
  report += i18n("Starting balance on bank statement");
  report += "</td><td>";
  report += MyMoneyUtils::formatMoney(startingBalance, currency);
  report += "</td></tr>";
  // row 2
  report += "<tr class=\"row-even\"><td class=\"left\">";
  report += i18np("%1 cleared payment", "%1 cleared payments in total", clearedPayments);
  report += "</td><td>";
  report += MyMoneyUtils::formatMoney(clearedPaymentAmount, currency);
  report += "</td></tr>";
  // row 3
  report += "<tr class=\"row-odd\"><td class=\"left\">";
  if (account.accountType() == MyMoneyAccount::CreditCard)
    report += i18np("%1 cleared charge", "%1 cleared charges in total", clearedDeposits);
  else
    report += i18np("%1 cleared deposit", "%1 cleared deposits in total", clearedDeposits);
  report += "</td><td>";
  report += MyMoneyUtils::formatMoney(clearedDepositAmount, currency);
  report += "</td></tr>";
  // row 4
  report += "<tr class=\"row-even\"><td class=\"left\">";
  report += i18n("Ending balance on bank statement");
  report += "</td><td>";
  report += MyMoneyUtils::formatMoney(endingBalance, currency);
  report += "</td></tr>";

  // separator
  report += "<tr class=\"spacer\"><td colspan=\"2\"> </td></tr>";

  // row 5
  report += "<tr class=\"row-odd\"><td class=\"left\">";
  report += i18n("Cleared balance");
  report += "</td><td>";
  report += MyMoneyUtils::formatMoney(clearedBalance, currency);
  report += "</td></tr>";
  // row 6
  report += "<tr class=\"row-even\"><td class=\"left\">";
  report += i18np("%1 outstanding payment", "%1 outstanding payments in total", outstandingPayments);
  report += "</td><td>";
  report += MyMoneyUtils::formatMoney(outstandingPaymentAmount, currency);
  report += "</td></tr>";
  // row 7
  report += "<tr class=\"row-odd\"><td class=\"left\">";
  if (account.accountType() == MyMoneyAccount::CreditCard)
    report += i18np("%1 outstanding charge", "%1 outstanding charges in total", outstandingDeposits);
  else
    report += i18np("%1 outstanding deposit", "%1 outstanding deposits in total", outstandingDeposits);
  report += "</td><td>";
  report += MyMoneyUtils::formatMoney(outstandingDepositAmount, currency);
  report += "</td></tr>";
  // row 8
  report += "<tr class=\"row-even\"><td class=\"left\">";
  report += i18n("Register balance as of %1", KGlobal::locale()->formatDate(date, KLocale::ShortDate));
  report += "</td><td>";
  report += MyMoneyUtils::formatMoney(MyMoneyFile::instance()->balance(account.id(), date), currency);
  report += "</td></tr>";

  // retrieve list of all transactions after the reconciliation date that are not reconciled or cleared
  QList<QPair<MyMoneyTransaction, MyMoneySplit> > afterTransactionList;
  MyMoneyTransactionFilter filter(account.id());
  filter.addState(MyMoneyTransactionFilter::cleared);
  filter.addState(MyMoneyTransactionFilter::notReconciled);
  filter.setDateFilter(date.addDays(1), QDate());
  filter.setConsiderCategory(false);
  filter.setReportAllSplits(true);
  file->transactionList(afterTransactionList, filter);

  MyMoneyMoney afterDepositAmount, afterPaymentAmount;
  int afterDeposits = 0;
  int afterPayments = 0;
  for (it = afterTransactionList.constBegin(); it != afterTransactionList.constEnd(); ++it) {
    // if this split is a stock split, we can't just add the amount of shares
    if ((*it).second.reconcileFlag() == MyMoneySplit::NotReconciled) {
      if ((*it).second.shares().isNegative()) {
        afterPayments++;
        afterPaymentAmount += (*it).second.shares();
      } else {
        afterDeposits++;
        afterDepositAmount += (*it).second.shares();
      }
    }
  }

  // row 9
  report += "<tr class=\"row-odd\"><td class=\"left\">";
  report += i18np("%1 payment after %2", "%1 payments after %2", afterPayments, KGlobal::locale()->formatDate(date, KLocale::ShortDate));
  report += "</td><td>";
  report += MyMoneyUtils::formatMoney(afterPaymentAmount, currency);
  report += "</td></tr>";
  // row 10
  report += "<tr class=\"row-even\"><td class=\"left\">";
  if (account.accountType() == MyMoneyAccount::CreditCard)
    report += i18np("%1 charge after %2", "%1 charges after %2", afterDeposits, KGlobal::locale()->formatDate(date, KLocale::ShortDate));
  else
    report += i18np("%1 deposit after %2", "%1 deposits after %2", afterDeposits, KGlobal::locale()->formatDate(date, KLocale::ShortDate));
  report += "</td><td>";
  report += MyMoneyUtils::formatMoney(afterDepositAmount, currency);
  report += "</td></tr>";
  // row 11
  report += "<tr class=\"row-odd\"><td class=\"left\">";
  report += i18n("Register ending balance");
  report += "</td><td>";
  report += MyMoneyUtils::formatMoney(MyMoneyFile::instance()->balance(account.id()), currency);
  report += "</td></tr>";

  // end of the table
  report += "</table>\n";

  QString detailsTableHeader;
  detailsTableHeader += "<table class=\"report\">\n<thead><tr class=\"itemheader\">";
  detailsTableHeader += "<th>" + i18n("Date") + "</th>";
  detailsTableHeader += "<th>" + i18n("Number") + "</th>";
  detailsTableHeader += "<th>" + i18n("Payee") + "</th>";
  detailsTableHeader += "<th>" + i18n("Memo") + "</th>";
  detailsTableHeader += "<th>" + i18n("Category") + "</th>";
  detailsTableHeader += "<th>" + i18n("Amount") + "</th>";
  detailsTableHeader += "</tr></thead>\n";


  QString detailsReport = QString("<h2 class=\"report\">%1</h2>\n").arg(i18n("Outstanding payments"));
  detailsReport += detailsTableHeader;

  int index = 0;
  for (it = transactionList.begin(); it != transactionList.end(); ++it) {
    if ((*it).second.reconcileFlag() == MyMoneySplit::NotReconciled && (*it).second.shares().isNegative()) {
      QList<MyMoneySplit>::const_iterator it_s;
      QString category;
      for (it_s = (*it).first.splits().begin(); it_s != (*it).first.splits().end(); ++it_s) {
        if ((*it_s).accountId() != account.id()) {
          if (!category.isEmpty())
            category += ", "; // this is a split transaction
          category = file->account((*it_s).accountId()).name();
        }
      }

      detailsReport += QString("<tr class=\"%1\"><td>").arg((index++ % 2 == 1) ? "row-odd" : "row-even");
      detailsReport += QString("%1").arg(KGlobal::locale()->formatDate((*it).first.entryDate(), KLocale::ShortDate));
      detailsReport += "</td><td>";
      detailsReport += QString("%1").arg((*it).second.number());
      detailsReport += "</td><td>";
      detailsReport += QString("%1").arg(file->payee((*it).second.payeeId()).name());
      detailsReport += "</td><td>";
      detailsReport += QString("%1").arg((*it).first.memo());
      detailsReport += "</td><td>";
      detailsReport += QString("%1").arg(category);
      detailsReport += "</td><td>";
      detailsReport += QString("%1").arg(MyMoneyUtils::formatMoney((*it).second.shares(), file->currency(account.currencyId())));
      detailsReport += "</td></tr>";
    }
  }

  detailsReport += "<tr class=\"sectionfooter\">";
  detailsReport += QString("<td class=\"left1\" colspan=\"5\">%1</td><td>%2</td></tr>").arg(i18np("One outstanding payment of", "Total of %1 outstanding payments amounting to", outstandingPayments)).arg(MyMoneyUtils::formatMoney(outstandingPaymentAmount, currency));

  detailsReport += "</table>\n";
  detailsReport += QString("<h2 class=\"report\">%1</h2>\n").arg(
    account.accountType() == MyMoneyAccount::CreditCard ? i18n("Outstanding charges") : i18n("Outstanding deposits"));
  detailsReport += detailsTableHeader;

  index = 0;
  for (it = transactionList.begin(); it != transactionList.end(); ++it) {
    if ((*it).second.reconcileFlag() == MyMoneySplit::NotReconciled && !(*it).second.shares().isNegative()) {
      QList<MyMoneySplit>::const_iterator it_s;
      QString category;
      for (it_s = (*it).first.splits().begin(); it_s != (*it).first.splits().end(); ++it_s) {
        if ((*it_s).accountId() != account.id()) {
          if (!category.isEmpty())
            category += ", "; // this is a split transaction
          category = file->account((*it_s).accountId()).name();
        }
      }

      detailsReport += QString("<tr class=\"%1\"><td>").arg((index++ % 2 == 1) ? "row-odd" : "row-even");
      detailsReport += QString("%1").arg(KGlobal::locale()->formatDate((*it).first.entryDate(), KLocale::ShortDate));
      detailsReport += "</td><td>";
      detailsReport += QString("%1").arg((*it).second.number());
      detailsReport += "</td><td>";
      detailsReport += QString("%1").arg(file->payee((*it).second.payeeId()).name());
      detailsReport += "</td><td>";
      detailsReport += QString("%1").arg((*it).first.memo());
      detailsReport += "</td><td>";
      detailsReport += QString("%1").arg(category);
      detailsReport += "</td><td>";
      detailsReport += QString("%1").arg(MyMoneyUtils::formatMoney((*it).second.shares(), file->currency(account.currencyId())));
      detailsReport += "</td></tr>";
    }
  }

  detailsReport += "<tr class=\"sectionfooter\">";
  detailsReport += QString("<td class=\"left1\" colspan=\"5\">%1</td><td>%2</td></tr>")
                     .arg(account.accountType() == MyMoneyAccount::CreditCard ?
                            i18np("One outstanding charge of", "Total of %1 outstanding charges amounting to", outstandingDeposits) :
                            i18np("One outstanding deposit of", "Total of %1 outstanding deposits amounting to", outstandingDeposits))
                     .arg(MyMoneyUtils::formatMoney(outstandingDepositAmount, currency));

  // end of the table
  detailsReport += "</table>\n";

  QPointer<KReportDlg> dlg = new KReportDlg(0, header + report + footer, header + detailsReport + footer);
  dlg->exec();
  delete dlg;
}

void KMMReconciliationReportPlugin::slotPlug(KPluginInfo* info)
{
  if (info->pluginName() == objectName()) {
    connect(viewInterface(), SIGNAL(accountReconciled(MyMoneyAccount,QDate,MyMoneyMoney,MyMoneyMoney,QList<QPair<MyMoneyTransaction,MyMoneySplit> >)), this, SLOT(slotGenerateReconciliationReport(MyMoneyAccount,QDate,MyMoneyMoney,MyMoneyMoney,QList<QPair<MyMoneyTransaction,MyMoneySplit> >)));
  }
}

void KMMReconciliationReportPlugin::slotUnplug(KPluginInfo* info)
{
  if (info->pluginName() == objectName()) {
    disconnect(viewInterface(), SIGNAL(accountReconciled(MyMoneyAccount,QDate,MyMoneyMoney,MyMoneyMoney,QList<QPair<MyMoneyTransaction,MyMoneySplit> >)), this, SLOT(slotGenerateReconciliationReport(MyMoneyAccount,QDate,MyMoneyMoney,MyMoneyMoney,QList<QPair<MyMoneyTransaction,MyMoneySplit> >)));
  }
}
