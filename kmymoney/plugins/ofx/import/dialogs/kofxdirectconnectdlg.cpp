/*
    SPDX-FileCopyrightText: 2002 Ace Jones <acejones@users.sourceforge.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kofxdirectconnectdlg.h"
#include "kmymoneysettings.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDebug>
#include <QFile>
#include <QLabel>
#include <QRegularExpression>
#include <QTemporaryFile>
#include <QTextCodec>
#include <QTextStream>
#include <QUuid>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KIO/TransferJob>
#include <KJobUiDelegate>
#include <KLocalizedString>
#include <KMessageBox>
#include <KProtocolManager>
#include <kio/job_base.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyofxconnector.h"

class KOfxDirectConnectDlg::Private
{
public:
    Private() : m_firstData(true) {}
    QFile    m_fpTrace;
    bool     m_firstData;
};

KOfxDirectConnectDlg::KOfxDirectConnectDlg(const MyMoneyAccount& account, QWidget *parent) :
    KOfxDirectConnectDlgDecl(parent),
    d(new Private),
    m_tmpfile(0),
    m_connector(account),
    m_job(0)
{
}

KOfxDirectConnectDlg::~KOfxDirectConnectDlg()
{
    if (d->m_fpTrace.isOpen()) {
        d->m_fpTrace.close();
    }
    delete m_tmpfile;
    delete d;
}

bool KOfxDirectConnectDlg::init()
{
    show();

    auto request = m_connector.statementRequest();
    if (request.isEmpty()) {
        hide();
        return false;
    }

    // For debugging, dump out the request
#if 0
    QFile g("request.ofx");
    g.open(QIODevice::WriteOnly);
    QTextStream(&g) << m_connector.url() << "\n" << QString(request);
    g.close();
#endif

    if (KMyMoneySettings::logOfxTransactions()) {
        QString logPath = KMyMoneySettings::logPath();
        d->m_fpTrace.setFileName(QString("%1/ofxlog.txt").arg(logPath));
        d->m_fpTrace.open(QIODevice::WriteOnly | QIODevice::Append);
    }

    // Check if we need to tweak the request for specific institutions
    MyMoneyOfxConnector::institutionSpecificRequestAdjustment(request);

    if (d->m_fpTrace.isOpen()) {
        QByteArray connectorData = m_connector.url().toUtf8();
        d->m_fpTrace.write("url: ", 5);
        d->m_fpTrace.write(connectorData, strlen(connectorData));
        d->m_fpTrace.write("\n", 1);
        d->m_fpTrace.write("request:\n", 9);
        auto trcData = request.toUtf8(); // make local UTF-8 byte array copy
        trcData.replace('\r', ""); // krazy:exclude=doublequote_chars
        d->m_fpTrace.write(trcData, trcData.size());
        d->m_fpTrace.write("\n", 1);
        d->m_fpTrace.write("response:\n", 10);
    }

    auto codec = QTextCodec::codecForName("Windows-1251");
    qDebug() << "creating job"; // << codec->fromUnicode(request);
    m_job = KIO::http_post(QUrl(m_connector.url()), codec->fromUnicode(request), KIO::HideProgressInfo);

    // open the temp file. We come around here twice if init() is called twice
    if (m_tmpfile) {
        qDebug() << "Already connected, using " << m_tmpfile->fileName();
        delete m_tmpfile; //delete otherwise we mem leak
    }
    m_tmpfile = new QTemporaryFile();
    // for debugging purposes one might want to leave the temp file around
    // in order to achieve this, please uncomment the next line
    // m_tmpfile->setAutoRemove(false);
    if (!m_tmpfile->open()) {
        qWarning("Unable to open tempfile '%s' for download.", qPrintable(m_tmpfile->fileName()));
        return false;
    }

    m_job->addMetaData(QLatin1String("content-type"), QLatin1String("Content-type: application/x-ofx"));
    auto userAgent = m_connector.userAgent();
    if (!userAgent.isEmpty()) {
        m_job->addMetaData(QLatin1String("UserAgent"), userAgent);
        m_job->addMetaData(QLatin1String("SendUserAgent"), QLatin1String("true"));
    }

    connect(m_job, SIGNAL(result(KJob*)), this, SLOT(slotOfxFinished(KJob*)));
    connect(m_job, SIGNAL(data(KIO::Job*,QByteArray)), this, SLOT(slotOfxData(KIO::Job*,QByteArray)));

    setStatus(QString("Contacting %1...").arg(m_connector.url()));
    kProgress1->setMaximum(3);
    kProgress1->setValue(1);
    return true;
}

void KOfxDirectConnectDlg::setStatus(const QString& _status)
{
    textLabel1->setText(_status);
    qDebug() << "STATUS:" << _status;
}

void KOfxDirectConnectDlg::setDetails(const QString& _details)
{
    qDebug() << "DETAILS: " << _details;
}

void KOfxDirectConnectDlg::slotOfxData(KIO::Job*, const QByteArray& _ba)
{
    qDebug("Got %d bytes of data", _ba.size());
    if (d->m_firstData) {
        setStatus("Connection established, retrieving data...");
        setDetails(QString("Downloading data to %1...").arg(m_tmpfile->fileName()));
        kProgress1->setValue(kProgress1->value() + 1);
        d->m_firstData = false;
    }
    m_tmpfile->write(_ba);

    setDetails(QString("Got %1 bytes").arg(_ba.size()));

    if (d->m_fpTrace.isOpen()) {
        QByteArray trcData(_ba);
        trcData.replace('\r', ""); // krazy:exclude=doublequote_chars
        d->m_fpTrace.write(trcData, trcData.size());
    }
}

void KOfxDirectConnectDlg::slotOfxFinished(KJob* /* e */)
{
    qDebug("Job finished");
    kProgress1->setValue(kProgress1->value() + 1);
    setStatus("Completed.");

    if (d->m_fpTrace.isOpen()) {
        d->m_fpTrace.write("\nCompleted\n\n\n\n", 14);
    }

    int error = m_job->error();

    if (m_tmpfile) {
        qDebug("Closing tempfile");
        m_tmpfile->close();
    }
    qDebug("Tempfile closed");

    if (error) {
        qDebug("Show error message");
        m_job->uiDelegate()->showErrorMessage();
    } else if (m_job->isErrorPage()) {
        qDebug("Process error page");
        QString details;
        if (m_tmpfile) {
            QFile f(m_tmpfile->fileName());
            if (f.open(QIODevice::ReadOnly)) {
                QTextStream stream(&f);
                while (!stream.atEnd()) {
                    details += stream.readLine(); // line of text excluding '\n'
                }
                f.close();

                qDebug() << "The HTTP request failed: " << details;
            }
        }
        KMessageBox::detailedSorry(this, i18n("The HTTP request failed."), details, i18nc("The HTTP request failed", "Failed"));
    } else if (m_tmpfile) {
        qDebug("Emit statementReady signal with '%s'", qPrintable(m_tmpfile->fileName()));
        emit statementReady(m_tmpfile->fileName());
        qDebug("Return from signal statementReady() processing");
    }
    delete m_tmpfile;
    m_tmpfile = 0;
    hide();
    qDebug("Finishing slotOfxFinished");
}

void KOfxDirectConnectDlg::reject()
{
    if (m_job)
        m_job->kill();
    if (m_tmpfile) {
        m_tmpfile->close();
        delete m_tmpfile;
        m_tmpfile = 0;
    }
    QDialog::reject();
}
