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

#include <QDialog>
#include <QWizard>
#include <QLabel>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QUrl>

#include <QComboBox>
#include <KSharedConfig>

#include "csvimporterplugin.h"
#include "investmentwizardpage.h"

class ConvertDate;
class Parse;
class CsvUtil;
class BankingPage;
class FormatsPage;
class IntroPage;
class SeparatorPage;
class RowsPage;
class CsvImporterPlugin;

namespace Ui
{
class CSVWizard;
}

class CSVWizard : public QDialog
{
  Q_OBJECT

public:
  explicit CSVWizard();
  virtual ~CSVWizard();

  enum { PageIntro, PageSeparator, PageRows,
         PageBanking, PageInvestment, PageFormats
       };

  typedef enum:int { ProfileInvest, ProfileBank, ProfileNone = 0xFF,
       } profileTypeE;

  typedef enum:uchar { AutoFieldDelimiter, AutoDecimalSymbol, AutoDateFormat,
                       AutoAccountInvest, AutoAccountBank,
       } autodetectTypeE;

  Ui::CSVWizard*   ui;
  QWizard*            m_wizard;
  IntroPage*          m_pageIntro;
  SeparatorPage*      m_pageSeparator;
  RowsPage*           m_pageRows;
  BankingPage*        m_pageBanking;
  QPointer<InvestmentPage>     m_pageInvestment;
  FormatsPage*        m_pageFormats;
  QPointer<CSVDialog> m_csvDialog;
  ConvertDate*        m_convertDate;
  CsvUtil*            m_csvUtil;
  Parse*              m_parse;
  CsvImporterPlugin*  m_plugin;

  MyMoneyStatement st;

  KSharedConfigPtr m_config;

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

  QBrush           m_clearBrush;
  QBrush           m_clearBrushText;
  QBrush           m_colorBrush;
  QBrush           m_colorBrushText;
  QBrush           m_errorBrush;
  QBrush           m_errorBrushText;

  QString          m_fieldDelimiterCharacter;
  QString          m_textDelimiterCharacter;
  QString          m_decimalSymbol;
  QString          m_inFileName;
  int              m_profileType;
  QString          m_date;
  QString          m_profileName;
  QStringList      m_lineList;
  QStringList      m_dateFormats;
  QStringList      m_profileList;
  QList<int>       m_memoColList;
  QUrl             m_url;

  int              m_initialHeight;
  int              m_initialWidth;
  int              m_fieldDelimiterIndex;
  int              m_textDelimiterIndex;
  int              m_decimalSymbolIndex;
  QMap<int, int>   m_decimalSymbolIndexMap;
  int              m_row;
  int              m_maxColumnCount;
  int              m_endColumn;
  int              m_encodeIndex;
  int              m_startLine;
  int              m_endLine;
  int              m_trailerLines;
  int              m_fileEndLine;
  int              m_dateFormatIndex;
  int              m_memoColumn;
  int              m_dateColumn;

  QMap<autodetectTypeE, bool> m_autodetect;

  bool             m_accept;
  bool             m_importError;
  bool             m_skipSetup;
  bool             m_acceptAllInvalid;

  /**
  * This method is called after startup, to initialise some parameters.
  */
  void           init();

  void           showStage();

  void           readMiscSettings(const KSharedConfigPtr& config);
  void           saveWindowSize(const KSharedConfigPtr& config);

  /**
  * This method contains routines to update configuration file
  * from kmmVer to latest.
  */
  bool           updateConfigFile(const KSharedConfigPtr &config, const QList<int> &kmmVer);

  /**
  * This method ensures that configuration file contains all neccesary fields
  * and that it is up to date.
  */
  void           validateConfigFile(const KSharedConfigPtr &config);

  /**
  * This method is called on opening the plugin.
  * It will add all codec names to the encoding combobox.
  */
  void           setCodecList(const QList<QTextCodec *> &list, QComboBox *comboBoxEncode);

  /**
  * This method is called on opening the plugin.
  * It will populate a list with all available codecs.
  */
  void           findCodecs();

  void           clearColumnsBackground(int col);
  void           clearColumnsBackground(QList<int>& columnList);
  void           clearBackground();
  void           markUnwantedRows();
  QList<MyMoneyAccount> findAccounts(QList<MyMoneyAccount::accountTypeE> &accountTypes, QString& statementHeader);
  bool                detectAccount(MyMoneyStatement& st);

signals:
  void           statementReady(MyMoneyStatement&);
public slots:

  /**
  * This method is called when the user clicks 'Encoding' and selects an
  * encoding setting.  The file is re-read with the corresponding codec.
  */
  void           encodingChanged(int);

  bool           detectDecimalSymbol(const int col, int& symbol);

  /**
  * This method is called when the amountRadio button is clicked.
  * It will disable all elements of the alternate, debit/credit ui.
  */
  void           amountRadioClicked(bool checked);

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
  * This method is called when 'Exit' is clicked.  The plugin settings will
  * be saved and the plugin will be terminated.
  */
  void           slotClose();

