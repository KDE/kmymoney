/***************************************************************************
                       investprocessing.h
                       ------------------
begin                 : Sat Jan 01 2010
copyright             : (C) 2010 by Allan Anderson
email                 : agander93@gmail.com
copyright             : (C) 2016 by Łukasz Wojniłowicz
email                 : lukasz.wojnilowicz@gmail.com
****************************************************************************/

/***************************************************************************

*   This program is free software; you can redistribute it and/or modify   *
*   it under the terms of the GNU General Public License as published      *
*   by the Free Software Foundation; either version 2 of the License,      *
*   or  (at your option) any later version.                                *
*                                                                          *
****************************************************************************/


#ifndef INVESTPROCESSING_H
#define INVESTPROCESSING_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QtCore/QFile>
#include <QCompleter>
#include <QPointer>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KSharedConfig>

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneystatement.h>

class RedefineDlg;
class SecuritiesDlg;
class CSVWizard;

class InvestProcessing : public QObject
{
  Q_OBJECT

private:

public:
  InvestProcessing();
  ~InvestProcessing();

  CSVWizard*        m_wiz;
  RedefineDlg*      m_redefine;
  QPointer<SecuritiesDlg>   m_securitiesDlg;

  typedef enum:uchar { ColumnDate, ColumnType, ColumnAmount,
         ColumnPrice, ColumnQuantity, ColumnFee,
         ColumnSymbol, ColumnName, ColumnMemo,
         ColumnEmpty = 0xFE, ColumnInvalid = 0xFF
       } columnTypeE;

  QMap<columnTypeE, int>   m_colTypeNum;
  QMap<int ,columnTypeE>   m_colNumType;
  QMap<columnTypeE, QString> m_colTypeName;

  int            m_feeIsPercentage;
  int            m_securityNameIndex;
  int            m_priceFraction;

  QString        m_priceFractionValue;
  QString        m_feeRate;
  QString        m_minFee;
  QString        m_nameFilter;

  void           saveSettings();

  /**
  * This method is called after startup, to initialise some parameters.
  */
  void           init();

  /**
  * This method is called on opening, to load settings from the resource file.
  */

  void           readSettings(const KSharedConfigPtr &config);

  QMap<QString, QString> m_mapSymbolName;

  QStringList    m_investList;
  QStringList    m_shrsinList;
  QStringList    m_divXList;
  QStringList    m_intIncList;
  QStringList    m_feeList;
  QStringList    m_brokerageList;
  QStringList    m_reinvdivList;
  QStringList    m_buyList;
  QStringList    m_sellList;
  QStringList    m_shrsoutList;
  QStringList    m_columnList;
  QStringList    m_securityList;

  QList<MyMoneyStatement::Transaction::EAction> m_validActionTypes;
  QList<MyMoneyStatement::Security> m_listSecurities;
  void           setSecurityName(QString name);

public:
  /**
  * This method ensures that every security has symbol and name.
  */
  bool           validateSecurities();
signals:
  bool           isImportable();
  /**
  * This signal is raised when the plugin has completed a transaction.  This
  * then needs to be processed by MyMoneyStatement.
  */
  void           statementReady(MyMoneyStatement&);

public slots:

  /**
  * This method is called if any of the inputs (i.e Amount/Fee column selected or Fee rate entered)
  * has changed. The method enables and disables appropriate controls on Investment page
  */
  void           feeInputsChanged();

  /**
  * This method will check whether memo combobox is still valid
  * after changing name or type column.
  */
  bool           validateMemoComboBox();

  /**
  * This method is called when the Fraction column is activated.
  * It will update m_priceFractionValue variable.
  */
  void           fractionColumnChanged(int col);

  /**
  * This method is called column on investment page is selected.
  * It sets m_colTypeNum, m_colNumType and runs column validation.
  */
  bool           validateSelectedColumn(int col, columnTypeE type);

  /**
  * This method is called when the Memo column is activated.
  * Multiple columns may be selected sequentially.
  */
  void           memoColumnSelected(int col);

  /**
  * This method is called when the Date column is activated.
  * It will validate the column selection.
  */
  void           dateColumnSelected(int);

