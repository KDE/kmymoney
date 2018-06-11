/***************************************************************************
                          khomeview_p.h  -  description
                             -------------------
    begin                : Tue Jan 22 2002
    copyright            : (C) 2000-2002 by Michael Edwardes <mte@users.sourceforge.net>
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

#ifndef KHOMEVIEW_P_H
#define KHOMEVIEW_P_H

#include "khomeview.h"

#include <config-kmymoney.h>

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>
#include <QPixmap>
#include <QTimer>
#include <QBuffer>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QUrlQuery>
#include <QWheelEvent>
#include <QPrintDialog>
#include <QPrinter>
#include <QPointer>
#ifdef ENABLE_WEBENGINE
#include <QWebEngineView>
#else
#include <KWebView>
#include <QWebFrame>
#endif

// ----------------------------------------------------------------------------
// KDE Includes

#include <KChartAbstractCoordinatePlane>
#include <KChartChart>
#include <KLocalizedString>
#include <KXmlGuiWindow>
#include <KActionCollection>
#include <KMessageBox>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyviewbase_p.h"
#include "mymoneyutils.h"
#include "kmymoneyutils.h"
#include "kwelcomepage.h"
#include "kmymoneysettings.h"
#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneyprice.h"
#include "mymoneyforecast.h"
#include "kreportchartview.h"
#include "pivottable.h"
#include "pivotgrid.h"
#include "reportaccount.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "icons.h"
#include "kmymoneywebpage.h"
#include "mymoneyschedule.h"
#include "mymoneysecurity.h"
#include "mymoneyexception.h"
#include "mymoneyenums.h"
#include "menuenums.h"

#define VIEW_LEDGER         "ledger"
#define VIEW_SCHEDULE       "schedule"
#define VIEW_WELCOME        "welcome"
#define VIEW_HOME           "home"
#define VIEW_REPORTS        "reports"

using namespace Icons;
using namespace eMyMoney;

/**
 * @brief Converts a QPixmap to an data URI scheme
 *
 * According to RFC 2397
 *
 * @param pixmap Source to convert
 * @return full data URI
 */
QString QPixmapToDataUri(const QPixmap& pixmap)
{
  QImage image(pixmap.toImage());
  QByteArray byteArray;
  QBuffer buffer(&byteArray);
  buffer.open(QIODevice::WriteOnly);
  image.save(&buffer, "PNG"); // writes the image in PNG format inside the buffer
  return QLatin1String("data:image/png;base64,") + QString(byteArray.toBase64());
}

bool accountNameLess(const MyMoneyAccount &acc1, const MyMoneyAccount &acc2)
{
  return acc1.name().localeAwareCompare(acc2.name()) < 0;
}

using namespace reports;

class KHomeViewPrivate : public KMyMoneyViewBasePrivate
{
  Q_DECLARE_PUBLIC(KHomeView)

public:
  explicit KHomeViewPrivate(KHomeView *qq) :
    KMyMoneyViewBasePrivate(),
    q_ptr(qq),
    m_view(nullptr),
    m_showAllSchedules(false),
    m_needLoad(true),
    m_netWorthGraphLastValidSize(400, 300),
    m_currentPrinter(nullptr)
  {
  }

  ~KHomeViewPrivate() {
    // if user wants to remember the font size, store it here
    if (KMyMoneySettings::rememberZoomFactor() && m_view) {
        KMyMoneySettings::setZoomFactor(m_view->zoomFactor());
        KMyMoneySettings::self()->save();
      }
  }

  /**
    * Definition of bitmap used as argument for showAccounts().
    */
  enum paymentTypeE {
    Preferred = 1,          ///< show preferred accounts
    Payment = 2             ///< show payment accounts
  };

