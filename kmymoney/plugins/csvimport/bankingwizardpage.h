/*******************************************************************************
*                                 bankingwizardpage.h
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

#ifndef BANKINGWIZARDPAGE_H
#define BANKINGWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QtCore/QFile>
#include <QPointer>
#include <QWizardPage>
#include <QVBoxLayout>
#include <QSet>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KSharedConfig>

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneystatement.h>

// ----------------------------------------------------------------------------

class CSVWizard;
class BankingPage;

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

  CSVWizard*          m_wiz;

  typedef enum:uchar { ColumnNumber, ColumnDate, ColumnPayee, ColumnAmount,
         ColumnCredit, ColumnDebit, ColumnCategory,
         ColumnMemo, ColumnEmpty = 0xFE, ColumnInvalid = 0xFF
       } columnTypeE;

  QMap<columnTypeE, int>   m_colTypeNum;
  QMap<int ,columnTypeE>   m_colNumType;
  QMap<columnTypeE, QString> m_colTypeName;

  void                saveSettings();
  void                readSettings(const KSharedConfigPtr &config);
  void                setParent(CSVWizard* dlg);

  /**
  * This method fills QIF file with bank/credit card data
  */
  void                makeQIF(MyMoneyStatement &st, QFile &file);

  /**
  * This method feeds file buffer in banking lines parser.
  */
  bool                createStatement(MyMoneyStatement& st);

private:

  QStringList         m_columnList;
  QSet<QString>       m_hashSet;
  int                 m_oppositeSigns;

  void                initializePage();
  bool                isComplete() const;
  int                 nextId() const;

  void                initializeComboBoxes();
  bool                validateMemoComboBox();
  void                resetComboBox(const columnTypeE comboBox);
  bool                validateSelectedColumn(int col, columnTypeE type);

  /**
  * This method is called during processing. It ensures that processed credit/debit
  * are valid.
  */
  bool                processCreditDebit(QString& credit, QString& debit , MyMoneyMoney& amount);

  /**
  * This method is called when the user clicks 'import'.
  * or 'Make QIF' It will evaluate a line and prepare it to be added to a statement,
  * and to a QIF file, if required.
  */
  bool                processBankLine(const QString &line, MyMoneyStatement &st);

private slots:
  void                slotMemoColSelected(int col);
  void                slotCategoryColSelected(int col);
  void                slotNumberColSelected(int col);
  void                slotPayeeColSelected(int col);
  void                slotDateColSelected(int col);
  void                slotDebitColSelected(int col);
  void                slotCreditColSelected(int col);
  void                slotAmountColSelected(int col);
  void                slotAmountToggled(bool checked);
  void                slotDebitCreditToggled(bool checked);
  void                slotOppositeSignsClicked(bool checked);
  void                slotClearColumns();
};

#endif // BANKINGWIZARDPAGE_H
