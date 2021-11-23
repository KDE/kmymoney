/*
    SPDX-FileCopyrightText: 2009 Cristian Onet onet.cristian @gmail.com
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "kreconciliationreportdlg.h"

// Qt includes
#include <QPushButton>
#include <QTabWidget>
#include <QPointer>

// KDE includes
#include <KStandardGuiItem>

#include "kmm_printer.h"
#include "kmmtextbrowser.h"

KReportDlg::KReportDlg(QWidget* parent, const QString& summaryReportHTML, const QString& detailsReportHTML) :
    QDialog(parent)
{
    setupUi(this);

    m_summaryHTMLPart = new KMMTextBrowser(m_summaryTab);
    m_detailsHTMLPart = new KMMTextBrowser(m_detailsTab);

    m_summaryLayout->addWidget(m_summaryHTMLPart);
    m_detailsLayout->addWidget(m_detailsHTMLPart);

    m_summaryHTMLPart->setHtml(summaryReportHTML);
    m_detailsHTMLPart->setHtml(detailsReportHTML);

    QPushButton* printButton = m_buttonBox->addButton(QString(), QDialogButtonBox::ActionRole);
    KGuiItem::assign(printButton, KStandardGuiItem::print());

    // signals and slots connections
    connect(printButton, &QPushButton::clicked, this, &KReportDlg::print);
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
            m_summaryHTMLPart->print(printer);
            break;
        case 1:
            m_detailsHTMLPart->print(printer);
            break;
        default:
            qDebug("KReportDlg::print() current page index not handled correctly");
            break;
        }
    }
}
