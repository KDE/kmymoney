/*******************************************************************************
*                                 priceswizardpage.h
*                              ------------------
* begin                       : Sat Jan 21 2017
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

#ifndef PRICESWIZARDPAGE_H
#define PRICESWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QtCore/QFile>
#include <QPointer>
#include <QWizardPage>
#include <QVBoxLayout>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KSharedConfig>

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneystatement.h>

// ----------------------------------------------------------------------------

class SecurityDlg;
class CurrenciesDlg;
class CSVWizard;
class PricesPage;

namespace Ui
{
class PricesPage;
}

class PricesPage : public QWizardPage
{
  Q_OBJECT

public:
  explicit PricesPage(QDialog *parent = 0);
  ~PricesPage();

  Ui::PricesPage  *ui;

  QVBoxLayout         *m_pageLayout;

  CSVWizard*          m_wiz;
  typedef enum:uchar { ColumnDate, ColumnPrice,
         ColumnEmpty = 0xFE, ColumnInvalid = 0xFF
       } columnTypeE;

  QMap<columnTypeE, int>   m_colTypeNum;
  QMap<int ,columnTypeE>   m_colNumType;
  QMap<columnTypeE, QString> m_colTypeName;

  void                saveSettings();
  void                readSettings(const KSharedConfigPtr &config);
  void                setParent(CSVWizard* dlg);

  /**
  * This method feeds file buffer in prices lines parser.
  */
  bool                createStatement(MyMoneyStatement& st);

private:
  QPointer<SecurityDlg>     m_securityDlg;
  QPointer<CurrenciesDlg>   m_currenciesDlg;

  QMap<QString, QString> m_mapSymbolName;

  QStringList         m_columnList;

  QString             m_priceFractionValue;
  QString             m_fromCurrency;
  QString             m_toCurrency;
  QString             m_securityName;
  QString             m_securitySymbol;

  int                 m_priceFraction;
  int                 m_dontAsk;

  void                initializePage();
  bool                isComplete() const;
  bool                validatePage();

  void                initializeComboBoxes();

  void                resetComboBox(const columnTypeE comboBox);
  /**
  * This method is called column on prices page is selected.
  * It sets m_colTypeNum, m_colNumType and runs column validation.
  */
  bool                validateSelectedColumn(int col, columnTypeE type);

  /**
  * This method is called when the user clicks 'Import'.
  * It will evaluate an input line and append it to a statement.
  */
  bool                processPriceLine(const QString& line, MyMoneyStatement& st);

  /**
  * This method ensures that there is security for price import.
  */
  bool                validateSecurity();

  /**
  * This method ensures that there are currencies for price import.
  */
  bool                validateCurrencies();

public slots:

private slots:
  void                slotDateColSelected(int col);
  void                slotPriceColSelected(int col);
  void                slotFractionChanged(int col);
  void                slotClearColumns();
};

#endif // PRICESWIZARDPAGE_H
