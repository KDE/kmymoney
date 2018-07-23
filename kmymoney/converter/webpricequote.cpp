/***************************************************************************
                          webpricequote.cpp
                             -------------------
    begin                : Thu Dec 30 2004
    copyright            : (C) 2004 by Ace Jones
    email                : Ace Jones <acejones@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "webpricequote.h"

// ----------------------------------------------------------------------------
// QT Headers

#include <QFile>
#include <QRegExp>
#include <QTextStream>
#include <QTextCodec>
#include <QByteArray>

// ----------------------------------------------------------------------------
// KDE Headers
#include <kio/netaccess.h>
#include <kio/scheduler.h>
#include <klocale.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kcalendarsystem.h>
#include <ktemporaryfile.h>
#include <kshell.h>
#include <KConfigGroup>
#include <kprocess.h>
#include <kencodingprober.h>

// ----------------------------------------------------------------------------
// Project Headers

#include "mymoneyexception.h"
#include "mymoneyqifprofile.h"

// define static members
QString WebPriceQuote::m_financeQuoteScriptPath;
QStringList WebPriceQuote::m_financeQuoteSources;

class WebPriceQuote::Private
{
public:
  WebPriceQuoteProcess m_filter;
  QString m_quoteData;
  QString m_symbol;
  QString m_id;
  QDate m_date;
  double m_price;
  WebPriceQuoteSource m_source;

  static int dbgArea() {
    static int s_area = KDebug::registerArea("KMyMoney (WebPriceQuote)");
    return s_area;
  }
};

WebPriceQuote::WebPriceQuote(QObject* _parent):
    QObject(_parent),
    d(new Private)
{
  // only do this once (I know, it is not thread safe, but it should
  // always yield the same result so we don't do any semaphore foo here)
  if (m_financeQuoteScriptPath.isEmpty()) {
    m_financeQuoteScriptPath = KGlobal::dirs()->findResource("appdata",
                               QString("misc/financequote.pl"));
  }
  connect(&d->m_filter, SIGNAL(processExited(QString)), this, SLOT(slotParseQuote(QString)));
}

WebPriceQuote::~WebPriceQuote()
{
  delete d;
}

bool WebPriceQuote::launch(const QString& _symbol, const QString& _id, const QString& _sourcename)
{
  if (_sourcename.contains("Finance::Quote"))
    return (launchFinanceQuote(_symbol, _id, _sourcename));
  else
    return (launchNative(_symbol, _id, _sourcename));
}

bool WebPriceQuote::launchNative(const QString& _symbol, const QString& _id, const QString& _sourcename)
{
  bool result = true;
  d->m_symbol = _symbol;
  d->m_id = _id;

//   emit status(QString("(Debug) symbol=%1 id=%2...").arg(_symbol,_id));

  // Get sources from the config file
  QString sourcename = _sourcename;
  if (sourcename.isEmpty())
    sourcename = "KMyMoney Currency";

  if (quoteSources().contains(sourcename))
    d->m_source = WebPriceQuoteSource(sourcename);
  else
    emit error(i18n("Source <placeholder>%1</placeholder> does not exist.", sourcename));

  KUrl url;

  // if the source has room for TWO symbols..
  if (d->m_source.m_url.contains("%2")) {
    // this is a two-symbol quote.  split the symbol into two.  valid symbol
    // characters are: 0-9, A-Z and the dot.  anything else is a separator
    QRegExp splitrx("([0-9a-z\\.]+)[^a-z0-9]+([0-9a-z\\.]+)", Qt::CaseInsensitive);
    // if we've truly found 2 symbols delimited this way...
    if (splitrx.indexIn(d->m_symbol) != -1)
      url = KUrl(d->m_source.m_url.arg(splitrx.cap(1), splitrx.cap(2)));
    else
      kDebug(Private::dbgArea()) << "WebPriceQuote::launch() did not find 2 symbols";
  } else
    // a regular one-symbol quote
    url = KUrl(d->m_source.m_url.arg(d->m_symbol));

  if (url.isLocalFile()) {
    emit status(i18nc("The process x is executing", "Executing %1...", url.toLocalFile()));

    d->m_filter.clearProgram();
    d->m_filter << url.toLocalFile().split(' ', QString::SkipEmptyParts);
    d->m_filter.setSymbol(d->m_symbol);

    d->m_filter.setOutputChannelMode(KProcess::MergedChannels);
    d->m_filter.start();

    if (d->m_filter.waitForStarted()) {
      result = true;
    } else {
      emit error(i18n("Unable to launch: %1", url.toLocalFile()));
      slotParseQuote(QString());
    }
  } else {
    emit status(i18n("Fetching URL %1...", url.prettyUrl()));

    QString tmpFile;
    if (KIO::NetAccess::download(url, tmpFile, 0)) {
      // kDebug(Private::dbgArea()) << "Downloaded " << tmpFile;
      kDebug(Private::dbgArea()) << "Downloaded" << tmpFile << "from" << url;
      QFile f(tmpFile);
      if (f.open(QIODevice::ReadOnly)) {
        result = true;
        // Find out the page encoding and convert it to unicode
        QByteArray page = f.readAll();
        KEncodingProber prober(KEncodingProber::Universal);
        prober.feed(page);
        QTextCodec* codec = QTextCodec::codecForName(prober.encoding());
        if (!codec)
          codec = QTextCodec::codecForLocale();
        QString quote = codec->toUnicode(page);
        f.close();
        slotParseQuote(quote);
      } else {
        emit error(i18n("Failed to open downloaded file"));
        slotParseQuote(QString());
      }
      KIO::NetAccess::removeTempFile(tmpFile);
    } else {
      emit error(KIO::NetAccess::lastErrorString());
      slotParseQuote(QString());
    }
  }
  return result;
}

bool WebPriceQuote::launchFinanceQuote(const QString& _symbol, const QString& _id,
                                       const QString& _sourcename)
{
  bool result = true;
  d->m_symbol = _symbol;
  d->m_id = _id;
  QString FQSource = _sourcename.section(' ', 1);
  d->m_source = WebPriceQuoteSource(_sourcename, m_financeQuoteScriptPath,
                                    "\"([^,\"]*)\",.*",  // symbol regexp
                                    "[^,]*,[^,]*,\"([^\"]*)\"", // price regexp
                                    "[^,]*,([^,]*),.*", // date regexp
                                    "%y-%m-%d"); // date format

  //emit status(QString("(Debug) symbol=%1 id=%2...").arg(_symbol,_id));

  d->m_filter.clearProgram();
  d->m_filter << "perl" << m_financeQuoteScriptPath << FQSource << KShell::quoteArg(_symbol);
  d->m_filter.setSymbol(d->m_symbol);
  emit status(i18nc("Executing 'script' 'online source' 'investment symbol' ", "Executing %1 %2 %3...", m_financeQuoteScriptPath, FQSource, _symbol));

  d->m_filter.setOutputChannelMode(KProcess::MergedChannels);
  d->m_filter.start();

  // This seems to work best if we just block until done.
  if (d->m_filter.waitForFinished()) {
    result = true;
  } else {
    emit error(i18n("Unable to launch: %1", m_financeQuoteScriptPath));
    slotParseQuote(QString());
  }

  return result;
}

void WebPriceQuote::slotParseQuote(const QString& _quotedata)
{
  QString quotedata = _quotedata;
  d->m_quoteData = quotedata;
  bool gotprice = false;
  bool gotdate = false;

  kDebug(Private::dbgArea()) << "quotedata" << _quotedata;

  if (! quotedata.isEmpty()) {
    if (!d->m_source.m_skipStripping) {
      //
      // First, remove extranous non-data elements
      //

      // HTML tags
      quotedata.remove(QRegExp("<[^>]*>"));

      // &...;'s
      quotedata.replace(QRegExp("&\\w+;"), " ");

      // Extra white space
      quotedata = quotedata.simplified();
      kDebug(Private::dbgArea()) << "stripped text" << quotedata;
    }

    QRegExp symbolRegExp(d->m_source.m_sym);
    QRegExp dateRegExp(d->m_source.m_date);
    QRegExp priceRegExp(d->m_source.m_price);

    if (symbolRegExp.indexIn(quotedata) > -1) {
      kDebug(Private::dbgArea()) << "Symbol" << symbolRegExp.cap(1);
      emit status(i18n("Symbol found: '%1'", symbolRegExp.cap(1)));
    }

    if (priceRegExp.indexIn(quotedata) > -1) {
      gotprice = true;

      // Deal with european quotes that come back as X.XXX,XX or XX,XXX
      //
      // We will make the assumption that ALL prices have a decimal separator.
      // So "1,000" always means 1.0, not 1000.0.
      //
      // Remove all non-digits from the price string except the last one, and
      // set the last one to a period.
      QString pricestr = priceRegExp.cap(1);

      int pos = pricestr.lastIndexOf(QRegExp("\\D"));
      if (pos > 0) {
        pricestr[pos] = '.';
        pos = pricestr.lastIndexOf(QRegExp("\\D"), pos - 1);
      }
      while (pos > 0) {
        pricestr.remove(pos, 1);
        pos = pricestr.lastIndexOf(QRegExp("\\D"), pos);
      }

      d->m_price = pricestr.toDouble();
      kDebug(Private::dbgArea()) << "Price" << pricestr;
      emit status(i18n("Price found: '%1' (%2)", pricestr, d->m_price));
    }

    if (dateRegExp.indexIn(quotedata) > -1) {
      QString datestr = dateRegExp.cap(1);

      MyMoneyDateFormat dateparse(d->m_source.m_dateformat);
      try {
        d->m_date = dateparse.convertString(datestr, false /*strict*/);
        gotdate = true;
        kDebug(Private::dbgArea()) << "Date" << datestr;
        emit status(i18n("Date found: '%1'", d->m_date.toString()));;
      } catch (const MyMoneyException &) {
        // emit error(i18n("Unable to parse date %1 using format %2: %3").arg(datestr,dateparse.format(),e.what()));
        d->m_date = QDate::currentDate();
        gotdate = true;
      }
    }

    if (gotprice && gotdate) {
      emit quote(d->m_id, d->m_symbol, d->m_date, d->m_price);
    } else {
      emit error(i18n("Unable to update price for %1 (no price or no date)", d->m_symbol));
      emit failed(d->m_id, d->m_symbol);
    }
  } else {
    emit error(i18n("Unable to update price for %1 (empty quote data)", d->m_symbol));
    emit failed(d->m_id, d->m_symbol);
  }
}

