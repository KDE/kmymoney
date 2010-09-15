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
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kpayeesview.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QLayout>
#include <QPixmap>
#include <QTabWidget>
#include <QCheckBox>
#include <QRadioButton>
#include <QPainter>
#include <QSplitter>
#include <QMap>
#include <QList>
#include <QResizeEvent>
#include <QtAlgorithms>
#include <QTimer>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kiconloader.h>
#include <kguiitem.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kapplication.h>
#include <keditlistbox.h>
#include <ktoolinvocation.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "kmymoneyglobalsettings.h"
#include "kmymoney.h"
#include "models.h"

/* -------------------------------------------------------------------------------*/
/*                               KTransactionPtrVector                            */
/* -------------------------------------------------------------------------------*/

// *** KPayeeListItem Implementation ***

KPayeeListItem::KPayeeListItem(QListWidget *parent, const MyMoneyPayee& payee) :
    QListWidgetItem(parent, QListWidgetItem::UserType),
    m_payee(payee)
{
  setText(payee.name());
  // allow in column rename
  setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

KPayeeListItem::~KPayeeListItem()
{
}

// *** KPayeesView Implementation ***

KPayeesView::KPayeesView(QWidget *parent) :
    QWidget(parent),
    m_needReload(false),
    m_needConnection(true),
    m_updatesQueued(0),
    m_inSelection(false),
    m_payeeInEditing(false)
{
  setupUi(this);

  m_filterProxyModel = new AccountNamesFilterProxyModel(this);
  m_filterProxyModel->addAccountGroup(MyMoneyAccount::Asset);
  m_filterProxyModel->addAccountGroup(MyMoneyAccount::Liability);
  m_filterProxyModel->addAccountGroup(MyMoneyAccount::Income);
  m_filterProxyModel->addAccountGroup(MyMoneyAccount::Expense);
  m_filterProxyModel->setSourceModel(Models::instance()->accountsModel());
  m_filterProxyModel->sort(0);
  comboDefaultAccount->setModel(m_filterProxyModel);

  m_matchType->setId(radioNoMatch, 0);
  m_matchType->setId(radioNameMatch, 1);
  m_matchType->setId(radioKeyMatch, 2);

  // create the searchline widget
  // and insert it into the existing layout
  m_searchWidget = new KListWidgetSearchLine(this, m_payeesList);
  m_searchWidget->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
  m_payeesList->setContextMenuPolicy(Qt::CustomContextMenu);

  m_listLayout->insertWidget(1, m_searchWidget);

  KGuiItem newButtonItem(QString(""),
                         KIcon("list-add-user"),
                         i18n("Creates a new payee"),
                         i18n("Use this to create a new payee."));
  m_newButton->setGuiItem(newButtonItem);
  m_newButton->setToolTip(newButtonItem.toolTip());

  KGuiItem renameButtonItem(QString(""),
                            KIcon("payee-rename"),
                            i18n("Rename the current selected payee"),
                            i18n("Use this to start renaming the selected payee."));
  m_renameButton->setGuiItem(renameButtonItem);
  m_renameButton->setToolTip(renameButtonItem.toolTip());

  KGuiItem deleteButtonItem(QString(""),
                            KIcon("list-remove-user"),
                            i18n("Delete the current selected payee"),
                            i18n("Use this to delete the selected payee."));
  m_deleteButton->setGuiItem(deleteButtonItem);
  m_deleteButton->setToolTip(deleteButtonItem.toolTip());

  KGuiItem updateButtonItem(i18nc("Update payee", "Update"),
                            KIcon("dialog-ok"),
                            i18n("Accepts the entered data and stores it"),
                            i18n("Use this to accept the modified data."));
  m_updateButton->setGuiItem(updateButtonItem);

  m_updateButton->setEnabled(false);
  radioNoMatch->setChecked(true);

  checkMatchIgnoreCase->setEnabled(false);

  checkEnableDefaultAccount->setChecked(false);
  labelDefaultAccount->setEnabled(false);
  comboDefaultAccount->setEnabled(false);

  QList<KMyMoneyRegister::Column> cols;
  cols << KMyMoneyRegister::DateColumn;
  cols << KMyMoneyRegister::AccountColumn;
  cols << KMyMoneyRegister::DetailColumn;
  cols << KMyMoneyRegister::ReconcileFlagColumn;
  cols << KMyMoneyRegister::PaymentColumn;
  cols << KMyMoneyRegister::DepositColumn;
  m_register->setupRegister(MyMoneyAccount(), cols);
  m_register->setSelectionMode(QTableWidget::SingleSelection);
  m_register->setDetailsColumnType(KMyMoneyRegister::AccountFirst);
  m_balanceLabel->hide();

  connect(m_payeesList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(slotSelectPayee()));
  connect(m_payeesList, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(slotSelectPayee()));
  connect(m_payeesList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(slotStartRename(QListWidgetItem*)));
  connect(m_payeesList, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(slotRenamePayee(QListWidgetItem*)));

  connect(m_renameButton, SIGNAL(clicked()), this, SLOT(slotRenameButtonCliked()));
  connect(m_deleteButton, SIGNAL(clicked()), kmymoney->action("payee_delete"), SLOT(trigger()));
  connect(m_newButton, SIGNAL(clicked()), this, SLOT(slotPayeeNew()));

  connect(addressEdit, SIGNAL(textChanged()), this, SLOT(slotPayeeDataChanged()));
  connect(postcodeEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotPayeeDataChanged()));
  connect(telephoneEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotPayeeDataChanged()));
  connect(emailEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotPayeeDataChanged()));
  connect(notesEdit, SIGNAL(textChanged()), this, SLOT(slotPayeeDataChanged()));
  connect(matchKeyEditList, SIGNAL(changed()), this, SLOT(slotKeyListChanged()));

  connect(radioNoMatch, SIGNAL(toggled(bool)), this, SLOT(slotPayeeDataChanged()));
  connect(radioNameMatch, SIGNAL(toggled(bool)), this, SLOT(slotPayeeDataChanged()));
  connect(radioKeyMatch, SIGNAL(toggled(bool)), this, SLOT(slotPayeeDataChanged()));
  connect(checkMatchIgnoreCase, SIGNAL(toggled(bool)), this, SLOT(slotPayeeDataChanged()));

  connect(checkEnableDefaultAccount, SIGNAL(toggled(bool)), this, SLOT(slotPayeeDataChanged()));
  connect(comboDefaultAccount, SIGNAL(accountSelected(const QString&)), this, SLOT(slotPayeeDataChanged()));
  connect(buttonSelectMyAccount, SIGNAL(clicked()), this, SLOT(slotChooseDefaultAccount()));

  connect(m_updateButton, SIGNAL(clicked()), this, SLOT(slotUpdatePayee()));
  connect(m_helpButton, SIGNAL(clicked()), this, SLOT(slotHelp()));

  connect(m_payeesList, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(slotOpenContextMenu(const QPoint&)));

  connect(m_register, SIGNAL(editTransaction()), this, SLOT(slotSelectTransaction()));

  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadPayees()));

  // use the size settings of the last run (if any)
  KConfigGroup grp = KGlobal::config()->group("Last Use Settings");
  QList<int> sizes = grp.readEntry("KPayeesViewSplitterSize", QList<int>());
  if (sizes.size() == 2) {
    if (!sizes[0] || !sizes[1]) {
      sizes[0] = 1;
      sizes[1] = 2;
    }
    m_splitter->setSizes(sizes);
  }
}

