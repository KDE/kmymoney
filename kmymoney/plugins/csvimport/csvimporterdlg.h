/***************************************************************************
*                          csvimporterdlg.h
*                          ---------------
* begin                  : Sat Jan 01 2010
* copyright            : (C) 2010 by Allan Anderson
* email                : agander93@gmail.com
****************************************************************************/

/***************************************************************************
*                                                                          *
*   This program is free software; you can redistribute it and/or modify   *
*   it under the terms of the GNU General Public License as published      *
*   by the Free Software Foundation; either version 2 of the License,      *
*   or  (at your option) any later version.                                *
*                                                                          *
****************************************************************************/

#ifndef CSVIMPORTERDLG_H
#define CSVIMPORTERDLG_H

// ----------------------------------------------------------------------------
// QT Headers

#include <QtCore/QFile>
#include <QtCore/QDate>
// ----------------------------------------------------------------------------
// KDE Headers
#include <KUrl>
// ----------------------------------------------------------------------------
// Project Headers

#include "ui_csvimporterdlgdecl.h"
#include "csvprocessing.h"
#include "csvimporterplugin.h"

class ConvertDate;
class CsvProcessing;
class InvestmentDlg;
class InvestProcessing;
class CsvImporterPlugin;
class MyMoneyStatement;
class Parse;

class CsvImporterDlgDecl : public QWidget, public Ui::CsvImporterDlgDecl
{
public:
  CsvImporterDlgDecl(QWidget *parent) : QWidget(parent) {
    setupUi(this);
  }
};

class CsvImporterDlg: public CsvImporterDlgDecl
{
  Q_OBJECT

public:
  CsvImporterDlg(QWidget* parent = 0);
  ~CsvImporterDlg();

  ConvertDate*        m_convertDate;
  CsvProcessing*      m_csvprocessing;
  CsvImporterDlg*     m_csvDialog;
  InvestmentDlg*      m_investmentDlg;
  InvestProcessing*   m_investProcessing;
  CsvImporterPlugin*  m_plugin;
  Parse*              m_parse;

  bool             m_decimalSymbolChanged;

  void             saveSettings();

  QString          columnType(int column);
  QString          decimalSymbol();

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
  QString          currentUI();
  void             setCurrentUI(QString val);

  int              debitColumn();
  void             setDebitColumn(int val);
  int              maxColumnCount();
  void             setMaxColumnCount(int val);

  /**
  * This method is called when the user selects a new decimal symbol.  The
  * UI is updated using the new symbol, and on importing, the new symbol
  * also will be used.
  */
  void             updateDecimalSymbol(const QString&, int col);

  QString          m_fileType;
  QString          m_tab;

public slots:
  void             tabSelected(int index);

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

  QBrush           m_clearBrush;
  QBrush           m_colorBrush;
  QBrush           m_errorBrush;
  QColor           m_clearColor;
  QColor           m_setColor;
  QColor           m_errorColor;

  /**
  * This method checks that any column contents are numeric.
  */
  bool           checkContents(int col);

  /**
  * This method will receive close events, calling slotClose().
  */
  void           closeEvent(QCloseEvent *event);

  /**
  * This method will receive resize events, calling updateScreen().
  */
  void           resizeEvent(QResizeEvent * event);

  void           restoreBackground();

  int            validateColumn(const int& col, const QString& type);

private slots:
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

  /** This method is called when the debitCreditRadio button is clicked.
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

signals:
  /**
  * This signal is raised when the plugin has completed a transaction.  This
  * then needs to be processed by MyMoneyStatement.
  */
  void           statementReady(MyMoneyStatement&);
};

#endif // CSVIMPORTERDLG_H
