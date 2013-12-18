/*
  This file is part of KMyMoney, A Personal Finance Manager for KDE
  Copyright (C) 2013 Christian Dávid <christian-david@web.de>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "konlinetransferform.h"
#include "ui_konlinetransferformdecl.h"


#include <QtCore/QList>
#include <QtCore/QSharedPointer>
#include <QDebug>

#include "mymoneyaccount.h"
#include "kmymoneylineedit.h"
#include "mymoneyfile.h"

#include "mymoney/accountidentifier.h"
#include "mymoney/onlinejobadministration.h"

kOnlineTransferForm::kOnlineTransferForm(QWidget *parent)
  : QDialog(parent),
    m_activeTransferType( 0 ),
    ui(new Ui::kOnlineTransferFormDecl)
{
  ui->setupUi(this);
  
  //! @todo replace with kMyMoneyAccountCombo or so
  QList<MyMoneyAccount> list;
  MyMoneyFile::instance()->accountList(list);
  MyMoneyAccount acc;
  foreach( acc, list ) {
    if ( onlineJobAdministration::instance()->isJobSupported( acc.id(), germanOnlineTransfer::name() ) != 0 ) {
      ui->orderAccount->addItem(acc.name() + " (" + acc.number() + ')', acc.id());
    }
  }

  if (ui->orderAccount->count() != 0)
    activateSepaTransfer();
  else
    ui->creditTransferEdits->setCurrentIndex(pageUnsupportedByAccount);

  connect(ui->radioTransferNational, SIGNAL(toggled(bool)), this, SLOT(activateGermanTransfer(bool)));
  connect(ui->radioTransferSepa, SIGNAL(toggled(bool)), this, SLOT(activateSepaTransfer(bool)));

  connect(ui->buttonAbort, SIGNAL(clicked(bool)), this, SLOT(reject()));
  connect(ui->buttonSend, SIGNAL(clicked(bool)), this, SLOT(sendJob()));
  connect(ui->buttonEnque, SIGNAL(clicked(bool)), this, SLOT(accept()));
  
  connect(ui->orderAccount, SIGNAL(currentIndexChanged(int)), this, SLOT(accountChanged()));
  
  connect(ui->sepaPage, SIGNAL(onlineJobChanged()), this, SLOT(jobChanged()));
}

onlineJobTyped<onlineTransfer> kOnlineTransferForm::activeOnlineJob() const
{
  if (m_activeTransferType == sepaOnlineTransfer::hash)
    return ui->sepaPage->getOnlineJob();
  else if (m_activeTransferType == germanOnlineTransfer::hash)
    return ui->germanPage->getOnlineJob();
  return onlineJob();
}

void kOnlineTransferForm::activateSepaTransfer(bool active )
{
  if (!active)
    return;

  // Convert german credit transfer if possible
  if( m_activeTransferType == germanOnlineTransfer::hash ) {
    const onlineJob convert = onlineJobAdministration::instance()->convert(ui->germanPage->getOnlineJob(), sepaOnlineTransfer::name(), ui->germanPage->getOnlineJob().id());
    ui->sepaPage->setOnlineJob( convert );
  }
  
  m_activeTransferType = sepaOnlineTransfer::hash;
  ui->sepaPage->setOriginAccount( originAccount() );
  
  setTransferWidget( sepaOnlineTransfer::hash );
}

void kOnlineTransferForm::activateGermanTransfer( bool active )
{
  if (!active)
    return;

  // Convert sepa credit transfer if possible
  if( m_activeTransferType == sepaOnlineTransfer::hash ) {
    const onlineJob convert = onlineJobAdministration::instance()->convert(ui->sepaPage->getOnlineJob(), germanOnlineTransfer::name(), ui->sepaPage->getOnlineJob().id());
    ui->germanPage->setOnlineJob( convert );
  }

  m_activeTransferType = germanOnlineTransfer::hash;
  ui->germanPage->setOriginAccount( originAccount() );
  
  setTransferWidget( germanOnlineTransfer::hash );
}

void kOnlineTransferForm::accept()
{
  emit acceptedForSave( activeOnlineJob() );
  QDialog::accept();
}

void kOnlineTransferForm::sendJob()
{
    emit acceptedForSend( activeOnlineJob() );
    QDialog::accept();
}


void kOnlineTransferForm::reject()
{
  QDialog::reject();
}

bool kOnlineTransferForm::setOnlineJob(const onlineJobTyped<onlineTransfer> transfer)
{
  if (transfer.task()->taskHash() == sepaOnlineTransfer::hash) {
    return setOnlineJob( onlineJobTyped<sepaOnlineTransfer>( transfer ) );
  } else if ( transfer.task()->taskHash() == germanOnlineTransfer::hash ) {
    return setOnlineJob( onlineJobTyped<germanOnlineTransfer>( transfer ));
  }
  return false;
}

bool kOnlineTransferForm::setOnlineJob(const onlineJobTyped<sepaOnlineTransfer> job)
{
  setCurrentAccount( job.responsibleAccount() );
  ui->sepaPage->setOnlineJob( job );
  if ( m_activeTransferType != sepaOnlineTransfer::hash ) {
    activateSepaTransfer( true );
  }
  return true;
}

bool kOnlineTransferForm::setOnlineJob(const onlineJobTyped<germanOnlineTransfer> job)
{
  setCurrentAccount( job.responsibleAccount() );
  ui->germanPage->setOnlineJob( job );
  if ( m_activeTransferType != germanOnlineTransfer::hash ) {
    activateGermanTransfer( true );
  }
  return true;
}

void kOnlineTransferForm::accountChanged()
{
  const QString accountId = originAccount();
  ui->orderAccountBalance->setValue(MyMoneyFile::instance()->balance( accountId ));

  ui->sepaPage->setOriginAccount( accountId );
  ui->germanPage->setOriginAccount( accountId );
  
  setTransferWidget( m_activeTransferType );
}

void kOnlineTransferForm::setCurrentAccount( const QString& accountId )
{
  for( int i = 0; i < ui->orderAccount->count(); ++i ) {
    if (ui->orderAccount->itemData(i).toString() == accountId ) {
      ui->orderAccount->setCurrentIndex(i);
      accountChanged();
      break;
    }
  }
}

inline QString kOnlineTransferForm::originAccount() const
{
  if( ui->orderAccount->count() != 0 )
    return ( ui->orderAccount->itemData(ui->orderAccount->currentIndex()).toString() );
  return QString();
}

void kOnlineTransferForm::setTransferWidget(const size_t& onlineTaskHash)
{
  if ( germanOnlineTransfer::hash == onlineTaskHash )
    ui->radioTransferNational->setChecked(true);
  else if ( sepaOnlineTransfer::hash == onlineTaskHash )
    ui->radioTransferSepa->setChecked(true);
  
  if (!onlineJobAdministration::instance()->isJobSupported( originAccount(), onlineTaskHash))
    ui->creditTransferEdits->setCurrentIndex( pageUnsupportedByAccount );
  else if ( germanOnlineTransfer::hash == onlineTaskHash )
    ui->creditTransferEdits->setCurrentIndex( pageGermanCreditTransfer );
  else if ( sepaOnlineTransfer::hash == onlineTaskHash )
    ui->creditTransferEdits->setCurrentIndex( pageSepaCreditTransfer );
  else
    ui->creditTransferEdits->setCurrentIndex( pageUnsupportedByAccount );
}

kOnlineTransferForm::~kOnlineTransferForm()
{
  delete ui;
}

void kOnlineTransferForm::jobChanged()
{
    if (activeOnlineJob().isValid())
        ui->buttonSend->setEnabled( true );
    else
        ui->buttonSend->setEnabled( false );
}
