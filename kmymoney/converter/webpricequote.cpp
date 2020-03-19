/***************************************************************************
                          webpricequote.cpp
                             -------------------
    begin                : Thu Dec 30 2004
    copyright            : (C) 2004 by Ace Jones
    email                : Ace Jones <acejones@users.sourceforge.net>
    copyright            : (C) 2017 by Łukasz Wojniłowicz
    email                : Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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
#include <QTextCodec>
#include <QByteArray>
#include <QString>
#include <QTemporaryFile>
#include <QUrl>
#include <QRegularExpression>
#include <QDebug>
#include <QLoggingCategory>
#include <QLocale>

// ----------------------------------------------------------------------------
// KDE Headers

#include <KLocalizedString>
#include <KConfig>
#include <KShell>
#include <KConfigGroup>
#include <KEncodingProber>
#include <KIO/Scheduler>
#include <KIO/Job>
#include <KJobWidgets>

// ----------------------------------------------------------------------------
// Project Headers

#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneysecurity.h"

Q_DECLARE_LOGGING_CATEGORY(WEBPRICEQUOTE)
Q_LOGGING_CATEGORY(WEBPRICEQUOTE, "kmymoney_webpricequote")

// define static members
QString WebPriceQuote::m_financeQuoteScriptPath;
QStringList WebPriceQuote::m_financeQuoteSources;

class WebPriceQuote::Private
{
public:
  WebPriceQuoteProcess m_filter;
  QString m_quoteData;
  QString m_webID;
  QString m_kmmID;
  QDate m_date;
  QDate m_fromDate;
  QDate m_toDate;
  double m_price;
  WebPriceQuoteSource m_source;
  PricesProfile    m_CSVSource;
};

WebPriceQuote::WebPriceQuote(QObject* _parent):
    QObject(_parent),
    d(new Private)
{
  // only do this once (I know, it is not thread safe, but it should
  // always yield the same result so we don't do any semaphore foo here)
  if (m_financeQuoteScriptPath.isEmpty()) {
    m_financeQuoteScriptPath = QStandardPaths::locate(QStandardPaths::DataLocation, QString("misc/financequote.pl"));
  }
  connect(&d->m_filter, SIGNAL(processExited(QString)), this, SLOT(slotParseQuote(QString)));
}

WebPriceQuote::~WebPriceQuote()
{
  delete d;
}

void WebPriceQuote::setDate(const QDate& _from, const QDate& _to)
{
  d->m_fromDate = _from;
  d->m_toDate = _to;
}

bool WebPriceQuote::launch(const QString& _webID, const QString& _kmmID, const QString& _sourcename)
{
  if (_sourcename.contains("Finance::Quote"))
    return (launchFinanceQuote(_webID, _kmmID, _sourcename));
  else if ((!d->m_fromDate.isValid() || !d->m_toDate.isValid()) ||
           (d->m_fromDate == d->m_toDate && d->m_toDate == QDate::currentDate()))
    return (launchNative(_webID, _kmmID, _sourcename));
  else
    return launchCSV(_webID, _kmmID, _sourcename);
}

bool WebPriceQuote::launchCSV(const QString& _webID, const QString& _kmmID, const QString& _sourcename)
{
  d->m_webID = _webID;
  d->m_kmmID = _kmmID;

//   emit status(QString("(Debug) symbol=%1 id=%2...").arg(_symbol,_id));

  // Get sources from the config file
  QString sourcename = _sourcename;
  if (sourcename.isEmpty())
    return false;

  // for historical exchange rates we switch to Stooq
  if (sourcename == QLatin1String("KMyMoney Currency"))
    sourcename = QLatin1String("Stooq Currency");

  if (quoteSources().contains(sourcename))
    d->m_source = WebPriceQuoteSource(sourcename);
  else {
    emit error(i18n("Source <placeholder>%1</placeholder> does not exist.", sourcename));
    emit failed(d->m_kmmID, d->m_webID);
    return false;
  }

  int monthOffset = 0;
  if (sourcename.contains(QLatin1String("Yahoo"), Qt::CaseInsensitive))
    monthOffset = -1;

  QUrl url;
  QString urlStr = d->m_source.m_csvUrl;
  int i = urlStr.indexOf(QLatin1String("%y"));
  if (i != -1)
    urlStr.replace(i, 2, QString().setNum(d->m_fromDate.year()));
  i = urlStr.indexOf(QLatin1String("%y"));
  if (i != -1)
    urlStr.replace(i, 2, QString().setNum(d->m_toDate.year()));

  i = urlStr.indexOf(QLatin1String("%m"));
  if (i != -1)
    urlStr.replace(i, 2, QString().setNum(d->m_fromDate.month() + monthOffset).rightJustified(2, QLatin1Char('0')));
  i = urlStr.indexOf(QLatin1String("%m"));
  if (i != -1)
    urlStr.replace(i, 2, QString().setNum(d->m_toDate.month() + monthOffset).rightJustified(2, QLatin1Char('0')));

  i = urlStr.indexOf(QLatin1String("%d"));
  if (i != -1)
    urlStr.replace(i, 2, QString().setNum(d->m_fromDate.day()).rightJustified(2, QLatin1Char('0')));
  i = urlStr.indexOf(QLatin1String("%d"));
  if (i != -1)
    urlStr.replace(i, 2, QString().setNum(d->m_toDate.day()).rightJustified(2, QLatin1Char('0')));

  if (urlStr.contains(QLatin1String("%y")) || urlStr.contains(QLatin1String("%m")) || urlStr.contains(QLatin1String("%d"))) {
    emit error(i18n("Cannot resolve input date."));
    emit failed(d->m_kmmID, d->m_webID);
    return false;
  }

  bool isCurrency = false;
  if (urlStr.contains(QLatin1String("%2"))) {
    d->m_CSVSource.m_profileType = Profile::CurrencyPrices;
    isCurrency = true;
  } else
    d->m_CSVSource.m_profileType = Profile::StockPrices;

  d->m_CSVSource.m_profileName = sourcename;
  if (!d->m_CSVSource.readSettings(CSVImporterCore::configFile())) {
    QMap<QString, PricesProfile> result = defaultCSVQuoteSources();
    d->m_CSVSource = result.value(sourcename);
    if (d->m_CSVSource.m_profileName.isEmpty()) {
      emit error(i18n("CSV source <placeholder>%1</placeholder> does not exist.", sourcename));
      emit failed(d->m_kmmID, d->m_webID);
      return false;
    }
  }

  if (isCurrency) {
    // this is a two-symbol quote.  split the symbol into two.  valid symbol
    // characters are: 0-9, A-Z and the dot.  anything else is a separator
    QRegularExpression splitrx("([0-9a-z\\.]+)[^a-z0-9]+([0-9a-z\\.]+)", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match;
    // if we've truly found 2 symbols delimited this way...
    if (d->m_webID.indexOf(splitrx, 0, &match) != -1) {
      url = QUrl(urlStr.arg(match.captured(1), match.captured(2)));
      d->m_CSVSource.m_currencySymbol = match.captured(2);
      d->m_CSVSource.m_securitySymbol = match.captured(1);
    } else {
      qCDebug(WEBPRICEQUOTE) << "WebPriceQuote::launch() did not find 2 symbols";
      emit error(i18n("Cannot find from and to currency."));
      emit failed(d->m_kmmID, d->m_webID);
      return false;
    }

  } else {
    // a regular one-symbol quote
    url = QUrl(urlStr.arg(d->m_webID));
    d->m_CSVSource.m_securityName = MyMoneyFile::instance()->security(d->m_kmmID).name();
    d->m_CSVSource.m_securitySymbol = MyMoneyFile::instance()->security(d->m_kmmID).tradingSymbol();
  }

  if (url.isLocalFile()) {
    emit error(i18n("Local quote sources aren't supported."));
    emit failed(d->m_kmmID, d->m_webID);
    return false;
  } else {
    //silent download
    emit status(i18n("Fetching URL %1...", url.toDisplayString()));
    QString tmpFile;
    {
      QTemporaryFile tmpFileFile;
      tmpFileFile.setAutoRemove(false);
      if (tmpFileFile.open())
          qDebug() << "created tmpfile";

      tmpFile = tmpFileFile.fileName();
    }
    QFile::remove(tmpFile);
    const QUrl dest = QUrl::fromLocalFile(tmpFile);
    KIO::Scheduler::checkSlaveOnHold(true);
    KIO::Job *job = KIO::file_copy(url, dest, -1, KIO::HideProgressInfo);
    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(downloadCSV(KJob*)));
  }
  return true;
}

bool WebPriceQuote::launchNative(const QString& _webID, const QString& _kmmID, const QString& _sourcename)
{
  d->m_webID = _webID;
  d->m_kmmID = _kmmID;

  if (_webID == i18n("[No identifier]")) {
    emit error(i18n("<placeholder>%1</placeholder> skipped because it doesn't have identification number.", _kmmID));
    emit failed(d->m_kmmID, d->m_webID);
    return false;
  }
//   emit status(QString("(Debug) symbol=%1 id=%2...").arg(_symbol,_id));

  // Get sources from the config file
  QString sourcename = _sourcename;
  if (sourcename.isEmpty())
    sourcename = "Yahoo";

  if (quoteSources().contains(sourcename))
    d->m_source = WebPriceQuoteSource(sourcename);
  else {
    emit error(i18n("Source <placeholder>%1</placeholder> does not exist.", sourcename));
    emit failed(d->m_kmmID, d->m_webID);
    return false;
  }

  QUrl url;

  // if the source has room for TWO symbols..
  if (d->m_source.m_url.contains("%2")) {
    // this is a two-symbol quote.  split the symbol into two.  valid symbol
    // characters are: 0-9, A-Z and the dot.  anything else is a separator
    QRegularExpression splitrx("([0-9a-z\\.]+)[^a-z0-9]+([0-9a-z\\.]+)", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match;
    // if we've truly found 2 symbols delimited this way...
    if (d->m_webID.indexOf(splitrx, 0, &match) != -1) {
      url = QUrl(d->m_source.m_url.arg(match.captured(1), match.captured(2)));
    } else {
      qCDebug(WEBPRICEQUOTE) << "WebPriceQuote::launch() did not find 2 symbols";
    }
  } else {
    // a regular one-symbol quote
    url = QUrl(d->m_source.m_url.arg(d->m_webID));
  }

  if (url.isLocalFile()) {
    emit status(i18nc("The process x is executing", "Executing %1...", url.toLocalFile()));

    QString program;
    QStringList arguments = url.toLocalFile().split(' ', QString::SkipEmptyParts);
    if (!arguments.isEmpty()) {
#ifndef IS_APPIMAGE
        program = arguments.first();
        arguments.removeFirst();
#else
        program = QStringLiteral("/bin/sh");
        arguments.clear();
        arguments << QStringLiteral("-c");
        arguments << url.toLocalFile();
#endif
    }
    d->m_filter.setWebID(d->m_webID);

    d->m_filter.setProcessChannelMode(QProcess::MergedChannels);
    d->m_filter.start(program, arguments);

    if (!d->m_filter.waitForStarted()) {
      emit error(i18n("Unable to launch: %1", url.toLocalFile()));
      slotParseQuote(QString());
    }
  } else {
    //silent download
    emit status(i18n("Fetching URL %1...", url.toDisplayString()));
    QString tmpFile;
    {
      QTemporaryFile tmpFileFile;
      tmpFileFile.setAutoRemove(false);
      if (tmpFileFile.open())
          qDebug() << "created tmpfile";

      tmpFile = tmpFileFile.fileName();
    }
    QFile::remove(tmpFile);
    const QUrl dest = QUrl::fromLocalFile(tmpFile);
    KIO::Scheduler::checkSlaveOnHold(true);
    KIO::Job *job = KIO::file_copy(url, dest, -1, KIO::HideProgressInfo);
    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(downloadResult(KJob*)));
  }
  return true;
}

void WebPriceQuote::downloadCSV(KJob* job)
{
  QString tmpFile = dynamic_cast<KIO::FileCopyJob*>(job)->destUrl().toLocalFile();
  QUrl url = dynamic_cast<KIO::FileCopyJob*>(job)->srcUrl();
  if (!job->error())
  {
    qDebug() << "Downloaded" << tmpFile << "from" << url;
    QFile f(tmpFile);
    if (f.open(QIODevice::ReadOnly)) {
      f.close();
      slotParseCSVQuote(tmpFile);
    } else {
      emit error(i18n("Failed to open downloaded file"));
      slotParseCSVQuote(QString());
    }
  } else {
    emit error(job->errorString());
    slotParseCSVQuote(QString());
  }
}

void WebPriceQuote::downloadResult(KJob* job)
{
  QString tmpFile = dynamic_cast<KIO::FileCopyJob*>(job)->destUrl().toLocalFile();
  QUrl url = dynamic_cast<KIO::FileCopyJob*>(job)->srcUrl();
  if (!job->error())
  {
    qDebug() << "Downloaded" << tmpFile << "from" << url;
    QFile f(tmpFile);
    if (f.open(QIODevice::ReadOnly)) {
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
    QFile::remove(tmpFile);
  } else {
    emit error(job->errorString());
    slotParseQuote(QString());
  }
}

bool WebPriceQuote::launchFinanceQuote(const QString& _webID, const QString& _kmmID,
                                       const QString& _sourcename)
{
  bool result = true;
  d->m_webID = _webID;
  d->m_kmmID = _kmmID;
  QString FQSource = _sourcename.section(' ', 1);
  d->m_source = WebPriceQuoteSource(_sourcename, m_financeQuoteScriptPath, m_financeQuoteScriptPath,
                                    "\"([^,\"]*)\",.*",  // webIDRegExp
                                    WebPriceQuoteSource::identifyBy::Symbol,
                                    "[^,]*,[^,]*,\"([^\"]*)\"", // price regexp
                                    "[^,]*,([^,]*),.*", // date regexp
                                    "%y-%m-%d"); // date format

  //emit status(QString("(Debug) symbol=%1 id=%2...").arg(_symbol,_id));

  QStringList arguments;
  arguments << m_financeQuoteScriptPath << FQSource << KShell::quoteArg(_webID);
  d->m_filter.setWebID(d->m_webID);
  emit status(i18nc("Executing 'script' 'online source' 'investment symbol' ", "Executing %1 %2 %3...", m_financeQuoteScriptPath, FQSource, _webID));

  d->m_filter.setProcessChannelMode(QProcess::MergedChannels);
  d->m_filter.start(QLatin1Literal("perl"), arguments);

  // This seems to work best if we just block until done.
  if (d->m_filter.waitForFinished()) {
    result = true;
  } else {
    emit error(i18n("Unable to launch: %1", m_financeQuoteScriptPath));
    slotParseQuote(QString());
  }

  return result;
}

void WebPriceQuote::slotParseCSVQuote(const QString& filename)
{
  bool isOK = true;
  if (filename.isEmpty())
    isOK = false;
  else {
    MyMoneyStatement st;
    CSVImporterCore* csvImporter = new CSVImporterCore;
    st = csvImporter->unattendedImport(filename, &d->m_CSVSource);
    if (!st.m_listPrices.isEmpty())
      emit csvquote(d->m_kmmID, d->m_webID, st);
    else
      isOK = false;
    delete csvImporter;
    QFile::remove(filename);
  }

  if (!isOK) {
    emit error(i18n("Unable to update price for %1", d->m_webID));
    emit failed(d->m_kmmID, d->m_webID);
  }
}

void WebPriceQuote::slotParseQuote(const QString& _quotedata)
{
  QString quotedata = _quotedata;
  d->m_quoteData = quotedata;

  qCDebug(WEBPRICEQUOTE) << "quotedata" << _quotedata;

  if (! quotedata.isEmpty()) {
    if (!d->m_source.m_skipStripping) {
      // First, remove extraneous non-data elements

      // HTML tags
      quotedata.remove(QRegularExpression("<[^>]*>"));

      // &...;'s
      quotedata.replace(QRegularExpression("&\\w+;"), QLatin1String(" "));

      // Extra white space
      quotedata = quotedata.simplified();
      qCDebug(WEBPRICEQUOTE) << "stripped text" << quotedata;
    }

    QRegularExpression webIDRegExp(d->m_source.m_webID);
    QRegularExpression dateRegExp(d->m_source.m_date);
    QRegularExpression priceRegExp(d->m_source.m_price);
    QRegularExpressionMatch match;

    if (quotedata.indexOf(webIDRegExp, 0, &match) > -1) {
      qCDebug(WEBPRICEQUOTE) << "Identifier" << match.captured(1);
      emit status(i18n("Identifier found: '%1'", match.captured(1)));
    }

    bool gotprice = false;
    bool gotdate = false;

    if (quotedata.indexOf(priceRegExp, 0, &match) > -1) {
      gotprice = true;
      QString pricestr = match.captured(1);

      // Deal with exponential prices
      // we extract the exponent and add it again before we convert to a double
      QRegularExpression expRegExp("[eE][+-]?\\D+");
      int pos;
      QString exponent;
      if ((pos = pricestr.indexOf(expRegExp, 0, &match)) > -1) {
        exponent = pricestr.mid(pos);
        pricestr = pricestr.left(pos);
      }

      // Deal with european quotes that come back as X.XXX,XX or XX,XXX
      //
      // We will make the assumption that ALL prices have a decimal separator.
      // So "1,000" always means 1.0, not 1000.0.
      //
      // Remove all non-digits from the price string except the last one, and
      // set the last one to a period.

      pos = pricestr.lastIndexOf(QRegularExpression("\\D"));
      if (pos > 0) {
        pricestr[pos] = QLatin1Char('.');
        pos = pricestr.lastIndexOf(QRegularExpression("\\D"), pos - 1);
      }
      while (pos > 0) {
        pricestr.remove(pos, 1);
        pos = pricestr.lastIndexOf(QRegularExpression("\\D"), pos);
      }
      pricestr.append(exponent);

      d->m_price = pricestr.toDouble();
      qCDebug(WEBPRICEQUOTE) << "Price" << pricestr;
      emit status(i18n("Price found: '%1' (%2)", pricestr, d->m_price));
    }

    if (quotedata.indexOf(dateRegExp, 0, &match) > -1) {
      QString datestr = match.captured(1);

      MyMoneyDateFormat dateparse(d->m_source.m_dateformat);
      try {
        d->m_date = dateparse.convertString(datestr, false /*strict*/);
        gotdate = true;
        qCDebug(WEBPRICEQUOTE) << "Date" << datestr;
        emit status(i18n("Date found: '%1'", d->m_date.toString()));;
      } catch (const MyMoneyException &) {
        // emit error(i18n("Unable to parse date %1 using format %2: %3").arg(datestr,dateparse.format(),e.what()));
        d->m_date = QDate::currentDate();
        gotdate = true;
      }
    }

    if (gotprice && gotdate) {
      emit quote(d->m_kmmID, d->m_webID, d->m_date, d->m_price);
    } else {
      emit error(i18n("Unable to update price for %1 (no price or no date)", d->m_webID));
      emit failed(d->m_kmmID, d->m_webID);
    }
  } else {
    emit error(i18n("Unable to update price for %1 (empty quote data)", d->m_webID));
    emit failed(d->m_kmmID, d->m_webID);
  }
}

