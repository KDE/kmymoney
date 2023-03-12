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

#include "kpayeesview.h"
#include "kpayeesview_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QAction>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

using namespace Icons;

// *** KPayeesView Implementation ***

KPayeesView::KPayeesView(QWidget *parent) :
    KMyMoneyViewBase(*new KPayeesViewPrivate(this), parent)
{
    Q_D(KPayeesView);
    d->init();

    connect(d->ui->m_payees, &QWidget::customContextMenuRequested, this, [&](QPoint pos) {
        Q_D(KPayeesView);
        updateActions(d->m_selections);
        Q_EMIT requestCustomContextMenu(eMenu::Menu::Payee, d->ui->m_payees->mapToGlobal(pos));
    });

    connect(d->ui->m_register, &QWidget::customContextMenuRequested, this, [&](QPoint pos) {
        Q_D(KPayeesView);
        // it does not make sense to jump to myself
        pActions[eMenu::Action::GoToPayee]->setDisabled(true);
        Q_EMIT requestCustomContextMenu(eMenu::Menu::Transaction, d->ui->m_register->mapToGlobal(pos));
    });

    connect(d->ui->m_filterBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](int idx) {
        Q_D(KPayeesView);
        d->m_renameProxyModel->setReferenceFilter(d->ui->m_filterBox->itemData(idx));
    });

    connect(d->ui->m_register, &LedgerView::transactionSelectionChanged, this, [&](const SelectedObjects& selections) {
        Q_D(KPayeesView);
        d->m_selections.setSelection(SelectedObjects::JournalEntry, selections.selection(SelectedObjects::JournalEntry));
        if (selections.selection(SelectedObjects::JournalEntry).count() == 1) {
            const auto file = MyMoneyFile::instance();
            const auto idx = file->journalModel()->indexById(selections.firstSelection(SelectedObjects::JournalEntry));
            d->m_selections.setSelection(SelectedObjects::Account, idx.data(eMyMoney::Model::SplitAccountIdRole).toString());
        }
        Q_EMIT requestSelectionChange(d->m_selections);
    });

    connect(d->ui->m_register, &LedgerView::requestView, this, &KPayeesView::requestView);

    connect(LedgerViewSettings::instance(), &LedgerViewSettings::settingsChanged, this, [&]() {
        Q_D(KPayeesView);
        d->m_transactionFilter->setHideReconciledTransactions(LedgerViewSettings::instance()->hideReconciledTransactions());
        d->m_transactionFilter->setHideTransactionsBefore(LedgerViewSettings::instance()->hideTransactionsBefore());
    });

    connect(pActions[eMenu::Action::NewPayee], &QAction::triggered, this, &KPayeesView::slotNewPayee);
    connect(pActions[eMenu::Action::DeletePayee], &QAction::triggered, this, &KPayeesView::slotDeletePayee);
    connect(pActions[eMenu::Action::RenamePayee], &QAction::triggered, this, &KPayeesView::slotRenamePayee);
    connect(pActions[eMenu::Action::MergePayee], &QAction::triggered, this, &KPayeesView::slotMergePayee);

    d->m_sharedToolbarActions.insert(eMenu::Action::FileNew, pActions[eMenu::Action::NewPayee]);
}

KPayeesView::~KPayeesView()
{
}