KPayeesView::~KPayeesView()
{
  // remember the splitter settings for startup
  KConfigGroup grp = KGlobal::config()->group("Last Use Settings");
  grp.writeEntry("KPayeesViewSplitterSize", m_splitter->sizes());
  grp.sync();
}

void KPayeesView::slotChooseDefaultAccount(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  QMap<QString, int> account_count;

  KMyMoneyRegister::RegisterItem* item = m_register->firstItem();
  while (item) {
    //only walk through selectable items. eg. transactions and not group markers
    if (item->isSelectable()) {
      KMyMoneyRegister::Transaction* t = dynamic_cast<KMyMoneyRegister::Transaction*>(item);

      MyMoneySplit s = t->transaction().splitByPayee(m_payee.id());
      const MyMoneyAccount& acc = file->account(s.accountId());

      QString txt;
      if (s.action() != MyMoneySplit::ActionAmortization
          && acc.accountType() != MyMoneyAccount::AssetLoan
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
    checkEnableDefaultAccount->setChecked(true);
    comboDefaultAccount->setSelected(most_frequent.key());
  }
}

void KPayeesView::slotStartRename(QListWidgetItem* item)
{
  m_payeeInEditing = true;
  m_payeesList->editItem(item);
}

void KPayeesView::slotRenameButtonCliked()
{
  if (m_payeesList->currentItem() && m_payeesList->selectedItems().count() == 1) {
    slotStartRename(m_payeesList->currentItem());
  }

}

// This variant is only called when a single payee is selected and renamed.
void KPayeesView::slotRenamePayee(QListWidgetItem* p)
{
  //if there is no current item selected, exit
  if (m_payeeInEditing == false || !m_payeesList->currentItem() || p != m_payeesList->currentItem())
    return;

  m_payeeInEditing = false;
  //kDebug() << "[KPayeesView::slotRenamePayee]";
  // create a copy of the new name without appended whitespaces
  QString new_name = p->text();
  if (m_payee.name() != new_name) {
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
          p->setText(m_payee.name());
          return;
        }
      } catch (MyMoneyException *e) {
        // all ok, the name is unique
        delete e;
      }

      m_payee.setName(new_name);
      m_newName = new_name;
      MyMoneyFile::instance()->modifyPayee(m_payee);

      // the above call to modifyPayee will reload the view so
      // all references and pointers to the view have to be
      // re-established.

      // make sure, that the record is visible even if it moved
      // out of sight due to the rename operation
      ensurePayeeVisible(m_payee.id());

      ft.commit();

    } catch (MyMoneyException *e) {
      KMessageBox::detailedSorry(0, i18n("Unable to modify payee"),
                                 (e->what() + ' ' + i18n("thrown in") + ' ' + e->file() + ":%1").arg(e->line()));
      delete e;
    }
  } else {
    p->setText(new_name);
  }
}

