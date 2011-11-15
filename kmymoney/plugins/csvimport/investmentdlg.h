/***************************************************************************
                        investmentdlg.h
                      -------------------
begin                 : Sat Jan 01 2010
copyright             : (C) 2010 by Allan Anderson
email                 : agander93@gmail.com
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
#include <QtGui/QCloseEvent>
// ----------------------------------------------------------------------------
// KDE Headers
#include <KUrl>
// ----------------------------------------------------------------------------
// Project Headers

#include <mymoneystatement.h>

#define invMAXCOL 25    //                 maximum no. of columns (arbitrary value)

class CSVDialog;
class ConvertDate;
class InvestProcessing;
class MyMoneyStatement;
class SymbolTableDlg;

class InvestmentDlg: public QObject
{
  Q_OBJECT

public:
  InvestmentDlg();
  ~InvestmentDlg();

  CSVDialog*         m_csvDialog;
  ConvertDate*       m_convertDat;
  SymbolTableDlg*       m_redefine;
  InvestProcessing*  m_investProcessing;

  void               clearComboBoxText();
  void               init();
  void               saveSettings();

public slots:

private:

  /**
  * This method will receive resize events, calling updateScreen().
  */
  void           resizeEvent(QResizeEvent * event);

private slots:

  /**
  * This slot is called following the user selecting a new investment type
  * in the RedefineDlg window.  The new value is saved.
  */
  void           changedType(const QString&);

signals:
  /**
  * This signal is raised when the plugin has completed a transaction.  This
  * then needs to be processed by MyMoneyStatement.
  */
  void           statementReady(MyMoneyStatement&);
};

#endif // INVESTMENTDLG_H
