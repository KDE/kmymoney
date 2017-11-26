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
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kfindtransactiondlg.h"

class MyMoneyReport;

/**
  * @author Ace Jones
  */
class KReportConfigurationFilterDlgPrivate;
class KReportConfigurationFilterDlg : public KFindTransactionDlg
{
  Q_OBJECT
  Q_DISABLE_COPY(KReportConfigurationFilterDlg)

public:
  explicit KReportConfigurationFilterDlg(MyMoneyReport report, QWidget *parent = nullptr);
  ~KReportConfigurationFilterDlg();

  MyMoneyReport getConfig() const;

protected Q_SLOTS:
  void slotRowTypeChanged(int);
  void slotColumnTypeChanged(int);
  void slotReset();
  void slotSearch();
  void slotShowHelp();
  void slotUpdateCheckTransfers();
  void slotUpdateColumnsCombo();
  void slotUpdateColumnsCombo(int idx);
  void slotLogAxisChanged(int state);

private:
    Q_DECLARE_PRIVATE(KReportConfigurationFilterDlg)
};
#endif