const QMap<QString, WebPriceQuoteSource> WebPriceQuote::defaultQuoteSources()
{
  QMap<QString, WebPriceQuoteSource> result;

  // Use fx-rate.net as the standard currency exchange rate source until
  // we have the capability to use more than one source. Use a neutral
  // name for the source.
  result["KMyMoney Currency"] = WebPriceQuoteSource("KMyMoney Currency",
                                          "https://fx-rate.net/%1/%2",
                                           QString(),  // symbolregexp
                                          "1[ a-zA-Z]+=</span><br */?> *(\\d+|\\d+\\.\\d+)",
                                          "updated\\s\\d+:\\d+:\\d+\\(\\w+\\)\\s+(\\d{1,2}/\\d{2}/\\d{4})",
                                          "%d/%m/%y",
                                          true // skip HTML stripping
                                          );

  result["Globe & Mail"] = WebPriceQuoteSource("Globe & Mail",
                           "http://globefunddb.theglobeandmail.com/gishome/plsql/gis.price_history?pi_fund_id=%1",
                           QString(),  // symbolregexp
                           "Reinvestment Price \\w+ \\d+, \\d+ (\\d+\\.\\d+)", // priceregexp
                           "Reinvestment Price (\\w+ \\d+, \\d+)", // dateregexp
                           "%m %d %y" // dateformat
                                              );

  result["MSN.CA"] = WebPriceQuoteSource("MSN.CA",
                                         "http://ca.moneycentral.msn.com/investor/quotes/quotes.asp?symbol=%1",
                                         QString(),  // symbolregexp
                                         "(\\d+\\.\\d+) [+-]\\d+.\\d+", // priceregexp
                                         "Time of last trade (\\d+/\\d+/\\d+)", //dateregexp
                                         "%d %m %y" // dateformat
                                        );
  // Finanztreff (replaces VWD.DE) and boerseonline supplied by Micahel Zimmerman
  result["Finanztreff"] = WebPriceQuoteSource("Finanztreff",
                          "http://finanztreff.de/kurse_einzelkurs_detail.htn?u=100&i=%1",
                          QString(),  // symbolregexp
                          "([0-9]+,\\d+).+Gattung:Fonds", // priceregexp
                          "\\).(\\d+\\D+\\d+\\D+\\d+)", // dateregexp (doesn't work; date in chart
                          "%d.%m.%y" // dateformat
                                             );

  result["boerseonline"] = WebPriceQuoteSource("boerseonline",
                           "http://www.boerse-online.de/tools/boerse/einzelkurs_kurse.htm?&s=%1",
                           QString(),  // symbolregexp
                           "Akt\\. Kurs.([\\d\\.]+,\\d\\d)", // priceregexp
                           "Datum.(\\d+\\.\\d+\\.\\d+)", // dateregexp (doesn't work; date in chart
                           "%d.%m.%y" // dateformat
                                              );

  // The following two price sources were contributed by
  // Marc Zahnlecker <tf2k@users.sourceforge.net>

  result["Wallstreet-Online.DE (Default)"] = WebPriceQuoteSource("Wallstreet-Online.DE (Default)",
      "http://www.wallstreet-online.de/si/?k=%1&spid=ws",
      "Symbol:(\\w+)",  // symbolregexp
      "Letzter Kurs: ([0-9.]+,\\d+)", // priceregexp
      ", (\\d+\\D+\\d+\\D+\\d+)", // dateregexp
      "%d %m %y" // dateformat
                                                                );
  // This quote source provided by e-mail and should replace the previous one:
  // From: David Houlden <djhoulden@gmail.com>
  // To: kmymoney@kde.org
  // Date: Sat, 6 Apr 2013 13:22:45 +0100
  result["Financial Times UK Funds"] = WebPriceQuoteSource("Financial Times UK Funds",
                                       "http://funds.ft.com/uk/Tearsheet/Summary?s=%1",
                                       "data-display-symbol=\"(.*):", // symbol regexp
                                       "class=\"text first\">([\\d,]*\\d+\\.\\d+)</td>", // price regexp
                                       "As of market close\\ (.*)\\.", // date regexp
                                       "%m %d %y", // date format
                                       true // skip HTML stripping
                                                          );

  // (tf2k) The "mpid" is I think the market place id. In this case five
  // stands for Hamburg.
  //
  // Here the id for several market places: 2 Frankfurt, 3 Berlin, 4
  // Düsseldorf, 5 Hamburg, 6 München/Munich, 7 Hannover, 9 Stuttgart, 10
  // Xetra, 32 NASDAQ, 36 NYSE

  result["Wallstreet-Online.DE (Hamburg)"] = WebPriceQuoteSource("Wallstreet-Online.DE (Hamburg)",
      "http://fonds.wallstreet-online.de/si/?k=%1&spid=ws&mpid=5",
      "Symbol:(\\w+)",  // symbolregexp
      "Fonds \\(EUR\\) ([0-9.]+,\\d+)", // priceregexp
      ", (\\d+\\D+\\d+\\D+\\d+)", // dateregexp
      "%d %m %y" // dateformat
                                                                );

  // The following price quote was contributed by
  // Piotr Adacha <piotr.adacha@googlemail.com>

  // I would like to post new Online Query Settings for KMyMoney. This set is
  // suitable to query stooq.com service, providing quotes for stocks, futures,
  // mutual funds and other financial instruments from Polish Gielda Papierow
  // Wartosciowych (GPW). Unfortunately, none of well-known international
  // services provide quotes for this market (biggest one in central and eastern
  // Europe), thus, I think it could be helpful for Polish users of KMyMoney (and
  // I am one of them for almost a year).

  result["Gielda Papierow Wartosciowych (GPW)"] = WebPriceQuoteSource("Gielda Papierow Wartosciowych (GPW)",
      "http://stooq.com/q/?s=%1",
      QString(),                   // symbol regexp
      "Last.*(\\d+\\.\\d+).*Date",    // price regexp
      "(\\d{4,4}-\\d{2,2}-\\d{2,2})", // date regexp
      "%y %m %d"                   // date format
                                                                     );

  // The following price quote is for getting prices of different funds
  // at OMX Baltic market.
  result["OMX Baltic funds"] = WebPriceQuoteSource("OMX Baltic funds",
                               "http://www.baltic.omxgroup.com/market/?pg=nontradeddetails&currency=0&instrument=%1",
                               QString(),  // symbolregexp
                               "NAV (\\d+,\\d+)",  // priceregexp
                               "Kpv (\\d+.\\d+.\\d+)",  // dateregexp
                               "%d.%m.%y"   // dateformat
                                                  );

  // The following price quote was contributed by
  // Peter Hargreaves <pete.h@pdh-online.info>
  // The original posting can be found here:
  // http://sourceforge.net/mailarchive/message.php?msg_name=200806060854.11682.pete.h%40pdh-online.info

  // I have PEP and ISA accounts which I invest in Funds with Barclays
  // Stockbrokers. They give me Fund data via Financial Express:
  //
  // https://webfund6.financialexpress.net/Clients/Barclays/default.aspx
  //
  // A typical Fund Factsheet is:
  //
  // https://webfund6.financialexpress.net/Clients/Barclays/search_factsheet_summary.aspx?code=0585239
  //
  // On the Factsheet to identify the fund you can see ISIN Code GB0005852396.
  // In the url, this code is shortened by loosing the first four and last
  // characters.
  //
  // Update:
  //
  // Nick Elliot has contributed a modified regular expression to cope with values presented
  // in pounds as well as those presented in pence. The source can be found here:
  // http://forum.kde.org/update-stock-and-currency-prices-t-32049.html

  result["Financial Express"] = WebPriceQuoteSource("Financial Express",
                                "https://webfund6.financialexpress.net/Clients/Barclays/search_factsheet_summary.aspx?code=%1",
                                "ISIN Code[^G]*(GB..........).*",  // symbolregexp
                                "Current Market Information[^0-9]*([0-9,\\.]+).*", // priceregexp
                                "Price Date[^0-9]*(../../....).*", // dateregexp
                                "%d/%m/%y"                         // dateformat
                                                   );


  // I suggest to include www.cash.ch as one of the pre-populated online
  // quote sources.

  // Rationale
  // It features Swiss funds that are otherwise hard to find. A typical
  // example: Swiss private pension accounts (in German termed 'Säule 3a')
  // may usually only invest in dedicated funds that are otherwise (almost)
  // not traded; the UBS Vitainvest series
  // (http://www.ubs.com/1/e/ubs_ch/private/insurance/fisca/securities/part_wealth.html)
  //  is such a series of funds.

  result["Cash CH"] = WebPriceQuoteSource("Cash CH",
                                          "http://www.cash.ch/boerse/fonds/fondsguide/kursinfo/fullquote/%1",
                                          "",  // symbolregexp
                                          "<span class=\"fgdhLast\">([1-9][0-9]*\\.[0-9][0-9])</span>", // priceregexp
                                          "<span class=\"fgdhLastdt\">([0-3][0-9]\\.[0-1][0-9]\\.[1-2][0-9][0-9][0-9])</span>", // dateregexp
                                          "%d.%m.%y",                         // dateformat
                                          true                                // skip stripping
                                         );

  return result;
}

