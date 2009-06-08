/***************************************************************************
                         kofxdirectconnectdlg.cpp
                             -------------------
    begin                : Sat Nov 13 2004
    copyright            : (C) 2002 by Ace Jones
    email                : acejones@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "config-kmymoney.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QLabel>
#include <QDir>
#include <QFile>
#include <q3textstream.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kurl.h>
#include <kio/job.h>
#include <kio/jobclasses.h>
#include <kdebug.h>
#include <ktemporaryfile.h>
#include <kprogressdialog.h>
#include <kmessagebox.h>
#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyinstitution.h>
#include <mymoneyfile.h>
#include "mymoneyofxconnector.h"
#include "kofxdirectconnectdlg.h"


class KOfxDirectConnectDlg::Private
{
public:
  QFile    m_fpTrace;
};

KOfxDirectConnectDlg::KOfxDirectConnectDlg(const MyMoneyAccount& account, QWidget *parent) :
  KOfxDirectConnectDlgDecl(parent),
  d(new Private),
  m_tmpfile(NULL),
  m_connector(account),
  m_job(NULL)
{
}

KOfxDirectConnectDlg::~KOfxDirectConnectDlg()
{
  if(d->m_fpTrace.isOpen()) {
    d->m_fpTrace.close();
  }
  delete m_tmpfile;
  delete d;
}

void KOfxDirectConnectDlg::init(void)
{
  show();

  QByteArray request = m_connector.statementRequest();

  // For debugging, dump out the request
#if 0
  QFile g( "request.ofx" );
  g.open( QIODevice::WriteOnly );
  Q3TextStream(&g) << m_connector.url() << "\n" << QString(request);
  g.close();
#endif

#warning "port to kde4"
#if 0

  QDir homeDir(QDir::home());
  if(homeDir.exists("ofxlog.txt")) {
    d->m_fpTrace.setName(QString("%1/ofxlog.txt").arg(QDir::homePath()));
    d->m_fpTrace.open(QIODevice::WriteOnly | QIODevice::Append);
  }

  m_job = KIO::http_post(
    m_connector.url(),
    request,
    true
  );
  if(d->m_fpTrace.isOpen()) {
    QByteArray data = m_connector.url().utf8();
    d->m_fpTrace.write("url: ", 5);
    d->m_fpTrace.write(data, strlen(data));
    d->m_fpTrace.write("\n", 1);
    d->m_fpTrace.write("request:\n", 9);
    d->m_fpTrace.write(request, request.size());
    d->m_fpTrace.write("\n", 1);
    d->m_fpTrace.write("response:\n", 10);
  }

  m_job->addMetaData("content-type", "Content-type: application/x-ofx" );
  connect(m_job,SIGNAL(result(KIO::Job*)),this,SLOT(slotOfxFinished(KIO::Job*)));
  connect(m_job,SIGNAL(data(KIO::Job*, const QByteArray&)),this,SLOT(slotOfxData(KIO::Job*,const QByteArray&)));
  connect(m_job,SIGNAL(connected(KIO::Job*)),this,SLOT(slotOfxConnected(KIO::Job*)));

  setStatus(QString("Contacting %1...").arg(m_connector.url()));
  kProgress1->setMaximum(3);
  kProgress1->setValue(1);
#endif
}

void KOfxDirectConnectDlg::setStatus(const QString& _status)
{
  textLabel1->setText(_status);
  kDebug(2) << "STATUS: " << _status;
}

void KOfxDirectConnectDlg::setDetails(const QString& _details)
{
  kDebug(2) << "DETAILS: " << _details;
}

void KOfxDirectConnectDlg::slotOfxConnected(KIO::Job*)
{
  if ( m_tmpfile )
  {
//     throw new MYMONEYEXCEPTION(QString("Already connected, using %1.").arg(m_tmpfile->name()));
    kDebug(2) << "Already connected, using " << m_tmpfile->name();
    delete m_tmpfile; //delete otherwise we mem leak
  }
  m_tmpfile = new KTemporaryFile();
  setStatus("Connection established, retrieving data...");
  setDetails(QString("Downloading data to %1...").arg(m_tmpfile->name()));
  kProgress1->setValue(kProgress1->value()+1);
}

void KOfxDirectConnectDlg::slotOfxData(KIO::Job*,const QByteArray& _ba)
{
#warning "port to kde4"	
#if 0	
  if ( !m_tmpfile )
//     throw new MYMONEYEXCEPTION("Not currently connected!!");
    kDebug(2) << "void ofxdcon::slotOfxData():: Not currently connected!";
  *(m_tmpfile->textStream()) << QString(_ba);

  if(d->m_fpTrace.isOpen()) {
    d->m_fpTrace.write(_ba, _ba.size());
  }

  setDetails(QString("Got %1 bytes").arg(_ba.size()));
#endif
}

void KOfxDirectConnectDlg::slotOfxFinished(KIO::Job* /* e */)
{
  kProgress1->setValue(kProgress1->value()+1);
  setStatus("Completed.");

  if(d->m_fpTrace.isOpen()) {
    d->m_fpTrace.write("\nCompleted\n\n\n\n", 14);
  }

  int error = m_job->error();

  if ( m_tmpfile )
  {
    m_tmpfile->close();
  }

  if ( error )
  {
    m_job->showErrorDialog();
  }
  else if ( m_job->isErrorPage() )
  {
    QString details;
    QFile f( m_tmpfile->name() );
    if ( f.open( QIODevice::ReadOnly ) )
    {
      Q3TextStream stream( &f );
      QString line;
      while ( !stream.atEnd() ) {
          details += stream.readLine(); // line of text excluding '\n'
      }
      f.close();

      kDebug(2) << "The HTTP request failed: " << details;
    }
    KMessageBox::detailedSorry( this, i18n("The HTTP request failed."), details, i18n("Failed") );
  }
  else if ( m_tmpfile )
  {

    emit statementReady(m_tmpfile->name());

// TODO (Ace) unlink this file, when I'm sure this is all really working.
// in the meantime, I'll leave the file around to assist people in debugging.
//     m_tmpfile->unlink();
  }
  delete m_tmpfile;
  m_tmpfile = 0;
  hide();
}

void KOfxDirectConnectDlg::reject(void)
{
  m_job->kill();
  if ( m_tmpfile )
  {
    m_tmpfile->close();
    delete m_tmpfile;
    m_tmpfile = NULL;
  }
  QDialog::reject();
}

#include "kofxdirectconnectdlg.moc"
