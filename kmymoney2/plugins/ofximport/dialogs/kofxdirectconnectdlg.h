/***************************************************************************
                         kofxdirectconnectdlg.h
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

#ifndef KOFXDIRECTCONNECTDLG_H
#define KOFXDIRECTCONNECTDLG_H

#include "config-kmymoney.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

class KTemporaryFile;

namespace KIO
{
class Job;
class TransferJob;
}

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyofxconnector.h"
#include "ui_kofxdirectconnectdlgdecl.h"

/**
@author ace jones
*/

class KOfxDirectConnectDlgDecl : public QDialog, public Ui::KOfxDirectConnectDlgDecl
{
public:
  KOfxDirectConnectDlgDecl( QWidget *parent ) : QDialog( parent ) {
    setupUi( this );
  }
};

class KOfxDirectConnectDlg : public KOfxDirectConnectDlgDecl
{
Q_OBJECT
public:
  KOfxDirectConnectDlg(const MyMoneyAccount&, QWidget *parent = 0);
  ~KOfxDirectConnectDlg();

  void init(void);

signals:
  /**
    * This signal is emitted when the statement is downloaded
    * and stored in file @a fname.
    */
  void statementReady(const QString& fname);

protected slots:
  void slotOfxFinished(KIO::Job*);
  void slotOfxData(KIO::Job*,const QByteArray&);
  void slotOfxConnected(KIO::Job*);
  virtual void reject(void);

protected:
  void setStatus(const QString& _status);
  void setDetails(const QString& _details);

  KTemporaryFile* m_tmpfile;
  MyMoneyOfxConnector m_connector;
  KIO::TransferJob* m_job;

private:
  /// \internal d-pointer class.
  class Private;
  /// \internal d-pointer instance.
  Private* const d;
};

#endif // KOFXDIRECTCONNECTDLG_H