const QStringList WebPriceQuote::quoteSources(const _quoteSystemE _system)
{
  if (_system == Native)
    return (quoteSourcesNative());
  else
    return (quoteSourcesFinanceQuote());
}

const QStringList WebPriceQuote::quoteSourcesNative()
{
  KSharedConfigPtr kconfig = KGlobal::config();
  QStringList groups = kconfig->groupList();

  QStringList::Iterator it;
  QRegExp onlineQuoteSource(QString("^Online-Quote-Source-(.*)$"));

  // get rid of all 'non online quote source' entries
  for (it = groups.begin(); it != groups.end(); it = groups.erase(it)) {
    if (onlineQuoteSource.indexIn(*it) >= 0) {
      // Insert the name part
      it = groups.insert(it, onlineQuoteSource.cap(1));
      ++it;
    }
  }

  // if the user has the OLD quote source defined, now is the
  // time to remove that entry and convert it to the new system.
  if (! groups.count() && kconfig->hasGroup("Online Quotes Options")) {
    KConfigGroup grp = kconfig->group("Online Quotes Options");
    QString url(grp.readEntry("URL", "http://finance.yahoo.com/d/quotes.csv?s=%1&f=sl1d1"));
    QString symbolRegExp(grp.readEntry("SymbolRegex", "\"([^,\"]*)\",.*"));
    QString priceRegExp(grp.readEntry("PriceRegex", "[^,]*,([^,]*),.*"));
    QString dateRegExp(grp.readEntry("DateRegex", "[^,]*,[^,]*,\"([^\"]*)\""));
    kconfig->deleteGroup("Online Quotes Options");

    groups += "Old Source";
    grp = kconfig->group(QString("Online-Quote-Source-%1").arg("Old Source"));
    grp.writeEntry("URL", url);
    grp.writeEntry("SymbolRegex", symbolRegExp);
    grp.writeEntry("PriceRegex", priceRegExp);
    grp.writeEntry("DateRegex", dateRegExp);
    grp.writeEntry("DateFormatRegex", "%m %d %y");
    grp.sync();
  }

  // Set up each of the default sources.  These are done piecemeal so that
  // when we add a new source, it's automatically picked up. And any changes
  // are also picked up.
  QMap<QString, WebPriceQuoteSource> defaults = defaultQuoteSources();
  QMap<QString, WebPriceQuoteSource>::const_iterator it_source = defaults.constBegin();
  while (it_source != defaults.constEnd()) {
    if (! groups.contains((*it_source).m_name)) {
      groups += (*it_source).m_name;
      (*it_source).write();
      kconfig->sync();
    }
    ++it_source;
  }

  return groups;
}

