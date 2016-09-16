/*******************************************************************************
*                                 investmentwizardpage.h
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

#ifndef INVESTMENTWIZARDPAGE_H
#define INVESTMENTWIZARDPAGE_H

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

class TransactionDlg;
class SecuritiesDlg;
class CSVWizard;
class InvestmentPage;

namespace Ui
{
class InvestmentPage;
}

class InvestmentPage : public QWizardPage
{
  Q_OBJECT

public:
  explicit InvestmentPage(QDialog *parent = 0);
  ~InvestmentPage();

  Ui::InvestmentPage  *ui;

  QVBoxLayout         *m_pageLayout;

  CSVWizard*          m_wiz;
  typedef enum:uchar { ColumnDate, ColumnType, ColumnAmount,
         ColumnPrice, ColumnQuantity, ColumnFee,
         ColumnSymbol, ColumnName, ColumnMemo,
         ColumnEmpty = 0xFE, ColumnInvalid = 0xFF
       } columnTypeE;

  QMap<columnTypeE, int>   m_colTypeNum;
  QMap<int ,columnTypeE>   m_colNumType;
  QMap<columnTypeE, QString> m_colTypeName;

  void                saveSettings();
  void                readSettings(const KSharedConfigPtr &config);
  void                setParent(CSVWizard* dlg);

  /**
  * This method fills QIF file with investment data
  */
  void                makeQIF(MyMoneyStatement& st, QFile& file);

  /**
  * This method feeds file buffer in investment lines parser.
  */
  bool                createStatement(MyMoneyStatement& st);

private:
  QPointer<SecuritiesDlg>   m_securitiesDlg;

  QMap<QString, QString> m_mapSymbolName;

  QStringList         m_shrsinList;
  QStringList         m_divXList;
  QStringList         m_intIncList;
  QStringList         m_feeList;
  QStringList         m_brokerageList;
  QStringList         m_reinvdivList;
  QStringList         m_buyList;
  QStringList         m_sellList;
  QStringList         m_shrsoutList;
  QStringList         m_columnList;

  QString             m_priceFractionValue;
  QString             m_feeRate;
  QString             m_minFee;

  int                 m_feeIsPercentage;
  int                 m_priceFraction;

  void                initializePage();
  bool                isComplete() const;
  bool                validatePage();
  void                cleanupPage();

  void                initializeComboBoxes();
  void                clearFeeCol();

  /**
  * This method will check whether memo combobox is still valid
  * after changing name or type column.
  */
  bool                validateMemoComboBox();

  void                resetComboBox(const columnTypeE comboBox);
  /**
  * This method is called column on investment page is selected.
  * It sets m_colTypeNum, m_colNumType and runs column validation.
  */
  bool                validateSelectedColumn(int col, columnTypeE type);

  /**
  * This method is called when the user clicks 'Import'.
  * It will evaluate an input line and append it to a statement.
  */
  bool                processInvestLine(const QString& line, MyMoneyStatement& st);

  /**
  * This method creates valid set of possible transactions
  * according to quantity, amount and price
  */
  bool                createValidActionTypes(QList<MyMoneyStatement::Transaction::EAction> &validActionTypes, MyMoneyStatement::Transaction &tr);

  /**
  * This method stores user selection of action type so it will not ask
  * for the same action type twice.
  */
  void                storeActionType(MyMoneyStatement::Transaction::EAction &actionType, const QString &userType);

  /**
  * This method validates the column numbers entered by the user.  It then
  * checks the values in those columns for compatibility with the input
  * investment activity type.
  */
  bool                validateActionType(MyMoneyStatement::Transaction &tr, QStringList &colList);

  /**
  * This method is called during input.  It validates the action types
  * in the input file, and assigns appropriate QString types.
  */
  MyMoneyStatement::Transaction::EAction processActionType(QString& type);

  /**
  * This method gets securities from investment statement and
  * tries to get pairs of symbol and name either
  * from KMM or from statement data.
  * In case it's not successfull onlySymbols and onlyNames won't be empty.
  */
  bool                sortSecurities(QSet<QString>& onlySymbols, QSet<QString>& onlyNames, QMap<QString, QString>& mapSymbolName);

  /**
  * This method ensures that every security has symbol and name.
  */
  bool                validateSecurities();

public slots:
  void                slotClearFee();

private slots:
  void                slotMemoColSelected(int col);
  void                slotDateColSelected(int col);
  void                slotFeeColSelected(int col);
  void                slotTypeColSelected(int col);
  void                slotQuantityColSelected(int col);
  void                slotPriceColSelected(int col);
  void                slotAmountColSelected(int col);
  void                slotSymbolColSelected(int col);
  void                slotNameColSelected(int col);
  void                slotFeeIsPercentageClicked(bool checked);
  void                slotFractionChanged(int col);
  void                slotClearColumns();
  void                slotCalculateFee();
  void                slotFeeInputsChanged();
  void                slotFeeRateChanged(const QString text);
  void                slotMinFeeChanged(const QString text);
};

#endif // INVESTMENTWIZARDPAGE_H
