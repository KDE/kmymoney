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

#include <KComboBox>

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
  explicit CSVDialog(QWidget *parent = 0);
  ~CSVDialog();

  enum { Page_Intro, Page_Separator, Page_Banking, Page_Investment,
         Page_LinesDate, Page_Completion
       };

  QWizard*            m_wizard;
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

  IntroPage*       m_pageIntro;
  SeparatorPage*   m_pageSeparator;
  BankingPage*     m_pageBanking;
  InvestmentPage*  m_pageInvestment;
  LinesDatePage*   m_pageLinesDate;
  CompletionPage*  m_pageCompletion;

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

  int            m_dateFormatIndex;
  int            m_debitFlag;
  int            m_encodeIndex;
  int            m_fieldDelimiterIndex;
  int            m_textDelimiterIndex;
  int            m_endColumn;
  int            m_flagCol;
  int            m_row;
  int            m_visibleRows;
  int            m_rowHght;
  int            m_tableHeight;
  int            m_activityType;
  int            m_initHeight;
  int            m_startHeight;
  int            m_hScrollBarHeight;
  int            m_header;
  int            m_borders;
  int            m_possibleDelimiter;
  int            m_lastDelimiterIndex;
  int            m_errorColumn;

  QUrl           m_url;
  KComboBox*     m_comboBoxEncode;

  QFile*         m_inFile;

  void             saveSettings();

  QString          columnType(int column);
  QString          decimalSymbol();
  int              decimalSymbolIndex();
  void             setDecimalSymbol(int val);
  QString          currentUI();
  QStringList      columnTypeList();

  void             clearPreviousColumn();
  bool             amountSelected();
  void             setAmountSelected(bool val);
  bool             creditSelected();
  void             setCreditSelected(bool val);
  bool             debitSelected();
  void             setDebitSelected(bool val);
  bool             dateSelected();
  void             setDateSelected(bool val);
  bool             payeeSelected();
  void             setPayeeSelected(bool val);
  void             setMemoSelected(bool val);
  void             setNumberSelected(bool val);
  void             setCurrentUI(QString val);

  int              maxColumnCount();
  void             setMaxColumnCount(int val);

  int              amountColumn();
  int              debitColumn();
  int              creditColumn();
  int              dateColumn();
  int              payeeColumn();
  int              numberColumn();
  int              memoColumn();
  int              categoryColumn();

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

  void           redrawWindow(int startLine);
  void           showStage();

  /**
   *  Clear cells background.
   */
  void           clearCellsBackground();
  void           clearColumnTypeList();
  void           setMemoColSelections();

  int            endColumn();
  int            fieldDelimiterIndex();
  int            lastLine();
  int            fileLastLine();
  int            startLine();
  void           setStartLine(int);

  bool           importNow();

  QPixmap        m_iconBack;
  QPixmap        m_iconCancel;
  QPixmap        m_iconCSV;
  QPixmap        m_iconFinish;
  QPixmap        m_iconImport;

  QPixmap        m_iconNext;
  QPixmap        m_iconQIF;

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
  void           slotIdChanged(int id);
  void           slotNamesEdited();
  void           slotBackButtonClicked();
  void           slotVertScrollBarMoved(int val);

  /**
    * This method is called when the user clicks 'Open File', and opens
    * a file selector dialog.
    */
  void           slotFileDialogClicked();

  /**
  * This method is called when the user selects a new field or text delimiter.  The
  * input file is reread using the new delimiter.
  */
  void           delimiterChanged();

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
  QStringList      m_columnTypeList;  //  holds field types - date, payee, etc.

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

  bool             m_amountSelected;
  bool             m_creditSelected;
  bool             m_dateSelected;
  bool             m_debitSelected;
  bool             m_duplicate;
  bool             m_memoSelected;
  bool             m_numberSelected;
  bool             m_payeeSelected;
  bool             m_memoColCopied;
  bool             m_payeeColCopied;
  bool             m_payeeColAdded;
  bool             m_categorySelected;
  bool             m_clearAll;
  bool             m_firstIsValid;
  bool             m_secondIsValid;

  int              m_amountColumn;
  int              m_creditColumn;
  int              m_dateColumn;
  int              m_debitColumn;
  int              m_memoColumn;
  int              m_numberColumn;
  int              m_payeeColumn;
  int              m_categoryColumn;
  int              m_previousColumn;
  int              m_maxColumnCount;
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

  QBrush           m_clearBrush;
  QBrush           m_colorBrush;
  QBrush           m_errorBrush;
  QColor           m_clearColor;
  QColor           m_setColor;
  QColor           m_errorColor;

  QList<QLabel*>   m_stageLabels;
  QList<int>       m_memoColList;

  void             closeEvent(QCloseEvent *event);
  bool             eventFilter(QObject *object, QEvent *event);
  void             restoreBackground();

  /**
   * Check that the debit and credit field combination
   * is valid.
   */
  int              ensureBothFieldsValid(int);
  int              validateColumn(const int& col, QString& type);

