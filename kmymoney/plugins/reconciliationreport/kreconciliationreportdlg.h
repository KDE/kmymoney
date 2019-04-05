/***************************************************************************
 *   Copyright 2009  Cristian Onet onet.cristian@gmail.com                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>  *
 ***************************************************************************/

#ifndef KRECONCILIATIONREPORTDLG_H
#define KRECONCILIATIONREPORTDLG_H

#include <config-kmymoney.h>

#include "ui_kreconciliationreportdlgdecl.h"

#ifdef ENABLE_WEBENGINE
class QWebEngineView;
#else
class KWebView;
#endif

class KReportDlg : public QDialog, public Ui::KReconciliationReportDlgDecl
{
  Q_OBJECT

public:
  KReportDlg(QWidget* parent, const QString& summaryReportHTML, const QString& detailsReportHTML);
  ~KReportDlg();

protected Q_SLOTS:
  void print();

private:
  #ifdef ENABLE_WEBENGINE
  QWebEngineView *m_summaryHTMLPart;
  QWebEngineView *m_detailsHTMLPart;
  #else
  KWebView       *m_summaryHTMLPart;
  KWebView       *m_detailsHTMLPart;
  #endif
};

#endif
