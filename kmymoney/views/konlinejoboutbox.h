#ifndef KONLINEJOBOUTBOX_H
#define KONLINEJOBOUTBOX_H

#include <QWidget>
#include <QModelIndex>

#include "onlinejob.h"

namespace Ui {
class KOnlineJobOutbox;
}

class KOnlineJobOutbox : public QWidget
{
  Q_OBJECT
    
public:
  explicit KOnlineJobOutbox(QWidget *parent = 0);
  ~KOnlineJobOutbox();

signals:
  void sendJobs( QList<onlineJob> );
  void editJob( QString );
  void newCreditTransfer();
    
private:
  Ui::KOnlineJobOutbox *ui;

private slots:
  void slotRemoveJob();
  void slotSendJobs();
  void slotEditJob();
  void slotEditJob( const QModelIndex& );
};

#endif // KONLINEJOBOUTBOX_H
