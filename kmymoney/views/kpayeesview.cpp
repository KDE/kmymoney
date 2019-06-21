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
#include <QTimer>
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
#include "models.h"
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

  KMyMoneyRegister::RegisterItem* item = d->ui->m_register->firstItem();
  while (item) {
    //only walk through selectable items. eg. transactions and not group markers
    if (item->isSelectable()) {
      auto t = dynamic_cast<KMyMoneyRegister::Transaction*>(item);
      if (!t)
        return;
      MyMoneySplit s = t->transaction().splitByPayee(d->m_payee.id());
      const MyMoneyAccount& acc = file->account(s.accountId());

      if (s.action() != MyMoneySplit::actionName(eMyMoney::Split::Action::Amortization)
          && acc.accountType() != eMyMoney::Account::Type::AssetLoan
          && !file->isTransfer(t->transaction())
          && t->transaction().splitCount() == 2) {
        MyMoneySplit s0 = t->transaction().splitByAccount(s.accountId(), false);
        if (account_count.contains(s0.accountId())) {
          account_count[s0.accountId()]++;
        } else {
          account_count[s0.accountId()] = 1;
        }
      }
    }
    item = item->nextItem();
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


void KPayeesView::slotStartRename(QListWidgetItem* item)
{
  Q_D(KPayeesView);
  d->m_allowEditing = true;
  d->ui->m_payeesList->editItem(item);
}

// This variant is only called when a single payee is selected and renamed.
void KPayeesView::slotRenameSinglePayee(QListWidgetItem* p)
{
  Q_D(KPayeesView);
  //if there is no current item selected, exit
  if (d->m_allowEditing == false || !d->ui->m_payeesList->currentItem() || p != d->ui->m_payeesList->currentItem())
    return;

  //qDebug() << "[KPayeesView::slotRenamePayee]";
  // create a copy of the new name without appended whitespaces
  QString new_name = p->text();
  if (d->m_payee.name() != new_name) {
    MyMoneyFileTransaction ft;
    try {
      // check if we already have a payee with the new name
      try {
        // this function call will throw an exception, if the payee
        // hasn't been found.
        MyMoneyFile::instance()->payeeByName(new_name);
        // the name already exists, ask the user whether he's sure to keep the name
        if (KMessageBox::questionYesNo(this,
                                       i18n("A payee with the name '%1' already exists. It is not advisable to have "
                                            "multiple payees with the same identification name. Are you sure you would like "
                                            "to rename the payee?", new_name)) != KMessageBox::Yes) {
          p->setText(d->m_payee.name());
          return;
        }
      } catch (const MyMoneyException &) {
        // all ok, the name is unique
      }

      d->m_payee.setName(new_name);
      d->m_newName = new_name;
      MyMoneyFile::instance()->modifyPayee(d->m_payee);

      // the above call to modifyPayee will reload the view so
      // all references and pointers to the view have to be
      // re-established.

      // make sure, that the record is visible even if it moved
      // out of sight due to the rename operation
      d->ensurePayeeVisible(d->m_payee.id());

      ft.commit();

    } catch (const MyMoneyException &e) {
      KMessageBox::detailedSorry(this, i18n("Unable to modify payee"), QString::fromLatin1(e.what()));
    }
  } else {
    p->setText(new_name);
  }
}

void KPayeesView::slotSelectPayee(QListWidgetItem* cur, QListWidgetItem* prev)
{
  Q_D(KPayeesView);
  Q_UNUSED(cur);
  Q_UNUSED(prev);

  d->m_allowEditing = false;
}

void KPayeesView::slotSelectPayee()
{
  Q_D(KPayeesView);
  // check if the content of a currently selected payee was modified
  // and ask to store the data
  if (d->isDirty()) {
    if (KMessageBox::questionYesNo(this,
                                   i18n("<qt>Do you want to save the changes for <b>%1</b>?</qt>", d->m_newName),
                                   i18n("Save changes")) == KMessageBox::Yes) {
      d->m_inSelection = true;
      slotUpdatePayee();
      d->m_inSelection = false;
    }
  }

  // make sure we always clear the selected list when listing again
  d->m_selectedPayeesList.clear();

  // loop over all payees and count the number of payees, also
  // obtain last selected payee
  d->selectedPayees(d->m_selectedPayeesList);
  updatePayeeActions(d->m_selectedPayeesList);

  emit selectObjects(d->m_selectedPayeesList);

  if (d->m_selectedPayeesList.isEmpty()) {
    d->ui->m_tabWidget->setEnabled(false); // disable tab widget
    d->ui->m_balanceLabel->hide();
    d->ui->m_deleteButton->setEnabled(false); //disable delete, rename and merge buttons
    d->ui->m_renameButton->setEnabled(false);
    d->ui->m_mergeButton->setEnabled(false);
    d->clearItemData();
    d->m_payee = MyMoneyPayee();
    d->ui->m_syncAddressbook->setEnabled(false);
    return; // make sure we don't access an undefined payee
  }

  d->ui->m_deleteButton->setEnabled(true); //re-enable delete button

  d->ui->m_syncAddressbook->setEnabled(true);

  // if we have multiple payees selected, clear and disable the payee information
  if (d->m_selectedPayeesList.count() > 1) {
    d->ui->m_tabWidget->setEnabled(false); // disable tab widget
    d->ui->m_renameButton->setEnabled(false); // disable also the rename button
    d->ui->m_mergeButton->setEnabled(true);
    d->ui->m_balanceLabel->hide();
    d->clearItemData();
  } else {
    d->ui->m_mergeButton->setEnabled(false);
    d->ui->m_renameButton->setEnabled(true);
  }

  // otherwise we have just one selected, enable payee information widget
  d->ui->m_tabWidget->setEnabled(true);
  d->ui->m_balanceLabel->show();

  // as of now we are updating only the last selected payee, and until
  // selection mode of the QListView has been changed to Extended, this
  // will also be the only selection and behave exactly as before - Andreas
  try {
    d->m_payee = d->m_selectedPayeesList[0];

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
    d->ui->m_register->clear();
    d->m_selectedPayeesList.clear();
    d->m_payee = MyMoneyPayee();
  }
  d->m_allowEditing = true;
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
  if (d->m_payeeRows.isEmpty()) {                            // empty list means no syncing is pending...
    foreach (auto item, d->ui->m_payeesList->selectedItems()) {
      d->m_payeeRows.append(d->ui->m_payeesList->row(item));        // ...so initialize one
    }
    d->ui->m_payeesList->clearSelection();                       // otherwise slotSelectPayee will be run after every payee update
//    d->ui->m_syncAddressbook->setEnabled(false);                 // disallow concurrent syncs
  }

  if (d->m_payeeRows.count() <= d->m_payeeRow) {
    if (auto item = dynamic_cast<KPayeeListItem*>(d->ui->m_payeesList->currentItem())) { // update ui if something is selected
      d->m_payee = item->payee();
      d->ui->addressEdit->setText(d->m_payee.address());
      d->ui->postcodeEdit->setText(d->m_payee.postcode());
      d->ui->telephoneEdit->setText(d->m_payee.telephone());
    }
    d->m_payeeRows.clear();  // that means end of sync
    d->m_payeeRow = 0;
    return;
  }

  if (auto item = dynamic_cast<KPayeeListItem*>(d->ui->m_payeesList->item(d->m_payeeRows.at(d->m_payeeRow))))
    d->m_payee = item->payee();
  ++d->m_payeeRow;

  d->m_contact->fetchContact(d->m_payee.email()); // search for payee's data in addressbook and receive it in slotContactFetched
}

void KPayeesView::slotContactFetched(const ContactData &identity)
{
  Q_D(KPayeesView);
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

    if (!identity.postalCode.isEmpty() && d->m_payee.postcode().compare(identity.postalCode) != 0)
      d->m_payee.setPostcode(identity.postalCode);

    if (!identity.phoneNumber.isEmpty() && d->m_payee.telephone().compare(identity.phoneNumber) != 0)
      d->m_payee.setTelephone(identity.phoneNumber);

    MyMoneyFileTransaction ft;
    try {
      MyMoneyFile::instance()->modifyPayee(d->m_payee);
      ft.commit();
    } catch (const MyMoneyException &e) {
      KMessageBox::detailedSorry(this, i18n("Unable to modify payee"), QString::fromLatin1(e.what()));
    }
  }

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
    case eView::Action::Refresh:
      refresh();
      break;

    case eView::Action::SetDefaultFocus:
      {
        Q_D(KPayeesView);
        QTimer::singleShot(0, d->m_searchWidget, SLOT(setFocus()));
      }
      break;

    case eView::Action::ClosePayeeIdentifierSource:
      slotClosePayeeIdentifierSource();
      break;

    default:
      break;
  }
}

