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
#include <q3multilineedit.h>
#include <QPixmap>
#include <QTabWidget>
#include <QCursor>
#include <QCheckBox>
#include <QRadioButton>
#include <QPainter>
#include <q3header.h>
#include <q3buttongroup.h>
#include <QSplitter>
#include <qmap.h>
#include <QList>
#include <QResizeEvent>

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
#include <K3ListViewSearchLineWidget>
// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyfile.h>
#include <kmymoneyaccounttree.h>
#include <kmymoneyglobalsettings.h>
#include <ktoolinvocation.h>

/* -------------------------------------------------------------------------------*/
/*                               KTransactionPtrVector                            */
/* -------------------------------------------------------------------------------*/

int KTransactionPtrVector::compareItems(const QString& s1, const QString& s2) const
{
  if (s1 == s2)
    return 0;
  if (s1 < s2)
    return -1;
  return 1;
}

int KTransactionPtrVector::compareItems(KTransactionPtrVector::Item d1, KTransactionPtrVector::Item d2)
{
  int   rc = 0;
  MyMoneyTransaction* t1 = static_cast<MyMoneyTransaction*>(d1);
  MyMoneyTransaction* t2 = static_cast<MyMoneyTransaction*>(d2);
  MyMoneyMoney tmp;

  try {
    MyMoneySplit s1;
    MyMoneySplit s2;
    switch (m_idMode) {
    case AccountMode:
      s1 = t1->splitByAccount(m_id);
      s2 = t2->splitByAccount(m_id);
      break;
    case PayeeMode:
      s1 = t1->splitByPayee(m_id);
      s2 = t2->splitByPayee(m_id);
      break;
    }
    QString p1, p2;

    switch (m_sortType) {
    case SortValue:
      rc = 1;
      tmp = s2.value() - s1.value();
      if (tmp.isZero()) {
        // same value? Sort by date
        rc = t2->postDate().daysTo(t1->postDate());
        if (rc == 0) {
          // same date? Sort by id
          rc = compareItems(t1->id(), t2->id());
        }
      } else if (tmp.isNegative()) {
        rc = -1;
      }
      break;

    case SortEntryDate:
      rc = t2->entryDate().daysTo(t1->entryDate());
      if (rc == 0) {
        // on same day, lower check numbers show up first
        rc = compareItems(s1.number(), s2.number());
        if (rc == 0) {
          // same number (e.g. empty)? larger amounts show up first
          rc = 1;
          tmp = s2.value() - s1.value();
          if (tmp.isZero()) {
            // same value? Sort by id
            rc = compareItems(t1->id(), t2->id());
          } else if (tmp.isNegative()) {
            rc = -1;
          }
        }
      }
      break;

    case SortEntryOrder:
      // sort by id
      rc = compareItems(t1->id(), t2->id());
      break;

    case SortTypeNr:
      rc = compareItems(s1.action(), s2.action());

      if (rc == 0) {
        // same action? Sort by nr
        rc = compareItems(s1.number(), s2.number());
        if (rc == 0) {
          // same number? Sort by date
          rc = t2->postDate().daysTo(t1->postDate());
          if (rc == 0) {
            // same date? Sort by value
            rc = 1;
            tmp = s2.value() - s1.value();
            if (tmp.isZero()) {
              // same value? sort by id
              rc = compareItems(t1->id(), t2->id());
            } else if (tmp.isNegative()) {
              rc = -1;
            }
          }
        }
      }
      break;

    case SortReceiver:
      if (!s2.payeeId().isEmpty()) {
        p2 = MyMoneyFile::instance()->payee(s2.payeeId()).name();
      }
      if (!s1.payeeId().isEmpty()) {
        p1 = MyMoneyFile::instance()->payee(s1.payeeId()).name();
      }

      rc = compareItems(p1, p2);

      if (rc == 0) {
        // same payee? Sort by date
        rc = t2->postDate().daysTo(t1->postDate());
        if (rc == 0) {
          // same date? Sort by value
          rc = 1;
          tmp = s2.value() - s1.value();
          if (tmp.isZero()) {
            // same value? sort by id
            rc = compareItems(t1->id(), t2->id());
          } else if (tmp.isNegative()) {
            rc = -1;
          }
        }
      }
      break;

    case SortNr:
      rc = compareItems(s1.number(), s2.number());
      if (rc == 0) {
        // same number? Sort by date
        rc = t2->postDate().daysTo(t1->postDate());
        if (rc == 0) {
          // same date? Sort by value
          rc = 1;
          tmp = s2.value() - s1.value();
          if (tmp.isZero()) {
            // same value? sort by id
            rc = compareItems(t1->id(), t2->id());
          } else if (tmp.isNegative()) {
            rc = -1;
          }
        }
      }
      break;

    case SortPostDate:
      // tricky fall through here!
    default:
      // sort by post date
      rc = t2->postDate().daysTo(t1->postDate());
      if (rc == 0) {
        // on same day, lower check numbers show up first
        rc = compareItems(s1.number(), s2.number());
        if (rc == 0) {
          // same number (e.g. empty)? larger amounts show up first
          rc = 1;
          tmp = s2.value() - s1.value();
          if (tmp.isZero()) {
            // same value? Sort by id
            rc = compareItems(t1->id(), t2->id());
          } else if (tmp.isNegative()) {
            rc = -1;
          }
        }
      }
      break;
    }
  } catch (MyMoneyException *e) {
    delete e;
  }
  return rc;
}