  /**
  * This method is called if the Fee column is activated.  The fee column may
  * contain either a value or a percentage. The user needs to set the 'Fee is
  * percentage' check box appropriately.  Caution may be needed here, as the fee
  * may already have been taken into account in the price.
  */
  void           feeColumnSelected(int);

  /**
  * This method is called when the activity 'Type/Action' column is activated.
  * It will validate the column selection.
  */
  void           typeColumnSelected(int);

  /**
  * This method is called when the Quantity column is activated.
  * It will validate the column selection.
  */
  void           quantityColumnSelected(int);

  /**
  * This method is called when the Price column is activated.
  * It will validate the column selection.
  */
  void           priceColumnSelected(int);

  /**
  * This method is called when the Amount column is activated.
  * It will validate the column selection.
  */
  void           amountColumnSelected(int);

  /**
  * This method is called when the Symbol column is activated.
  * It will validate the column selection.
  */
  void           symbolColumnSelected(int);

  /**
  * This method is called when the Name column is activated.
  * It will validate the column selection.
  */
  void           nameColumnSelected(int);

  /**
  * This method is called when the feeIsPercentageCheckBox checkbox is clicked.
  * It will set m_feeIsPercentage.
  */
  void           feeIsPercentageCheckBoxClicked(bool checked);

  /**
  * This method fills QIF file with investment data
  */
  void           makeQIF(MyMoneyStatement& st, QFile& file);

  /**
  * This method is called when the user clicks 'Clear fees'.
  * All fees selections are cleared. Generated fees colum is removed.
  */
  void           clearFeesSelected();

  /**
  * This method is called to calculate fee based on fee rate and amount.
  * The method generates column with fees in importer's window afterwards.
  */
  void           calculateFee();

  /**
  * This method feeds file buffer in investment lines parser.
  */
  bool           createStatement(MyMoneyStatement& st);

  /**
  * This method is called when the user clicks 'Import'.
  * It will evaluate an input line and append it to a statement.
  */
  bool processInvestLine(const QString& line, MyMoneyStatement& st);

signals:
  void           slotGetStatement();

private:

  /**
  * This method creates valid set of possible transactions
  * according to quantity, amount and price
  */
  bool createValidActionTypes(QList<MyMoneyStatement::Transaction::EAction> &validActionTypes, MyMoneyStatement::Transaction &tr);

  /**
  * This method stores user selection of action type so it will not ask
  * for the same action type twice.
  */
  void storeActionType(MyMoneyStatement::Transaction::EAction &actionType, const QString &userType);

  /**
  * This method validates the column numbers entered by the user.  It then
  * checks the values in those columns for compatibility with the input
  * investment activity type.
  */
  bool  validateActionType(MyMoneyStatement::Transaction::EAction &actionType, const QString &userType);

  /**
  * This method is called during input.  It validates the action types
  * in the input file, and assigns appropriate QString types.
  */
  MyMoneyStatement::Transaction::EAction processActionType(QString& type);

  /**
    * This method is used to get the account id of the split for
    * a transaction from the text found in the QIF $ or L record.
    * If an account with the name is not found, the user is asked
    * if it should be created.
  */
  const QString checkCategory(const QString& name, const MyMoneyMoney& value, const MyMoneyMoney& value2);

  /**
  * This method gets securities from investment statement and
  * tries to get pairs of symbol and name either
  * from KMM or from statement data.
  * In case it's not successfull onlySymbols and onlyNames won't be empty.
  */
  bool sortSecurities(QSet<QString>& onlySymbols, QSet<QString>& onlyNames, QMap<QString, QString>& mapSymbolName);

  void createAccount(MyMoneyAccount& newAccount, MyMoneyAccount& parentAccount, MyMoneyAccount& brokerageAccount, MyMoneyMoney openingBal);

  QString        m_securityName;

  QCompleter*     m_completer;

private slots:

  /**
  * This method is called when it is detected that the user has selected the
  * same column for two different fields.  The column detecting the error
  * has to reset the other column.
  */
  void           resetComboBox(columnTypeE comboBox);

  /**
  * This method is called to remove a security name from the combobox list.
  * It does not affect the underlying security.
  */
  void           hideSecurity();

  void           securityNameSelected(const QString& name);
  void           securityNameEdited();
};
#endif // INVESTPROCESSING_H
