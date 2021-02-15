/*
    SPDX-FileCopyrightText: 2002 Thomas Baumgart <thb@net-bembel.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYQIFPROFILE_H
#define MYMONEYQIFPROFILE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QMap>
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
  explicit MyMoneyQifProfile(const QString& name);
  ~MyMoneyQifProfile();

  inline const QString& profileName() const {
    return m_profileName;
  }
  void setProfileName(const QString& name);

  void loadProfile(const QString& name);
  void saveProfile();

  const QDate date(const QString& datein) const;
  const QString date(const QDate& datein) const;

  const MyMoneyMoney value(const QChar& def, const QString& valuein) const;
  const QString value(const QChar& def, const MyMoneyMoney& valuein) const;

  inline const QString& outputDateFormat() const {
    return m_dateFormat;
  }
  const QString inputDateFormat() const;
  inline const QString& apostropheFormat() const {
    return m_apostropheFormat;
  }
  const QChar amountDecimal(const QChar& def) const;
  const QChar amountThousands(const QChar& def) const;
  inline const QString& profileDescription() const {
    return m_profileDescription;
  }
  inline const QString& profileType() const {
    return m_profileType;
  }
  inline const QString& openingBalanceText() const {
    return m_openingBalanceText;
  }
  const QString accountDelimiter() const;
  inline const QString& voidMark() const {
    return m_voidMark;
  }
  inline const QString& filterScriptImport() const {
    return m_filterScriptImport;
  }
  inline const QString& filterScriptExport() const {
    return m_filterScriptExport;
  }
  inline const QString& filterFileType() const {
    return m_filterFileType;
  }
  inline bool attemptMatchDuplicates() const {
    return m_attemptMatchDuplicates;
  }

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
  void clear();

  /**
    * This method is used to determine, if a profile has been changed or not
    */
  inline bool isDirty() const {
    return m_isDirty;
  };

public Q_SLOTS:
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
  const QString twoDigitYear(const QChar& delim, int yr) const;
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
