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
#include <KConfigGroup>

// ----------------------------------------------------------------------------
// QT Includes

#include <QStandardItemModel>

// Project Includes

#include "convdate.h"
#include "csvutil.h"
#include "mymoneystatement.h"

#include <QWizardPage>

class CSVWizard;
class CSVImporter;

class CSVWizardPage : public QWizardPage
{
public:
  CSVWizardPage(CSVWizard *dlg, CSVImporter *imp) : QWizardPage(nullptr), m_dlg(dlg), m_imp(imp) {}

protected:
  CSVWizard   *m_dlg;
  CSVImporter *m_imp;
};

enum profileTypeE { ProfileBank, ProfileInvest,
                    ProfileCurrencyPrices, ProfileStockPrices
                  };

enum profilesActionE { ProfilesAdd, ProfilesRemove, ProfilesRename, ProfilesUpdateLastUsed };

enum autodetectTypeE { AutoFieldDelimiter, AutoDecimalSymbol, AutoDateFormat,
                       AutoAccountInvest, AutoAccountBank
                     };

enum columnTypeE { ColumnDate, ColumnMemo,
                   ColumnNumber, ColumnPayee, ColumnAmount,
                   ColumnCredit, ColumnDebit, ColumnCategory,
                   ColumnType, ColumnPrice, ColumnQuantity,
                   ColumnFee, ColumnSymbol, ColumnName,
                   ColumnEmpty = 0xFE, ColumnInvalid = 0xFF
                 };

enum miscSettingsE { ConfDirectory, ConfEncoding, ConfDateFormat,
                     ConfFieldDelimiter, ConfTextDeimiter, ConfDecimalSymbol,
                     ConfStartLine, ConfTrailerLines,
                     ConfOppositeSigns,
                     ConfFeeIsPercentage, ConfFeeRate, ConfMinFee,
                     ConfSecurityName, ConfSecuritySymbol, ConfCurrencySymbol,
                     ConfPriceFraction, ConfDontAsk,
                     ConfHeight, ConfWidth
};

enum validationResultE { ValidActionType, InvalidActionValues, NoActionType };


class CSVProfile
{
protected:
  CSVProfile() {}
  CSVProfile(const QString &profileName, int encodingMIBEnum,
             int startLine, int trailerLines,
             int dateFormatIndex, int fieldDelimiterIndex, int textDelimiterIndex, int decimalSymbolIndex,
             QMap<columnTypeE, int> &colTypeNum) :
    m_profileName(profileName), m_encodingMIBEnum(encodingMIBEnum),
    m_startLine(startLine), m_trailerLines(trailerLines),
    m_dateFormatIndex(dateFormatIndex), m_fieldDelimiterIndex(fieldDelimiterIndex),
    m_textDelimiterIndex(textDelimiterIndex), m_decimalSymbolIndex(decimalSymbolIndex),
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
  virtual profileTypeE type() const = 0;
  virtual bool readSettings(const KSharedConfigPtr &config) = 0;
  virtual void writeSettings(const KSharedConfigPtr &config) = 0;

  QString                   m_profileName;
  QString                   m_lastUsedDirectory;

  int                       m_encodingMIBEnum;

  int                       m_startLine;
  int                       m_endLine;
  int                       m_trailerLines;

  int                       m_dateFormatIndex;
  int                       m_fieldDelimiterIndex;
  int                       m_textDelimiterIndex;
  int                       m_decimalSymbolIndex;

  QMap<columnTypeE, int>    m_colTypeNum;
  QMap<int, columnTypeE>    m_colNumType;
};

class BankingProfile : public CSVProfile
{
public:
  profileTypeE type() const { return ProfileBank; }
  bool readSettings(const KSharedConfigPtr &config);
  void writeSettings(const KSharedConfigPtr &config);

  QList<int>       m_memoColList;

  bool             m_oppositeSigns;
};

class InvestmentProfile : public CSVProfile
{
public:
  profileTypeE type() const { return ProfileInvest; }
  bool readSettings(const KSharedConfigPtr &config);
  void writeSettings(const KSharedConfigPtr &config);

  QMap <MyMoneyStatement::Transaction::EAction, QStringList> m_transactionNames;

  QString     m_feeRate;
  QString     m_minFee;
  QString     m_securityName;
  QString     m_securitySymbol;

  QList<int>  m_memoColList;

  int         m_priceFraction;
  int         m_feeIsPercentage;
  int         m_dontAsk;
};

class PricesProfile : public CSVProfile
{
public:
  explicit PricesProfile() : CSVProfile() {}
  explicit PricesProfile(const profileTypeE profileType) : CSVProfile(), m_profileType(profileType) {}
  PricesProfile(QString profileName, int encodingMIBEnum,
                     int startLine, int trailerLines,
                     int dateFormatIndex, int fieldDelimiterIndex, int textDelimiterIndex, int decimalSymbolIndex,
                     QMap<columnTypeE, int> colTypeNum,
                     int priceFraction, profileTypeE profileType) :
    CSVProfile(profileName, encodingMIBEnum,
               startLine, trailerLines,
               dateFormatIndex, fieldDelimiterIndex, textDelimiterIndex, decimalSymbolIndex,
               colTypeNum),
    m_priceFraction(priceFraction), m_profileType(profileType) {}

  profileTypeE type() const { return m_profileType; }
  bool readSettings(const KSharedConfigPtr &config);
  void writeSettings(const KSharedConfigPtr &config);

  QString m_securityName;
  QString m_securitySymbol;
  QString m_currencySymbol;

  int     m_dontAsk;
  int     m_priceFraction;

  profileTypeE m_profileType;
};

class CSVFile
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

  CsvUtil            *m_csvUtil;
  Parse              *m_parse;
  QStandardItemModel *m_model;

  QString             m_inFileName;

  int                 m_columnCount;
  int                 m_rowCount;
};

class CSVImporter : public QObject
{
  Q_OBJECT
public:
  explicit CSVImporter();
  ~CSVImporter();

  /**
  * This method will silently import csv file. Main purpose of this method are online quotes.
  */
  MyMoneyStatement unattendedPricesImport(const QString &filename, PricesProfile *profile);

  static KSharedConfigPtr configFile();
  void profileFactory(const profileTypeE type, const QString &name);
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
  static bool profilesAction(const profileTypeE type, const profilesActionE action, const QString &name, const QString &newname);

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
  int detectDecimalSymbol(const int col, const QString &exclude);

  /**
  * This method will try to detect account from csv header.
  */
  QList<MyMoneyAccount> findAccounts(const QList<MyMoneyAccount::accountTypeE> &accountTypes, const QString &statementHeader);
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
  QMap<int, int>              m_decimalSymbolIndexMap;
  QMap<QString, QString>      m_mapSymbolName;
  QMap<autodetectTypeE, bool> m_autodetect;

  static const QMap<columnTypeE, QString>                            m_colTypeConfName;
  static const QMap<profileTypeE, QString>                           m_profileConfPrefix;
  static const QMap<MyMoneyStatement::Transaction::EAction, QString> m_transactionConfName;
  static const QMap<miscSettingsE, QString>                          m_miscSettingsConfName;
  static const QString                                               m_confProfileNames;
  static const QString                                               m_confPriorName;
  static const QString                                               m_confMiscName;

signals:
  void           statementReady(MyMoneyStatement&);
};

#endif
