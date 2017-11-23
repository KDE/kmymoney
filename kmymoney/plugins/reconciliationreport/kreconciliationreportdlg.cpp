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

#include "kreconciliationreportdlg.h"

// Qt includes
#include <QPushButton>
#include <QTabWidget>
#include <QPrinter>
#include <QPrintDialog>

// KDE includes
#include <KStandardGuiItem>
#ifdef ENABLE_WEBENGINE
#include <QWebEngineView>
#else
#include <KWebView>
#endif

KReportDlg::KReportDlg(QWidget* parent, const QString& summaryReportHTML, const QString& detailsReportHTML) : QDialog(parent)
{
  setupUi(this);
  #ifdef ENABLE_WEBENGINE
  m_summaryHTMLPart = new QWebEngineView(m_summaryTab);
  m_detailsHTMLPart = new QWebEngineView(m_detailsTab);
  #else
  m_summaryHTMLPart = new KWebView(m_summaryTab);
  m_detailsHTMLPart = new KWebView(m_detailsTab);
  #endif

  m_summaryLayout->addWidget(m_summaryHTMLPart);
  m_detailsLayout->addWidget(m_detailsHTMLPart);

  m_summaryHTMLPart->setHtml(summaryReportHTML, QUrl("file://"));
  m_detailsHTMLPart->setHtml(detailsReportHTML, QUrl("file://"));

  QPushButton* printButton = m_buttonBox->addButton(QString(), QDialogButtonBox::ActionRole);
  KGuiItem::assign(printButton, KStandardGuiItem::print());

  // signals and slots connections
  connect(printButton, SIGNAL(clicked()), this, SLOT(print()));
}

KReportDlg::~KReportDlg()
{
}

void KReportDlg::print()
{
  m_currentPrinter = new QPrinter();
  QPrintDialog *dialog = new QPrintDialog(m_currentPrinter, this);
  dialog->setWindowTitle(QString());
  if (dialog->exec() != QDialog::Accepted) {
    delete m_currentPrinter;
    m_currentPrinter = nullptr;
    return;
  }

  // do the actual painting job
  switch (m_tabWidget->currentIndex()) {
    case 0:
      #ifdef ENABLE_WEBENGINE
      m_summaryHTMLPart->page()->print(m_currentPrinter, [=] (bool) {delete m_currentPrinter; m_currentPrinter = nullptr;});
      #else
      m_summaryHTMLPart->print(m_currentPrinter);
      #endif
      break;
    case 1:
      #ifdef ENABLE_WEBENGINE
      m_detailsHTMLPart->page()->print(m_currentPrinter, [=] (bool) {delete m_currentPrinter; m_currentPrinter = nullptr;});
      #else
      m_detailsHTMLPart->print(m_currentPrinter);
      #endif
      break;
    default:
      delete m_currentPrinter;
      m_currentPrinter = nullptr;
      qDebug("KReportDlg::print() current page index not handled correctly");
  }
}
