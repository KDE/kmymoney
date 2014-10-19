/***************************************************************************
                       investprocessing.h
                       ------------------
begin                 : Sat Jan 01 2010
copyright             : (C) 2010 by Allan Anderson
email                 : agander93@gmail.com
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
#include <QtGui/QCompleter>

// ----------------------------------------------------------------------------
// KDE Headers

#include <KUrl>

// ----------------------------------------------------------------------------
// Project Headers

#include <mymoneystatement.h>

class ConvertDate;
class CSVDialog;
class InvestmentDlg;
class RedefineDlg;
class Parse;
class CsvUtil;
class MyMoneyStatement;
class KAbstractFileWidget;
class KHBox;
class KComboBox;

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

  CSVDialog*        m_csvDialog;
  InvestmentDlg*    m_investDlg;
  Parse*            m_parse;
  ConvertDate*      m_convertDat;
  RedefineDlg*      m_redefine;
  CsvUtil*          m_csvUtil;

  KComboBox*        m_comboBoxEncode;

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
    * This method is called initially after an input file has been selected.
    * It will call other routines to display file content and to complete the
    * statement import. It will also be called to reposition the file after row
    * selection, or to reread following encoding or delimiter change.
    */
  void           readFile(const QString& fname);

  /**
  * This method is called when the user clicks 'Clear selections', in order to
  * clear incorrect column number entries.  Also called on initialisation.
  */
  void           clearSelectedFlags();

  /**
  * This method is called when the user clicks 'Clear selections', in order to
  * clear incorrect column number entries.
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

  void           setInFileName(const QString& val);

  /**
  * This method ensures the rows and columns are correctly aligned
  * when a diffent set of lines is displayed, as, among other things,
  * the horizontal scroll-bar needs to be taken into account.
  */
  void           redrawWindow(int startLine);

  /**
   * Recalculates column widths for the visible rows
  */
  void           updateColumnWidths(int firstLine, int lastLine);

  /**
  * Called on reading a file in order to
  * adjust window size to suit the file.
  */
  void           setWindowSize(int firstLine, int lastLine);

  QString        columnType(int column);
  QString        invPath();
  QString        inFileName();
  QString        m_inFileName;
  QString        m_buf;
  QString        m_invPath;

  QStringList    securityList();
  QStringList    m_symbolsList;
  QStringList    m_namesList;

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
  QStringList    m_dateFormats;
  QStringList    m_columnList;
  QStringList    m_securityList;
  QStringList    m_lineList;

  QList<MyMoneyStatement::Security> m_listSecurities;
  QList<int>     m_memoColList;
  QList<int>     m_columnCountList;

  int            lastLine();
  int            amountColumn();
  int            priceColumn();
  int            quantityColumn();
  int            dateColumn();
  int            detailColumn();
  int            feeColumn();
  int            symbolColumn();
  int            typeColumn();
  int            memoColumn();

  bool           importNow();
  bool           m_symbolTableScanned;
  bool           m_firstRead;

  void           setSecurityName(QString name);

  int            m_endColumn;
  int            m_endLine;
  int            m_fileEndLine;
  int            m_startLine;
  int            m_topLine;
  int            m_row;
  int            m_rowWidth;
  int            m_fieldDelimiterIndex;

  bool           m_screenUpdated;
  bool           m_moreCommas;
  bool           m_importCompleted;

public:
signals:
  /**
  * This signal is raised when the plugin has completed a transaction.  This
  * then needs to be processed by MyMoneyStatement.
  */
  void           statementReady(MyMoneyStatement&);

public slots:

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
  * This method is called when the user selects a new field delimiter.  The
  * input file is reread using the current delimiter.
  */
  void           fieldDelimiterChanged();

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
  * This method is called when the Detail column is activated.
  * It will validate the column selection.
  */
  void           detailColumnSelected(int);

  /**
  * This method is called when the user clicks Accept. It performs further
  * validity checks on the user's choices, then redraws the required rows.
  * Finally, it rereads the file, which this time will result in the import
  * actually taking place.
  */
  void           slotImportClicked();

  /**
  * This method is called when the user clicks the Date button and selects
  * the date format for the input file.
  */
  void           dateFormatSelected(int dF);

  /**
  * This method is called should the user click 'Save as QIF'. A File Selection
  * dialog is presented and the data is output in QIF format.
  */
  void           saveAs();

  /**
  * This method is called when the user selects the start line.  The requested
  * start line  value is saved.
  */
  void           startLineChanged(int);
  void           startLineChanged();
  /**
  * This method is called when the user selects the end line.  The requested
  * end line  value is saved, to be used on import.
  */
  void           endLineChanged(int val);

  /**
  * This method is called when the activity 'Type/Action' column is activated.
  * It will validate the column selection.
  */
  void           typeColumnSelected(int);

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
  * This method is called on opening the input file.
  * It will display a line in the UI table widget.
  */
  void           displayLine(const QString&);

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
  * This method is called on opening the plugin.
  * It will populate a list with all available codecs.
  */
  void           findCodecs();

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
  * This method is called on opening the plugin.
  * It will add all codec names to the encoding combobox.
  */
  void           setCodecList(const QList<QTextCodec *> &list);

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

  QList<csvSplit> m_csvSplitsList;
  QList<QTextCodec *>   m_codecs;

  bool           m_amountSelected;
  bool           m_brokerage;
  bool           m_brokerageItems;
  bool           m_importNow;
  bool           m_dateSelected;
  bool           m_feeSelected;
  bool           m_firstPass;
  bool           m_memoSelected;
  bool           m_priceSelected;
  bool           m_quantitySelected;
  bool           m_typeSelected;
  bool           m_memoColCopied;
  bool           m_typeColCopied;
  bool           m_detailColCopied;
  bool           m_symbolSelected;
  bool           m_detailSelected;
  bool           m_needFieldDelimiter;

  int            m_dateFormatIndex;
  int            m_maxColumnCount;
  int            m_encodeIndex;
  int            m_payeeColumn;
  int            m_amountColumn;
  int            m_dateColumn;
  int            m_feeColumn;
  int            m_memoColumn;
  int            m_priceColumn;
  int            m_previousColumn;
  int            m_quantityColumn;
  int            m_symbolColumn;
  int            m_detailColumn;
  int            m_textDelimiterIndex;
  int            m_typeColumn;
  int            m_symbolRow;
  int            m_maxRowWidth;
  int            m_initWindow;
  int            m_screenWidth;

  QString        m_accountName;
  QString        m_brokerBuff;
  QString        m_dateFormat;
  QString        m_fieldDelimiterCharacter;
  QString        m_textDelimiterCharacter;
  QString        m_inBuffer;

  QString        m_outBuffer;
  QString        m_previousType;
  QString        m_securityName;
  QString        m_tempBuffer;

  QStringList    m_columnTypeList;  //  holds field types - date, payee, etc.

  KUrl           m_url;
  QFile*         m_inFile;

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
  * This method is called to remove a security name from the combobox list.
  * It does not affect the underlying security.
  */
  void           hideSecurity();

  void           securityNameSelected(const QString& name);
  void           securityNameEdited();

  int            validateNewColumn(const int& col, const QString& type);

};
#endif // INVESTPROCESSING_H