  /**
  * If delimiter = -1 this method tries different fild
  * delimiters to get the one with which file has the most columns.
  * Otherwise it gets only column count for specified delimiter.
  */
  int            getMaxColumnCount(QStringList &lineList, int &delimiter);

  /**
  * This method gets the filename of
  * the financial statement.
  */
  bool getInFileName(QString &startDir);

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
  * This method is called when the user clicks 'Open File'. It ends
  * in importing ready bank statement.
  */
  void           slotFileDialogClicked();

  void           slotIdChanged(int id);

private:
  int              m_curId;
  int              m_lastId;

  void             closeEvent(QCloseEvent *event);
  bool             eventFilter(QObject *object, QEvent *event);
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
  explicit IntroPage(QDialog *parent = 0);
  ~IntroPage();

  Ui::IntroPage       *ui;
  void                initializePage();
  void                setParent(CSVWizard* dlg);

  QVBoxLayout*        m_pageLayout;

signals:
  void             signalBankClicked(bool);
  void             activated(int);
  void             returnPressed();

private:
  CSVWizard*       m_wizDlg;
  bool             validatePage();
  int              nextId() const;

  typedef enum:uchar { ProfileAdd, ProfileRemove, ProfileRename,
       } profileActionsE;

  void             profileChanged(const profileActionsE& action);
  void             profileTypeChanged(const CSVWizard::profileTypeE profileType, bool toggled);

private slots:
  void             slotAddProfile();
  void             slotRemoveProfile();
  void             slotRenameProfile();
  void             slotComboSourceIndexChanged(int idx);
  void             slotBankRadioToggled(bool toggled);
  void             slotInvestRadioToggled(bool toggled);
};


namespace Ui
{
class SeparatorPage;
}

class SeparatorPage : public QWizardPage
{
  Q_OBJECT

public:
  explicit SeparatorPage(QDialog *parent = 0);
  ~SeparatorPage();

  Ui::SeparatorPage   *ui;

  QVBoxLayout         *m_pageLayout;

  void                setParent(CSVWizard* dlg);
  void                initializePage();
  bool                isComplete() const;

public slots:
  /**
  * This method is called when a text delimiter is changed.
  */
  void           textDelimiterChanged(const int index);

  /**
  * This method is called when a field delimiter is changed.  The
  * input file is redisplayed using the new delimiter.
  */
  void           fieldDelimiterChanged(const int index);

signals:
  void                completeChanged();

private:
  CSVWizard*          m_wizDlg;
  void                cleanupPage();
  bool                validatePage();

private slots:

signals:
};

namespace Ui
{
class RowsPage;
}

class RowsPage : public QWizardPage
{
  Q_OBJECT

public:
  explicit RowsPage(QDialog *parent = 0);
  ~RowsPage();

  Ui::RowsPage   *ui;

  QVBoxLayout         *m_pageLayout;

  void                initializePage();
  void                setParent(CSVWizard* dlg);
  int                 nextId() const;

signals:

public slots:
  /**
  * This method is called when the user edits the startLine setting.
  */
  void           startRowChanged(int val);

  /**
  * This method is called when the user edits the lastLine setting.
  */
  void           endRowChanged(int val);
private:
  CSVWizard*          m_wizDlg;

  void                cleanupPage();
};

namespace Ui
{
class BankingPage;
}

class BankingPage : public QWizardPage
{
  Q_OBJECT

public:
  explicit BankingPage(QDialog *parent = 0);
  ~BankingPage();

  Ui::BankingPage     *ui;
  QVBoxLayout         *m_pageLayout;

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
  void                clearColumnsSelected();
};

namespace Ui
{
class FormatsPage;
}

class FormatsPage : public QWizardPage
{
  Q_OBJECT

public:
  explicit FormatsPage(QDialog *parent = 0);
  ~FormatsPage();

  Ui::FormatsPage   *ui;

  QVBoxLayout         *m_pageLayout;

  void                initializePage();
  void                setParent(CSVWizard* dlg);

  /**
  * This method is called when the user selects a new decimal symbol.  The
  * UI is updated using the new symbol, and on importing, the new symbol
  * also will be used.
  */
  bool           validateDecimalSymbol(int col);

  /**
  * This method checks if all dates in date column are valid.
  */
  bool            validateDateFormat(int index);

signals:
void                completeChanged();
public slots:
  void           slotImportClicked();
  void           slotSaveAsQIFClicked();
  /**
  * This method is called when the user selects a new decimal symbol.  The
  * UI is updated using the new symbol.
  */
  void           decimalSymbolChanged(int);

  /**
  * This method is called when the user clicks 'Date format' and selects a
  * format, which is used by convertDate().
  */
  void           dateFormatChanged(int index);
private:
  CSVWizard*          m_wizDlg;
  bool                m_isDecimalSymbolOK;
  bool                m_isDateFormatOK;

  bool                isComplete() const;
  void                cleanupPage();
};

#endif // CSVWIZARD_H
