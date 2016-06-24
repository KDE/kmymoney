/*******************************************************************************
*                                 csvdialog.h
*                              ------------------
* begin                       : Sat Jan 01 2010
* copyright                   : (C) 2010 by Allan Anderson
* email                       : agander93@gmail.com
********************************************************************************/

/*******************************************************************************
*                                                                              *
*   This program is free software; you can redistribute it and/or modify       *
*   it under the terms of the GNU General Public License as published by       *
*   the Free Software Foundation; either version 2 of the License, or          *
*   (at your option) any later version.                                        *
*                                                                              *
********************************************************************************/

#ifndef CSVDIALOG_H
#define CSVDIALOG_H

#include <QWidget>
#include <QWizard>
#include <QLabel>
#include <QVBoxLayout>
#include <QScrollBar>

#include <QComboBox>
#include <KColorScheme>

#include "csvimporterplugin.h"

class ConvertDate;
class Parse;
class CsvUtil;
class BankingPage;
class InvestmentPage;
class LinesDatePage;
class IntroPage;
class CompletionPage;
class SeparatorPage;
class InvestmentDlg;
class InvestProcessing;
class CsvImporterPlugin;
class SymbolTableDlg;
class CSVWizard;

namespace Ui
{
class CSVDialog;
}

class CSVDialog : public QWidget
{
  Q_OBJECT

private:
  struct csvSplit {
    QString      m_strCategoryName;
    QString      m_strMemo;
    QString      m_amount;
  } m_csvSplit;

public:
  explicit CSVDialog();
  ~CSVDialog();

  CSVWizard*          m_wiz;
  CsvImporterPlugin*  m_plugin;
  CSVDialog*          m_csvDlg;
  InvestmentDlg*      m_investmentDlg;
  InvestProcessing*   m_investProcessing;
  ConvertDate*        m_convertDate;
  Parse*              m_parse;
  SymbolTableDlg*     m_symbolTableDlg;
  CsvUtil*            m_csvUtil;

  Ui::CSVDialog*      ui;
  QVBoxLayout*        m_wizardLayout;
  QScrollBar*         m_vScrollBar;

  struct qifData {
    QString number;
    QDate   date;
    QString payee;
    QString amount;
    QString memo;
    QString category;
    QString id;
  } m_trData;

  QList<MyMoneyStatement> statements;
  QList<QTextCodec *>     m_codecs;
  QList<int>     m_columnCountList;
  QList<int>     m_memoColList;

  QStringList    m_shrsinList;
  QStringList    m_divXList;
  QStringList    m_brokerageList;
  QStringList    m_reinvdivList;
  QStringList    m_buyList;
  QStringList    m_sellList;
  QStringList    m_removeList;
  QStringList    m_dateFormats;
  QStringList    m_columnList;
  QStringList    m_securityList;
  QStringList    m_typeOfFile;
  QStringList    m_profileList;
  QStringList    m_columnTypeList;  //  holds field types - date, payee, etc.

  QString        m_csvPath;
  QString        m_profileName;
  QString        m_date;
  QString        m_fieldDelimiterCharacter;
  QString        m_inBuffer;
  QString        m_inFileName;
  QString        m_lastFileName;
  QString        m_outBuffer;
  QString        m_qifBuffer;
  QString        m_textDelimiterCharacter;
  QString        inFileName();
  QString        m_fileType;
  QString        m_priorCsvProfile;
  QString        m_priorInvProfile;
  QString        m_detailFilter;

  /**
   * a list of already used hashes in this file
   */
  QMap<QString, bool> m_hashMap;

  bool           m_importNow;
  bool           m_showEmptyCheckBox;
  bool           m_accept;
  bool           m_acceptAllInvalid;
  bool           m_screenUpdated;
  bool           m_goBack;
  bool           m_importIsValid;
  bool           m_isTableTrimmed;
  bool           m_importError;
  bool           m_profileExists;
  bool           m_firstPass;
  bool           m_firstRead;
  bool           m_columnsNotSet;
  bool           m_heightWasIncreased;
  bool           m_separatorPageVisible;
  bool           m_delimiterError;
  bool           m_needFieldDelimiter;
  bool           m_firstField;
  bool           m_errorFoundAlready;
  bool           m_rowWidthsDone;
  bool           m_widthResized;
  bool           m_amountSelected;
  bool           m_creditSelected;
  bool           m_dateSelected;
  bool           m_debitSelected;
  bool           m_memoSelected;
  bool           m_memoColCopied;
  bool           m_payeeColAdded;
  bool           m_payeeColCopied;
  bool           m_payeeSelected;
  bool           m_numberSelected;
  bool           m_categorySelected;
  bool           m_closing;

