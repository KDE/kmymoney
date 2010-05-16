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

enum KCurrencyEditItemRoles {
  IdRole = Qt::UserRole,      /**< The id is stored in this role in column 0 as a string.*/
};

KCurrencyEditDlg::KCurrencyEditDlg(QWidget *parent) :
    KCurrencyEditDlgDecl(parent)
{
  m_currencyList->header()->setStretchLastSection(true);

  connect(m_currencyList, SIGNAL(itemPressed(QTreeWidgetItem*, int)), this, SLOT(slotItemPressed(QTreeWidgetItem*, int)));
  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadCurrencies()));

  slotLoadCurrencies();

  connect(m_baseCurrencyButton, SIGNAL(clicked()), this, SLOT(slotSelectBaseCurrency()));
  connect(buttonClose, SIGNAL(clicked()), this, SLOT(slotClose()));

  // FIXME: currently, no online help available
  buttonHelp->hide();

  QTimer::singleShot(10, this, SLOT(timerDone()));
}

void KCurrencyEditDlg::timerDone(void)
{
  if (!m_currency.id().isEmpty()) {
    QTreeWidgetItemIterator it(m_currencyList);
    QTreeWidgetItem* q;
    while ((q = *it) != 0) {
      if (q->data(0, IdRole).toString() == m_currency.id()) {
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

  QString baseCurrency = MyMoneyFile::instance()->baseCurrency().id();
  // construct a transparent 16x16 pixmap
  QPixmap empty(16, 16);
  QBitmap mask(16, 16);
  mask.clear();
  empty.setMask(mask);

  m_currencyList->clear();
  for (it = list.constBegin(); it != list.constEnd(); ++it) {
    QTreeWidgetItem *p = new QTreeWidgetItem(m_currencyList);
    p->setText(0, (*it).name());
    p->setData(0, IdRole, (*it).id());
    p->setFlags(p->flags() | Qt::ItemIsEditable);

    if ((*it).id() == baseCurrency) {
      p->setData(0, Qt::DecorationRole, KIcon("kmymoney"));
      if (m_currency.id().isEmpty())
        first = p;
    } else {
      p->setData(0, Qt::DecorationRole, empty);
    }

    // if we had a previously selected
    if (!m_currency.id().isEmpty()) {
      if (m_currency.id() == p->data(0, IdRole).toString())
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

void KCurrencyEditDlg::updateCurrency(void)
{
  if (!m_currency.id().isEmpty()) {
    if (m_symbolEdit->text() != m_currency.tradingSymbol()) {
      m_currency.setTradingSymbol(m_symbolEdit->text());
      MyMoneyFileTransaction ft;
      try {
        MyMoneyFile::instance()->modifyCurrency(m_currency);
        ft.commit();
      } catch (MyMoneyException *e) {
        qWarning("Updateing the currency failed!");
        delete e;
      }
    }
  }
}

void KCurrencyEditDlg::slotSelectCurrency(const QString& id)
{
  QTreeWidgetItemIterator it(m_currencyList);

  while (*it) {
    if ((*it)->data(0, IdRole).toString() == id) {
      slotSelectCurrency(*it);
      m_currencyList->setCurrentItem(*it);
      m_currencyList->scrollToItem(*it);
      break;
    }
    ++it;
  }
}

void KCurrencyEditDlg::slotSelectCurrency(QTreeWidgetItem *item)
{
  QMap<QDate, MyMoneyMoney> history;
  MyMoneyFile* file = MyMoneyFile::instance();

  updateCurrency();

  m_detailGroup->setEnabled(item != 0);
  m_idLabel->setText(QString());
  m_symbolEdit->setText(QString());

  if (item) {
    try {
      m_currency = file->security(item->data(0, IdRole).toString());
      m_idLabel->setText(m_currency.id());
      m_symbolEdit->setText(m_currency.tradingSymbol());

    } catch (MyMoneyException *e) {
      delete e;
      m_currency = MyMoneySecurity();
      m_idLabel->setText(QString());
      m_symbolEdit->setText(QString());
    }
    m_baseCurrencyButton->setDisabled(m_currency.id() == file->baseCurrency().id());
    emit selectObject(m_currency);
  }
}

void KCurrencyEditDlg::slotClose(void)
{
  updateCurrency();
  accept();
}

void KCurrencyEditDlg::slotStartRename(void)
{
  QTreeWidgetItemIterator it_l(m_currencyList, QTreeWidgetItemIterator::Selected);
  QTreeWidgetItem* it_v;
  if ((it_v = *it_l) != 0) {
    m_currencyList->editItem(it_v);
  }
}

void KCurrencyEditDlg::slotItemPressed(QTreeWidgetItem* item, int)
{
  if (QApplication::mouseButtons() != Qt::RightButton)
    return;

  slotSelectCurrency(item);
  emit openContextMenu(m_currency);
}

void KCurrencyEditDlg::slotRenameCurrency(QTreeWidgetItem* item)
{
  QString currencyId = item->data(0, IdRole).toString();
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