const QStringList WebPriceQuote::quoteSourcesFinanceQuote()
{
  if (m_financeQuoteSources.empty()) { // run the process one time only
    // since this is a static function it can be called without constructing an object
    // so we need to make sure that m_financeQuoteScriptPath is properly initialized
    if (m_financeQuoteScriptPath.isEmpty()) {
      m_financeQuoteScriptPath = KGlobal::dirs()->findResource("appdata",
                                 QString("misc/financequote.pl"));
    }
    FinanceQuoteProcess getList;
    getList.launch(m_financeQuoteScriptPath);
    while (!getList.isFinished()) {
      kapp->processEvents();
    }
    m_financeQuoteSources = getList.getSourceList();
  }
  return (m_financeQuoteSources);
}

//
// Helper class to load/save an individual source
//

WebPriceQuoteSource::WebPriceQuoteSource(const QString& name, const QString& url, const QString& sym, const QString& price, const QString& date, const QString& dateformat, bool skipStripping):
    m_name(name),
    m_url(url),
    m_sym(sym),
    m_price(price),
    m_date(date),
    m_dateformat(dateformat),
    m_skipStripping(skipStripping)
{ }

WebPriceQuoteSource::WebPriceQuoteSource(const QString& name)
{
  m_name = name;
  KSharedConfigPtr kconfig = KGlobal::config();
  KConfigGroup grp = kconfig->group(QString("Online-Quote-Source-%1").arg(m_name));
  m_sym = grp.readEntry("SymbolRegex");
  m_date = grp.readEntry("DateRegex");
  m_dateformat = grp.readEntry("DateFormatRegex", "%m %d %y");
  m_price = grp.readEntry("PriceRegex");
  m_url = grp.readEntry("URL");
  m_skipStripping = grp.readEntry("SkipStripping", false);
}