void KPayeesView::refresh()
{
  Q_D(KPayeesView);
  if (isVisible()) {
    if (d->m_inSelection) {
      QTimer::singleShot(0, this, SLOT(refresh()));
    } else {
      d->loadPayees();
      d->m_needsRefresh = false;
    }
  } else {
    d->m_needsRefresh = true;
  }
}

void KPayeesView::showEvent(QShowEvent* event)
{
  if (MyMoneyFile::instance()->storageAttached()) {
    Q_D(KPayeesView);
    if (d->m_needLoad)
      d->init();

    emit customActionRequested(View::Payees, eView::Action::AboutToShow);

    if (d->m_needsRefresh)
      refresh();

    QList<MyMoneyPayee> list;
    d->selectedPayees(list);
    emit selectObjects(list);
  }

  // don't forget base class implementation
  QWidget::showEvent(event);
}

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

void KPayeesView::slotSelectTransaction()
{
  Q_D(KPayeesView);
  auto list = d->ui->m_register->selectedItems();
  if (!list.isEmpty()) {
    const auto t = dynamic_cast<KMyMoneyRegister::Transaction*>(list[0]);
    if (t)
      emit selectByVariant(QVariantList {QVariant(t->split().accountId()), QVariant(t->transaction().id()) }, eView::Intent::ShowTransaction);
  }
}

