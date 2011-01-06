/***************************************************************************
                          kcurrencyeditdlg.cpp  -  description
                             -------------------
    begin                : Wed Mar 24 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           Alvaro Soliverez <asoliverez@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kcurrencyeditdlg.h"

#include <locale.h>

// ----------------------------------------------------------------------------
// QT Includes

#include <QTimer>
#include <QPixmap>
#include <QBitmap>
#include <QLabel>
#include <QList>
#include <QTreeWidget>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneysecurity.h>
#include <mymoneyfile.h>

KCurrencyEditDlg::KCurrencyEditDlg(QWidget *parent) :
    KCurrencyEditDlgDecl(parent),
    m_currencyInEditing(false)
{
  setButtons(KDialog::Close | KDialog::User1);
  button(KDialog::User1)->setText(i18n("Select as base currency"));
  setButtonsOrientation(Qt::Horizontal);
  setMainWidget(m_layoutWidget);

  // create the searchline widget
  // and insert it into the existing layout
  m_searchWidget = new KTreeWidgetSearchLineWidget(this, m_currencyList);
  m_searchWidget->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
  m_listLayout->insertWidget(0, m_searchWidget);

  m_currencyList->header()->setStretchLastSection(true);
  m_currencyList->setContextMenuPolicy(Qt::CustomContextMenu);

  connect(m_currencyList, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(slotOpenContextMenu(const QPoint&)));
  connect(m_currencyList, SIGNAL(itemActivated(QTreeWidgetItem*, int)), this, SLOT(slotStartEditId()));
  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadCurrencies()));
  connect(m_currencyList, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(slotUpdateCurrency(QTreeWidgetItem*)));

  slotLoadCurrencies();

  //resize the column widths
  m_currencyList->resizeColumnToContents(0);
  m_currencyList->resizeColumnToContents(1);
  m_currencyList->resizeColumnToContents(2);

  connect(this, SIGNAL(user1Clicked()), this, SLOT(slotSelectBaseCurrency()));

  QTimer::singleShot(10, this, SLOT(timerDone()));
}

void KCurrencyEditDlg::timerDone(void)
{
  if (!m_currency.id().isEmpty()) {
    QTreeWidgetItemIterator it(m_currencyList);
    QTreeWidgetItem* q;
    while ((q = *it) != 0) {
      if (q->text(1) == m_currency.id()) {
        m_currencyList->scrollToItem(q);
        break;
      }
      ++it;
    }
  }
}

KCurrencyEditDlg::~KCurrencyEditDlg()
{
}

void KCurrencyEditDlg::slotLoadCurrencies(void)
{
  disconnect(m_currencyList, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT(slotSelectCurrency(QTreeWidgetItem*)));
  disconnect(m_currencyList, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(slotRenameCurrency(QTreeWidgetItem*)));
  QList<MyMoneySecurity> list = MyMoneyFile::instance()->currencyList();
  QList<MyMoneySecurity>::ConstIterator it;
  QTreeWidgetItem *first = 0;

  QString localCurrency(localeconv()->int_curr_symbol);
  localCurrency.truncate(3);

  QString baseCurrency;
  try {
    baseCurrency = MyMoneyFile::instance()->baseCurrency().id();
  } catch (MyMoneyException *e) {
    qDebug("%s", qPrintable(e->what()));
    delete e;
  }

  // construct a transparent 16x16 pixmap
  QPixmap empty(16, 16);
  QBitmap mask(16, 16);
  mask.clear();
  empty.setMask(mask);

  m_currencyList->clear();
  for (it = list.constBegin(); it != list.constEnd(); ++it) {
    QTreeWidgetItem *p = new QTreeWidgetItem(m_currencyList);
    p->setText(0, (*it).name());
    p->setData(0, Qt::UserRole, QVariant::fromValue(*it));
    p->setFlags(p->flags() | Qt::ItemIsEditable);
    p->setText(1, (*it).id());
    p->setText(2, (*it).tradingSymbol());

    if ((*it).id() == baseCurrency) {
      p->setData(0, Qt::DecorationRole, KIcon("kmymoney"));
      if (m_currency.id().isEmpty())
        first = p;
    } else {
      p->setData(0, Qt::DecorationRole, empty);
    }

    // if we had a previously selected
    if (!m_currency.id().isEmpty()) {
      if (m_currency.id() == p->text(1))
        first = p;
    } else if ((*it).id() == localCurrency && !first)
      first = p;
  }

  m_currencyList->sortItems(0, Qt::AscendingOrder);

  connect(m_currencyList, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT(slotSelectCurrency(QTreeWidgetItem*)));
  connect(m_currencyList, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(slotRenameCurrency(QTreeWidgetItem*)));

  if (first == 0)
    first = m_currencyList->invisibleRootItem()->child(0);
  if (first != 0) {
    m_currencyList->setCurrentItem(first);
    m_currencyList->scrollToItem(first);
  }

  slotSelectCurrency(first);
}

void KCurrencyEditDlg::slotUpdateCurrency(QTreeWidgetItem* item)
{
  //if there is no current item selected, exit
  if (m_currencyInEditing == false || !m_currencyList->currentItem() || item != m_currencyList->currentItem())
    return;

  m_currencyInEditing = false;

  //verify that the stored currency id is not empty and the edited fields are not empty either
  if (!m_currency.id().isEmpty()
      && !m_currencyList->currentItem()->text(2).isEmpty()
      && !m_currencyList->currentItem()->text(0).isEmpty()) {
    //check that either the name or the id have changed
    if (m_currencyList->currentItem()->text(2) != m_currency.tradingSymbol()
        || m_currencyList->currentItem()->text(0) != m_currency.name()) {
      //update the name and the id
      m_currency.setName(m_currencyList->currentItem()->text(0));
      m_currency.setTradingSymbol(m_currencyList->currentItem()->text(2));

      //save the changes to MyMoneyFile
      MyMoneyFileTransaction ft;
      try {
        MyMoneyFile::instance()->modifyCurrency(m_currency);
        ft.commit();
      } catch (MyMoneyException *e) {
        qWarning("Updating the currency failed!");
        delete e;
      }
    }
  }
}

void KCurrencyEditDlg::slotSelectCurrency(const QString& id)
{
  QTreeWidgetItemIterator it(m_currencyList);

  while (*it) {
    if ((*it)->text(1) == id) {
      m_currencyList->blockSignals(true);
      slotSelectCurrency(*it);
      m_currencyList->setCurrentItem(*it);
      m_currencyList->scrollToItem(*it);
      m_currencyList->blockSignals(false);
      break;
    }
    ++it;
  }
}

void KCurrencyEditDlg::slotSelectCurrency(QTreeWidgetItem *item)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  QString baseId;
  try {
    baseId = MyMoneyFile::instance()->baseCurrency().id();
  } catch (MyMoneyException *e) {
    delete e;
  }

  m_currencyInEditing = false;

  if (item) {
    try {
      m_currency = file->security(item->text(1));

    } catch (MyMoneyException *e) {
      delete e;
      m_currency = MyMoneySecurity();
    }
    button(KDialog::User1)->setDisabled(m_currency.id() == baseId);
    emit selectObject(m_currency);
  }
}

void KCurrencyEditDlg::slotStartRename(void)
{
  m_currencyInEditing = true;
  QTreeWidgetItemIterator it_l(m_currencyList, QTreeWidgetItemIterator::Selected);
  QTreeWidgetItem* it_v;
  if ((it_v = *it_l) != 0) {
    m_currencyList->editItem(it_v, 0);
  }
}

void KCurrencyEditDlg::slotStartEditId(void)
{
  m_currencyInEditing = true;
  QTreeWidgetItemIterator it_l(m_currencyList, QTreeWidgetItemIterator::Selected);
  QTreeWidgetItem* it_v;
  if ((it_v = *it_l) != 0) {
    m_currencyList->editItem(it_v, 2);
  }
}

void KCurrencyEditDlg::slotOpenContextMenu(const QPoint& p)
{
  QTreeWidgetItem* item = m_currencyList->itemAt(p);
  if (item)
    emit openContextMenu(item->data(0, Qt::UserRole).value<MyMoneySecurity>());
}

void KCurrencyEditDlg::slotRenameCurrency(QTreeWidgetItem* item)
{
  QString currencyId = item->text(1);
  if (currencyId == m_currency.id())
    emit renameCurrency(currencyId, item->text(0));
}

void KCurrencyEditDlg::slotSelectBaseCurrency(void)
{
  if (!m_currency.id().isEmpty()) {
    QTreeWidgetItem* p = m_currencyList->currentItem();
    emit selectBaseCurrency(m_currency);
    // in case the dataChanged() signal was not sent out (nested FileTransaction)
    // we update the list manually
    if (p == m_currencyList->currentItem())
      slotLoadCurrencies();
  }
}

#include "kcurrencyeditdlg.moc"
