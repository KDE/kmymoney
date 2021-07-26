/*
    SPDX-FileCopyrightText: 2014-2015 Romain Bignon <romain@symlink.me>
    SPDX-FileCopyrightText: 2014-2015 Florent Fourcot <weboob@flo.fourcot.fr>
    SPDX-FileCopyrightText: 2016 Christian David <christian-david@web.de>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <config-kmymoney.h>
#include "woob.h"

#include <memory>

// ----------------------------------------------------------------------------
// QT Includes

#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QProgressDialog>
#ifdef IS_APPIMAGE
#include <QCoreApplication>
#include <QStandardPaths>
#endif

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KMessageBox>
#include <KPluginFactory>

// ----------------------------------------------------------------------------
// Project Includes

#include "mapaccountwizard.h"
#include "accountsettings.h"
#include "woobinterface.h"

#include "mymoneyaccount.h"
#include "mymoneykeyvaluecontainer.h"
#include "mymoneystatement.h"
#include "statementinterface.h"

class WoobPrivate
{
public:
    WoobPrivate()
    {
    }

    ~WoobPrivate()
    {
    }

    bool checkInitialized() const
    {
        if (!woob.isPythonInitialized()) {
            KMessageBox::error(
                nullptr,
                i18n("Woob plugin failed to fully initialize, most likely due to a missing or a misconfigured Python environment. Please refer to the "
                     "manual on how to fix it."));
            return false;
        } else if (!woob.isWoobInitialized()) {
            KMessageBox::error(
                nullptr,
                i18n("Woob plugin failed to fully initialize, most likely due to a missing or a misconfigured Woob Python module. Please refer to the "
                     "manual on how to fix it."));
            return false;
        }

        return true;
    }

    WoobInterface woob;
    QFutureWatcher<WoobInterface::Account> watcher;
    std::unique_ptr<QProgressDialog> progress;
    AccountSettings* accountSettings;
};

Woob::Woob(QObject *parent, const QVariantList &args) :
    KMyMoneyPlugin::Plugin(parent, "woob"),
    d_ptr(new WoobPrivate)
{
    Q_UNUSED(args)
    const auto componentName = QLatin1String("woob");
    const auto rcFileName = QLatin1String("woob.rc");
    setComponentName(componentName, i18n("Woob"));

#ifdef IS_APPIMAGE
    const QString rcFilePath = QString("%1/../share/kxmlgui5/%2/%3").arg(QCoreApplication::applicationDirPath(), componentName, rcFileName);
    setXMLFile(rcFilePath);

    const QString localRcFilePath = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation).first() + QLatin1Char('/') + componentName + QLatin1Char('/') + rcFileName;
    setLocalXMLFile(localRcFilePath);
#else
    setXMLFile(rcFileName);
#endif

    qDebug("Plugins: woob loaded");
}

Woob::~Woob()
{
    Q_D(Woob);
    delete d;
    qDebug("Plugins: woob unloaded");
}

void Woob::plug()
{
    Q_D(Woob);
    connect(&d->watcher, &QFutureWatcher<WoobInterface::Account>::finished, this, &Woob::gotAccount);
}

void Woob::unplug()
{
    Q_D(Woob);
    disconnect(&d->watcher, &QFutureWatcher<WoobInterface::Account>::finished, this, &Woob::gotAccount);
}

void Woob::protocols(QStringList& protocolList) const
{
    protocolList << "woob";
}

QWidget* Woob::accountConfigTab(const MyMoneyAccount& account, QString& tabName)
{
    Q_D(Woob);
    const MyMoneyKeyValueContainer& kvp = account.onlineBankingSettings();
    tabName = i18n("Woob configuration");

    d->accountSettings = new AccountSettings(account, 0);
    d->accountSettings->loadUi(kvp);

    return d->accountSettings;
}

MyMoneyKeyValueContainer Woob::onlineBankingSettings(const MyMoneyKeyValueContainer& current)
{
    Q_D(Woob);
    MyMoneyKeyValueContainer kvp(current);
    kvp["provider"] = objectName().toLower();
    if (d->accountSettings) {
        d->accountSettings->loadKvp(kvp);
    }
    return kvp;
}

bool Woob::mapAccount(const MyMoneyAccount& acc, MyMoneyKeyValueContainer& onlineBankingSettings)
{
    Q_D(Woob);
    Q_UNUSED(acc);

    bool rc = false;

    if (d->checkInitialized()) {
        QPointer<MapAccountWizard> w = new MapAccountWizard(nullptr, &d->woob);
        if (w->exec() == QDialog::Accepted && w != nullptr) {
            onlineBankingSettings.setValue("wb-backend", w->currentBackend());
            onlineBankingSettings.setValue("wb-id", w->currentAccount());
            onlineBankingSettings.setValue("wb-max", "0");
            rc = true;
        }
        delete w;
    }

    return rc;
}

bool Woob::updateAccount(const MyMoneyAccount& kacc, bool moreAccounts)
{
    Q_D(Woob);
    Q_UNUSED(moreAccounts);

    if (d->checkInitialized()) {
        QString bname = kacc.onlineBankingSettings().value("wb-backend");
        QString id = kacc.onlineBankingSettings().value("wb-id");
        QString max = kacc.onlineBankingSettings().value("wb-max");

        d->progress = std::make_unique<QProgressDialog>(nullptr);
        d->progress->setWindowTitle(i18n("Connecting to bank..."));
        d->progress->setLabelText(i18n("Retrieving transactions..."));
        d->progress->setModal(true);
        d->progress->setCancelButton(nullptr);
        d->progress->setMinimum(0);
        d->progress->setMaximum(0);
        d->progress->setMinimumDuration(0);

        QFuture<WoobInterface::Account> future = QtConcurrent::run(&d->woob, &WoobInterface::getAccount, bname, id, max);
        d->watcher.setFuture(future);

        d->progress->exec();
        d->progress.reset();

        return true;
    } else
        return false;
}

void Woob::gotAccount()
{
    Q_D(Woob);
    WoobInterface::Account acc = d->watcher.result();

    MyMoneyAccount kacc = statementInterface()->account("wb-id", acc.id);
    MyMoneyStatement ks;

    ks.m_accountId = kacc.id();
    ks.m_strAccountName = acc.name;
    ks.m_closingBalance = acc.balance;
    if (acc.transactions.length() > 0)
        ks.m_dateEnd = acc.transactions.front().date;

#if 0
    switch (acc.type) {
    case Woob::Account::TYPE_CHECKING:
        ks.m_eType = MyMoneyStatement::etCheckings;
        break;
    case Woob::Account::TYPE_SAVINGS:
        ks.m_eType = MyMoneyStatement::etSavings;
        break;
    case Woob::Account::TYPE_MARKET:
        ks.m_eType = MyMoneyStatement::etInvestment;
        break;
    case Woob::Account::TYPE_DEPOSIT:
    case Woob::Account::TYPE_LOAN:
    case Woob::Account::TYPE_JOINT:
    case Woob::Account::TYPE_UNKNOWN:
        break;
    }
#endif

    for (QListIterator<WoobInterface::Transaction> it(acc.transactions); it.hasNext();) {
        WoobInterface::Transaction tr = it.next();
        MyMoneyStatement::Transaction kt;

        kt.m_strBankID = QLatin1String("ID ") + tr.id;
        kt.m_datePosted = tr.rdate;
        kt.m_amount = tr.amount;
        kt.m_strMemo = tr.raw;
        kt.m_strPayee = tr.label;

        ks.m_listTransactions += kt;
    }

    statementInterface()->import(ks);

    d->progress->hide();
}

K_PLUGIN_FACTORY_WITH_JSON(WoobFactory, "woob.json", registerPlugin<Woob>();)

#include "woob.moc"
