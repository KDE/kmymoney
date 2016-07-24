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
// QT Headers

#include <QtCore/QDate>
#include <QtCore/QTextStream>
#include <QtCore/QFile>
#include <QtCore/QStringList>
#include <QtCore/QList>
#include <QCompleter>
#include <QComboBox>
#include <QUrl>

#include <mymoneystatement.h>

class RedefineDlg;
class SymbolTableDlg;
class CSVWizard;

class InvestProcessing : public QObject
{
  Q_OBJECT

private:
  struct csvSplit {
    QString      m_strCategoryName;
    QString      m_strMemo;
    QString      m_amount;
  } m_csvSplit;

public:
  InvestProcessing();
  ~InvestProcessing();

  CSVWizard*        m_wiz;
  RedefineDlg*      m_redefine;
  SymbolTableDlg*   m_symbolTableDlg;

  int            m_payeeColumn;
  int            m_amountColumn;
  int            m_feeColumn;
  int            m_priceColumn;
  int            m_quantityColumn;
  int            m_symbolColumn;
  int            m_nameColumn;
  int            m_feeIsPercentage;
  int            m_typeColumn;
  int            m_symbolRow;
  int            m_securityNameIndex;
  int            m_priceFraction;

  QString        m_feeRate;
  QString        m_minFee;
  QString        m_nameFilter;

  void           saveSettings();

  void           setTrInvestDataType(const QString& val);

  /**
  * This method is called after startup, to initialise some parameters.
  */
  void           init();

  /**
  * This method is called on opening, to load settings from the resource file.
  */

  void           readSettings();

  /**
  * This method is called to reload column settings from the UI.
  */
  void           reloadUISettings();

  void           clearColumnType(int column);
  void           setColumnType(int column, const QString& type);

  QString        previousType();
  void           clearPreviousType();
  void           setPreviousType(const QString& type);

  /**
  * This method feeds file buffer in investment lines parser.
  */
  void           createStatement();

  /**
  * This method is called when the user clicks 'Clear selections', in order to
  * clear incorrect column number entries.  Also called on initialisation.
  */
  void           clearSelectedFlags();

  /**
  * This method is called when the user clicks 'Clear', in order to
  * clear incorrect column number entries.  Also called on initialisation.
  */
  void           clearColumnNumbers();

  /**
  * This method is called when an input file has been selected, to clear
  * previous column selections.
  */
  void           clearColumnTypes();

  /**
  * Because the memo field allows multiple selections, it helps to be able
  * to see which columns are selected already, particularly if a column
  * selection gets deleted. This is achieved by adding a '*' character
  * after the column number of each selected column in the menu combobox.
  * This method is called to remove the '*' characters when a file is
  * reloaded, or when the user clears his selections.
  */
  void           clearComboBoxText();

  QString        columnType(int column);
  QString        invPath();
  QString        m_invPath;

  QStringList    securityList();
  QStringList    m_symbolsList;
  QStringList    m_namesList;
  QStringList    m_columnTypeList;  //  holds field types - date, payee, etc.

  QMap<QString, QString> m_map;

  QStringList    m_investList;
  QStringList    m_shrsinList;
  QStringList    m_divXList;
  QStringList    m_intIncList;
  QStringList    m_feeList;
  QStringList    m_brokerageList;
  QStringList    m_reinvdivList;
  QStringList    m_buyList;
  QStringList    m_sellList;
  QStringList    m_removeList;
  QStringList    m_columnList;
  QStringList    m_securityList;

  QList<MyMoneyStatement::Security> m_listSecurities;

  int            amountColumn();
  int            priceColumn();
  int            quantityColumn();
  int            dateColumn();
  int            nameColumn();
  int            feeColumn();
  int            symbolColumn();
  int            typeColumn();
  int            memoColumn();
  int            feeIsPercentage();

  bool           importNow();
  bool           m_symbolTableScanned;

  void           setSecurityName(QString name);

//  bool           m_importCompleted;

public:
signals:
  bool           isImportable();
  /**
  * This signal is raised when the plugin has completed a transaction.  This
  * then needs to be processed by MyMoneyStatement.
  */
  void           statementReady(MyMoneyStatement&);

public slots:

  void           slotNamesEdited();

  /**
  * This method is called when the user clicks 'Open File', and opens
  * a file selector dialog.
  */
  void           slotFileDialogClicked();

  /**
  * This method is called when the Date column is activated.
  * It will validate the column selection.
  */
  void           dateColumnSelected(int);

  /**
  * This method is called when the user clicks 'Encoding' and selects an
  * encoding setting.  The file is re-read with the corresponding codec.
  */
  void           encodingChanged(int);

  /**
  * This method is called if any of the inputs (i.e Amount/Fee column selected or Fee rate entered)
  * has changed. The method enables and disables appropriate controls on Investment page
  */
  void           feeInputsChanged();