void KPayeesView::slotChooseDefaultAccount()
{
    Q_D(KPayeesView);
    MyMoneyFile* file = MyMoneyFile::instance();
    QMap<QString, int> account_count;


    QModelIndex idx;
    const auto rows = d->m_transactionFilter->rowCount();
    for(int row = 0; row < rows; ++row) {
        idx = d->m_transactionFilter->index(row, 0);
        if (!idx.data(eMyMoney::Model::TransactionIsTransferRole).toBool()
                && idx.data(eMyMoney::Model::TransactionSplitCountRole).toInt() == 2) {
            const auto accountId = idx.data(eMyMoney::Model::SplitAccountIdRole).toString();
            const auto action = idx.data(eMyMoney::Model::SplitActionRole).toString();
            const MyMoneyAccount& acc = file->account(accountId);
            if (action != MyMoneySplit::actionName(eMyMoney::Split::Action::Amortization)
                    && acc.accountType() != eMyMoney::Account::Type::AssetLoan) {
                const auto counterAccount = idx.data(eMyMoney::Model::TransactionCounterAccountIdRole).toString();
                if (!counterAccount.isEmpty()) {
                    if (account_count.contains(counterAccount)) {
                        account_count[counterAccount]++;
                    } else {
                        account_count[counterAccount] = 1;
                    }
                }
            }
        }
    }

    QMap<QString, int>::Iterator most_frequent, iter;
    most_frequent = account_count.begin();
    for (iter = account_count.begin(); iter != account_count.end(); ++iter) {
        if (iter.value() > most_frequent.value()) {
            most_frequent = iter;
        }
    }

    if (most_frequent != account_count.end()) {
        d->ui->checkEnableDefaultCategory->setChecked(true);
        d->ui->comboDefaultCategory->setSelected(most_frequent.key());
        d->setDirty(true);
    }
}

void KPayeesView::slotRenameSinglePayee(const QModelIndex& idx, const QVariant& value)
{
    Q_D(KPayeesView);
    //if there is no current item selected, exit
    if (!idx.isValid())
        return;

    //qDebug() << "[KPayeesView::slotRenamePayee]";
    // create a copy of the new name without appended whitespaces
    QString new_name = value.toString();
    if (d->m_payee.name() != new_name) {
        MyMoneyFileTransaction ft;
        try {
            // check if we already have a payee with the new name
            if (!MyMoneyFile::instance()->payeeByName(new_name).id().isEmpty()) {
                // the name already exists, ask the user whether he's sure to keep the name
                if (KMessageBox::questionTwoActions(this,
                                                    i18n("A payee with the name '%1' already exists. It is not advisable to have "
                                                         "multiple payees with the same identification name. Are you sure you would like "
                                                         "to rename the payee?",
                                                         new_name),
                                                    i18nc("@title:window", "Duplicate payee name"),
                                                    KMMYesNo::yes(),
                                                    KMMYesNo::no())
                    != KMessageBox::PrimaryAction) {
                    // p->setText(d->m_payee.name());
                    return;
                }
            }

            d->m_payee.setName(new_name);
            d->m_newName = new_name;
            MyMoneyFile::instance()->modifyPayee(d->m_payee);

            // the above call to modifyPayee will reload the view so
            // all references and pointers to the view have to be
            // re-established.

            ft.commit();

            // make sure, that the record is visible even if it moved
            // out of sight due to the rename operation
            d->ensurePayeeVisible(d->m_payee.id());

        } catch (const MyMoneyException &e) {
            KMessageBox::detailedError(this, i18n("Unable to modify payee"), QString::fromLatin1(e.what()));
        }
    }
}

void KPayeesView::updateActions(const SelectedObjects& selections)
{
    Q_D(KPayeesView);
    const auto selectedItemCount = selections.selection(SelectedObjects::Payee).count();

    d->ui->m_syncAddressbook->setEnabled(true);
    d->ui->m_tabWidget->setEnabled(false); // disable tab widget

    pActions[eMenu::Action::RenamePayee]->setEnabled(false);
    pActions[eMenu::Action::DeletePayee]->setEnabled(false);
    pActions[eMenu::Action::MergePayee]->setEnabled(false);

    switch (selectedItemCount) {
    case 0: // no selection
        d->ui->m_balanceLabel->hide();
        if (isVisible()) {
            d->clearItemData();
        }
        d->ui->m_syncAddressbook->setEnabled(false);
        break;

    case 1: // single selection
        pActions[eMenu::Action::RenamePayee]->setEnabled(true);
        pActions[eMenu::Action::DeletePayee]->setEnabled(true);
        d->ui->m_tabWidget->setEnabled(true); // enable tab widget
        break;

    default:  // if we have multiple payees selected, clear and disable the payee information
        pActions[eMenu::Action::DeletePayee]->setEnabled(true);
        pActions[eMenu::Action::MergePayee]->setEnabled(true);
        d->ui->m_balanceLabel->hide();
        if (isVisible()) {
            d->clearItemData();
        }
        break;
    }
}

