/*
    SPDX-FileCopyrightText: 2000-2004 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2004 Javier Campos Morales <javi_c@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2004 Felix Rodriguez <frodriguez@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2004 John C <thetacoturtle@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2004 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2004 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2004 Ace Jones <ace.j@hotpop.com>
    SPDX-FileCopyrightText: 2017, 2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2018 Michael Kiefer <Michael-Kiefer@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KREPORTCONFIGURATIONFILTERDLG_H
#define KREPORTCONFIGURATIONFILTERDLG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

//#include "kfindtransactiondlg.h"

class MyMoneyReport;

/**
  * @author Ace Jones
  * @author Łukasz Wojniłowicz
  */
class KReportConfigurationFilterDlgPrivate;
class KReportConfigurationFilterDlg : public QDialog
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
    void slotNegExpensesChanged(int state);

private:
    Q_DECLARE_PRIVATE(KReportConfigurationFilterDlg)
    KReportConfigurationFilterDlgPrivate * const d_ptr;
};
#endif