void KTransactionPtrVector::setSortType(const TransactionSortE type)
{
  m_sortType = type;
  sort();
}

void KTransactionPtrVector::setAccountId(const QString& id)
{
  m_id = id;
  m_idMode = AccountMode;
}

void KTransactionPtrVector::setPayeeId(const QString& id)
{
  m_id = id;
  m_idMode = PayeeMode;
}


// *** KPayeeListItem Implementation ***

KPayeeListItem::KPayeeListItem(K3ListView *parent, const MyMoneyPayee& payee) :
    K3ListViewItem(parent),
    m_payee(payee)
{
  setText(0, payee.name());
  // allow in column rename
  setRenameEnabled(0, true);
}

KPayeeListItem::~KPayeeListItem()
{
}

void KPayeeListItem::paintCell(QPainter *p, const QColorGroup & cg, int column, int width, int align)
{
  QColorGroup cg2(cg);

  if (isAlternate())
    cg2.setColor(QColorGroup::Base, KMyMoneyGlobalSettings::listColor());
  else
    cg2.setColor(QColorGroup::Base, KMyMoneyGlobalSettings::listBGColor());

  p->setFont(KMyMoneyGlobalSettings::listCellFont());

  Q3ListViewItem::paintCell(p, cg2, column, width, align);
}

KTransactionListItem::KTransactionListItem(K3ListView* view, KTransactionListItem* parent, const QString& accountId, const QString& transactionId) :
    K3ListViewItem(view, parent)
{
  m_accountId = accountId;
  m_transactionId = transactionId;
}

KTransactionListItem::~KTransactionListItem()
{
}

void KTransactionListItem::paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment)
{
  QColorGroup _cg = cg;
  _cg.setColor(QColorGroup::Base, backgroundColor());
  Q3ListViewItem::paintCell(p, _cg, column, width, alignment);
}

const QColor KTransactionListItem::backgroundColor(void)
{
  return isAlternate() ? KMyMoneyGlobalSettings::listBGColor() : KMyMoneyGlobalSettings::listColor();
}




// *** KPayeesView Implementation ***