void KPayeesView::aboutToShow()
{
    Q_D(KPayeesView);
    d->loadDetails();
    d->showTransactions();

    // don't forget base class logic
    KMyMoneyViewBase::aboutToShow();
}

void KPayeesView::aboutToHide()
{
    Q_D(KPayeesView);

    d->finalizePendingChanges();

    KMyMoneyViewBase::aboutToHide();
}

void KPayeesView::slotPayeeSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_UNUSED(selected)
    Q_UNUSED(deselected)
    Q_D(KPayeesView);

    d->finalizePendingChanges();
    d->selectPayee();
}

void KPayeesView::slotKeyListChanged()
{
    Q_D(KPayeesView);
    bool rc = false;
    bool ignorecase = false;
    QStringList keys;

    d->m_payee.matchData(ignorecase, keys);
    if (static_cast<eMyMoney::Payee::MatchType>(d->ui->matchTypeCombo->currentData().toUInt()) == eMyMoney::Payee::MatchType::Key) {
        rc |= (keys != d->ui->matchKeyEditList->items());
    }
    d->setDirty(rc);
}

void KPayeesView::slotModelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    Q_D(KPayeesView);
    QModelIndex idx;
    if (topLeft.model() == d->m_renameProxyModel) {
        const auto baseModel = MyMoneyFile::instance()->payeesModel();
        for (int row = topLeft.row(); row <= bottomRight.row(); ++row) {
            idx = topLeft.model()->index(row, 0, topLeft.parent());
            if (d->m_payee.id() == idx.data(eMyMoney::Model::IdRole).toString()) {
                d->m_payee = baseModel->itemById(d->m_payee.id());
                d->loadDetails();
            }
        }
    }
}


void KPayeesView::slotPayeeDataChanged()
{
    Q_D(KPayeesView);
    bool rc = false;

    if (d->ui->m_tabWidget->isEnabled()) {
        rc |= ((d->m_payee.email().isEmpty() != d->ui->emailEdit->text().isEmpty())
               || (!d->ui->emailEdit->text().isEmpty() && d->m_payee.email() != d->ui->emailEdit->text()));
        rc |= ((d->m_payee.address().isEmpty() != d->ui->addressEdit->toPlainText().isEmpty())
               || (!d->ui->addressEdit->toPlainText().isEmpty() && d->m_payee.address() != d->ui->addressEdit->toPlainText()));
        rc |= ((d->m_payee.city().isEmpty() != d->ui->payeecityEdit->text().isEmpty())
               || (!d->ui->payeecityEdit->text().isEmpty() && d->m_payee.city() != d->ui->payeecityEdit->text()));
        rc |= ((d->m_payee.state().isEmpty() != d->ui->payeestateEdit->text().isEmpty())
               || (!d->ui->payeestateEdit->text().isEmpty() && d->m_payee.state() != d->ui->payeestateEdit->text()));
        rc |= ((d->m_payee.postcode().isEmpty() != d->ui->postcodeEdit->text().isEmpty())
               || (!d->ui->postcodeEdit->text().isEmpty() && d->m_payee.postcode() != d->ui->postcodeEdit->text()));
        rc |= ((d->m_payee.telephone().isEmpty() != d->ui->telephoneEdit->text().isEmpty())
               || (!d->ui->telephoneEdit->text().isEmpty() && d->m_payee.telephone() != d->ui->telephoneEdit->text()));
        rc |= ((d->m_payee.name().isEmpty() != d->m_newName.isEmpty())
               || (!d->m_newName.isEmpty() && d->m_payee.name() != d->m_newName));
        rc |= ((d->m_payee.notes().isEmpty() != d->ui->notesEdit->toPlainText().isEmpty())
               || (!d->ui->notesEdit->toPlainText().isEmpty() && d->m_payee.notes() != d->ui->notesEdit->toPlainText()));

        bool ignorecase = false;
        QStringList keys;

        const auto selectedMatchType = static_cast<eMyMoney::Payee::MatchType>(d->ui->matchTypeCombo->currentData().toUInt());
        const auto storedMatchType = d->m_payee.matchData(ignorecase, keys);
        rc |= (storedMatchType != selectedMatchType);

        d->ui->checkMatchIgnoreCase->setEnabled(false);
        d->ui->matchKeyEditList->setEnabled(false);

        if (selectedMatchType != eMyMoney::Payee::MatchType::Disabled) {
            d->ui->checkMatchIgnoreCase->setEnabled(true);
            // if we turn matching on, we default to 'ignore case'
            // TODO maybe make the default a user option
            if (storedMatchType == eMyMoney::Payee::MatchType::Disabled && (selectedMatchType != eMyMoney::Payee::MatchType::Disabled))
                d->ui->checkMatchIgnoreCase->setChecked(true);
            rc |= (ignorecase != d->ui->checkMatchIgnoreCase->isChecked());
            if (selectedMatchType == eMyMoney::Payee::MatchType::Key) {
                d->ui->matchKeyEditList->setEnabled(true);
                rc |= (keys != d->ui->matchKeyEditList->items());
            }
        }

        rc |= (d->ui->checkEnableDefaultCategory->isChecked() != !d->m_payee.defaultAccountId().isEmpty());
        if (d->ui->checkEnableDefaultCategory->isChecked()) {
            d->ui->comboDefaultCategory->setEnabled(true);
            d->ui->labelDefaultCategory->setEnabled(true);
            // this is only going to understand the first in the list of selected accounts
            if (d->ui->comboDefaultCategory->getSelected().isEmpty()) {
                rc |= !d->m_payee.defaultAccountId().isEmpty();
            } else {
                QString temp = d->ui->comboDefaultCategory->getSelected();
                rc |= (temp.isEmpty() != d->m_payee.defaultAccountId().isEmpty())
                      || (!d->m_payee.defaultAccountId().isEmpty() && temp != d->m_payee.defaultAccountId());
            }
        } else {
            d->ui->comboDefaultCategory->setEnabled(false);
            d->ui->labelDefaultCategory->setEnabled(false);
        }

        rc |= (d->m_payee.payeeIdentifiers() != d->ui->payeeIdentifiers->identifiers());
    }
    d->setDirty(rc);
}

void KPayeesView::slotUpdatePayee()
{
    Q_D(KPayeesView);
    if (d->isDirty()) {
        MyMoneyFileTransaction ft;
        d->setDirty(false);
        try {
            d->m_payee.setName(d->m_newName);
            d->m_payee.setAddress(d->ui->addressEdit->toPlainText());
            d->m_payee.setCity(d->ui->payeecityEdit->text());
            d->m_payee.setState(d->ui->payeestateEdit->text());
            d->m_payee.setPostcode(d->ui->postcodeEdit->text());
            d->m_payee.setTelephone(d->ui->telephoneEdit->text());
            d->m_payee.setEmail(d->ui->emailEdit->text());
            d->m_payee.setNotes(d->ui->notesEdit->toPlainText());
            d->m_payee.setMatchData(static_cast<eMyMoney::Payee::MatchType>(d->ui->matchTypeCombo->currentData().toUInt()), d->ui->checkMatchIgnoreCase->isChecked(), d->ui->matchKeyEditList->items());
            d->m_payee.setDefaultAccountId();
            d->m_payee.resetPayeeIdentifiers(d->ui->payeeIdentifiers->identifiers());

            if (d->ui->checkEnableDefaultCategory->isChecked()) {
                QString temp;
                if (!d->ui->comboDefaultCategory->getSelected().isEmpty()) {
                    temp = d->ui->comboDefaultCategory->getSelected();
                    d->m_payee.setDefaultAccountId(temp);
                }
            }

            MyMoneyFile::instance()->modifyPayee(d->m_payee);
            ft.commit();

        } catch (const MyMoneyException &e) {
            KMessageBox::detailedError(this, i18n("Unable to modify payee"), QString::fromLatin1(e.what()));
        }
    }
}

