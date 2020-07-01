/***************************************************************************
                          webpricequote.h
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

#ifndef WEBPRICEQUOTE_H
#define WEBPRICEQUOTE_H

// ----------------------------------------------------------------------------
// QT Headers

#include <QProcess>
#include <QDate>

// ----------------------------------------------------------------------------
// KDE Headers

// ----------------------------------------------------------------------------
// Project Headers

#include "csv/import/core/csvimportercore.h"

class KJob;
class QDate;
class QTextCodec;
/**
Helper class to attend the process which is running the script, in the case
of a local script being used to fetch the quote.

@author Thomas Baumgart <thb@net-bembel.de> & Ace Jones <acejones@users.sourceforge.net>
*/
class WebPriceQuoteProcess: public QProcess
{
  Q_OBJECT
public:
  WebPriceQuoteProcess();
  inline void setWebID(const QString& _webID) {
    m_webID = _webID; m_string.truncate(0);
  }

public Q_SLOTS:
  void slotReceivedDataFromFilter();
  void slotProcessExited(int exitCode, QProcess::ExitStatus exitStatus);

Q_SIGNALS:
  void processExited(const QString&);

private:
  QString m_webID;
  QString m_string;
};

/**
  * @author Thomas Baumgart & Ace Jones
  *
  * This is a helper class to store information about an online source
  * for stock prices or currency exchange rates.
  */
struct WebPriceQuoteSource {
  enum identifyBy {Symbol, IdentificationNumber, Name};
  WebPriceQuoteSource() : m_webIDBy(Symbol), m_skipStripping(false) {}
  explicit WebPriceQuoteSource(const QString& name);
  WebPriceQuoteSource(const QString& name, const QString& url, const QString& csvUrl, const QString& id, const identifyBy idBy, const QString& price, const QString& date, const QString& dateformat, bool skipStripping = false);
  ~WebPriceQuoteSource() {}

  void write() const;
  void rename(const QString& name);
  void remove() const;

  QString    m_name;
  QString    m_url;
  QString    m_csvUrl;
  QString    m_webID;
  identifyBy m_webIDBy;
  QString    m_price;
  QString    m_date;
  QString    m_dateformat;
  bool       m_skipStripping;
};

/**
Retrieves a price quote from a web-based quote source

@author Ace Jones <acejones@users.sourceforge.net>
*/
class WebPriceQuote: public QObject
{
  Q_OBJECT
public:
  explicit WebPriceQuote(QObject* = 0);
  ~WebPriceQuote();

  void setDate(const QDate& _from, const QDate& _to);
  /**
    * This launches a web-based quote update for the given @p _webID.
    * When the quote is received back from the web source, it will be
    * emitted on the 'quote' signal.
    *
    * @param _webID the identification of the stock to fetch a price for
    * @param _kmmID an arbitrary identifier, which will be emitted in the quote
    *                signal when a price is sent back.
    * @param _source the source of the quote (must be a valid value returned
    *                by quoteSources().  Send QString() to use the default
    *                source.
    * @return bool Whether the quote fetch process was launched successfully
    */

  bool launch(const QString& _webID, const QString& _kmmID, const QString& _source = QString());

  /**
    * This returns a list of the names of the quote sources
    * currently defined.
    *
   * @return QStringList of quote source names
    */
  static const QStringList quoteSources();
  static const QMap<QString, PricesProfile> defaultCSVQuoteSources();

Q_SIGNALS:
  void csvquote(const QString&, const QString&, MyMoneyStatement&);
  void quote(const QString&, const QString&, const QDate&, const double&);
  void failed(const QString&, const QString&);
  void status(const QString&);
  void error(const QString&);

protected Q_SLOTS:
  void slotParseCSVQuote(const QString& filename);
  void slotParseQuote(const QString&);
  void downloadCSV(KJob* job);
  void downloadResult(KJob* job);

protected:
  static const QMap<QString, WebPriceQuoteSource> defaultQuoteSources();

private:
  bool launchCSV(const QString& _webID, const QString& _kmmID, const QString& _source = QString());
  bool launchNative(const QString& _webID, const QString& _kmmID, const QString& _source = QString());
  void enter_loop();

private:
  /// \internal d-pointer class.
  class Private;
  /// \internal d-pointer instance.
  Private* const d;
};

class MyMoneyDateFormat
{
public:
  explicit MyMoneyDateFormat(const QString& _format): m_format(_format) {}
  const QString convertDate(const QDate& _in) const;
  const QDate convertString(const QString& _in, bool _strict = true, unsigned _centurymidpoint = QDate::currentDate().year()) const;
  const QString& format() const {
    return m_format;
  }
private:
  QString m_format;
};

namespace convertertest
{

/**
Simple class to handle signals/slots for unit tests

@author Ace Jones <acejones@users.sourceforge.net>
*/
class QuoteReceiver : public QObject
{
  Q_OBJECT
public:
  explicit QuoteReceiver(WebPriceQuote* q, QObject *parent = 0);
  ~QuoteReceiver();
public Q_SLOTS:
  void slotGetQuote(const QString&, const QString&, const QDate&, const double&);
  void slotStatus(const QString&);
  void slotError(const QString&);
public:
  QStringList m_statuses;
  QStringList m_errors;
  MyMoneyMoney m_price;
  QDate m_date;
};

} // end namespace convertertest


#endif // WEBPRICEQUOTE_H
