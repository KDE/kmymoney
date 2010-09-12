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

  void m_action(bool arg1);

  ConvertDate*        m_convertDate;
  CsvProcessing*      m_csvprocessing;
  InvestmentDlg*      m_investmentDlg;//below
  CsvImporterPlugin*  m_plugin;

  bool             m_amountSelected;
  bool             m_creditSelected;
  bool             m_debitSelected;
  bool             m_dateSelected;
  bool             m_payeeSelected;
  bool             m_memoSelected;
  bool             m_numberSelected;

  int              m_amountColumn;
  int              m_creditColumn;
  int              m_dateColumn;
  int              m_debitColumn;
  int              m_memoColumn;
  int              m_numberColumn;
  int              m_payeeColumn;

  int              m_tableFrameHeight;
  int              m_tableFrameWidth;
  int              m_maxColumnCount;

  QString          m_columnType[MAXCOL];

private:

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
  void           numberColumnChanged(int);

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

signals:
  /**
  * This signal is raised when the plugin has completed a transaction.  This
  * then needs to be processed by MyMoneyStatement.
  */
  void statementReady(MyMoneyStatement&);
};

#endif // CSVIMPORTERDLG_H
