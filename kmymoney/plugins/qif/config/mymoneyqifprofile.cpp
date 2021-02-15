/***************************************************************************
                          mymoneyqifprofile.cpp  -  description
                             -------------------
    begin                : Tue Dec 24 2002
    copyright            : (C) 2002 by Thomas Baumgart
    email                : thb@net-bembel.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#include "mymoneyqifprofile.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>
#include <QRegExp>
#include <QVector>
#include <QLocale>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KConfigGroup>
#include <KSharedConfig>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyexception.h"
#include "mymoneymoney.h"
#include "mymoneyenums.h"

/*
 * CENTURY_BREAK is used to identify the century for a two digit year
 *
 * if yr is < CENTURY_BREAK it is in 2000
 * if yr is >= CENTURY_BREAK it is in 1900
 *
 * so with CENTURY_BREAK being 70 the following will happen:
 *
 *  00..69  ->  2000..2069
 *  70..99  ->  1970..1999
 */
const int CENTURY_BREAK = 70;

class MyMoneyQifProfile::Private
{
public:
  Private() :
      m_changeCount(3, 0),
      m_lastValue(3, 0),
      m_largestValue(3, 0) { }

  void getThirdPosition();
  void dissectDate(QVector<QString>& parts, const QString& txt) const;

  QVector<int>    m_changeCount;
  QVector<int>    m_lastValue;
  QVector<int>    m_largestValue;
  QMap<QChar, int>     m_partPos;
};

void MyMoneyQifProfile::Private::dissectDate(QVector<QString>& parts, const QString& txt) const
{
  QRegExp nonDelimChars("[ 0-9a-zA-Z]");
  int part = 0;                 // the current part we scan
  int pos;             // the current scan position
  int maxPartSize = txt.length() > 6 ? 4 : 2;
  // the maximum size of a part
  // some fu... up MS-Money versions write two delimiter in a row
  // so we need to keep track of them. Example: D14/12/'08
  bool lastWasDelim = false;

  // separate the parts of the date and keep the locations of the delimiters
  for (pos = 0; pos < txt.length() && part < 3; ++pos) {
    if (nonDelimChars.indexIn(txt[pos]) == -1) {
      if (!lastWasDelim) {
        ++part;
        maxPartSize = 0;         // make sure to pick the right one depending if next char is numeric or not
        lastWasDelim = true;
      }
    } else {
      lastWasDelim = false;
      // check if the part is over and we did not see a delimiter
      if ((maxPartSize != 0) && (parts[part].length() == maxPartSize)) {
        ++part;
        maxPartSize = 0;
      }
      if (maxPartSize == 0) {
        maxPartSize = txt[pos].isDigit() ? 2 : 3;
        if (part == 2)
          maxPartSize = 4;
      }
      if (part < 3)
        parts[part] += txt[pos];
    }
  }

  if (part == 3) { // invalid date
    for (int i = 0; i < 3; ++i) {
      parts[i] = '0';
    }
  }
}


void MyMoneyQifProfile::Private::getThirdPosition()
{
  // if we have detected two parts we can calculate the third and its position
  if (m_partPos.count() == 2) {
    QList<QChar> partsPresent = m_partPos.keys();
    QStringList partsAvail = QString("d,m,y").split(',');
    int missingIndex = -1;
    int value = 0;
    for (int i = 0; i < 3; ++i) {
      if (!partsPresent.contains(partsAvail[i][0])) {
        missingIndex = i;
      } else {
        value += m_partPos[partsAvail[i][0]];
      }
    }
    m_partPos[partsAvail[missingIndex][0]] = 3 - value;
  }
}



MyMoneyQifProfile::MyMoneyQifProfile() :
    d(new Private),
    m_isDirty(false)
{
  clear();
}

MyMoneyQifProfile::MyMoneyQifProfile(const QString& name) :
    d(new Private),
    m_isDirty(false)
{
  loadProfile(name);
}

