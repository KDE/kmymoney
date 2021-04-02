/*
    SPDX-FileCopyrightText: 2009 Cristian Onet onet.cristian @gmail.com
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "kreconciliationreportdlg.h"

// Qt includes
#include <QPushButton>
#include <QTabWidget>
#include <QPointer>

// KDE includes
#include <KStandardGuiItem>
#ifdef ENABLE_WEBENGINE
#include <QWebEngineView>
#else
#include <KWebView>
#endif

#include "kmm_printer.h"

KReportDlg::KReportDlg(QWidget* parent, const QString& summaryReportHTML, const QString& detailsReportHTML) :
    QDialog(parent)
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
    auto printer = KMyMoneyPrinter::startPrint();
    if (printer != nullptr) {
        // do the actual painting job
        switch (m_tabWidget->currentIndex()) {
        case 0:
#ifdef ENABLE_WEBENGINE
            m_summaryHTMLPart->page()->print(printer, [=] (bool) {});
#else
            m_summaryHTMLPart->print(printer);
#endif
            break;
        case 1:
#ifdef ENABLE_WEBENGINE
            m_detailsHTMLPart->page()->print(printer, [=] (bool) {});
#else
            m_detailsHTMLPart->print(printer);
#endif
            break;
        default:
            qDebug("KReportDlg::print() current page index not handled correctly");
            break;
        }
    }
}