void KPayeesView::ensurePayeeVisible(const QString& id)
{
  for (int i = 0; i < m_payeesList->count(); ++i) {
    KPayeeListItem* p = dynamic_cast<KPayeeListItem*>(m_payeesList->item(0));
    if (p && p->payee().id() == id) {
      m_payeesList->scrollToItem(p, QAbstractItemView::PositionAtCenter);

      m_payeesList->setCurrentItem(p);      // active item and deselect all others
      m_payeesList->setCurrentRow(i, QItemSelectionModel::ClearAndSelect);   // and select it
      break;
    }
  }
}

void KPayeesView::selectedPayees(QList<MyMoneyPayee>& payeesList) const
{
  QList<QListWidgetItem *> selectedItems = m_payeesList->selectedItems();
  QList<QListWidgetItem *>::ConstIterator itemsIt = selectedItems.constBegin();
  while (itemsIt != selectedItems.constEnd()) {
    KPayeeListItem* item = dynamic_cast<KPayeeListItem*>(*itemsIt);
    if (item)
      payeesList << item->payee();
    ++itemsIt;
  }
}

void KPayeesView::slotSelectPayee(void)
{
  m_payeeInEditing = false;

  // check if the content of a currently selected payee was modified
  // and ask to store the data
  if (m_updateButton->isEnabled()) {
    if (KMessageBox::questionYesNo(this, QString("<qt>%1</qt>").arg(
                                     i18n("Do you want to save the changes for <b>%1</b>?", m_newName)),
                                   i18n("Save changes")) == KMessageBox::Yes) {
      m_inSelection = true;
      slotUpdatePayee();
      m_inSelection = false;
    }
  }

  // loop over all payees and count the number of payees, also
  // optain last selected payee
  QList<MyMoneyPayee> payeesList;
  selectedPayees(payeesList);

  emit selectObjects(payeesList);

  if (payeesList.isEmpty()) {
    m_tabWidget->setEnabled(false); // disable tab widget
    m_balanceLabel->hide();
    clearItemData();
    m_payee = MyMoneyPayee();
    return; // make sure we don't access an undefined payee
  }

  // if we have multiple payees selected, clear and disable the payee information
  if (payeesList.count() > 1) {
    m_tabWidget->setEnabled(false); // disable tab widget
    m_balanceLabel->hide();
    clearItemData();
    // disable renaming in all listviewitem
    for (int i = 0; i < m_payeesList->count(); ++i)
      m_payeesList->item(i)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    return;
  }
  // otherwise we have just one selected, enable payee information widget
  m_tabWidget->setEnabled(true);
  m_balanceLabel->show();
  // enable renaming in all listviewitem
  for (int i = 0; i < m_payeesList->count(); ++i)
    m_payeesList->item(i)->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);

  // as of now we are updating only the last selected payee, and until
  // selection mode of the QListView has been changed to Extended, this
  // will also be the only selection and behave exactly as before - Andreas
  try {
    m_payee = payeesList[0];
    m_newName = m_payee.name();

    addressEdit->setEnabled(true);
    addressEdit->setText(m_payee.address());
    postcodeEdit->setEnabled(true);
    postcodeEdit->setText(m_payee.postcode());
    telephoneEdit->setEnabled(true);
    telephoneEdit->setText(m_payee.telephone());
    emailEdit->setEnabled(true);
    emailEdit->setText(m_payee.email());
    notesEdit->setText(m_payee.notes());

    QStringList keys;
    bool ignorecase = false;
    MyMoneyPayee::payeeMatchType type = m_payee.matchData(ignorecase, keys);

    m_matchType->button(static_cast<int>(type))->setChecked(true);
    matchKeyEditList->clear();
    matchKeyEditList->insertStringList(keys);
    checkMatchIgnoreCase->setChecked(ignorecase);

    checkEnableDefaultAccount->setChecked(m_payee.defaultAccountEnabled());
    comboDefaultAccount->setSelected(m_payee.defaultAccountId());

    slotPayeeDataChanged();

    showTransactions();

  } catch (MyMoneyException *e) {
    qDebug("exception during display of payee: %s at %s:%ld", qPrintable(e->what()), qPrintable(e->file()), e->line());
    m_register->clear();
    m_payee = MyMoneyPayee();
    delete e;
  }
}