MyMoneyQifProfile::~MyMoneyQifProfile()
{
  delete d;
}

void MyMoneyQifProfile::clear()
{
  m_dateFormat = "%d.%m.%yyyy";
  m_apostropheFormat = "2000-2099";
  m_valueMode = "";
  m_filterScriptImport = "";
  m_filterScriptExport = "";
  m_filterFileType = "*.qif *.QIF";

  m_decimal.clear();
  m_decimal['$'] =
    m_decimal['Q'] =
      m_decimal['T'] =
        m_decimal['O'] =
          m_decimal['I'] = QLocale().decimalPoint();

  m_thousands.clear();
  m_thousands['$'] =
    m_thousands['Q'] =
      m_thousands['T'] =
        m_thousands['O'] =
          m_thousands['I'] = QLocale().groupSeparator();

  m_openingBalanceText = "Opening Balance";
  m_voidMark = "VOID ";
  m_accountDelimiter = '[';

  m_profileName = "";
  m_profileDescription = "";
  m_profileType = "Bank";

  m_attemptMatchDuplicates = true;
}

void MyMoneyQifProfile::loadProfile(const QString& name)
{
  KSharedConfigPtr config = KSharedConfig::openConfig();
  KConfigGroup grp = config->group(name);

  clear();

  m_profileName = name;
  m_profileDescription = grp.readEntry("Description", m_profileDescription);
  m_profileType = grp.readEntry("Type", m_profileType);
  m_dateFormat = grp.readEntry("DateFormat", m_dateFormat);
  m_apostropheFormat = grp.readEntry("ApostropheFormat", m_apostropheFormat);
  m_accountDelimiter = grp.readEntry("AccountDelimiter", m_accountDelimiter);
  m_openingBalanceText = grp.readEntry("OpeningBalance", m_openingBalanceText);
  m_voidMark = grp.readEntry("VoidMark", m_voidMark);
  m_filterScriptImport = grp.readEntry("FilterScriptImport", m_filterScriptImport);
  m_filterScriptExport = grp.readEntry("FilterScriptExport", m_filterScriptExport);
  m_filterFileType = grp.readEntry("FilterFileType", m_filterFileType);

  m_attemptMatchDuplicates = grp.readEntry("AttemptMatchDuplicates", m_attemptMatchDuplicates);

  // make sure, we remove any old stuff for now
  grp.deleteEntry("FilterScript");

  QString tmp = QString(m_decimal['Q']) + m_decimal['T'] + m_decimal['I'] +
                m_decimal['$'] + m_decimal['O'];
  tmp = grp.readEntry("Decimal", tmp);
  m_decimal['Q'] = tmp[0];
  m_decimal['T'] = tmp[1];
  m_decimal['I'] = tmp[2];
  m_decimal['$'] = tmp[3];
  m_decimal['O'] = tmp[4];

  tmp = QString(m_thousands['Q']) + m_thousands['T'] + m_thousands['I'] +
        m_thousands['$'] + m_thousands['O'];
  tmp = grp.readEntry("Thousand", tmp);
  m_thousands['Q'] = tmp[0];
  m_thousands['T'] = tmp[1];
  m_thousands['I'] = tmp[2];
  m_thousands['$'] = tmp[3];
  m_thousands['O'] = tmp[4];

  m_isDirty = false;
}