KPayeesView::KPayeesView(QWidget *parent) :
    KPayeesViewDecl(parent),
    m_needReload(false),
    m_needConnection(true),
    m_updatesQueued(0),
    m_inSelection(false)
{

  m_matchType->setId(radioNoMatch, 0);
  m_matchType->setId(radioNameMatch, 1);
  m_matchType->setId(radioKeyMatch, 2);

  // create the searchline widget
  // and insert it into the existing layout
  m_searchWidget = new K3ListViewSearchLineWidget(m_payeesList, this);
  m_searchWidget->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));

  verticalLayout->insertWidget(0, m_searchWidget);

  m_transactionView->setSorting(-1);
  m_transactionView->setColumnWidthMode(2, Q3ListView::Manual);
  m_transactionView->setColumnAlignment(3, Qt::AlignRight);
  // never show horizontal scroll bars
  m_transactionView->setHScrollBarMode(Q3ScrollView::AlwaysOff);

  m_payeesList->addColumn(i18nc("Payee name", "Name"));

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

  connect(m_payeesList, SIGNAL(selectionChanged()), this, SLOT(slotSelectPayee()));
  connect(m_payeesList, SIGNAL(itemRenamed(Q3ListViewItem*, int, const QString&)), this, SLOT(slotRenamePayee(Q3ListViewItem*, int, const QString&)));

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

  connect(m_payeesList, SIGNAL(contextMenu(K3ListView*, Q3ListViewItem*, const QPoint&)), this, SLOT(slotOpenContextMenu(K3ListView*, Q3ListViewItem*, const QPoint&)));

  connect(m_transactionView, SIGNAL(doubleClicked(Q3ListViewItem*)),
          this, SLOT(slotTransactionDoubleClicked(Q3ListViewItem*)));

  connect(m_tabWidget, SIGNAL(currentChanged(QWidget*)), this, SLOT(rearrange(void)));

  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadPayees()));

  // use the size settings of the last run (if any)
  KConfigGroup grp = KGlobal::config()->group("Last Use Settings");
  QList<int> sizes = grp.readEntry("KPayeesViewSplitterSize", QList<int>());
  if (sizes.size() == 2) {
    if(!sizes[0] || !sizes[1]) {
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

void KPayeesView::slotQueueUpdate(void)
{
  m_updatesQueued++;
  // The K3ListViewSearchLineWidget has an internal timer for update purposes
  // of 200 ms, so we should be safe with 250 ms here
  QTimer::singleShot(250, this, SLOT(slotActivateUpdate()));
}

void KPayeesView::slotActivateUpdate(void)
{
  --m_updatesQueued;
  if (m_updatesQueued == 0)
    slotSelectPayee();
}

void KPayeesView::slotChooseDefaultAccount(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  QMap<QString, int> account_count;

  for (uint i = 0; i < m_transactionPtrVector.size(); ++i) {
    KMyMoneyTransaction* t = m_transactionPtrVector[i];
    MyMoneySplit s = t->splitById(t->splitId());
    const MyMoneyAccount& acc = file->account(s.accountId());

    QString txt;
    if (s.action() != MyMoneySplit::ActionAmortization
        && acc.accountType() != MyMoneyAccount::AssetLoan
        && !file->isTransfer(*t)
        && t->splitCount() == 2) {
      MyMoneySplit s0 = t->splitByAccount(s.accountId(), false);
      if (account_count.contains(s0.accountId())) {
        account_count[s0.accountId()]++;
      } else {
        account_count[s0.accountId()] = 1;
      }
    }
  }
  QMap<QString, int>::Iterator most_frequent, iter;
  most_frequent = account_count.end();
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

void KPayeesView::slotStartRename(void)
{
  Q3ListViewItemIterator it_l(m_payeesList, Q3ListViewItemIterator::Selected);
  Q3ListViewItem* it_v;
  if ((it_v = it_l.current()) != 0) {
    it_v->startRename(0);
  }
}

// This variant is only called when a single payee is selected and renamed.
void KPayeesView::slotRenamePayee(Q3ListViewItem* p , int /* col */, const QString& txt)
{
  //kDebug() << "[KPayeesView::slotRenamePayee]";
  // create a copy of the new name without appended whitespaces
  QString new_name = txt.trimmed();
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
          p->setText(0, m_payee.name());
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
    p->setText(0, new_name);
  }
}

void KPayeesView::ensurePayeeVisible(const QString& id)
{
  for (Q3ListViewItem * item = m_payeesList->firstChild(); item; item = item->itemBelow()) {
    KPayeeListItem* p = dynamic_cast<KPayeeListItem*>(item);
    if (p && p->payee().id() == id) {
      if (p->itemAbove())
        m_payeesList->ensureItemVisible(p->itemAbove());
      if (p->itemBelow())
        m_payeesList->ensureItemVisible(p->itemBelow());

      m_payeesList->setCurrentItem(p);      // active item and deselect all others
      m_payeesList->setSelected(p, true);   // and select it
      m_payeesList->ensureItemVisible(p);
      break;
    }
  }
}

void KPayeesView::selectedPayees(QList<MyMoneyPayee>& payeesList) const
{
  Q3ListViewItemIterator it_l(m_payeesList, Q3ListViewItemIterator::Selected | Q3ListViewItemIterator::Visible);
  Q3ListViewItem* it_v;
  while ((it_v = it_l.current()) != 0) {
    KPayeeListItem* item = dynamic_cast<KPayeeListItem*>(it_v);
    if (item)
      payeesList << item->payee();
    ++it_l;
  }
}

void KPayeesView::slotSelectPayee(void)
{
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
    clearItemData();
    m_payee = MyMoneyPayee();
    return; // make sure we don't access an undefined payee
  }

  // if we have multiple payees selected, clear and disable the payee information
  if (payeesList.count() > 1) {
    m_tabWidget->setEnabled(false); // disable tab widget
    clearItemData();
    // disable renaming in all listviewitem
    for (Q3ListViewItem * i = m_payeesList->firstChild(); i; i = i->itemBelow())
      i->setRenameEnabled(0, false);
    return;
  }
  // otherwise we have just one selected, enable payee information widget
  m_tabWidget->setEnabled(true);
  // enable renaming in all listviewitem
  for (Q3ListViewItem * i = m_payeesList->firstChild(); i; i = i->itemBelow())
    i->setRenameEnabled(0, true);

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
    m_transactionView->clear();
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
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyMoney balance(0);
  unsigned int   i;

  // clear the current transaction listview
  m_transactionView->clear();

  if (m_payee.id().isEmpty() || !m_tabWidget->isEnabled()) {
    m_balanceLabel->setText(i18n("Balance: %1", balance.formatMoney(MyMoneyFile::instance()->baseCurrency().smallestAccountFraction())));
    return;
  }

  // setup the list and the pointer vector
  MyMoneyTransactionFilter filter;
  filter.addPayee(m_payee.id());
  filter.setDateFilter(KMyMoneyGlobalSettings::startDate().date(), QDate());

  QList<MyMoneyTransaction> list = file->transactionList(filter);
  m_transactionList.clear();

  m_transactionPtrVector.clear();
  m_transactionPtrVector.resize(list.size());
  m_transactionPtrVector.setPayeeId(m_payee.id());
  m_transactionPtrVector.setSortType(KTransactionPtrVector::SortPostDate);

  QList<MyMoneyTransaction>::ConstIterator it_t;
  QString lastId;
  int ofs = 0;

  for (i = 0, it_t = list.constBegin(); it_t != list.constEnd(); ++it_t) {
    KMyMoneyTransaction k(*it_t);

    filter.match(*it_t);
    if (lastId != (*it_t).id()) {
      ofs = 0;
      lastId = (*it_t).id();
    } else
      ofs++;

    k.setSplitId(filter.matchingSplits()[ofs].id());
    MyMoneyAccount acc = MyMoneyFile::instance()->account(filter.matchingSplits()[ofs].accountId());
    if (acc.accountGroup() == MyMoneyAccount::Asset
        || acc.accountGroup() == MyMoneyAccount::Liability) {
      QList<KMyMoneyTransaction>::const_iterator it_k;
      m_transactionList.append(k);
      balance += k.splitById(k.splitId()).value();
      m_transactionPtrVector.insert(i, &(m_transactionList.last()));
      ++i;
    }
  }
  m_transactionPtrVector.resize(i);

  // sort the transactions
  m_transactionPtrVector.sort();

  // and fill the m_transactionView
  KTransactionListItem *item = 0;

  for (i = 0; i < m_transactionPtrVector.size(); ++i) {
    KMyMoneyTransaction* t = m_transactionPtrVector[i];
    MyMoneySplit s = t->splitById(t->splitId());
    const MyMoneyAccount& acc = file->account(s.accountId());

    item = new KTransactionListItem(m_transactionView, item, s.accountId(), t->id());
    item->setText(0, s.number());
    item->setText(1, KGlobal::locale()->formatDate(t->postDate(), KLocale::ShortDate));

    QString txt;
    if (s.action() == MyMoneySplit::ActionAmortization) {
      if (acc.accountType() == MyMoneyAccount::Loan) {
        if (s.value().isPositive()) {
          txt = i18n("Amortization of %1", acc.name());
        } else {
          txt = i18n("Payment to %1", acc.name());
        }
      } else if (acc.accountType() == MyMoneyAccount::AssetLoan) {
        if (s.value().isNegative()) {
          txt = i18n("Amortization of %1", acc.name());
        } else {
          txt = i18n("Payment to %1", acc.name());
        }
      } else {
        txt = i18n("Loan payment from %1", acc.name());
      }
    } else if (file->isTransfer(*t)) {
      if (!s.value().isNegative()) {
        txt = i18n("Transfer to %1", acc.name());
      } else {
        txt = i18n("Transfer from %1", acc.name());
      }
    } else if (t->splitCount() > 2) {
      txt = i18nc("Split transaction (category replacement)", "Split transaction");
    } else if (t->splitCount() == 2) {
      MyMoneySplit s0 = t->splitByAccount(s.accountId(), false);
      txt = MyMoneyFile::instance()->accountToCategory(s0.accountId());
    }
    item->setText(2, txt);
    item->setText(3, s.value().formatMoney(acc.fraction()));
  }
  m_balanceLabel->setText(i18n("Balance: %1", balance.formatMoney(MyMoneyFile::instance()->baseCurrency().smallestAccountFraction())));

  // Trick: it seems, that the initial sizing of the view does
  // not work correctly. At least, the columns do not get displayed
  // correct. Reason: the return value of m_transactionView->visibleWidth()
  // is incorrect. If the widget is visible, resizing works correctly.
  // So, we let the dialog show up and resize it then. It's not really
  // clean, but the only way I got the damned thing working.
  QTimer::singleShot(50, this, SLOT(rearrange()));
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
    rc |= ((m_payee.address().isEmpty() != addressEdit->text().isEmpty())
           || (!addressEdit->text().isEmpty() && m_payee.address() != addressEdit->text()));
    rc |= ((m_payee.postcode().isEmpty() != postcodeEdit->text().isEmpty())
           || (!postcodeEdit->text().isEmpty() && m_payee.postcode() != postcodeEdit->text()));
    rc |= ((m_payee.telephone().isEmpty() != telephoneEdit->text().isEmpty())
           || (!telephoneEdit->text().isEmpty() && m_payee.telephone() != telephoneEdit->text()));
    rc |= ((m_payee.name().isEmpty() != m_newName.isEmpty())
           || (!m_newName.isEmpty() && m_payee.name() != m_newName));
    rc |= ((m_payee.notes().isEmpty() != notesEdit->text().isEmpty())
           || (!notesEdit->text().isEmpty() && m_payee.notes() != notesEdit->text()));

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
      if (comboDefaultAccount->selectedAccounts().empty()) {
        rc |= !m_payee.defaultAccountId().isEmpty();
      } else {
        QString temp = comboDefaultAccount->selectedAccounts().front();
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
      m_payee.setAddress(addressEdit->text());
      m_payee.setPostcode(postcodeEdit->text());
      m_payee.setTelephone(telephoneEdit->text());
      m_payee.setEmail(emailEdit->text());
      m_payee.setNotes(notesEdit->text());
      m_payee.setMatchData(static_cast<MyMoneyPayee::payeeMatchType>(m_matchType->checkedId()), checkMatchIgnoreCase->isChecked(), matchKeyEditList->items());
      m_payee.setDefaultAccountId();

      if (checkEnableDefaultAccount->isChecked()) {
        QString temp;
        if (!comboDefaultAccount->selectedAccounts().empty()) {
          temp = comboDefaultAccount->selectedAccounts().front();
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

void KPayeesView::readConfig(void)
{
  m_transactionView->setFont(KMyMoneyGlobalSettings::listCellFont());

  QFontMetrics fm(KMyMoneyGlobalSettings::listHeaderFont());
  int height = fm.lineSpacing() + 6;

  m_transactionView->header()->setMinimumHeight(height);
  m_transactionView->header()->setMaximumHeight(height);
  m_transactionView->header()->setFont(KMyMoneyGlobalSettings::listHeaderFont());

  m_payeesList->setDefaultRenameAction(
    KMyMoneyGlobalSettings::focusChangeIsEnter() ? Q3ListView::Accept : Q3ListView::Reject);

  //initialize the account list?
  comboDefaultAccount->loadList((KMyMoneyUtils::categoryTypeE)(KMyMoneyUtils::asset | KMyMoneyUtils::liability | MyMoneyAccount::Income | MyMoneyAccount::Expense));

}

void KPayeesView::show(void)
{
  // since we could not construct the connection in our own ctor,
  // we set it up now. The widgets of the K3ListViewSearchLineWidget must exist by now.
  // If you want to learn about the details, see the source of K3ListViewSearchLineWidget's
  // constructor
  if (m_needConnection) {
    connect(m_searchWidget->searchLine(), SIGNAL(textChanged(const QString&)), this, SLOT(slotQueueUpdate(void)));
    m_needConnection = false;
  }

  if (m_needReload) {
    loadPayees();
    m_needReload = false;
  }

  // fixup the layout
  QTimer::singleShot(0, this, SLOT(rearrange()));

  // don't forget base class implementation
  KPayeesViewDecl::show();

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
  readConfig();

  // remember which items are selected in the list
  Q3ListViewItemIterator it_l(m_payeesList, Q3ListViewItemIterator::Selected);
  Q3ListViewItem* it_v;
  while ((it_v = it_l.current()) != 0) {
    KPayeeListItem* item = dynamic_cast<KPayeeListItem*>(it_v);
    if (item)
      isSelected[item->payee().id()] = true;
    ++it_l;
  }

  // keep current selected item
  KPayeeListItem *currentItem = static_cast<KPayeeListItem *>(m_payeesList->currentItem());
  if (currentItem)
    id = currentItem->payee().id();

  // remember the upper left corner of the viewport
  QPoint startPoint = m_payeesList->viewportToContents(QPoint(0, 0));

  // turn off updates to avoid flickering during reload
  //m_payeesList->setUpdatesEnabled(false);

  // clear the list
  m_payeesList->clear();
  m_transactionView->clear();
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

  if (currentItem) {
    m_payeesList->setCurrentItem(currentItem);
  }

  // reposition viewport
  m_payeesList->setContentsPos(startPoint.x(), startPoint.y());

  m_searchWidget->searchLine()->updateSearch(QString());

  // turn updates back on
  //m_payeesList->setUpdatesEnabled(true);

  slotSelectPayee();

  ::timetrace("End KPayeesView::loadPayees");
}

void KPayeesView::rearrange(void)
{
  resizeEvent(0);
}

void KPayeesView::resizeEvent(QResizeEvent* ev)
{
  // resize the register
  int w = m_transactionView->visibleWidth();
  w -= m_transactionView->columnWidth(0);
  w -= m_transactionView->columnWidth(1);
  w -= m_transactionView->columnWidth(3);
  m_transactionView->setColumnWidth(2, w);
  m_transactionView->resizeContents(
    m_transactionView->visibleWidth(),
    m_transactionView->contentsHeight());

  m_payeesList->setColumnWidth(0, m_payeesList->visibleWidth());
  KPayeesViewDecl::resizeEvent(ev);
}

void KPayeesView::slotTransactionDoubleClicked(Q3ListViewItem* i)
{
  KTransactionListItem* item = static_cast<KTransactionListItem *>(i);
  if (item)
    emit transactionSelected(item->accountId(), item->transactionId());
}

void KPayeesView::slotSelectPayeeAndTransaction(const QString& payeeId, const QString& accountId, const QString& transactionId)
{
  if (!isVisible())
    return;

  try {
    // clear filter
    m_searchWidget->searchLine()->clear();
    m_searchWidget->searchLine()->updateSearch();

    // deselect all other selected items
    Q3ListViewItemIterator it_l(m_payeesList, Q3ListViewItemIterator::Selected);
    Q3ListViewItem* it_v;
    while ((it_v = it_l.current()) != 0) {
      KPayeeListItem* item = dynamic_cast<KPayeeListItem*>(it_v);
      if (item)
        item->setSelected(false);
      ++it_l;
    }

    // find the payee in the list
    Q3ListViewItem* it;
    for (it = m_payeesList->firstChild(); it; it = it->itemBelow()) {
      KPayeeListItem* item = dynamic_cast<KPayeeListItem *>(it);
      if (item && item->payee().id() == payeeId) {
        if (it->itemAbove())
          m_payeesList->ensureItemVisible(it->itemAbove());
        if (it->itemBelow())
          m_payeesList->ensureItemVisible(it->itemBelow());

        m_payeesList->setCurrentItem(it);     // active item and deselect all others
        m_payeesList->setSelected(it, true);  // and select it
        m_payeesList->ensureItemVisible(it);

        KTransactionListItem* item = dynamic_cast<KTransactionListItem*>(m_transactionView->firstChild());
        while (item != 0) {
          if (item->accountId() == accountId && item->transactionId() == transactionId)
            break;
          item = dynamic_cast<KTransactionListItem*>(item->nextSibling());
        }
        if (!item) {
          item = dynamic_cast<KTransactionListItem*>(m_transactionView->firstChild());
        }
        if (item) {
          m_transactionView->setSelected(item, true);
          m_transactionView->ensureItemVisible(item);
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

void KPayeesView::slotOpenContextMenu(K3ListView* lv, Q3ListViewItem* i, const QPoint& p)
{
  Q_UNUSED(p);
  if (lv == m_payeesList) {
    KPayeeListItem* item = dynamic_cast<KPayeeListItem*>(i);
    if (item) {
      emit openContextMenu(item->payee());
    }
  }
}

void KPayeesView::slotHelp(void)
{
  KToolInvocation::invokeHelp("details.payees.personalinformation");
}

#include "kpayeesview.moc"
// vim:cin:si:ai:et:ts=2:sw=2:
