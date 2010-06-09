/***************************************************************************
                          kmymoneypriceview.cpp  -  description
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

#include "kmymoneypriceview.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <Q3Header>
#include <QCursor>
#include <QTimer>
#include <QCheckBox>
#include <Q3GroupBox>
#include <QResizeEvent>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kglobal.h>
#include <k3listview.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <kmessagebox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneysecurity.h>
#include <mymoneyfile.h>
#include "kmymoneyglobalsettings.h"

#define COMMODITY_COL   0
#define CURRENCY_COL    1
#define DATE_COL        2
#define PRICE_COL       3
#define SOURCE_COL      4

KMyMoneyPriceItem::KMyMoneyPriceItem(K3ListView *view, const MyMoneyPrice& pr) :
    KMyMoneyListViewItem(view, QString(), QString(), QString()),
    m_pr(pr)
{
  MyMoneySecurity from, to;
  KSharedConfigPtr kconfig = KGlobal::config();
  KConfigGroup grp =  kconfig->group("General Options");
  int prec = grp.readEntry("PricePrecision", 4);

  if (!m_pr.isValid())
    m_pr = MyMoneyFile::instance()->price(m_pr.from(), m_pr.to(), m_pr.date());

  if (m_pr.isValid()) {
    QString priceBase = m_pr.to();
    from = MyMoneyFile::instance()->security(m_pr.from());
    to = MyMoneyFile::instance()->security(m_pr.to());
    if (!to.isCurrency()) {
      from = MyMoneyFile::instance()->security(m_pr.to());
      to = MyMoneyFile::instance()->security(m_pr.from());
      priceBase = m_pr.from();
    }

    setText(COMMODITY_COL, (from.isCurrency()) ? from.id() : from.tradingSymbol());
    setText(CURRENCY_COL, to.id());
    setText(DATE_COL, KGlobal::locale()->formatDate(m_pr.date(), KLocale::ShortDate));
    setText(PRICE_COL, m_pr.rate(priceBase).formatMoney("", prec));
    setText(SOURCE_COL, m_pr.source());
  }
}

int KMyMoneyPriceItem::compare(Q3ListViewItem* i, int col, bool ascending) const
{
  KMyMoneyPriceItem* item = static_cast<KMyMoneyPriceItem*>(i);
  int rc = 0;

  switch (col) {
    case DATE_COL:   // date
      if (m_pr.date() > item->m_pr.date())
        rc = 1;
      else if (m_pr.date() < item->m_pr.date())
        rc = -1;
      break;

    case PRICE_COL:   // value
      if (m_pr.rate(QString()) > item->m_pr.rate(QString()))
        rc = 1;
      else if (m_pr.rate(QString()) < item->m_pr.rate(QString()))
        rc = -1;
      break;

    default:
      rc = Q3ListViewItem::compare(i, col, ascending);
      break;
  }
  return rc;
}

KMyMoneyPriceView::KMyMoneyPriceView(QWidget *parent) :
    K3ListView(parent),
    m_showAll(false)
{
  addColumn(i18n("Commodity"));
  addColumn(i18n("Currency"));
  addColumn(i18n("Date"));
  addColumn(i18n("Price"));
  addColumn(i18nc("Price source", "Source"));
  setAllColumnsShowFocus(true);
  setMultiSelection(false);
  setColumnWidthMode(0, Q3ListView::Maximum);
  setColumnWidthMode(1, Q3ListView::Maximum);
  setShowSortIndicator(true);
  setSorting(COMMODITY_COL);

  header()->setFont(KMyMoneyGlobalSettings::listHeaderFont());
  header()->setResizeEnabled(false);

  connect(this, SIGNAL(rightButtonClicked(Q3ListViewItem* , const QPoint&, int)),
          this, SLOT(slotListClicked(Q3ListViewItem*, const QPoint&, int)));

  // connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotReloadWidget()));

  // slotReloadWidget();

  // If the widget is shown, the size must be fixed a little later
  // to be appropriate. I saw this in some other places and the only
  // way to solve this problem is to postpone the setup of the size
  // to the time when the widget is on the screen.
  resize(width() - 1, height() - 1);
  QTimer::singleShot(50, this, SLOT(slotTimerDone()));
}

KMyMoneyPriceView::~KMyMoneyPriceView()
{
}

void KMyMoneyPriceView::slotTimerDone(void)
{
  // the resize operation does the trick to adjust
  // all widgets in the view to the size they should
  // have and show up correctly. Don't ask me, why
  // this is, but it cured the problem (ipwizard).
  resize(width() + 1, height() + 1);
}

#if 0
void KMyMoneyPriceView::slotReloadWidget(void)
{
  m_priceHistory->clear();

  MyMoneyPriceList list = MyMoneyFile::instance()->priceList();
  MyMoneyPriceList::ConstIterator it_l;
  for (it_l = list.begin(); it_l != list.end(); ++it_l) {
    MyMoneyPriceEntries::ConstIterator it_e;
    if (m_showAll) {
      for (it_e = (*it_l).begin(); it_e != (*it_l).end(); ++it_e) {
        new kMyMoneyPriceItem(m_priceHistory, *it_e);
      }
    } else {
      if ((*it_l).count() > 0) {
        it_e = (*it_l).end();
        --it_e;
        new kMyMoneyPriceItem(m_priceHistory, *it_e);
      }
    }
  }
}
#endif

void KMyMoneyPriceView::resizeEvent(QResizeEvent* e)
{
  int w = visibleWidth() / 5;

  setColumnWidth(0, w);
  setColumnWidth(1, w);
  setColumnWidth(2, w);
  setColumnWidth(3, w);
  setColumnWidth(4, w);
  resizeContents(visibleWidth(), contentsHeight());

  K3ListView::resizeEvent(e);
}

void KMyMoneyPriceView::slotListClicked(Q3ListViewItem* item, const QPoint&, int)
{
  KMyMoneyPriceItem* priceItem = dynamic_cast<KMyMoneyPriceItem*>(item);
  emit openContextMenu(priceItem->price());
}

#if 0
void KMyMoneyPriceView::slotNewPrice(void)
{
  KUpdateStockPriceDlg dlg(this);
  kMyMoneyPriceItem* item = dynamic_cast<kMyMoneyPriceItem*>(m_priceHistory->selectedItem());
  if (item) {
    MyMoneySecurity security;
    security = MyMoneyFile::instance()->security(item->price().from());
    dlg.m_security->setSecurity(security);
    security = MyMoneyFile::instance()->security(item->price().to());
    dlg.m_currency->setSecurity(security);
  }
  if (dlg.exec()) {
    MyMoneyPrice price(dlg.m_security->security().id(), dlg.m_currency->security().id(), dlg.date(), MyMoneyMoney(1, 1));
    kMyMoneyPriceItem* p = new kMyMoneyPriceItem(m_priceHistory, price);
    m_priceHistory->setSelected(p, true);
    // If the user cancels the following operation, we delete the new item
    // and re-select any previously selected one
    if (slotEditPrice() == QDialog::Rejected) {
      delete p;
      if (item)
        m_priceHistory->setSelected(item, true);
    }
  }
}

int KMyMoneyPriceView::slotEditPrice(void)
{
  int rc = QDialog::Rejected;
  kMyMoneyPriceItem* item = dynamic_cast<kMyMoneyPriceItem*>(m_priceHistory->selectedItem());
  if (item) {
    MyMoneySecurity from(MyMoneyFile::instance()->security(item->price().from()));
    MyMoneySecurity to(MyMoneyFile::instance()->security(item->price().to()));
    signed64 fract = MyMoneyMoney::precToDenom(KMyMoneyGlobalSettings::pricePrecision());

    KCurrencyCalculator calc(from,
                             to,
                             MyMoneyMoney(1, 1),
                             item->price().rate(),
                             item->price().date(),
                             fract,
                             this, "currencyCalculator");
    // we always want to update the price, that's why we're here
    calc.m_updateButton->setChecked(true);
    calc.m_updateButton->hide();

    rc = calc.exec();
  }
  return rc;
}

void KMyMoneyPriceView::slotDeletePrice(void)
{
  kMyMoneyPriceItem* item = dynamic_cast<kMyMoneyPriceItem*>(m_priceHistory->selectedItem());
  if (item) {
    if (KMessageBox::questionYesNo(this, i18n("Do you really want to delete the selected price entry?"), i18n("Delete price information"), KStandardGuiItem::yes(), KStandardGuiItem::no(), "DeletePrice") == KMessageBox::Yes) {
      MyMoneyFileTransaction ft;
      try {
        MyMoneyFile::instance()->removePrice(item->price());
        ft.commit();
      } catch (MyMoneyException *e) {
        qDebug("Cannot delete price");
        delete e;
      }
    }
  }
}

void KMyMoneyPriceView::slotShowAllPrices(bool enabled)
{
  if (m_showAll != enabled) {
    m_showAll = enabled;
    slotReloadWidget();
  }
}

void KMyMoneyPriceView::slotOnlinePriceUpdate(void)
{
  KMyMoneyPriceItem* item = dynamic_cast<KMyMoneyPriceItem*>(m_priceHistory->selectedItem());
  if (item) {
    KEquityPriceUpdateDlg dlg(this, (item->text(COMMODITY_COL) + " " + item->text(CURRENCY_COL)).utf8());
    if (dlg.exec() == QDialog::Accepted)
      dlg.storePrices();
  }
}

#endif