void MyMoneyQifProfile::saveProfile()
{
  if (m_isDirty == true) {
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup grp = config->group(m_profileName);

    grp.writeEntry("Description", m_profileDescription);
    grp.writeEntry("Type", m_profileType);
    grp.writeEntry("DateFormat", m_dateFormat);
    grp.writeEntry("ApostropheFormat", m_apostropheFormat);
    grp.writeEntry("AccountDelimiter", m_accountDelimiter);
    grp.writeEntry("OpeningBalance", m_openingBalanceText);
    grp.writeEntry("VoidMark", m_voidMark);
    grp.writeEntry("FilterScriptImport", m_filterScriptImport);
    grp.writeEntry("FilterScriptExport", m_filterScriptExport);
    grp.writeEntry("FilterFileType", m_filterFileType);
    grp.writeEntry("AttemptMatchDuplicates", m_attemptMatchDuplicates);

    QString tmp;

    tmp = QString(m_decimal['Q']) + m_decimal['T'] + m_decimal['I'] +
          m_decimal['$'] + m_decimal['O'];
    grp.writeEntry("Decimal", tmp);
    tmp = QString(m_thousands['Q']) + m_thousands['T'] + m_thousands['I'] +
          m_thousands['$'] + m_thousands['O'];
    grp.writeEntry("Thousand", tmp);
  }
  m_isDirty = false;
}

void MyMoneyQifProfile::setProfileName(const QString& name)
{
  if (m_profileName != name)
    m_isDirty = true;

  m_profileName = name;
}

void MyMoneyQifProfile::setProfileDescription(const QString& desc)
{
  if (m_profileDescription != desc)
    m_isDirty = true;

  m_profileDescription = desc;
}

void MyMoneyQifProfile::setProfileType(const QString& type)
{
  if (m_profileType != type)
    m_isDirty = true;
  m_profileType = type;
}

void MyMoneyQifProfile::setOutputDateFormat(const QString& dateFormat)
{
  if (m_dateFormat != dateFormat)
    m_isDirty = true;

  m_dateFormat = dateFormat;
}

void MyMoneyQifProfile::setInputDateFormat(const QString& dateFormat)
{
  int j = -1;
  if (dateFormat.length() > 0) {
    for (int i = 0; i < dateFormat.length() - 1; ++i) {
      if (dateFormat[i] == '%') {
        d->m_partPos[dateFormat[++i]] = ++j;
      }
    }
  }
}

void MyMoneyQifProfile::setApostropheFormat(const QString& apostropheFormat)
{
  if (m_apostropheFormat != apostropheFormat)
    m_isDirty = true;

  m_apostropheFormat = apostropheFormat;
}

void MyMoneyQifProfile::setAmountDecimal(const QChar& def, const QChar& chr)
{
  QChar ch(chr);
  if (ch == QChar())
    ch = ' ';

  if (m_decimal[def] != ch)
    m_isDirty = true;

  m_decimal[def] = ch;
}

void MyMoneyQifProfile::setAmountThousands(const QChar& def, const QChar& chr)
{
  QChar ch(chr);
  if (ch == QChar())
    ch = ' ';

  if (m_thousands[def] != ch)
    m_isDirty = true;

  m_thousands[def] = ch;
}

const QChar MyMoneyQifProfile::amountDecimal(const QChar& def) const
{
  QChar chr = m_decimal[def];
  return chr;
}

const QChar MyMoneyQifProfile::amountThousands(const QChar& def) const
{
  QChar chr = m_thousands[def];
  return chr;
}

void MyMoneyQifProfile::setAccountDelimiter(const QString& delim)
{
  QString txt(delim);

  if (txt.isEmpty())
    txt = ' ';
  else if (txt[0] != '[')
    txt = '[';

  if (m_accountDelimiter[0] != txt[0])
    m_isDirty = true;
  m_accountDelimiter = txt[0];
}

void MyMoneyQifProfile::setOpeningBalanceText(const QString& txt)
{
  if (m_openingBalanceText != txt)
    m_isDirty = true;
  m_openingBalanceText = txt;
}

void MyMoneyQifProfile::setVoidMark(const QString& txt)
{
  if (m_voidMark != txt)
    m_isDirty = true;
  m_voidMark = txt;
}

const QString MyMoneyQifProfile::accountDelimiter() const
{
  QString rc;
  if (m_accountDelimiter[0] == ' ')
    rc = ' ';
  else
    rc = "[]";
  return rc;
}

