/***************************************************************************
                          khomeview.cpp  -  description
                             -------------------
    begin                : Tue Jan 22 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
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

// ----------------------------------------------------------------------------
// QT Includes
#include <q3tl.h>
#include <qlayout.h>
#include <qdatetime.h>
#include <qapplication.h>
//Added by qt3to4:
#include <QList>
#include <QPixmap>
#include <dom/dom_element.h>
#include <dom/dom_doc.h>
#include <dom/dom_text.h>
#include <qfile.h>
#include <q3textstream.h>
#include <qtimer.h>
#include <qbuffer.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <khtmlview.h>
#include <kconfig.h>
#include <kstandardaction.h>
#include <kxmlguiwindow.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kcodecs.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <ktoolinvocation.h>
#include <KToggleAction>
// ----------------------------------------------------------------------------
// Project Includes
#include "khomeview.h"
#include "kmymoneyutils.h"
#include "kmymoneyglobalsettings.h"
#include "mymoneyfile.h"
#include "mymoneyforecast.h"
#include "kmymoney2.h"
//#include "kreportchartview.h"
#include "pivottable.h"
#include "pivotgrid.h"
#include "reportaccount.h"
#include "kmymoneyglobalsettings.h"


#define VIEW_LEDGER         "ledger"
#define VIEW_SCHEDULE       "schedule"
#define VIEW_WELCOME        "welcome"
#define VIEW_HOME           "home"
#define VIEW_REPORTS        "reports"

// in KOffice version < 1.5 KDCHART_PROPSET_NORMAL_DATA was a static const
// but in 1.5 this has been changed into a #define'd value. So we have to
// make sure, we use the right one.
#ifndef KDCHART_PROPSET_NORMAL_DATA
#define KMM_KDCHART_PROPSET_NORMAL_DATA KDChartParams::KDCHART_PROPSET_NORMAL_DATA
#else
#define KMM_KDCHART_PROPSET_NORMAL_DATA KDCHART_PROPSET_NORMAL_DATA
#endif

//using namespace reports;

KHomeView::KHomeView(QWidget *parent, const char *name ) :
  KMyMoneyViewBase(parent, name, i18n("Home")),
  m_showAllSchedules(false),
  m_needReload(true)
{
  m_part = new KHTMLPart(this);
  addWidget(m_part->view());

  m_filename = KMyMoneyUtils::findResource("appdata", QString("html/home%1.html"));

  m_part->openUrl(m_filename);
  connect(m_part->browserExtension(), SIGNAL(openUrlRequest(const KUrl &,
          const KParts::OpenUrlArguments &,const KParts::BrowserArguments & )),
          this, SLOT(slotOpenUrl(const KUrl&, const KParts::OpenUrlArguments &,const KParts::BrowserArguments & )));

  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadView()));
}

KHomeView::~KHomeView()
{
  // if user wants to remember the font size, store it here
  if (KMyMoneyGlobalSettings::rememberFontSize())
  {
    KMyMoneyGlobalSettings::setFontSizePercentage(m_part->zoomFactor());
    //kDebug() << "Storing font size: " << m_part->zoomFactor();
    KMyMoneyGlobalSettings::self()->writeConfig();
  }
}

void KHomeView::slotLoadView(void)
{
  m_needReload = true;
  if(isVisible()) {
    loadView();
    m_needReload = false;
  }
}

void KHomeView::show(void)
{
  if(m_needReload) {
    loadView();
    m_needReload = false;
  }
  QWidget::show();
}

void KHomeView::slotPrintView(void)
{
  if(m_part && m_part->view())
    m_part->view()->print();
}

void KHomeView::loadView(void)
{
  m_part->setZoomFactor( KMyMoneyGlobalSettings::fontSizePercentage() );
  //kDebug() << "Setting font size: " << m_part->zoomFactor();

  QList<MyMoneyAccount> list;
  MyMoneyFile::instance()->accountList(list);
  if(list.count() == 0)
  {
    m_part->openUrl(m_filename);

#if 0
    // (ace) I am experimenting with replacing links in the
    // html depending on the state of the engine.  It's not
    // working.  That's why it's #if0'd out.

    DOM::Element e = m_part->document().getElementById("test");
    if ( e.isNull() )
    {
      qDebug("Element id=test not found");
    }
    else
    {
      qDebug("Element id=test found!");
      QString tagname = e.tagName().string();
      qDebug("%s",tagname.toLatin1());
      qDebug("%s id=%s",e.tagName().string().toLatin1(),e.getAttribute("id").string().toLatin1());

      // Find the character data node
      DOM::Node n = e.firstChild();
      while (!n.isNull())
      {
        qDebug("Child type %u",static_cast<unsigned>(n.nodeType()));
        if ( n.nodeType() == DOM::Node::TEXT_NODE )
        {
          DOM::Text t = n;
          t.setData("Success!!");
          e.replaceChild(n,t);
          m_part->document().setDesignMode(true);
          m_part->document().importNode(e,true);
          m_part->document().updateRendering();

          qDebug("Data is now %s",t.data().string().toLatin1());
        }
        n = n.nextSibling();
      }
    }
#endif
  } else {
    //clear the forecast flag so it will be reloaded
    m_forecast.setForecastDone(false);

    QString filename = KGlobal::dirs()->findResource("appdata", "html/kmymoney2.css");
    QString header = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\">\n<html><head><link rel=\"stylesheet\" type=\"text/css\" href=\"%1\">\n").arg(filename);

    header += KMyMoneyUtils::variableCSS();

    header += "</head><body id=\"summaryview\">\n";

    QString footer = "</body></html>\n";

    m_part->begin();
    m_part->write(header);

    m_part->write(QString("<div id=\"summarytitle\">%1</div>").arg(i18n("Your Financial Summary")));

    QStringList settings = KMyMoneyGlobalSettings::itemList();

    QStringList::ConstIterator it;

    for(it = settings.begin(); it != settings.end(); ++it) {
      int option = (*it).toInt();
      if(option > 0) {
        switch(option) {
          case 1:         // payments
            showPayments();
            break;

          case 2:         // preferred accounts
            showAccounts(Preferred, i18n("Preferred Accounts"));
            break;

          case 3:         // payment accounts
            // Check if preferred accounts are shown separately
            if(settings.find("2") == settings.end()) {
              showAccounts(static_cast<paymentTypeE> (Payment | Preferred),
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
        m_part->write("<div class=\"gap\">&nbsp;</div>\n");
      }
    }

    m_part->write("<div id=\"returnlink\">");
    m_part->write(link(VIEW_WELCOME, QString()) + i18n("Show KMyMoney welcome page") + linkend());
    m_part->write("</div>");
    m_part->write("<div id=\"vieweffect\"></div>");
    m_part->write(footer);
    m_part->end();

  }
}

void KHomeView::showNetWorthGraph(void)
{
#warning "port to kde4"
#if 0
#ifdef HAVE_KDCHART
  m_part->write(QString("<div class=\"shadow\"><div class=\"displayblock\"><div class=\"summaryheader\">%1</div>\n<div class=\"gap\">&nbsp;</div>\n").arg(i18n("Networth Forecast")));

  MyMoneyReport reportCfg = MyMoneyReport(
      MyMoneyReport::eAssetLiability,
      MyMoneyReport::eMonths,
      MyMoneyTransactionFilter::userDefined, // overridden by the setDateFilter() call below
      MyMoneyReport::eDetailTotal,
      i18n("Networth Forecast"),
      i18n("Generated Report"));

  reportCfg.setChartByDefault(true);
  reportCfg.setChartGridLines(false);
  reportCfg.setChartDataLabels(false);
  reportCfg.setChartType(MyMoneyReport::eChartLine);
  reportCfg.setIncludingSchedules( false );
  reportCfg.addAccountGroup(MyMoneyAccount::Asset);
  reportCfg.addAccountGroup(MyMoneyAccount::Liability);
  reportCfg.setColumnsAreDays( true );
  reportCfg.setConvertCurrency( true );
  reportCfg.setIncludingForecast( true );
  reportCfg.setDateFilter(QDate::currentDate(),QDate::currentDate().addDays(+90));
  reports::PivotTable table(reportCfg);

  reports::KReportChartView* chartWidget = new reports::KReportChartView(0, 0);

  table.drawChart(*chartWidget);

  chartWidget->params()->setLineMarker(false);
  chartWidget->params()->setLegendPosition(KDChartParams::NoLegend);
  chartWidget->params()->setLineWidth(2);
  chartWidget->params()->setDataColor(0, KGlobalSettings::textColor());

    // draw future values in a different line style
  KDChartPropertySet propSetFutureValue("future value", KMM_KDCHART_PROPSET_NORMAL_DATA);
  propSetFutureValue.setLineStyle(KDChartPropertySet::OwnID, Qt::DotLine);
  const int idPropFutureValue = chartWidget->params()->registerProperties(propSetFutureValue);

  //KDChartPropertySet propSetLastValue("last value", idPropFutureValue);
  //propSetLastValue.setExtraLinesAlign(KDChartPropertySet::OwnID, Qt::AlignLeft | Qt::AlignBottom);
  //propSetLastValue.setExtraLinesWidth(KDChartPropertySet::OwnID, -4);
  //propSetLastValue.setExtraLinesColor(KDChartPropertySet::OwnID, KMyMoneyGlobalSettings::listGridColor());
  // propSetLastValue.setShowMarker(KDChartPropertySet::OwnID, true);
  // propSetLastValue.setMarkerStyle(KDChartPropertySet::OwnID, KDChartParams::LineMarkerDiamond);

  //const int idPropLastValue = chartWidget->params()->registerProperties(propSetLastValue);
  for(int iCell = 0; iCell < 90; ++iCell) {
    chartWidget->setProperty(0, iCell, idPropFutureValue);
  }
  //chartWidget->setProperty(0, 10, idPropLastValue);

  // Adjust the size
  if(width() < chartWidget->width()) {
    int nh;
    nh = (width()*chartWidget->height() ) / chartWidget->width();
    chartWidget->resize(width()-60, nh);
  }

  QPixmap pm(chartWidget->width(), chartWidget->height());
  pm.fill(KGlobalSettings::baseColor());
  QPainter p(&pm);
  chartWidget->paintTo(p);

  QByteArray ba;
  QBuffer buffer( ba );
  buffer.open( QIODevice::WriteOnly );
  pm.save( &buffer, "PNG" ); // writes pixmap into ba in PNG format

  m_part->write("<table width=\"100%\" cellspacing=\"0\" cellpadding=\"2\" class=\"summarytable\" >");
  m_part->write("<tr>");
  m_part->write(QString("<td><center><IMG SRC=\"data:image/png;base64,%1\" ALT=\"Networth\"></center></td>").arg(KCodecs::base64Encode(ba)));
  m_part->write("</tr>");
  m_part->write("</table></div></div>");

  delete chartWidget;
#endif
#endif
}

void KHomeView::showPayments(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  QList<MyMoneySchedule> overdues;
  QList<MyMoneySchedule> schedule;
  int i = 0;

  //if forecast has not been executed yet, do it.
  if(!m_forecast.isForecastDone())
    doForecast();

  schedule = file->scheduleList("", MyMoneySchedule::TYPE_ANY,
                                 MyMoneySchedule::OCCUR_ANY,
                                 MyMoneySchedule::STYPE_ANY,
                                 QDate::currentDate(),
                                 QDate::currentDate().addMonths(1));
  overdues = file->scheduleList("", MyMoneySchedule::TYPE_ANY,
                                 MyMoneySchedule::OCCUR_ANY,
                                 MyMoneySchedule::STYPE_ANY,
                                 QDate(), QDate(), true);

  if(schedule.empty() && overdues.empty())
    return;

  // HACK
  // Remove the finished schedules

  QList<MyMoneySchedule>::Iterator d_it;
  for (d_it=schedule.begin(); d_it!=schedule.end();)
  {
    // FIXME cleanup old code
    // if ((*d_it).isFinished() || (*d_it).nextPayment((*d_it).lastPayment()) == QDate())
    if ((*d_it).isFinished())
    {
      d_it = schedule.remove(d_it);
      continue;
    }
    ++d_it;
  }

  for (d_it=overdues.begin(); d_it!=overdues.end();)
  {
    // FIXME cleanup old code
    // if ((*d_it).isFinished() || (*d_it).nextPayment((*d_it).lastPayment()) == QDate())
    if ((*d_it).isFinished())
    {
      d_it = overdues.remove(d_it);
      continue;
    }
    ++d_it;
  }

  m_part->write("<div class=\"shadow\"><div class=\"displayblock\">");
  m_part->write(QString("<div class=\"summaryheader\">%1</div>\n").arg(i18n("Payments")));

  if(overdues.count() > 0) {
    m_part->write("<div class=\"gap\">&nbsp;</div>\n");

    qBubbleSort(overdues);
    QList<MyMoneySchedule>::Iterator it;
    QList<MyMoneySchedule>::Iterator it_f;

    m_part->write("<table width=\"100%\" cellspacing=\"0\" cellpadding=\"2\" class=\"summarytable\" >");
    m_part->write(QString("<tr class=\"itemtitle warningtitle\" ><td colspan=\"5\">%1</td></tr>\n").arg(showColoredAmount(i18n("Overdue payments"), true)));
    m_part->write("<tr class=\"item warning\">");
    m_part->write("<td class=\"left\" width=\"10%\">");
    m_part->write(i18n("Date"));
    m_part->write("</td>");
    m_part->write("<td class=\"left\" width=\"40%\">");
    m_part->write(i18n("Schedule"));
    m_part->write("</td>");
    m_part->write("<td class=\"left\" width=\"20%\">");
    m_part->write(i18n("Account"));
    m_part->write("</td>");
    m_part->write("<td class=\"right\" width=\"15%\">");
    m_part->write(i18n("Amount"));
    m_part->write("</td>");
    m_part->write("<td class=\"right\" width=\"15%\">");
    m_part->write(i18n("Balance after"));
    m_part->write("</td>");
    m_part->write("</tr>");

    for(it = overdues.begin(); it != overdues.end(); ++it) {
      // determine number of overdue payments
      QDate nextDate = (*it).nextDueDate();
      int cnt = 0;
      while(nextDate.isValid() && nextDate < QDate::currentDate()) {
        ++cnt;
        nextDate = (*it).nextPayment(nextDate);
        // for single occurence nextDate will not change, so we
        // better get out of here.
        if((*it).occurence() == MyMoneySchedule::OCCUR_ONCE)
          break;
      }

      m_part->write(QString("<tr class=\"row-%1\">").arg(i++ & 0x01 ? "even" : "odd"));
      showPaymentEntry(*it, cnt);
      m_part->write("</tr>");
      // make sure to not repeat overdues later again
      for(it_f = schedule.begin(); it_f != schedule.end();) {
        if((*it).id() == (*it_f).id()) {
          it_f = schedule.remove(it_f);
          continue;
        }
        ++it_f;
      }
    }
    m_part->write("</table>");
  }

  if(schedule.count() > 0) {
    qBubbleSort(schedule);

    // Extract todays payments if any
    QList<MyMoneySchedule> todays;
    QList<MyMoneySchedule>::Iterator t_it;
    for (t_it=schedule.begin(); t_it!=schedule.end();) {
      if ((*t_it).nextDueDate() == QDate::currentDate()) {
        todays.append(*t_it);
        (*t_it).setNextDueDate((*t_it).nextPayment((*t_it).nextDueDate()));

        //if nextDueDate is still currentDate then remove it from scheduled payments
        if ((*t_it).nextDueDate() == QDate::currentDate()) {
          t_it = schedule.remove(t_it);

          //break it that was the last schedule
          if (t_it == schedule.end())
            break;
        }
      }
      ++t_it;
    }

    if (todays.count() > 0) {
      m_part->write("<div class=\"gap\">&nbsp;</div>\n");
      m_part->write("<table width=\"100%\" cellspacing=\"0\" cellpadding=\"2\" class=\"summarytable\" >");
      m_part->write(QString("<tr class=\"itemtitle\"><td class=\"left\" colspan=\"5\">%1</td></tr>\n").arg(i18n("Today's payments")));
      m_part->write("<tr class=\"item\">");
      m_part->write("<td class=\"left\" width=\"10%\">");
      m_part->write(i18n("Date"));
      m_part->write("</td>");
      m_part->write("<td class=\"left\" width=\"40%\">");
      m_part->write(i18n("Schedule"));
      m_part->write("</td>");
      m_part->write("<td class=\"left\" width=\"20%\">");
      m_part->write(i18n("Account"));
      m_part->write("</td>");
      m_part->write("<td class=\"right\" width=\"15%\">");
      m_part->write(i18n("Amount"));
      m_part->write("</td>");
      m_part->write("<td class=\"right\" width=\"15%\">");
      m_part->write(i18n("Balance after"));
      m_part->write("</td>");
      m_part->write("</tr>");

      for(t_it = todays.begin(); t_it != todays.end(); ++t_it) {
        m_part->write(QString("<tr class=\"row-%1\">").arg(i++ & 0x01 ? "even" : "odd"));
        showPaymentEntry(*t_it);
        m_part->write("</tr>");
      }
      m_part->write("</table>");
    }

    if (schedule.count() > 0)
    {
      m_part->write("<div class=\"gap\">&nbsp;</div>\n");

      QList<MyMoneySchedule>::Iterator it;

      m_part->write("<table width=\"100%\" cellspacing=\"0\" cellpadding=\"2\" class=\"summarytable\" >");
      m_part->write(QString("<tr class=\"itemtitle\"><td class=\"left\" colspan=\"5\">%1</td></tr>\n").arg(i18n("Future payments")));
      m_part->write("<tr class=\"item\">");
      m_part->write("<td class=\"left\" width=\"10%\">");
      m_part->write(i18n("Date"));
      m_part->write("</td>");
      m_part->write("<td class=\"left\" width=\"40%\">");
      m_part->write(i18n("Schedule"));
      m_part->write("</td>");
      m_part->write("<td class=\"left\" width=\"20%\">");
      m_part->write(i18n("Account"));
      m_part->write("</td>");
      m_part->write("<td class=\"right\" width=\"15%\">");
      m_part->write(i18n("Amount"));
      m_part->write("</td>");
      m_part->write("<td class=\"right\" width=\"15%\">");
      m_part->write(i18n("Balance after"));
      m_part->write("</td>");
      m_part->write("</tr>");

      // show all or the first 6 entries
      int cnt;
      cnt = (m_showAllSchedules) ? -1 : 6;
      bool needMoreLess = m_showAllSchedules;

      QDate lastDate = QDate::currentDate().addMonths(1);
      qBubbleSort(schedule);
      do {
        it = schedule.begin();
        if(it == schedule.end())
          break;

        // if the next due date is invalid (schedule is finished)
        // we remove it from the list
        QDate nextDate = (*it).nextDueDate();
        if(!nextDate.isValid()) {
          schedule.remove(it);
          continue;
        }

        if (nextDate > lastDate)
          break;

        if(cnt == 0) {
          needMoreLess = true;
          break;
        }
        if(cnt > 0)
          --cnt;

        m_part->write(QString("<tr class=\"row-%1\">").arg(i++ & 0x01 ? "even" : "odd"));
        showPaymentEntry(*it);
        m_part->write("</tr>");

        // for single occurence we have reported everything so we
        // better get out of here.
        if((*it).occurence() == MyMoneySchedule::OCCUR_ONCE) {
          schedule.remove(it);
          continue;
        }

        (*it).setNextDueDate((*it).nextPayment((*it).nextDueDate()));
        qBubbleSort(schedule);
      }
      while(1);

      if (needMoreLess) {
        m_part->write(QString("<tr class=\"row-%1\">").arg(i++ & 0x01 ? "even" : "odd"));
        m_part->write("<td>");
        if(m_showAllSchedules) {
          m_part->write(link(VIEW_SCHEDULE,  QString("?mode=%1").arg("reduced")) + i18n("Less...") + linkend());
        } else {
          m_part->write(link(VIEW_SCHEDULE,  QString("?mode=%1").arg("full")) + i18n("More...") + linkend());
        }
        m_part->write("</td><td></td><td></td><td></td><td></td>");
        m_part->write("</tr>");
      }
      m_part->write("</table>");
    }
  }
  m_part->write("</div></div>");
}

void KHomeView::showPaymentEntry(const MyMoneySchedule& sched, int cnt)
{
  QString tmp;
  MyMoneyFile* file = MyMoneyFile::instance();

  try {
    MyMoneyAccount acc = sched.account();
    if(!acc.id().isEmpty()) {
      MyMoneyTransaction t = sched.transaction();
      // only show the entry, if it is still active
      // FIXME clean old code
      // if(!sched.isFinished() && sched.nextPayment(sched.lastPayment()) != QDate()) {
      if(!sched.isFinished()) {
        MyMoneySplit sp = t.splitByAccount(acc.id(), true);

        QString pathEnter, pathSkip;
        KIconLoader::global()->loadIcon(QString("key_enter"), KIconLoader::Small, KIconLoader::SizeSmall, KIconLoader::DefaultState, QStringList(), &pathEnter, false);
        KIconLoader::global()->loadIcon(QString("player_fwd"), KIconLoader::Small, KIconLoader::SizeSmall, KIconLoader::DefaultState, QStringList(), &pathSkip);

        //show payment date
        tmp = QString("<td>") +
          KGlobal::locale()->formatDate(sched.nextDueDate()) +
          "</td><td>";
        if(pathEnter.length() > 0)
          tmp += link(VIEW_SCHEDULE, QString("?id=%1&mode=enter").arg(sched.id()), i18n("Enter schedule")) + QString("<img src=\"file://%1\" border=\"0\"></a>").arg(pathEnter) + linkend();
        if(pathSkip.length() > 0)
          tmp += "&nbsp;" + link(VIEW_SCHEDULE, QString("?id=%1&mode=skip").arg(sched.id()), i18n("Skip schedule")) + QString("<img src=\"file://%1\" border=\"0\"></a>").arg(pathSkip) + linkend();

        tmp += QString("&nbsp;");
        tmp += link(VIEW_SCHEDULE, QString("?id=%1&mode=edit").arg(sched.id()), i18n("Edit schedule")) + sched.name() + linkend();

        //show quantity of payments overdue if any
        if(cnt > 1)
          tmp += i18n(" (%1 payments)").arg(cnt);

        //show account of the main split
        tmp += "</td><td>";
        tmp += QString(file->account(acc.id()).name());

        //show amount of the schedule
        tmp += "</td><td align=\"right\">";

        const MyMoneySecurity& currency = MyMoneyFile::instance()->currency(acc.currencyId());
        QString amount = (sp.value()*cnt).formatMoney(acc, currency);
        amount.replace(" ","&nbsp;");
        tmp += showColoredAmount(amount, (sp.value()*cnt).isNegative()) ;
        tmp += "</td>";
        //show balance after payments
        tmp += "<td align=\"right\">";
        MyMoneyMoney payment = MyMoneyMoney((sp.value()*cnt));
        QDate paymentDate = QDate(sched.nextDueDate());
        MyMoneyMoney balanceAfter = forecastPaymentBalance(acc, payment, paymentDate);
        QString balance = balanceAfter.formatMoney(acc, currency);
        balance.replace(" ","&nbsp;");
        tmp += showColoredAmount(balance, balanceAfter.isNegative());
        tmp += "</td>";

        // qDebug("paymentEntry = '%s'", tmp.toLatin1());
        m_part->write(tmp);
      }
    }
  } catch(MyMoneyException* e) {
    qDebug("Unable to display schedule entry: %s", e->what().data());
    delete e;
  }
}

void KHomeView::showAccounts(KHomeView::paymentTypeE type, const QString& header)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  QList<MyMoneyAccount> accounts;
  QList<MyMoneyAccount>::Iterator it;
  QList<MyMoneyAccount>::Iterator prevIt;
  QMap<QString, MyMoneyAccount> nameIdx;

  bool showClosedAccounts = kmymoney2->toggleAction("view_show_all_accounts")->isChecked();

  // get list of all accounts
  file->accountList(accounts);
  for(it = accounts.begin(); it != accounts.end();) {
    prevIt = it;
    if(!(*it).isClosed() || showClosedAccounts) {
      switch((*it).accountType()) {
        case MyMoneyAccount::Expense:
        case MyMoneyAccount::Income:
          // never show a category account
          // Note: This might be different in a future version when
          //       the homepage also shows category based information
          it = accounts.remove(it);
          break;

        // Asset and Liability accounts are only shown if they
        // have the preferred flag set
        case MyMoneyAccount::Asset:
        case MyMoneyAccount::Liability:
        case MyMoneyAccount::Investment:
          // if preferred accounts are requested, then keep in list
          if((*it).value("PreferredAccount") != "Yes"
          || (type & Preferred) == 0) {
            it = accounts.remove(it);
          }
          break;

        // Check payment accounts. If payment and preferred is selected,
        // then always show them. If only payment is selected, then
        // show only if preferred flag is not set.
        case MyMoneyAccount::Checkings:
        case MyMoneyAccount::Savings:
        case MyMoneyAccount::Cash:
        case MyMoneyAccount::CreditCard:
          switch(type & (Payment | Preferred)) {
            case Payment:
              if((*it).value("PreferredAccount") == "Yes")
                it = accounts.remove(it);
              break;

            case Preferred:
              if((*it).value("PreferredAccount") != "Yes")
                it = accounts.remove(it);
              break;

            case Payment | Preferred:
              break;

            default:
              it = accounts.remove(it);
              break;
          }
          break;

        // filter all accounts that are not used on homepage views
        default:
          it = accounts.remove(it);
          break;
      }

    } else if((*it).isClosed() || (*it).isInvest()) {
      // don't show if closed or a stock account
      it = accounts.remove(it);
    }

    // if we still point to the same account we keep it in the list and move on ;-)
    if(prevIt == it) {
      QString key = (*it).name();
      if(nameIdx[key].id().isEmpty()) {
        nameIdx[key] = *it;

      } else if(nameIdx[key].id() != (*it).id()) {
        key = (*it).name() + "[%1]";
        int dup = 2;
        while(!nameIdx[key.arg(dup)].id().isEmpty()
        && nameIdx[key.arg(dup)].id() != (*it).id())
          ++dup;
        nameIdx[key.arg(dup)] = *it;
      }
      ++it;
    }
  }

  if(accounts.count() > 0) {
    QString tmp;
    int i = 0;
    tmp = "<div class=\"shadow\"><div class=\"displayblock\"><div class=\"summaryheader\">" + header + "</div>\n<div class=\"gap\">&nbsp;</div>\n";
    m_part->write(tmp);
    m_part->write("<table width=\"100%\" cellspacing=\"0\" cellpadding=\"2\" class=\"summarytable\" >");
    m_part->write("<tr class=\"item\"><td class=\"left\" width=\"35%\">");
    m_part->write(i18n("Account"));
    m_part->write("</td><td width=\"25%\" class=\"right\">");
    m_part->write(i18n("Current Balance"));
    m_part->write("</td>");
    //only show limit info if user chose to do so
    if(KMyMoneyGlobalSettings::showLimitInfo()) {
      m_part->write("<td width=\"40%\" class=\"right\">");
      m_part->write(i18n("To Minimum Balance / Maximum Credit"));
      m_part->write("</td>");
    }
    m_part->write("</tr>");


    QMap<QString, MyMoneyAccount>::const_iterator it_m;
    for(it_m = nameIdx.begin(); it_m != nameIdx.end(); ++it_m) {
      m_part->write(QString("<tr class=\"row-%1\">").arg(i++ & 0x01 ? "even" : "odd"));
      showAccountEntry(*it_m);
      m_part->write("</tr>");
    }
    m_part->write("</table></div></div>");
  }
}

void KHomeView::showAccountEntry(const MyMoneyAccount& acc)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneySecurity currency = file->currency(acc.currencyId());
  MyMoneyMoney value;

  bool showLimit = KMyMoneyGlobalSettings::showLimitInfo();

  if(acc.accountType() == MyMoneyAccount::Investment) {
    //investment accounts show the balances of all its subaccounts
    value = investmentBalance(acc);

    //investment accounts have no minimum balance
    showAccountEntry(acc, value, MyMoneyMoney(), showLimit);
  } else {
    //get balance for normal accounts
    value = file->balance(acc.id(), QDate::currentDate());

    //if credit card or checkings account, show maximum credit
    if( acc.accountType() == MyMoneyAccount::CreditCard ||
        acc.accountType() == MyMoneyAccount::Checkings ) {
      QString maximumCredit = acc.value("maxCreditAbsolute");
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

void KHomeView::showAccountEntry(const MyMoneyAccount& acc, const MyMoneyMoney& value, const MyMoneyMoney& valueToMinBal, const bool showMinBal)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  QString tmp;
  MyMoneySecurity currency = file->currency(acc.currencyId());
  QString amount;
  QString amountToMinBal;

  //format amounts
  amount = value.formatMoney(acc, currency);
  amount.replace(" ","&nbsp;");
  if(showMinBal) {
    amountToMinBal = valueToMinBal.formatMoney(acc, currency);
    amountToMinBal.replace(" ","&nbsp;");
  }

  tmp = QString("<td>") +
      link(VIEW_LEDGER, QString("?id=%1").arg(acc.id())) + acc.name() + linkend() + "</td>";

  //show account balance
  tmp += QString("<td class=\"right\">%1</td>").arg(showColoredAmount(amount, value.isNegative()));

  //show minimum balance column if requested
  if(showMinBal) {
    //if it is an investment, show minimum balance empty
    if(acc.accountType() == MyMoneyAccount::Investment) {
      tmp += QString("<td class=\"right\">&nbsp;</td>");
    } else {
      //show minimum balance entry
      tmp += QString("<td class=\"right\">%1</td>").arg(showColoredAmount(amountToMinBal, valueToMinBal.isNegative()));
    }
  }
  // qDebug("accountEntry = '%s'", tmp.toLatin1());
  m_part->write(tmp);
}

MyMoneyMoney KHomeView::investmentBalance(const MyMoneyAccount& acc)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyMoney value;
  value = file->balance(acc.id());
  QList<QString>::const_iterator it_a;
  for(it_a = acc.accountList().begin(); it_a != acc.accountList().end(); ++it_a) {
    MyMoneyAccount stock = file->account(*it_a);
    try {
      MyMoneyMoney val;
      MyMoneyMoney balance = file->balance(stock.id());
      MyMoneySecurity security = file->security(stock.currencyId());
      MyMoneyPrice price = file->price(stock.currencyId(), security.tradingCurrency());
      val = (balance * price.rate(security.tradingCurrency())).convert(MyMoneyMoney::precToDenom(KMyMoneyGlobalSettings::pricePrecision()));
      // adjust value of security to the currency of the account
      MyMoneySecurity accountCurrency = file->currency(acc.currencyId());
      val = val * file->price(security.tradingCurrency(), accountCurrency.id()).rate(accountCurrency.id());
      val = val.convert(acc.fraction());
      value += val;
    } catch(MyMoneyException* e) {
      qWarning("%s", qPrintable(QString("cannot convert stock balance of %1 to base currency: %2").arg(stock.name(), e->what())));
      delete e;
    }
  }
  return value;
}

void KHomeView::showFavoriteReports(void)
{
  QList<MyMoneyReport> reports = MyMoneyFile::instance()->reportList();

  if ( reports.count() > 0 )
  {
    bool firstTime = 1;
    int row = 0;
    QList<MyMoneyReport>::const_iterator it_report = reports.begin();
    while( it_report != reports.end() )
    {
      if ( (*it_report).isFavorite() ) {
        if(firstTime) {
          m_part->write(QString("<div class=\"shadow\"><div class=\"displayblock\"><div class=\"summaryheader\">%1</div>\n<div class=\"gap\">&nbsp;</div>\n").arg(i18n("Favorite Reports")));
          m_part->write("<table width=\"100%\" cellspacing=\"0\" cellpadding=\"2\" class=\"summarytable\" >");
          m_part->write("<tr class=\"item\"><td class=\"left\" width=\"40%\">");
          m_part->write(i18n("Report"));
          m_part->write("</td><td width=\"60%\" class=\"left\">");
          m_part->write(i18n("Comment"));
          m_part->write("</td></tr>");
          firstTime = false;
        }

        m_part->write(QString("<tr class=\"row-%1\"><td>%2%3%4</td><td align=\"left\">%5</td></tr>")
          .arg(row++ & 0x01 ? "even" : "odd")
          .arg(link(VIEW_REPORTS, QString("?id=%1").arg((*it_report).id())))
          .arg((*it_report).name())
          .arg(linkend())
          .arg((*it_report).comment())
        );
      }

      ++it_report;
    }
    if(!firstTime)
      m_part->write("</table></div></div>");
  }
}

void KHomeView::showForecast(void)
{
  QMap<QString, MyMoneyAccount> nameIdx;
  MyMoneyFile* file = MyMoneyFile::instance();
  QList<MyMoneyAccount> accList;

  //if forecast has not been executed yet, do it.
  if(!m_forecast.isForecastDone())
    doForecast();

  accList = m_forecast.accountList();

  //add it to a map to have it ordered by name
  QList<MyMoneyAccount>::const_iterator accList_t = accList.begin();
  for ( ; accList_t != accList.end(); ++accList_t )
  {
    QString key = (*accList_t).name();
    if(nameIdx[key].id().isEmpty()) {
      nameIdx[key] = *accList_t;
            //take care of accounts with duplicate names
    } else if(nameIdx[key].id() != (*accList_t).id()) {
      key = (*accList_t).name() + "[%1]";
      int dup = 2;
      while(!nameIdx[key.arg(dup)].id().isEmpty()
             && nameIdx[key.arg(dup)].id() != (*accList_t).id())
        ++dup;
      nameIdx[key.arg(dup)] = *accList_t;
    }
  }

  if(nameIdx.count() > 0) {
    int i = 0;

    int colspan = 1;
    //get begin day
    int beginDay = QDate::currentDate().daysTo(m_forecast.beginForecastDate());
    //if begin day is today skip to next cycle
    if(beginDay == 0)
      beginDay = m_forecast.accountsCycle();

    // Now output header
    m_part->write(QString("<div class=\"shadow\"><div class=\"displayblock\"><div class=\"summaryheader\">%1</div>\n<div class=\"gap\">&nbsp;</div>\n").arg(i18n("%1 Day Forecast").arg(m_forecast.forecastDays())));
    m_part->write("<table width=\"100%\" cellspacing=\"0\" cellpadding=\"2\" class=\"summarytable\" >");
    m_part->write("<tr class=\"item\"><td class=\"left\" width=\"40%\">");
    m_part->write(i18n("Account"));
    m_part->write("</td>");
    int colWidth = 55/ (m_forecast.forecastDays() / m_forecast.accountsCycle());
    for(i = 0; (i*m_forecast.accountsCycle() + beginDay) <= m_forecast.forecastDays(); ++i) {
      m_part->write(QString("<td width=\"%1%\" class=\"right\">").arg(colWidth));

      m_part->write(i18n("%1 days").arg(i*m_forecast.accountsCycle() + beginDay));
      m_part->write("</td>");
      colspan++;
    }
    m_part->write("</tr>");

    // Now output entries
    i = 0;

    QMap<QString, MyMoneyAccount>::ConstIterator it_account;
    for(it_account = nameIdx.begin(); it_account != nameIdx.end(); ++it_account) {
      //MyMoneyAccount acc = (*it_n);

      m_part->write(QString("<tr class=\"row-%1\">").arg(i++ & 0x01 ? "even" : "odd"));
      m_part->write(QString("<td width=\"40%\">") +
          link(VIEW_LEDGER, QString("?id=%1").arg((*it_account).id())) + (*it_account).name() + linkend() + "</td>");

      int dropZero = -1; //account dropped below zero
      int dropMinimum = -1; //account dropped below minimum balance
      QString minimumBalance = (*it_account).value("minimumBalance");
      MyMoneyMoney minBalance = MyMoneyMoney(minimumBalance);
      MyMoneySecurity currency;
      MyMoneyMoney forecastBalance;

      //change account to deep currency if account is an investment
      if((*it_account).isInvest()) {
        MyMoneySecurity underSecurity = file->security((*it_account).currencyId());
        currency = file->security(underSecurity.tradingCurrency());
      } else {
        currency = file->security((*it_account).currencyId());
      }

      for (int f = beginDay; f <= m_forecast.forecastDays(); f += m_forecast.accountsCycle()) {
        forecastBalance = m_forecast.forecastBalance(*it_account, QDate::currentDate().addDays(f));
        QString amount;
        amount = forecastBalance.formatMoney( *it_account, currency);
        amount.replace(" ","&nbsp;");
        m_part->write(QString("<td width=\"%1%\" align=\"right\">").arg(colWidth));
        m_part->write(QString("%1</td>").arg(showColoredAmount(amount, forecastBalance.isNegative())));
      }

      m_part->write("</tr>");

      //Check if the account is going to be below zero or below the minimal balance in the forecast period

      //Check if the account is going to be below minimal balance
      dropMinimum = m_forecast.daysToMinimumBalance(*it_account);

      //Check if the account is going to be below zero in the future
      dropZero = m_forecast.daysToZeroBalance(*it_account);


      // spit out possible warnings
      QString msg;

      // if a minimum balance has been specified, an appropriate warning will
      // only be shown, if the drop below 0 is on a different day or not present

      if(dropMinimum != -1
         && !minBalance.isZero()
         && (dropMinimum < dropZero
         || dropZero == -1)) {
        switch(dropMinimum) {
          case -1:
            break;
          case 0:
            msg = i18n("The balance of %1 is below the minimum balance %2 today.",(*it_account).name(),minBalance.formatMoney(*it_account, currency));
            msg = showColoredAmount(msg, true);
            break;
          default:
            msg = i18n("The balance of %1 will drop below the minimum balance %2 in %3 days.",(*it_account).name(),minBalance.formatMoney(*it_account, currency),dropMinimum-1);
            msg = showColoredAmount(msg, true);
            break;
        }

        if(!msg.isEmpty()) {
          m_part->write(QString("<tr class=\"warning\" style=\"font-weight: normal;\" ><td colspan=%2 align=\"center\" >%1</td></tr>").arg(msg).arg(colspan));
        }
         }
      // a drop below zero is always shown
         msg = QString();
         switch(dropZero) {
           case -1:
             break;
           case 0:
             if((*it_account).accountGroup() == MyMoneyAccount::Asset) {
               msg = i18n("The balance of %1 is below %2 today.",(*it_account).name(),MyMoneyMoney().formatMoney(*it_account, currency));
               msg = showColoredAmount(msg, true);
               break;
             }
             if((*it_account).accountGroup() == MyMoneyAccount::Liability) {
               msg = i18n("The balance of %1 is above %2 today.",(*it_account).name(),MyMoneyMoney().formatMoney(*it_account, currency));
               break;
             }
             break;
           default:
             if((*it_account).accountGroup() == MyMoneyAccount::Asset) {
               msg = i18n("The balance of %1 will drop below %2 in %3 days.",(*it_account).name(),MyMoneyMoney().formatMoney(*it_account, currency),dropZero);
               msg = showColoredAmount(msg, true);
               break;
             }
             if((*it_account).accountGroup() == MyMoneyAccount::Liability) {
               msg = i18n("The balance of %1 will raise above %2 in %3 days.",(*it_account).name(),MyMoneyMoney().formatMoney(*it_account, currency),dropZero);
               break;
             }
         }
         if(!msg.isEmpty()) {
           m_part->write(QString("<tr class=\"warning\"><td colspan=%2 align=\"center\" ><b>%1</b></td></tr>").arg(msg).arg(colspan));
         }
    }
    m_part->write("</table></div></div>");

  }
}

const QString KHomeView::link(const QString& view, const QString& query, const QString& _title) const
{
  QString titlePart;
  QString title(_title);
  if(!title.isEmpty())
    titlePart = QString(" title=\"%1\"").arg(title.replace(" ", "&nbsp;"));

  return QString("<a href=\"/%1%2\"%3>").arg(view, query, titlePart);
}

const QString KHomeView::linkend(void) const
{
  return "</a>";
}

void KHomeView::slotOpenUrl(const KUrl &url, const KParts::OpenUrlArguments&,const KParts::BrowserArguments& )
{
  QString protocol = url.protocol();
  QString view = url.fileName(false);
  QString id = url.queryItem("id");
  QString mode = url.queryItem("mode");

  if ( protocol == "http" )
  {
    KToolInvocation::invokeBrowser(url.prettyUrl());
  }
  else if ( protocol == "mailto" )
  {
    KToolInvocation::invokeMailer(url);
  }
  else
  {
    if(view == VIEW_LEDGER) {
      emit ledgerSelected(id, QString());

    } else if(view == VIEW_SCHEDULE) {
      if(mode == "enter") {
        emit scheduleSelected(id);
        KXmlGuiWindow* mw = dynamic_cast<KXmlGuiWindow*>(qApp->mainWidget());
        Q_CHECK_PTR(mw);
        QTimer::singleShot(0, mw->actionCollection()->action("schedule_enter"), SLOT(activate()));
      } else if(mode == "edit") {
        emit scheduleSelected(id);
        KXmlGuiWindow* mw = dynamic_cast<KXmlGuiWindow*>(qApp->mainWidget());
        Q_CHECK_PTR(mw);
        QTimer::singleShot(0, mw->actionCollection()->action("schedule_edit"), SLOT(activate()));
      } else if(mode == "skip") {
        emit scheduleSelected(id);
        KXmlGuiWindow* mw = dynamic_cast<KXmlGuiWindow*>(qApp->mainWidget());
        Q_CHECK_PTR(mw);
        QTimer::singleShot(0, mw->actionCollection()->action("schedule_skip"), SLOT(activate()));
      } else if(mode == "full") {
        m_showAllSchedules = true;
        loadView();

      } else if(mode == "reduced") {
        m_showAllSchedules = false;
        loadView();
      }

    } else if(view == VIEW_REPORTS) {
      emit reportSelected(id);

    } else if(view == VIEW_WELCOME) {
      KXmlGuiWindow* mw = dynamic_cast<KXmlGuiWindow*>(qApp->mainWidget());
      Q_CHECK_PTR(mw);
      if ( mode == "whatsnew" )
      {
        QString fname = KMyMoneyUtils::findResource("appdata",QString("html/whats_new%1.html"));
        if(!fname.isEmpty())
          m_part->openUrl(fname);
      }
      else
        m_part->openUrl(m_filename);

    } else if(view == "action") {
      KXmlGuiWindow* mw = dynamic_cast<KXmlGuiWindow*>(qApp->mainWidget());
      Q_CHECK_PTR(mw);
      QTimer::singleShot(0, mw->actionCollection()->action( id ), SLOT(activate()));
    } else if(view == VIEW_HOME) {
      QList<MyMoneyAccount> list;
      MyMoneyFile::instance()->accountList(list);
      if(list.count() == 0) {
        KMessageBox::information(this, i18n("Before KMyMoney can give you detailed information about your financial status, you need to create at least one account. Until then, KMyMoney shows the welcome page instead."));
      }
      loadView();

    } else {
      qDebug("Unknown view '%s' in KHomeView::slotOpenURL()", qPrintable(view));
    }
  }
}

void KHomeView::showAssetsLiabilities(void)
{
  #warning #Port to KDE4
  #if 0
  QList<MyMoneyAccount> accounts;
  QList<MyMoneyAccount>::Iterator it;
  QMap<QString, MyMoneyAccount> nameAssetsIdx;
  QMap<QString, MyMoneyAccount> nameLiabilitiesIdx;
  MyMoneyMoney netAssets;
  MyMoneyMoney netLiabilities;
  QString fontStart, fontEnd;

  MyMoneyFile* file = MyMoneyFile::instance();
  int prec = MyMoneyMoney::denomToPrec(file->baseCurrency().smallestAccountFraction());
  int i = 0;


  // get list of all accounts
  file->accountList(accounts);
  for(it = accounts.begin(); it != accounts.end();) {
    if(!(*it).isClosed()) {
      switch((*it).accountType()) {
        //group all assets into one list
        case MyMoneyAccount::Checkings:
        case MyMoneyAccount::Savings:
        case MyMoneyAccount::Cash:
        case MyMoneyAccount::Investment:
        case MyMoneyAccount::Asset:
        case MyMoneyAccount::AssetLoan:
        {
          //add it to a map to have it ordered by name
          QString key = (*it).name();
          if(nameAssetsIdx[key].id().isEmpty()) {
            nameAssetsIdx[key] = *it;
            //take care of accounts with duplicate names
          } else if(nameAssetsIdx[key].id() != (*it).id()) {
            key = (*it).name() + "[%1]";
            int dup = 2;
            while(!nameAssetsIdx[key.arg(dup)].id().isEmpty()
                   && nameAssetsIdx[key.arg(dup)].id() != (*it).id())
              ++dup;
            nameAssetsIdx[key.arg(dup)] = *it;
          }
          break;
        }
        //group the liabilities into the other
        case MyMoneyAccount::CreditCard:
        case MyMoneyAccount::Liability:
        case MyMoneyAccount::Loan:
        {
          //add it to a map to have it ordered by name
          QString key = (*it).name();
          if(nameLiabilitiesIdx[key].id().isEmpty()) {
            nameLiabilitiesIdx[key] = *it;
            //take care of duplicate account names
          } else if(nameLiabilitiesIdx[key].id() != (*it).id()) {
            key = (*it).name() + "[%1]";
            int dup = 2;
            while(!nameLiabilitiesIdx[key.arg(dup)].id().isEmpty()
                   && nameLiabilitiesIdx[key.arg(dup)].id() != (*it).id())
              ++dup;
            nameLiabilitiesIdx[key.arg(dup)] = *it;
          }
          break;
        }
        default:
          break;
      }
    }
    ++it;
  }

  //only do it if we have assets or liabilities account
  if(nameAssetsIdx.count() > 0 || nameLiabilitiesIdx.count() > 0) {
    //print header
    m_part->write("<div class=\"shadow\"><div class=\"displayblock\"><div class=\"summaryheader\">" + i18n("Assets and Liabilities Summary") + "</div>\n<div class=\"gap\">&nbsp;</div>\n");
    m_part->write("<table width=\"100%\" cellspacing=\"0\" cellpadding=\"2\" class=\"summarytable\" >");
    //column titles
    m_part->write("<tr class=\"item\"><td class=\"left\" width=\"30%\">");
    m_part->write(i18n("Asset Accounts"));
    m_part->write("</td>");
    m_part->write("<td width=\"15%\" class=\"right\">");
    m_part->write(i18n("Current Balance"));
    m_part->write("</td>");
    //intermediate row to separate both columns
    m_part->write("<td width=\"10%\" class=\"setcolor\"></td>");
    m_part->write("<td class=\"left\" width=\"30%\">");
    m_part->write(i18n("Liability Accounts"));
    m_part->write("</td>");
    m_part->write("<td width=\"15%\" class=\"right\">");
    m_part->write(i18n("Current Balance"));
    m_part->write("</td></tr>");

    //get asset and liability accounts
    QMap<QString, MyMoneyAccount>::const_iterator asset_it = nameAssetsIdx.begin();
    QMap<QString,MyMoneyAccount>::const_iterator liabilities_it = nameLiabilitiesIdx.begin();
    for(; asset_it != nameAssetsIdx.end() || liabilities_it != nameLiabilitiesIdx.end();) {
      m_part->write(QString("<tr class=\"row-%1\">").arg(i++ & 0x01 ? "even" : "odd"));
      //write an asset account if we still have any
      if(asset_it != nameAssetsIdx.end()) {
        MyMoneyMoney value;
        //investment accounts consolidate the balance of its subaccounts
        if( (*asset_it).accountType() == MyMoneyAccount::Investment) {
          value = investmentBalance(*asset_it);
        } else {
          value = MyMoneyFile::instance()->balance((*asset_it).id(), QDate::currentDate());
        }
        //calculate balance for foreign currency accounts
        if((*asset_it).currencyId() != file->baseCurrency().id()) {
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
        m_part->write("<td></td><td></td>");
      }

      //leave the intermediate column empty
      m_part->write("<td class=\"setcolor\"></td>");

      //write a liability account
      if(liabilities_it != nameLiabilitiesIdx.end()) {
        MyMoneyMoney value;
        value = MyMoneyFile::instance()->balance((*liabilities_it).id(), QDate::currentDate());
        //calculate balance if foreign currency
        if((*liabilities_it).currencyId() != file->baseCurrency().id()) {
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
        m_part->write("<td></td><td></td>");
      }
      m_part->write("</tr>");
    }
    //calculate net worth
    MyMoneyMoney netWorth = netAssets+netLiabilities;

    //format assets, liabilities and net worth
    QString amountAssets = netAssets.formatMoney(file->baseCurrency().tradingSymbol(), prec);
    QString amountLiabilities = netLiabilities.formatMoney(file->baseCurrency().tradingSymbol(), prec);
    QString amountNetWorth = netWorth.formatMoney(file->baseCurrency().tradingSymbol(), prec);
    amountAssets.replace(" ","&nbsp;");
    amountLiabilities.replace(" ","&nbsp;");
    amountNetWorth.replace(" ","&nbsp;");

    m_part->write(QString("<tr class=\"row-%1\" style=\"font-weight:bold;\">").arg(i++ & 0x01 ? "even" : "odd"));

    //print total for assets
    m_part->write(QString("<td class=\"left\">%1</td><td align=\"right\">%2</td>").arg(i18n("Total Assets")).arg(showColoredAmount(amountAssets, netAssets.isNegative())));

    //leave the intermediate column empty
    m_part->write("<td class=\"setcolor\"></td>");

    //print total liabilities
    m_part->write(QString("<td class=\"left\">%1</td><td align=\"right\">%2</td>").arg(i18n("Total Liabilities")).arg(showColoredAmount(amountLiabilities, netLiabilities.isNegative())));
    m_part->write("</tr>");

    //print net worth
    m_part->write(QString("<tr class=\"row-%1\" style=\"font-weight:bold;\">").arg(i++ & 0x01 ? "even" : "odd"));

    m_part->write("<td></td><td></td><td class=\"setcolor\"></td>");
    m_part->write(QString("<td class=\"left\">%1</td><td align=\"right\">%2</td>").arg(i18n("Net Worth")).arg(showColoredAmount(amountNetWorth, netWorth.isNegative() )));

    m_part->write("</tr>");
    m_part->write("</table>");
    m_part->write("</div></div>");
  }
  #endif
}

void KHomeView::showBudget(void)
{
#warning "port to kde4"
#if 0
    MyMoneyFile* file = MyMoneyFile::instance();

  if ( file->countBudgets() ) {
    int prec = MyMoneyMoney::denomToPrec(file->baseCurrency().smallestAccountFraction());
    int i = 0;

    //config report just like "Monthly Budgeted vs Actual
    MyMoneyReport reportCfg = MyMoneyReport(
      MyMoneyReport::eBudgetActual,
      MyMoneyReport::eMonths,
      MyMoneyTransactionFilter::currentMonth,
      MyMoneyReport::eDetailAll,
      i18n("Monthly Budgeted vs. Actual"),
      i18n("Generated Report"));

    reportCfg.setBudget("Any",true);

    reports::PivotTable table(reportCfg);

    PivotGrid grid = table.grid();

    //div header
    m_part->write("<div class=\"shadow\"><div class=\"displayblock\"><div class=\"summaryheader\">" + i18n("Budget") + "</div>\n<div class=\"gap\">&nbsp;</div>\n");

    //display budget summary
    m_part->write("<table width=\"75%\" cellspacing=\"0\" cellpadding=\"2\" class=\"summarytable\" >");
    m_part->write("<tr class=\"itemtitle\">");
    m_part->write("<td class=\"left\" colspan=\"3\">");
    m_part->write(i18n("Current Month Summary"));
    m_part->write("</td></tr>");
    m_part->write("<tr class=\"item\">");
    m_part->write("<td class=\"right\" width=\"33%\">");
    m_part->write(i18n("Budgeted"));
    m_part->write("</td>");
    m_part->write("<td class=\"right\" width=\"33%\">");
    m_part->write(i18n("Actual"));
    m_part->write("</td>");
    m_part->write("<td class=\"right\" width=\"33%\">");
    m_part->write(i18n("Difference"));
    m_part->write("</td></tr>");

    m_part->write(QString("<tr class=\"row-odd\">"));

    MyMoneyMoney totalBudgetValue = grid.m_total[eBudget].m_total;
    MyMoneyMoney totalActualValue = grid.m_total[eActual].m_total;
    MyMoneyMoney totalBudgetDiffValue = grid.m_total[eBudgetDiff].m_total;

    QString totalBudgetAmount = totalBudgetValue.formatMoney(file->baseCurrency().tradingSymbol(), prec);
    QString totalActualAmount = totalActualValue.formatMoney(file->baseCurrency().tradingSymbol(), prec);
    QString totalBudgetDiffAmount = totalBudgetDiffValue.formatMoney(file->baseCurrency().tradingSymbol(), prec);

    m_part->write(QString("<td align=\"right\">%1</td>").arg(showColoredAmount(totalBudgetAmount, totalBudgetValue.isNegative())));
    m_part->write(QString("<td align=\"right\">%1</td>").arg(showColoredAmount(totalActualAmount, totalActualValue.isNegative())));
    m_part->write(QString("<td align=\"right\">%1</td>").arg(showColoredAmount(totalBudgetDiffAmount, totalBudgetDiffValue.isNegative())));
    m_part->write("</tr>");
    m_part->write("</table>");

    //budget overrun
    m_part->write("<div class=\"gap\">&nbsp;</div>\n");
    m_part->write("<table width=\"75%\" cellspacing=\"0\" cellpadding=\"2\" class=\"summarytable\" >");
    m_part->write("<tr class=\"itemtitle\">");
    m_part->write("<td class=\"left\" colspan=\"4\">");
    m_part->write(i18n("Budget Overruns"));
    m_part->write("</td></tr>");
    m_part->write("<tr class=\"item\">");
    m_part->write("<td class=\"left\" width=\"30%\">");
    m_part->write(i18n("Account"));
    m_part->write("</td>");
    m_part->write("<td class=\"right\" width=\"20%\">");
    m_part->write(i18n("Budgeted"));
    m_part->write("</td>");
    m_part->write("<td class=\"right\" width=\"20%\">");
    m_part->write(i18n("Actual"));
    m_part->write("</td>");
    m_part->write("<td class=\"right\" width=\"20%\">");
    m_part->write(i18n("Difference"));
    m_part->write("</td></tr>");


    PivotGrid::iterator it_outergroup = grid.begin();
    while ( it_outergroup != grid.end() )
    {
      i = 0;
      PivotOuterGroup::iterator it_innergroup = (*it_outergroup).begin();
      while ( it_innergroup != (*it_outergroup).end() )
      {
        PivotInnerGroup::iterator it_row = (*it_innergroup).begin();
        while ( it_row != (*it_innergroup).end() )
        {
          //column number is 1 because the report includes only current month
          if(it_row.data()[eBudgetDiff][1].isNegative()) {
            //get report account to get the name later
            ReportAccount rowname = it_row.key();

            //write the outergroup if it is the first row of outergroup being shown
            if(i == 0) {
              m_part->write("<tr style=\"font-weight:bold;\">");
              m_part->write(QString("<td class=\"left\" colspan=\"4\">%1</td>").arg(KMyMoneyUtils::accountTypeToString( rowname.accountType())));
              m_part->write("</tr>");
            }
            m_part->write(QString("<tr class=\"row-%1\">").arg(i++ & 0x01 ? "even" : "odd"));

            //get values from grid
            MyMoneyMoney actualValue = it_row.data()[eActual][1];
            MyMoneyMoney budgetValue = it_row.data()[eBudget][1];
            MyMoneyMoney budgetDiffValue = it_row.data()[eBudgetDiff][1];

            //format amounts
            QString actualAmount = actualValue.formatMoney(file->baseCurrency().tradingSymbol(), prec);
            QString budgetAmount = budgetValue.formatMoney(file->baseCurrency().tradingSymbol(), prec);
            QString budgetDiffAmount = budgetDiffValue.formatMoney(file->baseCurrency().tradingSymbol(), prec);

            //account name
            m_part->write(QString("<td>") + link(VIEW_LEDGER, QString("?id=%1").arg(rowname.id())) + rowname.name() + linkend() + "</td>");

            //show amounts
            m_part->write(QString("<td align=\"right\">%1</td>").arg(showColoredAmount(budgetAmount, budgetValue.isNegative())));
            m_part->write(QString("<td align=\"right\">%1</td>").arg(showColoredAmount(actualAmount, actualValue.isNegative())));
            m_part->write(QString("<td align=\"right\">%1</td>").arg(showColoredAmount(budgetDiffAmount, budgetDiffValue.isNegative())));
            m_part->write("</tr>");
          }
          ++it_row;
        }
        ++it_innergroup;
      }
      ++it_outergroup;
    }

    //if no negative differences are found, then inform that
    if(i == 0) {
      m_part->write(QString("<tr class=\"row-%1\" style=\"font-weight:bold;\">").arg(i++ & 0x01 ? "even" : "odd"));
      m_part->write(QString("<td class=\"center\" colspan=\"4\">%1</td>").arg(i18n("No Budget Categories have been overrun")));
      m_part->write("</tr>");
    }
    m_part->write("</table></div></div>");
  }
#endif
}

QString KHomeView::showColoredAmount(const QString& amount, bool isNegative)
{
  if(isNegative) {
    //if negative, get the settings for negative numbers
    return QString("<font color=\"%1\">%2</font>").arg(KMyMoneyGlobalSettings::listNegativeValueColor().name(), amount);
  }

  //if positive, return the same string
  return amount;
}

void KHomeView::doForecast(void)
{
  //clear m_accountList because forecast is about to changed
  m_accountList.clear();

  //reinitialize the object
  m_forecast = MyMoneyForecast();

  //If forecastDays lower than accountsCycle, adjust to the first cycle
  if(m_forecast.accountsCycle() > m_forecast.forecastDays())
    m_forecast.setForecastDays(m_forecast.accountsCycle());

  //Get all accounts of the right type to calculate forecast
  m_forecast.doForecast();
}

MyMoneyMoney KHomeView::forecastPaymentBalance(const MyMoneyAccount& acc, const MyMoneyMoney& payment, QDate& paymentDate)
{
  //if paymentDate before or equal to currentDate set it to current date plus 1
  //so we get to accumulate forecast balance correctly
  if(paymentDate <= QDate::currentDate())
    paymentDate = QDate::currentDate().addDays(1);

  //check if the account is already there
  if(m_accountList.find(acc.id()) == m_accountList.end()
     || m_accountList[acc.id()].find(paymentDate) == m_accountList[acc.id()].end())
  {
    if(paymentDate == QDate::currentDate()) {
      m_accountList[acc.id()][paymentDate] = m_forecast.forecastBalance(acc, paymentDate);
    } else {
      m_accountList[acc.id()][paymentDate] = m_forecast.forecastBalance(acc, paymentDate.addDays(-1));
    }
  }
  m_accountList[acc.id()][paymentDate] = m_accountList[acc.id()][paymentDate] + payment;
  return m_accountList[acc.id()][paymentDate];
}

void KHomeView::showCashFlowSummary()
{
#warning "port to kde4"
#if 0
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
  if(transactions.size() > 0) {
    QList<MyMoneyTransaction>::const_iterator it_transaction;

    //get all transactions for this month
    for(it_transaction = transactions.begin(); it_transaction != transactions.end(); ++it_transaction ) {

      //get the splits for each transaction
      const QList<MyMoneySplit>& splits = (*it_transaction).splits();
      QList<MyMoneySplit>::const_iterator it_split;
      for(it_split = splits.begin(); it_split != splits.end(); ++it_split) {
        if(!(*it_split).shares().isZero()) {
          ReportAccount repSplitAcc = ReportAccount((*it_split).accountId());

          //only add if it is an income or expense
          if(repSplitAcc.isIncomeExpense()) {
            MyMoneyMoney value;

            //convert to base currency if necessary
            if(repSplitAcc.currencyId() != file->baseCurrency().id()) {
              MyMoneyMoney curPrice = repSplitAcc.baseCurrencyPrice((*it_transaction).postDate());
              value = ((*it_split).shares() * MyMoneyMoney(-1, 1)) * curPrice;
              value = value.convert(10000);
            } else {
              value = ((*it_split).shares() * MyMoneyMoney(-1, 1));
            }

            //store depending on account type
            if(repSplitAcc.accountType() == MyMoneyAccount::Income) {
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
  amountIncome.replace(" ","&nbsp;");
  amountExpense.replace(" ","&nbsp;");

  //calculate schedules

  //Add all schedules for this month
  MyMoneyMoney scheduledIncome;
  MyMoneyMoney scheduledExpense;
  MyMoneyMoney scheduledLiquidTransfer;
  MyMoneyMoney scheduledOtherTransfer;

  //get overdues and schedules until the end of this month
  QList<MyMoneySchedule> schedule = file->scheduleList("", MyMoneySchedule::TYPE_ANY,
      MyMoneySchedule::OCCUR_ANY,
      MyMoneySchedule::STYPE_ANY,
      QDate(),
            endOfMonth);

  //Remove the finished schedules
  QList<MyMoneySchedule>::Iterator finished_it;
  for (finished_it=schedule.begin(); finished_it!=schedule.end();) {
    if ((*finished_it).isFinished()) {
      finished_it = schedule.remove(finished_it);
      continue;
    }
    ++finished_it;
  }

  //add income and expenses
  QList<MyMoneySchedule>::Iterator sched_it;
  for (sched_it=schedule.begin(); sched_it!=schedule.end();) {
    QDate nextDate = (*sched_it).nextDueDate();
    int cnt = 0;

    while(nextDate.isValid() && nextDate <= endOfMonth) {
      ++cnt;
      nextDate = (*sched_it).nextPayment(nextDate);
        // for single occurence nextDate will not change, so we
        // better get out of here.
      if((*sched_it).occurence() == MyMoneySchedule::OCCUR_ONCE)
        break;
    }

    MyMoneyAccount acc = (*sched_it).account();
    if(acc.id()) {
      MyMoneyTransaction transaction = (*sched_it).transaction();
      // only show the entry, if it is still active

      MyMoneySplit sp = transaction.splitByAccount(acc.id(), true);

      // take care of the autoCalc stuff
      if((*sched_it).type() == MyMoneySchedule::TYPE_LOANPAYMENT) {
        QDate nextDate = (*sched_it).nextPayment((*sched_it).lastPayment());

        //make sure we have all 'starting balances' so that the autocalc works
        QList<MyMoneySplit>::const_iterator it_s;
        QMap<QString, MyMoneyMoney> balanceMap;

        for(it_s = transaction.splits().begin(); it_s != transaction.splits().end(); ++it_s ) {
          MyMoneyAccount acc = file->account((*it_s).accountId());
            // collect all overdues on the first day
            QDate schedDate = nextDate;
            if(QDate::currentDate() >= nextDate)
              schedDate = QDate::currentDate().addDays(1);

            balanceMap[acc.id()] += file->balance(acc.id());
        }
        KMyMoneyUtils::calculateAutoLoan(*sched_it, transaction, balanceMap);
      }

      //go through the splits and assign to liquid or other transfers
      const QList<MyMoneySplit> splits = transaction.splits();
      QList<MyMoneySplit>::const_iterator split_it;
      for (split_it = splits.begin(); split_it != splits.end(); ++split_it) {
        if( (*split_it).accountId() != acc.id() ) {
          ReportAccount repSplitAcc = ReportAccount((*split_it).accountId());

          //get the shares and multiply by the quantity of occurences in the period
          MyMoneyMoney value = (*split_it).shares() * cnt;

          //convert to foreign currency if needed
          if(repSplitAcc.currencyId() != file->baseCurrency().id()) {
            MyMoneyMoney curPrice = repSplitAcc.baseCurrencyPrice(QDate::currentDate());
            value = value * curPrice;
            value = value.convert(10000);
          }

          if(( repSplitAcc.isLiquidLiability()
             || repSplitAcc.isLiquidAsset() )
             && acc.accountGroup() != repSplitAcc.accountGroup()) {
            scheduledLiquidTransfer += value;
          } else if(repSplitAcc.isAssetLiability()
             && !repSplitAcc.isLiquidLiability()
             && !repSplitAcc.isLiquidAsset() ) {
            scheduledOtherTransfer += value;
          } else if(repSplitAcc.isIncomeExpense()) {
            //income and expenses are stored as negative values
            if(repSplitAcc.accountType() == MyMoneyAccount::Income)
              scheduledIncome -= value;
            if(repSplitAcc.accountType() == MyMoneyAccount::Expense)
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

  amountScheduledIncome.replace(" ","&nbsp;");
  amountScheduledExpense.replace(" ","&nbsp;");
  amountScheduledLiquidTransfer.replace(" ","&nbsp;");
  amountScheduledOtherTransfer.replace(" ","&nbsp;");

  //get liquid assets and liabilities
  QList<MyMoneyAccount> accounts;
  QList<MyMoneyAccount>::const_iterator account_it;
  MyMoneyMoney liquidAssets;
  MyMoneyMoney liquidLiabilities;

  // get list of all accounts
  file->accountList(accounts);
  for(account_it = accounts.begin(); account_it != accounts.end();) {
    if(!(*account_it).isClosed()) {
      switch((*account_it).accountType()) {
        //group all assets into one list
        case MyMoneyAccount::Checkings:
        case MyMoneyAccount::Savings:
        case MyMoneyAccount::Cash:
        {
          MyMoneyMoney value = MyMoneyFile::instance()->balance((*account_it).id(), QDate::currentDate());
          //calculate balance for foreign currency accounts
          if((*account_it).currencyId() != file->baseCurrency().id()) {
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
        case MyMoneyAccount::CreditCard:
        {
          MyMoneyMoney value;
          value = MyMoneyFile::instance()->balance((*account_it).id(), QDate::currentDate());
          //calculate balance if foreign currency
          if((*account_it).currencyId() != file->baseCurrency().id()) {
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
  MyMoneyMoney liquidWorth = liquidAssets+liquidLiabilities;

    //format assets, liabilities and net worth
  QString amountLiquidAssets = liquidAssets.formatMoney(file->baseCurrency().tradingSymbol(), prec);
  QString amountLiquidLiabilities = liquidLiabilities.formatMoney(file->baseCurrency().tradingSymbol(), prec);
  QString amountLiquidWorth = liquidWorth.formatMoney(file->baseCurrency().tradingSymbol(), prec);
  amountLiquidAssets.replace(" ","&nbsp;");
  amountLiquidLiabilities.replace(" ","&nbsp;");
  amountLiquidWorth.replace(" ","&nbsp;");

  //show the summary
  m_part->write("<div class=\"shadow\"><div class=\"displayblock\"><div class=\"summaryheader\">" + i18n("Cash Flow Summary") + "</div>\n<div class=\"gap\">&nbsp;</div>\n");

  //print header
  m_part->write("<table width=\"100%\" cellspacing=\"0\" cellpadding=\"2\" class=\"summarytable\" >");
  //income and expense title
  m_part->write("<tr class=\"itemtitle\">");
  m_part->write("<td class=\"left\" colspan=\"4\">");
  m_part->write(i18n("Income and Expenses of Current Month"));
  m_part->write("</td></tr>");
  //column titles
  m_part->write("<tr class=\"item\">");
  m_part->write("<td width=\"25%\" class=\"center\">");
  m_part->write(i18n("Income"));
  m_part->write("</td>");
  m_part->write("<td width=\"25%\" class=\"center\">");
  m_part->write(i18n("Scheduled Income"));
  m_part->write("</td>");
  m_part->write("<td width=\"25%\" class=\"center\">");
  m_part->write(i18n("Expenses"));
  m_part->write("</td>");
  m_part->write("<td width=\"25%\" class=\"center\">");
  m_part->write(i18n("Scheduled Expenses"));
  m_part->write("</td>");
  m_part->write("</tr>");

  //add row with banding
  m_part->write(QString("<tr class=\"row-even\" style=\"font-weight:bold;\">"));

  //print current income
  m_part->write(QString("<td align=\"right\">%2</td>").arg(showColoredAmount(amountIncome, incomeValue.isNegative())));

  //print the scheduled income
  m_part->write(QString("<td align=\"right\">%2</td>").arg(showColoredAmount(amountScheduledIncome, scheduledIncome.isNegative())));

  //print current expenses
  m_part->write(QString("<td align=\"right\">%2</td>").arg(showColoredAmount(amountExpense,  expenseValue.isNegative())));

  //print the scheduled expenses
  m_part->write(QString("<td align=\"right\">%2</td>").arg(showColoredAmount(amountScheduledExpense,  scheduledExpense.isNegative())));
  m_part->write("</tr>");

  m_part->write("</table>");

  //print header of assets and liabilities
  m_part->write("<div class=\"gap\">&nbsp;</div>\n");
  m_part->write("<table width=\"100%\" cellspacing=\"0\" cellpadding=\"2\" class=\"summarytable\" >");
  //assets and liabilities title
  m_part->write("<tr class=\"itemtitle\">");
  m_part->write("<td class=\"left\" colspan=\"4\">");
  m_part->write(i18n("Liquid Assets and Liabilities"));
  m_part->write("</td></tr>");
  //column titles
  m_part->write("<tr class=\"item\">");
  m_part->write("<td width=\"25%\" class=\"center\">");
  m_part->write(i18n("Liquid Assets"));
  m_part->write("</td>");
  m_part->write("<td width=\"25%\" class=\"center\">");
  m_part->write(i18n("Transfers to Liquid Liabilities"));
  m_part->write("</td>");
  m_part->write("<td width=\"25%\" class=\"center\">");
  m_part->write(i18n("Liquid Liabilities"));
  m_part->write("</td>");
  m_part->write("<td width=\"25%\" class=\"center\">");
  m_part->write(i18n("Other Transfers"));
  m_part->write("</td>");
  m_part->write("</tr>");

  //add row with banding
  m_part->write(QString("<tr class=\"row-even\" style=\"font-weight:bold;\">"));

  //print current liquid assets
  m_part->write(QString("<td align=\"right\">%2</td>").arg(showColoredAmount(amountLiquidAssets, liquidAssets.isNegative())));

  //print the scheduled transfers
  m_part->write(QString("<td align=\"right\">%2</td>").arg(showColoredAmount(amountScheduledLiquidTransfer, scheduledLiquidTransfer.isNegative())));

  //print current liabilities
  m_part->write(QString("<td align=\"right\">%2</td>").arg(showColoredAmount(amountLiquidLiabilities,  liquidLiabilities.isNegative())));

  //print the scheduled transfers
  m_part->write(QString("<td align=\"right\">%2</td>").arg(showColoredAmount(amountScheduledOtherTransfer, scheduledOtherTransfer.isNegative())));


  m_part->write("</tr>");

  m_part->write("</table>");

  //final conclusion
  MyMoneyMoney profitValue = incomeValue + expenseValue + scheduledIncome + scheduledExpense;
  MyMoneyMoney expectedAsset = liquidAssets + scheduledIncome + scheduledExpense + scheduledLiquidTransfer + scheduledOtherTransfer;
  MyMoneyMoney expectedLiabilities = liquidLiabilities + scheduledLiquidTransfer;

  QString amountExpectedAsset = expectedAsset.formatMoney(file->baseCurrency().tradingSymbol(), prec);
  QString amountExpectedLiabilities = expectedLiabilities.formatMoney(file->baseCurrency().tradingSymbol(), prec);
  QString amountProfit = profitValue.formatMoney(file->baseCurrency().tradingSymbol(), prec);
  amountProfit.replace(" ","&nbsp;");
  amountExpectedAsset.replace(" ","&nbsp;");
  amountExpectedLiabilities.replace(" ","&nbsp;");



  //print header of cash flow status
  m_part->write("<div class=\"gap\">&nbsp;</div>\n");
  m_part->write("<table width=\"100%\" cellspacing=\"0\" cellpadding=\"2\" class=\"summarytable\" >");
  //income and expense title
  m_part->write("<tr class=\"itemtitle\">");
  m_part->write("<td class=\"left\" colspan=\"4\">");
  m_part->write(i18n("Cash Flow Status"));
  m_part->write("</td></tr>");
  //column titles
  m_part->write("<tr class=\"item\">");
  m_part->write("<td>&nbsp;</td>");
  m_part->write("<td width=\"25%\" class=\"center\">");
  m_part->write(i18n("Expected Liquid Assets"));
  m_part->write("</td>");
  m_part->write("<td width=\"25%\" class=\"center\">");
  m_part->write(i18n("Expected Liquid Liabilities"));
  m_part->write("</td>");
  m_part->write("<td width=\"25%\" class=\"center\">");
  m_part->write(i18n("Expected Profit/Loss"));
  m_part->write("</td>");
  m_part->write("</tr>");

  //add row with banding
  m_part->write(QString("<tr class=\"row-even\" style=\"font-weight:bold;\">"));
  m_part->write("<td>&nbsp;</td>");

  //print expected assets
  m_part->write(QString("<td align=\"right\">%2</td>").arg(showColoredAmount(amountExpectedAsset, expectedAsset.isNegative())));

  //print expected liabilities
  m_part->write(QString("<td align=\"right\">%2</td>").arg(showColoredAmount(amountExpectedLiabilities, expectedLiabilities.isNegative())));

  //print expected profit
  m_part->write(QString("<td align=\"right\">%2</td>").arg(showColoredAmount(amountProfit, profitValue.isNegative())));

  m_part->write("</tr>");

  m_part->write("</table>");

  m_part->write("</div></div>");

#endif
}

// Make sure, that these definitions are only used within this file
// this does not seem to be necessary, but when building RPMs the
// build option 'final' is used and all CPP files are concatenated.
// So it could well be, that in another CPP file these definitions
// are also used.
#undef VIEW_LEDGER
#undef VIEW_SCHEDULE
#undef VIEW_WELCOME
#undef VIEW_HOME
#undef VIEW_REPORTS

#include "khomeview.moc"