  void init()
  {
    Q_Q(KHomeView);
    m_needLoad = false;

    auto vbox = new QVBoxLayout(q);
    q->setLayout(vbox);
    vbox->setSpacing(6);
    vbox->setMargin(0);

  #ifdef ENABLE_WEBENGINE
    m_view = new QWebEngineView(q);
  #else
    m_view = new KWebView(q);
  #endif
    m_view->setPage(new MyQWebEnginePage(m_view));

    vbox->addWidget(m_view);

    m_view->setHtml(KWelcomePage::welcomePage(), QUrl("file://"));
  #ifdef ENABLE_WEBENGINE
    q->connect(m_view->page(), &QWebEnginePage::urlChanged,
            q, &KHomeView::slotOpenUrl);
  #else
    m_view->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    q->connect(m_view->page(), &KWebPage::linkClicked,
            q, &KHomeView::slotOpenUrl);
  #endif

    q->connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, q, &KHomeView::refresh);
  }

  /**
    * Print an account and its balance and limit
    */
  void showAccountEntry(const MyMoneyAccount& acc, const MyMoneyMoney& value, const MyMoneyMoney& valueToMinBal, const bool showMinBal)
  {
    MyMoneyFile* file = MyMoneyFile::instance();
    QString tmp;
    MyMoneySecurity currency = file->currency(acc.currencyId());
    QString amount;
    QString amountToMinBal;

    //format amounts
    amount = MyMoneyUtils::formatMoney(value, acc, currency);
    amount.replace(QChar(' '), "&nbsp;");
    if (showMinBal) {
      amountToMinBal = MyMoneyUtils::formatMoney(valueToMinBal, acc, currency);
      amountToMinBal.replace(QChar(' '), "&nbsp;");
    }

    QString cellStatus, cellCounts, pathOK, pathTODO, pathNotOK;

    if (KMyMoneySettings::showBalanceStatusOfOnlineAccounts()) {
      //show account's online-status
      pathOK = QPixmapToDataUri(Icons::get(Icon::DialogOKApply).pixmap(QSize(16,16)));
      pathTODO = QPixmapToDataUri(Icons::get(Icon::MailReceive).pixmap(QSize(16,16)));
      pathNotOK = QPixmapToDataUri(Icons::get(Icon::DialogCancel).pixmap(QSize(16,16)));

      if (acc.value("lastImportedTransactionDate").isEmpty() || acc.value("lastStatementBalance").isEmpty())
        cellStatus = '-';
      else if (file->hasMatchingOnlineBalance(acc)) {
        if (file->hasNewerTransaction(acc.id(), QDate::fromString(acc.value("lastImportedTransactionDate"), Qt::ISODate)))
          cellStatus = QString("<img src=\"%1\" border=\"0\">").arg(pathTODO);
        else
          cellStatus = QString("<img src=\"%1\" border=\"0\">").arg(pathOK);
      } else
        cellStatus = QString("<img src=\"%1\" border=\"0\">").arg(pathNotOK);

      tmp = QString("<td class=\"center\">%1</td>").arg(cellStatus);
    }

    tmp += QString("<td>") +
           link(VIEW_LEDGER, QString("?id=%1").arg(acc.id())) + acc.name() + linkend() + "</td>";

    int countNotMarked = 0, countCleared = 0, countNotReconciled = 0;
    QString countStr;

    if (KMyMoneySettings::showCountOfUnmarkedTransactions() || KMyMoneySettings::showCountOfNotReconciledTransactions())
      countNotMarked = file->countTransactionsWithSpecificReconciliationState(acc.id(), TransactionFilter::State::NotReconciled);

    if (KMyMoneySettings::showCountOfClearedTransactions() || KMyMoneySettings::showCountOfNotReconciledTransactions())
      countCleared = file->countTransactionsWithSpecificReconciliationState(acc.id(), TransactionFilter::State::Cleared);

    if (KMyMoneySettings::showCountOfNotReconciledTransactions())
      countNotReconciled = countNotMarked + countCleared;

    if (KMyMoneySettings::showCountOfUnmarkedTransactions()) {
      if (countNotMarked)
        countStr = QString("%1").arg(countNotMarked);
      else
        countStr = '-';
      tmp += QString("<td class=\"center\">%1</td>").arg(countStr);
    }

    if (KMyMoneySettings::showCountOfClearedTransactions()) {
      if (countCleared)
        countStr = QString("%1").arg(countCleared);
      else
        countStr = '-';
      tmp += QString("<td class=\"center\">%1</td>").arg(countStr);
    }

    if (KMyMoneySettings::showCountOfNotReconciledTransactions()) {
      if (countNotReconciled)
        countStr = QString("%1").arg(countNotReconciled);
      else
        countStr = '-';
      tmp += QString("<td class=\"center\">%1</td>").arg(countStr);
    }

    if (KMyMoneySettings::showDateOfLastReconciliation()) {
      const auto lastReconciliationDate = acc.lastReconciliationDate().toString(Qt::SystemLocaleShortDate).replace(QChar(' '), "&nbsp;");
      tmp += QString("<td>%1</d>").arg(lastReconciliationDate);
    }

    //show account balance
    tmp += QString("<td class=\"right\">%1</td>").arg(showColoredAmount(amount, value.isNegative()));

    //show minimum balance column if requested
    if (showMinBal) {
      //if it is an investment, show minimum balance empty
      if (acc.accountType() == Account::Type::Investment) {
        tmp += QString("<td class=\"right\">&nbsp;</td>");
      } else {
        //show minimum balance entry
        tmp += QString("<td class=\"right\">%1</td>").arg(showColoredAmount(amountToMinBal, valueToMinBal.isNegative()));
      }
    }
    // qDebug("accountEntry = '%s'", tmp.toLatin1());
    m_html += tmp;
  }

  void showAccountEntry(const MyMoneyAccount& acc)
  {
    MyMoneyFile* file = MyMoneyFile::instance();
    MyMoneySecurity currency = file->currency(acc.currencyId());
    MyMoneyMoney value;

    bool showLimit = KMyMoneySettings::showLimitInfo();

    if (acc.accountType() == Account::Type::Investment) {
      //investment accounts show the balances of all its subaccounts
      value = investmentBalance(acc);

      //investment accounts have no minimum balance
      showAccountEntry(acc, value, MyMoneyMoney(), showLimit);
    } else {
      //get balance for normal accounts
      value = file->balance(acc.id(), QDate::currentDate());
      if (acc.currencyId() != file->baseCurrency().id()) {
        ReportAccount repAcc = ReportAccount(acc.id());
        MyMoneyMoney curPrice = repAcc.baseCurrencyPrice(QDate::currentDate());
        MyMoneyMoney baseValue = value * curPrice;
        baseValue = baseValue.convert(file->baseCurrency().smallestAccountFraction());
        m_total += baseValue;
      } else {
        m_total += value;
      }
      //if credit card or checkings account, show maximum credit
      if (acc.accountType() == Account::Type::CreditCard ||
          acc.accountType() == Account::Type::Checkings) {
        QString maximumCredit = acc.value("maxCreditAbsolute");
        if (maximumCredit.isEmpty()) {
          maximumCredit = acc.value("minBalanceAbsolute");
        }
        MyMoneyMoney maxCredit = MyMoneyMoney(maximumCredit);
        showAccountEntry(acc, value, value - maxCredit, showLimit);
      } else {
        //otherwise use minimum balance
        QString minimumBalance = acc.value("minBalanceAbsolute");
        MyMoneyMoney minBalance = MyMoneyMoney(minimumBalance);
        showAccountEntry(acc, value, value - minBalance, showLimit);
      }
    }
  }

  /**
    * @param acc the investment account
    * @return the balance in the currency of the investment account
    */
  MyMoneyMoney investmentBalance(const MyMoneyAccount& acc)
  {
    auto file = MyMoneyFile::instance();
    auto value = file->balance(acc.id(), QDate::currentDate());
    foreach (const auto accountID, acc.accountList()) {
      auto stock = file->account(accountID);
      if (!stock.isClosed()) {
        try {
          MyMoneyMoney val;
          MyMoneyMoney balance = file->balance(stock.id(), QDate::currentDate());
          MyMoneySecurity security = file->security(stock.currencyId());
          const MyMoneyPrice &price = file->price(stock.currencyId(), security.tradingCurrency());
          val = (balance * price.rate(security.tradingCurrency())).convertPrecision(security.pricePrecision());
          // adjust value of security to the currency of the account
          MyMoneySecurity accountCurrency = file->currency(acc.currencyId());
          val = val * file->price(security.tradingCurrency(), accountCurrency.id()).rate(accountCurrency.id());
          val = val.convert(acc.fraction());
          value += val;
        } catch (const MyMoneyException &e) {
          qWarning("%s", qPrintable(QString("cannot convert stock balance of %1 to base currency: %2").arg(stock.name(), e.what())));
        }
      }
    }
    return value;
  }

  /**
   * Print text in the color set for negative numbers, if @p amount is negative
   * abd @p isNegative is true
   */
  QString showColoredAmount(const QString& amount, bool isNegative)
  {
    if (isNegative) {
      //if negative, get the settings for negative numbers
      return QString("<font color=\"%1\">%2</font>").arg(KMyMoneySettings::schemeColor(SchemeColor::Negative).name(), amount);
    }

    //if positive, return the same string
    return amount;
  }

  /**
   * Run the forecast
   */
  void doForecast()
  {
    //clear m_accountList because forecast is about to changed
    m_accountList.clear();

    //reinitialize the object
    m_forecast = KMyMoneyUtils::forecast();

    //If forecastDays lower than accountsCycle, adjust to the first cycle
    if (m_forecast.accountsCycle() > m_forecast.forecastDays())
      m_forecast.setForecastDays(m_forecast.accountsCycle());

    //Get all accounts of the right type to calculate forecast
    m_forecast.doForecast();
  }

  /**
   * Calculate the forecast balance after a payment has been made
   */
  MyMoneyMoney forecastPaymentBalance(const MyMoneyAccount& acc, const MyMoneyMoney& payment, QDate& paymentDate)
  {
    //if paymentDate before or equal to currentDate set it to current date plus 1
    //so we get to accumulate forecast balance correctly
    if (paymentDate <= QDate::currentDate())
      paymentDate = QDate::currentDate().addDays(1);

    //check if the account is already there
    if (m_accountList.find(acc.id()) == m_accountList.end()
        || m_accountList[acc.id()].find(paymentDate) == m_accountList[acc.id()].end()) {
      if (paymentDate == QDate::currentDate()) {
        m_accountList[acc.id()][paymentDate] = m_forecast.forecastBalance(acc, paymentDate);
      } else {
        m_accountList[acc.id()][paymentDate] = m_forecast.forecastBalance(acc, paymentDate.addDays(-1));
      }
    }
    m_accountList[acc.id()][paymentDate] = m_accountList[acc.id()][paymentDate] + payment;
    return m_accountList[acc.id()][paymentDate];
  }

  void loadView()
  {
    m_view->setZoomFactor(KMyMoneySettings::zoomFactor());

    QList<MyMoneyAccount> list;
    if (MyMoneyFile::instance()->storage())
      MyMoneyFile::instance()->accountList(list);
    if (list.isEmpty()) {
      m_view->setHtml(KWelcomePage::welcomePage(), QUrl("file://"));
    } else {
      // keep current location on page
      int scrollBarPos = 0;
#ifdef ENABLE_WEBENGINE
        /// @todo cannot test this
#else
      scrollBarPos = m_view->page()->mainFrame()->scrollBarValue(Qt::Vertical);
#endif

      //clear the forecast flag so it will be reloaded
      m_forecast.setForecastDone(false);

      const QString filename = QStandardPaths::locate(QStandardPaths::AppConfigLocation, "html/kmymoney.css");
      QString header = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\">\n<html><head><link rel=\"stylesheet\" type=\"text/css\" href=\"%1\">\n").arg(QUrl::fromLocalFile(filename).url());

      header += KMyMoneyUtils::variableCSS();

      header += "</head><body id=\"summaryview\">\n";

      QString footer = "</body></html>\n";

      m_html.clear();
      m_html += header;

      m_html += QString("<div id=\"summarytitle\">%1</div>").arg(i18n("Your Financial Summary"));

      QStringList settings = KMyMoneySettings::listOfItems();

      QStringList::ConstIterator it;

      for (it = settings.constBegin(); it != settings.constEnd(); ++it) {
        int option = (*it).toInt();
        if (option > 0) {
          switch (option) {
            case 1:         // payments
              showPayments();
              break;

            case 2:         // preferred accounts
              showAccounts(Preferred, i18n("Preferred Accounts"));
              break;

            case 3:         // payment accounts
              // Check if preferred accounts are shown separately
              if (settings.contains("2")) {
                showAccounts(static_cast<paymentTypeE>(Payment | Preferred),
                             i18n("Payment Accounts"));
              } else {
                showAccounts(Payment, i18n("Payment Accounts"));
              }
              break;
            case 4:         // favorite reports
              showFavoriteReports();
              break;
            case 5:         // forecast
              showForecast();
              break;
            case 6:         // net worth graph over all accounts
              showNetWorthGraph();
              break;
            case 8:         // assets and liabilities
              showAssetsLiabilities();
              break;
            case 9:         // budget
              showBudget();
              break;
            case 10:         // cash flow summary
              showCashFlowSummary();
              break;


          }
          m_html += "<div class=\"gap\">&nbsp;</div>\n";
        }
      }

      m_html += "<div id=\"returnlink\">";
      m_html += link(VIEW_WELCOME, QString()) + i18n("Show KMyMoney welcome page") + linkend();
      m_html += "</div>";
      m_html += "<div id=\"vieweffect\"></div>";
      m_html += footer;

      m_view->setHtml(m_html, QUrl("file://"));

      if (scrollBarPos) {
#ifdef ENABLE_WEBENGINE
        /// @todo cannot test this
#else
        m_view->page()->mainFrame()->setScrollBarValue(Qt::Vertical, scrollBarPos);
#endif
      }
    }
  }

  void showNetWorthGraph()
  {
    Q_Q(KHomeView);
    m_html += QString("<div class=\"shadow\"><div class=\"displayblock\"><div class=\"summaryheader\">%1</div>\n<div class=\"gap\">&nbsp;</div>\n").arg(i18n("Net Worth Forecast"));

    MyMoneyReport reportCfg = MyMoneyReport(
                                MyMoneyReport::eAssetLiability,
                                MyMoneyReport::eMonths,
                                TransactionFilter::Date::UserDefined, // overridden by the setDateFilter() call below
                                MyMoneyReport::eDetailTotal,
                                i18n("Net Worth Forecast"),
                                i18n("Generated Report"));

    reportCfg.setChartByDefault(true);
    reportCfg.setChartCHGridLines(false);
    reportCfg.setChartSVGridLines(false);
    reportCfg.setChartDataLabels(false);
    reportCfg.setChartType(MyMoneyReport::eChartLine);
    reportCfg.setIncludingSchedules(false);
    reportCfg.addAccountGroup(Account::Type::Asset);
    reportCfg.addAccountGroup(Account::Type::Liability);
    reportCfg.setColumnsAreDays(true);
    reportCfg.setConvertCurrency(true);
    reportCfg.setIncludingForecast(true);
    reportCfg.setDateFilter(QDate::currentDate(), QDate::currentDate().addDays(+ 90));
    reports::PivotTable table(reportCfg);

    reports::KReportChartView* chartWidget = new reports::KReportChartView(0);

    table.drawChart(*chartWidget);

    // Adjust the size
    QSize netWorthGraphSize = q->size();
    netWorthGraphSize -= QSize(80, 30);
    // consider the computed size valid only if it's smaller on both axes that the applications size
//    if (netWorthGraphSize.width() < kmymoney->width() || netWorthGraphSize.height() < kmymoney->height()) {
      m_netWorthGraphLastValidSize = netWorthGraphSize;
//    }
    chartWidget->resize(m_netWorthGraphLastValidSize);

    //save the chart to an image
    QString chart = QPixmapToDataUri(chartWidget->coordinatePlane()->parent()->grab());

    m_html += QString("<table width=\"100%\" cellspacing=\"0\" cellpadding=\"2\" class=\"summarytable\" >");
    m_html += QString("<tr>");
    m_html += QString("<td><center><img src=\"%1\" ALT=\"Networth\" width=\"100%\" ></center></td>").arg(chart);
    m_html += QString("</tr>");
    m_html += QString("</table></div></div>");

    //delete the widget since we no longer need it
    delete chartWidget;
  }

  void showPayments()
  {
    MyMoneyFile* file = MyMoneyFile::instance();
    QList<MyMoneySchedule> overdues;
    QList<MyMoneySchedule> schedule;
    int i = 0;

    //if forecast has not been executed yet, do it.
    if (!m_forecast.isForecastDone())
      doForecast();

    schedule = file->scheduleList(QString(), Schedule::Type::Any,
                                  Schedule::Occurrence::Any,
                                  Schedule::PaymentType::Any,
                                  QDate::currentDate(),
                                  QDate::currentDate().addMonths(1), false);
    overdues = file->scheduleList(QString(), Schedule::Type::Any,
                                  Schedule::Occurrence::Any,
                                  Schedule::PaymentType::Any,
                                  QDate(), QDate(), true);

    if (schedule.empty() && overdues.empty())
      return;

    // HACK
    // Remove the finished schedules
    QList<MyMoneySchedule>::Iterator d_it;
    //regular schedules
    d_it = schedule.begin();
    while (d_it != schedule.end()) {
      if ((*d_it).isFinished()) {
        d_it = schedule.erase(d_it);
        continue;
      }
      ++d_it;
    }
    //overdue schedules
    d_it = overdues.begin();
    while (d_it != overdues.end()) {
      if ((*d_it).isFinished()) {
        d_it = overdues.erase(d_it);
        continue;
      }
      ++d_it;
    }

    m_html += "<div class=\"shadow\"><div class=\"displayblock\">";
    m_html += QString("<div class=\"summaryheader\">%1</div>\n").arg(i18n("Payments"));

    if (!overdues.isEmpty()) {
      m_html += "<div class=\"gap\">&nbsp;</div>\n";

      qSort(overdues);
      QList<MyMoneySchedule>::Iterator it;
      QList<MyMoneySchedule>::Iterator it_f;

      m_html += "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"2\" class=\"summarytable\" >";
      m_html += QString("<tr class=\"itemtitle warningtitle\" ><td colspan=\"5\">%1</td></tr>\n").arg(showColoredAmount(i18n("Overdue payments"), true));
      m_html += "<tr class=\"item warning\">";
      m_html += "<td class=\"left\" width=\"10%\">";
      m_html += i18n("Date");
      m_html += "</td>";
      m_html += "<td class=\"left\" width=\"40%\">";
      m_html += i18n("Schedule");
      m_html += "</td>";
      m_html += "<td class=\"left\" width=\"20%\">";
      m_html += i18n("Account");
      m_html += "</td>";
      m_html += "<td class=\"right\" width=\"15%\">";
      m_html += i18n("Amount");
      m_html += "</td>";
      m_html += "<td class=\"right\" width=\"15%\">";
      m_html += i18n("Balance after");
      m_html += "</td>";
      m_html += "</tr>";

      for (it = overdues.begin(); it != overdues.end(); ++it) {
        // determine number of overdue payments
        int cnt =
          (*it).transactionsRemainingUntil(QDate::currentDate().addDays(-1));

        m_html += QString("<tr class=\"row-%1\">").arg(i++ & 0x01 ? "even" : "odd");
        showPaymentEntry(*it, cnt);
        m_html += "</tr>";
      }
      m_html += "</table>";
    }

    if (!schedule.isEmpty()) {
      qSort(schedule);

      // Extract todays payments if any
      QList<MyMoneySchedule> todays;
      QList<MyMoneySchedule>::Iterator t_it;
      for (t_it = schedule.begin(); t_it != schedule.end();) {
        if ((*t_it).adjustedNextDueDate() == QDate::currentDate()) {
          todays.append(*t_it);
          (*t_it).setNextDueDate((*t_it).nextPayment(QDate::currentDate()));

          // if adjustedNextDueDate is still currentDate then remove it from
          // scheduled payments
          if ((*t_it).adjustedNextDueDate() == QDate::currentDate()) {
            t_it = schedule.erase(t_it);
            continue;
          }
        }
        ++t_it;
      }

      if (todays.count() > 0) {
        m_html += "<div class=\"gap\">&nbsp;</div>\n";
        m_html += "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"2\" class=\"summarytable\" >";
        m_html += QString("<tr class=\"itemtitle\"><td class=\"left\" colspan=\"5\">%1</td></tr>\n").arg(i18n("Today's due payments"));
        m_html += "<tr class=\"item\">";
        m_html += "<td class=\"left\" width=\"10%\">";
        m_html += i18n("Date");
        m_html += "</td>";
        m_html += "<td class=\"left\" width=\"40%\">";
        m_html += i18n("Schedule");
        m_html += "</td>";
        m_html += "<td class=\"left\" width=\"20%\">";
        m_html += i18n("Account");
        m_html += "</td>";
        m_html += "<td class=\"right\" width=\"15%\">";
        m_html += i18n("Amount");
        m_html += "</td>";
        m_html += "<td class=\"right\" width=\"15%\">";
        m_html += i18n("Balance after");
        m_html += "</td>";
        m_html += "</tr>";

        for (t_it = todays.begin(); t_it != todays.end(); ++t_it) {
          m_html += QString("<tr class=\"row-%1\">").arg(i++ & 0x01 ? "even" : "odd");
          showPaymentEntry(*t_it);
          m_html += "</tr>";
        }
        m_html += "</table>";
      }

      if (!schedule.isEmpty()) {
        m_html += "<div class=\"gap\">&nbsp;</div>\n";

        QList<MyMoneySchedule>::Iterator it;

        m_html += "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"2\" class=\"summarytable\" >";
        m_html += QString("<tr class=\"itemtitle\"><td class=\"left\" colspan=\"5\">%1</td></tr>\n").arg(i18n("Future payments"));
        m_html += "<tr class=\"item\">";
        m_html += "<td class=\"left\" width=\"10%\">";
        m_html += i18n("Date");
        m_html += "</td>";
        m_html += "<td class=\"left\" width=\"40%\">";
        m_html += i18n("Schedule");
        m_html += "</td>";
        m_html += "<td class=\"left\" width=\"20%\">";
        m_html += i18n("Account");
        m_html += "</td>";
        m_html += "<td class=\"right\" width=\"15%\">";
        m_html += i18n("Amount");
        m_html += "</td>";
        m_html += "<td class=\"right\" width=\"15%\">";
        m_html += i18n("Balance after");
        m_html += "</td>";
        m_html += "</tr>";

        // show all or the first 6 entries
        int cnt;
        cnt = (m_showAllSchedules) ? -1 : 6;
        bool needMoreLess = m_showAllSchedules;

        QDate lastDate = QDate::currentDate().addMonths(1);
        qSort(schedule);
        do {
          it = schedule.begin();
          if (it == schedule.end())
            break;

          // if the next due date is invalid (schedule is finished)
          // we remove it from the list
          QDate nextDate = (*it).nextDueDate();
          if (!nextDate.isValid()) {
            schedule.erase(it);
            continue;
          }

          if (nextDate > lastDate)
            break;

          if (cnt == 0) {
            needMoreLess = true;
            break;
          }

          // in case we've shown the current recurrence as overdue,
          // we don't show it here again, but keep the schedule
          // as it might show up later in the list again
          if (!(*it).isOverdue()) {
            if (cnt > 0)
              --cnt;

            m_html += QString("<tr class=\"row-%1\">").arg(i++ & 0x01 ? "even" : "odd");
            showPaymentEntry(*it);
            m_html += "</tr>";

            // for single occurrence we have reported everything so we
            // better get out of here.
            if ((*it).occurrence() == Schedule::Occurrence::Once) {
              schedule.erase(it);
              continue;
            }
          }

          // if nextPayment returns an invalid date, setNextDueDate will
          // just skip it, resulting in a loop
          // we check the resulting date and erase the schedule if invalid
          if (!((*it).nextPayment((*it).nextDueDate())).isValid()) {
            schedule.erase(it);
            continue;
          }

          (*it).setNextDueDate((*it).nextPayment((*it).nextDueDate()));
          qSort(schedule);
        } while (1);

        if (needMoreLess) {
          m_html += QString("<tr class=\"row-%1\">").arg(i++ & 0x01 ? "even" : "odd");
          m_html += "<td colspan=\"5\">";
          if (m_showAllSchedules) {
            m_html += link(VIEW_SCHEDULE,  QString("?mode=%1").arg("reduced")) + i18nc("Less...", "Show fewer schedules on the list") + linkend();
          } else {
            m_html += link(VIEW_SCHEDULE,  QString("?mode=%1").arg("full")) + i18nc("More...", "Show more schedules on the list") + linkend();
          }
          m_html += "</td>";
          m_html += "</tr>";
        }
        m_html += "</table>";
      }
    }
    m_html += "</div></div>";
  }

  void showPaymentEntry(const MyMoneySchedule& sched, int cnt = 1)
  {
    QString tmp;
    MyMoneyFile* file = MyMoneyFile::instance();

    try {
      MyMoneyAccount acc = sched.account();
      if (!acc.id().isEmpty()) {
        MyMoneyTransaction t = sched.transaction();
        // only show the entry, if it is still active
        if (!sched.isFinished()) {
          MyMoneySplit sp = t.splitByAccount(acc.id(), true);

          QString pathEnter = QPixmapToDataUri(Icons::get(Icon::KeyEnter).pixmap(QSize(16,16)));
          QString pathSkip = QPixmapToDataUri(Icons::get(Icon::MediaSkipForward).pixmap(QSize(16,16)));

          //show payment date
          tmp = QString("<td>") +
                QLocale().toString(sched.adjustedNextDueDate(), QLocale::ShortFormat) +
                "</td><td>";
          if (!pathEnter.isEmpty())
            tmp += link(VIEW_SCHEDULE, QString("?id=%1&amp;mode=enter").arg(sched.id()), i18n("Enter schedule")) + QString("<img src=\"%1\" border=\"0\"></a>").arg(pathEnter) + linkend();
          if (!pathSkip.isEmpty())
            tmp += "&nbsp;" + link(VIEW_SCHEDULE, QString("?id=%1&amp;mode=skip").arg(sched.id()), i18n("Skip schedule")) + QString("<img src=\"%1\" border=\"0\"></a>").arg(pathSkip) + linkend();

          tmp += QString("&nbsp;");
          tmp += link(VIEW_SCHEDULE, QString("?id=%1&amp;mode=edit").arg(sched.id()), i18n("Edit schedule")) + sched.name() + linkend();

          //show quantity of payments overdue if any
          if (cnt > 1)
            tmp += i18np(" (%1 payment)", " (%1 payments)", cnt);

          //show account of the main split
          tmp += "</td><td>";
          tmp += QString(file->account(acc.id()).name());

          //show amount of the schedule
          tmp += "</td><td align=\"right\">";

          const MyMoneySecurity& currency = MyMoneyFile::instance()->currency(acc.currencyId());
          MyMoneyMoney payment = MyMoneyMoney(sp.value(t.commodity(), acc.currencyId()) * cnt);
          QString amount = MyMoneyUtils::formatMoney(payment, acc, currency);
          amount.replace(QChar(' '), "&nbsp;");
          tmp += showColoredAmount(amount, payment.isNegative());
          tmp += "</td>";
          //show balance after payments
          tmp += "<td align=\"right\">";
          QDate paymentDate = QDate(sched.adjustedNextDueDate());
          MyMoneyMoney balanceAfter = forecastPaymentBalance(acc, payment, paymentDate);
          QString balance = MyMoneyUtils::formatMoney(balanceAfter, acc, currency);
          balance.replace(QChar(' '), "&nbsp;");
          tmp += showColoredAmount(balance, balanceAfter.isNegative());
          tmp += "</td>";

          // qDebug("paymentEntry = '%s'", tmp.toLatin1());
          m_html += tmp;
        }
      }
    } catch (const MyMoneyException &e) {
      qDebug("Unable to display schedule entry: %s", e.what());
    }
  }

  void showAccounts(paymentTypeE type, const QString& header)
  {
    MyMoneyFile* file = MyMoneyFile::instance();
    int prec = MyMoneyMoney::denomToPrec(file->baseCurrency().smallestAccountFraction());
    QList<MyMoneyAccount> accounts;

    auto showClosedAccounts = KMyMoneySettings::showAllAccounts();

    // get list of all accounts
    file->accountList(accounts);
    for (QList<MyMoneyAccount>::Iterator it = accounts.begin(); it != accounts.end();) {
      bool removeAccount = false;
      if (!(*it).isClosed() || showClosedAccounts) {
        switch ((*it).accountType()) {
          case Account::Type::Expense:
          case Account::Type::Income:
            // never show a category account
            // Note: This might be different in a future version when
            //       the homepage also shows category based information
            removeAccount = true;
            break;

            // Asset and Liability accounts are only shown if they
            // have the preferred flag set
          case Account::Type::Asset:
          case Account::Type::Liability:
          case Account::Type::Investment:
            // if preferred accounts are requested, then keep in list
            if ((*it).value("PreferredAccount") != "Yes"
                || (type & Preferred) == 0) {
              removeAccount = true;
            }
            break;

            // Check payment accounts. If payment and preferred is selected,
            // then always show them. If only payment is selected, then
            // show only if preferred flag is not set.
          case Account::Type::Checkings:
          case Account::Type::Savings:
          case Account::Type::Cash:
          case Account::Type::CreditCard:
            switch (type & (Payment | Preferred)) {
              case Payment:
                if ((*it).value("PreferredAccount") == "Yes")
                  removeAccount = true;
                break;

              case Preferred:
                if ((*it).value("PreferredAccount") != "Yes")
                  removeAccount = true;
                break;

              case Payment | Preferred:
                break;

              default:
                removeAccount = true;
                break;
            }
            break;

            // filter all accounts that are not used on homepage views
          default:
            removeAccount = true;
            break;
        }

      } else if ((*it).isClosed() || (*it).isInvest()) {
        // don't show if closed or a stock account
        removeAccount = true;
      }

      if (removeAccount)
        it = accounts.erase(it);
      else
        ++it;
    }

    if (!accounts.isEmpty()) {
      // sort the accounts by name
      qStableSort(accounts.begin(), accounts.end(), accountNameLess);
      QString tmp;
      int i = 0;
      tmp = "<div class=\"shadow\"><div class=\"displayblock\"><div class=\"summaryheader\">" + header + "</div>\n<div class=\"gap\">&nbsp;</div>\n";
      m_html += tmp;
      m_html += "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"2\" class=\"summarytable\" >";
      m_html += "<tr class=\"item\">";

      if (KMyMoneySettings::showBalanceStatusOfOnlineAccounts()) {
        QString pathStatusHeader = QPixmapToDataUri(Icons::get(Icon::Download).pixmap(QSize(16,16)));
        m_html += QString("<td class=\"center\"><img src=\"%1\" border=\"0\"></td>").arg(pathStatusHeader);
      }

      m_html += "<td class=\"left\" width=\"35%\">";
      m_html += i18n("Account");
      m_html += "</td>";

      if (KMyMoneySettings::showCountOfUnmarkedTransactions())
        m_html += QString("<td class=\"center\">!M</td>");

      if (KMyMoneySettings::showCountOfClearedTransactions())
        m_html += QString("<td class=\"center\">C</td>");

      if (KMyMoneySettings::showCountOfNotReconciledTransactions())
        m_html += QString("<td class=\"center\">!R</td>");

      if (KMyMoneySettings::showDateOfLastReconciliation())
        m_html += QString("<td>%1</td>").arg(i18n("Last Reconciled"));

      m_html += "<td width=\"25%\" class=\"right\">";
      m_html += i18n("Current Balance");
      m_html += "</td>";

      //only show limit info if user chose to do so
      if (KMyMoneySettings::showLimitInfo()) {
        m_html += "<td width=\"40%\" class=\"right\">";
        m_html += i18n("To Minimum Balance / Maximum Credit");
        m_html += "</td>";
      }
      m_html += "</tr>";

      m_total = 0;
      QList<MyMoneyAccount>::const_iterator it_m;
      for (it_m = accounts.constBegin(); it_m != accounts.constEnd(); ++it_m) {
        m_html += QString("<tr class=\"row-%1\">").arg(i++ & 0x01 ? "even" : "odd");
        showAccountEntry(*it_m);
        m_html += "</tr>";
      }
      m_html += QString("<tr class=\"row-%1\">").arg(i++ & 0x01 ? "even" : "odd");
      QString amount = m_total.formatMoney(file->baseCurrency().tradingSymbol(), prec);
      if (KMyMoneySettings::showBalanceStatusOfOnlineAccounts()) m_html += "<td></td>";
      m_html += QString("<td class=\"right\"><b>%1</b></td>").arg(i18n("Total"));
      if (KMyMoneySettings::showCountOfUnmarkedTransactions()) m_html += "<td></td>";
      if (KMyMoneySettings::showCountOfClearedTransactions()) m_html += "<td></td>";
      if (KMyMoneySettings::showCountOfNotReconciledTransactions()) m_html += "<td></td>";
      if (KMyMoneySettings::showDateOfLastReconciliation()) m_html += "<td></td>";
      m_html += QString("<td class=\"right\"><b>%1</b></td></tr>").arg(showColoredAmount(amount, m_total.isNegative()));
      m_html += "</table></div></div>";
    }
  }

  void showFavoriteReports()
  {
    QList<MyMoneyReport> reports = MyMoneyFile::instance()->reportList();

    if (!reports.isEmpty()) {
      bool firstTime = 1;
      int row = 0;
      QList<MyMoneyReport>::const_iterator it_report = reports.constBegin();
      while (it_report != reports.constEnd()) {
        if ((*it_report).isFavorite()) {
          if (firstTime) {
            m_html += QString("<div class=\"shadow\"><div class=\"displayblock\"><div class=\"summaryheader\">%1</div>\n<div class=\"gap\">&nbsp;</div>\n").arg(i18n("Favorite Reports"));
            m_html += "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"2\" class=\"summarytable\" >";
            m_html += "<tr class=\"item\"><td class=\"left\" width=\"40%\">";
            m_html += i18n("Report");
            m_html += "</td><td width=\"60%\" class=\"left\">";
            m_html += i18n("Comment");
            m_html += "</td></tr>";
            firstTime = false;
          }

          m_html += QString("<tr class=\"row-%1\"><td>%2%3%4</td><td align=\"left\">%5</td></tr>")
                       .arg(row++ & 0x01 ? "even" : "odd")
                       .arg(link(VIEW_REPORTS, QString("?id=%1").arg((*it_report).id())))
                       .arg((*it_report).name())
                       .arg(linkend())
                       .arg((*it_report).comment());
        }

        ++it_report;
      }
      if (!firstTime)
        m_html += "</table></div></div>";
    }
  }

  void showForecast()
  {
    MyMoneyFile* file = MyMoneyFile::instance();
    QList<MyMoneyAccount> accList;

    //if forecast has not been executed yet, do it.
    if (!m_forecast.isForecastDone())
      doForecast();

    accList = m_forecast.accountList();

    if (accList.count() > 0) {
      // sort the accounts by name
      qStableSort(accList.begin(), accList.end(), accountNameLess);
      auto i = 0;

      auto colspan = 1;
      //get begin day
      auto beginDay = QDate::currentDate().daysTo(m_forecast.beginForecastDate());
      //if begin day is today skip to next cycle
      if (beginDay == 0)
        beginDay = m_forecast.accountsCycle();

      // Now output header
      m_html += QString("<div class=\"shadow\"><div class=\"displayblock\"><div class=\"summaryheader\">%1</div>\n<div class=\"gap\">&nbsp;</div>\n").arg(i18n("%1 Day Forecast", m_forecast.forecastDays()));
      m_html += "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"2\" class=\"summarytable\" >";
      m_html += "<tr class=\"item\"><td class=\"left\" width=\"40%\">";
      m_html += i18n("Account");
      m_html += "</td>";
      auto colWidth = 55 / (m_forecast.forecastDays() / m_forecast.accountsCycle());
      for (i = 0; (i*m_forecast.accountsCycle() + beginDay) <= m_forecast.forecastDays(); ++i) {
        m_html += QString("<td width=\"%1%\" class=\"right\">").arg(colWidth);

        m_html += i18ncp("Forecast days", "%1 day", "%1 days", i * m_forecast.accountsCycle() + beginDay);
        m_html += "</td>";
        colspan++;
      }
      m_html += "</tr>";

      // Now output entries
      i = 0;

      QList<MyMoneyAccount>::ConstIterator it_account;
      for (it_account = accList.constBegin(); it_account != accList.constEnd(); ++it_account) {
        //MyMoneyAccount acc = (*it_n);

        m_html += QString("<tr class=\"row-%1\">").arg(i++ & 0x01 ? "even" : "odd");
        m_html += QString("<td width=\"40%\">") +
                     link(VIEW_LEDGER, QString("?id=%1").arg((*it_account).id())) + (*it_account).name() + linkend() + "</td>";

        qint64 dropZero = -1; //account dropped below zero
        qint64 dropMinimum = -1; //account dropped below minimum balance
        QString minimumBalance = (*it_account).value("minimumBalance");
        MyMoneyMoney minBalance = MyMoneyMoney(minimumBalance);
        MyMoneySecurity currency;
        MyMoneyMoney forecastBalance;

        //change account to deep currency if account is an investment
        if ((*it_account).isInvest()) {
          MyMoneySecurity underSecurity = file->security((*it_account).currencyId());
          currency = file->security(underSecurity.tradingCurrency());
        } else {
          currency = file->security((*it_account).currencyId());
        }

        for (auto f = beginDay; f <= m_forecast.forecastDays(); f += m_forecast.accountsCycle()) {
          forecastBalance = m_forecast.forecastBalance(*it_account, QDate::currentDate().addDays(f));
          QString amount;
          amount = MyMoneyUtils::formatMoney(forecastBalance, *it_account, currency);
          amount.replace(QChar(' '), "&nbsp;");
          m_html += QString("<td width=\"%1%\" align=\"right\">").arg(colWidth);
          m_html += QString("%1</td>").arg(showColoredAmount(amount, forecastBalance.isNegative()));
        }

        m_html += "</tr>";

        //Check if the account is going to be below zero or below the minimal balance in the forecast period

        //Check if the account is going to be below minimal balance
        dropMinimum = m_forecast.daysToMinimumBalance(*it_account);

        //Check if the account is going to be below zero in the future
        dropZero = m_forecast.daysToZeroBalance(*it_account);


        // spit out possible warnings
        QString msg;

        // if a minimum balance has been specified, an appropriate warning will
        // only be shown, if the drop below 0 is on a different day or not present

        if (dropMinimum != -1
            && !minBalance.isZero()
            && (dropMinimum < dropZero
                || dropZero == -1)) {
          switch (dropMinimum) {
            case 0:
              msg = i18n("The balance of %1 is below the minimum balance %2 today.", (*it_account).name(), MyMoneyUtils::formatMoney(minBalance, *it_account, currency));
              msg = showColoredAmount(msg, true);
              break;
            default:
              msg = i18np("The balance of %2 will drop below the minimum balance %3 in %1 day.",
                          "The balance of %2 will drop below the minimum balance %3 in %1 days.",
                          dropMinimum - 1, (*it_account).name(), MyMoneyUtils::formatMoney(minBalance, *it_account, currency));
              msg = showColoredAmount(msg, true);
              break;
          }

          if (!msg.isEmpty()) {
            m_html += QString("<tr class=\"warning\" style=\"font-weight: normal;\" ><td colspan=%2 align=\"center\" >%1</td></tr>").arg(msg).arg(colspan);
          }
        }
        // a drop below zero is always shown
        msg.clear();
        switch (dropZero) {
          case -1:
            break;
          case 0:
            if ((*it_account).accountGroup() == Account::Type::Asset) {
              msg = i18n("The balance of %1 is below %2 today.", (*it_account).name(), MyMoneyUtils::formatMoney(MyMoneyMoney(), *it_account, currency));
              msg = showColoredAmount(msg, true);
              break;
            }
            if ((*it_account).accountGroup() == Account::Type::Liability) {
              msg = i18n("The balance of %1 is above %2 today.", (*it_account).name(), MyMoneyUtils::formatMoney(MyMoneyMoney(), *it_account, currency));
              break;
            }
            break;
          default:
            if ((*it_account).accountGroup() == Account::Type::Asset) {
              msg = i18np("The balance of %2 will drop below %3 in %1 day.",
                          "The balance of %2 will drop below %3 in %1 days.",
                          dropZero, (*it_account).name(), MyMoneyUtils::formatMoney(MyMoneyMoney(), *it_account, currency));
              msg = showColoredAmount(msg, true);
              break;
            }
            if ((*it_account).accountGroup() == Account::Type::Liability) {
              msg = i18np("The balance of %2 will raise above %3 in %1 day.",
                          "The balance of %2 will raise above %3 in %1 days.",
                          dropZero, (*it_account).name(), MyMoneyUtils::formatMoney(MyMoneyMoney(), *it_account, currency));
              break;
            }
        }
        if (!msg.isEmpty()) {
          m_html += QString("<tr class=\"warning\"><td colspan=%2 align=\"center\" ><b>%1</b></td></tr>").arg(msg).arg(colspan);
        }
      }
      m_html += "</table></div></div>";

    }
  }

  QString link(const QString& view, const QString& query, const QString& _title = QString()) const
  {
    QString titlePart;
    QString title(_title);
    if (!title.isEmpty())
      titlePart = QString(" title=\"%1\"").arg(title.replace(QLatin1Char(' '), "&nbsp;"));

    return QString("<a href=\"/%1%2\"%3>").arg(view, query, titlePart);
  }

  QString linkend() const
  {
    return QStringLiteral("</a>");
  }

  void showAssetsLiabilities()
  {
    QList<MyMoneyAccount> accounts;
    QList<MyMoneyAccount>::ConstIterator it;
    QList<MyMoneyAccount> assets;
    QList<MyMoneyAccount> liabilities;
    MyMoneyMoney netAssets;
    MyMoneyMoney netLiabilities;
    QString fontStart, fontEnd;

    MyMoneyFile* file = MyMoneyFile::instance();
    int prec = MyMoneyMoney::denomToPrec(file->baseCurrency().smallestAccountFraction());
    int i = 0;

    // get list of all accounts
    file->accountList(accounts);

    for (it = accounts.constBegin(); it != accounts.constEnd();) {
      if (!(*it).isClosed()) {
        switch ((*it).accountType()) {
            // group all assets into one list but make sure that investment accounts always show up
          case Account::Type::Investment:
            assets << *it;
            break;

          case Account::Type::Checkings:
          case Account::Type::Savings:
          case Account::Type::Cash:
          case Account::Type::Asset:
          case Account::Type::AssetLoan:
            // list account if it's the last in the hierarchy or has transactions in it
            if ((*it).accountList().isEmpty() || (file->transactionCount((*it).id()) > 0)) {
              assets << *it;
            }
            break;

            // group the liabilities into the other
          case Account::Type::CreditCard:
          case Account::Type::Liability:
          case Account::Type::Loan:
            // list account if it's the last in the hierarchy or has transactions in it
            if ((*it).accountList().isEmpty() || (file->transactionCount((*it).id()) > 0)) {
              liabilities << *it;
            }
            break;

          default:
            break;
        }
      }
      ++it;
    }

    //only do it if we have assets or liabilities account
    if (assets.count() > 0 || liabilities.count() > 0) {
      // sort the accounts by name
      qStableSort(assets.begin(), assets.end(), accountNameLess);
      qStableSort(liabilities.begin(), liabilities.end(), accountNameLess);
      QString statusHeader;
      if (KMyMoneySettings::showBalanceStatusOfOnlineAccounts()) {
        QString pathStatusHeader;
        pathStatusHeader = QPixmapToDataUri(Icons::get(Icon::ViewOutbox).pixmap(QSize(16,16)));
        statusHeader = QString("<img src=\"%1\" border=\"0\">").arg(pathStatusHeader);
      }

      //print header
      m_html += "<div class=\"shadow\"><div class=\"displayblock\"><div class=\"summaryheader\">" + i18n("Assets and Liabilities Summary") + "</div>\n<div class=\"gap\">&nbsp;</div>\n";
      m_html += "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"2\" class=\"summarytable\" >";

      //column titles

      m_html += "<tr class=\"item\">";

      if (KMyMoneySettings::showBalanceStatusOfOnlineAccounts()) {
        m_html += "<td class=\"setcolor\">";
        m_html += statusHeader;
        m_html += "</td>";
      }

      m_html += "<td class=\"left\" width=\"30%\">";
      m_html += i18n("Asset Accounts");
      m_html += "</td>";

      if (KMyMoneySettings::showCountOfUnmarkedTransactions())
        m_html += "<td class=\"setcolor\">!M</td>";

      if (KMyMoneySettings::showCountOfClearedTransactions())
        m_html += "<td class=\"setcolor\">C</td>";

      if (KMyMoneySettings::showCountOfNotReconciledTransactions())
        m_html += "<td class=\"setcolor\">!R</td>";

      if (KMyMoneySettings::showDateOfLastReconciliation())
        m_html += "<td class=\"setcolor\">" + i18n("Last Reconciled") + "</td>";

      m_html += "<td width=\"15%\" class=\"right\">";
      m_html += i18n("Current Balance");
      m_html += "</td>";

      //intermediate row to separate both columns
      m_html += "<td width=\"10%\" class=\"setcolor\"></td>";

      if (KMyMoneySettings::showBalanceStatusOfOnlineAccounts()) {
        m_html += "<td class=\"setcolor\">";
        m_html += statusHeader;
        m_html += "</td>";
      }

      m_html += "<td class=\"left\" width=\"30%\">";
      m_html += i18n("Liability Accounts");
      m_html += "</td>";

      if (KMyMoneySettings::showCountOfUnmarkedTransactions())
        m_html += "<td class=\"setcolor\">!M</td>";

      if (KMyMoneySettings::showCountOfClearedTransactions())
        m_html += "<td class=\"setcolor\">C</td>";

      if (KMyMoneySettings::showCountOfNotReconciledTransactions())
        m_html += "<td class=\"setcolor\">!R</td>";

      if (KMyMoneySettings::showDateOfLastReconciliation())
        m_html += "<td class=\"setcolor\">" + i18n("Last Reconciled") + "</td>";

      m_html += "<td width=\"15%\" class=\"right\">";
      m_html += i18n("Current Balance");
      m_html += "</td></tr>";

      QString placeHolder_Status, placeHolder_Counts;
      if (KMyMoneySettings::showBalanceStatusOfOnlineAccounts()) placeHolder_Status = "<td></td>";
      if (KMyMoneySettings::showCountOfUnmarkedTransactions()) placeHolder_Counts = "<td></td>";
      if (KMyMoneySettings::showCountOfClearedTransactions()) placeHolder_Counts += "<td></td>";
      if (KMyMoneySettings::showCountOfNotReconciledTransactions()) placeHolder_Counts += "<td></td>";
      if (KMyMoneySettings::showDateOfLastReconciliation()) placeHolder_Counts += "<td></td>";

      //get asset and liability accounts
      QList<MyMoneyAccount>::const_iterator asset_it = assets.constBegin();
      QList<MyMoneyAccount>::const_iterator liabilities_it = liabilities.constBegin();
      for (; asset_it != assets.constEnd() || liabilities_it != liabilities.constEnd();) {
        m_html += QString("<tr class=\"row-%1\">").arg(i++ & 0x01 ? "even" : "odd");
        //write an asset account if we still have any
        if (asset_it != assets.constEnd()) {
          MyMoneyMoney value;
          //investment accounts consolidate the balance of its subaccounts
          if ((*asset_it).accountType() == Account::Type::Investment) {
            value = investmentBalance(*asset_it);
          } else {
            value = MyMoneyFile::instance()->balance((*asset_it).id(), QDate::currentDate());
          }

          //calculate balance for foreign currency accounts
          if ((*asset_it).currencyId() != file->baseCurrency().id()) {
            ReportAccount repAcc = ReportAccount((*asset_it).id());
            MyMoneyMoney curPrice = repAcc.baseCurrencyPrice(QDate::currentDate());
            MyMoneyMoney baseValue = value * curPrice;
            baseValue = baseValue.convert(10000);
            netAssets += baseValue;
          } else {
            netAssets += value;
          }
          //show the account without minimum balance
          showAccountEntry(*asset_it, value, MyMoneyMoney(), false);
          ++asset_it;
        } else {
          //write a white space if we don't
          m_html += QString("%1<td></td>%2<td></td>").arg(placeHolder_Status).arg(placeHolder_Counts);
        }

        //leave the intermediate column empty
        m_html += "<td class=\"setcolor\"></td>";

        //write a liability account
        if (liabilities_it != liabilities.constEnd()) {
          MyMoneyMoney value;
          value = MyMoneyFile::instance()->balance((*liabilities_it).id(), QDate::currentDate());
          //calculate balance if foreign currency
          if ((*liabilities_it).currencyId() != file->baseCurrency().id()) {
            ReportAccount repAcc = ReportAccount((*liabilities_it).id());
            MyMoneyMoney curPrice = repAcc.baseCurrencyPrice(QDate::currentDate());
            MyMoneyMoney baseValue = value * curPrice;
            baseValue = baseValue.convert(10000);
            netLiabilities += baseValue;
          } else {
            netLiabilities += value;
          }
          //show the account without minimum balance
          showAccountEntry(*liabilities_it, value, MyMoneyMoney(), false);
          ++liabilities_it;
        } else {
          //leave the space empty if we run out of liabilities
          m_html += QString("%1<td></td>%2<td></td>").arg(placeHolder_Status).arg(placeHolder_Counts);
        }
        m_html += "</tr>";
      }

      //calculate net worth
      MyMoneyMoney netWorth = netAssets + netLiabilities;

      //format assets, liabilities and net worth
      QString amountAssets = netAssets.formatMoney(file->baseCurrency().tradingSymbol(), prec);
      QString amountLiabilities = netLiabilities.formatMoney(file->baseCurrency().tradingSymbol(), prec);
      QString amountNetWorth = netWorth.formatMoney(file->baseCurrency().tradingSymbol(), prec);
      amountAssets.replace(QChar(' '), "&nbsp;");
      amountLiabilities.replace(QChar(' '), "&nbsp;");
      amountNetWorth.replace(QChar(' '), "&nbsp;");

      m_html += QString("<tr class=\"row-%1\" style=\"font-weight:bold;\">").arg(i++ & 0x01 ? "even" : "odd");

      //print total for assets
      m_html += QString("%1<td class=\"left\">%2</td>%3<td align=\"right\">%4</td>").arg(placeHolder_Status).arg(i18n("Total Assets")).arg(placeHolder_Counts).arg(showColoredAmount(amountAssets, netAssets.isNegative()));

      //leave the intermediate column empty
      m_html += "<td class=\"setcolor\"></td>";

      //print total liabilities
      m_html += QString("%1<td class=\"left\">%2</td>%3<td align=\"right\">%4</td>").arg(placeHolder_Status).arg(i18n("Total Liabilities")).arg(placeHolder_Counts).arg(showColoredAmount(amountLiabilities, netLiabilities.isNegative()));
      m_html += "</tr>";

      //print net worth
      m_html += QString("<tr class=\"row-%1\" style=\"font-weight:bold;\">").arg(i++ & 0x01 ? "even" : "odd");

      m_html += QString("%1<td></td><td></td>%2<td class=\"setcolor\"></td>").arg(placeHolder_Status).arg(placeHolder_Counts);
      m_html += QString("%1<td class=\"left\">%2</td>%3<td align=\"right\">%4</td>").arg(placeHolder_Status).arg(i18n("Net Worth")).arg(placeHolder_Counts).arg(showColoredAmount(amountNetWorth, netWorth.isNegative()));

      m_html += "</tr>";
      m_html += "</table>";
      m_html += "</div></div>";
    }
  }

  void showBudget()
  {
    MyMoneyFile* file = MyMoneyFile::instance();

    if (file->countBudgets()) {
      int prec = MyMoneyMoney::denomToPrec(file->baseCurrency().smallestAccountFraction());
      bool isOverrun = false;
      int i = 0;

      //config report just like "Monthly Budgeted vs Actual
      MyMoneyReport reportCfg = MyMoneyReport(
                                  MyMoneyReport::eBudgetActual,
                                  MyMoneyReport::eMonths,
                                  TransactionFilter::Date::CurrentMonth,
                                  MyMoneyReport::eDetailAll,
                                  i18n("Monthly Budgeted vs. Actual"),
                                  i18n("Generated Report"));

      reportCfg.setBudget("Any", true);

      reports::PivotTable table(reportCfg);

      PivotGrid grid = table.grid();

      //div header
      m_html += "<div class=\"shadow\"><div class=\"displayblock\"><div class=\"summaryheader\">" + i18n("Budget") + "</div>\n<div class=\"gap\">&nbsp;</div>\n";

      //display budget summary
      m_html += "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"2\" class=\"summarytable\" >";
      m_html += "<tr class=\"itemtitle\">";
      m_html += "<td class=\"left\" colspan=\"3\">";
      m_html += i18n("Current Month Summary");
      m_html += "</td></tr>";
      m_html += "<tr class=\"item\">";
      m_html += "<td class=\"right\" width=\"50%\">";
      m_html += i18n("Budgeted");
      m_html += "</td>";
      m_html += "<td class=\"right\" width=\"20%\">";
      m_html += i18n("Actual");
      m_html += "</td>";
      m_html += "<td class=\"right\" width=\"20%\">";
      m_html += i18n("Difference");
      m_html += "</td></tr>";

      m_html += QString("<tr class=\"row-odd\">");

      MyMoneyMoney totalBudgetValue = grid.m_total[eBudget].m_total;
      MyMoneyMoney totalActualValue = grid.m_total[eActual].m_total;
      MyMoneyMoney totalBudgetDiffValue = grid.m_total[eBudgetDiff].m_total;

      QString totalBudgetAmount = totalBudgetValue.formatMoney(file->baseCurrency().tradingSymbol(), prec);
      QString totalActualAmount = totalActualValue.formatMoney(file->baseCurrency().tradingSymbol(), prec);
      QString totalBudgetDiffAmount = totalBudgetDiffValue.formatMoney(file->baseCurrency().tradingSymbol(), prec);

      m_html += QString("<td align=\"right\">%1</td>").arg(showColoredAmount(totalBudgetAmount, totalBudgetValue.isNegative()));
      m_html += QString("<td align=\"right\">%1</td>").arg(showColoredAmount(totalActualAmount, totalActualValue.isNegative()));
      m_html += QString("<td align=\"right\">%1</td>").arg(showColoredAmount(totalBudgetDiffAmount, totalBudgetDiffValue.isNegative()));
      m_html += "</tr>";
      m_html += "</table>";

      //budget overrun
      m_html += "<div class=\"gap\">&nbsp;</div>\n";
      m_html += "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"2\" class=\"summarytable\" >";
      m_html += "<tr class=\"itemtitle\">";
      m_html += "<td class=\"left\" colspan=\"4\">";
      m_html += i18n("Budget Overruns");
      m_html += "</td></tr>";
      m_html += "<tr class=\"item\">";
      m_html += "<td class=\"left\" width=\"30%\">";
      m_html += i18n("Account");
      m_html += "</td>";
      m_html += "<td class=\"right\" width=\"20%\">";
      m_html += i18n("Budgeted");
      m_html += "</td>";
      m_html += "<td class=\"right\" width=\"20%\">";
      m_html += i18n("Actual");
      m_html += "</td>";
      m_html += "<td class=\"right\" width=\"20%\">";
      m_html += i18n("Difference");
      m_html += "</td></tr>";


      PivotGrid::iterator it_outergroup = grid.begin();
      while (it_outergroup != grid.end()) {
        i = 0;
        PivotOuterGroup::iterator it_innergroup = (*it_outergroup).begin();
        while (it_innergroup != (*it_outergroup).end()) {
          PivotInnerGroup::iterator it_row = (*it_innergroup).begin();
          while (it_row != (*it_innergroup).end()) {
            //column number is 1 because the report includes only current month
            if (it_row.value()[eBudgetDiff].value(1).isNegative()) {
              //get report account to get the name later
              ReportAccount rowname = it_row.key();

              //write the outergroup if it is the first row of outergroup being shown
              if (i == 0) {
                m_html += "<tr style=\"font-weight:bold;\">";
                m_html += QString("<td class=\"left\" colspan=\"4\">%1</td>").arg(MyMoneyAccount::accountTypeToString(rowname.accountType()));
                m_html += "</tr>";
              }
              m_html += QString("<tr class=\"row-%1\">").arg(i++ & 0x01 ? "even" : "odd");

              //get values from grid
              MyMoneyMoney actualValue = it_row.value()[eActual][1];
              MyMoneyMoney budgetValue = it_row.value()[eBudget][1];
              MyMoneyMoney budgetDiffValue = it_row.value()[eBudgetDiff][1];

              //format amounts
              QString actualAmount = actualValue.formatMoney(file->baseCurrency().tradingSymbol(), prec);
              QString budgetAmount = budgetValue.formatMoney(file->baseCurrency().tradingSymbol(), prec);
              QString budgetDiffAmount = budgetDiffValue.formatMoney(file->baseCurrency().tradingSymbol(), prec);

              //account name
              m_html += QString("<td>") + link(VIEW_LEDGER, QString("?id=%1").arg(rowname.id())) + rowname.name() + linkend() + "</td>";

              //show amounts
              m_html += QString("<td align=\"right\">%1</td>").arg(showColoredAmount(budgetAmount, budgetValue.isNegative()));
              m_html += QString("<td align=\"right\">%1</td>").arg(showColoredAmount(actualAmount, actualValue.isNegative()));
              m_html += QString("<td align=\"right\">%1</td>").arg(showColoredAmount(budgetDiffAmount, budgetDiffValue.isNegative()));
              m_html += "</tr>";

              //set the flag that there are overruns
              isOverrun = true;
            }
            ++it_row;
          }
          ++it_innergroup;
        }
        ++it_outergroup;
      }

      //if no negative differences are found, then inform that
      if (!isOverrun) {
        m_html += QString::fromLatin1("<tr class=\"row-%1\" style=\"font-weight:bold;\">").arg(((i++ & 1) == 1) ? QLatin1String("even") : QLatin1String("odd"));
        m_html += QString::fromLatin1("<td class=\"center\" colspan=\"4\">%1</td>").arg(i18n("No Budget Categories have been overrun"));
        m_html += "</tr>";
      }
      m_html += "</table></div></div>";
    }
  }

  void showCashFlowSummary()
  {
    MyMoneyTransactionFilter filter;
    MyMoneyMoney incomeValue;
    MyMoneyMoney expenseValue;

    MyMoneyFile* file = MyMoneyFile::instance();
    int prec = MyMoneyMoney::denomToPrec(file->baseCurrency().smallestAccountFraction());

    //set start and end of month dates
    QDate startOfMonth = QDate(QDate::currentDate().year(), QDate::currentDate().month(), 1);
    QDate endOfMonth = QDate(QDate::currentDate().year(), QDate::currentDate().month(), QDate::currentDate().daysInMonth());

    //Add total income and expenses for this month
    //get transactions for current month
    filter.setDateFilter(startOfMonth, endOfMonth);
    filter.setReportAllSplits(false);

    QList<MyMoneyTransaction> transactions = file->transactionList(filter);
    //if no transaction then skip and print total in zero
    if (transactions.size() > 0) {

      //get all transactions for this month
      foreach (const auto transaction, transactions) {
        //get the splits for each transaction
        foreach (const auto split, transaction.splits()) {
          if (!split.shares().isZero()) {
            ReportAccount repSplitAcc = ReportAccount(split.accountId());

            //only add if it is an income or expense
            if (repSplitAcc.isIncomeExpense()) {
              MyMoneyMoney value;

              //convert to base currency if necessary
              if (repSplitAcc.currencyId() != file->baseCurrency().id()) {
                MyMoneyMoney curPrice = repSplitAcc.baseCurrencyPrice(transaction.postDate());
                value = (split.shares() * MyMoneyMoney::MINUS_ONE) * curPrice;
                value = value.convert(10000);
              } else {
                value = (split.shares() * MyMoneyMoney::MINUS_ONE);
              }

              //store depending on account type
              if (repSplitAcc.accountType() == Account::Type::Income) {
                incomeValue += value;
              } else {
                expenseValue += value;
              }
            }
          }
        }
      }
    }

    //format income and expenses
    QString amountIncome = incomeValue.formatMoney(file->baseCurrency().tradingSymbol(), prec);
    QString amountExpense = expenseValue.formatMoney(file->baseCurrency().tradingSymbol(), prec);
    amountIncome.replace(QChar(' '), "&nbsp;");
    amountExpense.replace(QChar(' '), "&nbsp;");

    //calculate schedules

    //Add all schedules for this month
    MyMoneyMoney scheduledIncome;
    MyMoneyMoney scheduledExpense;
    MyMoneyMoney scheduledLiquidTransfer;
    MyMoneyMoney scheduledOtherTransfer;

    //get overdues and schedules until the end of this month
    QList<MyMoneySchedule> schedule = file->scheduleList(QString(), Schedule::Type::Any,
                                      Schedule::Occurrence::Any,
                                      Schedule::PaymentType::Any,
                                      QDate(),
                                      endOfMonth, false);

    //Remove the finished schedules
    QList<MyMoneySchedule>::Iterator finished_it;
    for (finished_it = schedule.begin(); finished_it != schedule.end();) {
      if ((*finished_it).isFinished()) {
        finished_it = schedule.erase(finished_it);
        continue;
      }
      ++finished_it;
    }

    //add income and expenses
    QList<MyMoneySchedule>::Iterator sched_it;
    for (sched_it = schedule.begin(); sched_it != schedule.end();) {
      QDate nextDate = (*sched_it).nextDueDate();
      int cnt = 0;

      while (nextDate.isValid() && nextDate <= endOfMonth) {
        ++cnt;
        nextDate = (*sched_it).nextPayment(nextDate);
        // for single occurrence nextDate will not change, so we
        // better get out of here.
        if ((*sched_it).occurrence() == Schedule::Occurrence::Once)
          break;
      }

      MyMoneyAccount acc = (*sched_it).account();
      if (!acc.id().isEmpty()) {
        MyMoneyTransaction transaction = (*sched_it).transaction();
        // only show the entry, if it is still active

        MyMoneySplit sp = transaction.splitByAccount(acc.id(), true);

        // take care of the autoCalc stuff
        if ((*sched_it).type() == Schedule::Type::LoanPayment) {
          nextDate = (*sched_it).nextPayment((*sched_it).lastPayment());

          //make sure we have all 'starting balances' so that the autocalc works
          QMap<QString, MyMoneyMoney> balanceMap;

          foreach (const auto split, transaction.splits()) {
            acc = file->account(split.accountId());
            // collect all overdues on the first day
            QDate schedDate = nextDate;
            if (QDate::currentDate() >= nextDate)
              schedDate = QDate::currentDate().addDays(1);

            balanceMap[acc.id()] += file->balance(acc.id(), QDate::currentDate());
          }
          KMyMoneyUtils::calculateAutoLoan(*sched_it, transaction, balanceMap);
        }

        //go through the splits and assign to liquid or other transfers
        const QList<MyMoneySplit> splits = transaction.splits();
        QList<MyMoneySplit>::const_iterator split_it;
        for (split_it = splits.constBegin(); split_it != splits.constEnd(); ++split_it) {
          if ((*split_it).accountId() != acc.id()) {
            ReportAccount repSplitAcc = ReportAccount((*split_it).accountId());

            //get the shares and multiply by the quantity of occurrences in the period
            MyMoneyMoney value = (*split_it).shares() * cnt;

            //convert to foreign currency if needed
            if (repSplitAcc.currencyId() != file->baseCurrency().id()) {
              MyMoneyMoney curPrice = repSplitAcc.baseCurrencyPrice(QDate::currentDate());
              value = value * curPrice;
              value = value.convert(10000);
            }

            if ((repSplitAcc.isLiquidLiability()
                 || repSplitAcc.isLiquidAsset())
                && acc.accountGroup() != repSplitAcc.accountGroup()) {
              scheduledLiquidTransfer += value;
            } else if (repSplitAcc.isAssetLiability()
                       && !repSplitAcc.isLiquidLiability()
                       && !repSplitAcc.isLiquidAsset()) {
              scheduledOtherTransfer += value;
            } else if (repSplitAcc.isIncomeExpense()) {
              //income and expenses are stored as negative values
              if (repSplitAcc.accountType() == Account::Type::Income)
                scheduledIncome -= value;
              if (repSplitAcc.accountType() == Account::Type::Expense)
                scheduledExpense -= value;
            }
          }
        }
      }
      ++sched_it;
    }

    //format the currency strings
    QString amountScheduledIncome = scheduledIncome.formatMoney(file->baseCurrency().tradingSymbol(), prec);
    QString amountScheduledExpense = scheduledExpense.formatMoney(file->baseCurrency().tradingSymbol(), prec);
    QString amountScheduledLiquidTransfer = scheduledLiquidTransfer.formatMoney(file->baseCurrency().tradingSymbol(), prec);
    QString amountScheduledOtherTransfer = scheduledOtherTransfer.formatMoney(file->baseCurrency().tradingSymbol(), prec);

    amountScheduledIncome.replace(QChar(' '), "&nbsp;");
    amountScheduledExpense.replace(QChar(' '), "&nbsp;");
    amountScheduledLiquidTransfer.replace(QChar(' '), "&nbsp;");
    amountScheduledOtherTransfer.replace(QChar(' '), "&nbsp;");

    //get liquid assets and liabilities
    QList<MyMoneyAccount> accounts;
    QList<MyMoneyAccount>::const_iterator account_it;
    MyMoneyMoney liquidAssets;
    MyMoneyMoney liquidLiabilities;

    // get list of all accounts
    file->accountList(accounts);
    for (account_it = accounts.constBegin(); account_it != accounts.constEnd();) {
      if (!(*account_it).isClosed()) {
        switch ((*account_it).accountType()) {
            //group all assets into one list
          case Account::Type::Checkings:
          case Account::Type::Savings:
          case Account::Type::Cash: {
              MyMoneyMoney value = MyMoneyFile::instance()->balance((*account_it).id(), QDate::currentDate());
              //calculate balance for foreign currency accounts
              if ((*account_it).currencyId() != file->baseCurrency().id()) {
                ReportAccount repAcc = ReportAccount((*account_it).id());
                MyMoneyMoney curPrice = repAcc.baseCurrencyPrice(QDate::currentDate());
                MyMoneyMoney baseValue = value * curPrice;
                liquidAssets += baseValue;
                liquidAssets = liquidAssets.convert(10000);
              } else {
                liquidAssets += value;
              }
              break;
            }
            //group the liabilities into the other
          case Account::Type::CreditCard: {
              MyMoneyMoney value;
              value = MyMoneyFile::instance()->balance((*account_it).id(), QDate::currentDate());
              //calculate balance if foreign currency
              if ((*account_it).currencyId() != file->baseCurrency().id()) {
                ReportAccount repAcc = ReportAccount((*account_it).id());
                MyMoneyMoney curPrice = repAcc.baseCurrencyPrice(QDate::currentDate());
                MyMoneyMoney baseValue = value * curPrice;
                liquidLiabilities += baseValue;
                liquidLiabilities = liquidLiabilities.convert(10000);
              } else {
                liquidLiabilities += value;
              }
              break;
            }
          default:
            break;
        }
      }
      ++account_it;
    }
    //calculate net worth
    MyMoneyMoney liquidWorth = liquidAssets + liquidLiabilities;

    //format assets, liabilities and net worth
    QString amountLiquidAssets = liquidAssets.formatMoney(file->baseCurrency().tradingSymbol(), prec);
    QString amountLiquidLiabilities = liquidLiabilities.formatMoney(file->baseCurrency().tradingSymbol(), prec);
    QString amountLiquidWorth = liquidWorth.formatMoney(file->baseCurrency().tradingSymbol(), prec);
    amountLiquidAssets.replace(QChar(' '), "&nbsp;");
    amountLiquidLiabilities.replace(QChar(' '), "&nbsp;");
    amountLiquidWorth.replace(QChar(' '), "&nbsp;");

    //show the summary
    m_html += "<div class=\"shadow\"><div class=\"displayblock\"><div class=\"summaryheader\">" + i18n("Cash Flow Summary") + "</div>\n<div class=\"gap\">&nbsp;</div>\n";

    //print header
    m_html += "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"2\" class=\"summarytable\" >";
    //income and expense title
    m_html += "<tr class=\"itemtitle\">";
    m_html += "<td class=\"left\" colspan=\"4\">";
    m_html += i18n("Income and Expenses of Current Month");
    m_html += "</td></tr>";
    //column titles
    m_html += "<tr class=\"item\">";
    m_html += "<td width=\"25%\" class=\"center\">";
    m_html += i18n("Income");
    m_html += "</td>";
    m_html += "<td width=\"25%\" class=\"center\">";
    m_html += i18n("Scheduled Income");
    m_html += "</td>";
    m_html += "<td width=\"25%\" class=\"center\">";
    m_html += i18n("Expenses");
    m_html += "</td>";
    m_html += "<td width=\"25%\" class=\"center\">";
    m_html += i18n("Scheduled Expenses");
    m_html += "</td>";
    m_html += "</tr>";

    //add row with banding
    m_html += QString("<tr class=\"row-even\" style=\"font-weight:bold;\">");

    //print current income
    m_html += QString("<td align=\"right\">%2</td>").arg(showColoredAmount(amountIncome, incomeValue.isNegative()));

    //print the scheduled income
    m_html += QString("<td align=\"right\">%2</td>").arg(showColoredAmount(amountScheduledIncome, scheduledIncome.isNegative()));

    //print current expenses
    m_html += QString("<td align=\"right\">%2</td>").arg(showColoredAmount(amountExpense,  expenseValue.isNegative()));

    //print the scheduled expenses
    m_html += QString("<td align=\"right\">%2</td>").arg(showColoredAmount(amountScheduledExpense,  scheduledExpense.isNegative()));
    m_html += "</tr>";

    m_html += "</table>";

    //print header of assets and liabilities
    m_html += "<div class=\"gap\">&nbsp;</div>\n";
    m_html += "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"2\" class=\"summarytable\" >";
    //assets and liabilities title
    m_html += "<tr class=\"itemtitle\">";
    m_html += "<td class=\"left\" colspan=\"4\">";
    m_html += i18n("Liquid Assets and Liabilities");
    m_html += "</td></tr>";
    //column titles
    m_html += "<tr class=\"item\">";
    m_html += "<td width=\"25%\" class=\"center\">";
    m_html += i18n("Liquid Assets");
    m_html += "</td>";
    m_html += "<td width=\"25%\" class=\"center\">";
    m_html += i18n("Transfers to Liquid Liabilities");
    m_html += "</td>";
    m_html += "<td width=\"25%\" class=\"center\">";
    m_html += i18n("Liquid Liabilities");
    m_html += "</td>";
    m_html += "<td width=\"25%\" class=\"center\">";
    m_html += i18n("Other Transfers");
    m_html += "</td>";
    m_html += "</tr>";

    //add row with banding
    m_html += QString("<tr class=\"row-even\" style=\"font-weight:bold;\">");

    //print current liquid assets
    m_html += QString("<td align=\"right\">%2</td>").arg(showColoredAmount(amountLiquidAssets, liquidAssets.isNegative()));

    //print the scheduled transfers
    m_html += QString("<td align=\"right\">%2</td>").arg(showColoredAmount(amountScheduledLiquidTransfer, scheduledLiquidTransfer.isNegative()));

    //print current liabilities
    m_html += QString("<td align=\"right\">%2</td>").arg(showColoredAmount(amountLiquidLiabilities,  liquidLiabilities.isNegative()));

    //print the scheduled transfers
    m_html += QString("<td align=\"right\">%2</td>").arg(showColoredAmount(amountScheduledOtherTransfer, scheduledOtherTransfer.isNegative()));


    m_html += "</tr>";

    m_html += "</table>";

    //final conclusion
    MyMoneyMoney profitValue = incomeValue + expenseValue + scheduledIncome + scheduledExpense;
    MyMoneyMoney expectedAsset = liquidAssets + scheduledIncome + scheduledExpense + scheduledLiquidTransfer + scheduledOtherTransfer;
    MyMoneyMoney expectedLiabilities = liquidLiabilities + scheduledLiquidTransfer;

    QString amountExpectedAsset = expectedAsset.formatMoney(file->baseCurrency().tradingSymbol(), prec);
    QString amountExpectedLiabilities = expectedLiabilities.formatMoney(file->baseCurrency().tradingSymbol(), prec);
    QString amountProfit = profitValue.formatMoney(file->baseCurrency().tradingSymbol(), prec);
    amountProfit.replace(QChar(' '), "&nbsp;");
    amountExpectedAsset.replace(QChar(' '), "&nbsp;");
    amountExpectedLiabilities.replace(QChar(' '), "&nbsp;");



    //print header of cash flow status
    m_html += "<div class=\"gap\">&nbsp;</div>\n";
    m_html += "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"2\" class=\"summarytable\" >";
    //income and expense title
    m_html += "<tr class=\"itemtitle\">";
    m_html += "<td class=\"left\" colspan=\"4\">";
    m_html += i18n("Cash Flow Status");
    m_html += "</td></tr>";
    //column titles
    m_html += "<tr class=\"item\">";
    m_html += "<td>&nbsp;</td>";
    m_html += "<td width=\"25%\" class=\"center\">";
    m_html += i18n("Expected Liquid Assets");
    m_html += "</td>";
    m_html += "<td width=\"25%\" class=\"center\">";
    m_html += i18n("Expected Liquid Liabilities");
    m_html += "</td>";
    m_html += "<td width=\"25%\" class=\"center\">";
    m_html += i18n("Expected Profit/Loss");
    m_html += "</td>";
    m_html += "</tr>";

    //add row with banding
    m_html += QString("<tr class=\"row-even\" style=\"font-weight:bold;\">");
    m_html += "<td>&nbsp;</td>";

    //print expected assets
    m_html += QString("<td align=\"right\">%2</td>").arg(showColoredAmount(amountExpectedAsset, expectedAsset.isNegative()));

    //print expected liabilities
    m_html += QString("<td align=\"right\">%2</td>").arg(showColoredAmount(amountExpectedLiabilities, expectedLiabilities.isNegative()));

    //print expected profit
    m_html += QString("<td align=\"right\">%2</td>").arg(showColoredAmount(amountProfit, profitValue.isNegative()));

    m_html += "</tr>";
    m_html += "</table>";
    m_html += "</div></div>";
  }


  KHomeView     *q_ptr;

  /**
   * daily balances of an account
   */
  typedef QMap<QDate, MyMoneyMoney> dailyBalances;

  #ifdef ENABLE_WEBENGINE
  QWebEngineView   *m_view;
  #else
  KWebView         *m_view;
  #endif

  QString           m_html;
  bool              m_showAllSchedules;
  bool              m_needLoad;
  MyMoneyForecast   m_forecast;
  MyMoneyMoney      m_total;
  /**
    * Hold the last valid size of the net worth graph
    * for the times when the needed size can't be computed.
    */
  QSize           m_netWorthGraphLastValidSize;

  /**
    * daily forecast balance of accounts
    */
  QMap<QString, dailyBalances> m_accountList;
  QPrinter *m_currentPrinter;
};

#endif
