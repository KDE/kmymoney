/*
    SPDX-FileCopyrightText: 2013-2015 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-laterrg/licenses/>.
*/

#ifndef SEPACREDITTRANSFEREDIT_H
#define SEPACREDITTRANSFEREDIT_H

#include <KLocalizedString>

#include "mymoney/onlinejobtyped.h"
#include "onlinetasks/sepa/sepaonlinetransfer.h"
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
  onlineJob getOnlineJob() const final override {
    return getOnlineJobTyped();
  }

  QStringList supportedOnlineTasks() const final override {
    return QStringList(sepaOnlineTransfer::name());
  }
  QString label() const {
    return i18n("SEPA Credit Transfer");
  }

  bool isValid() const final override {
    return getOnlineJobTyped().isValid();
  }

  bool isReadOnly() const final override {
    return m_readOnly;
  }

  void showAllErrorMessages(const bool) final override;

  void showEvent(QShowEvent*) final override;

Q_SIGNALS:
  void onlineJobChanged();
  void readOnlyChanged(bool);

public Q_SLOTS:
  void setOnlineJob(const onlineJobTyped<sepaOnlineTransfer> &job);
  bool setOnlineJob(const onlineJob& job) final override;
  void setOriginAccount(const QString& accountId) final override;
  void setReadOnly(const bool&);

private Q_SLOTS:
  void updateSettings();
  void updateEveryStatus();

  /** @{
   * These slots are called when the corresponding field is changed
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