void WebPriceQuoteSource::write() const
{
  KSharedConfigPtr kconfig = KGlobal::config();
  KConfigGroup grp = kconfig->group(QString("Online-Quote-Source-%1").arg(m_name));
  grp.writeEntry("URL", m_url);
  grp.writeEntry("PriceRegex", m_price);
  grp.writeEntry("DateRegex", m_date);
  grp.writeEntry("DateFormatRegex", m_dateformat);
  grp.writeEntry("SymbolRegex", m_sym);
  if (m_skipStripping)
    grp.writeEntry("SkipStripping", m_skipStripping);
  else
    grp.deleteEntry("SkipStripping");
}

void WebPriceQuoteSource::rename(const QString& name)
{
  remove();
  m_name = name;
  write();
}

void WebPriceQuoteSource::remove() const
{
  KSharedConfigPtr kconfig = KGlobal::config();
  kconfig->deleteGroup(QString("Online-Quote-Source-%1").arg(m_name));
}

//
// Helper class to babysit the KProcess used for running the local script in that case
//

WebPriceQuoteProcess::WebPriceQuoteProcess()
{
  connect(this, SIGNAL(readyReadStandardOutput()), this, SLOT(slotReceivedDataFromFilter()));
  connect(this, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotProcessExited(int,QProcess::ExitStatus)));
}

