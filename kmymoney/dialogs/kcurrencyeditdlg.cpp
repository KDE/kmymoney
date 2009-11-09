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

#include <locale.h>

// ----------------------------------------------------------------------------
// QT Includes

#include <q3header.h>
#include <QTimer>

#include <QPixmap>
#include <QBitmap>
#include <QLabel>
#include <q3groupbox.h>
#include <QResizeEvent>
#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kpushbutton.h>
#include <k3listview.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kinputdialog.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kcurrencyeditdlg.h"

#include <mymoneysecurity.h>
#include <mymoneyfile.h>
#include <kmymoneylistviewitem.h>
#include <kmymoneyaccountselector.h>
#include <kmymoneylineedit.h>

#include "kmymoneypriceview.h"

KCurrencyEditDlg::KCurrencyEditDlg(QWidget *parent) :
  KCurrencyEditDlgDecl(parent)
{
  m_currencyList->addColumn(i18n("Currency"));
  m_currencyList->header()->hide();

  // FIXME: the online source table currently has no functionality
  m_onlineSourceTable->hide();

  connect(m_currencyList, SIGNAL(rightButtonPressed(Q3ListViewItem* , const QPoint&, int)),
          this, SLOT(slotListClicked(Q3ListViewItem*, const QPoint&, int)));
  connect(m_currencyList, SIGNAL(selectionChanged(Q3ListViewItem*)), this, SLOT(slotSelectCurrency(Q3ListViewItem*)));


  connect(m_currencyList, SIGNAL(itemRenamed(Q3ListViewItem*,int,const QString&)), this, SIGNAL(renameCurrency(Q3ListViewItem*,int,const QString&)));
  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadCurrencies()));

  slotLoadCurrencies();

  connect(m_baseCurrencyButton, SIGNAL(clicked()), this, SLOT(slotSelectBaseCurrency()));
  connect(buttonClose, SIGNAL(clicked()), this, SLOT(slotClose()));

  // FIXME: currently, no online help available
  buttonHelp->hide();

  // FIXME this is currently unused so we hide it also
  m_description->hide();

  resize(width()-1, height()-1);
  QTimer::singleShot(10, this, SLOT(timerDone()));
}

void KCurrencyEditDlg::timerDone(void)
{
  if(!m_currency.id().isEmpty()) {
    Q3ListViewItemIterator it(m_currencyList);
    Q3ListViewItem* q;
    while((q = it.current()) != 0) {
      KMyMoneyListViewItem* p = static_cast<KMyMoneyListViewItem *>(q);
      if(p->id() == m_currency.id()) {
        m_currencyList->ensureItemVisible(q);
        break;
      }
      ++it;
    }
  }
  // the resize operation does the trick to adjust
  // all widgets in the view to the size they should
  // have and show up correctly. Don't ask me, why
  // this is, but it cured the problem (ipwizard).
  resize(width()+1, height()+1);
}

KCurrencyEditDlg::~KCurrencyEditDlg()
{
}

void KCurrencyEditDlg::resizeEvent(QResizeEvent* /* e*/)
{
  int w = m_currencyList->visibleWidth();

  m_currencyList->setColumnWidth(0, w);
}

void KCurrencyEditDlg::slotLoadCurrencies(void)
{
  QList<MyMoneySecurity> list = MyMoneyFile::instance()->currencyList();
  QList<MyMoneySecurity>::ConstIterator it;
  Q3ListViewItem *first = 0;

  QString localCurrency(localeconv()->int_curr_symbol);
  localCurrency.truncate(3);

  QString baseCurrency = MyMoneyFile::instance()->baseCurrency().id();
  // construct a transparent 16x16 pixmap
  QPixmap empty(16, 16);
  empty.setMask(QBitmap(16, 16, true));

  m_currencyList->clear();
  for(it = list.constBegin(); it != list.constEnd(); ++it) {
    KMyMoneyListViewItem* p = new KMyMoneyListViewItem(m_currencyList, (*it).name(), QString(), (*it).id());
    p->setRenameEnabled(0, true);

    if((*it).id() == baseCurrency) {
        p->setPixmap(0, QPixmap( KStandardDirs::locate("icon","hicolor/16x16/apps/kmymoney.png")));
      if(m_currency.id().isEmpty())
        first = p;
    } else {
      p->setPixmap(0, empty);
    }

    // if we had a previously selected
    if(!m_currency.id().isEmpty()) {
      if(m_currency.id() == p->id())
        first = p;
    } else if ((*it).id() == localCurrency && !first)
      first = p;
  }

  if(first == 0)
    first = m_currencyList->firstChild();
  if(first != 0) {
    m_currencyList->setSelected(first, true);
    m_currencyList->ensureItemVisible(first);
  }

  slotSelectCurrency(first);
}

