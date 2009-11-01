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

// Qt includes
#include <qpainter.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtabwidget.h>
#include <qprinter.h>
#include <qprintdialog.h>

// KDE includes
#include <khtmlview.h>
#include <khtml_part.h>

#include "kreconciliationreportdlg.h"

KReportDlg::KReportDlg(QWidget* parent, const QString& summaryReportHTML, const QString& detailsReportHTML) : QDialog(parent)
{
  setupUi(this);
  m_summaryHTMLPart = new KHTMLPart(m_summaryFrame);
  m_summaryFrame->layout()->addWidget(m_summaryHTMLPart->view());

  m_detailsHTMLPart = new KHTMLPart(m_detailsFrame);
  m_detailsFrame->layout()->addWidget(m_detailsHTMLPart->view());

  m_summaryHTMLPart->begin();
  m_summaryHTMLPart->write(summaryReportHTML);
  m_summaryHTMLPart->end();

  m_detailsHTMLPart->begin();
  m_detailsHTMLPart->write(detailsReportHTML);
  m_detailsHTMLPart->end();

  // signals and slots connections
  connect( m_printButton, SIGNAL( clicked() ), this, SLOT( print() ) );
}

KReportDlg::~KReportDlg()
{
}

void KReportDlg::print()
{
  // create the QPrinter object with default options
  QPrinter printer;

  // start the print dialog to initialize the QPrinter object
  QPointer<QPrintDialog> dlg = new QPrintDialog(&printer, this);

  if (dlg->exec())
  {
    // create the painter object
    QPainter painter(&printer);

    // do the actual painting job
    switch (m_tabWidget->currentIndex())
    {
      case 0:
        m_summaryHTMLPart->paint(&painter, QRect(0, 0, 800, 600));
      break;
      case 1:
        m_detailsHTMLPart->paint(&painter, QRect(0, 0, 800, 600));
      break;
      default:
        qDebug("KReportDlg::print() current page index not handled correctly");
    }
  }
  delete dlg;
}

#include "kreconciliationreportdlg.moc"
