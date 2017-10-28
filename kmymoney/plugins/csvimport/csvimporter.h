/***************************************************************************
                             csvimporter.h
                             -------------------
    begin                       : Sun May 21 2017
    copyright                   : (C) 2015 by Allan Anderson
    email                       : agander93@gmail.com
    copyright                   : (C) 2017 by Łukasz Wojniłowicz
    email                       : lukasz.wojnilowicz@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CSVIMPORTER_H
#define CSVIMPORTER_H

// ----------------------------------------------------------------------------
// KDE Includes
#include <KSharedConfig>

// ----------------------------------------------------------------------------
// QT Includes

#include <QSet>

// Project Includes

#include "mymoneystatement.h"
#include "csvenums.h"
#include "csvimport/kmm_csvimport_core_export.h"
#include "mymoneyenums.h"

class MyMoneyAccount;
class KConfigGroup;
class QStandardItemModel;
class Parse;
class ConvertDate;

enum autodetectTypeE { AutoFieldDelimiter, AutoDecimalSymbol, AutoDateFormat,
                       AutoAccountInvest, AutoAccountBank
                     };

enum miscSettingsE { ConfDirectory, ConfEncoding, ConfDateFormat,
                     ConfFieldDelimiter, ConfTextDelimiter, ConfDecimalSymbol,
                     ConfStartLine, ConfTrailerLines,
                     ConfOppositeSigns,
                     ConfFeeIsPercentage, ConfFeeRate, ConfMinFee,
                     ConfSecurityName, ConfSecuritySymbol, ConfCurrencySymbol,
                     ConfPriceFraction, ConfDontAsk,
                     ConfHeight, ConfWidth
};

enum validationResultE { ValidActionType, InvalidActionValues, NoActionType };


class KMM_CSVIMPORT_CORE_NO_EXPORT CSVProfile
{
protected:
  CSVProfile() {}
  CSVProfile(const QString &profileName, int encodingMIBEnum,
             int startLine, int trailerLines,
             DateFormat dateFormat, FieldDelimiter fieldDelimiter, TextDelimiter textDelimiter, DecimalSymbol decimalSymbol,
             QMap<Column, int> &colTypeNum) :
    m_profileName(profileName), m_encodingMIBEnum(encodingMIBEnum),
    m_startLine(startLine), m_trailerLines(trailerLines),
    m_dateFormat(dateFormat), m_fieldDelimiter(fieldDelimiter),
    m_textDelimiter(textDelimiter), m_decimalSymbol(decimalSymbol),
    m_colTypeNum(colTypeNum)
  {
  initColNumType();
  }
  void readSettings(const KConfigGroup &profilesGroup);
  void writeSettings(KConfigGroup &profilesGroup);
  void initColNumType() {
    for (auto it = m_colTypeNum.constBegin(); it != m_colTypeNum.constEnd(); ++it)
      m_colNumType.insert(it.value(), it.key());
  }

public:
  virtual ~CSVProfile() {}
  virtual Profile type() const = 0;
  virtual bool readSettings(const KSharedConfigPtr &config) = 0;
  virtual void writeSettings(const KSharedConfigPtr &config) = 0;

  QString                   m_profileName;
  QString                   m_lastUsedDirectory;

  int                       m_encodingMIBEnum;

  int                       m_startLine;
  int                       m_endLine;
  int                       m_trailerLines;

  DateFormat                m_dateFormat;
  FieldDelimiter            m_fieldDelimiter;
  TextDelimiter             m_textDelimiter;
  DecimalSymbol             m_decimalSymbol;

  QMap<Column, int>         m_colTypeNum;
  QMap<int, Column>         m_colNumType;
};

class KMM_CSVIMPORT_CORE_EXPORT BankingProfile : public CSVProfile
{
public:
  explicit BankingProfile() : CSVProfile() {}
  BankingProfile(QString profileName, int encodingMIBEnum,
                 int startLine, int trailerLines,
                 DateFormat dateFormat, FieldDelimiter fieldDelimiter, TextDelimiter textDelimiter, DecimalSymbol decimalSymbol,
                 QMap<Column, int> colTypeNum,
                 bool oppositeSigns) :
    CSVProfile(profileName, encodingMIBEnum,
               startLine, trailerLines,
               dateFormat, fieldDelimiter, textDelimiter, decimalSymbol,
               colTypeNum),
    m_oppositeSigns(oppositeSigns) {}


  Profile type() const { return Profile::Banking; }
  bool readSettings(const KSharedConfigPtr &config);
  void writeSettings(const KSharedConfigPtr &config);

  QList<int>       m_memoColList;

  bool             m_oppositeSigns;
};

class KMM_CSVIMPORT_CORE_EXPORT InvestmentProfile : public CSVProfile
{
public:
  explicit InvestmentProfile() : CSVProfile() {}
  InvestmentProfile(QString profileName, int encodingMIBEnum,
                    int startLine, int trailerLines,
                    DateFormat dateFormat, FieldDelimiter fieldDelimiter, TextDelimiter textDelimiter, DecimalSymbol decimalSymbol,
                    QMap<Column, int> colTypeNum,
                    int priceFraction, QMap <MyMoneyStatement::Transaction::EAction, QStringList> transactionNames) :
    CSVProfile(profileName, encodingMIBEnum,
               startLine, trailerLines,
               dateFormat, fieldDelimiter, textDelimiter, decimalSymbol,
               colTypeNum),
    m_transactionNames(transactionNames), m_priceFraction(priceFraction), m_feeIsPercentage(false) {}

  Profile type() const { return Profile::Investment; }
  bool readSettings(const KSharedConfigPtr &config);
  void writeSettings(const KSharedConfigPtr &config);

  QMap <MyMoneyStatement::Transaction::EAction, QStringList> m_transactionNames;

  QString     m_feeRate;
  QString     m_minFee;
  QString     m_securityName;
  QString     m_securitySymbol;

  QList<int>  m_memoColList;

  int         m_priceFraction;
  int         m_dontAsk;

  bool        m_feeIsPercentage;
};

class KMM_CSVIMPORT_CORE_EXPORT PricesProfile : public CSVProfile
{
public:
  explicit PricesProfile() : CSVProfile() {}
  explicit PricesProfile(const Profile profileType) : CSVProfile(), m_profileType(profileType) {}
  PricesProfile(QString profileName, int encodingMIBEnum,
                     int startLine, int trailerLines,
                     DateFormat dateFormat, FieldDelimiter fieldDelimiter, TextDelimiter textDelimiter, DecimalSymbol decimalSymbol,
                     QMap<Column, int> colTypeNum,
                     int priceFraction, Profile profileType) :
    CSVProfile(profileName, encodingMIBEnum,
               startLine, trailerLines,
               dateFormat, fieldDelimiter, textDelimiter, decimalSymbol,
               colTypeNum),
    m_priceFraction(priceFraction), m_profileType(profileType) {}

  Profile type() const { return m_profileType; }
  bool readSettings(const KSharedConfigPtr &config);
  void writeSettings(const KSharedConfigPtr &config);

  QString m_securityName;
  QString m_securitySymbol;
  QString m_currencySymbol;

  int     m_dontAsk;
  int     m_priceFraction;

  Profile m_profileType;
};

class KMM_CSVIMPORT_CORE_EXPORT CSVFile
{
public:
  explicit CSVFile();
  ~CSVFile();

  void getStartEndRow(CSVProfile *profile);
  /**
  * If delimiter = -1 this method tries different field
  * delimiters to get the one with which file has the most columns.
  * Otherwise it gets only column count for specified delimiter.
  */
  void getColumnCount(CSVProfile *profile, const QStringList &rows);

  /**
  * This method gets the filename of
  * the financial statement.
  */
  bool getInFileName(QString startDir = QString());

  void setupParser(CSVProfile *profile);

  /**
  * This method gets file into buffer
  * It will laso store file's end column and row.
  */
  void readFile(CSVProfile *profile);

  Parse              *m_parse;
  QStandardItemModel *m_model;

  QString             m_inFileName;

  int                 m_columnCount;
  int                 m_rowCount;
};