void KPayeesView::clearItemData(void)
{
  addressEdit->setText(QString());
  postcodeEdit->setText(QString());
  telephoneEdit->setText(QString());
  emailEdit->setText(QString());
  notesEdit->setText(QString());
  showTransactions();
}

void KPayeesView::showTransactions(void)
{
  MyMoneyMoney balance(0);

  // setup sort order
  m_register->setSortOrder(KMyMoneyGlobalSettings::sortSearchView());

  // clear the register
  m_register->clear();

  if (m_payee.id().isEmpty() || !m_tabWidget->isEnabled()) {
    m_balanceLabel->setText(i18n("Balance: %1", balance.formatMoney(MyMoneyFile::instance()->baseCurrency().smallestAccountFraction())));
    return;
  }

  // setup the list and the pointer vector
  MyMoneyTransactionFilter filter;
  filter.addPayee(m_payee.id());
  filter.setDateFilter(KMyMoneyGlobalSettings::startDate().date(), QDate());

  // retrieve the list from the engine
  MyMoneyFile::instance()->transactionList(m_transactionList, filter);

  // create the elements for the register
  QList<QPair<MyMoneyTransaction, MyMoneySplit> >::const_iterator it;
  QMap<QString, int> uniqueMap;
  MyMoneyMoney deposit, payment;

  int splitCount = 0;
  for (it = m_transactionList.constBegin(); it != m_transactionList.constEnd(); ++it) {
    const MyMoneySplit& split = (*it).second;
    MyMoneyAccount acc = MyMoneyFile::instance()->account(split.accountId());
    ++splitCount;
    uniqueMap[(*it).first.id()]++;

    KMyMoneyRegister::Register::transactionFactory(m_register, (*it).first, (*it).second, uniqueMap[(*it).first.id()]);
    if (split.shares().isNegative()) {
      payment += split.shares().abs();
    } else {
      deposit += split.shares().abs();
    }
  }
  balance = deposit - payment;

  // add the group markers
  m_register->addGroupMarkers();

  // sort the transactions according to the sort setting
  m_register->sortItems();

  // remove trailing and adjacent markers
  m_register->removeUnwantedGroupMarkers();

  m_register->updateRegister(true);

  // we might end up here with updates disabled on the register so make sure that we enable updates here
  m_register->setUpdatesEnabled(true);

  m_balanceLabel->setText(i18n("Balance: %1", balance.formatMoney(MyMoneyFile::instance()->baseCurrency().smallestAccountFraction())));
}

