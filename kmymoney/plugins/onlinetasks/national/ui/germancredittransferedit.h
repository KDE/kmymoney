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

#ifndef GERMANCREDITTRANSFEREDIT_H
#define GERMANCREDITTRANSFEREDIT_H

#include <KLocale>

#include "onlinetasks/interfaces/ui/ionlinejobedit.h"
#include "mymoney/onlinejobtyped.h"
#include "../tasks/germanonlinetransfer.h"
#include "mymoney/swiftaccountidentifier.h"

class kMandatoryFieldGroup;

namespace Ui {
class germanCreditTransferEdit;
}

class germanCreditTransferEdit : public IonlineJobEdit
{
    Q_OBJECT
    
public:
  explicit germanCreditTransferEdit(QWidget *parent = 0);
  ~germanCreditTransferEdit();

  /** @brief Reads interface and creates an onlineJob */
  onlineJobTyped<germanOnlineTransfer> getOnlineJobTyped() const;
  onlineJob getOnlineJob() const { return getOnlineJobTyped(); }

  QStringList supportedOnlineTasks() const { return QStringList(germanOnlineTransfer::name()); }
  QString label() const { return i18n("German Credit Transfer"); }
  
public slots:
    bool setOnlineJob( const onlineJob& );
    bool setOnlineJob( const onlineJobTyped<germanOnlineTransfer>& );
    void setOriginAccount( const QString& );

private:
    Ui::germanCreditTransferEdit* ui;
    QString m_originAccount;
    onlineJobTyped<germanOnlineTransfer> m_germanCreditTransfer;
    kMandatoryFieldGroup* m_requiredFields;
    
private slots:
    void beneficiaryNameChanged( const QString& );
    void beneficiaryAccountChanged( const QString& );
    void beneficiaryBankCodeChanged( QString );
    void valueChanged();
    void purposeChanged();
    
    void updateEveryStatus();
    void updateTaskSettings();
};

#endif // GERMANCREDITTRANSFEREDIT_H
