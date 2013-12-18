#ifndef SEPACREDITTRANSFEREDIT_H
#define SEPACREDITTRANSFEREDIT_H

#include <QWidget>

#include "mymoney/onlinejobtyped.h"
#include "mymoney/sepaonlinetransfer.h"

namespace Ui {
class sepaCreditTransferEdit;
}

class sepaCreditTransferEdit : public QWidget
{
  Q_OBJECT
    
public:
  explicit sepaCreditTransferEdit(QWidget *parent = 0);
  ~sepaCreditTransferEdit();
  onlineJobTyped<sepaOnlineTransfer> getOnlineJob() const;

signals:
  void onlineJobChanged();
  
public slots:
  void setOnlineJob( const onlineJobTyped<sepaOnlineTransfer> &job );
  void setOnlineJob( const onlineJob& job );
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
};

#endif // SEPACREDITTRANSFEREDIT_H
