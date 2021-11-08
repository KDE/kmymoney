/*
    SPDX-FileCopyrightText: 2009 Cristian Onet onet.cristian @gmail.com
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KRECONCILIATIONREPORTDLG_H
#define KRECONCILIATIONREPORTDLG_H

#include <config-kmymoney.h>

#include "ui_kreconciliationreportdlgdecl.h"

class QTextBrowser;

class KReportDlg : public QDialog, public Ui::KReconciliationReportDlgDecl
{
    Q_OBJECT

public:
    KReportDlg(QWidget* parent, const QString& summaryReportHTML, const QString& detailsReportHTML);
    ~KReportDlg();

protected Q_SLOTS:
    void print();

private:
    QTextBrowser* m_summaryHTMLPart;
    QTextBrowser* m_detailsHTMLPart;
};

#endif