class KMM_CSVIMPORT_CORE_EXPORT CSVImporter
{
public:
  explicit CSVImporter();
  ~CSVImporter();

  /**
  * This method will silently import csv file. Main purpose of this method are online quotes.
  */
  MyMoneyStatement unattendedImport(const QString &filename, CSVProfile *profile);

  static KSharedConfigPtr configFile();
  void profileFactory(const Profile type, const QString &name);
  void readMiscSettings();

  /**
  * This method ensures that configuration file contains all neccesary fields
  * and that it is up to date.
  */
  void validateConfigFile();

  /**
  * This method contains routines to update configuration file
  * from kmmVer to latest.
  */
  bool updateConfigFile(QList<int> &confVer);

  /**
  * This method will update [ProfileNames] in csvimporterrrc
  */
  static bool profilesAction(const Profile type, const ProfileAction action, const QString &name, const QString &newname);

  /**
  * This methods will ensure that fields of input rows are correct.
  */
  bool validateDateFormat(const int col);
  bool validateDecimalSymbols(const QList<int> &columns);
  bool validateCurrencies(const PricesProfile *profile);
  bool validateSecurity(const PricesProfile *profile);
  bool validateSecurity(const InvestmentProfile *profile);
  bool validateSecurities();
  validationResultE validateActionType(MyMoneyStatement::Transaction &tr);

