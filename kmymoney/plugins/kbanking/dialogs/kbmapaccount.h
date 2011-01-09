/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef KBMAPACCOUNT_H
#define KBMAPACCOUNT_H

#include <QDialog>

#include "mymoneybanking.h"
#include "kbaccountlist.h"

#include <aqbanking/account.h>

class KMyMoneyBanking;

class KBMapAccount: public QDialog
{
  Q_OBJECT
public:
  KBMapAccount(KMyMoneyBanking *kb,
               const char *bankCode,
               const char *accountId,
               QWidget* parent = 0,
               Qt::WFlags fl = 0);
  ~KBMapAccount();

  AB_ACCOUNT *getAccount();

  void accept();

protected slots:
  void slotSelectionChanged();
  void slotHelpClicked();

private:
  /// \internal d-pointer class.
  struct Private;
  /// \internal d-pointer instance.
  Private* const d;
  /*
    KMyMoneyBanking *_banking;
    AB_ACCOUNT *_account;
    KBAccountListView *_accountList;
  */
};





#endif /* QBANKING_MAPACCOUNT_H */

