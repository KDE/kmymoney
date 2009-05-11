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

// ----------------------------------------------------------------------------
// QT Headers

#include <qfile.h>
#include <qregexp.h>
#include <q3textstream.h>
#include <q3process.h>
//Added by qt3to4:
#include <Q3CString>

// ----------------------------------------------------------------------------
// KDE Headers

#include <kio/netaccess.h>
#include <kio/scheduler.h>
#include <kurl.h>
#include <klocale.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kcalendarsystem.h>
#include <ktemporaryfile.h>
#include <kshell.h>

// ----------------------------------------------------------------------------
// Project Headers

#include "../mymoney/mymoneyexception.h"
#include "mymoneyqifprofile.h"
#include "webpricequote.h"

// define static members
QString WebPriceQuote::m_financeQuoteScriptPath;
QStringList WebPriceQuote::m_financeQuoteSources;

QString * WebPriceQuote::lastErrorMsg;
int WebPriceQuote::lastErrorCode = 0;

WebPriceQuote::WebPriceQuote( QObject* _parent, const char* _name ):
  QObject( _parent, _name )
{
  m_financeQuoteScriptPath =
      KGlobal::dirs()->findResource("appdata", QString("misc/financequote.pl"));
  connect(&m_filter,SIGNAL(processExited(const QString&)),this,SLOT(slotParseQuote(const QString&)));
}

WebPriceQuote::~WebPriceQuote()
{
}

bool WebPriceQuote::launch( const QString& _symbol, const QString& _id, const QString& _sourcename )
{
  if (_sourcename.contains("Finance::Quote"))
    return (launchFinanceQuote (_symbol, _id, _sourcename));
  else
    return (launchNative (_symbol, _id, _sourcename));
}

bool WebPriceQuote::launchNative( const QString& _symbol, const QString& _id, const QString& _sourcename ) {
  bool result = true;
  m_symbol = _symbol;
  m_id = _id;

//   emit status(QString("(Debug) symbol=%1 id=%2...").arg(_symbol,_id));

  // if we're running normally, with a UI, we can just get these the normal way,
  // from the config file
  if ( kapp )
  {
    QString sourcename = _sourcename;
    if ( sourcename.isEmpty() )
      sourcename = "Yahoo";

    if ( quoteSources().contains(sourcename) )
      m_source = WebPriceQuoteSource(sourcename);
    else
      emit error(QString("Source <%1> does not exist.").arg(sourcename));
  }
  // otherwise, if we have no kapp, we have no config.  so we just get them from
  // the defaults
  else
  {
    if ( _sourcename.isEmpty() )
      m_source = defaultQuoteSources()["Yahoo"];
    else
      m_source = defaultQuoteSources()[_sourcename];
  }

  KUrl url;

  // if the source has room for TWO symbols..
  if ( m_source.m_url.contains("%2") )
  {
    // this is a two-symbol quote.  split the symbol into two.  valid symbol
    // characters are: 0-9, A-Z and the dot.  anything else is a separator
    QRegExp splitrx("([0-9a-z\\.]+)[^a-z0-9]+([0-9a-z\\.]+)",false /*case sensitive*/);

    // if we've truly found 2 symbols delimited this way...
    if ( splitrx.search(m_symbol) != -1 )
      url = KUrl::fromPathOrUrl(m_source.m_url.arg(splitrx.cap(1),splitrx.cap(2)));
    else
      kDebug(2) << "WebPriceQuote::launch() did not find 2 symbols";
  }
  else
    // a regular one-symbol quote
    url = KUrl::fromPathOrUrl(m_source.m_url.arg(m_symbol));

  // If we're running a non-interactive session (with no UI), we can't
  // use KIO::NetAccess, so we have to get our web data the old-fashioned
  // way... with 'wget'.
  //
  // Note that a 'non-interactive' session right now means only the test
  // cases.  Although in the future if KMM gains a non-UI mode, this would
  // still be useful
  if ( ! kapp && ! url.isLocalFile() )
    url = KUrl::fromPathOrUrl("/usr/bin/wget -O - " + url.prettyUrl());

  if ( url.isLocalFile() )
  {
    emit status(QString("Executing %1...").arg(url.path()));

    m_filter.clearArguments();
    m_filter << QStringList::split(" ",url.path());
    m_filter.setSymbol(m_symbol);

    // if we're running non-interactive, we'll need to block.
    // otherwise, just let us know when it's done.
    K3Process::RunMode mode = K3Process::NotifyOnExit;
    if ( ! kapp )
      mode = K3Process::Block;

    if(m_filter.start(mode, K3Process::All))
    {
      result = true;
      m_filter.resume();
    }
    else
    {
      emit error(QString("Unable to launch: %1").arg(url.path()));
      slotParseQuote(QString());
    }
  }
  else
  {
    emit status(QString("Fetching URL %1...").arg(url.prettyUrl()));

    QString tmpFile;
    if( download( url, tmpFile, NULL ) )
    {
      kDebug(2) << "Downloaded " << tmpFile;
      QFile f(tmpFile);
      if ( f.open( QIODevice::ReadOnly ) )
      {
        result = true;
        QString quote = Q3TextStream(&f).read();
        f.close();
        slotParseQuote(quote);
      }
      else
      {
        slotParseQuote(QString());
      }
      removeTempFile( tmpFile );
    }
    else
    {
      emit error(KIO::NetAccess::lastErrorString());
      slotParseQuote(QString());
    }
  }
  return result;
}

