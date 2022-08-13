/*
    SPDX-FileCopyrightText: 2009 Cristian Onet onet.cristian @gmail.com
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-FileCopyrightText: 2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "reconciliationreport.h"

//! @todo remove
#include <QDate>
#include <QDebug>
#include <QFile>
#include <QPointer>
#include <QUrl>

// KDE includes
#include <KColorScheme>
#include <KLocalizedString>
#include <KPluginFactory>

// KMyMoney includes
#include "journalmodel.h"
#include "mymoneyaccount.h"
#include "mymoneyenums.h"
#include "mymoneyfile.h"
#include "mymoneypayee.h"
#include "mymoneysecurity.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "mymoneytransactionfilter.h"
#include "mymoneyutils.h"
#include "viewinterface.h"
#include <kmymoneyutils.h>

#include "kreconciliationreportdlg.h"

ReconciliationReport::ReconciliationReport(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args) :
    KMyMoneyPlugin::Plugin(parent, metaData, args)
{
    qDebug("Plugins: reconciliation report loaded");
}

ReconciliationReport::~ReconciliationReport()
{
    qDebug("Plugins: reconciliation report unloaded");
}

void ReconciliationReport::plug(KXMLGUIFactory* guiFactory)
{
    Q_UNUSED(guiFactory)
    connect(viewInterface(), &KMyMoneyPlugin::ViewInterface::accountReconciled, this, &ReconciliationReport::slotGenerateReconciliationReport);
//  qDebug() << "Connect was done" << viewInterface();
}

void ReconciliationReport::unplug()
{
    disconnect(viewInterface(), &KMyMoneyPlugin::ViewInterface::accountReconciled, this, &ReconciliationReport::slotGenerateReconciliationReport);
}

void ReconciliationReport::slotGenerateReconciliationReport(const MyMoneyAccount& account,
                                                            const QDate& date,
                                                            const MyMoneyMoney& startingBalance,
                                                            const MyMoneyMoney& endingBalance,
                                                            const QStringList& transactionList)
{
    MyMoneyFile* file = MyMoneyFile::instance();
    MyMoneySecurity currency = file->currency(account.currencyId());

    QString filename;
    QString header = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\">\n<html><head>\n");

    // inline the CSS
    header += "<style type=\"text/css\">\n";

    if (!MyMoneyFile::instance()->value("reportstylesheet").isEmpty())
        header +=
            KMyMoneyUtils::getStylesheet(QUrl::fromLocalFile(QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                                                    QString("html/%1").arg(MyMoneyFile::instance()->value("reportstylesheet"))))
                                             .url());
    else
        header += KMyMoneyUtils::getStylesheet();

    header += "</style>\n";
    header += "</head><body id=\"summaryview\">\n";

    QString footer = "</body></html>\n";

    MyMoneyMoney clearedBalance = startingBalance;
    MyMoneyMoney clearedDepositAmount, clearedPaymentAmount;
    int clearedDeposits = 0;
    int clearedPayments = 0;
    MyMoneyMoney outstandingDepositAmount, outstandingPaymentAmount;
    int outstandingDeposits = 0;
    int outstandingPayments = 0;
    for (const auto& journalEntryId : transactionList) {
        // if this split is a stock split, we can't just add the amount of shares
        const auto journalEntry = file->journalModel()->itemById(journalEntryId);
        if (!journalEntry.id().isEmpty()) {
            const auto& split = journalEntry.split();
            if (split.reconcileFlag() == eMyMoney::Split::State::NotReconciled) {
                if (split.shares().isNegative()) {
                    outstandingPayments++;
                    outstandingPaymentAmount += split.shares();
                } else {
                    outstandingDeposits++;
                    outstandingDepositAmount += split.shares();
                }
            } else {
                if (split.shares().isNegative()) {
                    clearedPayments++;
                    clearedPaymentAmount += split.shares();
                } else {
                    clearedDeposits++;
                    clearedDepositAmount += split.shares();
                }
                clearedBalance += split.shares();
            }
        }
    }

    QString reportName = i18n("Reconciliation report of account %1", account.name());
    QString report = QString("<h2 class=\"report\">%1</h2>\n").arg(reportName);
    report += QString("<div class=\"subtitle\">");
    report += QString("%1").arg(QLocale().toString(date, QLocale::ShortFormat));
    report += QString("</div>\n");
    report += QString("<div class=\"gap\">&nbsp;</div>\n");
    report += QString("<div class=\"subtitle\">");
    report += i18n("All values shown in %1", file->baseCurrency().name());
    report += QString("</div>\n");
    report += QString("<div class=\"gap\">&nbsp;</div>\n");

    report += "<table align=\"center\" class=\"report\">\n<thead><tr class=\"itemheader\">";
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
    report += i18np("%1 outstanding deposit", "%1 outstanding deposits in total", outstandingDeposits);
    report += "</td><td>";
    report += MyMoneyUtils::formatMoney(outstandingDepositAmount, currency);
    report += "</td></tr>";
    // row 8
    report += "<tr class=\"row-even\"><td class=\"left\">";
    report += i18n("Register balance as of %1", QLocale().toString(date, QLocale::ShortFormat));
    report += "</td><td>";
    report += MyMoneyUtils::formatMoney(MyMoneyFile::instance()->balance(account.id(), date), currency);
    report += "</td></tr>";

    // retrieve list of all transactions after the reconciliation date that are not reconciled or cleared
    QList<QPair<MyMoneyTransaction, MyMoneySplit> > afterTransactionList;
    MyMoneyTransactionFilter filter(account.id());
    filter.addState((int)eMyMoney::TransactionFilter::State::Cleared);
    filter.addState((int)eMyMoney::TransactionFilter::State::NotReconciled);
    filter.setDateFilter(date.addDays(1), QDate());
    filter.setConsiderCategory(false);
    filter.setReportAllSplits(true);
    file->transactionList(afterTransactionList, filter);

    MyMoneyMoney afterDepositAmount, afterPaymentAmount;
    QList<QPair<MyMoneyTransaction, MyMoneySplit>>::ConstIterator it;
    int afterDeposits = 0;
    int afterPayments = 0;
    for (it = afterTransactionList.constBegin(); it != afterTransactionList.constEnd(); ++it) {
        // if this split is a stock split, we can't just add the amount of shares
        if ((*it).second.reconcileFlag() == eMyMoney::Split::State::NotReconciled) {
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
    report += i18np("%1 payment after %2", "%1 payments after %2", afterPayments, QLocale().toString(date, QLocale::ShortFormat));
    report += "</td><td>";
    report += MyMoneyUtils::formatMoney(afterPaymentAmount, currency);
    report += "</td></tr>";
    // row 10
    report += "<tr class=\"row-even\"><td class=\"left\">";
    report += i18np("%1 deposit after %2", "%1 deposits after %2", afterDeposits, QLocale().toString(date, QLocale::ShortFormat));
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
    detailsTableHeader += "<table align=\"center\" class=\"report\">\n<thead><tr class=\"itemheader\">";
    detailsTableHeader += "<th>" + i18n("Date") + "</th>";
    detailsTableHeader += "<th>" + i18n("Number") + "</th>";
    detailsTableHeader += "<th>" + i18n("Payee") + "</th>";
    detailsTableHeader += "<th>" + i18n("Memo") + "</th>";
    detailsTableHeader += "<th>" + i18n("Category") + "</th>";
    detailsTableHeader += "<th>" + i18n("Amount") + "</th>";
    detailsTableHeader += "</tr></thead>\n";


    QString detailsReport = QString("<h2 class=\"report\">%1</h2>\n").arg(i18n("Outstanding payments"));
    detailsReport += detailsTableHeader;

    auto index = 0;

    for (const auto& journalEntryId : transactionList) {
        // if this split is a stock split, we can't just add the amount of shares
        const auto journalEntry = file->journalModel()->itemById(journalEntryId);
        if (!journalEntry.id().isEmpty()) {
            const auto& split = journalEntry.split();
            const auto& transaction = journalEntry.transaction();

            if (split.reconcileFlag() == eMyMoney::Split::State::NotReconciled && split.shares().isNegative()) {
                QString category;
                for (const auto& tSplit : transaction.splits()) {
                    if (tSplit.accountId() != account.id()) {
                        if (!category.isEmpty())
                            category += QLatin1String(", "); // this is a split transaction
                        category += file->account(tSplit.accountId()).name();
                    }
                }

                detailsReport += QString("<tr class=\"%1\"><td>").arg((index++ % 2 == 1) ? "row-odd" : "row-even");
                detailsReport += QString("%1").arg(QLocale().toString(transaction.postDate(), QLocale::ShortFormat));
                detailsReport += "</td><td>";
                detailsReport += QString("%1").arg(split.number());
                detailsReport += "</td><td>";
                detailsReport += QString("%1").arg(file->payee(split.payeeId()).name());
                detailsReport += "</td><td>";
                detailsReport += QString("%1").arg(transaction.memo());
                detailsReport += "</td><td>";
                detailsReport += QString("%1").arg(category);
                detailsReport += "</td><td>";
                detailsReport += QString("%1").arg(MyMoneyUtils::formatMoney(split.shares(), file->currency(account.currencyId())));
                detailsReport += "</td></tr>";
            }
        }
    }

    detailsReport += "<tr class=\"sectionfooter\">";
    detailsReport += QString("<td class=\"left1\" colspan=\"5\">%1</td><td>%2</td></tr>").arg(i18np("One outstanding payment of", "Total of %1 outstanding payments amounting to", outstandingPayments)).arg(MyMoneyUtils::formatMoney(outstandingPaymentAmount, currency));

    detailsReport += "</table>\n";
    detailsReport += QString("<h2 class=\"report\">%1</h2>\n").arg(
                         account.accountType() == eMyMoney::Account::Type::CreditCard ? i18n("Outstanding charges") : i18n("Outstanding deposits"));
    detailsReport += detailsTableHeader;

    index = 0;
    for (const auto& journalEntryId : transactionList) {
        // if this split is a stock split, we can't just add the amount of shares
        const auto journalEntry = file->journalModel()->itemById(journalEntryId);
        if (!journalEntry.id().isEmpty()) {
            const auto& split = journalEntry.split();
            const auto& transaction = journalEntry.transaction();

            if (split.reconcileFlag() == eMyMoney::Split::State::NotReconciled && !split.shares().isNegative()) {
                QString category;
                for (const auto& tSplit : transaction.splits()) {
                    if (tSplit.accountId() != account.id()) {
                        if (!category.isEmpty())
                            category += QLatin1String(", "); // this is a split transaction
                        category += file->account(tSplit.accountId()).name();
                    }
                }

                detailsReport += QString("<tr class=\"%1\"><td>").arg((index++ % 2 == 1) ? "row-odd" : "row-even")
                    + QString("%1").arg(QLocale().toString(transaction.postDate(), QLocale::ShortFormat)) + "</td><td>" + QString("%1").arg(split.number())
                    + "</td><td>" + QString("%1").arg(file->payee(split.payeeId()).name()) + "</td><td>" + QString("%1").arg(transaction.memo()) + "</td><td>"
                    + QString("%1").arg(category) + "</td><td>"
                    + QString("%1").arg(MyMoneyUtils::formatMoney(split.shares(), file->currency(account.currencyId()))) + "</td></tr>";
            }
        }
    }

    detailsReport += "<tr class=\"sectionfooter\">"
                     + QString("<td class=\"left1\" colspan=\"5\">%1</td><td>%2</td></tr>").arg(
                         account.accountType() == eMyMoney::Account::Type::CreditCard ?
                         i18np("One outstanding charges of", "Total of %1 outstanding charges amounting to", outstandingDeposits) :
                         i18np("One outstanding deposit of", "Total of %1 outstanding deposits amounting to", outstandingDeposits)
                     ).arg(MyMoneyUtils::formatMoney(outstandingDepositAmount, currency))

                     // end of the table
                     + "</table>\n";

    QPointer<KReportDlg> dlg = new KReportDlg(0, header + report + footer, header + detailsReport + footer);
    dlg->exec();
    delete dlg;
}

K_PLUGIN_CLASS_WITH_JSON(ReconciliationReport, "reconciliationreport.json")

#include "reconciliationreport.moc"
