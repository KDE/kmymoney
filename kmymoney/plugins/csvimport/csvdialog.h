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

#include <KComboBox>

#include "csvimporterplugin.h"

#define MAXCOL 25    //                 maximum no. of columns (arbitrary value)

class ConvertDate;
class Parse;
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

public:
  explicit CSVDialog(QWidget *parent = 0);
  ~CSVDialog();

  enum { Page_Intro, Page_Separator, Page_Banking, Page_Investment,
         Page_LinesDate, Page_Completion
       };

  QWizard*            m_wizard;
  CsvImporterPlugin*  m_plugin;
  CSVDialog*        m_csvDlg;
  InvestmentDlg*      m_investmentDlg;
  InvestProcessing*   m_investProcessing;
  ConvertDate*        m_convertDate;
  Parse*              m_parse;
  SymbolTableDlg*     m_symbolTableDlg;

  Ui::CSVDialog*      ui;
  QVBoxLayout*        m_wizardLayout;

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
  } m_trData;

  QList<MyMoneyStatement> statements;
  QList<QTextCodec *>     m_codecs;

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

  QString        m_csvPath;
  QString        m_date;
  QString        m_fieldDelimiterCharacter;
  QString        m_filename;
  QString        m_inBuffer;
  QString        m_inFileName;
  QString        m_outBuffer;
  QString        m_qifBuffer;
  QString        m_textDelimiterCharacter;
  QString        csvPath();
  QString        inFileName();
  QString        m_fileType;
  QString        m_detailFilter;

  bool           m_decimalSymbolChanged;
  bool           m_importNow;
  bool           m_showEmptyCheckBox;
  bool           m_accept;
  bool           m_screenUpdated;
  bool           m_goBack;
  bool           m_importIsValid;
  bool           m_isTableTrimmed;

  int            m_dateFormatIndex;
  int            m_debitFlag;
  int            m_encodeIndex;
  int            m_fieldDelimiterIndex;
  int            m_textDelimiterIndex;
  int            m_endColumn;
  int            m_flagCol;
  int            m_row;
  int            m_activityType;

  KUrl           m_url;
  KComboBox*     m_comboBoxEncode;

  QFile*         m_inFile;

  void             saveSettings();

  QString          columnType(int column);
  QString          decimalSymbol();
  QString          currentUI();

  void             clearColumnType(int column);
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
  int              amountColumn();
  void             setAmountColumn(int column);
  int              creditColumn();
  void             setCreditColumn(int column);

  void             setCurrentUI(QString val);

  int              debitColumn();
  void             setDebitColumn(int val);
  int              maxColumnCount();
  void             setMaxColumnCount(int val);

  void             setupNextPage();

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

  void           clearColumnTypes();

  int            columnNumber(const QString& msg);

  /**
    * This method is called initially after an input file has been selected.
    * It will call other routines to display file content and to complete the
    * statement import. It will also be called to reposition the file after row
    * deletion, or to reread following encoding or delimiter change.
    */
  void           readFile(const QString& fname, int skipLines);

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
  * This method is called on opening, to load settings from the resource file.
  */
  void           readSettings();

  /**
  * This method is called to redraw the window according to the number of
  * columns and rows to be displayed.
  */
  void           updateScreen();
  void           showStage();

  int            endColumn();
  int            fieldDelimiterIndex();
  int            lastLine();
  int            startLine();
  int            textDelimiterIndex();

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

public slots:
  void           slotIdChanged(int id);
  void           slotNamesEdited();
  void           slotBackButtonClicked();

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