void KPayeesView::slotKeyListChanged(void)
{
  bool rc = false;
  bool ignorecase = false;
  QStringList keys;
  // J.Rodehueser: delete unused variable 'type'
  // orig:  MyMoneyPayee::payeeMatchType type = m_payee.matchData(ignorecase, keys);
  m_payee.matchData(ignorecase, keys);
  if (m_matchType->checkedId() == MyMoneyPayee::matchKey) {
    rc |= (keys != matchKeyEditList->items());
  }
  m_updateButton->setEnabled(rc);
}

void KPayeesView::slotPayeeDataChanged(void)
{
  bool rc = false;

  if (m_tabWidget->isEnabled()) {
    rc |= ((m_payee.email().isEmpty() != emailEdit->text().isEmpty())
           || (!emailEdit->text().isEmpty() && m_payee.email() != emailEdit->text()));
    rc |= ((m_payee.address().isEmpty() != addressEdit->toPlainText().isEmpty())
           || (!addressEdit->toPlainText().isEmpty() && m_payee.address() != addressEdit->toPlainText()));
    rc |= ((m_payee.postcode().isEmpty() != postcodeEdit->text().isEmpty())
           || (!postcodeEdit->text().isEmpty() && m_payee.postcode() != postcodeEdit->text()));
    rc |= ((m_payee.telephone().isEmpty() != telephoneEdit->text().isEmpty())
           || (!telephoneEdit->text().isEmpty() && m_payee.telephone() != telephoneEdit->text()));
    rc |= ((m_payee.name().isEmpty() != m_newName.isEmpty())
           || (!m_newName.isEmpty() && m_payee.name() != m_newName));
    rc |= ((m_payee.notes().isEmpty() != notesEdit->toPlainText().isEmpty())
           || (!notesEdit->toPlainText().isEmpty() && m_payee.notes() != notesEdit->toPlainText()));

    bool ignorecase = false;
    QStringList keys;

    MyMoneyPayee::payeeMatchType type = m_payee.matchData(ignorecase, keys);
    rc |= (static_cast<int>(type) != m_matchType->checkedId());

    checkMatchIgnoreCase->setEnabled(false);
    matchKeyEditList->setEnabled(false);

    if (m_matchType->checkedId() != MyMoneyPayee::matchDisabled) {
      checkMatchIgnoreCase->setEnabled(true);
      // if we turn matching on, we default to 'ignore case'
      // TODO maybe make the default a user option
      if (type == MyMoneyPayee::matchDisabled && m_matchType->checkedId() != MyMoneyPayee::matchDisabled)
        checkMatchIgnoreCase->setChecked(true);
      rc |= (ignorecase != checkMatchIgnoreCase->isChecked());
      if (m_matchType->checkedId() == MyMoneyPayee::matchKey) {
        matchKeyEditList->setEnabled(true);
        rc |= (keys != matchKeyEditList->items());
      }
    }

    rc |= (checkEnableDefaultAccount->isChecked() != m_payee.defaultAccountEnabled());
    if (checkEnableDefaultAccount->isChecked()) {
      comboDefaultAccount->setEnabled(true);
      labelDefaultAccount->setEnabled(true);
      // this is only going to understand the first in the list of selected accounts
      if (comboDefaultAccount->getSelected().isEmpty()) {
        rc |= !m_payee.defaultAccountId().isEmpty();
      } else {
        QString temp = comboDefaultAccount->getSelected();
        rc |= (temp.isEmpty() != m_payee.defaultAccountId().isEmpty())
              || (!m_payee.defaultAccountId().isEmpty() && temp != m_payee.defaultAccountId());
      }
    } else {
      comboDefaultAccount->setEnabled(false);
      labelDefaultAccount->setEnabled(false);
    }
  }
  m_updateButton->setEnabled(rc);
}