const QString MyMoneyQifProfile::date(const QDate& datein) const
{
  QString::const_iterator format = m_dateFormat.begin();;
  QString buffer;
  QChar delim;
  int maskLen;
  QChar maskChar;

  while (format != m_dateFormat.end()) {
    if (*format == '%') {
      maskLen = 0;
      maskChar = *(++format);
      while ((format != m_dateFormat.end()) && (*format == maskChar)) {
        ++maskLen;
        ++format;
      }

      if (maskChar == 'd') {
        if (! delim.isNull())
          buffer += delim;
        buffer += QString::number(datein.day()).rightJustified(2, '0');

      } else if (maskChar == 'm') {
        if (! delim.isNull())
          buffer += delim;
        if (maskLen == 3)
          buffer += QLocale().monthName(datein.month(), QLocale::ShortFormat);
        else
          buffer += QString::number(datein.month()).rightJustified(2, '0');
      } else if (maskChar == 'y') {
        if (maskLen == 2) {
          buffer += twoDigitYear(delim, datein.year());
        } else {
          if (! delim.isNull())
            buffer += delim;
          buffer += QString::number(datein.year());
        }
        break;
      } else {
        throw MYMONEYEXCEPTION_CSTRING("Invalid char in QifProfile date field");
      }
      delim = 0;
    } else {
      if (! delim.isNull())
        buffer += delim;
      delim = *format++;
    }
  }
  return buffer;
}

