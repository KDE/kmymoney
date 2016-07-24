/***************************************************************************
                               csvutil.h
                              -----------
begin                : Sat Jan 01 2010
copyright            : (C) 2010 by Allan Anderson
email                : agander93@gmail.com
***************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef CSVUTIL_H
#define CSVUTIL_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QDate>
#include <QtCore/QStringList>
#include <QObject>

#include "investtransactioneditor.h"

//class CSVDialog;
class MyMoneyAccount;
class InvestTransactionEditor;
class TransactionEditor;

class Parse: public QObject
{
  Q_OBJECT

public:
  Parse();
  ~Parse();

//  CSVDialog*     m_csvDialog;

  /**
   * This method is used to parse each line of data, splitting it into
   * separate fields, via the field delimiter character.  It also detects
   * where a string has been erroneously split because it contains one or
   * more 'thousand separators' which happen to be the same as the field
   * delimiter, and re-assembles the string.
   */
  QStringList      parseLine(const QString& data);
  QStringList      parseFile(const QString& buf, int strt, int end);
  QStringList      m_fieldDelimiterCharList;

  QString          fieldDelimiterCharacter(int index);
  QString          decimalSymbol(int index);
  int              decimalSymbolIndex();
  QString          textDelimiterCharacter(int index);
  void             thousandsSeparatorChanged(int index);
  QString          thousandsSeparator();

  /**
   * Check for presence of the selected decimal symbol
   * and evaluate if the proposed conversion is valid.
   * If so, change the symbol.
   */
  QString          possiblyReplaceSymbol(const QString&  str);

  void             setFieldDelimiterIndex(int index);
  void             setFieldDelimiterCharacter(int index);

  void             setTextDelimiterIndex(int index);
  void             setTextDelimiterCharacter(int index);

  void             setDecimalSymbolIndex(int index);
  void             setDecimalSymbol(int index);

  void             setThousandsSeparatorIndex(int index);
  void             setThousandsSeparator(int index);

  bool             symbolFound();
  void             setSymbolFound(bool found);

  bool             invalidConversion();

  int              lastLine();

public slots:

  void             decimalSymbolSelected(int index);

private :

  QStringList      m_decimalSymbolList;
  QStringList      m_textDelimiterCharList;
  QStringList      m_thousandsSeparatorList;

  QString          m_decimalSymbol;
  QString          m_fieldDelimiterCharacter;
  QString          m_textDelimiterCharacter;
  QString          m_thousandsSeparator;

  int              m_decimalSymbolIndex;
  int              m_fieldDelimiterIndex;
  int              m_lastLine;
  int              m_textDelimiterIndex;
  int              m_thousandsSeparatorIndex;

  bool             m_symbolFound;
  bool             m_invalidConversion;
}
;

class CsvUtil: public QObject
{
  Q_OBJECT

public:
  CsvUtil();
  ~CsvUtil();

  InvestTransactionEditor*    m_investTransactionEditor;

  const QString&     feeId(const MyMoneyAccount& invAcc);
  const QString&     interestId(const MyMoneyAccount& invAcc);
  QString            expenseId(const QString& name);
  QString            interestId(const QString& name);
  QString            feeId(const QString& name);
  QString            nameToId(const QString& name, MyMoneyAccount& parent);
  void               scanCategories(QString& id, const MyMoneyAccount& invAcc, const MyMoneyAccount& parentAccount, const QString& defaultName);
  void               previouslyUsedCategories(const QString& investmentAccount, QString& feesId, QString& interestId);
  void               dissectTransaction(const MyMoneyTransaction& transaction, const MyMoneySplit& split, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency, MyMoneySplit::investTransactionTypeE& transactionType);

  const QString      checkCategory(const QString& name, const MyMoneyMoney& value, const MyMoneyMoney& value2);
  void               createAccount(MyMoneyAccount& newAccount, MyMoneyAccount& parentAccount, MyMoneyAccount& brokerageAccount, MyMoneyMoney openingBal);
private:
  QString          m_feeId;
  QString          m_interestId;
  bool             m_scannedCategories;
};

#endif