void KPayeesView::slotSelectPayeeAndTransaction(const QString& payeeId, const QString& accountId, const QString& transactionId)
{
  Q_D(KPayeesView);
  if (!isVisible())
    return;

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
}

void KPayeesView::slotShowPayeesMenu(const QPoint& /*p*/)
{
  Q_D(KPayeesView);
  if (dynamic_cast<KPayeeListItem*>(d->ui->m_payeesList->currentItem())) {
    slotSelectPayee();
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
  //update the filter type then reload the payees list
  d->m_payeeFilterType = index;
  d->loadPayees();
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
  if (d->ui->m_payeesList->currentItem() && d->ui->m_payeesList->selectedItems().count() == 1) {
    slotStartRename(d->ui->m_payeesList->currentItem());
  }
}

void KPayeesView::slotDeletePayee()
{
  Q_D(KPayeesView);
  if (d->m_selectedPayeesList.isEmpty())
    return; // shouldn't happen

  // get confirmation from user
  QString prompt;
  if (d->m_selectedPayeesList.size() == 1)
    prompt = i18n("<p>Do you really want to remove the payee <b>%1</b>?</p>", d->m_selectedPayeesList.front().name());
  else
    prompt = i18n("Do you really want to remove all selected payees?");

  if (KMessageBox::questionYesNo(this, prompt, i18n("Remove Payee")) == KMessageBox::No)
    return;

  d->payeeReassign(KPayeeReassignDlg::TypeDelete);
}

void KPayeesView::slotMergePayee()
{
  Q_D(KPayeesView);
  if (d->m_selectedPayeesList.size() < 1)
    return; // shouldn't happen

  if (KMessageBox::questionYesNo(this, i18n("<p>Do you really want to merge the selected payees?"),
                                 i18n("Merge Payees")) == KMessageBox::No)
    return;

  if (d->payeeReassign(KPayeeReassignDlg::TypeMerge))
    // clean selection since we just deleted the selected payees
    d->m_selectedPayeesList.clear();
}