void KPayeesView::slotSyncAddressBook()
{
    Q_D(KPayeesView);
    if (d->m_payeesToSync.isEmpty()) {                            // empty list means no syncing is pending...
        d->m_payeesToSync = d->selectedPayees();                    // ...so initialize one
        d->m_syncedPayees = 0;
        d->ui->m_syncAddressbook->setEnabled(false);                // disallow concurrent syncs
    }

    if (d->m_payeesToSync.count() <= d->m_syncedPayees) {
        if (!d->m_payeesToSync.isEmpty()) {
            MyMoneyFileTransaction ft;
            try {
                for (const auto& payee : d->m_payeesToSync) {
                    MyMoneyFile::instance()->modifyPayee(payee);
                }
                ft.commit();
            } catch (const MyMoneyException &e) {
                KMessageBox::detailedError(this, i18n("Unable to modify payee"), QString::fromLatin1(e.what()));
            }
        }
        d->m_payeesToSync.clear();  // that means end of sync
        d->m_syncedPayees = 0;
        d->ui->m_syncAddressbook->setEnabled(true);
        return;
    }

    // search for payee's data in addressbook and receive it in slotContactFetched
    d->m_contact->fetchContact(d->m_payeesToSync[d->m_syncedPayees].email());
}

void KPayeesView::slotContactFetched(const ContactData &identity)
{
    Q_D(KPayeesView);
    MyMoneyPayee& payee = d->m_payeesToSync[d->m_syncedPayees];

    if (!identity.email.isEmpty()) {  // empty e-mail means no identity fetched
        QString txt;
        if (!identity.street.isEmpty())
            txt.append(identity.street + '\n');
        if (!identity.locality.isEmpty()) {
            txt.append(identity.locality);
            if (!identity.postalCode.isEmpty())
                txt.append(' ' + identity.postalCode + '\n');
            else
                txt.append('\n');
        }
        if (!identity.country.isEmpty())
            txt.append(identity.country + '\n');

        if (!txt.isEmpty() && d->m_payee.address().compare(txt) != 0)
            d->m_payee.setAddress(txt);

        if (!identity.city.isEmpty() && d->m_payee.city().compare(identity.city) != 0)
            d->m_payee.setCity(identity.city);

        if (!identity.state.isEmpty() && d->m_payee.state().compare(identity.state) != 0)
            d->m_payee.setState(identity.state);

        if (!identity.postalCode.isEmpty() && d->m_payee.postcode().compare(identity.postalCode) != 0)
            d->m_payee.setPostcode(identity.postalCode);

        if (!identity.postalCode.isEmpty() && payee.postcode().compare(identity.postalCode) != 0)
            payee.setPostcode(identity.postalCode);

        if (!identity.phoneNumber.isEmpty() && payee.telephone().compare(identity.phoneNumber) != 0)
            payee.setTelephone(identity.phoneNumber);
    }

    ++d->m_syncedPayees;
    slotSyncAddressBook();  // process next payee
}

void KPayeesView::slotSendMail()
{
    Q_D(KPayeesView);
    QRegularExpression re(".+@.+");
    if (re.match(d->m_payee.email()).hasMatch())
        QDesktopServices::openUrl(QUrl(QStringLiteral("mailto:?to=") + d->m_payee.email(), QUrl::TolerantMode));
}