void KPayeesView::slotUpdatePayee(void)
{
  if (m_updateButton->isEnabled()) {
    MyMoneyFileTransaction ft;
    m_updateButton->setEnabled(false);
    try {
      m_payee.setName(m_newName);
      m_payee.setAddress(addressEdit->toPlainText());
      m_payee.setPostcode(postcodeEdit->text());
      m_payee.setTelephone(telephoneEdit->text());
      m_payee.setEmail(emailEdit->text());
      m_payee.setNotes(notesEdit->toPlainText());
      m_payee.setMatchData(static_cast<MyMoneyPayee::payeeMatchType>(m_matchType->checkedId()), checkMatchIgnoreCase->isChecked(), matchKeyEditList->items());
      m_payee.setDefaultAccountId();

      if (checkEnableDefaultAccount->isChecked()) {
        QString temp;
        if (!comboDefaultAccount->getSelected().isEmpty()) {
          temp = comboDefaultAccount->getSelected();
          m_payee.setDefaultAccountId(temp);
        }
      }

      MyMoneyFile::instance()->modifyPayee(m_payee);
      ft.commit();

    } catch (MyMoneyException *e) {
      KMessageBox::detailedSorry(0, i18n("Unable to modify payee"),
                                 (e->what() + ' ' + i18n("thrown in") + ' ' + e->file() + ":%1").arg(e->line()));
      delete e;
    }
  }
}

void KPayeesView::showEvent(QShowEvent* event)
{
  if (m_needReload) {
    loadPayees();
    m_needReload = false;
  }

  // don't forget base class implementation
  QWidget::showEvent(event);

  QList<MyMoneyPayee> list;
  selectedPayees(list);
  emit selectObjects(list);
}

void KPayeesView::slotLoadPayees(void)
{
  if (isVisible()) {
    if (m_inSelection)
      QTimer::singleShot(0, this, SLOT(slotLoadPayees()));
    else
      loadPayees();
  } else {
    m_needReload = true;
  }
}

void KPayeesView::loadPayees(void)
{
  if (m_inSelection)
    return;

  QMap<QString, bool> isSelected;
  QString id;

  ::timetrace("Start KPayeesView::loadPayees");

  // remember which items are selected in the list
  QList<QListWidgetItem *> selectedItems = m_payeesList->selectedItems();
  QList<QListWidgetItem *>::const_iterator payeesIt = selectedItems.constBegin();

  while (payeesIt != selectedItems.constEnd()) {
    KPayeeListItem* item = dynamic_cast<KPayeeListItem*>(*payeesIt);
    if (item)
      isSelected[item->payee().id()] = true;
    ++payeesIt;
  }

  // keep current selected item
  KPayeeListItem *currentItem = static_cast<KPayeeListItem *>(m_payeesList->currentItem());
  if (currentItem)
    id = currentItem->payee().id();

  // clear the list
  m_payeesList->clear();
  m_register->clear();
  currentItem = 0;

  QList<MyMoneyPayee>list = MyMoneyFile::instance()->payeeList();
  QList<MyMoneyPayee>::ConstIterator it;

  for (it = list.constBegin(); it != list.constEnd(); ++it) {
    KPayeeListItem* item = new KPayeeListItem(m_payeesList, *it);
    if (item->payee().id() == id)
      currentItem = item;
    if (isSelected[item->payee().id()])
      item->setSelected(true);
  }
  m_payeesList->sortItems();

  if (currentItem) {
    m_payeesList->setCurrentItem(currentItem);
    m_payeesList->scrollToItem(currentItem);
  }

  m_searchWidget->updateSearch(QString());

  m_filterProxyModel->invalidate();
  comboDefaultAccount->expandAll();

  slotSelectPayee();

  ::timetrace("End KPayeesView::loadPayees");
}

