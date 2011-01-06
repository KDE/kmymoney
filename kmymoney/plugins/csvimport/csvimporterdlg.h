/***************************************************************************
*                          csvimporterdlg.h
*                          ---------------
* begin                  : Sat Jan 01 2010
* copyright            : (C) 2010 by Allan Anderson
* email                : aganderson@ukonline.co.uk
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
class CsvImporterPlugin;
class MyMoneyStatement;
class ValidateColumn;

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
  InvestmentDlg*      m_investmentDlg;//below
  ValidateColumn*     m_validateColumn;
  CsvImporterPlugin*  m_plugin;

  QString          columnType(int column);
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
  int              debitColumn();
  void             setDebitColumn(int val);
  int              maxColumnCount();
  void             setMaxColumnCount(int val);

private:
  QString          m_columnType[MAXCOL];//  holds field types - date, payee, etc.
  QString          m_previousType;

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

  /**
  * This method will receive close events, calling slotClose().
  */
  void           closeEvent(QCloseEvent *event);

  /**
  * This method will receive resize events, calling updateScreen().
  */
  void           resizeEvent(QResizeEvent * event);

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
  * This method is called when 'Go to Investments' is clicked.  The current
  * Banking window will be hidden and the Investment dialog will be shown.
  */
  void           investmentSelected();

  /**
  * This method is called when 'Quit' is clicked.  The plugin settings will
  * be saved and the plugin will be terminated.
  */
  void           slotClose();

  /**
  * This method is called when it is detected that the user has selected the
  * same column for two different fields.  The column detecting the error
  * has to reset the other column.
  */
  void           resetComboBox(const QString& comboBox, const int& col);

  int            validateColumn(const int& col, const QString& type);

signals:
  /**
  * This signal is raised when the plugin has completed a transaction.  This
  * then needs to be processed by MyMoneyStatement.
  */
  void statementReady(MyMoneyStatement&);
};

#endif // CSVIMPORTERDLG_H
