/*
    SPDX-FileCopyrightText: 2000-2002 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 Javier Campos Morales <javi_c@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 Felix Rodriguez <frodriguez@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 John C <thetacoturtle@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 Andreas Nicolai <Andreas.Nicolai@gmx.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KPAYEESVIEW_P_H
#define KPAYEESVIEW_P_H

#include "kpayeesview.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QMap>
#include <QList>
#include <QTimer>
#include <QDesktopServices>
#include <QIcon>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KMessageBox>
#include <KHelpClient>
#include <KSharedConfig>
#include <KListWidgetSearchLine>

// ----------------------------------------------------------------------------
// Project Includes

#include <config-kmymoney.h>
#include "ui_kpayeesview.h"
#include "kmymoneyviewbase_p.h"
#include "kmymoneyutils.h"
#include "kmymoneymvccombo.h"
#include "mymoneypayee.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneymoney.h"
#include "mymoneytransactionfilter.h"
#include "kmymoneysettings.h"
#include "kpayeereassigndlg.h"
#include "models.h"
#include "accountsmodel.h"
#include "mymoneysecurity.h"
#include "mymoneyschedule.h"
#include "mymoneycontact.h"
#include "mymoneyaccountloan.h"
#include "mymoneysplit.h"
#include "mymoneyprice.h"
#include "mymoneytransaction.h"
#include "icons/icons.h"
#include "transaction.h"
#include "widgetenums.h"
#include "mymoneyenums.h"
#include "modelenums.h"
#include "menuenums.h"

using namespace Icons;

// *** KPayeeListItem Implementation ***

class KPayeeListItem : public QListWidgetItem
{
public:
    /**
      * Constructor to be used to construct a payee entry object.
      *
      * @param parent pointer to the QListWidget object this entry should be
      *               added to.
      * @param payee const reference to MyMoneyPayee for which
      *               the QListWidget entry is constructed
      */
    explicit KPayeeListItem(QListWidget *parent, const MyMoneyPayee& payee) :
        QListWidgetItem(parent, QListWidgetItem::UserType),
        m_payee(payee)
    {
        setText(payee.name());
        // allow in column rename
        setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    }

    ~KPayeeListItem()
    {
    }

    const MyMoneyPayee& payee() const {
        return m_payee;
    }

private:
    MyMoneyPayee  m_payee;
};

enum filterTypeE { eAllPayees = 0, eReferencedPayees = 1, eUnusedPayees = 2 };

class KPayeesViewPrivate : public KMyMoneyViewBasePrivate
{
    Q_DECLARE_PUBLIC(KPayeesView)

public:
    explicit KPayeesViewPrivate(KPayeesView *qq) :
        KMyMoneyViewBasePrivate(),
        q_ptr(qq),
        ui(new Ui::KPayeesView),
        m_contact(nullptr),
        m_payeeRow(0),
        m_needLoad(true),
        m_searchWidget(nullptr),
        m_inSelection(false),
        m_allowEditing(true),
        m_payeeFilterType(0),
        m_filterProxyModel(nullptr)
    {
    }

    ~KPayeesViewPrivate()
    {
        if(!m_needLoad) {
            // remember the splitter settings for startup
            auto grp = KSharedConfig::openConfig()->group("Last Use Settings");
            grp.writeEntry("KPayeesViewSplitterSize", ui->m_splitter->saveState());
            grp.sync();
        }
        delete ui;
    }

