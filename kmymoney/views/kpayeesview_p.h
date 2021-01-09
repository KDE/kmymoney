/***************************************************************************
                          kpayeesview_p.h
                          ---------------
    begin                : Thu Jan 24 2002
    copyright            : (C) 2000-2002 by Michael Edwardes <mte@users.sourceforge.net>
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           Andreas Nicolai <Andreas.Nicolai@gmx.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KPAYEESVIEW_P_H
#define KPAYEESVIEW_P_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDesktopServices>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KMessageBox>
#include <KHelpClient>
#include <KSharedConfig>

// ----------------------------------------------------------------------------
// Project Includes

#include <config-kmymoney.h>
#include <ui_kpayeesview.h>
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
#include "accountsmodel.h"
#include "mymoneysecurity.h"
#include "mymoneyschedule.h"
#include "mymoneycontact.h"
#include "mymoneyaccountloan.h"
#include "mymoneysplit.h"
#include "mymoneyprice.h"
#include "mymoneytransaction.h"
#include "icons.h"
#include "transaction.h"
#include "mymoneyenums.h"
#include "modelenums.h"
#include "payeesmodel.h"
#include "menuenums.h"
#include "ledgerpayeefilter.h"
#include "journalmodel.h"
#include "itemrenameproxymodel.h"
#include "specialdatesmodel.h"
#include "specialdatesfilter.h"

using namespace Icons;

enum filterTypeE { eAllPayees = 0, eReferencedPayees = 1, eUnusedPayees = 2 };

class KPayeesViewPrivate : public KMyMoneyViewBasePrivate
{
  Q_DECLARE_PUBLIC(KPayeesView)

public:
  enum struct eTransactionDisplay {
    ShowTransactions,
    ClearTransactionDisplay
  };

  explicit KPayeesViewPrivate(KPayeesView *qq)
    : KMyMoneyViewBasePrivate(qq)
    , ui(new Ui::KPayeesView)
    , m_transactionFilter (nullptr)
    , m_contact(nullptr)
    , m_syncedPayees(0)
    , m_filterProxyModel(nullptr)
  {
  }

  ~KPayeesViewPrivate()
  {
    // remember the splitter settings for startup
    auto grp = KSharedConfig::openConfig()->group("Last Use Settings");
    grp.writeEntry("KPayeesViewSplitterSize", ui->m_splitter->saveState());
    grp.sync();
    delete ui;
  }

  void init()
  {
    Q_Q(KPayeesView);
    ui->setupUi(q);

    ui->m_register->setSingleLineDetailRole(eMyMoney::Model::TransactionCounterAccountRole);

    // setup the model stack
    auto file = MyMoneyFile::instance();
    m_transactionFilter = new LedgerPayeeFilter(ui->m_register, QVector<QAbstractItemModel*> { file->specialDatesModel() });
    auto specialDatesFilter = new SpecialDatesFilter(file->specialDatesModel(), q);
    specialDatesFilter->setSourceModel(m_transactionFilter);
    ui->m_register->setModel(specialDatesFilter);

    // keep track of changing balances
    q->connect(file->journalModel(), &JournalModel::balanceChanged, m_transactionFilter, &LedgerPayeeFilter::recalculateBalancesOnIdle);

    m_filterProxyModel = new AccountNamesFilterProxyModel(q);
    m_filterProxyModel->setHideEquityAccounts(!KMyMoneySettings::expertMode());
    m_filterProxyModel->addAccountGroup(QVector<eMyMoney::Account::Type> {eMyMoney::Account::Type::Asset, eMyMoney::Account::Type::Liability, eMyMoney::Account::Type::Income, eMyMoney::Account::Type::Expense, eMyMoney::Account::Type::Equity});

    m_filterProxyModel->setSourceModel(MyMoneyFile::instance()->accountsModel());
    m_filterProxyModel->sort(AccountsModel::Column::AccountName);


    ui->comboDefaultCategory->setModel(m_filterProxyModel);

    ui->matchTypeCombo->addItem(i18nc("@item No matching", "No matching"), static_cast<int>(eMyMoney::Payee::MatchType::Disabled));
    ui->matchTypeCombo->addItem(i18nc("@item Match Payees name partially", "Match Payees name (partial)"), static_cast<int>(eMyMoney::Payee::MatchType::Name));
    ui->matchTypeCombo->addItem(i18nc("@item Match Payees name exactly", "Match Payees name (exact)"), static_cast<int>(eMyMoney::Payee::MatchType::NameExact));
    ui->matchTypeCombo->addItem(i18nc("@item Search match in list", "Match on a name listed below"), static_cast<int>(eMyMoney::Payee::MatchType::Key));

    //load the filter type
    ui->m_filterBox->addItem(i18nc("@item Show all payees", "All"), ItemRenameProxyModel::eAllItem);
    ui->m_filterBox->addItem(i18nc("@item Show only used payees", "Used"), ItemRenameProxyModel::eReferencedItems);
    ui->m_filterBox->addItem(i18nc("@item Show only unused payees", "Unused"), ItemRenameProxyModel::eUnReferencedItems);
    ui->m_filterBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);

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

    m_renameProxyModel = new ItemRenameProxyModel(q);
    ui->m_payees->setModel(m_renameProxyModel);
    ui->m_payees->setContextMenuPolicy(Qt::CustomContextMenu);

    m_renameProxyModel->setReferenceFilter(ItemRenameProxyModel::eAllItem);
    m_renameProxyModel->setFilterKeyColumn(PayeesModel::Column::Name);
    m_renameProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_renameProxyModel->setRenameColumn(PayeesModel::Column::Name);
    m_renameProxyModel->setSortRole(eMyMoney::Model::PayeeNameRole);
    m_renameProxyModel->setSortLocaleAware(true);
    m_renameProxyModel->sort(0);
    m_renameProxyModel->setDynamicSortFilter(true);

    m_renameProxyModel->setSourceModel(MyMoneyFile::instance()->payeesModel());

    // setup the searchline widget
    q->connect(ui->m_searchWidget, &QLineEdit::textChanged, m_renameProxyModel, &QSortFilterProxyModel::setFilterFixedString);
    ui->m_searchWidget->setClearButtonEnabled(true);
    ui->m_searchWidget->setPlaceholderText(i18nc("Placeholder text", "Search"));

#if 0
    ui->m_balanceLabel->hide();
#endif

    m_contact = new MyMoneyContact(q);
    q->connect(m_contact, &MyMoneyContact::contactFetched, q, &KPayeesView::slotContactFetched);

    q->connect(ui->m_payees->selectionModel(), &QItemSelectionModel::selectionChanged, q, &KPayeesView::slotPayeeSelectionChanged);
    ui->m_payees->setSelectionMode(QListView::ExtendedSelection);
    q->connect(m_renameProxyModel, &ItemRenameProxyModel::renameItem,  q, &KPayeesView::slotRenameSinglePayee);
    q->connect(m_renameProxyModel, &ItemRenameProxyModel::dataChanged, q, &KPayeesView::slotModelDataChanged);

    ui->m_newButton->setDefaultAction(pActions[eMenu::Action::NewPayee]);
    ui->m_renameButton->setDefaultAction(pActions[eMenu::Action::RenamePayee]);
    ui->m_deleteButton->setDefaultAction(pActions[eMenu::Action::DeletePayee]);
    ui->m_mergeButton->setDefaultAction(pActions[eMenu::Action::MergePayee]);

    q->connect(ui->addressEdit,   &QTextEdit::textChanged, q, &KPayeesView::slotPayeeDataChanged);
    q->connect(ui->payeecityEdit,  &QLineEdit::textChanged, q, &KPayeesView::slotPayeeDataChanged);
    q->connect(ui->payeestateEdit,  &QLineEdit::textChanged, q, &KPayeesView::slotPayeeDataChanged);
    q->connect(ui->postcodeEdit,  &QLineEdit::textChanged, q, &KPayeesView::slotPayeeDataChanged);
    q->connect(ui->telephoneEdit, &QLineEdit::textChanged, q, &KPayeesView::slotPayeeDataChanged);
    q->connect(ui->emailEdit,     &QLineEdit::textChanged, q, &KPayeesView::slotPayeeDataChanged);
    q->connect(ui->notesEdit,     &QTextEdit::textChanged, q, &KPayeesView::slotPayeeDataChanged);
    q->connect(ui->payeeIdentifiers, &KPayeeIdentifierView::dataChanged, q, &KPayeesView::slotPayeeDataChanged);
    q->connect(ui->matchTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), q, &KPayeesView::slotPayeeDataChanged);
    q->connect(ui->checkMatchIgnoreCase, &QAbstractButton::toggled, q, &KPayeesView::slotPayeeDataChanged);
    q->connect(ui->checkEnableDefaultCategory,  &QAbstractButton::toggled, q,               &KPayeesView::slotPayeeDataChanged);
    q->connect(ui->comboDefaultCategory,        &KMyMoneyAccountCombo::accountSelected, q,  &KPayeesView::slotPayeeDataChanged);

    q->connect(ui->buttonSuggestACategory,      &QAbstractButton::clicked, q,               &KPayeesView::slotChooseDefaultAccount);

    q->connect(ui->matchKeyEditList, &KEditListWidget::changed, q, &KPayeesView::slotKeyListChanged);

    q->connect(ui->m_updateButton,    &QAbstractButton::clicked, q, &KPayeesView::slotUpdatePayee);
    q->connect(ui->m_syncAddressbook, &QAbstractButton::clicked, q, &KPayeesView::slotSyncAddressBook);
    q->connect(ui->m_helpButton,      &QAbstractButton::clicked, q, &KPayeesView::slotHelp);
    q->connect(ui->m_sendMail,        &QAbstractButton::clicked, q, &KPayeesView::slotSendMail);


    // use the size settings of the last run (if any)
    KConfigGroup grp = KSharedConfig::openConfig()->group("Last Use Settings");
    ui->m_splitter->restoreState(grp.readEntry("KPayeesViewSplitterSize", QByteArray()));
    ui->m_splitter->setChildrenCollapsible(false);

    QVector<int> columns;
    columns = {
      JournalModel::Column::Number,
      JournalModel::Column::Security,
      JournalModel::Column::CostCenter,
      JournalModel::Column::Quantity,
      JournalModel::Column::Price,
      JournalModel::Column::Amount,
      JournalModel::Column::Value,
      JournalModel::Column::Balance,
    };
    ui->m_register->setColumnsHidden(columns);
    columns = {
      JournalModel::Column::Date,
      JournalModel::Column::Account,
      JournalModel::Column::Detail,
      JournalModel::Column::Reconciliation,
      JournalModel::Column::Payment,
      JournalModel::Column::Deposit,
    };
    ui->m_register->setColumnsShown(columns);

    // At start we haven't any payee selected
    ui->m_tabWidget->setEnabled(false); // disable tab widget

    m_payee = MyMoneyPayee(); // make sure we don't access an undefined payee
    clearItemData();
  }

  void ensurePayeeVisible(const QString& id)
  {
    const auto baseIdx = MyMoneyFile::instance()->payeesModel()->indexById(id);
    if (baseIdx.isValid()) {
      const auto idx = MyMoneyFile::baseModel()->mapFromBaseSource(m_renameProxyModel, baseIdx);
      ui->m_payees->setCurrentIndex(idx);
      ui->m_payees->scrollTo(idx);
    }
  }

  void loadDetails()
  {
    ui->addressEdit->setText(m_payee.address());
    ui->postcodeEdit->setText(m_payee.postcode());
    ui->telephoneEdit->setText(m_payee.telephone());
    ui->emailEdit->setText(m_payee.email());
    ui->notesEdit->setText(m_payee.notes());
    ui->payeestateEdit->setText(m_payee.state());
    ui->payeecityEdit->setText(m_payee.city());

    QStringList keys;
    bool ignorecase = false;
    auto type = m_payee.matchData(ignorecase, keys);

    ui->matchTypeCombo->setCurrentIndex(ui->matchTypeCombo->findData(static_cast<int>(type)));
    ui->matchKeyEditList->clear();
    ui->matchKeyEditList->insertStringList(keys);
    ui->checkMatchIgnoreCase->setChecked(ignorecase);

    ui->comboDefaultCategory->setCurrentIndex(-1);
    ui->checkEnableDefaultCategory->setChecked(!m_payee.defaultAccountId().isEmpty());
    ui->comboDefaultCategory->setSelected(m_payee.defaultAccountId());

    ui->payeeIdentifiers->setSource(m_payee);
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
    showTransactions(eTransactionDisplay::ClearTransactionDisplay);
  }

  QStringList selectedPayeeIds() const
  {
    QStringList payees;
    auto baseModel = MyMoneyFile::instance()->payeesModel();
    const QModelIndexList selection = ui->m_payees->selectionModel()->selectedIndexes();
    for (const auto idx : selection) {
      auto baseIdx = baseModel->mapToBaseSource(idx);
      const auto id = baseIdx.data(eMyMoney::Model::IdRole).toString();
      if (!id.isEmpty()) {
        payees.append(id);
      }
    }
    return payees;
  }

  QList<MyMoneyPayee> selectedPayees() const
  {
    QList<MyMoneyPayee> payees;
    auto baseModel = MyMoneyFile::instance()->payeesModel();
    const QModelIndexList selection = ui->m_payees->selectionModel()->selectedIndexes();
    for (const auto idx : selection) {
      auto baseIdx = baseModel->mapToBaseSource(idx);
      auto payee = baseModel->itemByIndex(baseIdx);
      if (!payee.id().isEmpty()) {
        payees.append(payee);
      }
    }
    return payees;
  }

  void showTransactions(eTransactionDisplay clearDisplay = eTransactionDisplay::ShowTransactions)
  {
    MyMoneyMoney balance;
    const auto file = MyMoneyFile::instance();
    MyMoneySecurity base = file->baseCurrency();

    const auto selection = selectedPayees();

    if (selection.isEmpty() || !ui->m_tabWidget->isEnabled()) {
      ui->m_balanceLabel->setText(i18n("Balance: %1", balance.formatMoney(file->baseCurrency().smallestAccountFraction())));
      return;
    }

    QStringList payeeIds;
    if (clearDisplay == eTransactionDisplay::ShowTransactions) {
      for (const auto& payee : selection) {
        payeeIds.append(payee.id());
      }
    }
    m_transactionFilter->setPayeeIdList(payeeIds);

    bool balanceAccurate = true;
    QModelIndex idx;
    MyMoneyMoney deposit, payment;
    const auto rows = m_transactionFilter->rowCount();
    for(int row = 0; row < rows; ++row) {
      idx = m_transactionFilter->index(row, 0);
      const auto accountId = idx.data(eMyMoney::Model::SplitAccountIdRole).toString();
      // in case of an entry of the specialDatesModel the accountId is empty
      if (!accountId.isEmpty()) {
        const auto acc = file->account(accountId);

        const auto shares = idx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>();
        auto val = shares.abs();
        if (acc.currencyId() != base.id()) {
          const MyMoneyPrice &price = file->price(acc.currencyId(), base.id());
          // in case the price is valid, we use it. Otherwise, we keep
          // a flag that tells us that the balance is somewhat inaccurate
          if (price.isValid()) {
            val *= price.rate(base.id());
          } else {
            balanceAccurate = false;
          }
        }

        if (shares.isNegative()) {
          payment += val;
        } else {
          deposit += val;
        }
      }
    }
    balance = deposit - payment;

    ui->m_balanceLabel->setText(i18n("Balance: %1%2",
                                 balanceAccurate ? QString() : QStringLiteral("~"),
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

    const auto file = MyMoneyFile::instance();

    MyMoneyFileTransaction ft;
    try {
      // create a transaction filter that contains all payees selected for removal
      MyMoneyTransactionFilter f = MyMoneyTransactionFilter();
      const auto list = selectedPayees();
      for (const auto& payee : list) {
        f.addPayee(payee.id());
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
          if (payeeInList(list, split.payeeId())) {
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
      for (const MyMoneyAccount &account : allAccounts) {
        if (account.isLoan()) {
          MyMoneyAccountLoan loanAccount(account);
          for (const MyMoneyPayee &payee : list) {
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
          remainingPayees = list;
        } else {
          remainingPayees = file->payeeList();
          QList<MyMoneyPayee>::iterator it_p;
          for (it_p = remainingPayees.begin(); it_p != remainingPayees.end();) {
            if (list.contains(*it_p)) {
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
            // so it's garantee not to change its content
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
                  if (payeeInList(list , sm.payeeId())) {
                    sm.setPayeeId(payee_id); // first modify payee in current split
                    // then modify the split in our local copy of the transaction list
                    tm.modifySplit(sm); // this does not modify the list object 'splits'!
                  }
                }
                split.addMatch(tm);
                transaction.modifySplit(split); // this does not modify the list object 'splits'!
              }
              if (payeeInList(list, split.payeeId())) {
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
              if (payeeInList(list, split.payeeId())) {
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
      QStringList deletedPayeeNames;

      // now loop over all selected payees and remove them
      for (QList<MyMoneyPayee>::const_iterator it = list.constBegin(); it != list.constEnd(); ++it) {
        if (newPayee.id() != (*it).id()) {
          if (addToMatchList) {
            deletedPayeeNames << (*it).name();
          }
          file->removePayee(*it);
        }
      }

      // if we initially have no matching turned on, we just ignore the case (default)
      if (matchType == eMyMoney::Payee::MatchType::Disabled)
        ignorecase = true;

      // update the destination payee if this was requested by the user
      if (addToMatchList && deletedPayeeNames.count() > 0) {
        // add new names to the list
        // TODO: it would be cool to somehow shrink the list to make better use
        //       of regular expressions at this point. For now, we leave this task
        //       to the user himeself.
        QStringList::const_iterator it_n;
        for (it_n = deletedPayeeNames.constBegin(); it_n != deletedPayeeNames.constEnd(); ++it_n) {
          if (matchType == eMyMoney::Payee::MatchType::Key) {
            // make sure we really need it and it is not caught by an existing regexp
            QStringList::const_iterator it_k;
            for (it_k = payeeNames.constBegin(); it_k != payeeNames.constEnd(); ++it_k) {
              QRegExp exp(*it_k, ignorecase ? Qt::CaseInsensitive : Qt::CaseSensitive);
              if (exp.indexIn(*it_n) != -1)
                break;
            }
            if (it_k == payeeNames.constEnd())
              payeeNames << QRegExp::escape(*it_n);
          } else if (payeeNames.contains(*it_n) == 0)
            payeeNames << QRegExp::escape(*it_n);
        }

        // and update the payee in the engine context
        // make sure to turn on matching for this payee in the right mode
        newPayee.setMatchData(eMyMoney::Payee::MatchType::Key, ignorecase, payeeNames);
        file->modifyPayee(newPayee);
      }
      ft.commit();

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
    for (const auto& payee : list) {
      if (payee.id() == id) {
        return true;
      }
    }
    return false;
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

  void selectPayeeAndTransaction(const QString& payeeId, const QString& accountId = QString(), const QString& transactionId = QString())
  {
    Q_Q(KPayeesView);
    if (!q->isVisible())
      return;

    QModelIndex idx;
    const auto list = ui->m_payees->model()->match(ui->m_payees->model()->index(0, 0), eMyMoney::Model::IdRole,
                                         payeeId,
                                         -1,                         // all splits
                                         Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive | Qt::MatchRecursive));

    if (list.isEmpty()) {
      qDebug() << "selectPayeeAndTransaction: Payee not found" << payeeId;
      return;
    }
    ui->m_payees->setCurrentIndex(list.at(0));

    const auto model = ui->m_register->model();
    Q_ASSERT(model != nullptr);

    const auto rows = model->rowCount();
    int transactionRow = -1;
    // scan all transactions shown for this payee
    for (int row = 0; row < rows; ++row) {
      idx = model->index(row, 0);
      if (idx.data(eMyMoney::Model::JournalTransactionIdRole).toString() == transactionId) {
        // if the transaction id matches and we don't have a match already we keep it
        if (transactionRow == -1)
          transactionRow = row;
        // in case the account is available and it matches, we use it right away
        if (!accountId.isEmpty() && (idx.data(eMyMoney::Model::SplitAccountIdRole).toString() == accountId)) {
          transactionRow = row;
          break;
        }
      } else if (transactionRow != -1) {
        // if we have a match for a transaction, we use it
        break;
      }
    }

    if (transactionRow != -1) {
      ui->m_register->setCurrentIndex(model->index(transactionRow, 0));
    }

    ensurePayeeVisible(payeeId);
  }

  void finalizePendingChanges()
  {
    Q_Q(KPayeesView);
    if (isDirty()) {
      if (KMessageBox::questionYesNo(q,
        i18n("<qt>Do you want to save the changes for <b>%1</b>?</qt>", m_newName),
                                     i18n("Save changes")) == KMessageBox::Yes) {
        q->slotUpdatePayee();
      }
    }
  }

  Ui::KPayeesView*    ui;
  LedgerPayeeFilter*  m_transactionFilter;

  MyMoneyPayee        m_payee;
  QString             m_newName;
  MyMoneyContact*     m_contact;
  int                 m_syncedPayees;
  QList<MyMoneyPayee> m_payeesToSync;

  AccountNamesFilterProxyModel *m_filterProxyModel;
  ItemRenameProxyModel*         m_renameProxyModel;
};

#endif
