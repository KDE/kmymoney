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

#include "kguiutils.h"
#include "kmymoneylineedit.h"
#include "onlinejobedit/ionlinejobedit.h"

#include "mymoney/mymoneyfile.h"
#include "mymoney/mymoneyaccount.h"
#include "mymoney/accountidentifier.h"
#include "mymoney/onlinejobadministration.h"

#include "onlinejobedit/germancredittransferedit.h"
#include "onlinejobedit/sepacredittransferedit.h"

#include "models/models.h"

kOnlineTransferForm::kOnlineTransferForm(QWidget *parent)
  : QDialog(parent),
    ui(new Ui::kOnlineTransferFormDecl),
    m_onlineJobEditWidgets( QList<IonlineJobEdit*>() ),
    m_requiredFields( new kMandatoryFieldGroup(this) )
{
  ui->setupUi(this);


  OnlineBankingAccountNamesFilterProxyModel* accountsModel = new OnlineBankingAccountNamesFilterProxyModel(this);  
  accountsModel->setSourceModel( Models::instance()->accountsModel() );
  ui->originAccount->setModel( accountsModel );

  addOnlineJobEditWidget( new sepaCreditTransferEdit() );
  addOnlineJobEditWidget( new germanCreditTransferEdit() );
  
  connect(ui->transferTypeSelection, SIGNAL(currentIndexChanged(int)), this, SLOT(convertCurrentJob(int)));

  connect(ui->buttonAbort, SIGNAL(clicked(bool)), this, SLOT(reject()));
  connect(ui->buttonSend, SIGNAL(clicked(bool)), this, SLOT(sendJob()));
  connect(ui->buttonEnque, SIGNAL(clicked(bool)), this, SLOT(accept()));
  connect(m_requiredFields, SIGNAL(stateChanged(bool)), ui->buttonEnque, SLOT(setEnabled(bool)));
  
  connect(ui->originAccount, SIGNAL(accountSelected(QString)), this, SLOT(accountChanged()));
  
  accountChanged();
  m_requiredFields->add(ui->originAccount);
  m_requiredFields->setOkButton(ui->buttonSend);
}

/**
 * @internal Only add IonlineJobEdit* to ui->creditTransferEdits, static_casts are used!
 */
void kOnlineTransferForm::addOnlineJobEditWidget(IonlineJobEdit* widget)
{
  if (!m_onlineJobEditWidgets.isEmpty())
    widget->setEnabled(false);

  m_onlineJobEditWidgets.append( widget );
  ui->creditTransferEdits->addWidget(widget); // this also pases ownership to QStackedWidget
  ui->transferTypeSelection->addItem(widget->label());
  m_requiredFields->add(widget);
}

void kOnlineTransferForm::convertCurrentJob( const int& index )
{
  Q_ASSERT( index < m_onlineJobEditWidgets.count() );

  const onlineJob job = activeOnlineJob();
  try {
    m_onlineJobEditWidgets.at(index)->setOnlineJob(
      onlineJobAdministration::instance()->convertBest(job, m_onlineJobEditWidgets.at(index)->supportedOnlineTasks(), job.id() )
    );
  } catch ( onlineTask::badConvert* e ) {
    delete e;
  }

  showEditWidget(index);
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

bool kOnlineTransferForm::setOnlineJob(const onlineJob job)
{
  QString name;
  try {
    name = job.task()->taskName();
  } catch ( onlineJob::emptyTask* e ) {
    delete e;
    return false;
  }
  
  setCurrentAccount( job.responsibleAccount() );

  foreach ( IonlineJobEdit* widget, m_onlineJobEditWidgets ) {
    if ( widget->supportedOnlineTasks().contains( name ) ) {
      widget->setOnlineJob(job);
      showEditWidget(widget);
      return true;
    }
  }
  return false;
}

void kOnlineTransferForm::accountChanged()
{
  const QString accountId = ui->originAccount->getSelected();
  try {
    ui->orderAccountBalance->setValue(MyMoneyFile::instance()->balance( accountId ));
  } catch ( MyMoneyException* e ) {
    // @todo this can happen until the selection allows to select correct accounts only
    delete e;
    ui->orderAccountBalance->setText("");
  }

  foreach (IonlineJobEdit* widget, m_onlineJobEditWidgets)
    widget->setOriginAccount( accountId );
  
  checkNotSupportedWidget();
}

bool kOnlineTransferForm::checkEditWidget()
{
  return checkEditWidget( qobject_cast<IonlineJobEdit*>(ui->creditTransferEdits->currentWidget()) );
}

bool kOnlineTransferForm::checkEditWidget( IonlineJobEdit* widget )
{
  if ( widget == 0 ) {
    return false;
  }
  
  if (onlineJobAdministration::instance()->isJobSupported( ui->originAccount->getSelected(), widget->supportedOnlineTasks() )) {
    return true;
  }
  return false;
}

/** @todo auto set another widget if a loseless convert is possible */
void kOnlineTransferForm::checkNotSupportedWidget( )
{
  if ( !checkEditWidget() ) {
    ui->displayStack->setCurrentIndex(0);
  } else {
    ui->displayStack->setCurrentIndex(1);
  }
}

void kOnlineTransferForm::setCurrentAccount( const QString& accountId )
{
  ui->originAccount->setSelected( accountId );
}

onlineJob kOnlineTransferForm::activeOnlineJob() const
{
  IonlineJobEdit* widget = qobject_cast<IonlineJobEdit*>(ui->creditTransferEdits->currentWidget());
  if ( widget == 0 )
    return onlineJob();
  
  return widget->getOnlineJob();
}

void kOnlineTransferForm::showEditWidget(const QString& onlineTaskName)
{
  foreach (IonlineJobEdit* widget, m_onlineJobEditWidgets) {
    if ( widget->supportedOnlineTasks().contains(onlineTaskName) ) {
      const onlineJob convert = onlineJobAdministration::instance()->convert(activeOnlineJob(), onlineTaskName);
      widget->setOnlineJob(convert);
      showEditWidget( widget );
    }
  }
}

/** @internal Changes in this method usually need changes in showEditWidget(const int& index) as well */
void kOnlineTransferForm::showEditWidget( IonlineJobEdit* widget )
{
  ui->creditTransferEdits->currentWidget()->setEnabled(false);
  widget->setEnabled(true);
  ui->creditTransferEdits->setCurrentWidget(widget);
  
  const int index = m_onlineJobEditWidgets.indexOf(widget);
  ui->transferTypeSelection->setCurrentIndex( index );
  
  checkNotSupportedWidget();
  m_requiredFields->changed();
}

/** @internal Changes in this method usually need changes in showEditWidget( IonlineJobEdit* widget ) as well */
void kOnlineTransferForm::showEditWidget(const int& index)
{
  Q_ASSERT( m_onlineJobEditWidgets.count() > index );
  ui->creditTransferEdits->currentWidget()->setEnabled(false);
  ui->creditTransferEdits->setCurrentIndex( index );
  ui->creditTransferEdits->currentWidget()->setEnabled(true);
  
  checkNotSupportedWidget();
  m_requiredFields->changed();
}

kOnlineTransferForm::~kOnlineTransferForm()
{
  delete ui;
}