    void init()
    {
        Q_Q(KPayeesView);
        m_needLoad = false;
        ui->setupUi(q);

        m_contact = new MyMoneyContact(q);
        m_filterProxyModel = new AccountNamesFilterProxyModel(q);
        m_filterProxyModel->setHideEquityAccounts(!KMyMoneySettings::expertMode());
        m_filterProxyModel->addAccountGroup(QVector<eMyMoney::Account::Type> {eMyMoney::Account::Type::Asset, eMyMoney::Account::Type::Liability, eMyMoney::Account::Type::Income, eMyMoney::Account::Type::Expense, eMyMoney::Account::Type::Equity});

        auto const model = Models::instance()->accountsModel();
        m_filterProxyModel->setSourceColumns(model->getColumns());
        m_filterProxyModel->setSourceModel(model);
        m_filterProxyModel->sort((int)eAccountsModel::Column::Account);
        ui->comboDefaultCategory->setModel(m_filterProxyModel);

        ui->matchTypeCombo->addItem(i18nc("@item No matching", "No matching"), static_cast<int>(eMyMoney::Payee::MatchType::Disabled));
        ui->matchTypeCombo->addItem(i18nc("@item Match Payees name partially", "Match Payees name (partial)"), static_cast<int>(eMyMoney::Payee::MatchType::Name));
        ui->matchTypeCombo->addItem(i18nc("@item Match Payees name exactly", "Match Payees name (exact)"), static_cast<int>(eMyMoney::Payee::MatchType::NameExact));
        ui->matchTypeCombo->addItem(i18nc("@item Search match in list", "Match on a name listed below"), static_cast<int>(eMyMoney::Payee::MatchType::Key));

        // create the searchline widget
        // and insert it into the existing layout
        m_searchWidget = new KListWidgetSearchLine(q, ui->m_payeesList);
        m_searchWidget->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
        ui->m_payeesList->setContextMenuPolicy(Qt::CustomContextMenu);
        ui->m_listTopHLayout->insertWidget(0, m_searchWidget);

        //load the filter type
        ui->m_filterBox->addItem(i18nc("@item Show all payees", "All"));
        ui->m_filterBox->addItem(i18nc("@item Show only used payees", "Used"));
        ui->m_filterBox->addItem(i18nc("@item Show only unused payees", "Unused"));
        ui->m_filterBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);

        ui->m_newButton->setIcon(Icons::get(Icon::ListAddUser));
        ui->m_renameButton->setIcon(Icons::get(Icon::UserProperties));
        ui->m_deleteButton->setIcon(Icons::get(Icon::ListRemoveUser));
        ui->m_mergeButton->setIcon(Icons::get(Icon::Merge));
        ui->m_updateButton->setIcon(Icons::get(Icon::DialogOK));
        ui->m_syncAddressbook->setIcon(Icons::get(Icon::Refresh));
        ui->m_sendMail->setIcon(Icons::get(Icon::MailMessage));

        ui->m_updateButton->setEnabled(false);
        ui->m_syncAddressbook->setEnabled(false);
#ifndef ENABLE_ADDRESSBOOK
        ui->m_syncAddressbook->hide();
#endif
        ui->matchTypeCombo->setCurrentIndex(0);

        ui->checkMatchIgnoreCase->setEnabled(false);

        ui->checkEnableDefaultCategory->setChecked(false);
        ui->labelDefaultCategory->setEnabled(false);
        ui->comboDefaultCategory->setEnabled(false);

        QList<eWidgets::eTransaction::Column> cols {
            eWidgets::eTransaction::Column::Date,
            eWidgets::eTransaction::Column::Account,
            eWidgets::eTransaction::Column::Detail,
            eWidgets::eTransaction::Column::ReconcileFlag,
            eWidgets::eTransaction::Column::Payment,
            eWidgets::eTransaction::Column::Deposit};
        ui->m_register->setupRegister(MyMoneyAccount(), cols);
        ui->m_register->setSelectionMode(QTableWidget::SingleSelection);
        ui->m_register->setDetailsColumnType(eWidgets::eRegister::DetailColumn::AccountFirst);
        ui->m_balanceLabel->hide();

        q->connect(m_contact, &MyMoneyContact::contactFetched, q, &KPayeesView::slotContactFetched);