private slots:
  /**
  * This method is called when the user clicks 'Encoding' and selects an
  * encoding setting.  The file is re-read with the corresponding codec.
  */
  void           encodingChanged(int index);

  /**
  * This method is called when the user clicks 'Clear selections'.
  * All column selections are cleared.
  */
  void           clearColumnsSelected();

  /**
  * This method is called when the amountRadio button is clicked.
  * It will disable all elements of the alternate, debit/credit ui.
  */
  void           amountRadioClicked(bool checked);

  /**
  * This method is called when the Amount column is activated.
  * It will validate the column selection.
  */
  void           amountColumnSelected(int);

  /**
  * This method is called when the Date column is activated.
  * It will validate the column selection.
  */
  void           dateColumnSelected(int col);

  /**
  * This method is called when the debitCreditRadio button is clicked.
  * It will disable all elements of the alternate, amount ui.
  */
  void           debitCreditRadioClicked(bool checked);

  /**
  * This method is called when the Credit column is activated.
  * It will validate the column selection.
  */
  void           creditColumnSelected(int);

  /**
  * This method is called when the Debit column is activated.
  * It will validate the column selection.
  */
  void           debitColumnSelected(int);

  /**
  * This method is called when the user edits the lastLine setting.
  */
  void           endLineChanged(int val);

  /**
  * This method is called when the user edits the startLine setting.
  */
  void           startLineChanged(int val);

  /**
  * This method is called when the Memo column is activated.
  * Multiple columns may be selected sequentially.
  */
  void           memoColumnSelected(int);

  /**
  * This method is called when the Number column is activated.
  * It will validate the column selection.
  */
  void           numberColumnSelected(int);

  /**
  * This method is called when the Payee column is activated.
  * It will validate the column selection.
  */
  void           payeeColumnSelected(int);

  /**
  * This method is called when the Category column is activated.
  * It will validate the column selection.
  */
  void           categoryColumnSelected(int);

  /**
  * This method is called when 'Cancel' is clicked.  Unless the user chooses
  * to continue, no settings will be saved, and the plugin will be terminated.
  */
  void           slotCancel();

  /**
  * This method is called when 'Quit' is clicked.  The plugin settings will
  * be saved and the plugin will be terminated.
  */
  void           slotClose();

  /**
    * This method is called when the user selects a new thousands separator.  The
    * UI is updated using the new symbol.
    */
  void           thousandsSeparatorChanged();

  /**
  * This method is called when it is detected that the user has selected the
  * same column for two different fields.  The column detecting the error
  * has to reset the other column.
  */
  void           resetComboBox(const QString& comboBox, const int& col);
};

//----------------------------------------------------------------------------------------------------------------------

namespace Ui
{
class IntroPage;
}

class IntroPage : public QWizardPage
{
  Q_OBJECT

public:
  explicit IntroPage(QWidget *parent = 0);
  ~IntroPage();

  void                initializePage();
  void                setParent(CSVDialog* dlg);

  QVBoxLayout*        m_pageLayout;

  QString             m_activity;

  Ui::IntroPage       *ui;
  CSVDialog           *m_dlg;
  QStringList         m_sourceList;

  bool                m_set;

  int                 addItem(QString txt);
  int                 m_index;

  QMap<QString, int>  m_map;
  QMap<QString, QString>  m_mapFileType;

signals:
  void             signalBankClicked(bool);
  void             activated(int);
  void             returnPressed();
  bool             isSet();

private:
  bool             validatePage();
  bool             m_messageBoxJustCancelled;
  bool             m_firstEdit;
  bool             m_editAccepted;
  bool             m_addRequested;
  bool             m_firstLineEdit;