  /**
  * This method is called if the Fee column is activated.  The fee column may
  * contain either a value or a percentage. The user needs to set the 'Fee is
  * percentage' check box appropriately.  Caution may be needed here, as the fee
  * may already have been taken into account in the price.
  */
  void           feeColumnSelected(int);

  /**
  * This method is called when the Memo column is activated.
  * Multiple columns may be selected sequentially.
  */
  void           memoColumnSelected(int col);

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
  * This method is called when the user clicks Accept. It performs further
  * validity checks on the user's choices, then redraws the required rows.
  * Finally, it rereads the file, which this time will result in the import
  * actually taking place.
  */
  void           slotImportClicked();

  /**
  * This method is called should the user click 'Save as QIF'. A File Selection
  * dialog is presented and the data is output in QIF format.
  */
  void           saveAs();

  /**
  * This method is called when the activity 'Type/Action' column is activated.
  * It will validate the column selection.
  */
  void           typeColumnSelected(int);

  /**
  * This method is called when the user clicks 'Clear fees'.
  * All fees selections are cleared. Generated fees colum is removed.
  */
  void           clearFeesSelected();

  /**
  * This method is called when the user clicks 'Clear selections'.
  * All column selections are cleared.
  */
  void           clearColumnsSelected();

private:
signals:
  void           slotGetStatement();

private:
  /**
  * This method is called during import, to convert the QString activity type
  * to a MyMoneyStatement::Transaction::EAction& convType, which is added to
  * the transaction about to be imported.
  */
  void           convertType(const QString&, MyMoneyStatement::Transaction::EAction&);

  /**
  * This method is called when a date cannot be recognised  and the user
  * cancels the statement import. It will disable the UI elements for column
  * selection, neccessitating file reselection.
  */
  void           disableInputs();

  /**
  * This method is called when an input file has been selected.
  * It will enable the UI elements for column selection.
  */
  void           enableInputs();

  /**
  * This method is called during input.  It builds the
  * MyMoneyStatement, ready for importing.
  */
  void           investCsvImport(MyMoneyStatement&);

  /**
  * This method is called during input.  It validates the action types
  * in the input file, and assigns appropriate QString types.
  */
  int            processActionType(QString& type);

  /**
  * This method is called when the user clicks 'Accept'.
  * It will evaluate an input line and prepare it to be added to a statement,
  * and to a QIF file, if required.
  */
  int            processInvestLine(const QString& inBuffer);

  /**
  * This method is called during input if a brokerage type activity is found.
  * It will request user input of the brokerage/current account to be used, and
  * also of the column defining the payee/detail.
  */
  QString        accountName(const QString& aName);

  /**
  * This method is called during input if a brokerage type activity is found.
  * It will request user input of the column defining the payee/detail.
  */
  int            columnNumber(const QString& column);

  /**
    * This method is used to get the account id of the split for
    * a transaction from the text found in the QIF $ or L record.
    * If an account with the name is not found, the user is asked
    * if it should be created.
  */
  const QString checkCategory(const QString& name, const MyMoneyMoney& value, const MyMoneyMoney& value2);

  void createAccount(MyMoneyAccount& newAccount, MyMoneyAccount& parentAccount, MyMoneyAccount& brokerageAccount, MyMoneyMoney openingBal);

  struct qifInvestData {
    QString      memo;
    MyMoneyMoney price;
    MyMoneyMoney quantity;
    MyMoneyMoney amount;
    MyMoneyMoney fee;
    QString      payee;
    QString      security;
    QString      symbol;
    QString      brokerageAccnt;
    QString      type;
    QDate        date;
  }              m_trInvestData;

  bool           m_amountSelected;
  bool           m_brokerage;
  bool           m_brokerageItems;
  bool           m_importNow;
  bool           m_dateSelected;
  bool           m_feeSelected;
  bool           m_memoSelected;
  bool           m_priceSelected;
  bool           m_quantitySelected;
  bool           m_typeSelected;
  bool           m_symbolSelected;
  bool           m_nameSelected;
  bool           m_needFieldDelimiter;

  QString        m_accountName;
  QString        m_brokerBuff;
  QString        m_inBuffer;
  QString        m_outBuffer;
  QString        m_previousType;
  QString        m_securityName;
  QString        m_tempBuffer;

  QUrl           m_url;

  QCompleter*     m_completer;

private slots:

  /**
  * This method is called when it is detected that the user has selected the
  * same column for two different fields.  The column detecting the error
  * has to reset the other column.
  */
  void           resetComboBox(const QString& comboBox, const int& col);

  void           changedType(const QString& newType);

  /**
  * This method is called to calculate fee based on fee rate and amount.
  * The method generates column with fees in importer's window afterwards.
  */
  void           calculateFee();

  /**
  * This method is called to remove a security name from the combobox list.
  * It does not affect the underlying security.
  */
  void           hideSecurity();


  void           securityNameSelected(const QString& name);
  void           securityNameEdited();

  int            validateNewColumn(const int& col, const QString& type);

};
#endif // INVESTPROCESSING_H
