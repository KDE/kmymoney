/*
    SPDX-FileCopyrightText: 2004 Martin Preuss <martin@libchipcard.de>


    SPDX-License-Identifier: GPL-2.0-or-later

*/

#ifndef KBMAPACCOUNT_H
#define KBMAPACCOUNT_H

#include <QDialog>

#include <aqbanking/types/account_spec.h>

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

    AB_ACCOUNT_SPEC *getAccount();

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