  int            m_dateFormatIndex;
  int            m_debitFlag;
  int            m_encodeIndex;
  int            m_fieldDelimiterIndex;
  int            m_textDelimiterIndex;
  int            m_endColumn;
  int            m_flagCol;
  int            m_row;
  int            m_visibleRows;
  int            m_rowHeight;
  int            m_tableHeight;
  int            m_activityType;
  int            m_initHeight;
  int            m_startHeight;
  int            m_hScrollBarHeight;
  int            m_vScrollBarWidth;
  int            m_vHeaderWidth;
  int            m_header;
  int            m_borders;
  int            m_possibleDelimiter;
  int            m_lastDelimiterIndex;
  int            m_errorColumn;
  int            m_pluginWidth;
  int            m_pluginHeight;
  int            m_windowHeight;
  int            m_maxColumnCount;
  int            m_dpiDiff;

  QUrl           m_url;
  QComboBox*     m_comboBoxEncode;

  QFile*         m_inFile;

  void             saveSettings();

  QString          columnType(int column);
  QString          decimalSymbol();
  int              decimalSymbolIndex();
  void             setDecimalSymbol(int val);
  QString          currentUI();

  void             clearPreviousColumn();
  void             setPreviousColumn(int);
  void             setCurrentUI(QString);

  int              maxColumnCount() const;
  int              amountColumn() const;
  void             setAmountColumn(int);
  int              debitColumn() const;
  void             setDebitColumn(int);
  int              creditColumn() const;
  void             setCreditColumn(int);
  int              dateColumn() const;
  void             setDateColumn(int);
  int              payeeColumn() const;
  void             setPayeeColumn(int);
  int              numberColumn() const;
  void             setNumberColumn(int);
  int              memoColumn() const;
  void             setMemoColumn(int);
  int              categoryColumn() const;
  void             setCategoryColumn(int);
  int              validateColumn(const int& col, QString& type);

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
  * Because the memo field allows multiple selections, it helps to be able
  * to see which columns are selected already, particularly if a column
  * selection gets deleted. This is achieved by adding a '*' character
  * after the column number of each selected column in the menu combobox.
  * This method is called to remove the '*' characters when a file is
  * reloaded, or when the user clears his selections.
  */
  void           clearComboBoxText();

  /**
  * This method is called when an input file has been selected, to clear
  * previous column selections.
  */
  void           clearColumnTypesList();

  int            columnNumber(const QString& msg);

  /**
  * This method is called initially after an input file has been selected.
  * It will call other routines to display file content and to complete the
  * statement import. It will also be called to reposition the file after row
  * deletion, or to reread following encoding or delimiter change.
  */
  void           readFile(const QString& fname);

  /**
  * This method is called on opening the plugin.
  * It will add all codec names to the encoding combobox.
  */
  void           setCodecList(const QList<QTextCodec *> &list);

  /**
  * This method is called when the user selects a new decimal symbol.  The
  * UI is updated using the new symbol, and on importing, the new symbol
  * also will be used.
  */
  void           updateDecimalSymbol(const QString&, int col);

  /**
  * This method is called when an input file has been selected.
  * It will enable the UI elements for column selection.
  */
  void           enableInputs();

  /**
  * This method is called when a date cannot be recognised  and the user
  * cancels the statement import. It will disable the UI elements for column
  * selection.
  */
  void           disableInputs();

  /**
  * This method is called on opening the plugin.
  * It will populate a list with all available codecs.
  */
  void           findCodecs();

  /**
  * This method is called on opening the input file.
  * It will display a line in the UI table widget.
  */
  void           displayLine(const QString& data);

  /**
  * This method is called when the user clicks 'import'.
  * It will evaluate a line and prepare it to be added to a statement,
  * and to a QIF file, if required.
  */
  int            processQifLine(QString& iBuff);

  /**
  * This method is called after processQifLine, to add a transaction to a
  * list, ready to be imported.
  */
  void           csvImportTransaction(MyMoneyStatement& st);

  /**
  * This method is called after startup, to initialise some parameters.
  */
  void           init();

  /**
  * This method is called during file selection, to load settings from the resource file.
  */
  void           readSettings();

  /**
  * This method is called to reload column settings from the UI.
  */
  void           reloadUISettings();

  /**
  * This method is called when one of the radiobuttons is checked, to populate
  * the sourceNames combobox from the resource file.
  */
  void           readSettingsInit();

  /**
  * Immediately after installation, there is no local config file.
  * This method is called to copy the default version into the user's
  * local folder.
  */
  void           readSettingsProfiles();

  /**
  * This method is called when the user chooses to add a new profile, It achieves this by copying
  * the necessary basic parameters from an existing profile called "Profiles-New Profile###"
  * in the resource file,
  */
  void           createProfile(QString newName);

  void           showStage();

