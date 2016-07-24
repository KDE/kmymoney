/*******************************************************************************
*                                 csvwizard.h
*                              ------------------
* begin                       : Thur Jan 01 2015
* copyright                   : (C) 2015 by Allan Anderson
* email                       : agander93@gmail.com
* copyright                   : (C) 2016 by Łukasz Wojniłowicz
* email                       : lukasz.wojnilowicz@gmail.com
********************************************************************************/

/*******************************************************************************
*                                                                              *
*   This program is free software; you can redistribute it and/or modify       *
*   it under the terms of the GNU General Public License as published by       *
*   the Free Software Foundation; either version 2 of the License, or          *
*   (at your option) any later version.                                        *
*                                                                              *
********************************************************************************/

#ifndef CSVWIZARD_H
#define CSVWIZARD_H

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
class InvestProcessing;
class CsvImporterPlugin;

namespace Ui
{
class CSVWizard;
}

class CSVWizard : public QWidget
{
  Q_OBJECT

public:
  explicit CSVWizard();
  virtual ~CSVWizard();

  enum { Page_Intro, Page_Separator, Page_Banking, Page_Investment,
         Page_LinesDate, Page_Completion
       };

  Ui::CSVWizard*   ui;
  QWizard*            m_wizard;
  IntroPage*          m_pageIntro;
  SeparatorPage*      m_pageSeparator;
  BankingPage*        m_pageBanking;
  InvestmentPage*     m_pageInvestment;
  LinesDatePage*      m_pageLinesDate;
  CompletionPage*     m_pageCompletion;
  CSVDialog*          m_csvDialog;
  InvestProcessing*   m_investProcessing;
  ConvertDate*        m_convertDate;
  CsvUtil*            m_csvUtil;
  Parse*              m_parse;
  CsvImporterPlugin*  m_plugin;

  QPixmap        m_iconBack;
  QPixmap        m_iconCancel;
  QPixmap        m_iconCSV;
  QPixmap        m_iconFinish;
  QPixmap        m_iconImport;
  QPixmap        m_iconNext;
  QPixmap        m_iconQIF;

  QList<QLabel*>   m_stageLabels;
  QScrollBar*      m_vScrollBar;
  QList<QTextCodec *>   m_codecs;
  QComboBox*     m_comboBoxEncode;

  QBrush           m_clearBrush;
  QBrush           m_clearBrushText;
  QBrush           m_colorBrush;
  QBrush           m_colorBrushText;
  QBrush           m_errorBrush;
  QBrush           m_errorBrushText;

  QString          m_fieldDelimiterCharacter;
  QString          m_textDelimiterCharacter;
  QString          m_decimalSymbol;
  QString          m_thousandsSeparator;
  QString          m_inFileName;
  QString          m_fileType;
  QString          m_date;
  QString          m_profileName;
  QString          m_priorCsvProfile;
  QString          m_priorInvProfile;
  QStringList      m_lineList;
  QStringList      m_dateFormats;
  QStringList      m_profileList;
  QList<int>       m_memoColList;

  int              m_initialHeight;
  int              m_initialWidth;
  int              m_pluginHeight;
  int              m_pluginWidth;
  int              m_fieldDelimiterIndex;
  int              m_textDelimiterIndex;
  int              m_decimalSymbolIndex;
  int              m_ThousandsSeparatorIndex;
  int              m_row;
  int              m_maxColumnCount;
  int              m_endColumn;
  int              m_encodeIndex;
  int              m_startLine;
  int              m_endLine;
  int              m_fileEndLine;
  int              m_dateFormatIndex;
  int              m_memoColumn;
  int              m_dateColumn;

  bool             m_accept;
  bool             m_importError;
  bool             m_importIsValid;
  bool             m_errorFoundAlready;
  bool             m_importNow;
  bool             m_columnsNotSet;
  bool             m_skipSetup;
  bool             m_acceptAllInvalid;

  /**
  * This method is called after startup, to initialise some parameters.
  */
  void           init();

  void           showStage();

  /**
  * This method is called when the user chooses to add a new profile, It achieves this by copying
  * the necessary basic parameters from an existing profile called "Profiles-New Profile###"
  * in the resource file,
  */
  void           createProfile(QString newName);

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
  * This method is called on opening the plugin.
  * It will add all codec names to the encoding combobox.
  */
  void           setCodecList(const QList<QTextCodec *> &list);

  /**
  * This method is called on opening the plugin.
  * It will populate a list with all available codecs.
  */
  void           findCodecs();

public slots:

  void           clearBackground();
  void           markUnwantedRows();

