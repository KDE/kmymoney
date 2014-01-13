#ifndef GERMANCREDITTRANSFEREDIT_H
#define GERMANCREDITTRANSFEREDIT_H

#include "ionlinejobedit.h"
#include "mymoney/onlinejobtyped.h"
#include "mymoney/germanonlinetransfer.h"
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

public slots:
    bool setOnlineJob( const onlineJob& );
    bool setOnlineJob( const onlineJobTyped<germanOnlineTransfer>& );
    void setOriginAccount( const QString& );

private:
    QString m_originAccount;
    onlineJobTyped<germanOnlineTransfer> m_germanCreditTransfer;
    kMandatoryFieldGroup* m_requiredFields;
    Ui::germanCreditTransferEdit* ui;
    
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
