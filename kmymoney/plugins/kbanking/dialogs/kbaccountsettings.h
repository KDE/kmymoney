/*
    SPDX-FileCopyrightText: 2009 Cristian Onet onet.cristian @gmail.com
    SPDX-FileCopyrightText: 2008 Thomas Baumgart ipwizard @users.sourceforge.net
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KBANKING_KBACCOUNTSETTINGS_H
#define KBANKING_KBACCOUNTSETTINGS_H

#include <QWidget>

class MyMoneyAccount;
class MyMoneyKeyValueContainer;

class KBAccountSettings: public QWidget
{
public:
    KBAccountSettings(const MyMoneyAccount& acc, QWidget* parent);
    ~KBAccountSettings();

    void loadUi(const MyMoneyKeyValueContainer& kvp);
    void loadKvp(MyMoneyKeyValueContainer& kvp);

private:
    /// \internal d-pointer class.
    struct Private;
    /// \internal d-pointer instance.
    Private* const d;
};


#endif /* KBANKING_KBACCOUNTSETTINGS_H */