  /**
  *  Clear cells background.
  */
  void           clearCellsBackground();
  void           clearColumnTypeList();

  int            endColumn();
  int            fieldDelimiterIndex();
  int            lastLine() const;
  int            fileLastLine() const;
  int            startLine() const;
  void           setStartLine(int);

  bool           importNow();

signals:
  /**
  * This signal is raised when the plugin has completed a transaction.  This
  * then needs to be processed by MyMoneyStatement.
  */
  void           statementReady(MyMoneyStatement&);
  bool           isImportable();
  void           namesEdited();
  void           valueChanged(int);

public slots:
  void           slotNamesEdited();
  void           slotBackButtonClicked();
  void           slotVertScrollBarMoved(int val);

  /**
  * This method is called when the user clicks 'Open File', and opens
  * a file selector dialog.
  */
  void           slotFileDialogClicked();

  /**
  * This method is called when a field or text delimiter is changed.  The
  * input file is reread using the new delimiter.
  */
  void           delimiterChanged();

  /**
  * This method is called when the user selects a new field or text delimiter.  The
  * input file is reread using the new delimiter.
  */
  void           delimiterActivated();

  /**
  * This method is called when the user clicks 'import'. It performs further
  * validity checks on the user's choices, then redraws the required rows.
  * Finally, it rereads the file, which this time will result in the import
  * actually taking place.
  */
  void           slotImportClicked();

  /**
  * This method is called when the user clicks 'Date format' and selects a
  * format, which is used by convertDate().
  */
  void           dateFormatSelected(int dF);

  /**
  * This method is called when the user clicks 'Save as QIF'. A file selector
  * dialog is shown, where the user may select the save location.
  */
  void           slotSaveAsQIF();

  /**
  * This method is called when the user selects a new decimal symbol.  The
  * UI is updated using the new symbol.
  */
  void           decimalSymbolSelected(int);
  void           decimalSymbolSelected();
  void           markUnwantedRows();

private:
  /**
  * Clear an invalid debit or credit field and return
  * pointer to the valid field.
  */
  QString          clearInvalidField(QString, QString);
  QString          m_currentUI;
  QString          m_decimalSymbol;
  QString          m_previousType;
  QString          m_thousandsSeparator;
  QString          m_firstValue;
  QString          m_secondValue;
  QString          m_firstType;
  QString          m_secondType;
  QStringList      m_lineList;

  bool             m_duplicate;
  bool             m_clearAll;
  bool             m_firstIsValid;
  bool             m_secondIsValid;
  bool             m_initWindow;
  bool             m_resizing;
  bool             m_vScrollBarVisible;

  int              m_amountColumn;
  int              m_creditColumn;
  int              m_dateColumn;
  int              m_debitColumn;
  int              m_memoColumn;
  int              m_numberColumn;
  int              m_payeeColumn;
  int              m_categoryColumn;
  int              m_previousColumn;
  int              m_maxRowWidth;
  int              m_rowWidth;
  int              m_decimalSymbolIndex;
  int              m_endLine;
  int              m_fileEndLine;
  int              m_startLine;
  int              m_topLine;
  int              m_curId;
  int              m_lastId;
  int              m_lineNum;
  int              m_memoColCopy;
  int              m_lastHeight;
  int              m_round;
  int              m_minimumHeight;
  int              m_windowWidth;
  int              m_initialHeight;
  int              m_rectWidth;

  QBrush           m_clearBrush;
  QBrush           m_clearBrushText;
  QBrush           m_colorBrush;
  QBrush           m_colorBrushText;
  QBrush           m_errorBrush;
  QBrush           m_errorBrushText;

  void             closeEvent(QCloseEvent *event);
  bool             eventFilter(QObject *object, QEvent *event);

  void             restoreBackground();
  void             resizeEvent(QResizeEvent* ev);

  /**
  * Recalculates column widths for the visible rows
  */
  void             updateColumnWidths(int firstLine, int lastLine);

  /**
  * Called in order to adjust window size to suit the file,
  * depending upon column data width between current firstLine and lastLine
  * and also the number of rows.
  */
  void             setWindowSize(int firstLine, int lastLine);

  /**
  * Check that the debit and credit field combination
  * is valid.
  */
  int              ensureBothFieldsValid(int);

private slots:
  /**
  * This method is called when the user clicks 'Encoding' and selects an
  * encoding setting.  The file is re-read with the corresponding codec.
  */
  void           encodingChanged(int index);

  /**
  * This method is called when the user edits the lastLine setting.
  */
  void           endLineChanged(int val);

  /**
  * This method is called when the user edits the startLine setting.
  */
  void           startLineChanged(int val);

  void           thousandsSeparatorChanged();
};

#endif // CSVDIALOG_H