void KPayeesView::slotSelectTransaction(void)
{
  QList<KMyMoneyRegister::RegisterItem*> list = m_register->selectedItems();
  if (!list.isEmpty()) {
    KMyMoneyRegister::Transaction* t = dynamic_cast<KMyMoneyRegister::Transaction*>(list[0]);
    if (t)
      emit transactionSelected(t->split().accountId(), t->transaction().id());
  }
}

void KPayeesView::slotSelectPayeeAndTransaction(const QString& payeeId, const QString& accountId, const QString& transactionId)
{
  if (!isVisible())
    return;

  try {
    // clear filter
    m_searchWidget->clear();
    m_searchWidget->updateSearch();

    // deselect all other selected items
    QList<QListWidgetItem *> selectedItems = m_payeesList->selectedItems();
    QList<QListWidgetItem *>::const_iterator payeesIt = selectedItems.constBegin();
    while (payeesIt != selectedItems.constEnd()) {
      KPayeeListItem* item = dynamic_cast<KPayeeListItem*>(*payeesIt);
      if (item)
        item->setSelected(false);
      ++payeesIt;
    }

    // find the payee in the list
    QListWidgetItem* it;
    for (int i = 0; i < m_payeesList->count(); ++i) {
      it = m_payeesList->item(i);
      KPayeeListItem* item = dynamic_cast<KPayeeListItem *>(it);
      if (item && item->payee().id() == payeeId) {
        m_payeesList->scrollToItem(it, QAbstractItemView::PositionAtCenter);

        m_payeesList->setCurrentItem(it);     // active item and deselect all others
        m_payeesList->setCurrentRow(i, QItemSelectionModel::ClearAndSelect); // and select it

        //make sure the payee selection is updated and transactions are updated accordingly
        slotSelectPayee();

        KMyMoneyRegister::RegisterItem *item = 0;
        for (int i = 0; i < m_register->rowCount(); ++i) {
          item = m_register->itemAtRow(i);
          KMyMoneyRegister::Transaction* t = dynamic_cast<KMyMoneyRegister::Transaction*>(item);
          if (t) {
            if (t->transaction().id() == transactionId && t->transaction().accountReferenced(accountId)) {
              m_register->selectItem(item);
              m_register->ensureItemVisible(item);
              break;
            }
          }
        }
        // quit out of for() loop
        break;
      }
    }
  } catch (MyMoneyException *e) {
    qWarning("Unexpected exception in KPayeesView::slotSelectPayeeAndTransaction");
    delete e;
  }
}

void KPayeesView::slotOpenContextMenu(const QPoint& /*p*/)
{
  KPayeeListItem* item = dynamic_cast<KPayeeListItem*>(m_payeesList->currentItem());
  if (item) {
    slotSelectPayee();
    emit openContextMenu(item->payee());
  }
}

void KPayeesView::slotPayeeNew(void)
{
  kmymoney->action("payee_new")->trigger();
}

void KPayeesView::slotHelp(void)
{
  KToolInvocation::invokeHelp("details.payees.personalinformation");
}

#include "kpayeesview.moc"