void WebPriceQuote::removeTempFile(const QString& tmpFile)
{
  if(tmpFile == m_tmpFile) {
    unlink(tmpFile);
    m_tmpFile = QString();
  }
}

bool WebPriceQuote::download(const KUrl& u, QString & target, QWidget* window)
{
  m_tmpFile = QString();

  // the following code taken and adapted from KIO::NetAccess::download()
  if (target.isEmpty())
  {
    KTemporaryFile tmpFile;
    target = tmpFile.name();
    m_tmpFile = target;
  }

  KUrl dest;
  dest.setPath( target );


  // the following code taken and adapted from KIO::NetAccess::filecopyInternal()
  bJobOK = true; // success unless further error occurs

  KIO::Scheduler::checkSlaveOnHold(true);
  KIO::Job * job = KIO::file_copy( u, dest, -1, KIO::Overwrite | KIO::HideProgressInfo );
  job->setWindow (window);
  job->addMetaData("cache", "reload");  // bypass cache
  connect( job, SIGNAL( result (KIO::Job *) ),
           this, SLOT( slotResult (KIO::Job *) ) );

  enter_loop();
  return bJobOK;

}

// The following parts are copied and adjusted from KIO::NetAccess

// If a troll sees this, he kills me
void qt_enter_modal( QWidget *widget );
void qt_leave_modal( QWidget *widget );

void WebPriceQuote::enter_loop(void)
{
  QWidget dummy(0,0,Qt::WType_Dialog | WShowModal);
  dummy.setFocusPolicy( Qt::NoFocus );
  qt_enter_modal(&dummy);
  qApp->enter_loop();
  qt_leave_modal(&dummy);
}

void WebPriceQuote::slotResult( KIO::Job * job )
{
  lastErrorCode = job->error();
  bJobOK = !job->error();
  if ( !bJobOK )
  {
    if ( !lastErrorMsg )
      lastErrorMsg = new QString;
    *lastErrorMsg = job->errorString();
  }

  qApp->exit_loop();
}
// The above parts are copied and adjusted from KIO::NetAccess