void KPayeesView::executeAction(eMenu::Action action, const SelectedObjects& selections)
{
    const auto payeeId(selections.firstSelection(SelectedObjects::Payee));
    const auto accountId(selections.firstSelection(SelectedObjects::Account));
    const auto journalEntryId(selections.firstSelection(SelectedObjects::JournalEntry));

    Q_D(KPayeesView);
    const auto idx = d->ui->m_register->currentIndex();
    switch (action) {
    case eMenu::Action::GoToPayee:
        if (d->isActiveView()) {
            d->selectPayeeAndTransaction(payeeId, accountId, journalEntryId);
        }
        break;
    case eMenu::Action::EditTransaction:
        if (d->isActiveView()) {
            d->ui->m_register->edit(d->ui->m_register->currentIndex());
        }
        break;
    case eMenu::Action::EditSplits:
        if (d->isActiveView()) {
            d->ui->m_register->edit(idx);
            const auto editor = d->ui->m_register->indexWidget(d->ui->m_register->editIndex());
            if (editor) {
                QMetaObject::invokeMethod(editor, "editSplits", Qt::QueuedConnection);
            }
        }
        break;

    case eMenu::Action::SelectAllTransactions:
        if (d->isActiveView()) {
            d->ui->m_register->selectAllTransactions();
        }
        break;

    case eMenu::Action::FileClose:
        d->ui->payeeIdentifiers->closeSource();
        break;

    case eMenu::Action::ShowTransaction:
        d->ui->m_register->showEditor();
        break;

    default:
        break;
    }
}

void KPayeesView::slotSettingsChanged()
{
    Q_D(KPayeesView);
    if (d->ui->m_register) {
        d->ui->m_register->slotSettingsChanged();
    }
}

void KPayeesView::slotHelp()
{
    KHelpClient::invokeHelp("details.payees");
}

void KPayeesView::slotNewPayee()
{
    Q_D(KPayeesView);
    QString id;
    KMyMoneyUtils::newPayee(i18n("New Payee"), id);
    d->selectPayeeAndTransaction(id);
}

void KPayeesView::slotRenamePayee()
{
    Q_D(KPayeesView);
    if (d->ui->m_payees->currentIndex().isValid() && d->ui->m_payees->selectionModel()->selectedIndexes().count() == 1) {
        d->ui->m_payees->edit(d->ui->m_payees->currentIndex());
    }
}

void KPayeesView::slotDeletePayee()
{
    Q_D(KPayeesView);
    const auto payeesList = d->selectedPayees();

    if (payeesList.isEmpty())
        return; // shouldn't happen

    // get confirmation from user
    QString prompt;
    if (payeesList.count() == 1)
        prompt = i18n("<p>Do you really want to remove the payee <b>%1</b>?</p>", payeesList.front().name());
    else
        prompt = i18n("Do you really want to remove all selected payees?");

    if (KMessageBox::questionTwoActions(this, prompt, i18n("Remove Payee"), KMMYesNo::yes(), KMMYesNo::no()) == KMessageBox::SecondaryAction)
        return;

    d->payeeReassign(KPayeeReassignDlg::TypeDelete);
}

void KPayeesView::slotMergePayee()
{
    Q_D(KPayeesView);
    const auto payeesList = d->selectedPayees();
    if (payeesList.count() < 1)
        return; // shouldn't happen

    if (KMessageBox::questionTwoActions(this,
                                        i18n("<p>Do you really want to merge the selected payees?"),
                                        i18n("Merge Payees"),
                                        KMMYesNo::yes(),
                                        KMMYesNo::no())
        == KMessageBox::SecondaryAction)
        return;

    if (d->payeeReassign(KPayeeReassignDlg::TypeMerge))
        /// @todo maybe select the one that is remaining and call
        ///       d->ensurePayeeVisible
        // clean selection since we just deleted the selected payees
        d->ui->m_payees->selectionModel()->clear();
}