const QDate MyMoneyQifProfile::date(const QString& datein) const
{
  // in case we don't know the format, we return an invalid date
  if (d->m_partPos.count() != 3)
    return QDate();

  QVector<QString> scannedParts(3);
  d->dissectDate(scannedParts, datein);

  int yr, mon, day;
  bool ok;
  yr = scannedParts[d->m_partPos['y']].toInt();
  mon = scannedParts[d->m_partPos['m']].toInt(&ok);
  if (!ok) {
    QStringList monthNames = QString("jan,feb,mar,apr,may,jun,jul,aug,sep,oct,nov,dec").split(',');
    int j;
    for (j = 1; j <= 12; ++j) {
      if ((QLocale().monthName(j, QLocale::ShortFormat).toLower() == scannedParts[d->m_partPos['m']].toLower())
          || (monthNames[j-1] == scannedParts[d->m_partPos['m']].toLower())) {
        mon = j;
        break;
      }
    }
    if (j == 13) {
      qWarning("Unknown month '%s'", qPrintable(scannedParts[d->m_partPos['m']]));
      return QDate();
    }
  }

  day = scannedParts[d->m_partPos['d']].toInt();
  if (yr < 100) { // two digit year information?
    if (yr < CENTURY_BREAK)  // less than the CENTURY_BREAK we assume this century
      yr += 2000;
    else
      yr += 1900;
  }
  return QDate(yr, mon, day);

#if 0
  QString scannedDelim[2];
  QString formatParts[3];
  QString formatDelim[2];
  int part;
  int delim;
  unsigned int i, j;

  part = -1;
  delim = 0;
  for (i = 0; i < m_dateFormat.length(); ++i) {
    if (m_dateFormat[i] == '%') {
      ++part;
      if (part == 3) {
        qWarning("MyMoneyQifProfile::date(const QString& datein) Too many parts in date format");
        return QDate();
      }
      ++i;
    }
    switch (m_dateFormat[i].toLatin1()) {
      case 'm':
      case 'd':
      case 'y':
        formatParts[part] += m_dateFormat[i];
        break;
      case '/':
      case '-':
      case '.':
      case '\'':
        if (delim == 2) {
          qWarning("MyMoneyQifProfile::date(const QString& datein) Too many delimiters in date format");
          return QDate();
        }
        formatDelim[delim] = m_dateFormat[i];
        ++delim;
        break;
      default:
        qWarning("MyMoneyQifProfile::date(const QString& datein) Invalid char in date format");
        return QDate();
    }
  }


  part = 0;
  delim = 0;
  bool prevWasChar = false;
  for (i = 0; i < datein.length(); ++i) {
    switch (datein[i].toLatin1()) {
      case '/':
      case '.':
      case '-':
      case '\'':
        if (delim == 2) {
          qWarning("MyMoneyQifProfile::date(const QString& datein) Too many delimiters in date field");
          return QDate();
        }
        scannedDelim[delim] = datein[i];
        ++delim;
        ++part;
        prevWasChar = false;
        break;

      default:
        if (prevWasChar && datein[i].isDigit()) {
          ++part;
          prevWasChar = false;
        }
        if (datein[i].isLetter())
          prevWasChar = true;
        // replace blank with 0
        scannedParts[part] += (datein[i] == ' ') ? QChar('0') : datein[i];
        break;
    }
  }

  int day = 1,
            mon = 1,
                  yr = 1900;
  bool ok = false;
  for (i = 0; i < 2; ++i) {
    if (scannedDelim[i] != formatDelim[i]
        && scannedDelim[i] != QChar('\'')) {
      qWarning("MyMoneyQifProfile::date(const QString& datein) Invalid delimiter '%s' when '%s' was expected",
               scannedDelim[i].toLatin1(), formatDelim[i].toLatin1());
      return QDate();
    }
  }

  QString msg;
  for (i = 0; i < 3; ++i) {
    switch (formatParts[i][0].toLatin1()) {
      case 'd':
        day = scannedParts[i].toUInt(&ok);
        if (!ok)
          msg = "Invalid numeric character in day string";
        break;
      case 'm':
        if (formatParts[i].length() != 3) {
          mon = scannedParts[i].toUInt(&ok);
          if (!ok)
            msg = "Invalid numeric character in month string";
        } else {
          for (j = 1; j <= 12; ++j) {
            if (QLocale().monthName(j, 2000, true).toLower() == formatParts[i].toLower()) {
              mon = j;
              ok = true;
              break;
            }
          }
          if (j == 13) {
            msg = "Unknown month '" + scannedParts[i] + "'";
          }
        }
        break;
      case 'y':
        ok = false;
        if (scannedParts[i].length() == formatParts[i].length()) {
          yr = scannedParts[i].toUInt(&ok);
          if (!ok)
            msg = "Invalid numeric character in month string";
          if (yr < 100) {     // two digit year info
            if (i > 1) {
              ok = true;
              if (scannedDelim[i-1] == QChar('\'')) {
                if (m_apostropheFormat == "1900-1949") {
                  if (yr < 50)
                    yr += 1900;
                  else
                    yr += 2000;
                } else if (m_apostropheFormat == "1900-1999") {
                  yr += 1900;
                } else if (m_apostropheFormat == "2000-2099") {
                  yr += 2000;
                } else {
                  msg = "Unsupported apostropheFormat!";
                  ok = false;
                }
              } else {
                if (m_apostropheFormat == "1900-1949") {
                  if (yr < 50)
                    yr += 2000;
                  else
                    yr += 1900;
                } else if (m_apostropheFormat == "1900-1999") {
                  yr += 2000;
                } else if (m_apostropheFormat == "2000-2099") {
                  yr += 1900;
                } else {
                  msg = "Unsupported apostropheFormat!";
                  ok = false;
                }
              }
            } else {
              msg = "Year as first parameter is not supported!";
            }
          } else if (yr < 1900) {
            msg = "Year not in range < 100 or >= 1900!";
          } else {
            ok = true;
          }
        } else {
          msg = QString("Length of year (%1) does not match expected length (%2).")
                .arg(scannedParts[i].length()).arg(formatParts[i].length());
        }
        break;
    }
    if (!msg.isEmpty()) {
      qWarning("MyMoneyQifProfile::date(const QString& datein) %s", msg.toLatin1());
      return QDate();
    }
  }
  return QDate(yr, mon, day);
#endif
}