void WebPriceQuoteProcess::slotReceivedDataFromFilter()
{
//   kDebug(2) << "WebPriceQuoteProcess::slotReceivedDataFromFilter(): " << QString(data);
  m_string += QString(readAllStandardOutput());
}

void WebPriceQuoteProcess::slotProcessExited(int /*exitCode*/, QProcess::ExitStatus /*exitStatus*/)
{
//   kDebug(2) << "WebPriceQuoteProcess::slotProcessExited()";
  emit processExited(m_string);
  m_string.truncate(0);
}

//
// Helper class to babysit the KProcess used for running the Finance Quote sources script
//

FinanceQuoteProcess::FinanceQuoteProcess()
{
  m_isDone = false;
  m_string = "";
  m_fqNames["aex"] = "AEX";
  m_fqNames["aex_futures"] = "AEX Futures";
  m_fqNames["aex_options"] = "AEX Options";
  m_fqNames["amfiindia"] = "AMFI India";
  m_fqNames["asegr"] = "ASE";
  m_fqNames["asia"] = "Asia (Yahoo, ...)";
  m_fqNames["asx"] = "ASX";
  m_fqNames["australia"] = "Australia (ASX, Yahoo, ...)";
  m_fqNames["bmonesbittburns"] = "BMO NesbittBurns";
  m_fqNames["brasil"] = "Brasil (Yahoo, ...)";
  m_fqNames["canada"] = "Canada (Yahoo, ...)";
  m_fqNames["canadamutual"] = "Canada Mutual (Fund Library, ...)";
  m_fqNames["deka"] = "Deka Investments";
  m_fqNames["dutch"] = "Dutch (AEX, ...)";
  m_fqNames["dwsfunds"] = "DWS";
  m_fqNames["europe"] = "Europe (Yahoo, ...)";
  m_fqNames["fidelity"] = "Fidelity (Fidelity, ...)";
  m_fqNames["fidelity_direct"] = "Fidelity Direct";
  m_fqNames["financecanada"] = "Finance Canada";
  m_fqNames["ftportfolios"] = "First Trust (First Trust, ...)";
  m_fqNames["ftportfolios_direct"] = "First Trust Portfolios";
  m_fqNames["fundlibrary"] = "Fund Library";
  m_fqNames["greece"] = "Greece (ASE, ...)";
  m_fqNames["indiamutual"] = "India Mutual (AMFI, ...)";
  m_fqNames["maninv"] = "Man Investments";
  m_fqNames["fool"] = "Motley Fool";
  m_fqNames["nasdaq"] = "Nasdaq (Yahoo, ...)";
  m_fqNames["nz"] = "New Zealand (Yahoo, ...)";
  m_fqNames["nyse"] = "NYSE (Yahoo, ...)";
  m_fqNames["nzx"] = "NZX";
  m_fqNames["platinum"] = "Platinum Asset Management";
  m_fqNames["seb_funds"] = "SEB";
  m_fqNames["sharenet"] = "Sharenet";
  m_fqNames["za"] = "South Africa (Sharenet, ...)";
  m_fqNames["troweprice_direct"] = "T. Rowe Price";
  m_fqNames["troweprice"] = "T. Rowe Price";
  m_fqNames["tdefunds"] = "TD Efunds";
  m_fqNames["tdwaterhouse"] = "TD Waterhouse Canada";
  m_fqNames["tiaacref"] = "TIAA-CREF";
  m_fqNames["trustnet"] = "Trustnet";
  m_fqNames["uk_unit_trusts"] = "U.K. Unit Trusts";
  m_fqNames["unionfunds"] = "Union Investments";
  m_fqNames["tsp"] = "US Govt. Thrift Savings Plan";
  m_fqNames["usfedbonds"] = "US Treasury Bonds";
  m_fqNames["usa"] = "USA (Yahoo, Fool ...)";
  m_fqNames["vanguard"] = "Vanguard";
  m_fqNames["vwd"] = "VWD";
  m_fqNames["yahoo"] = "Yahoo";
  m_fqNames["yahoo_asia"] = "Yahoo Asia";
  m_fqNames["yahoo_australia"] = "Yahoo Australia";
  m_fqNames["yahoo_brasil"] = "Yahoo Brasil";
  m_fqNames["yahoo_europe"] = "Yahoo Europe";
  m_fqNames["yahoo_nz"] = "Yahoo New Zealand";
  m_fqNames["zifunds"] = "Zuerich Investments";
  connect(this, SIGNAL(readyReadStandardOutput()), this, SLOT(slotReceivedDataFromFilter()));
  connect(this, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotProcessExited()));
  connect(this, SIGNAL(error(QProcess::ProcessError)), this, SLOT(slotProcessExited()));
}

