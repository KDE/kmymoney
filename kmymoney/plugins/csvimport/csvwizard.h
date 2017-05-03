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

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>
#include <QWizard>
#include <QLabel>
#include <QScrollBar>
#include <QVBoxLayout>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KSharedConfig>

// ----------------------------------------------------------------------------
// Project Includes

#include "transactiondlg.h"
#include "securitydlg.h"
#include "securitiesdlg.h"
#include "currenciesdlg.h"

//#include "csvimporter.h"
#include "csvimporterplugin.h"
#include "bankingwizardpage.h"
#include "investmentwizardpage.h"
#include "priceswizardpage.h"


class CsvImporterPlugin;
class CSVImporter;

class TransactionDlg;
class SecurityDlg;
class SecuritiesDlg;
class CurrenciesDlg;

class BankingProfile;
class InvestmentProfile;
class PricesProfile;

class IntroPage;
class SeparatorPage;
class RowsPage;
class BankingPage;
class InvestmentPage;
class PricesPage;
class FormatsPage;

namespace Ui
{
class CSVWizard;
}

class CSVWizard : public QDialog
{
  Q_OBJECT

public:
  CSVWizard(CsvImporterPlugin *plugin, CSVImporter *importer);
  ~CSVWizard();

  enum wizardPageE  { PageIntro, PageSeparator, PageRows,
                      PageBanking, PageInvestment, PagePrices, PageFormats
                    };

  Ui::CSVWizard*   ui;

  CsvImporterPlugin*  m_plugin;
  CSVImporter*        m_imp;
  QWizard*            m_wiz;

  IntroPage                   *m_pageIntro;
  SeparatorPage               *m_pageSeparator;
  RowsPage                    *m_pageRows;
  QPointer<BankingPage>        m_pageBanking;
  QPointer<InvestmentPage>     m_pageInvestment;
  QPointer<PricesPage>         m_pagePrices;
  FormatsPage                 *m_pageFormats;

  MyMoneyStatement m_st;

  QList<QLabel *>  m_stageLabels;
  QScrollBar      *m_vScrollBar;

  QBrush           m_clearBrush;
  QBrush           m_clearBrushText;
  QBrush           m_colorBrush;
  QBrush           m_colorBrushText;
  QBrush           m_errorBrush;
  QBrush           m_errorBrushText;

  int              m_initialHeight;
  int              m_initialWidth;

  QMap <columnTypeE, QString> m_colTypeName;
  bool             m_skipSetup;

  void           showStage();

  void           clearColumnsBackground(const int col);
  void           clearColumnsBackground(const QList<int> &columnList);
  void           clearBackground();
  void           markUnwantedRows();

public slots:

  /**
  * This method is called when 'Exit' is clicked.  The plugin settings will
  * be saved and the plugin will be terminated.
  */
  void slotClose();

  /**
  * Called in order to adjust window size to suit the file,
  */
  void updateWindowSize();

  void readWindowSize(const KSharedConfigPtr& config);
  void saveWindowSize(const KSharedConfigPtr& config);


  void slotIdChanged(int id);

  void fileDialogClicked();
  void importClicked();
  void saveAsQIFClicked();
private:
  int m_curId;
  int m_lastId;

  void closeEvent(QCloseEvent *event);
  bool eventFilter(QObject *object, QEvent *event);
//  void             resizeEvent(QResizeEvent* ev);
}
;

namespace Ui
{
class IntroPage;
}

class IntroPage : public CSVWizardPage
{
  Q_OBJECT

public:
  explicit IntroPage(CSVWizard *dlg, CSVImporter *imp);
  ~IntroPage();

  Ui::IntroPage       *ui;
  void                initializePage();
//  void                setParent(CSVWizard* dlg, CSVImporter *imp);

  QVBoxLayout*        m_pageLayout;
  profileTypeE          m_profileType;

signals:
  void             signalBankClicked(bool);
  void             activated(int);
  void             returnPressed();

private:
  bool             validatePage();
  int              nextId() const;

  QStringList      m_profiles;

  void             profileChanged(const profilesActionE action);
  void             profileTypeChanged(const profileTypeE profileType, bool toggled);

public slots:

private slots:
  void             slotAddProfile();
  void             slotRemoveProfile();
  void             slotRenameProfile();
  void             slotComboSourceIndexChanged(int idx);
  void             slotBankRadioToggled(bool toggled);
  void             slotInvestRadioToggled(bool toggled);
  void             slotCurrencyPricesRadioToggled(bool toggled);
  void             slotStockPricesRadioToggled(bool toggled);
};

namespace Ui
{
class SeparatorPage;
}

class SeparatorPage : public CSVWizardPage
{
  Q_OBJECT

public:
  explicit SeparatorPage(CSVWizard *dlg, CSVImporter *imp);
  ~SeparatorPage();

  Ui::SeparatorPage   *ui;

  QVBoxLayout         *m_pageLayout;

  void                initializePage();
  bool                isComplete() const;

public slots:
  void           encodingChanged(const int index);
  void           fieldDelimiterChanged(const int index);
  void           textDelimiterChanged(const int index);

signals:
  void                completeChanged();

private:
  void                initializeEncodingCombobox();
  void                cleanupPage();
  bool                validatePage();

private slots:

signals:
};

namespace Ui
{
class RowsPage;
}

class RowsPage : public CSVWizardPage
{
  Q_OBJECT

public:
  explicit RowsPage(CSVWizard *dlg, CSVImporter *imp);
  ~RowsPage();

  Ui::RowsPage   *ui;

  QVBoxLayout         *m_pageLayout;

  void                initializePage();
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
  void                cleanupPage();
};



namespace Ui
{
class FormatsPage;
}

class FormatsPage : public CSVWizardPage
{
  Q_OBJECT

public:
  explicit FormatsPage(CSVWizard *dlg, CSVImporter *imp);
  ~FormatsPage();

  Ui::FormatsPage   *ui;

  QVBoxLayout         *m_pageLayout;

  void                initializePage();

  /**
  * This method is called when the user selects a new decimal symbol.  The
  * UI is updated using the new symbol, and on importing, the new symbol
  * also will be used.
  */
  bool validateDecimalSymbols(const QList<int> &columns);

  /**
  * This method checks if all dates in date column are valid.
  */
  bool            validateDateFormat(const int index);

signals:
void                completeChanged();
public slots:


  void           decimalSymbolChanged(int);
  void           dateFormatChanged(const int index);
private:
  bool                m_isDecimalSymbolOK;
  bool                m_isDateFormatOK;

  bool                isComplete() const;
  void                cleanupPage();
};

#endif // CSVWIZARD_H