void KCurrencyEditDlg::updateCurrency(void)
{
  if(!m_currency.id().isEmpty()) {
    if(m_symbolEdit->text() != m_currency.tradingSymbol()) {
      m_currency.setTradingSymbol(m_symbolEdit->text());
      MyMoneyFileTransaction ft;
      try {
        MyMoneyFile::instance()->modifyCurrency(m_currency);
        ft.commit();
      } catch(MyMoneyException *e) {
        qWarning("Updateing the currency failed!");
        delete e;
      }
    }
  }
}

void KCurrencyEditDlg::slotSelectCurrency(const QString& id)
{
  Q3ListViewItemIterator it(m_currencyList);

  while(it.current()) {
    KMyMoneyListViewItem* p = static_cast<KMyMoneyListViewItem*>(it.current());
    if(p->id() == id) {
      slotSelectCurrency(p);
      m_currencyList->setSelected(p, true);
      m_currencyList->ensureItemVisible(p);
      break;
    }
    ++it;
  }
}

void KCurrencyEditDlg::slotSelectCurrency(Q3ListViewItem *item)
{
  QMap<QDate, MyMoneyMoney> history;
  MyMoneyFile* file = MyMoneyFile::instance();

  updateCurrency();

  m_detailGroup->setEnabled(item != 0);
  m_onlineSourceTable->clear();
  m_idLabel->setText(QString());
  m_symbolEdit->setText(QString());

  if(item) {
    try {
      KMyMoneyListViewItem* p = static_cast<KMyMoneyListViewItem *>(item);
      m_currency = file->security(p->id());
      m_idLabel->setText(m_currency.id());
      m_symbolEdit->setText(m_currency.tradingSymbol());

    } catch(MyMoneyException *e) {
      delete e;
      m_currency = MyMoneySecurity();
      m_onlineSourceTable->clear();
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
  Q3ListViewItemIterator it_l(m_currencyList, Q3ListViewItemIterator::Selected);
  Q3ListViewItem* it_v;
  if((it_v = it_l.current()) != 0) {
    it_v->startRename(0);
  }
}

void KCurrencyEditDlg::slotListClicked(Q3ListViewItem* item, const QPoint&, int)
{
  slotSelectCurrency(item);
  emit openContextMenu(m_currency);
}

void KCurrencyEditDlg::slotRenameCurrency(Q3ListViewItem* item, int /* col */, const QString& txt)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  KMyMoneyListViewItem* p = static_cast<KMyMoneyListViewItem *>(item);

  try {
    if(txt != m_currency.name()) {
      qDebug("Renaming");
      MyMoneySecurity currency = file->currency(p->id());
      currency.setName(txt);
      MyMoneyFileTransaction ft;
      try {
        file->modifyCurrency(currency);
        m_currency = currency;
        ft.commit();
      } catch(MyMoneyException* e) {
        qDebug("Renaming currency failed");
        delete e;
      }
    }
  } catch(MyMoneyException *e) {
    delete e;
    updateCurrency();
  }
}

void KCurrencyEditDlg::slotSelectBaseCurrency(void)
{
  if(!m_currency.id().isEmpty()) {
    Q3ListViewItem* p = m_currencyList->selectedItem();
    emit selectBaseCurrency(m_currency);
    // in case the dataChanged() signal was not sent out (nested FileTransaction)
    // we update the list manually
    if(p == m_currencyList->selectedItem())
      slotLoadCurrencies();
  }
}

#include "kcurrencyeditdlg.moc"
