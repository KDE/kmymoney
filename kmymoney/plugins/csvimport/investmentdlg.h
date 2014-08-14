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
#include <QCloseEvent>
// ----------------------------------------------------------------------------
// KDE Headers
#include <QUrl>
// ----------------------------------------------------------------------------
// Project Headers

#include <mymoneystatement.h>

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
  SymbolTableDlg*    m_redefine;
  InvestProcessing*  m_investProcessing;

  int                m_round;
  int                m_lastHeight;

  void               init();
  void               saveSettings();

public slots:

private:

private slots:

signals:
  /**
  * This signal is raised when the plugin has completed a transaction.  This
  * then needs to be processed by MyMoneyStatement.
  */
  void           statementReady(MyMoneyStatement&);
};

#endif // INVESTMENTDLG_H
