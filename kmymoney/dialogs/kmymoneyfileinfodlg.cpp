/*
    SPDX-FileCopyrightText: 2005-2009 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kmymoneyfileinfodlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QLabel>
#include <QList>
#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kmymoneyfileinfodlg.h"

#include "mymoneystoragemgr.h"
#include "mymoneyfile.h"
#include "mymoneyinstitution.h"
#include "mymoneyaccount.h"
#include "mymoneypayee.h"
#include "mymoneyprice.h"
#include "mymoneyschedule.h"
#include "mymoneytransaction.h"
#include "mymoneytransactionfilter.h"
#include "mymoneyenums.h"

KMyMoneyFileInfoDlg::KMyMoneyFileInfoDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KMyMoneyFileInfoDlg)
{
    ui->setupUi(this);
    // Now fill the fields with data
    auto storage = MyMoneyFile::instance()->storage();

    ui->m_creationDate->setText(storage->creationDate().toString(Qt::ISODate));
    ui->m_lastModificationDate->setText(storage->lastModificationDate().toString(Qt::ISODate));
    ui->m_baseCurrency->setText(storage->value("kmm-baseCurrency"));

    ui->m_payeeCount->setText(QString::fromLatin1("%1").arg(storage->payeeList().count()));
    ui->m_institutionCount->setText(QString::fromLatin1("%1").arg(storage->institutionList().count()));

    QList<MyMoneyAccount> a_list;
    storage->accountList(a_list);
    ui->m_accountCount->setText(QString::fromLatin1("%1").arg(a_list.count()));

    QMap<eMyMoney::Account::Type, int> accountMap;
    QMap<eMyMoney::Account::Type, int> accountMapClosed;
    QList<MyMoneyAccount>::const_iterator it_a;
    for (it_a = a_list.constBegin(); it_a != a_list.constEnd(); ++it_a) {
        accountMap[(*it_a).accountType()] = accountMap[(*it_a).accountType()] + 1;
        accountMapClosed[(*it_a).accountType()] = accountMapClosed[(*it_a).accountType()] + 0;
        if ((*it_a).isClosed())
            accountMapClosed[(*it_a).accountType()] = accountMapClosed[(*it_a).accountType()] + 1;
    }

    QMap<eMyMoney::Account::Type, int>::const_iterator it_m;
    for (it_m = accountMap.constBegin(); it_m != accountMap.constEnd(); ++it_m) {
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(0, MyMoneyAccount::accountTypeToString(it_m.key()));
        item->setText(1, QString::fromLatin1("%1").arg(*it_m));
        item->setText(2, QString::fromLatin1("%1").arg(accountMapClosed[it_m.key()]));
        ui->m_accountView->invisibleRootItem()->addChild(item);
    }

    MyMoneyTransactionFilter filter;
    filter.setReportAllSplits(false);
    ui->m_transactionCount->setText(QString::fromLatin1("%1").arg(storage->transactionList(filter).count()));
    filter.setReportAllSplits(true);
    ui->m_splitCount->setText(QString::fromLatin1("%1").arg(storage->transactionList(filter).count()));
    ui->m_scheduleCount->setText(QString::fromLatin1("%1").arg(storage->scheduleList(QString(), eMyMoney::Schedule::Type::Any, eMyMoney::Schedule::Occurrence::Any, eMyMoney::Schedule::PaymentType::Any,
                                 QDate(), QDate(), false).count()));
    MyMoneyPriceList list = storage->priceList();
    MyMoneyPriceList::const_iterator it_p;
    int pCount = 0;
    for (it_p = list.constBegin(); it_p != list.constEnd(); ++it_p)
        pCount += (*it_p).count();
    ui->m_priceCount->setText(QString::fromLatin1("%1").arg(pCount));
}

KMyMoneyFileInfoDlg::~KMyMoneyFileInfoDlg()
{
    delete ui;
}
