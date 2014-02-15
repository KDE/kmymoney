/*
 * This file is part of KMyMoney, A Personal Finance Manager for KDE
 * Copyright (C) 2013 Christian Dávid <christian-david@web.de>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SEPACREDITTRANSFEREDIT_H
#define SEPACREDITTRANSFEREDIT_H

#include <KLocale>

#include "mymoney/onlinejobtyped.h"
#include "onlinetasks/sepa/tasks/sepaonlinetransfer.h"
#include "onlinetasks/interfaces/ui/ionlinejobedit.h"

class kMandatoryFieldGroup;

namespace Ui {
class sepaCreditTransferEdit;
}

class sepaCreditTransferEdit : public IonlineJobEdit
{
  Q_OBJECT
    
public:
  explicit sepaCreditTransferEdit(QWidget *parent = 0);
  ~sepaCreditTransferEdit();
  onlineJobTyped<sepaOnlineTransfer> getOnlineJobTyped() const;
  onlineJob getOnlineJob() const { return getOnlineJobTyped(); }

  QStringList supportedOnlineTasks() const { return QStringList( sepaOnlineTransfer::name() ); }
  QString label() const { return i18n("SEPA Credit Transfer"); };
signals:
  void onlineJobChanged();
  
public slots:
  void setOnlineJob( const onlineJobTyped<sepaOnlineTransfer> &job );
  bool setOnlineJob( const onlineJob& job );
  void setOriginAccount(const QString& accountId );

private slots:
  void updateSettings();
  void updateEveryStatus();
  
  /** @{
   * These slots are called when the corosponding field is changed
   * to start the validation
   */
  void purposeChanged();
  void beneficiaryIbanChanged( const QString& iban );
  void beneficiaryNameChanged( const QString& name );
  void beneficiaryBicChanged( const QString& bic );
  void valueChanged();
  void endToEndReferenceChanged( const QString& reference );
  /** @} */

private:
  Ui::sepaCreditTransferEdit *ui;
  onlineJobTyped<sepaOnlineTransfer> m_onlineJob;
  kMandatoryFieldGroup* m_requiredFields;
  
  QSharedPointer<const sepaOnlineTransfer::settings> taskSettings();
};

#endif // SEPACREDITTRANSFEREDIT_H
