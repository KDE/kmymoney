/*
    SPDX-FileCopyrightText: 2005-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kmmstatementinterface.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KMessageBox>

// ----------------------------------------------------------------------------
// Project Includes

#include "importsummarydlg.h"
#include "mymoneyaccount.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneykeyvaluecontainer.h"
#include "mymoneystatementreader.h"

KMyMoneyPlugin::KMMStatementInterface::KMMStatementInterface(QObject* parent, const char* name)
    : StatementInterface(parent, name)
{
}

bool KMyMoneyPlugin::KMMStatementInterface::import(const MyMoneyStatement& s)
{
    qDebug("KMyMoneyPlugin::KMMStatementInterface::import start");
    return MyMoneyStatementReader::importStatement(s);
}

void KMyMoneyPlugin::KMMStatementInterface::resetMessages() const
{
    MyMoneyStatementReader::clearImportResults();
}

void KMyMoneyPlugin::KMMStatementInterface::showMessages() const
{
    QScopedPointer<ImportSummaryDialog> dlg(new ImportSummaryDialog(nullptr));
    dlg->setModel(MyMoneyStatementReader::importResultsModel());
    dlg->exec();
}

MyMoneyAccount KMyMoneyPlugin::KMMStatementInterface::account(const QString& key, const QString& value) const
{
    QList<MyMoneyAccount> list;
    QList<MyMoneyAccount>::const_iterator it_a;
    MyMoneyFile::instance()->accountList(list);
    QString accId;
    for (it_a = list.cbegin(); it_a != list.cend(); ++it_a) {
        // search in the account's kvp container
        const auto& accountKvpValue = (*it_a).value(key);
        // search in the account's online settings kvp container
        const auto& onlineSettingsKvpValue = (*it_a).onlineBankingSettings().value(key);
        if (accountKvpValue.contains(value) || onlineSettingsKvpValue.contains(value)) {
            if (accId.isEmpty()) {
                accId = (*it_a).id();
            }
        }
        if (accountKvpValue == value || onlineSettingsKvpValue == value) {
            accId = (*it_a).id();
            break;
        }
    }

    // return the account found or an empty element
    return MyMoneyFile::instance()->account(accId);
}

void KMyMoneyPlugin::KMMStatementInterface::setAccountOnlineParameters(const MyMoneyAccount& acc, const MyMoneyKeyValueContainer& kvps) const
{
    MyMoneyFileTransaction ft;
    try {
        auto oAcc = MyMoneyFile::instance()->account(acc.id());
        oAcc.setOnlineBankingSettings(kvps);
        MyMoneyFile::instance()->modifyAccount(oAcc);
        ft.commit();

    } catch (const MyMoneyException&) {
        qDebug("Unable to setup online parameters for account '%s'", qPrintable(acc.name()));
        //    KMessageBox::detailedError(0, i18n("Unable to setup online parameters for account '%1'", acc.name()), QString::fromLatin1(e.what()));
    }
}
