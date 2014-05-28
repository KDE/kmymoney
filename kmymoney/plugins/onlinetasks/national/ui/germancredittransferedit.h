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

class kMandatoryFieldGroup;

namespace Ui {
class germanCreditTransferEdit;
}

class germanCreditTransferEdit : public IonlineJobEdit
{
    Q_OBJECT
    Q_PROPERTY(bool readOnly READ isReadOnly WRITE setReadOnly NOTIFY readOnlyChanged);
    Q_PROPERTY(onlineJob job READ getOnlineJob WRITE setOnlineJob STORED false );
    Q_INTERFACES( IonlineJobEdit )

public:
  explicit germanCreditTransferEdit(QWidget *parent = 0, QVariantList args = QVariantList());
  ~germanCreditTransferEdit();

  /** @brief Reads interface and creates an onlineJob */
  onlineJobTyped<germanOnlineTransfer> getOnlineJobTyped() const;
  onlineJob getOnlineJob() const { return getOnlineJobTyped(); }

  QStringList supportedOnlineTasks() const { return QStringList(germanOnlineTransfer::name()); }
  QString label() const { return i18n("German Credit Transfer"); }
  bool isValid() const { return getOnlineJobTyped().isValid(); }

  bool isReadOnly() const { return m_readOnly; }

signals:
  void readOnlyChanged( bool );

public slots:
    bool setOnlineJob( const onlineJob& );
    bool setOnlineJob( const onlineJobTyped<germanOnlineTransfer>& );
    void setOriginAccount( const QString& );
    void setReadOnly( const bool& );

private:
    Ui::germanCreditTransferEdit* ui;
    QString m_originAccount;
    onlineJobTyped<germanOnlineTransfer> m_germanCreditTransfer;
    kMandatoryFieldGroup* m_requiredFields;
    bool m_readOnly;

private slots:
    void beneficiaryNameChanged( const QString& );
    void beneficiaryAccountChanged( const QString& );
    void beneficiaryBankCodeChanged( QString );
    void valueChanged();
    void purposeChanged();

    void updateEveryStatus();
    void updateTaskSettings();

    /**
     * @brief Convenient slot to emit validityChanged()
     *
     * A default implementation to emit validityChanged() based on getOnlineJob().isValid().
     * This is useful if you use @a kMandatoryFieldsGroup in your widget. Just connect kMandatoryFieldsGroup::stateChanged(bool)
     * to this slot.
     *
     * @param status if false, validityChanged(false) is emitted without further checks.
     */
    void requiredFieldsCompleted( const bool& status = true )
    {
      if ( status ) {
        emit validityChanged( getOnlineJobTyped().isValid() );
      } else {
        emit validityChanged( false );
      }
    }
};

#endif // GERMANCREDITTRANSFEREDIT_H