  /**
  * This method is called when the user selects a new decimal symbol.  The
  * UI is updated using the new symbol.
  */
  void           decimalSymbolSelected(int);
  void           decimalSymbolSelected();

  /**
  * This method is called when the user selects a new decimal symbol.  The
  * UI is updated using the new symbol, and on importing, the new symbol
  * also will be used.
  */
  void           updateDecimalSymbol(int col);

  void           thousandsSeparatorChanged();

  /**
  * This method is called when a field or text delimiter is changed.  The
  * input file is reread using the new delimiter.
  */
  void           delimiterChanged(int index);

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
  * This method is called when the oppositeSignsCheckBox checkbox is clicked.
  * It will set m_oppositeSigns.
  */
  void           oppositeSignsCheckBoxClicked(bool checked);

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
  * This method is called when 'Exit' is clicked.  The plugin settings will
  * be saved and the plugin will be terminated.
  */
  void           slotClose();

  /**
  * This method checks if all dates in date column are valid.
  */
  bool            validateDateFormat(int dF);

  /**
  * If delimiter = -1 this method tries different fild
  * delimiters to get the one with which file has the most columns.
  * Otherwise it gets only column count for specified delimiter.
  */
  int            getMaxColumnCount(QStringList &lineList, int &delimiter);

  /**
  * This method gets file into buffer
  * It will laso store file's end column and row.
  */
  void           readFile(const QString& fname);

  /**
  * It will display lines list in the UI table widget.
  */
  void           displayLines(const QStringList &lineList, Parse *parse);

  /**
  * Called in order to adjust window size to suit the file,
  */
  void           updateWindowSize();

  /**
  * Appends memo field in lines buffer,
  */
  void           createMemoField(QStringList &columnTypeList);

  /**
  * This method is called when it is detected that the user has selected the
  * same column for two different fields.  The column detecting the error
  * has to reset the other column.
  */
  void           resetComboBox(const QString& comboBox, const int& col);

  void           slotIdChanged(int id);

private:
  int              m_curId;
  int              m_lastId;

  void             closeEvent(QCloseEvent *event);
  void             resizeEvent(QResizeEvent* ev);
}
;

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

  Ui::IntroPage       *ui;
  void                initializePage();
  void                setParent(CSVWizard* dlg);

  QVBoxLayout*        m_pageLayout;

  QString             m_activity;
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
  CSVWizard*       m_wizDlg;
  bool             validatePage();
  int              nextId() const;
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

  Ui::SeparatorPage   *ui;

  QVBoxLayout         *m_pageLayout;

  void                setParent(CSVWizard* dlg);
  void                initializePage();
  bool                isComplete() const;

public slots:
  void                delimiterActivated();

signals:
  void                completeChanged();

private:
  CSVWizard*          m_wizDlg;
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
  void                setParent(CSVWizard* dlg);

  void                initializePage();

signals:
  void                clicked();

private:
  CSVWizard*          m_wizDlg;


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

  Ui::InvestmentPage  *ui;

  QVBoxLayout         *m_pageLayout;

  bool                m_investPageInitialized;
  void                setParent(CSVWizard* dlg);
  void                initializePage();

signals:

public slots:
  void                slotsecurityNameChanged(int index);
private:
  CSVWizard*          m_wizDlg;

  bool                isComplete() const;
  void                cleanupPage();

private slots:
  void                slotDateColChanged(int col);
  void                slotTypeColChanged(int col);
  void                slotQuantityColChanged(int col);
  void                slotPriceColChanged(int col);
  void                slotAmountColChanged(int col);
  void                slotSymbolColChanged(int col);
  void                slotNameColChanged(int col);
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

  Ui::LinesDatePage   *ui;

  QVBoxLayout         *m_pageLayout;

  void                initializePage();
  void                setParent(CSVWizard* dlg);
  bool                isComplete() const;
  bool                validatePage();
  int                 nextId() const;

  int                 m_trailerLines;

signals:
  bool                isImportable();

public slots:
  /**
  * This method is called when the user clicks 'Date format' and selects a
  * format, which is used by convertDate().
  */
  void           dateFormatSelected(int dF);

  /**
  * This method is called when the user edits the startLine setting.
  */
  void           startLineChanged(int val);

  /**
  * This method is called when the user edits the lastLine setting.
  */
  void           endLineChanged(int val);
private:
  CSVWizard*          m_wizDlg;

  void                cleanupPage();

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

  Ui::CompletionPage  *ui;

  QVBoxLayout*        m_pageLayout;

  void                setParent(CSVWizard* dlg);
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
  CSVWizard*          m_wizDlg;
};

#endif // CSVWIZARD_H