private:
  QString          m_columnType[MAXCOL];//  holds field types - date, payee, etc.
  QString          m_currentUI;
  QString          m_decimalSymbol;
  QString          m_previousType;
  QString          m_thousandsSeparator;

  bool             m_amountSelected;
  bool             m_creditSelected;
  bool             m_dateSelected;
  bool             m_debitSelected;
  bool             m_duplicate;
  bool             m_memoSelected;
  bool             m_numberSelected;
  bool             m_payeeSelected;

  int              m_amountColumn;
  int              m_creditColumn;
  int              m_dateColumn;
  int              m_debitColumn;
  int              m_memoColumn;
  int              m_numberColumn;
  int              m_payeeColumn;
  int              m_previousColumn;
  int              m_maxColumnCount;
  int              m_decimalSymbolIndex;
  int              m_endLine;
  int              m_startLine;
  int              m_curId;
  int              m_lastId;

  QBrush           m_clearBrush;
  QBrush           m_colorBrush;
  QBrush           m_errorBrush;
  QColor           m_clearColor;
  QColor           m_setColor;
  QColor           m_errorColor;

  QList<QLabel*>   m_stageLabels;

  int              nextId() const;

  void             closeEvent(QCloseEvent *event);
  void             resizeEvent(QResizeEvent * event);
  void             restoreBackground(/*int lastRow, int lastCol*/);
  int              validateColumn(const int& col, const QString& type);

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
  * This method is called when the user selects a new decimal symbol.  The
  * UI is updated using the new symbol.
  */
  void           decimalSymbolSelected(int val);

  /**
  * This method is called when the user edits the lastLine setting.
  */
  void           endLineChanged(int val);
  void           endLineChanged();

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
  IntroPage(QWidget *parent = 0);
  ~IntroPage();

  QVBoxLayout*        m_pageLayout;

  QString             m_activity;

  Ui::IntroPage       *ui;
  CSVDialog           *m_dlg;
  QStringList         m_sourceList;

  void                setParent(CSVDialog* dlg);

  bool                m_set;

signals:
  void             signalBankClicked(bool);
  void             activated(int);
  void             returnPressed();
  bool             isSet();

private:
  void             initializePage();
  bool             validatePage();

private slots:
  void             slotComboSourceClicked(int);
  void             slotSourceNameEdited();
};


namespace Ui
{
class SeparatorPage;
}

class SeparatorPage : public QWizardPage
{
  Q_OBJECT

public:
  SeparatorPage(QWidget *parent = 0);
  ~SeparatorPage();

  QVBoxLayout         *m_pageLayout;
  Ui::SeparatorPage   *ui;
  CSVDialog*          m_dlg;
  void                setParent(CSVDialog* dlg);

signals:

private:
  void                initializePage();
  void                cleanupPage();

  int                 nextId() const;
};

namespace Ui
{
class BankingPage;
}

class BankingPage : public QWizardPage
{
  Q_OBJECT

public:
  BankingPage(QWidget *parent = 0);
  ~BankingPage();

  Ui::BankingPage     *ui;
  QVBoxLayout         *m_pageLayout;
  void                setParent(CSVDialog* dlg);
  
signals:
  void                clicked();

private:
  CSVDialog*          m_dlg;

  void                initializePage();
  int                 nextId() const;
};

namespace Ui
{
class InvestmentPage;
}

class InvestmentPage : public QWizardPage
{
  Q_OBJECT

public:
  InvestmentPage(QWidget *parent = 0);
  ~InvestmentPage();

  QVBoxLayout         *m_pageLayout;
  Ui::InvestmentPage  *ui;
  void                setParent(CSVDialog* dlg);

signals:

private:
  CSVDialog*          m_dlg;

  bool                isComplete() const;
  void                initializePage();

private slots:
  void                slotDateColChanged(int col);
  void                slotTypeColChanged(int col);
  void                slotQuantityColChanged(int col);
  void                slotPriceColChanged(int col);
  void                slotAmountColChanged(int col);
  void                slotSymbolColChanged(int col);
  void                slotDetailColChanged(int col);
  void                slotsecurityNameChanged(int index);
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
  LinesDatePage(QWidget *parent = 0);
  ~LinesDatePage();

  QVBoxLayout         *m_pageLayout;

  Ui::LinesDatePage   *ui;

  void                initializePage();
  void                setParent(CSVDialog* dlg);

private:
  CSVDialog           *m_dlg;
  bool                validatePage();
};

namespace Ui
{
class CompletionPage;
}

class CompletionPage : public QWizardPage
{
  Q_OBJECT

public:
  CompletionPage(QWidget *parent = 0);
  ~CompletionPage();

  QVBoxLayout*        m_pageLayout;
  Ui::CompletionPage  *ui;
  void                setParent(CSVDialog* dlg);

signals:
  void                completeChanged();
  void                importBanking();
  void                importInvestment();

public slots:
  void                slotImportClicked();
  void                slotImportValid();

private:
  void                initializePage();
  void                cleanupPage();

  bool                validatePage();
  CSVDialog           *m_dlg;


private slots:

};

#endif // CSVDIALOG_H

