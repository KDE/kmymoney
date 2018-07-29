/*
 * Copyright 2015-2016  Allan Anderson <agander93@gmail.com>
 * Copyright 2016-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CSVWIZARD_H
#define CSVWIZARD_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QPointer>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KSharedConfig>

// ----------------------------------------------------------------------------
// Project Includes

#include "csvwizardpage.h"
#include "mymoneystatement.h"

class CSVImporter;
class CSVImporterCore;

class IntroPage;
class SeparatorPage;
class RowsPage;
class BankingPage;
class InvestmentPage;
class PricesPage;
class FormatsPage;

namespace Ui { class CSVWizard; }

class QScrollBar;
class QComboBox;
class QLabel;
class CSVWizard : public QDialog
{
  Q_OBJECT

public:
  explicit CSVWizard(CSVImporter *plugin);
  ~CSVWizard();

  enum wizardPageE  { PageIntro, PageSeparator, PageRows,
                      PageBanking, PageInvestment, PagePrices, PageFormats
                    };

  const MyMoneyStatement& statement() const;

  Ui::CSVWizard    *ui;
  MyMoneyStatement  m_st;
  QScrollBar       *m_vScrollBar;
  IntroPage        *m_pageIntro;

  int               m_initialHeight;
  int               m_initialWidth;

  QBrush            m_clearBrush;
  QBrush            m_clearBrushText;
  QBrush            m_colorBrush;
  QBrush            m_colorBrushText;
  QBrush            m_errorBrush;
  QBrush            m_errorBrushText;

  QMap <Column, QString> m_colTypeName;
  bool              m_skipSetup;

  void              clearColumnsBackground(const int col);
  void              clearColumnsBackground(const QList<int> &columnList);
  void              clearBackground();
  void              markUnwantedRows();
  void              importClicked();
  /**
  * Called in order to adjust window size to suit the file,
  */
  void              updateWindowSize();

  void              initializeComboBoxes(const QHash<Column, QComboBox *> &columns);

  void              presetFilename(const QString& name);

private:
  QList<QLabel *>  m_stageLabels;

  int m_curId;
  int m_lastId;

  SeparatorPage               *m_pageSeparator;
  RowsPage                    *m_pageRows;
  QPointer<BankingPage>        m_pageBanking;
  QPointer<InvestmentPage>     m_pageInvestment;
  QPointer<PricesPage>         m_pagePrices;
  FormatsPage                 *m_pageFormats;

  CSVImporter*        m_plugin;
  CSVImporterCore*    m_imp;
  QWizard*            m_wiz;
  QString             m_fileName;

  void readWindowSize(const KSharedConfigPtr& config);
  void saveWindowSize(const KSharedConfigPtr& config);
  void showStage();

  void saveSettings() const;
  bool eventFilter(QObject *object, QEvent *event) override;

private Q_SLOTS:
  /**
  * This method is called when 'Exit' is clicked.  The plugin settings will
  * be saved and the plugin will be terminated.
  */
  void slotClose();
  void slotIdChanged(int id);
  void fileDialogClicked();
  void saveAsQIFClicked();
};

namespace Ui
{
class IntroPage;
}

class IntroPage : public CSVWizardPage
{
  Q_OBJECT

public:
  explicit IntroPage(CSVWizard *dlg, CSVImporterCore *imp);
  ~IntroPage();

  void             initializePage() override;

  Profile     m_profileType;
  Ui::IntroPage   *ui;

Q_SIGNALS:
  void             signalBankClicked(bool);
  void             activated(int);
  void             returnPressed();

private:
  QStringList      m_profiles;

  bool             validatePage() override;
  int              nextId() const override;

  void             profileChanged(const ProfileAction action);
  void             profileTypeChanged(const Profile profileType, bool toggled);

private Q_SLOTS:
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
  explicit SeparatorPage(CSVWizard *dlg, CSVImporterCore *imp);
  ~SeparatorPage();

private Q_SLOTS:
  void                encodingChanged(const int index);
  void                fieldDelimiterChanged(const int index);
  void                textDelimiterChanged(const int index);

Q_SIGNALS:
  void                completeChanged();

private:
  Ui::SeparatorPage   *ui;
  void                initializeEncodingCombobox();
  void                initializePage() override;
  bool                isComplete() const override;
  void                cleanupPage() override;
  bool                validatePage() override;
};

namespace Ui
{
class RowsPage;
}

class RowsPage : public CSVWizardPage
{
  Q_OBJECT

public:
  explicit RowsPage(CSVWizard *dlg, CSVImporterCore *imp);
  ~RowsPage();

private Q_SLOTS:
  /**
  * This method is called when the user edits the startLine setting.
  */
  void            startRowChanged(int val);

  /**
  * This method is called when the user edits the lastLine setting.
  */
  void            endRowChanged(int val);
private:
  Ui::RowsPage   *ui;
  void            initializePage() override;
  int             nextId() const override;
  void            cleanupPage() override;
};

namespace Ui
{
class FormatsPage;
}

class FormatsPage : public CSVWizardPage
{
  Q_OBJECT

public:
  explicit FormatsPage(CSVWizard *dlg, CSVImporterCore *imp);
  ~FormatsPage();

private:
  Ui::FormatsPage  *ui;
  bool              m_isDecimalSymbolOK;
  bool              m_isDateFormatOK;

  /**
  * This method is called when the user selects a new decimal symbol.  The
  * UI is updated using the new symbol, and on importing, the new symbol
  * also will be used.
  */
  bool              validateDecimalSymbols(const QList<int> &columns);

  /**
  * This method checks if all dates in date column are valid.
  */
  bool              validateDateFormat(const int index);

  void              initializePage() override;
  bool              isComplete() const override;
  void              cleanupPage() override;

Q_SIGNALS:
  void              completeChanged();

private Q_SLOTS:
  void              decimalSymbolChanged(int);
  void              dateFormatChanged(const int index);
};

#endif // CSVWIZARD_H
