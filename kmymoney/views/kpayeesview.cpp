/***************************************************************************
                          kpayeesview.cpp
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

#include "kpayeesview_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QMap>
#include <QList>
#include <QMenu>
#include <QDesktopServices>
#include <QIcon>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KMessageBox>
#include <KGuiItem>
#include <KHelpClient>
#include <KSharedConfig>
#include <KListWidgetSearchLine>

// ----------------------------------------------------------------------------
// Project Includes

#include <config-kmymoney.h>
#include "ui_kpayeesview.h"
#include "kmymoneyviewbase_p.h"
#include "kpayeeidentifierview.h"
#include "mymoneypayee.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneymoney.h"
#include "mymoneytransactionfilter.h"
#include "kmymoneysettings.h"
#include "accountsmodel.h"
#include "mymoneysecurity.h"
#include "mymoneycontact.h"
#include "mymoneyprice.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "icons/icons.h"
#include "transaction.h"
#include "widgetenums.h"
#include "mymoneyenums.h"
#include "modelenums.h"
#include "menuenums.h"

using namespace Icons;

// *** KPayeesView Implementation ***

KPayeesView::KPayeesView(QWidget *parent) :
    KMyMoneyViewBase(*new KPayeesViewPrivate(this), parent)
{
  Q_D(KPayeesView);
  d->init();

  connect(pActions[eMenu::Action::NewPayee],    &QAction::triggered, this, &KPayeesView::slotNewPayee);
  connect(pActions[eMenu::Action::RenamePayee], &QAction::triggered, this, &KPayeesView::slotRenamePayee);
  connect(pActions[eMenu::Action::DeletePayee], &QAction::triggered, this, &KPayeesView::slotDeletePayee);
  connect(pActions[eMenu::Action::MergePayee],  &QAction::triggered, this, &KPayeesView::slotMergePayee);

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

void KPayeesView::slotClosePayeeIdentifierSource()
{
  Q_D(KPayeesView);
  if (!d->m_needLoad)
    d->ui->payeeIdentifiers->closeSource();
}

void KPayeesView::slotSelectByVariant(const QVariantList& variant, eView::Intent intent)
{
  switch (intent) {
    case eView::Intent::ShowPayee:
      if (variant.count() == 3)
        slotSelectPayeeAndTransaction(variant.at(0).toString(), variant.at(1).toString(), variant.at(2).toString());
      break;
    default:
      break;
  }
}

void KPayeesView::slotRenameSinglePayee(const QModelIndex& idx, const QVariant& value)
{
  Q_D(KPayeesView);
  //if there is no current item selected, exit
  if (d->m_allowEditing == false || !idx.isValid())
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
        if (KMessageBox::questionYesNo(this,
                                      i18n("A payee with the name '%1' already exists. It is not advisable to have "
                                            "multiple payees with the same identification name. Are you sure you would like "
                                            "to rename the payee?", new_name)) != KMessageBox::Yes) {
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
      KMessageBox::detailedSorry(this, i18n("Unable to modify payee"), QString::fromLatin1(e.what()));
    }
  }
}

void KPayeesView::slotPayeeSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  Q_D(KPayeesView);

  if (d->isDirty()) {
    if (KMessageBox::questionYesNo(this,
      i18n("<qt>Do you want to save the changes for <b>%1</b>?</qt>", d->m_newName),
                                   i18n("Save changes")) == KMessageBox::Yes) {
      d->m_inSelection = true;
      slotUpdatePayee();
      d->m_inSelection = false;
    }
  }

  const auto selectedItemCount = d->ui->m_payees->selectionModel()->selectedIndexes().count();
  updatePayeeActions(selectedItemCount);

  d->ui->m_deleteButton->setEnabled(true); //re-enable delete button
  d->ui->m_syncAddressbook->setEnabled(true);

  switch (selectedItemCount) {
    case 0: // no selection
      d->ui->m_tabWidget->setEnabled(false); // disable tab widget
      d->ui->m_balanceLabel->hide();
      d->ui->m_deleteButton->setEnabled(false); //disable delete, rename and merge buttons
      d->ui->m_renameButton->setEnabled(false);
      d->ui->m_mergeButton->setEnabled(false);
      d->clearItemData();
      d->m_payee = MyMoneyPayee();
      d->ui->m_syncAddressbook->setEnabled(false);
      return; // make sure we don't access an undefined payee

    case 1: // single selection
      d->ui->m_mergeButton->setEnabled(false);
      d->ui->m_renameButton->setEnabled(true);
      break;

    default:  // if we have multiple payees selected, clear and disable the payee information
      d->ui->m_tabWidget->setEnabled(false); // disable tab widget
      d->ui->m_renameButton->setEnabled(false); // disable also the rename button
      d->ui->m_mergeButton->setEnabled(true);
      d->ui->m_balanceLabel->hide();
      d->clearItemData();
      break;
  }
  d->ui->m_tabWidget->setEnabled(true); // enable tab widget

  const auto selectedPayees = d->selectedPayees();
  d->m_payee = MyMoneyPayee();
  if (!selectedPayees.isEmpty()) {
    d->m_payee = selectedPayees.at(0);
  }
  emit selectObjects(selectedPayees);

  // as of now we are updating only the last selected payee, and until
  // selection mode of the QListView has been changed to Extended, this
  // will also be the only selection and behave exactly as before - Andreas
  try {
    d->m_newName = d->m_payee.name();

    d->ui->addressEdit->setEnabled(true);
    d->ui->addressEdit->setText(d->m_payee.address());
    d->ui->postcodeEdit->setEnabled(true);
    d->ui->postcodeEdit->setText(d->m_payee.postcode());
    d->ui->telephoneEdit->setEnabled(true);
    d->ui->telephoneEdit->setText(d->m_payee.telephone());
    d->ui->emailEdit->setEnabled(true);
    d->ui->emailEdit->setText(d->m_payee.email());
    d->ui->notesEdit->setText(d->m_payee.notes());

    QStringList keys;
    bool ignorecase = false;
    auto type = d->m_payee.matchData(ignorecase, keys);

    d->ui->matchTypeCombo->setCurrentIndex(d->ui->matchTypeCombo->findData(static_cast<int>(type)));
    d->ui->matchKeyEditList->clear();
    d->ui->matchKeyEditList->insertStringList(keys);
    d->ui->checkMatchIgnoreCase->setChecked(ignorecase);

    d->ui->checkEnableDefaultCategory->setChecked(!d->m_payee.defaultAccountId().isEmpty());
    d->ui->comboDefaultCategory->setSelected(d->m_payee.defaultAccountId());

    d->ui->payeeIdentifiers->setSource(d->m_payee);

    slotPayeeDataChanged();
    d->showTransactions();

  } catch (const MyMoneyException &e) {
    qDebug("exception during display of payee: %s", e.what());
    /// @todo port to new model code
    // d->ui->m_register->clear();
    d->m_payee = MyMoneyPayee();
  }
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

    auto type = d->m_payee.matchData(ignorecase, keys);
    rc |= (static_cast<unsigned int>(type) != d->ui->matchTypeCombo->currentData().toUInt());

    d->ui->checkMatchIgnoreCase->setEnabled(false);
    d->ui->matchKeyEditList->setEnabled(false);

    if (static_cast<eMyMoney::Payee::MatchType>(d->ui->matchTypeCombo->currentData().toUInt()) != eMyMoney::Payee::MatchType::Disabled) {
      d->ui->checkMatchIgnoreCase->setEnabled(true);
      // if we turn matching on, we default to 'ignore case'
      // TODO maybe make the default a user option
      if (type == eMyMoney::Payee::MatchType::Disabled && static_cast<eMyMoney::Payee::MatchType>(d->ui->matchTypeCombo->currentData().toUInt()) != eMyMoney::Payee::MatchType::Disabled)
        d->ui->checkMatchIgnoreCase->setChecked(true);
      rc |= (ignorecase != d->ui->checkMatchIgnoreCase->isChecked());
      if (static_cast<eMyMoney::Payee::MatchType>(d->ui->matchTypeCombo->currentData().toUInt()) == eMyMoney::Payee::MatchType::Key) {
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
      KMessageBox::detailedSorry(this, i18n("Unable to modify payee"), QString::fromLatin1(e.what()));
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
#if 0
    if (auto item = dynamic_cast<KPayeeListItem*>(d->ui->m_payeesList->currentItem())) { // update ui if something is selected
      d->m_payee = item->payee();
      d->ui->addressEdit->setText(d->m_payee.address());
      d->ui->payeecityEdit->setText(d->m_payee.city());
      d->ui->payeestateEdit->setText(d->m_payee.state());
      d->ui->postcodeEdit->setText(d->m_payee.postcode());
      d->ui->telephoneEdit->setText(d->m_payee.telephone());
    }
    // update synced data in engine
#endif

    if (!d->m_payeesToSync.isEmpty()) {
      MyMoneyFileTransaction ft;
      try {
        for (const auto& payee : d->m_payeesToSync) {
          MyMoneyFile::instance()->modifyPayee(payee);
        }
        ft.commit();
      } catch (const MyMoneyException &e) {
        KMessageBox::detailedSorry(this, i18n("Unable to modify payee"), QString::fromLatin1(e.what()));
      }
    }
    d->m_payeesToSync.clear();  // that means end of sync
    d->m_syncedPayees = 0;
    d->ui->m_syncAddressbook->setEnabled(true);
    return;
  }

  // search for payee's data in addressbook and receive it in slotContactFetched
  d->m_contact->fetchContact(d->m_payee.email());
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

void KPayeesView::executeCustomAction(eView::Action action)
{
  switch(action) {
    case eView::Action::SetDefaultFocus:
      {
        Q_D(KPayeesView);
        QMetaObject::invokeMethod(d->ui->m_searchWidget, "setFocus");
      }
      break;

    case eView::Action::ClosePayeeIdentifierSource:
      slotClosePayeeIdentifierSource();
      break;

    default:
      break;
  }
}

void KPayeesView::showEvent(QShowEvent* event)
{
  Q_D(KPayeesView);

  emit customActionRequested(View::Payees, eView::Action::AboutToShow);

  // don't forget base class implementation
  QWidget::showEvent(event);
}

/// @todo cleanup
void KPayeesView::updatePayeeActions(const QList<MyMoneyPayee> &payees)
{
  pActions[eMenu::Action::NewPayee]->setEnabled(true);
  const auto payeesCount = payees.count();
  auto b = payeesCount == 1 ? true : false;
  pActions[eMenu::Action::RenamePayee]->setEnabled(b);
  b = payeesCount > 1 ? true : false;
  pActions[eMenu::Action::MergePayee]->setEnabled(b);
  b = payeesCount == 0 ? false : true;
  pActions[eMenu::Action::DeletePayee]->setEnabled(b);
}

void KPayeesView::updatePayeeActions(int payeesCount)
{
  pActions[eMenu::Action::NewPayee]->setEnabled(true);
  auto b = payeesCount == 1 ? true : false;
  pActions[eMenu::Action::RenamePayee]->setEnabled(b);
  b = payeesCount > 1 ? true : false;
  pActions[eMenu::Action::MergePayee]->setEnabled(b);
  b = payeesCount == 0 ? false : true;
  pActions[eMenu::Action::DeletePayee]->setEnabled(b);
}

void KPayeesView::slotSelectTransaction()
{
  Q_D(KPayeesView);
  /// @todo port to new model code
#if 0
  auto list = d->ui->m_register->selectedItems();
  if (!list.isEmpty()) {
    const auto t = dynamic_cast<KMyMoneyRegister::Transaction*>(list[0]);
    if (t)
      emit selectByVariant(QVariantList {QVariant(t->split().accountId()), QVariant(t->transaction().id()) }, eView::Intent::ShowTransaction);
  }
#endif
}

void KPayeesView::slotSelectPayeeAndTransaction(const QString& payeeId, const QString& accountId, const QString& transactionId)
{
  Q_D(KPayeesView);
  if (!isVisible())
    return;

  /// @todo port to new model code
#if 0
  try {
    // clear filter
    d->m_searchWidget->clear();
    d->m_searchWidget->updateSearch();

    // deselect all other selected items
    QList<QListWidgetItem *> selectedItems = d->ui->m_payeesList->selectedItems();
    QList<QListWidgetItem *>::const_iterator payeesIt = selectedItems.constBegin();
    while (payeesIt != selectedItems.constEnd()) {
      if (auto item = dynamic_cast<KPayeeListItem*>(*payeesIt))
        item->setSelected(false);
      ++payeesIt;
    }

    // find the payee in the list
    QListWidgetItem* it;
    for (int i = 0; i < d->ui->m_payeesList->count(); ++i) {
      it = d->ui->m_payeesList->item(i);
      auto item = dynamic_cast<KPayeeListItem *>(it);
      if (item && item->payee().id() == payeeId) {
        d->ui->m_payeesList->scrollToItem(it, QAbstractItemView::PositionAtCenter);

        d->ui->m_payeesList->setCurrentItem(it);     // active item and deselect all others
        d->ui->m_payeesList->setCurrentRow(i, QItemSelectionModel::ClearAndSelect); // and select it

        //make sure the payee selection is updated and transactions are updated accordingly
        slotSelectPayee();

        KMyMoneyRegister::RegisterItem *registerItem = 0;
        for (i = 0; i < d->ui->m_register->rowCount(); ++i) {
          registerItem = d->ui->m_register->itemAtRow(i);
          if (auto t = dynamic_cast<KMyMoneyRegister::Transaction*>(registerItem)) {
            if (t->transaction().id() == transactionId && t->transaction().accountReferenced(accountId)) {
              d->ui->m_register->selectItem(registerItem);
              d->ui->m_register->ensureItemVisible(registerItem);
              break;
            }
          }
        }
        // quit out of outer for() loop
        break;
      }
    }
  } catch (const MyMoneyException &e) {
    qWarning("Unexpected exception in KPayeesView::slotSelectPayeeAndTransaction %s", e.what());
  }
#endif
  d->ensurePayeeVisible(payeeId);
}

void KPayeesView::slotShowPayeesMenu(const QPoint& /*p*/)
{
  Q_D(KPayeesView);
  if (!d->ui->m_payees->selectionModel()->selectedIndexes().isEmpty()) {
    pMenus[eMenu::Menu::Payee]->exec(QCursor::pos());
  }
}

