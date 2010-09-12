/***************************************************************************
                        investmentdlg.h
                      -------------------
begin                 : Sat Jan 01 2010
copyright             : (C) 2010 by Allan Anderson
email                 : aganderson@ukonline.co.uk
****************************************************************************/

/***************************************************************************

*   This program is free software; you can redistribute it and/or modify   *
*   it under the terms of the GNU General Public License as published      *
*   by the Free Software Foundation; either version 2 of the License,      *
*   or  (at your option) any later version.                                *
*                                                                          *
****************************************************************************/


#ifndef INVESTMENTDLG_H
#define INVESTMENTDLG_H

// ----------------------------------------------------------------------------
// QT Headers

#include <QtCore/QDate>
#include <QtCore/QTextStream>
#include <QtCore/QFile>
// ----------------------------------------------------------------------------
// KDE Headers
#include <KUrl>
// ----------------------------------------------------------------------------
// Project Headers

#include <mymoneystatement.h>
#include "ui_investmentdlgdecl.h"

#define invMAXCOL 14    //                 maximum no. of columns (arbitrary value)

class ConvertDate;
class CsvImporterDlg;
class InvestProcessing;
class MyMoneyStatement;
class RedefineDlg;

class InvestmentDlgDecl : public QWidget, public Ui::InvestmentDlgDecl
{
public:
  InvestmentDlgDecl(QWidget *parent) : QWidget(parent) {
    setupUi(this);
  }
};

class InvestmentDlg: public InvestmentDlgDecl
{
  Q_OBJECT

public:
  InvestmentDlg(QWidget *parent = 0);
  ~InvestmentDlg();

  int                m_tableFrameHeight;
  int                m_tableFrameWidth;

  CsvImporterDlg*    m_csvImportDlg;
  ConvertDate*       m_convertDat;
  RedefineDlg*       m_redefine;

private:
  InvestProcessing*  m_investProcessing;

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
  * This slot is called when 'Quit' is clicked.  The plugin settings will
  * be saved and the plugin will be terminated.
  */
  void           slotClose();

  /**
  * This slot is called when 'Go to Banking' is clicked.  The current
  * Investment window will be hidden and the Banking dialog will be shown.
  */
  void           bankingSelected();

  /**
  * This slot is called following the user selecting a new investment type
  * in the RedefineDlg window.  The new value is saved.
  */
  void           changedType(const QString&);

  /**
  * This method displays the help dialog.
  */
  void           helpSelected();

signals:
  /**
  * This signal is raised when the plugin has completed a transaction.  This
  * then needs to be processed by MyMoneyStatement.
  */
  void           statementReady(MyMoneyStatement&);
};

#endif // INVESTMENTDLG_H