void FinanceQuoteProcess::slotReceivedDataFromFilter()
{
  QByteArray data(readAllStandardOutput());

//   kDebug(2) << "WebPriceQuoteProcess::slotReceivedDataFromFilter(): " << QString(data);
  m_string += QString(data);
}

void FinanceQuoteProcess::slotProcessExited()
{
//   kDebug(2) << "WebPriceQuoteProcess::slotProcessExited()";
  m_isDone = true;
}

void FinanceQuoteProcess::launch(const QString& scriptPath)
{
  clearProgram();

  *this << "perl" << scriptPath << "-l";
  setOutputChannelMode(KProcess::OnlyStdoutChannel);
  start();
  if (! waitForStarted()) qWarning("Unable to start FQ script");
  return;
}

const QStringList FinanceQuoteProcess::getSourceList() const
{
  QStringList raw = m_string.split(0x0A, QString::SkipEmptyParts);
  QStringList sources;
  QStringList::iterator it;
  for (it = raw.begin(); it != raw.end(); ++it) {
    if (m_fqNames[*it].isEmpty()) sources.append(*it);
    else sources.append(m_fqNames[*it]);
  }
  sources.sort();
  return (sources);
}

const QString FinanceQuoteProcess::crypticName(const QString& niceName) const
{
  QString ret(niceName);
  fqNameMap::const_iterator it;
  for (it = m_fqNames.begin(); it != m_fqNames.end(); ++it) {
    if (niceName == it.value()) {
      ret = it.key();
      break;
    }
  }
  return (ret);
}

const QString FinanceQuoteProcess::niceName(const QString& crypticName) const
{
  QString ret(m_fqNames[crypticName]);
  if (ret.isEmpty()) ret = crypticName;
  return (ret);
}
//
// Universal date converter
//

// In 'strict' mode, this is designed to be compatable with the QIF profile date
// converter.  However, that converter deals with the concept of an apostrophe
// format in a way I don't understand.  So for the moment, they are 99%
// compatable, waiting on that issue. (acejones)

