/***************************************************************************
                          kreportconfigurationdlg.h  -  description
                             -------------------
    begin                : Mon Jun 21 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           Ace Jones <ace.j@hotpop.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KREPORTCONFIGURATIONFILTERDLG_H
#define KREPORTCONFIGURATIONFILTERDLG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QVector>
#include <QPointer>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kfindtransactiondlg.h"
#include "mymoneyreport.h"
#include "mymoneybudget.h"

class ReportTabCapitalGain;
class ReportTabChart;
class ReportTabGeneral;
class ReportTabPerformance;
class ReportTabRange;
class ReportTabRowColPivot;
class ReportTabRowColQuery;

/**
  * @author Ace Jones
  */
class KReportConfigurationFilterDlg : public KFindTransactionDlg
{
  Q_OBJECT
public:
  explicit KReportConfigurationFilterDlg(MyMoneyReport report, QWidget *parent = 0);
  ~KReportConfigurationFilterDlg();

  const MyMoneyReport& getConfig() const {
    return m_currentState;
  }

protected:
  QPointer<ReportTabGeneral>     m_tabGeneral;
  QPointer<ReportTabRowColPivot> m_tabRowColPivot;
  QPointer<ReportTabRowColQuery> m_tabRowColQuery;
  QPointer<ReportTabChart>       m_tabChart;
  QPointer<ReportTabRange>       m_tabRange;
  QPointer<ReportTabCapitalGain> m_tabCapitalGain;
  QPointer<ReportTabPerformance> m_tabPerformance;

  MyMoneyReport m_initialState;
  MyMoneyReport m_currentState;

protected slots:
  void slotRowTypeChanged(int);
  void slotColumnTypeChanged(int);
  void slotReset();
  void slotSearch();
  void slotShowHelp();
  void slotUpdateCheckTransfers();
  void slotUpdateColumnsCombo();
  void slotLogAxisChanged(int state);

private:
  QVector<MyMoneyBudget> m_budgets;
};
#endif