bool WebPriceQuote::launchFinanceQuote ( const QString& _symbol, const QString& _id,
                               const QString& _sourcename ) {
  bool result = true;
  m_symbol = _symbol;
  m_id = _id;
  QString FQSource = _sourcename.section (" ", 1);
  m_source = WebPriceQuoteSource (_sourcename, m_financeQuoteScriptPath,
                                  "\"([^,\"]*)\",.*",  // symbol regexp
                                  "[^,]*,[^,]*,\"([^\"]*)\"", // price regexp
                                  "[^,]*,([^,]*),.*", // date regexp
                                  "%y-%m-%d"); // date format

  //emit status(QString("(Debug) symbol=%1 id=%2...").arg(_symbol,_id));


  m_filter.clearArguments();
  m_filter << "perl" << m_financeQuoteScriptPath << FQSource << KShell::quoteArg(_symbol);
  m_filter.setUseShell(true);
  m_filter.setSymbol(m_symbol);
  emit status(QString("Executing %1 %2 %3...").arg(m_financeQuoteScriptPath).arg(FQSource).arg(_symbol));

    // if we're running non-interactive, we'll need to block.
    // otherwise, just let us know when it's done.
  K3Process::RunMode mode = K3Process::NotifyOnExit;
  if ( ! kapp )
    mode = K3Process::Block;

  if(m_filter.start(mode, K3Process::All))
  {
    result = true;
    m_filter.resume();
  }
  else
  {
    emit error(QString("Unable to launch: %1").arg(m_financeQuoteScriptPath));
    slotParseQuote(QString());
  }

  return result;
}

void WebPriceQuote::slotParseQuote(const QString& _quotedata)
{
  QString quotedata = _quotedata;
  bool gotprice = false;
  bool gotdate = false;

//    kDebug(2) << "WebPriceQuote::slotParseQuote( " << _quotedata << " ) ";

  if ( ! quotedata.isEmpty() )
  {
    if(!m_source.m_skipStripping) {
      //
      // First, remove extranous non-data elements
      //

      // HTML tags
      quotedata.remove(QRegExp("<[^>]*>"));

      // &...;'s
      quotedata.replace(QRegExp("&\\w+;")," ");

      // Extra white space
      quotedata = quotedata.simplified();
    }

#if KMM_DEBUG
    // Enable to get a look at the data coming back from the source after it's stripped
    QFile file("stripped.txt");
    if ( file.open( QIODevice::WriteOnly ) )
    {
      Q3TextStream( &file ) << quotedata;
      file.close();
    }
#endif

    QRegExp symbolRegExp(m_source.m_sym);
    QRegExp dateRegExp(m_source.m_date);
    QRegExp priceRegExp(m_source.m_price);

    if( symbolRegExp.search(quotedata) > -1)
      emit status(i18n("Symbol found: %1").arg(symbolRegExp.cap(1)));

    if(priceRegExp.search(quotedata)> -1)
    {
      gotprice = true;

      // Deal with european quotes that come back as X.XXX,XX or XX,XXX
      //
      // We will make the assumption that ALL prices have a decimal separator.
      // So "1,000" always means 1.0, not 1000.0.
      //
      // Remove all non-digits from the price string except the last one, and
      // set the last one to a period.
      QString pricestr = priceRegExp.cap(1);

      int pos = pricestr.findRev(QRegExp("\\D"));
      if ( pos > 0 )
      {
        pricestr[pos] = '.';
        pos = pricestr.findRev(QRegExp("\\D"),pos-1);
      }
      while ( pos > 0 )
      {
        pricestr.remove(pos,1);
        pos = pricestr.findRev(QRegExp("\\D"),pos);
      }

      m_price = pricestr.toDouble();
      emit status(i18n("Price found: %1 (%2)").arg(pricestr).arg(m_price));
    }

    if(dateRegExp.search(quotedata) > -1)
    {
      QString datestr = dateRegExp.cap(1);

      MyMoneyDateFormat dateparse(m_source.m_dateformat);
      try
      {
        m_date = dateparse.convertString( datestr,false /*strict*/ );
        gotdate = true;
        emit status(i18n("Date found: %1").arg(m_date.toString()));;
      }
      catch (MyMoneyException* e)
      {
        // emit error(i18n("Unable to parse date %1 using format %2: %3").arg(datestr,dateparse.format(),e->what()));
        m_date = QDate::currentDate();
        gotdate = true;
        delete e;
      }
    }

    if ( gotprice && gotdate )
    {
      emit quote( m_id, m_symbol, m_date, m_price );
    }
    else
    {
      emit error(i18n("Unable to update price for %1").arg(m_symbol));
      emit failed( m_id, m_symbol );
    }
  }
  else
  {
    emit error(i18n("Unable to update price for %1").arg(m_symbol));
    emit failed( m_id, m_symbol );
  }
}

