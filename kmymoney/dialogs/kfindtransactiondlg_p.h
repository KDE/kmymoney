/*
    SPDX-FileCopyrightText: 2002-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KFINDTRANSACTIONDLG_P_H
#define KFINDTRANSACTIONDLG_P_H

#include "kfindtransactiondlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QTimer>
#include <QPushButton>
#include <QPointer>
#include <QHeaderView>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KGuiItem>
#include <KStandardGuiItem>
#include <KLocalizedString>
#include <KLineEdit>

// ----------------------------------------------------------------------------
// Project Includes

#include "ktransactionfilter.h"
#include "mymoneyaccount.h"
#include "mymoneyfile.h"
#include "mymoneymoney.h"
#include "mymoneypayee.h"
#include "mymoneytag.h"
#include "kmymoneysettings.h"
#include "register.h"
#include "transaction.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "mymoneytransactionfilter.h"

#include "ui_kfindtransactiondlg.h"

#include "widgetenums.h"
#include "mymoneyenums.h"

class KFindTransactionDlgPrivate
{
    Q_DISABLE_COPY(KFindTransactionDlgPrivate)
    Q_DECLARE_PUBLIC(KFindTransactionDlg)

public:
    enum opTypeE {
        addAccountToFilter = 0,
        addCategoryToFilter,
        addPayeeToFilter,
        addTagToFilter,
    };

    explicit KFindTransactionDlgPrivate(KFindTransactionDlg *qq) :
        q_ptr(qq),
        ui(new Ui::KFindTransactionDlg),
        m_needReload(true)
    {
    }

    ~KFindTransactionDlgPrivate()
    {
        delete ui;
    }

    void init(bool withEquityAccounts)
    {
        Q_Q(KFindTransactionDlg);
        m_needReload = false;
        ui->setupUi(q);

        m_tabFilters = new KTransactionFilter(q, withEquityAccounts);
        ui->m_tabWidget->insertTab(0, m_tabFilters, i18nc("Criteria tab", "Criteria"));

        ui->m_register->installEventFilter(q);
        ui->m_tabWidget->setTabEnabled(ui->m_tabWidget->indexOf(ui->m_resultPage), false);

        // setup the register
        QList<eWidgets::eTransaction::Column> cols {
            eWidgets::eTransaction::Column::Date,
            eWidgets::eTransaction::Column::Account,
            eWidgets::eTransaction::Column::Detail,
            eWidgets::eTransaction::Column::ReconcileFlag,
            eWidgets::eTransaction::Column::Payment,
            eWidgets::eTransaction::Column::Deposit,
        };
        ui->m_register->setupRegister(MyMoneyAccount(), cols);
        ui->m_register->setSelectionMode(QTableWidget::SingleSelection);

        q->connect(ui->m_register, &KMyMoneyRegister::Register::editTransaction, q, &KFindTransactionDlg::slotSelectTransaction);
        q->connect(ui->m_register->horizontalHeader(), &QWidget::customContextMenuRequested, q, &KFindTransactionDlg::slotSortOptions);

        // setup the connections
        q->connect(ui->buttonBox->button(QDialogButtonBox::Apply), &QAbstractButton::clicked, q, &KFindTransactionDlg::slotSearch);
        q->connect(ui->buttonBox->button(QDialogButtonBox::Reset), &QAbstractButton::clicked, q, &KFindTransactionDlg::slotReset);
        q->connect(ui->buttonBox->button(QDialogButtonBox::Close), &QAbstractButton::clicked, q, &QObject::deleteLater);
        q->connect(ui->buttonBox->button(QDialogButtonBox::Help), &QAbstractButton::clicked, q, &KFindTransactionDlg::slotShowHelp);

        // only allow searches when a selection has been made
        ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
        ui->buttonBox->button(QDialogButtonBox::Apply)->setDefault(true);
        ui->buttonBox->button(QDialogButtonBox::Apply)->setAutoDefault(true);
        KGuiItem::assign(ui->buttonBox->button(QDialogButtonBox::Apply), KStandardGuiItem::find());
        ui->buttonBox->button(QDialogButtonBox::Apply)->setToolTip(i18nc("@info:tooltip for find transaction apply button", "Search transactions"));
        q->connect(m_tabFilters, &KTransactionFilter::selectionNotEmpty, ui->buttonBox->button(QDialogButtonBox::Apply), &QWidget::setEnabled);


        // get signal about engine changes
        q->connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, q, &KFindTransactionDlg::slotRefreshView);

        // set initial focus to text entry field
        KLineEdit* textEdit = m_tabFilters->findChild<KLineEdit*>(QLatin1String("m_textEdit"));
        if (textEdit)
            textEdit->setFocus();
    }

    /**
      * q method loads the register with the matching transactions
      */
    void loadView()
    {
        // setup sort order
        ui->m_register->setSortOrder(KMyMoneySettings::sortSearchView());

        // clear out old data
        ui->m_register->clear();

        // retrieve the list from the engine
        MyMoneyFile::instance()->transactionList(m_transactionList, m_filter);

        // create the elements for the register
        QList<QPair<MyMoneyTransaction, MyMoneySplit> >::const_iterator it;
        QMap<QString, int>uniqueMap;
        MyMoneyMoney deposit, payment;

        int splitCount = 0;
        for (it = m_transactionList.constBegin(); it != m_transactionList.constEnd(); ++it) {
            const MyMoneySplit& split = (*it).second;
            MyMoneyAccount acc = MyMoneyFile::instance()->account(split.accountId());
            ++splitCount;
            uniqueMap[(*it).first.id()]++;

            KMyMoneyRegister::Register::transactionFactory(ui->m_register, (*it).first, (*it).second, uniqueMap[(*it).first.id()]);
            {   // debug stuff
                if (split.shares().isNegative()) {
                    payment += split.shares().abs();
                } else {
                    deposit += split.shares().abs();
                }
            }
        }

        // add the group markers
        ui->m_register->addGroupMarkers();

        // sort the transactions according to the sort setting
        ui->m_register->sortItems();

        // remove trailing and adjacent markers
        ui->m_register->removeUnwantedGroupMarkers();

        // turn on the ledger lens for the register
        ui->m_register->setLedgerLensForced();

        ui->m_register->updateRegister(true);

        ui->m_register->setFocusToTop();
        ui->m_register->selectItem(ui->m_register->focusItem());

#ifdef KMM_DEBUG
        ui->m_foundText->setText(i18np("Found %1 matching transaction (D %2 / P %3 = %4)",
                                       "Found %1 matching transactions (D %2 / P %3 = %4)", splitCount, deposit.formatMoney("", 2), payment.formatMoney("", 2), (deposit - payment).formatMoney("", 2)));
#else
        ui->m_foundText->setText(i18np("Found %1 matching transaction", "Found %1 matching transactions", splitCount));
#endif

        ui->m_tabWidget->setTabEnabled(ui->m_tabWidget->indexOf(ui->m_resultPage), true);
        ui->m_tabWidget->setCurrentIndex(ui->m_tabWidget->indexOf(ui->m_resultPage));

        Q_Q(KFindTransactionDlg);
        QTimer::singleShot(10, q, SLOT(slotRightSize()));
    }


    KFindTransactionDlg      *q_ptr;
    Ui::KFindTransactionDlg  *ui;

    /**
      * q member holds a list of all transactions matching the filter criteria
      */
    QList<QPair<MyMoneyTransaction, MyMoneySplit> > m_transactionList;

    MyMoneyTransactionFilter        m_filter;

    bool                            m_needReload;
    QPointer<KTransactionFilter>            m_tabFilters;

};

#endif