void KPayeesView::slotHelp()
{
  KHelpClient::invokeHelp("details.payees");
}

void KPayeesView::slotChangeFilter(int index)
{
  Q_D(KPayeesView);
  // update the filter type
  switch(index) {
    case eReferencedPayees:
      d->m_renameProxyModel->setReferenceFilter(ItemRenameProxyModel::eReferencedItems);
      break;
    case eUnusedPayees:
      d->m_renameProxyModel->setReferenceFilter(ItemRenameProxyModel::eUnReferencedItems);
      break;
    default:
      d->m_renameProxyModel->setReferenceFilter(ItemRenameProxyModel::eAllItem);
      break;
  }
}

void KPayeesView::slotNewPayee()
{
  QString id;
  KMyMoneyUtils::newPayee(i18n("New Payee"), id);
  slotSelectPayeeAndTransaction(id);
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

  if (KMessageBox::questionYesNo(this, prompt, i18n("Remove Payee")) == KMessageBox::No)
    return;

  d->payeeReassign(KPayeeReassignDlg::TypeDelete);
}

void KPayeesView::slotMergePayee()
{
  Q_D(KPayeesView);
  const auto payeesList = d->selectedPayees();
  if (payeesList.count() < 1)
    return; // shouldn't happen

  if (KMessageBox::questionYesNo(this, i18n("<p>Do you really want to merge the selected payees?"),
                                 i18n("Merge Payees")) == KMessageBox::No)
    return;

  if (d->payeeReassign(KPayeeReassignDlg::TypeMerge))
    /// @todo maybe select the one that is remaining and call
    ///       d->ensurePayeeVisible
    // clean selection since we just deleted the selected payees
    d->ui->m_payees->selectionModel()->clear();
}