const QString MyMoneyQifProfile::twoDigitYear(const QChar& delim, int yr) const
{
  QChar realDelim = delim;
  QString buffer;
  if (!delim.isNull()) {
    if ((m_apostropheFormat == "1900-1949" && yr <= 1949)
        || (m_apostropheFormat == "1900-1999" && yr <= 1999)
        || (m_apostropheFormat == "2000-2099" && yr >= 2000))
      realDelim = '\'';
    buffer += realDelim;
  }
  yr -= 1900;
  if (yr > 100)
    yr -= 100;

  if (yr < 10)
    buffer += '0';

  buffer += QString::number(yr);
  return buffer;
}

const QString MyMoneyQifProfile::value(const QChar& def, const MyMoneyMoney& valuein) const
{
  QChar _decimalSeparator;
  QChar _thousandsSeparator;
  QString res;

  _decimalSeparator = MyMoneyMoney::decimalSeparator();
  _thousandsSeparator = MyMoneyMoney::thousandSeparator();
  eMyMoney::Money::signPosition _signPosition = MyMoneyMoney::negativeMonetarySignPosition();

  MyMoneyMoney::setDecimalSeparator(amountDecimal(def).toLatin1());
  MyMoneyMoney::setThousandSeparator(amountThousands(def).toLatin1());
  MyMoneyMoney::setNegativeMonetarySignPosition(eMyMoney::Money::PreceedQuantityAndSymbol);
  res = valuein.formatMoney("", 2);

  MyMoneyMoney::setDecimalSeparator(_decimalSeparator);
  MyMoneyMoney::setThousandSeparator(_thousandsSeparator);
  MyMoneyMoney::setNegativeMonetarySignPosition(_signPosition);

  return res;
}

const MyMoneyMoney MyMoneyQifProfile::value(const QChar& def, const QString& valuein) const
{
  QChar _decimalSeparator;
  QChar _thousandsSeparator;
  MyMoneyMoney res;

  _decimalSeparator = MyMoneyMoney::decimalSeparator();
  _thousandsSeparator = MyMoneyMoney::thousandSeparator();
  eMyMoney::Money::signPosition _signPosition = MyMoneyMoney::negativeMonetarySignPosition();

  MyMoneyMoney::setDecimalSeparator(amountDecimal(def).toLatin1());
  MyMoneyMoney::setThousandSeparator(amountThousands(def).toLatin1());
  MyMoneyMoney::setNegativeMonetarySignPosition(eMyMoney::Money::PreceedQuantityAndSymbol);

  res = MyMoneyMoney(valuein);

  MyMoneyMoney::setDecimalSeparator(_decimalSeparator);
  MyMoneyMoney::setThousandSeparator(_thousandsSeparator);
  MyMoneyMoney::setNegativeMonetarySignPosition(_signPosition);

  return res;
}

void MyMoneyQifProfile::setFilterScriptImport(const QString& script)
{
  if (m_filterScriptImport != script)
    m_isDirty = true;

  m_filterScriptImport = script;
}

void MyMoneyQifProfile::setFilterScriptExport(const QString& script)
{
  if (m_filterScriptExport != script)
    m_isDirty = true;

  m_filterScriptExport = script;
}

void MyMoneyQifProfile::setFilterFileType(const QString& txt)
{
  if (m_filterFileType != txt)
    m_isDirty = true;

  m_filterFileType = txt;
}

void MyMoneyQifProfile::setAttemptMatchDuplicates(bool f)
{
  if (m_attemptMatchDuplicates != f)
    m_isDirty = true;

  m_attemptMatchDuplicates = f;
}

const QString MyMoneyQifProfile::inputDateFormat() const
{
  QStringList list;
  possibleDateFormats(list);
  if (list.count() == 1)
    return list.first();
  return QString();
}

