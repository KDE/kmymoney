/*******************************************************************************
*                                 csvwizard.h
*                              ------------------
* begin                       : Thur Jan 01 2015
* copyright                   : (C) 2015 by Allan Anderson
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

#ifndef CSVWIZARD_H
#define CSVWIZARD_H

#include <QtGui/QWidget>
#include <QtGui/QWizard>
#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>
#include <QtGui/QScrollBar>

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
  InvestProcessing*   m_investProcessing;
  QWizard*         m_wizard;
  IntroPage*       m_pageIntro;
  SeparatorPage*   m_pageSeparator;
  BankingPage*     m_pageBanking;
  InvestmentPage*  m_pageInvestment;
  LinesDatePage*   m_pageLinesDate;
  CompletionPage*  m_pageCompletion;
  CSVDialog*       m_csvDialog;

  QPixmap        m_iconBack;
  QPixmap        m_iconCancel;
  QPixmap        m_iconCSV;
  QPixmap        m_iconFinish;
  QPixmap        m_iconImport;
  QPixmap        m_iconNext;
  QPixmap        m_iconQIF;

  QList<QLabel*>   m_stageLabels;

  /**
  * This method is called after startup, to initialise some parameters.
  */
  void           init();

  void           showStage();

public slots:

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
  * This method is called when it is detected that the user has selected the
  * same column for two different fields.  The column detecting the error
  * has to reset the other column.
  */
  void           resetComboBox(const QString& comboBox, const int& col);

  void           slotIdChanged(int id);

private:
  int              m_curId;
  int              m_lastId;

  bool             eventFilter(QObject *object, QEvent *event);
  void             resizeEvent(QResizeEvent* ev);
}
;

namespace Ui
{
class IntroPage;
}

class CSVWizardPage : public QWizardPage {
public:
  CSVWizardPage(QWidget *parent = 0) : QWizardPage(parent), m_wizDlg(0) {}

  virtual void setParent(CSVWizard* dlg) { m_wizDlg = dlg; }

protected:
  CSVWizard*         m_wizDlg;
};

class IntroPage : public CSVWizardPage
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

class SeparatorPage : public CSVWizardPage
{
  Q_OBJECT

public:
  explicit SeparatorPage(QWidget *parent = 0);
  ~SeparatorPage();

  Ui::SeparatorPage   *ui;

  QVBoxLayout         *m_pageLayout;

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

class BankingPage : public CSVWizardPage
{
  Q_OBJECT

public:
  explicit BankingPage(QWidget *parent = 0);
  ~BankingPage();

  Ui::BankingPage     *ui;
  QVBoxLayout         *m_pageLayout;

  bool                m_bankingPageInitialized;

  void                initializePage();

signals:
  void                clicked();

private:
  void                cleanupPage();
  int                 nextId() const;
  bool                isComplete() const;

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

class InvestmentPage : public CSVWizardPage
{
  Q_OBJECT

public:
  explicit InvestmentPage(QWidget *parent = 0);
  ~InvestmentPage();

  Ui::InvestmentPage  *ui;

  QVBoxLayout         *m_pageLayout;

  bool                m_investPageInitialized;
  void                initializePage();

signals:

public slots:
  void                slotsecurityNameChanged(int index);
private:
  bool                isComplete() const;
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

class LinesDatePage : public CSVWizardPage
{
  Q_OBJECT

public:
  explicit LinesDatePage(QWidget *parent = 0);
  ~LinesDatePage();

  Ui::LinesDatePage   *ui;

  QVBoxLayout         *m_pageLayout;

  void                initializePage();
  bool                validatePage();
  int                 nextId() const;
  bool                m_isColumnSelectionComplete;

  int                 m_trailerLines;

signals:
  bool                isImportable();

private:
  void                cleanupPage();

};

namespace Ui
{
class CompletionPage;
}

class CompletionPage : public CSVWizardPage
{
  Q_OBJECT

public:
  explicit CompletionPage(QWidget *parent = 0);
  ~CompletionPage();

  Ui::CompletionPage  *ui;

  QVBoxLayout*        m_pageLayout;

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
};

#endif // CSVWIZARD_H
