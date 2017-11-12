/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2013-2015 Christian Dávid <christian-david@web.de>
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

#include <KLocalizedString>

#include "mymoney/onlinejobtyped.h"
#include "onlinetasks/sepa/tasks/sepaonlinetransfer.h"
#include "onlinetasks/interfaces/ui/ionlinejobedit.h"

class KMandatoryFieldGroup;

namespace Ui
{
class sepaCreditTransferEdit;
}

/**
 * @brief Widget to edit sepaOnlineTransfer
 */
class sepaCreditTransferEdit : public IonlineJobEdit
{
  Q_OBJECT
  Q_PROPERTY(bool readOnly READ isReadOnly WRITE setReadOnly NOTIFY readOnlyChanged);
  Q_PROPERTY(onlineJob job READ getOnlineJob WRITE setOnlineJob);
  Q_INTERFACES(IonlineJobEdit);

public:
  explicit sepaCreditTransferEdit(QWidget *parent = 0, QVariantList args = QVariantList());
  ~sepaCreditTransferEdit();
  onlineJobTyped<sepaOnlineTransfer> getOnlineJobTyped() const;
  onlineJob getOnlineJob() const {
    return getOnlineJobTyped();
  }

  QStringList supportedOnlineTasks() const {
    return QStringList(sepaOnlineTransfer::name());
  }
  QString label() const {
    return i18n("SEPA Credit Transfer");
  };

  bool isValid() const {
    return getOnlineJobTyped().isValid();
  };

  bool isReadOnly() const {
    return m_readOnly;
  }

  virtual void showAllErrorMessages(const bool);

  virtual void showEvent(QShowEvent*);

signals:
  void onlineJobChanged();
  void readOnlyChanged(bool);

public slots:
  void setOnlineJob(const onlineJobTyped<sepaOnlineTransfer> &job);
  bool setOnlineJob(const onlineJob& job);
  void setOriginAccount(const QString& accountId);
  void setReadOnly(const bool&);

private slots:
  void updateSettings();
  void updateEveryStatus();

  /** @{
   * These slots are called when the corosponding field is changed
   * to start the validation.
   */
  void purposeChanged();
  void beneficiaryIbanChanged(const QString& iban);
  void beneficiaryBicChanged(const QString& bic);
  void beneficiaryNameChanged(const QString& name);
  void valueChanged();
  void endToEndReferenceChanged(const QString& reference);
  /** @} */


  /**
   * @brief Convenient slot to emit validityChanged()
   *
   * A default implementation to emit validityChanged() based on getOnlineJob().isValid().
   * This is useful if you use @a kMandatoryFieldsGroup in your widget. Just connect kMandatoryFieldsGroup::stateChanged(bool)
   * to this slot.
   *
   * @param status if false, validityChanged(false) is emitted without further checks.
   */
  void requiredFieldsCompleted(const bool& status = true) {
    if (status) {
      emit validityChanged(getOnlineJobTyped().isValid());
    } else {
      emit validityChanged(false);
    }
  }

private:
  Ui::sepaCreditTransferEdit *ui;
  onlineJobTyped<sepaOnlineTransfer> m_onlineJob;
  KMandatoryFieldGroup* m_requiredFields;
  bool m_readOnly;
  bool m_showAllErrors;

  QSharedPointer<const sepaOnlineTransfer::settings> taskSettings();
};

#endif // SEPACREDITTRANSFEREDIT_H
