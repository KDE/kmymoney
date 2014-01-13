#ifndef SEPACREDITTRANSFEREDIT_H
#define SEPACREDITTRANSFEREDIT_H

#include "mymoney/onlinejobtyped.h"
#include "mymoney/sepaonlinetransfer.h"
#include "ionlinejobedit.h"

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