        q->connect(ui->m_payeesList, static_cast<void (QListWidget::*)(QListWidgetItem*, QListWidgetItem*)>(&QListWidget::currentItemChanged), q, static_cast<void (KPayeesView::*)(QListWidgetItem*, QListWidgetItem*)>(&KPayeesView::slotSelectPayee));
        q->connect(ui->m_payeesList, &QListWidget::itemSelectionChanged, q,   static_cast<void (KPayeesView::*)()>(&KPayeesView::slotSelectPayee));
        q->connect(ui->m_payeesList, &QListWidget::itemDoubleClicked, q,      &KPayeesView::slotStartRename);
        q->connect(ui->m_payeesList, &QListWidget::itemChanged, q,            &KPayeesView::slotRenameSinglePayee);
        q->connect(ui->m_payeesList, &QWidget::customContextMenuRequested, q, &KPayeesView::slotShowPayeesMenu);

        q->connect(ui->m_newButton,     &QAbstractButton::clicked, q, &KPayeesView::slotNewPayee);
        q->connect(ui->m_renameButton,  &QAbstractButton::clicked, q, &KPayeesView::slotRenamePayee);
        q->connect(ui->m_deleteButton,  &QAbstractButton::clicked, q, &KPayeesView::slotDeletePayee);
        q->connect(ui->m_mergeButton,   &QAbstractButton::clicked, q, &KPayeesView::slotMergePayee);

        q->connect(ui->addressEdit,   &QTextEdit::textChanged, q, &KPayeesView::slotPayeeDataChanged);
        q->connect(ui->payeecityEdit,  &QLineEdit::textChanged, q, &KPayeesView::slotPayeeDataChanged);
        q->connect(ui->payeestateEdit,  &QLineEdit::textChanged, q, &KPayeesView::slotPayeeDataChanged);
        q->connect(ui->postcodeEdit,  &QLineEdit::textChanged, q, &KPayeesView::slotPayeeDataChanged);
        q->connect(ui->telephoneEdit, &QLineEdit::textChanged, q, &KPayeesView::slotPayeeDataChanged);
        q->connect(ui->emailEdit,     &QLineEdit::textChanged, q, &KPayeesView::slotPayeeDataChanged);
        q->connect(ui->notesEdit,     &QTextEdit::textChanged, q, &KPayeesView::slotPayeeDataChanged);
        q->connect(ui->matchKeyEditList, &KEditListWidget::changed, q, &KPayeesView::slotKeyListChanged);

