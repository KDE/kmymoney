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

#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneyprice.h"
#include "mymoneysecurity.h"
#include "mymoneyenums.h"
#include "pricemodel.h"
#include "parametersmodel.h"
#include "payeesmodel.h"
#include "institutionsmodel.h"
#include "journalmodel.h"
#include "schedulesmodel.h"

KMyMoneyFileInfoDlg::KMyMoneyFileInfoDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KMyMoneyFileInfoDlg)
{
  ui->setupUi(this);
  // Now fill the fields with data

  const auto file = MyMoneyFile::instance();
  ui->m_creationDate->setText(file->parametersModel()->itemById(file->fixedKey(MyMoneyFile::CreationDate)).value());
  ui->m_lastModificationDate->setText(file->parametersModel()->itemById(file->fixedKey(MyMoneyFile::LastModificationDate)).value());
  ui->m_baseCurrency->setText(file->baseCurrency().name());

  ui->m_payeeCount->setText(QString::fromLatin1("%1").arg(file->payeesModel()->rowCount()));
  ui->m_institutionCount->setText(QString::fromLatin1("%1").arg(file->institutionsModel()->rowCount()));

  QList<MyMoneyAccount> a_list = file->accountsModel()->itemList();
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

  ui->m_transactionCount->setText(QString::fromLatin1("%1").arg(file->journalModel()->transactionCount(QString())));
  ui->m_splitCount->setText(QString::fromLatin1("%1").arg(file->journalModel()->rowCount()));
  ui->m_scheduleCount->setText(QString::fromLatin1("%1").arg(file->scheduleList().count()));
  ui->m_priceCount->setText(QString::fromLatin1("%1").arg(file->priceModel()->rowCount()));
}

KMyMoneyFileInfoDlg::~KMyMoneyFileInfoDlg()
{
  delete ui;
}