void MyMoneyQifProfile::possibleDateFormats(QStringList& list) const
{
  QStringList defaultList = QString("y,m,d:y,d,m:m,d,y:m,y,d:d,m,y:d,y,m").split(':');
  list.clear();
  QStringList::const_iterator it_d;
  for (it_d = defaultList.constBegin(); it_d != defaultList.constEnd(); ++it_d) {
    const QStringList parts = (*it_d).split(',', QString::SkipEmptyParts);
    int i;
    for (i = 0; i < 3; ++i) {
      if (d->m_partPos.contains(parts[i][0])) {
        if (d->m_partPos[parts[i][0]] != i)
          break;
      }
      // months can't be larger than 12
      if (parts[i] == "m" && d->m_largestValue[i] > 12)
        break;
      // days can't be larger than 31
      if (parts[i] == "d" && d->m_largestValue[i] > 31)
        break;
    }
    // matches all tests
    if (i == 3) {
      QString format = *it_d;
      format.replace('y', "%y");
      format.replace('m', "%m");
      format.replace('d', "%d");
      format.replace(',', " ");
      list << format;
    }
  }
  // if we haven't found any, then there's something wrong.
  // in this case, we present the full list and let the user decide
  if (list.count() == 0) {
    for (it_d = defaultList.constBegin(); it_d != defaultList.constEnd(); ++it_d) {
      QString format = *it_d;
      format.replace('y', "%y");
      format.replace('m', "%m");
      format.replace('d', "%d");
      format.replace(',', " ");
      list << format;
    }
  }
}

void MyMoneyQifProfile::autoDetect(const QStringList& lines)
{
  m_dateFormat.clear();
  m_decimal.clear();
  m_thousands.clear();

  QString numericRecords = "BT$OIQ";
  QStringList::const_iterator it;
  int datesScanned = 0;
  // section: used to switch between different QIF sections,
  // because the Record identifiers are ambiguous between sections
  // eg. in transaction records, T identifies a total amount, in
  // account sections it's the type.
  //
  // 0 - unknown
  // 1 - account
  // 2 - transactions
  // 3 - prices
  int section = 0;
  QRegExp price("\"(.*)\",(.*),\"(.*)\"");
  for (it = lines.begin(); it != lines.end(); ++it) {
    QChar c((*it)[0]);
    if (c == '!') {
      QString sname = (*it).toLower();
      if (!sname.startsWith(QLatin1String("!option:"))) {
        section = 0;
        if (sname.startsWith(QLatin1String("!account")))
          section = 1;
        else if (sname.startsWith(QLatin1String("!type"))) {
          if (sname.startsWith(QLatin1String("!type:cat"))
              || sname.startsWith(QLatin1String("!type:payee"))
              || sname.startsWith(QLatin1String("!type:security"))
              || sname.startsWith(QLatin1String("!type:class"))) {
            section = 0;
          } else if (sname.startsWith(QLatin1String("!type:price"))) {
            section = 3;
          } else
            section = 2;
        }
      }
    }

    switch (section) {
      case 1:
        if (c == 'B') {
          scanNumeric((*it).mid(1), m_decimal[c], m_thousands[c]);
        }
        break;
      case 2:
        if (numericRecords.contains(c)) {
          scanNumeric((*it).mid(1), m_decimal[c], m_thousands[c]);
        } else if ((c == 'D') && (m_dateFormat.isEmpty())) {
          if (d->m_partPos.count() != 3) {
            scanDate((*it).mid(1));
            ++datesScanned;
            if (d->m_partPos.count() == 2) {
              // if we have detected two parts we can calculate the third and its position
              d->getThirdPosition();
            }
          }
        }
        break;
      case 3:
        if (price.indexIn(*it) != -1) {
          scanNumeric(price.cap(2), m_decimal['P'], m_thousands['P']);
          scanDate(price.cap(3));
          ++datesScanned;
        }
        break;
    }
  }

  // the following algorithm is only applied if we have more
  // than 20 dates found. Smaller numbers have shown that the
  // results are inaccurate which leads to a reduced number of
  // date formats presented to choose from.
  if (d->m_partPos.count() != 3 && datesScanned > 20) {
    QMap<int, int> sortedPos;
    // make sure to reset the known parts for the following algorithm
    if (d->m_partPos.contains('y')) {
      d->m_changeCount[d->m_partPos['y']] = -1;
      for (int i = 0; i < 3; ++i) {
        if (d->m_partPos['y'] == i)
          continue;
        // can we say for sure that we hit the day field?
        if (d->m_largestValue[i] > 12) {
          d->m_partPos['d'] = i;
        }
      }
    }
    if (d->m_partPos.contains('d'))
      d->m_changeCount[d->m_partPos['d']] = -1;
    if (d->m_partPos.contains('m'))
      d->m_changeCount[d->m_partPos['m']] = -1;

    for (int i = 0; i < 3; ++i) {
      if (d->m_changeCount[i] != -1) {
        sortedPos[d->m_changeCount[i]] = i;
      }
    }

    QMap<int, int>::const_iterator it_a;
    QMap<int, int>::const_iterator it_b;
    switch (sortedPos.count()) {
      case 1: // all the same
        // let the user decide, we can't figure it out
        break;

      case 2: // two are the same, we treat the largest as the day
        // if it's 20% larger than the other one and let the
        // user pick the other two
        {
          it_b = sortedPos.constBegin();
          it_a = it_b;
          ++it_b;
          double a = d->m_changeCount[*it_a];
          double b = d->m_changeCount[*it_b];
          if (b > (a * 1.2)) {
            d->m_partPos['d'] = *it_b;
          }
        }
        break;

      case 3: // three different, we check if they are 20% apart each
        it_b = sortedPos.constBegin();
        for (int i = 0; i < 2; ++i) {
          it_a = it_b;
          ++it_b;
          double a = d->m_changeCount[*it_a];
          double b = d->m_changeCount[*it_b];
          if (b > (a * 1.2)) {
            switch (i) {
              case 0:
                d->m_partPos['y'] = *it_a;
                break;
              case 1:
                d->m_partPos['d'] = *it_b;
                break;
            }
          }
        }
        break;
    }
    // extract the last if necessary and possible date position
    d->getThirdPosition();
  }
}