const QDate MyMoneyDateFormat::convertString(const QString& _in, bool _strict, unsigned _centurymidpoint) const
{
  //
  // Break date format string into component parts
  //

  QRegExp formatrex("%([mdy]+)(\\W+)%([mdy]+)(\\W+)%([mdy]+)", Qt::CaseInsensitive);
  if (formatrex.indexIn(m_format) == -1) {
    throw MYMONEYEXCEPTION("Invalid format string");
  }

  QStringList formatParts;
  formatParts += formatrex.cap(1);
  formatParts += formatrex.cap(3);
  formatParts += formatrex.cap(5);

  QStringList formatDelimiters;
  formatDelimiters += formatrex.cap(2);
  formatDelimiters += formatrex.cap(4);

  //
  // Break input string up into component parts,
  // using the delimiters found in the format string
  //

  QRegExp inputrex;
  inputrex.setCaseSensitivity(Qt::CaseInsensitive);

  // strict mode means we must enforce the delimiters as specified in the
  // format.  non-strict allows any delimiters
  if (_strict)
    inputrex.setPattern(QString("(\\w+)%1(\\w+)%2(\\w+)").arg(formatDelimiters[0], formatDelimiters[1]));
  else
    inputrex.setPattern("(\\w+)\\W+(\\w+)\\W+(\\w+)");

  if (inputrex.indexIn(_in) == -1) {
    throw MYMONEYEXCEPTION("Invalid input string");
  }

  QStringList scannedParts;
  scannedParts += inputrex.cap(1).toLower();
  scannedParts += inputrex.cap(2).toLower();
  scannedParts += inputrex.cap(3).toLower();

  //
  // Convert the scanned parts into actual date components
  //
  unsigned day = 0, month = 0, year = 0;
  bool ok;
  QRegExp digitrex("(\\d+)");
  QStringList::const_iterator it_scanned = scannedParts.constBegin();
  QStringList::const_iterator it_format = formatParts.constBegin();
  while (it_scanned != scannedParts.constEnd()) {
    // decide upon the first character of the part
    switch ((*it_format).at(0).cell()) {
      case 'd':
        // remove any extraneous non-digits (e.g. read "3rd" as 3)
        ok = false;
        if (digitrex.indexIn(*it_scanned) != -1)
          day = digitrex.cap(1).toUInt(&ok);
        if (!ok || day > 31)
          throw MYMONEYEXCEPTION(QString("Invalid day entry: %1").arg(*it_scanned));
        break;
      case 'm':
        month = (*it_scanned).toUInt(&ok);
        if (!ok) {
          // maybe it's a textual date
          unsigned i = 1;
          while (i <= 12) {
            if (KGlobal::locale()->calendar()->monthName(i, 2000).toLower() == *it_scanned
                || KGlobal::locale()->calendar()->monthName(i, 2000, KCalendarSystem::ShortName).toLower() == *it_scanned)
              month = i;
            ++i;
          }
        }

        if (month < 1 || month > 12)
          throw MYMONEYEXCEPTION(QString("Invalid month entry: %1").arg(*it_scanned));

        break;
      case 'y':
        if (_strict && (*it_scanned).length() != (*it_format).length())
          throw MYMONEYEXCEPTION(QString("Length of year (%1) does not match expected length (%2).")
                                 .arg(*it_scanned, *it_format));

        year = (*it_scanned).toUInt(&ok);

        if (!ok)
          throw MYMONEYEXCEPTION(QString("Invalid year entry: %1").arg(*it_scanned));

        //
        // 2-digit year case
        //
        // this algorithm will pick a year within +/- 50 years of the
        // centurymidpoint parameter.  i.e. if the midpoint is 2000,
        // then 0-49 will become 2000-2049, and 50-99 will become 1950-1999
        if (year < 100) {
          unsigned centuryend = _centurymidpoint + 50;
          unsigned centurybegin = _centurymidpoint - 50;

          if (year < centuryend % 100)
            year += 100;
          year += centurybegin - centurybegin % 100;
        }

        if (year < 1900)
          throw MYMONEYEXCEPTION(QString("Invalid year (%1)").arg(year));

        break;
      default:
        throw MYMONEYEXCEPTION("Invalid format character");
    }

    ++it_scanned;
    ++it_format;
  }
  QDate result(year, month, day);
  if (! result.isValid())
    throw MYMONEYEXCEPTION(QString("Invalid date (yr%1 mo%2 dy%3)").arg(year).arg(month).arg(day));

  return result;
}

//
// Unit test helpers
//

convertertest::QuoteReceiver::QuoteReceiver(WebPriceQuote* q, QObject* parent) :
    QObject(parent)
{
  connect(q, SIGNAL(quote(QString,QString,QDate,double)),
          this, SLOT(slotGetQuote(QString,QString,QDate,double)));
  connect(q, SIGNAL(status(QString)),
          this, SLOT(slotStatus(QString)));
  connect(q, SIGNAL(error(QString)),
          this, SLOT(slotError(QString)));
}

convertertest::QuoteReceiver::~QuoteReceiver()
{
}

void convertertest::QuoteReceiver::slotGetQuote(const QString&, const QString&, const QDate& d, const double& m)
{
//   kDebug(2) << "test::QuoteReceiver::slotGetQuote( , " << d << " , " << m.toString() << " )";

  m_price = MyMoneyMoney(m);
  m_date = d;
}

void convertertest::QuoteReceiver::slotStatus(const QString& msg)
{
//   kDebug(2) << "test::QuoteReceiver::slotStatus( " << msg << " )";

  m_statuses += msg;
}

void convertertest::QuoteReceiver::slotError(const QString& msg)
{
//   kDebug(2) << "test::QuoteReceiver::slotError( " << msg << " )";

  m_errors += msg;
}

// leave this moc until we will have resolved our dependency issues
// now 'converter' depends on 'kmymoney' a pointer to the application
// defined in main.cpp, which makes this static library unusable without
// the --as-needed linker flag;
// otherwise the 'moc' methods of this object will be linked into the automoc
// object file which contains methods from all the other objects from this
// library, thus even if the --as-needed option is used all objects will be
// pulled in while linking 'convertertest' which only needs the WebPriceQuote
// object - spent a whole day investigating this
#include "moc_webpricequote.cpp"
