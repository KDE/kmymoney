/***************************************************************************
                          mymoneyqifprofile.h  -  description
                             -------------------
    begin                : Tue Dec 24 2002
    copyright            : (C) 2002 by Thomas Baumgart
    email                : thb@net-bembel.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYQIFPROFILE_H
#define MYMONEYQIFPROFILE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QObject>
#include <QString>
class QDate;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyMoney;

/**
  * @author Thomas Baumgart
  */

class MyMoneyQifProfile : public QObject
{
  Q_OBJECT

public:
  MyMoneyQifProfile();
  MyMoneyQifProfile(const QString& name);
  ~MyMoneyQifProfile();

  const QString& profileName(void) const { return m_profileName; }
  void setProfileName(const QString& name);

  void loadProfile(const QString& name);
  void saveProfile(void);

  const QDate date(const QString& datein) const;
  QString date(const QDate& datein) const;

  MyMoneyMoney value(const QChar& def, const QString& valuein) const;
  QString value(const QChar& def, const MyMoneyMoney& valuein) const;

  const QString& outputDateFormat(void) const { return m_dateFormat; }
  QString inputDateFormat(void) const;
  const QString& apostropheFormat(void) const { return m_apostropheFormat; }
  QChar amountDecimal(const QChar& def) const;
  QChar amountThousands(const QChar& def) const;
  const QString& profileDescription(void) const { return m_profileDescription; }
  const QString& profileType(void) const { return m_profileType; }
  const QString& openingBalanceText(void) const { return m_openingBalanceText; }
  QString accountDelimiter(void) const;
  const QString& voidMark(void) const { return m_voidMark; }
  const QString& filterScriptImport(void) const { return m_filterScriptImport; }
  const QString& filterScriptExport(void) const { return m_filterScriptExport; }
  const QString& filterFileType(void) const { return m_filterFileType; }
  bool attemptMatchDuplicates(void) const { return m_attemptMatchDuplicates; }

  /**
   * This method scans all strings contained in @a lines and tries to figure
   * out the settings for m_decimal, m_thousands and m_dateFormat
   */
  void autoDetect(const QStringList& lines);

  /**
   * This method returns a list of possible date formats the user
   * can choose from. If autoDetect() has not been run, the @a list
   * contains all possible date formats, in the other case, the @a list
   * is adjusted to those that will match the data scanned.
   */
  void possibleDateFormats(QStringList& list) const;

  /**
    * This method presets the member variables with the default values.
    */
  void clear(void);

  /**
    * This method is used to determine, if a profile has been changed or not
    */
  bool isDirty(void) const { return m_isDirty; };

public slots:
  void setProfileDescription(const QString& desc);
  void setProfileType(const QString& type);
  void setOutputDateFormat(const QString& dateFormat);
  void setInputDateFormat(const QString& dateFormat);
  void setApostropheFormat(const QString& apostropheFormat);
  void setAmountDecimal(const QChar& def, const QChar& chr);
  void setAmountThousands(const QChar& def, const QChar& chr);
  void setAccountDelimiter(const QString& delim);
  void setOpeningBalanceText(const QString& text);
  void setVoidMark(const QString& txt);
  void setFilterScriptImport(const QString& txt);
  void setFilterScriptExport(const QString& txt);
  void setFilterFileType(const QString& txt);
  void setAttemptMatchDuplicates(bool);

private:
  QString twoDigitYear(const QChar delim, int yr) const;
  void scanNumeric(const QString& txt, QChar& decimal, QChar& thousands) const;
  void scanDate(const QString& txt) const;

private:
  /// \internal d-pointer class.
  class Private;
  /// \internal d-pointer instance.
  Private* const d;
  bool      m_isDirty;
  QString   m_profileName;
  QString   m_profileDescription;
  QString   m_dateFormat;
  QString   m_apostropheFormat;
  QString   m_valueMode;
  QString   m_profileType;
  QString   m_openingBalanceText;
  QString   m_voidMark;
  QString   m_accountDelimiter;
  QString   m_filterScriptImport;
  QString   m_filterScriptExport;
  QString   m_filterFileType;  /*< The kind of input files the filter will expect, e.g. "*.qif" */
  QMap<QChar, QChar> m_decimal;
  QMap<QChar, QChar> m_thousands;
  bool      m_attemptMatchDuplicates;
};

#endif