void MyMoneyQifProfile::scanNumeric(const QString& txt, QChar& decimal, QChar& thousands) const
{
  QChar first, second;
  QRegExp numericChars("[0-9-()]");
  for (int i = 0; i < txt.length(); ++i) {
    const QChar& c = txt[i];
    if (numericChars.indexIn(c) == -1) {
      if (c == '.' || c == ',') {
        first = second;
        second = c;
      }
    }
  }
  if (!second.isNull())
    decimal = second;
  if (!first.isNull())
    thousands = first;
}

void MyMoneyQifProfile::scanDate(const QString& txt) const
{
  // extract the parts from the txt
  QVector<QString> parts(3);             // the various parts of the date
  d->dissectDate(parts, txt);

  // now analyze the parts
  for (int i = 0; i < 3; ++i) {
    bool ok;
    int value = parts[i].toInt(&ok);
    if (!ok) { // this should happen only if the part is non-numeric -> month
      d->m_partPos['m'] = i;
    } else if (value != 0) {
      if (value != d->m_lastValue[i]) {
        d->m_changeCount[i]++;
        d->m_lastValue[i] = value;
        if (value > d->m_largestValue[i])
          d->m_largestValue[i] = value;
      }
      // if it's > 31 it can only be years
      if (value > 31) {
        d->m_partPos['y'] = i;
      }
      // and if it's in between 12 and 32 and we already identified the
      // position for the year it must be days
      if ((value > 12) && (value < 32) && d->m_partPos.contains('y')) {
        d->m_partPos['d'] = i;
      }
    }
  }
}
