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
#include <QPainter>
#include <QPushButton>
#include <QLayout>
#include <QTabWidget>
#include <QPrinter>
#include <QtPrintSupport/QPrintDialog>
#include <QPointer>

// KDE includes
#include <KStandardGuiItem>
#ifdef KF5KHtml_FOUND
#include <KHTMLPart>
#include <KHTMLView>
#endif

KReportDlg::KReportDlg(QWidget* parent, const QString& summaryReportHTML, const QString& detailsReportHTML) : QDialog(parent)
{
  setupUi(this);
  m_summaryHTMLPart = new QWebEngineView(m_summaryTab);
  m_summaryLayout->addWidget(m_summaryHTMLPart);

  m_detailsHTMLPart = new QWebEngineView(m_detailsTab);
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

#ifdef KF5KHtml_FOUND
void KReportDlg::handleHTML(const QString &sHTML)
{
  KHTMLPart *khtml = new KHTMLPart(this);
  khtml->begin();
  khtml->write(sHTML);
  khtml->end();
  khtml->view()->print();
  delete khtml;
}
#endif

void KReportDlg::print()
{
#ifdef KF5KHtml_FOUND
    // do the actual painting job
    connect(this, &KReportDlg::getHTML, this, &KReportDlg::handleHTML);
    switch (m_tabWidget->currentIndex()) {
      case 0:
        m_summaryHTMLPart->page()->toHtml([this](const QString &result){emit getHTML(result);});
        break;
      case 1:
        m_detailsHTMLPart->page()->toHtml([this](const QString &result){emit getHTML(result);});
        break;
      default:
        qDebug("KReportDlg::print() current page index not handled correctly");
    }
#else
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
      m_summaryHTMLPart->page()->print(m_currentPrinter, [=] (bool) {delete m_currentPrinter; m_currentPrinter = nullptr;});
      break;
    case 1:
      m_detailsHTMLPart->page()->print(m_currentPrinter, [=] (bool) {delete m_currentPrinter; m_currentPrinter = nullptr;});
      break;
    default:
      delete m_currentPrinter;
      m_currentPrinter = nullptr;
      qDebug("KReportDlg::print() current page index not handled correctly");
  }
  #endif
}