        q->connect(ui->matchTypeCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), q, &KPayeesView::slotPayeeDataChanged);
        q->connect(ui->checkMatchIgnoreCase, &QAbstractButton::toggled, q, &KPayeesView::slotPayeeDataChanged);

        q->connect(ui->checkEnableDefaultCategory,  &QAbstractButton::toggled, q,               &KPayeesView::slotPayeeDataChanged);
        q->connect(ui->comboDefaultCategory,        &KMyMoneyAccountCombo::accountSelected, q,  &KPayeesView::slotPayeeDataChanged);
        q->connect(ui->buttonSuggestACategory,      &QAbstractButton::clicked, q,               &KPayeesView::slotChooseDefaultAccount);

        q->connect(ui->m_updateButton,    &QAbstractButton::clicked, q, &KPayeesView::slotUpdatePayee);
        q->connect(ui->m_syncAddressbook, &QAbstractButton::clicked, q, &KPayeesView::slotSyncAddressBook);
        q->connect(ui->m_helpButton,      &QAbstractButton::clicked, q, &KPayeesView::slotHelp);
        q->connect(ui->m_sendMail,        &QAbstractButton::clicked, q, &KPayeesView::slotSendMail);

        q->connect(ui->m_register, &KMyMoneyRegister::Register::editTransaction, q, &KPayeesView::slotSelectTransaction);

        q->connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, q, &KPayeesView::refresh);

        q->connect(ui->m_filterBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), q, &KPayeesView::slotChangeFilter);

        q->connect(ui->payeeIdentifiers, &KPayeeIdentifierView::dataChanged, q, &KPayeesView::slotPayeeDataChanged);

        // use the size settings of the last run (if any)
        KConfigGroup grp = KSharedConfig::openConfig()->group("Last Use Settings");
        ui->m_splitter->restoreState(grp.readEntry("KPayeesViewSplitterSize", QByteArray()));
        ui->m_splitter->setChildrenCollapsible(false);

        //At start we haven't any payee selected
        ui->m_tabWidget->setEnabled(false); // disable tab widget
        ui->m_deleteButton->setEnabled(false); //disable delete, rename and merge buttons
        ui->m_renameButton->setEnabled(false);
        ui->m_mergeButton->setEnabled(false);
        m_payee = MyMoneyPayee(); // make sure we don't access an undefined payee
        clearItemData();
    }

    void loadPayees()
    {
        Q_Q(KPayeesView);
        if (m_inSelection)
            return;

        QMap<QString, bool> isSelected;
        QString id;
        MyMoneyFile* file = MyMoneyFile::instance();

        // remember which items are selected in the list
        QList<QListWidgetItem *> selectedItems = ui->m_payeesList->selectedItems();
        QList<QListWidgetItem *>::const_iterator payeesIt = selectedItems.constBegin();

        while (payeesIt != selectedItems.constEnd()) {
            KPayeeListItem* item = dynamic_cast<KPayeeListItem*>(*payeesIt);
            if (item)
                isSelected[item->payee().id()] = true;
            ++payeesIt;
        }

        // keep current selected item
        KPayeeListItem *currentItem = static_cast<KPayeeListItem *>(ui->m_payeesList->currentItem());
        if (currentItem)
            id = currentItem->payee().id();

        m_allowEditing = false;
        // clear the list
        m_searchWidget->clear();
        m_searchWidget->updateSearch();
        ui->m_payeesList->clear();
        ui->m_register->clear();
        currentItem = 0;

        QList<MyMoneyPayee>list = file->payeeList();
        QList<MyMoneyPayee>::ConstIterator it;

        for (it = list.constBegin(); it != list.constEnd(); ++it) {
            if (m_payeeFilterType == eAllPayees ||
                    (m_payeeFilterType == eReferencedPayees && file->isReferenced(*it)) ||
                    (m_payeeFilterType == eUnusedPayees && !file->isReferenced(*it))) {
                KPayeeListItem* item = new KPayeeListItem(ui->m_payeesList, *it);
                if (item->payee().id() == id)
                    currentItem = item;
                if (isSelected[item->payee().id()])
                    item->setSelected(true);
            }
        }
        ui->m_payeesList->sortItems();

        if (currentItem) {
            ui->m_payeesList->setCurrentItem(currentItem);
            ui->m_payeesList->scrollToItem(currentItem);
        }

        m_filterProxyModel->invalidate();
        ui->comboDefaultCategory->expandAll();

        q->slotSelectPayee(0, 0);
        m_allowEditing = true;
    }

    void selectedPayees(QList<MyMoneyPayee>& payeesList) const
    {
        QList<QListWidgetItem *> selectedItems = ui->m_payeesList->selectedItems();
        QList<QListWidgetItem *>::ConstIterator itemsIt = selectedItems.constBegin();
        while (itemsIt != selectedItems.constEnd()) {
            KPayeeListItem* item = dynamic_cast<KPayeeListItem*>(*itemsIt);
            if (item)
                payeesList << item->payee();
            ++itemsIt;
        }
    }

    void ensurePayeeVisible(const QString& id)
    {
        for (int i = 0; i < ui->m_payeesList->count(); ++i) {
            KPayeeListItem* p = dynamic_cast<KPayeeListItem*>(ui->m_payeesList->item(0));
            if (p && p->payee().id() == id) {
                ui->m_payeesList->scrollToItem(p, QAbstractItemView::PositionAtCenter);

                ui->m_payeesList->setCurrentItem(p);      // active item and deselect all others
                ui->m_payeesList->setCurrentRow(i, QItemSelectionModel::ClearAndSelect);   // and select it
                break;
            }
        }
    }

    void clearItemData()
    {
        ui->addressEdit->setText(QString());
        ui->payeecityEdit->setText(QString());
        ui->payeestateEdit->setText(QString());
        ui->postcodeEdit->setText(QString());
        ui->telephoneEdit->setText(QString());
        ui->emailEdit->setText(QString());
        ui->notesEdit->setText(QString());
        showTransactions();
    }

    /**
      * This method loads the m_transactionList, clears
      * the m_TransactionPtrVector and rebuilds and sorts
      * it according to the current settings. Then it
      * loads the m_transactionView with the transaction data.
      */
    void showTransactions()
    {
        MyMoneyMoney balance;
        const auto file = MyMoneyFile::instance();
        MyMoneySecurity base = file->baseCurrency();

        // setup sort order
        ui->m_register->setSortOrder(KMyMoneySettings::sortSearchView());

        // clear the register
        ui->m_register->clear();

        if (m_selectedPayeesList.isEmpty() || !ui->m_tabWidget->isEnabled()) {
            ui->m_balanceLabel->setText(i18n("Balance: %1", balance.formatMoney(file->baseCurrency().smallestAccountFraction())));
            return;
        }

        // setup the list and the pointer vector
        MyMoneyTransactionFilter filter;

        for (QList<MyMoneyPayee>::const_iterator it = m_selectedPayeesList.constBegin();
                it != m_selectedPayeesList.constEnd();
                ++it)
            filter.addPayee((*it).id());

        filter.setDateFilter(KMyMoneySettings::startDate().date(), QDate());
        filter.setConsiderCategorySplits(true);

        // retrieve the list from the engine
        file->transactionList(m_transactionList, filter);

        // create the elements for the register
        QList<QPair<MyMoneyTransaction, MyMoneySplit> >::const_iterator it;
        QMap<QString, int> uniqueMap;
        MyMoneyMoney deposit, payment;

        int splitCount = 0;
        bool balanceAccurate = true;
        for (it = m_transactionList.constBegin(); it != m_transactionList.constEnd(); ++it) {
            const MyMoneySplit& split = (*it).second;
            MyMoneyAccount acc = file->account(split.accountId());
            if (!acc.isIncomeExpense()) {
                ++splitCount;
                uniqueMap[(*it).first.id()]++;

                KMyMoneyRegister::Register::transactionFactory(ui->m_register, (*it).first, (*it).second, uniqueMap[(*it).first.id()]);

                // take care of foreign currencies
                MyMoneyMoney val = split.shares().abs();
                if (acc.currencyId() != base.id()) {
                    const MyMoneyPrice& price = file->price(acc.currencyId(), base.id());
                    // in case the price is valid, we use it. Otherwise, we keep
                    // a flag that tells us that the balance is somewhat inaccurate
                    if (price.isValid()) {
                        val *= price.rate(base.id());
                    } else {
                        balanceAccurate = false;
                    }
                }

                if (split.shares().isNegative()) {
                    payment += val;
                } else {
                    deposit += val;
                }
            }
        }
        balance = deposit - payment;

        // add the group markers
        ui->m_register->addGroupMarkers();

        // sort the transactions according to the sort setting
        ui->m_register->sortItems();

        // remove trailing and adjacent markers
        ui->m_register->removeUnwantedGroupMarkers();

        ui->m_register->updateRegister(true);

        // we might end up here with updates disabled on the register so
        // make sure that we enable updates here
        ui->m_register->setUpdatesEnabled(true);
        ui->m_balanceLabel->setText(i18n("Balance: %1%2",
                                         balanceAccurate ? "" : "~",
                                         balance.formatMoney(file->baseCurrency().smallestAccountFraction())));
    }

    /**
      * Implement common task when deleting or merging payees
      */
    bool payeeReassign(int type)
    {
        Q_Q(KPayeesView);
        if (!(type >= 0 && type < KPayeeReassignDlg::TypeCount))
            return false;

        auto addPatternOrName = [&](const QString& pattern) {
            if (pattern.startsWith(QLatin1String("^")) && pattern.startsWith(QLatin1String("^"))) {
                return pattern;
            }
            return QRegularExpression::escape(pattern);
        };

        const auto file = MyMoneyFile::instance();

        MyMoneyFileTransaction ft;
        try {
            // create a transaction filter that contains all payees selected for removal
            MyMoneyTransactionFilter f = MyMoneyTransactionFilter();
            for (QList<MyMoneyPayee>::const_iterator it = m_selectedPayeesList.constBegin();
                    it != m_selectedPayeesList.constEnd(); ++it) {
                f.addPayee((*it).id());
            }
            f.setConsiderCategorySplits(true);
            // request a list of all transactions that still use the payees in question
            QList<MyMoneyTransaction> translist = file->transactionList(f);
            //     qDebug() << "[KPayeesView::slotDeletePayee]  " << translist.count() << " transaction still assigned to payees";

            // now get a list of all schedules that make use of one of the payees
            QList<MyMoneySchedule> used_schedules;
            foreach (const auto schedule, file->scheduleList()) {
                // loop over all splits in the transaction of the schedule
                foreach (const auto split, schedule.transaction().splits()) {
                    // is the payee in the split to be deleted?
                    if (payeeInList(m_selectedPayeesList, split.payeeId())) {
                        used_schedules.push_back(schedule); // remember this schedule
                        break;
                    }
                }
            }
            //     qDebug() << "[KPayeesView::slotDeletePayee]  " << used_schedules.count() << " schedules use one of the selected payees";

            // and a list of all loan accounts that references one of the payees
            QList<MyMoneyAccount> allAccounts;
            QList<MyMoneyAccount> usedAccounts;
            file->accountList(allAccounts);
            foreach (const MyMoneyAccount &account, allAccounts) {
                if (account.isLoan()) {
                    MyMoneyAccountLoan loanAccount(account);
                    foreach (const MyMoneyPayee &payee, m_selectedPayeesList) {
                        if (loanAccount.hasReferenceTo(payee.id())) {
                            usedAccounts.append(account);
                        }
                    }
                }
            }


            MyMoneyPayee newPayee;
            bool addToMatchList = false;
            // if at least one payee is still referenced, we need to reassign its transactions first
            if (!translist.isEmpty() || !used_schedules.isEmpty() || !usedAccounts.isEmpty()) {

                // first create list with all non-selected payees
                QList<MyMoneyPayee> remainingPayees;
                if (type == KPayeeReassignDlg::TypeMerge) {
                    remainingPayees = m_selectedPayeesList;
                } else {
                    remainingPayees = file->payeeList();
                    QList<MyMoneyPayee>::iterator it_p;
                    for (it_p = remainingPayees.begin(); it_p != remainingPayees.end();) {
                        if (m_selectedPayeesList.contains(*it_p)) {
                            it_p = remainingPayees.erase(it_p);
                        } else {
                            ++it_p;
                        }
                    }
                }

                // show error message if no payees remain
                if (remainingPayees.isEmpty()) {
                    KMessageBox::sorry(q, i18n("At least one transaction/scheduled transaction or loan account is still referenced by a payee. "
                                               "Currently you have all payees selected. However, at least one payee must remain so "
                                               "that the transaction/scheduled transaction or loan account can be reassigned."));
                    return false;
                }

                // show transaction reassignment dialog
                KPayeeReassignDlg * dlg = new KPayeeReassignDlg(static_cast<KPayeeReassignDlg::OperationType>(type), q);
                KMyMoneyMVCCombo::setSubstringSearchForChildren(dlg, !KMyMoneySettings::stringMatchFromStart());
                QString payee_id = dlg->show(remainingPayees);
                addToMatchList = dlg->addToMatchList();
                delete dlg; // and kill the dialog
                if (payee_id.isEmpty())
                    return false; // the user aborted the dialog, so let's abort as well

                // try to get selected payee. If not possible and we are merging payees,
                // then we create a new one
                try {
                    newPayee = file->payee(payee_id);
                } catch (const MyMoneyException &) {
                    if (type == KPayeeReassignDlg::TypeMerge) {
                        // it's ok to use payee_id for both arguments since the first is const,
                        // so it's guaranteed not to change its content
                        if (!KMyMoneyUtils::newPayee(payee_id, payee_id))
                            return false; // the user aborted the dialog, so let's abort as well
                        newPayee = file->payee(payee_id);
                    } else {
                        return false;
                    }
                }

                // TODO : check if we have a report that explicitly uses one of our payees
                //        and issue an appropriate warning
                try {
                    // now loop over all transactions and reassign payee
                    for (auto& transaction : translist) {
                        // create a copy of the splits list in the transaction
                        // loop over all splits
                        for (auto& split : transaction.splits()) {
                            // if the split is assigned to one of the selected payees, we need to modify it
                            if (split.isMatched()) {
                                auto tm = split.matchedTransaction();
                                for (auto& sm : tm.splits()) {
                                    if (payeeInList(m_selectedPayeesList, sm.payeeId())) {
                                        sm.setPayeeId(payee_id); // first modify payee in current split
                                        // then modify the split in our local copy of the transaction list
                                        tm.modifySplit(sm); // this does not modify the list object 'splits'!
                                    }
                                }
                                split.addMatch(tm);
                                transaction.modifySplit(split); // this does not modify the list object 'splits'!
                            }
                            if (payeeInList(m_selectedPayeesList, split.payeeId())) {
                                split.setPayeeId(payee_id); // first modify payee in current split
                                // then modify the split in our local copy of the transaction list
                                transaction.modifySplit(split); // this does not modify the list object 'splits'!
                            }
                        } // for - Splits
                        file->modifyTransaction(transaction);  // modify the transaction in the MyMoney object
                    } // for - Transactions

                    // now loop over all schedules and reassign payees
                    for (auto& schedule : used_schedules) {
                        // create copy of transaction in current schedule
                        auto trans = schedule.transaction();
                        // create copy of lists of splits
                        for (auto& split : trans.splits()) {
                            if (payeeInList(m_selectedPayeesList, split.payeeId())) {
                                split.setPayeeId(payee_id);
                                trans.modifySplit(split); // does not modify the list object 'splits'!
                            }
                        } // for - Splits
                        // store transaction in current schedule
                        schedule.setTransaction(trans);
                        file->modifySchedule(schedule);  // modify the schedule in the MyMoney engine
                    } // for - Schedules

                    // reassign the payees in the loans that reference the deleted payees
                    foreach (const MyMoneyAccount &account, usedAccounts) {
                        MyMoneyAccountLoan loanAccount(account);
                        loanAccount.setPayee(payee_id);
                        file->modifyAccount(loanAccount);
                    }

                } catch (const MyMoneyException &e) {
                    KMessageBox::detailedSorry(q, i18n("Unable to reassign payee of transaction/split"), e.what());
                }
            } else { // if !translist.isEmpty()
                if (type == KPayeeReassignDlg::TypeMerge) {
                    KMessageBox::sorry(q, i18n("Nothing to merge."), i18n("Merge Payees"));
                    return false;
                }
            }

            bool ignorecase;
            QStringList payeeNames;
            auto matchType = newPayee.matchData(ignorecase, payeeNames);
            QStringList deletedMatchPattern;

            // now loop over all selected payees and remove them
            for (QList<MyMoneyPayee>::iterator it = m_selectedPayeesList.begin();
                    it != m_selectedPayeesList.end(); ++it) {
                if (newPayee.id() != (*it).id()) {
                    if (addToMatchList) {
                        QStringList matchPattern;
                        auto mType = (*it).matchData(ignorecase, matchPattern);
                        switch (mType) {
                        case eMyMoney::Payee::MatchType::NameExact:
                            matchPattern << QStringLiteral("^%1$").arg((*it).name());
                            break;
                        case eMyMoney::Payee::MatchType::Name:
                            matchPattern << (*it).name();
                            break;
                        default:
                            break;
                        }
                        if (!matchPattern.isEmpty()) {
                            deletedMatchPattern << matchPattern;
                        }
                    }
                    file->removePayee(*it);
                }
            }

            // if we initially have no matching turned on, we just ignore the case (default)
            if (matchType == eMyMoney::Payee::MatchType::Disabled)
                ignorecase = true;

            // update the destination payee if this was requested by the user
            if (addToMatchList && deletedMatchPattern.count() > 0) {
                // add new names to the list
                // TODO: it would be cool to somehow shrink the list to make better use
                //       of regular expressions at this point. For now, we leave this task
                //       to the user himeself.
                QStringList::const_iterator it_n;
                for (it_n = deletedMatchPattern.constBegin(); it_n != deletedMatchPattern.constEnd(); ++it_n) {
                    if (matchType == eMyMoney::Payee::MatchType::Key) {
                        // make sure we really need it and it is not caught by an existing regexp
                        QStringList::const_iterator it_k;
                        for (it_k = payeeNames.constBegin(); it_k != payeeNames.constEnd(); ++it_k) {
                            QRegExp exp(*it_k, ignorecase ? Qt::CaseInsensitive : Qt::CaseSensitive);
                            if (exp.indexIn(*it_n) != -1)
                                break;
                        }
                        if (it_k == payeeNames.constEnd()) {
                            payeeNames << addPatternOrName(*it_n);
                        }
                    } else if (payeeNames.contains(*it_n) == 0)
                        payeeNames << addPatternOrName(*it_n);
                }

                // and update the payee in the engine context
                // make sure to turn on matching for this payee in the right mode
                newPayee.setMatchData(eMyMoney::Payee::MatchType::Key, ignorecase, payeeNames);
                file->modifyPayee(newPayee);
            }
            ft.commit();

            // If we just deleted the payees, they sure don't exist anymore
            m_selectedPayeesList.clear();

        } catch (const MyMoneyException &e) {
            KMessageBox::detailedSorry(q, i18n("Unable to remove payee(s)"), e.what());
        }

        return true;
    }

    /**
      * Check if a list contains a payee with a given id
      *
      * @param list const reference to value list
      * @param id const reference to id
      *
      * @retval true object has been found
      * @retval false object is not in list
      */
    bool payeeInList(const QList<MyMoneyPayee>& list, const QString& id) const
    {
        bool rc = false;
        QList<MyMoneyPayee>::const_iterator it_p = list.begin();
        while (it_p != list.end()) {
            if ((*it_p).id() == id) {
                rc = true;
                break;
            }
            ++it_p;
        }
        return rc;
    }

    /** Checks whether the currently selected payee is "dirty"
     * @return true, if payee is modified (is "dirty"); false otherwise
     */
    bool isDirty() const
    {
        return ui->m_updateButton->isEnabled();
    }

    /** Sets the payee's "dirty" (modified) status
     * @param dirty if true (default), payee will be set to dirty
     */
    void setDirty(bool dirty)
    {
        ui->m_updateButton->setEnabled(dirty);
    }

    KPayeesView      *q_ptr;
    Ui::KPayeesView  *ui;
    MyMoneyPayee      m_payee;
    QString           m_newName;
    MyMoneyContact   *m_contact;
    int               m_payeeRow;
    QList<int>        m_payeeRows;

    /**
      * List of selected payees
      */
    QList<MyMoneyPayee> m_selectedPayeesList;

    /**
      * q member holds a list of all transactions
      */
    QList<QPair<MyMoneyTransaction, MyMoneySplit> > m_transactionList;

    /**
      * q member holds the load state of page
      */
    bool m_needLoad;

    /**
      * Search widget for the list
      */
    KListWidgetSearchLine*  m_searchWidget;

    /**
     * Semaphore to suppress loading during selection
     */
    bool m_inSelection;

    /**
     * q signals whether a payee can be edited
     **/
    bool m_allowEditing;

    /**
      * q holds the filter type
      */
    int m_payeeFilterType;

    AccountNamesFilterProxyModel *m_filterProxyModel;
};

#endif