const QMap<QString, PricesProfile> WebPriceQuote::defaultCSVQuoteSources()
{
  QMap<QString, PricesProfile> result;

  // tip: possible delimiter indexes are in csvenums.h

  result[QLatin1String("Stooq")] = PricesProfile(QLatin1String("Stooq"),
                                                 106, 1, 0, DateFormat::YearMonthDay, FieldDelimiter::Semicolon,
                                                 TextDelimiter::DoubleQuote, DecimalSymbol::Dot,
                                                 QMap<Column, int>{{Column::Date, 0}, {Column::Price, 4}},
                                                 2, Profile::StockPrices);

  result[QLatin1String("Stooq Currency")] = PricesProfile(QLatin1String("Stooq Currency"),
                                                          106, 1, 0, DateFormat::YearMonthDay, FieldDelimiter::Semicolon,
                                                          TextDelimiter::DoubleQuote, DecimalSymbol::Dot,
                                                          QMap<Column, int>{{Column::Date, 0}, {Column::Price, 4}},
                                                          2, Profile::CurrencyPrices);

  result[QLatin1String("Yahoo")] = PricesProfile(QLatin1String("Yahoo"),
                                                 106, 1, 0, DateFormat::YearMonthDay, FieldDelimiter::Comma,
                                                 TextDelimiter::DoubleQuote, DecimalSymbol::Dot,
                                                 QMap<Column, int>{{Column::Date, 0}, {Column::Price, 4}},
                                                 2, Profile::StockPrices);

  result[QLatin1String("Nasdaq Baltic - Shares")] = PricesProfile(QLatin1String("Nasdaq Baltic - Shares"),
                                                                  106, 1, 0, DateFormat::DayMonthYear, FieldDelimiter::Tab,
                                                                  TextDelimiter::DoubleQuote, DecimalSymbol::Dot,
                                                                  QMap<Column, int>{{Column::Date, 0}, {Column::Price, 5}},
                                                                  2, Profile::StockPrices);

  result[QLatin1String("Nasdaq Baltic - Funds")] = PricesProfile(QLatin1String("Nasdaq Baltic - Funds"),
                                                                 106, 1, 0, DateFormat::DayMonthYear, FieldDelimiter::Tab,
                                                                 TextDelimiter::DoubleQuote, DecimalSymbol::Dot,
                                                                 QMap<Column, int>{{Column::Date, 0}, {Column::Price, 5}},
                                                                 2, Profile::StockPrices);
  return result;
}

