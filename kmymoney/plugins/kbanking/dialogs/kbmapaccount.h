/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KBMAPACCOUNT_H
#define KBMAPACCOUNT_H

#include <QDialog>

#include <aqbanking/account.h>

class KBankingExt;

class KBMapAccount: public QDialog
{
  Q_OBJECT
public:
  KBMapAccount(KBankingExt *kb,
               const char *bankCode,
               const char *accountId,
               QWidget* parent = 0,
               Qt::WindowFlags fl = 0);
  ~KBMapAccount();

  AB_ACCOUNT *getAccount();

  void accept() final override;

protected Q_SLOTS:
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