QMap<QString,WebPriceQuoteSource> WebPriceQuote::defaultQuoteSources(void)
{
  QMap<QString,WebPriceQuoteSource> result;

  result["Yahoo"] = WebPriceQuoteSource("Yahoo",
    "http://finance.yahoo.com/d/quotes.csv?s=%1&f=sl1d1",
    "\"([^,\"]*)\",.*",  // symbolregexp
    "[^,]*,([^,]*),.*", // priceregexp
    "[^,]*,[^,]*,\"([^\"]*)\"", // dateregexp
    "%m %d %y" // dateformat
  );

  result["Yahoo Currency"] = WebPriceQuoteSource("Yahoo Currency",
    "http://finance.yahoo.com/d/quotes.csv?s=%1%2=X&f=sl1d1",
    "\"([^,\"]*)\",.*",  // symbolregexp
    "[^,]*,([^,]*),.*", // priceregexp
    "[^,]*,[^,]*,\"([^\"]*)\"", // dateregexp
    "%m %d %y" // dateformat
  );

  result["Yahoo UK"] = WebPriceQuoteSource("Yahoo UK",
    "http://uk.finance.yahoo.com/d/quotes.csv?s=%1&f=sl1d1",
    "\"([^,\"]*)\",.*",  // symbolregexp
    "[^,]*,([^,]*),.*", // priceregexp
    "[^,]*,[^,]*,\"([^\"]*)\"", // dateregexp
    "%m %d %y" // dateformat
  );
  // sl1d1 format for Yahoo France doesn't seem to give a date ever
  // sl1d3 gives us time (99h99) and date
  result["Yahoo France"] = WebPriceQuoteSource("Yahoo France",
    "http://fr.finance.yahoo.com/d/quotes.csv?s=%1&f=sl1d3",
    "([^;]*).*",             // symbolregexp
    "[^;]*.([^;]*),*",       // priceregexp
    "[^;]*.[^;]*...h...([^;]*)", // dateregexp
    "%d/%m/%y"               // dateformat
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
    "Net Asset Value (\\d+\\.\\d+)", // priceregexp
    "NAV update (\\d+\\D+\\d+\\D+\\d+)", // dateregexp
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
    "Akt\\. Kurs.(\\d+,\\d\\d)", // priceregexp
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

  // This quote source provided by Peter Lord
  // The trading symbol will normally be the SEDOL (see wikipedia) but
  // the flexibility presently (1/2008) in the code will allow use of
  // the ISIN or MEXID (FT specific) codes
  result["Financial Times UK Funds"] = WebPriceQuoteSource("Financial Times UK Funds",
      "http://funds.ft.com/funds/simpleSearch.do?searchArea=%&search=%1",
      "SEDOL[\\ ]*(\\d+.\\d+)", // symbol regexp
      "\\(GBX\\)[\\ ]*([0-9,]*.\\d+)[\\ ]*", // price regexp
      "Valuation date:[\\ ]*(\\d+/\\d+/\\d+)", // date regexp
      "%d/%m/%y" // date format
  );

  // This quote source provided by Danny Scott
  result["Yahoo Canada"] = WebPriceQuoteSource("Yahoo Canada",
      "http://ca.finance.yahoo.com/q?s=%1",
      "%1", // symbol regexp
      "Last Trade: (\\d+\\.\\d+)", // price regexp
      "day, (.\\D+\\d+\\D+\\d+)", // date regexp
      "%m %d %y" // date format
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
    "Kurs.*(\\d+\\.\\d+).*Data",    // price regexp
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

  return result;
}

QStringList WebPriceQuote::quoteSources (const _quoteSystemE _system) {
  if (_system == Native)
    return (quoteSourcesNative());
  else
    return (quoteSourcesFinanceQuote());
}

QStringList WebPriceQuote::quoteSourcesNative()
{
  KConfig *kconfig = KGlobal::config();
  QStringList groups = kconfig->groupList();

  QStringList::Iterator it;
  QRegExp onlineQuoteSource(QString("^Online-Quote-Source-(.*)$"));

  // get rid of all 'non online quote source' entries
  for(it = groups.begin(); it != groups.end(); it = groups.remove(it)) {
    if(onlineQuoteSource.search(*it) >= 0) {
      // Insert the name part
      groups.insert(it, onlineQuoteSource.cap(1));
    }
  }

  // if the user has the OLD quote source defined, now is the
  // time to remove that entry and convert it to the new system.
  if ( ! groups.count() && kconfig->hasGroup("Online Quotes Options") )
  {
    kconfig->setGroup("Online Quotes Options");
    QString url(kconfig->readEntry("URL","http://finance.yahoo.com/d/quotes.csv?s=%1&f=sl1d1"));
    QString symbolRegExp(kconfig->readEntry("SymbolRegex","\"([^,\"]*)\",.*"));
    QString priceRegExp(kconfig->readEntry("PriceRegex","[^,]*,([^,]*),.*"));
    QString dateRegExp(kconfig->readEntry("DateRegex","[^,]*,[^,]*,\"([^\"]*)\""));
    kconfig->deleteGroup("Online Quotes Options");

    groups += "Old Source";
    kconfig->setGroup(QString("Online-Quote-Source-%1").arg("Old Source"));
    kconfig->writeEntry("URL", url);
    kconfig->writeEntry("SymbolRegex", symbolRegExp);
    kconfig->writeEntry("PriceRegex",priceRegExp);
    kconfig->writeEntry("DateRegex", dateRegExp);
    kconfig->writeEntry("DateFormatRegex", "%m %d %y");
    kconfig->sync();
  }

  // Set up each of the default sources.  These are done piecemeal so that
  // when we add a new source, it's automatically picked up.
  QMap<QString,WebPriceQuoteSource> defaults = defaultQuoteSources();
  QMap<QString,WebPriceQuoteSource>::const_iterator it_source = defaults.begin();
  while ( it_source != defaults.end() )
  {
    if ( ! groups.contains( (*it_source).m_name ) )
    {
      groups += (*it_source).m_name;
      (*it_source).write();
      kconfig->sync();
    }
    ++it_source;
  }

  return groups;
}

QStringList WebPriceQuote::quoteSourcesFinanceQuote()
{
  if (m_financeQuoteSources.empty()) { // run the process one time only
    FinanceQuoteProcess getList;
    m_financeQuoteScriptPath =
        KGlobal::dirs()->findResource("appdata", QString("misc/financequote.pl"));
    getList.launch( m_financeQuoteScriptPath );
    while (!getList.isFinished()) {
      qApp->processEvents();
    }
    m_financeQuoteSources = getList.getSourceList();
  }
  return (m_financeQuoteSources);
}

//
// Helper class to load/save an individual source
//

WebPriceQuoteSource::WebPriceQuoteSource(const QString& name, const QString& url, const QString& sym, const QString& price, const QString& date, const QString& dateformat):
  m_name(name),
  m_url(url),
  m_sym(sym),
  m_price(price),
  m_date(date),
  m_dateformat(dateformat)
{
}

WebPriceQuoteSource::WebPriceQuoteSource(const QString& name)
{
  m_name = name;
  KConfig *kconfig = KGlobal::config();
  kconfig->setGroup(QString("Online-Quote-Source-%1").arg(m_name));
  m_sym = kconfig->readEntry("SymbolRegex");
  m_date = kconfig->readEntry("DateRegex");
  m_dateformat = kconfig->readEntry("DateFormatRegex","%m %d %y");
  m_price = kconfig->readEntry("PriceRegex");
  m_url = kconfig->readEntry("URL");
  m_skipStripping = kconfig->readBoolEntry("SkipStripping", false);
}

void WebPriceQuoteSource::write(void) const
{
  KConfig *kconfig = KGlobal::config();
  kconfig->setGroup(QString("Online-Quote-Source-%1").arg(m_name));
  kconfig->writeEntry("URL", m_url);
  kconfig->writeEntry("PriceRegex", m_price);
  kconfig->writeEntry("DateRegex", m_date);
  kconfig->writeEntry("DateFormatRegex", m_dateformat);
  kconfig->writeEntry("SymbolRegex", m_sym);
  if(m_skipStripping)
    kconfig->writeEntry("SkipStripping", m_skipStripping);
  else
    kconfig->deleteEntry("SkipStripping");
}

void WebPriceQuoteSource::rename(const QString& name)
{
  remove();
  m_name = name;
  write();
}

void WebPriceQuoteSource::remove(void) const
{
  KConfig *kconfig = KGlobal::config();
  kconfig->deleteGroup(QString("Online-Quote-Source-%1").arg(m_name));
}

//
// Helper class to babysit the K3Process used for running the local script in that case
//

WebPriceQuoteProcess::WebPriceQuoteProcess(void)
{
  connect(this, SIGNAL(receivedStdout(K3Process*, char*, int)), this, SLOT(slotReceivedDataFromFilter(K3Process*, char*, int)));
  connect(this, SIGNAL(processExited(K3Process*)), this, SLOT(slotProcessExited(K3Process*)));
}

void WebPriceQuoteProcess::slotReceivedDataFromFilter(K3Process* /*_process*/, char* _pcbuffer, int _nbufferlen)
{
  QByteArray data;
  data.duplicate(_pcbuffer, _nbufferlen);

//   kDebug(2) << "WebPriceQuoteProcess::slotReceivedDataFromFilter(): " << QString(data);
  m_string += QString(data);
}

void WebPriceQuoteProcess::slotProcessExited(K3Process*)
{
//   kDebug(2) << "WebPriceQuoteProcess::slotProcessExited()";
  emit processExited(m_string);
  m_string.truncate(0);
}

//
// Helper class to babysit the K3Process used for running the Finance Quote sources script
//

FinanceQuoteProcess::FinanceQuoteProcess(void)
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
  connect(this, SIGNAL(receivedStdout(K3Process*, char*, int)), this, SLOT(slotReceivedDataFromFilter(K3Process*, char*, int)));
  connect(this, SIGNAL(processExited(K3Process*)), this, SLOT(slotProcessExited(K3Process*)));
}

void FinanceQuoteProcess::slotReceivedDataFromFilter(K3Process* /*_process*/, char* _pcbuffer, int _nbufferlen)
{
  QByteArray data;
  data.duplicate(_pcbuffer, _nbufferlen);

//   kDebug(2) << "WebPriceQuoteProcess::slotReceivedDataFromFilter(): " << QString(data);
  m_string += QString(data);
}

void FinanceQuoteProcess::slotProcessExited(K3Process*)
{
//   kDebug(2) << "WebPriceQuoteProcess::slotProcessExited()";
  m_isDone = true;
}

void FinanceQuoteProcess::launch (const QString& scriptPath) {
  clearArguments();
  arguments.append(Q3CString("perl"));
  arguments.append (Q3CString(scriptPath));
  arguments.append (Q3CString("-l"));
  if (!start(K3Process::NotifyOnExit, K3Process::Stdout)) qFatal ("Unable to start FQ script");
  return;
}

QStringList FinanceQuoteProcess::getSourceList() {
  QStringList raw = QStringList::split(0x0A, m_string);
  QStringList sources;
  QStringList::iterator it;
  for (it = raw.begin(); it != raw.end(); ++it) {
    if (m_fqNames[*it].isEmpty()) sources.append(*it);
    else sources.append(m_fqNames[*it]);
  }
  sources.sort();
  return (sources);
}

const QString FinanceQuoteProcess::crypticName(const QString& niceName) {
  QString ret (niceName);
  fqNameMap::iterator it;
  for (it = m_fqNames.begin(); it != m_fqNames.end(); ++it) {
    if (niceName == it.data()) {
      ret = it.key();
      break;
    }
  }
  return (ret);
}

const QString FinanceQuoteProcess::niceName(const QString& crypticName) {
  QString ret (m_fqNames[crypticName]);
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

QDate MyMoneyDateFormat::convertString(const QString& _in, bool _strict, unsigned _centurymidpoint) const
{
  //
  // Break date format string into component parts
  //

  QRegExp formatrex("%([mdy]+)(\\W+)%([mdy]+)(\\W+)%([mdy]+)",false /* case sensitive */);
  if ( formatrex.search(m_format) == -1 )
  {
    throw new MYMONEYEXCEPTION("Invalid format string");
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
  inputrex.setCaseSensitive(false);

  // strict mode means we must enforce the delimiters as specified in the
  // format.  non-strict allows any delimiters
  if ( _strict )
    inputrex.setPattern(QString("(\\w+)%1(\\w+)%2(\\w+)").arg(formatDelimiters[0],formatDelimiters[1]));
  else
    inputrex.setPattern("(\\w+)\\W+(\\w+)\\W+(\\w+)");

  if ( inputrex.search(_in) == -1 )
  {
    throw new MYMONEYEXCEPTION("Invalid input string");
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
  QStringList::const_iterator it_scanned = scannedParts.begin();
  QStringList::const_iterator it_format = formatParts.begin();
  while ( it_scanned != scannedParts.end() )
  {
    switch ( (*it_format)[0] )
    {
    case 'd':
      // remove any extraneous non-digits (e.g. read "3rd" as 3)
      ok = false;
      if ( digitrex.search(*it_scanned) != -1 )
        day = digitrex.cap(1).toUInt(&ok);
      if ( !ok || day > 31 )
        throw new MYMONEYEXCEPTION(QString("Invalid day entry: %1").arg(*it_scanned));
      break;
    case 'm':
      month = (*it_scanned).toUInt(&ok);
      if ( !ok )
      {
        // maybe it's a textual date
        unsigned i = 1;
        while ( i <= 12 )
        {
          if(KGlobal::locale()->calendar()->monthName(i, 2000, true).toLower() == *it_scanned
          || KGlobal::locale()->calendar()->monthName(i, 2000, false).toLower() == *it_scanned)
            month = i;
          ++i;
        }
      }

      if ( month < 1 || month > 12 )
        throw new MYMONEYEXCEPTION(QString("Invalid month entry: %1").arg(*it_scanned));

      break;
    case 'y':
      if ( _strict && (*it_scanned).length() != (*it_format).length())
        throw new MYMONEYEXCEPTION(QString("Length of year (%1) does not match expected length (%2).")
                .arg(*it_scanned,*it_format));

      year = (*it_scanned).toUInt(&ok);

      if (!ok)
        throw new MYMONEYEXCEPTION(QString("Invalid year entry: %1").arg(*it_scanned));

      //
      // 2-digit year case
      //
      // this algorithm will pick a year within +/- 50 years of the
      // centurymidpoint parameter.  i.e. if the midpoint is 2000,
      // then 0-49 will become 2000-2049, and 50-99 will become 1950-1999
      if ( year < 100 )
      {
        unsigned centuryend = _centurymidpoint + 50;
        unsigned centurybegin = _centurymidpoint - 50;

        if ( year < centuryend % 100 )
          year += 100;
        year += centurybegin - centurybegin % 100;
      }

      if ( year < 1900 )
        throw new MYMONEYEXCEPTION(QString("Invalid year (%1)").arg(year));

      break;
    default:
      throw new MYMONEYEXCEPTION("Invalid format character");
    }

    ++it_scanned;
    ++it_format;
  }

  QDate result(year,month,day);
  if ( ! result.isValid() )
    throw new MYMONEYEXCEPTION(QString("Invalid date (yr%1 mo%2 dy%3)").arg(year).arg(month).arg(day));

  return result;
}

//
// Unit test helpers
//

convertertest::QuoteReceiver::QuoteReceiver(WebPriceQuote* q, QObject* parent, const char *name) :
  QObject(parent,name)
{
  connect(q,SIGNAL(quote(const QString&,const QDate&, const double&)),
    this,SLOT(slotGetQuote(const QString&,const QDate&, const double&)));
  connect(q,SIGNAL(status(const QString&)),
    this,SLOT(slotStatus(const QString&)));
  connect(q,SIGNAL(error(const QString&)),
    this,SLOT(slotError(const QString&)));
}

convertertest::QuoteReceiver::~QuoteReceiver()
{
}

void convertertest::QuoteReceiver::slotGetQuote(const QString&,const QDate& d, const double& m)
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

// vim:cin:si:ai:et:ts=2:sw=2:

#include "webpricequote.moc"