const QMap<QString, WebPriceQuoteSource> WebPriceQuote::defaultQuoteSources()
{
  QMap<QString, WebPriceQuoteSource> result;

  // Use fx-rate.net as the standard currency exchange rate source until
  // we have the capability to use more than one source. Use a neutral
  // name for the source.
  result["KMyMoney Currency"] = WebPriceQuoteSource("KMyMoney Currency",
                                          "https://fx-rate.net/%1/%2",
                                          QString(),
                                          "https://fx-rate.net/([^/]+/[^/]+)",
                                          WebPriceQuoteSource::identifyBy::Symbol,
                                          "1[ a-zA-Z]+=</span><br\\s*/?>\\s*([,\\d+\\.]+)",
                                          "updated\\s\\d+:\\d+:\\d+\\(\\w+\\)\\s+(\\d{1,2}/\\d{2}/\\d{4})",
                                          "%d/%m/%y",
                                          true // skip HTML stripping
                                       );

  // Update on 2017-06 by Łukasz Wojniłowicz
  result["Globe & Mail"] = WebPriceQuoteSource("Globe & Mail",
                                               "http://globefunddb.theglobeandmail.com/gishome/plsql/gis.price_history?pi_fund_id=%1",
                                               QString(),
                                               QString(),  // webIDRegExp
                                               WebPriceQuoteSource::identifyBy::IdentificationNumber,
                                               "Fund Price:\\D+(\\d+\\.\\d+)", // priceregexp
                                               "Fund Price:.+as at (\\w+ \\d+, \\d+)\\)", // dateregexp
                                               "%m %d %y" // dateformat
                                               );

  // Update on 2017-06 by Łukasz Wojniłowicz
  result["MSN"] = WebPriceQuoteSource("MSN",
                                      "http://www.msn.com/en-us/money/stockdetails/%1",
                                      QString(),
                                      QString(),  // webIDRegExp
                                      WebPriceQuoteSource::identifyBy::Symbol,
                                      "(\\d+\\.\\d+) [+-]\\d+.\\d+", // priceregexp
                                      "(\\d+/\\d+/\\d+)", //dateregexp
                                      "%y %m %d" // dateformat
                                      );

  // Finanztreff (replaces VWD.DE) supplied by Michael Zimmerman
  result["Finanztreff"] = WebPriceQuoteSource("Finanztreff",
                                              "http://finanztreff.de/kurse_einzelkurs_detail.htn?u=100&i=%1",
                                              "",
                                              QString(),  // webIDRegExp
                                              WebPriceQuoteSource::identifyBy::IdentificationNumber,
                                              "([0-9]+,\\d+).+Gattung:Fonds", // priceregexp
                                              "\\).(\\d+\\D+\\d+\\D+\\d+)", // dateregexp (doesn't work; date in chart
                                              "%d.%m.%y" // dateformat
                                              );

  // First revision by Michael Zimmerman
  // Update on 2017-06 by Łukasz Wojniłowicz
  result["boerseonlineaktien"] = WebPriceQuoteSource("Börse Online - Aktien",
                                               "http://www.boerse-online.de/aktie/%1-Aktie",
                                               QString(),
                                               QString(),  // webIDRegExp
                                               WebPriceQuoteSource::identifyBy::Name,
                                               "Aktienkurs\\D+(\\d+,\\d{2})", // priceregexp
                                               "Datum (\\d{2}\\.\\d{2}\\.\\d{4})", // dateregexp
                                               "%d.%m.%y" // dateformat
                                               );

  // This quote source provided by e-mail and should replace the previous one:
  // From: David Houlden <djhoulden@gmail.com>
  // To: kmymoney@kde.org
  // Date: Sat, 6 Apr 2013 13:22:45 +0100
  // Updated on 2017-06 by Łukasz Wojniłowicz
  result["Financial Times - UK Funds"] = WebPriceQuoteSource("Financial Times",
                                                           "http://funds.ft.com/uk/Tearsheet/Summary?s=%1",
                                                           QString(),
                                                           QString(), // webIDRegExp
                                                           WebPriceQuoteSource::identifyBy::IdentificationNumber,
                                                           "Price\\D+([\\d,]*\\d+\\.\\d+)", // price regexp
                                                           "Data delayed at least 15 minutes, as of\\ (.*)\\.", // date regexp
                                                           "%m %d %y", // date format
                                                           true // skip HTML stripping
                                                           );

  // The following two price sources were contributed by
  // Marc Zahnlecker <tf2k@users.sourceforge.net>

  result["Wallstreet-Online.DE (Default)"] = WebPriceQuoteSource("Wallstreet-Online.DE (Default)",
                                                                 "http://www.wallstreet-online.de/si/?k=%1&spid=ws",
                                                                 "",
                                                                 "Symbol:(\\w+)",  // webIDRegExp
                                                                 WebPriceQuoteSource::identifyBy::Symbol,
                                                                 "Letzter Kurs: ([0-9.]+,\\d+)", // priceregexp
                                                                 ", (\\d+\\D+\\d+\\D+\\d+)", // dateregexp
                                                                 "%d %m %y" // dateformat
                                                                 );

  // (tf2k) The "mpid" is I think the market place id. In this case five
  // stands for Hamburg.
  //
  // Here the id for several market places: 2 Frankfurt, 3 Berlin, 4
  // Düsseldorf, 5 Hamburg, 6 München/Munich, 7 Hannover, 9 Stuttgart, 10
  // Xetra, 32 NASDAQ, 36 NYSE

  result["Wallstreet-Online.DE (Hamburg)"] = WebPriceQuoteSource("Wallstreet-Online.DE (Hamburg)",
                                                                 "http://fonds.wallstreet-online.de/si/?k=%1&spid=ws&mpid=5",
                                                                 "",
                                                                 "Symbol:(\\w+)",  // webIDRegExp
                                                                 WebPriceQuoteSource::identifyBy::Symbol,
                                                                 "Fonds \\(EUR\\) ([0-9.]+,\\d+)", // priceregexp
                                                                 ", (\\d+\\D+\\d+\\D+\\d+)", // dateregexp
                                                                 "%d %m %y" // dateformat
                                                                 );
  // First revision on 2017-06 by Łukasz Wojniłowicz
  result["Puls Biznesu"] = WebPriceQuoteSource("Puls Biznesu",
                                        "http://notowania.pb.pl/instrument/%1",
                                        QString(),
                                        QString(),                   // webIDRegExp
                                        WebPriceQuoteSource::identifyBy::IdentificationNumber,
                                        "(\\d+,\\d{2})\\D+\\d+,\\d{2}%",    // price regexp
                                        "(\\d{4}-\\d{2}-\\d{2}) \\d{2}:\\d{2}:\\d{2}", // date regexp
                                        "%y %m %d"                   // date format
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

  // Update on 2017-06 by Łukasz Wojniłowicz
  result["Stooq"] = WebPriceQuoteSource("Stooq",
                                        "http://stooq.com/q/?s=%1",
                                        "http://stooq.pl/q/d/l/?s=%1&d1=%y%m%d&d2=%y%m%d&i=d&c=1",
                                        QString(),                   // webIDRegExp
                                        WebPriceQuoteSource::identifyBy::Symbol,
                                        "Last(\\d+\\.\\d+).*Date",    // price regexp
                                        "(\\d{4,4}-\\d{2,2}-\\d{2,2})", // date regexp
                                        "%y %m %d"                   // date format
                                        );

  // First revision on 2017-06 by Łukasz Wojniłowicz
  result[QLatin1String("Stooq Currency")] = WebPriceQuoteSource("Stooq Currency",
                                                                "http://stooq.com/q/?s=%1%2",
                                                                "http://stooq.pl/q/d/l/?s=%1%2&d1=%y%m%d&d2=%y%m%d&i=d&c=1",
                                                                QString(),                   // webIDRegExp
                                                                WebPriceQuoteSource::identifyBy::Symbol,
                                                                "Last.*(\\d+\\.\\d+).*Date",    // price regexp
                                                                "(\\d{4,4}-\\d{2,2}-\\d{2,2})", // date regexp
                                                                "%y %m %d"                   // date format
                                                                );

  // First revision on 2017-06 by Łukasz Wojniłowicz
  result["Nasdaq Baltic - Shares"] = WebPriceQuoteSource("Nasdaq Baltic - Shares",
                                                         "http://www.nasdaqbaltic.com/market/?pg=details&instrument=%1&lang=en",
                                                         "http://www.nasdaqbaltic.com/market/?instrument=%1&pg=details&tab=historical&lang=en&date=&start=%d.%m.%y&end=%d.%m.%y&pg=details&pg2=equity&downloadcsv=1&csv_style=english",
                                                         QString(),  // webIDRegExp
                                                         WebPriceQuoteSource::identifyBy::IdentificationNumber,
                                                         "lastPrice\\D+(\\d+,\\d+)",  // priceregexp
                                                         "as of: (\\d{2}.\\d{2}.\\d{4})",  // dateregexp
                                                         "%d.%m.%y",   // dateformat
                                                         true
                                                         );

  // First revision on 2017-06 by Łukasz Wojniłowicz
  result["Nasdaq Baltic - Funds"] = WebPriceQuoteSource("Nasdaq Baltic - Funds",
                                                        "http://www.nasdaqbaltic.com/market/?pg=details&instrument=%1&lang=en",
                                                        "http://www.nasdaqbaltic.com/market/?instrument=%1&pg=details&tab=historical&lang=en&date=&start=%d.%m.%y&end=%d.%m.%y&pg=details&pg2=equity&downloadcsv=1&csv_style=english",
                                                        QString(),  // webIDRegExp
                                                        WebPriceQuoteSource::identifyBy::IdentificationNumber,
                                                        "marketShareDetailTable(.+\\n){21}\\D+(\\d+)",  // priceregexp
                                                        "as of: (\\d{2}.\\d{2}.\\d{4})",  // dateregexp
                                                        "%d.%m.%y",   // dateformat
                                                        true
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
  KSharedConfigPtr kconfig = KSharedConfig::openConfig();
  QStringList groups = kconfig->groupList();

  QStringList::Iterator it;
  QRegularExpression onlineQuoteSource(QString("^Online-Quote-Source-(.*)$"));
  QRegularExpressionMatch match;

  // get rid of all 'non online quote source' entries
  for (it = groups.begin(); it != groups.end(); it = groups.erase(it)) {
    if ((*it).indexOf(onlineQuoteSource, 0, &match) >= 0) {
      // Insert the name part
      it = groups.insert(it, match.captured(1));
      ++it;
    }
  }

  // if the user has the OLD quote source defined, now is the
  // time to remove that entry and convert it to the new system.
  if (! groups.count() && kconfig->hasGroup("Online Quotes Options")) {
    KConfigGroup grp = kconfig->group("Online Quotes Options");
    QString url(grp.readEntry("URL", "http://finance.yahoo.com/d/quotes.csv?s=%1&f=sl1d1"));
    QString webIDRegExp(grp.readEntry("SymbolRegex", "\"([^,\"]*)\",.*"));
    QString priceRegExp(grp.readEntry("PriceRegex", "[^,]*,([^,]*),.*"));
    QString dateRegExp(grp.readEntry("DateRegex", "[^,]*,[^,]*,\"([^\"]*)\""));
    kconfig->deleteGroup("Online Quotes Options");

    groups += "Old Source";
    grp = kconfig->group(QString(QLatin1String("Online-Quote-Source-%1")).arg("Old Source"));
    grp.writeEntry("URL", url);
    grp.writeEntry("CSVURL", "http://finance.yahoo.com/d/quotes.csv?s=%1&f=sl1d1");
    grp.writeEntry("IDRegex", webIDRegExp);
    grp.writeEntry("PriceRegex", priceRegExp);
    grp.writeEntry("DateRegex", dateRegExp);
    grp.writeEntry("DateFormatRegex", "%m %d %y");
    grp.sync();
  }

  // if the user has OLD quote source based only on symbols (and not ISIN)
  // now is the time to convert it to the new system.
  foreach (const auto group, groups) {
    KConfigGroup grp = kconfig->group(QString(QLatin1String("Online-Quote-Source-%1")).arg(group));
    if (grp.hasKey("SymbolRegex")) {
      grp.writeEntry("IDRegex", grp.readEntry("SymbolRegex"));
      grp.deleteEntry("SymbolRegex");
    } else
      break;
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
      m_financeQuoteScriptPath = QStandardPaths::locate(QStandardPaths::DataLocation, QString("misc/financequote.pl"));
    }
    FinanceQuoteProcess getList;
    getList.launch(m_financeQuoteScriptPath);
    while (!getList.isFinished()) {
      QCoreApplication::processEvents();
    }
    m_financeQuoteSources = getList.getSourceList();
  }
  return (m_financeQuoteSources);
}

//
// Helper class to load/save an individual source
//

WebPriceQuoteSource::WebPriceQuoteSource(const QString& name, const QString& url, const QString &csvUrl, const QString& id, const identifyBy idBy, const QString& price, const QString& date, const QString& dateformat, bool skipStripping):
    m_name(name),
    m_url(url),
    m_csvUrl(csvUrl),
    m_webID(id),
    m_webIDBy(idBy),
    m_price(price),
    m_date(date),
    m_dateformat(dateformat),
    m_skipStripping(skipStripping)
{ }

WebPriceQuoteSource::WebPriceQuoteSource(const QString& name)
{
  m_name = name;
  KSharedConfigPtr kconfig = KSharedConfig::openConfig();
  KConfigGroup grp = kconfig->group(QString("Online-Quote-Source-%1").arg(m_name));
  m_webID = grp.readEntry("IDRegex");
  m_webIDBy = static_cast<WebPriceQuoteSource::identifyBy>(grp.readEntry("IDBy", "0").toInt());
  m_date = grp.readEntry("DateRegex");
  m_dateformat = grp.readEntry("DateFormatRegex", "%m %d %y");
  m_price = grp.readEntry("PriceRegex");
  m_url = grp.readEntry("URL");
  m_csvUrl = grp.readEntry("CSVURL");
  m_skipStripping = grp.readEntry("SkipStripping", false);
}

void WebPriceQuoteSource::write() const
{
  KSharedConfigPtr kconfig = KSharedConfig::openConfig();
  KConfigGroup grp = kconfig->group(QString("Online-Quote-Source-%1").arg(m_name));
  grp.writeEntry("URL", m_url);
  grp.writeEntry("CSVURL", m_csvUrl);
  grp.writeEntry("PriceRegex", m_price);
  grp.writeEntry("DateRegex", m_date);
  grp.writeEntry("DateFormatRegex", m_dateformat);
  grp.writeEntry("IDRegex", m_webID);
  grp.writeEntry("IDBy", static_cast<int>(m_webIDBy));
  if (m_skipStripping)
    grp.writeEntry("SkipStripping", m_skipStripping);
  else
    grp.deleteEntry("SkipStripping");
  kconfig->sync();
}

void WebPriceQuoteSource::rename(const QString& name)
{
  remove();
  m_name = name;
  write();
}

void WebPriceQuoteSource::remove() const
{
  KSharedConfigPtr kconfig = KSharedConfig::openConfig();
  kconfig->deleteGroup(QString("Online-Quote-Source-%1").arg(m_name));
  kconfig->sync();
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
//   qDebug() << "WebPriceQuoteProcess::slotReceivedDataFromFilter(): " << QString(data);
  m_string += QString(readAllStandardOutput());
}

void WebPriceQuoteProcess::slotProcessExited(int /*exitCode*/, QProcess::ExitStatus /*exitStatus*/)
{
//   qDebug() << "WebPriceQuoteProcess::slotProcessExited()";
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

//   qDebug() << "WebPriceQuoteProcess::slotReceivedDataFromFilter(): " << QString(data);
  m_string += QString(data);
}

void FinanceQuoteProcess::slotProcessExited()
{
//   qDebug() << "WebPriceQuoteProcess::slotProcessExited()";
  m_isDone = true;
}

void FinanceQuoteProcess::launch(const QString& scriptPath)
{
  QStringList arguments;
  arguments << scriptPath << QLatin1Literal("-l");
  setProcessChannelMode(QProcess::SeparateChannels);
  start(QLatin1Literal("perl"), arguments);
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

// In 'strict' mode, this is designed to be compatible with the QIF profile date
// converter.  However, that converter deals with the concept of an apostrophe
// format in a way I don't understand.  So for the moment, they are 99%
// compatible, waiting on that issue. (acejones)

const QDate MyMoneyDateFormat::convertString(const QString& _in, bool _strict, unsigned _centurymidpoint) const
{
  //
  // Break date format string into component parts
  //

  QRegularExpression formatrex("%([mdy]+)(\\W+)%([mdy]+)(\\W+)%([mdy]+)", QRegularExpression::CaseInsensitiveOption);
  QRegularExpressionMatch match;
  if (m_format.indexOf(formatrex, 0, &match) == -1) {
    throw MYMONEYEXCEPTION_CSTRING("Invalid format string");
  }

  QStringList formatParts;
  formatParts += match.captured(1);
  formatParts += match.captured(3);
  formatParts += match.captured(5);

  QStringList formatDelimiters;
  formatDelimiters += match.captured(2);
  formatDelimiters += match.captured(4);
  match = QRegularExpressionMatch();

  //
  // Break input string up into component parts,
  // using the delimiters found in the format string
  //

  QRegularExpression inputrex;
  inputrex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);

  // strict mode means we must enforce the delimiters as specified in the
  // format.  non-strict allows any delimiters
  if (_strict)
    inputrex.setPattern(QString("(\\w+)%1(\\w+)%2(\\w+)").arg(formatDelimiters[0], formatDelimiters[1]));
  else
    inputrex.setPattern("(\\w+)\\W+(\\w+)\\W+(\\w+)");

  if (_in.indexOf(inputrex, 0, &match) == -1) {
    throw MYMONEYEXCEPTION_CSTRING("Invalid input string");
  }

  QStringList scannedParts;
  scannedParts += match.captured(1).toLower();
  scannedParts += match.captured(2).toLower();
  scannedParts += match.captured(3).toLower();
  match = QRegularExpressionMatch();

  //
  // Convert the scanned parts into actual date components
  //
  unsigned day = 0, month = 0, year = 0;
  bool ok;
  QRegularExpression digitrex("(\\d+)");
  QStringList::const_iterator it_scanned = scannedParts.constBegin();
  QStringList::const_iterator it_format = formatParts.constBegin();
  while (it_scanned != scannedParts.constEnd()) {
    // decide upon the first character of the part
    switch ((*it_format).at(0).cell()) {
      case 'd':
        // remove any extraneous non-digits (e.g. read "3rd" as 3)
        ok = false;
        if ((*it_scanned).indexOf(digitrex, 0, &match) != -1)
          day = match.captured(1).toUInt(&ok);
        if (!ok || day > 31)
          throw MYMONEYEXCEPTION(QString::fromLatin1("Invalid day entry: %1").arg(*it_scanned));
        break;
      case 'm':
        month = (*it_scanned).toUInt(&ok);
        if (!ok) {
          month = 0;
          // maybe it's a textual date
          unsigned i = 1;
          // search the name in the current selected locale
          QLocale locale;
          while (i <= 12) {
            if (locale.standaloneMonthName(i).toLower() == *it_scanned
                || locale.standaloneMonthName(i, QLocale::ShortFormat).toLower() == *it_scanned) {
              month = i;
              break;
            }
            ++i;
          }
          // in case we did not find the month in the current locale,
          // we look for it in the C locale
          if(month == 0) {
            QLocale localeC(QLocale::C);
            if( !(locale == localeC)) {
              i = 1;
              while (i <= 12) {
                if (localeC.standaloneMonthName(i).toLower() == *it_scanned
                    || localeC.standaloneMonthName(i, QLocale::ShortFormat).toLower() == *it_scanned) {
                  month = i;
                  break;
                }
                ++i;
              }
            }
          }
        }

        if (month < 1 || month > 12)
          throw MYMONEYEXCEPTION(QString::fromLatin1("Invalid month entry: %1").arg(*it_scanned));

        break;
      case 'y':
        if (_strict && (*it_scanned).length() != (*it_format).length())
          throw MYMONEYEXCEPTION(QString::fromLatin1("Length of year (%1) does not match expected length (%2).")
                                 .arg(*it_scanned, *it_format));

        year = (*it_scanned).toUInt(&ok);

        if (!ok)
          throw MYMONEYEXCEPTION(QString::fromLatin1("Invalid year entry: %1").arg(*it_scanned));

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
          throw MYMONEYEXCEPTION(QString::fromLatin1("Invalid year (%1)").arg(year));

        break;
      default:
        throw MYMONEYEXCEPTION_CSTRING("Invalid format character");
    }

    ++it_scanned;
    ++it_format;
  }
  QDate result(year, month, day);
  if (! result.isValid())
    throw MYMONEYEXCEPTION(QString::fromLatin1("Invalid date (yr%1 mo%2 dy%3)").arg(year).arg(month).arg(day));

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
//   qDebug() << "test::QuoteReceiver::slotGetQuote( , " << d << " , " << m.toString() << " )";

  m_price = MyMoneyMoney(m);
  m_date = d;
}

void convertertest::QuoteReceiver::slotStatus(const QString& msg)
{
//   qDebug() << "test::QuoteReceiver::slotStatus( " << msg << " )";

  m_statuses += msg;
}

void convertertest::QuoteReceiver::slotError(const QString& msg)
{
//   qDebug() << "test::QuoteReceiver::slotError( " << msg << " )";

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
