#ifndef SEPACREDITTRANSFEREDIT_H
#define SEPACREDITTRANSFEREDIT_H

#include <QWidget>

#include "mymoney/onlinejobknowntask.h"
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
  onlineJobKnownTask<sepaOnlineTransfer> getOnlineJob() const;

public slots:
  void setOnlineJob( const onlineJobKnownTask<sepaOnlineTransfer> &job );
  void setOnlineJob( const onlineJob& job );
  void setOriginAccount(const QString& accountId );

private slots:
  void updateSettings();

private:
  Ui::sepaCreditTransferEdit *ui;
  onlineJobKnownTask<sepaOnlineTransfer> m_onlineJob;
};

#endif // SEPACREDITTRANSFEREDIT_H