  /**
  * This method will try to detect decimal symbol in input column.
  */
  int detectDecimalSymbols(const QList<int> &columns);
  DecimalSymbol detectDecimalSymbol(const int col, const QString &exclude);

  /**
  * This method will try to detect account from csv header.
  */
  QList<MyMoneyAccount> findAccounts(const QList<eMyMoney::Account> &accountTypes, const QString &statementHeader);
  bool detectAccount(MyMoneyStatement &st);

  /**
  * This methods will evaluate input row and append it to a statement.
  */
  bool processBankRow(MyMoneyStatement &st, const BankingProfile *profile, const int row);
  bool processInvestRow(MyMoneyStatement &st, const InvestmentProfile *profile, const int row);
  bool processPriceRow(MyMoneyStatement &st, const PricesProfile *profile, const int row);

  /**
  * This methods will evaluate fields of input row and return statement's useful value.
  */
  QDate processDateField(const int row, const int col);
  MyMoneyMoney processCreditDebit(QString &credit, QString &debit );
  MyMoneyMoney processPriceField(const InvestmentProfile *profile, const int row, const int col);
  MyMoneyMoney processPriceField(const PricesProfile *profile, const int row, const int col);
  MyMoneyMoney processAmountField(const CSVProfile *profile, const int row, const int col);
  MyMoneyMoney processQuantityField(const CSVProfile *profile, const int row, const int col);
  MyMoneyStatement::Transaction::EAction processActionTypeField(const InvestmentProfile *profile, const int row, const int col);

  /**
  * This method creates valid set of possible transactions
  * according to quantity, amount and price
  */
  QList<MyMoneyStatement::Transaction::EAction> createValidActionTypes(MyMoneyStatement::Transaction &tr);

  /**
  * This method will add fee column to model based on amount and fee rate.
  */
  bool calculateFee();

  /**
  * This method gets securities from investment statement and
  * tries to get pairs of symbol and name either
  * from KMM or from statement data.
  * In case it's not successfull onlySymbols and onlyNames won't be empty.
  */
  bool sortSecurities(QSet<QString>& onlySymbols, QSet<QString>& onlyNames, QMap<QString, QString>& mapSymbolName);

  /**
  * Helper method to set decimal symbol in case it was set to autodetect.
  */
  void setupFieldDecimalSymbol(int col);

  /**
  * Helper method to get all column numbers that were pointed as nummeric
  */
  QList<int> getNumericalColumns();

  bool createStatement(MyMoneyStatement &st);

  ConvertDate                *m_convertDate;
  CSVFile                    *m_file;
  CSVProfile                 *m_profile;
  KSharedConfigPtr            m_config;

  bool                        m_isActionTypeValidated;

  QList<MyMoneyMoney>         m_priceFractions;
  QSet<QString>               m_hashSet;
  QMap<int, DecimalSymbol>    m_decimalSymbolIndexMap;
  QMap<QString, QString>      m_mapSymbolName;
  QMap<autodetectTypeE, bool> m_autodetect;

  static const QHash<Column, QString>                            m_colTypeConfName;
  static const QHash<Profile, QString>                           m_profileConfPrefix;
  static const QHash<MyMoneyStatement::Transaction::EAction, QString> m_transactionConfName;
  static const QHash<miscSettingsE, QString>                          m_miscSettingsConfName;
  static const QString                                                m_confProfileNames;
  static const QString                                                m_confPriorName;
  static const QString                                                m_confMiscName;
};

#endif