  int              m_priorIndex;
  int              editProfileName(QString& fromName, QString& toName);

  QString          m_name;
  QString          m_priorName;
  QString          m_action;
  QString          m_newProfileCreated;
  QString          m_lastRadioButton;

  void             addProfileName();

private slots:
  void             slotComboEditTextChanged(QString txt);
  void             slotComboSourceClicked(int index);
  void             slotLineEditingFinished();
  void             slotRadioButton_bankClicked();
  void             slotRadioButton_investClicked();
};


namespace Ui
{
class SeparatorPage;
}

class SeparatorPage : public QWizardPage
{
  Q_OBJECT

public:
  explicit SeparatorPage(QWidget *parent = 0);
  ~SeparatorPage();

  QVBoxLayout         *m_pageLayout;
  Ui::SeparatorPage   *ui;
  CSVDialog*          m_dlg;
  void                setParent(CSVDialog* dlg);
  void                initializePage();
  bool                isComplete() const;

public slots:
  void                delimiterActivated();

signals:
  void                completeChanged();

private:
  void                cleanupPage();
  bool                validatePage();
  int                 nextId() const;

private slots:

signals:
};

namespace Ui
{
class BankingPage;
}

class BankingPage : public QWizardPage
{
  Q_OBJECT

public:
  explicit BankingPage(QWidget *parent = 0);
  ~BankingPage();

  Ui::BankingPage     *ui;
  QVBoxLayout         *m_pageLayout;

  bool                m_bankingPageInitialized;
  void                setParent(CSVDialog* dlg);

signals:
  void                clicked();

private:
  CSVDialog*          m_dlg;

  void                initializePage();
  void                cleanupPage();
  int                 nextId() const;
  bool                isComplete() const;
  bool                m_reloadNeeded;

private slots:
  void                slotDateColChanged(int col);
  void                slotPayeeColChanged(int col);
  void                slotDebitColChanged(int col);
  void                slotCreditColChanged(int col);
  void                slotAmountColChanged(int col);
  void                slotCategoryColChanged(int col);
};

namespace Ui
{
class InvestmentPage;
}

class InvestmentPage : public QWizardPage
{
  Q_OBJECT

public:
  explicit InvestmentPage(QWidget *parent = 0);
  ~InvestmentPage();

  QVBoxLayout         *m_pageLayout;
  Ui::InvestmentPage  *ui;

  bool                m_investPageInitialized;
  void                setParent(CSVDialog* dlg);

signals:

public slots:
  void                slotsecurityNameChanged(int index);
private:
  CSVDialog*          m_dlg;

  bool                isComplete() const;
  void                initializePage();
  void                cleanupPage();

private slots:
  void                slotDateColChanged(int col);
  void                slotTypeColChanged(int col);
  void                slotQuantityColChanged(int col);
  void                slotPriceColChanged(int col);
  void                slotAmountColChanged(int col);
  void                slotSymbolColChanged(int col);
  void                slotDetailColChanged(int col);
  void                slotFilterEditingFinished();
};

namespace Ui
{
class LinesDatePage;
}

class LinesDatePage : public QWizardPage
{
  Q_OBJECT

public:
  explicit LinesDatePage(QWidget *parent = 0);
  ~LinesDatePage();

  QVBoxLayout         *m_pageLayout;

  Ui::LinesDatePage   *ui;

  void                initializePage();
  void                setParent(CSVDialog* dlg);
  bool                validatePage();
  int                 nextId() const;
  bool                m_isColumnSelectionComplete;

  int                 m_trailerLines;

signals:
  bool                isImportable();
private:
  CSVDialog           *m_dlg;

};

namespace Ui
{
class CompletionPage;
}

class CompletionPage : public QWizardPage
{
  Q_OBJECT

public:
  explicit CompletionPage(QWidget *parent = 0);
  ~CompletionPage();

  QVBoxLayout*        m_pageLayout;
  Ui::CompletionPage  *ui;
  void                setParent(CSVDialog* dlg);
  void                initializePage();

signals:
  void                completeChanged();
  void                importBanking();
  void                importInvestment();

public slots:
  /**
  * This method is called when the user clicks the 'Import CSV' button.
  */
  void                slotImportClicked();
  void                slotImportValid();

private:

  void                cleanupPage();

  bool                validatePage();
  CSVDialog           *m_dlg;


private slots:

};

#endif // CSVDIALOG_H

