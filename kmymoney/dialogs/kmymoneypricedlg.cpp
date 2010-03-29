/***************************************************************************
                          kmymoneypricedlg.cpp
                             -------------------
    begin                : Wed Nov 24 2004
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

#include "kmymoneypricedlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QCheckBox>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kpushbutton.h>
#include <kiconloader.h>
#include <kguiitem.h>
#include <kmessagebox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kupdatestockpricedlg.h"
#include "kcurrencycalculator.h"
#include "kmymoneypriceview.h"
#include "kequitypriceupdatedlg.h"
#include <kmymoneycurrencyselector.h>
#include <mymoneyfile.h>
#include "kmymoneyglobalsettings.h"

#define COMMODITY_COL     0
#define CURRENCY_COL      1
#define DATE_COL          2
#define PRICE_COL         3
#define SOURCE_COL        4

KMyMoneyPriceDlg::KMyMoneyPriceDlg(QWidget* parent) :
    KMyMoneyPriceDlgDecl(parent)
{
  KGuiItem removeButtenItem(i18n("&Delete"),
                            KIcon("edit-delete"),
                            i18n("Delete this entry"),
                            i18n("Remove this price item from the file"));
  m_deleteButton->setGuiItem(removeButtenItem);

  KGuiItem newButtenItem(i18nc("New price entry", "&New"),
                         KIcon("document-new"),
                         i18n("Add a new entry"),
                         i18n("Create a new price entry."));
  m_newButton->setGuiItem(newButtenItem);

  KGuiItem editButtenItem(i18n("&Edit"),
                          KIcon("document-edit"),
                          i18n("Modify the selected entry"),
                          i18n("Change the details of selected price information."));
  m_editButton->setGuiItem(editButtenItem);

  KGuiItem okButtenItem(i18n("&Close"),
                        KIcon("dialog-ok"),
                        i18n("Close the dialog"),
                        i18n("Use this to close the dialog and return to the application."));
  m_closeButton->setGuiItem(okButtenItem);

  connect(m_closeButton, SIGNAL(clicked()), this, SLOT(accept()));
  connect(m_editButton, SIGNAL(clicked()), this, SLOT(slotEditPrice()));
  connect(m_priceList, SIGNAL(editPrice()), this, SLOT(slotEditPrice()));
  connect(m_deleteButton, SIGNAL(clicked()), this, SLOT(slotDeletePrice()));
  connect(m_priceList, SIGNAL(deletePrice()), this, SLOT(slotDeletePrice()));
  connect(m_newButton, SIGNAL(clicked()), this, SLOT(slotNewPrice()));
  connect(m_priceList, SIGNAL(newPrice()), this, SLOT(slotNewPrice()));
  connect(m_priceList, SIGNAL(selectionChanged(Q3ListViewItem*)), this, SLOT(slotSelectPrice(Q3ListViewItem*)));
  connect(m_onlineQuoteButton, SIGNAL(clicked()), this, SLOT(slotOnlinePriceUpdate()));
  connect(m_priceList, SIGNAL(onlinePriceUpdate()), this, SLOT(slotOnlinePriceUpdate()));

  connect(m_showAllPrices, SIGNAL(toggled(bool)), this, SLOT(slotLoadWidgets()));
  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadWidgets()));

  slotLoadWidgets();
  slotSelectPrice(0);

  // FIXME: for now, we don't have the logic to delete all prices in a given date range
  m_deleteRangeButton->setEnabled(false);
}

KMyMoneyPriceDlg::~KMyMoneyPriceDlg()
{
}

void KMyMoneyPriceDlg::slotLoadWidgets(void)
{
  m_priceList->clear();

  MyMoneyPriceList list = MyMoneyFile::instance()->priceList();
  MyMoneyPriceList::ConstIterator it_l;
  for (it_l = list.constBegin(); it_l != list.constEnd(); ++it_l) {
    MyMoneyPriceEntries::ConstIterator it_e;
    if (m_showAllPrices->isChecked()) {
      for (it_e = (*it_l).constBegin(); it_e != (*it_l).constEnd(); ++it_e) {
        new KMyMoneyPriceItem(m_priceList, *it_e);
      }
    } else {
      if ((*it_l).count() > 0) {
        it_e = (*it_l).end();
        --it_e;
        new KMyMoneyPriceItem(m_priceList, *it_e);
      }
    }
  }
}

void KMyMoneyPriceDlg::slotSelectPrice(Q3ListViewItem * item)
{
  m_currentItem = item;
  m_editButton->setEnabled(item != 0);
  m_deleteButton->setEnabled(item != 0);

  // Modification of automatically added entries is not allowed
  if (item) {
    KMyMoneyPriceItem* priceitem = dynamic_cast<KMyMoneyPriceItem*>(item);
    if (priceitem && (priceitem->price().source() == "KMyMoney")) {
      m_editButton->setEnabled(false);
      m_deleteButton->setEnabled(false);
    }
  }
}

void KMyMoneyPriceDlg::slotNewPrice(void)
{
  QPointer<KUpdateStockPriceDlg> dlg = new KUpdateStockPriceDlg(this);
  try {
    KMyMoneyPriceItem* item = dynamic_cast<KMyMoneyPriceItem*>(m_priceList->selectedItem());
    if (item) {
      MyMoneySecurity security;
      security = MyMoneyFile::instance()->security(item->price().from());
      dlg->m_security->setSecurity(security);
      security = MyMoneyFile::instance()->security(item->price().to());
      dlg->m_currency->setSecurity(security);
    }

    if (dlg->exec()) {
      MyMoneyPrice price(dlg->m_security->security().id(), dlg->m_currency->security().id(), dlg->date(), MyMoneyMoney(1, 1));
      KMyMoneyPriceItem* p = new KMyMoneyPriceItem(m_priceList, price);
      m_priceList->setSelected(p, true);
      // If the user cancels the following operation, we delete the new item
      // and re-select any previously selected one
      if (slotEditPrice() == Rejected) {
        delete p;
        if (item)
          m_priceList->setSelected(item, true);
      }
    }
  } catch (...) {
    delete dlg;
    throw;
  }
  delete dlg;
}

int KMyMoneyPriceDlg::slotEditPrice(void)
{
  int rc = Rejected;
  KMyMoneyPriceItem* item = dynamic_cast<KMyMoneyPriceItem*>(m_priceList->selectedItem());
  if (item) {
    MyMoneySecurity from(MyMoneyFile::instance()->security(item->price().from()));
    MyMoneySecurity to(MyMoneyFile::instance()->security(item->price().to()));
    signed64 fract = MyMoneyMoney::precToDenom(KMyMoneyGlobalSettings::pricePrecision());

    QPointer<KCurrencyCalculator> calc =
      new KCurrencyCalculator(from,
                              to,
                              MyMoneyMoney(1, 1),
                              item->price().rate(to.id()),
                              item->price().date(),
                              fract,
                              this);
    calc->setupPriceEditor();

    rc = calc->exec();
    delete calc;
  }
  return rc;
}


void KMyMoneyPriceDlg::slotDeletePrice(void)
{
  KMyMoneyPriceItem* item = dynamic_cast<KMyMoneyPriceItem*>(m_priceList->selectedItem());
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

void KMyMoneyPriceDlg::slotOnlinePriceUpdate(void)
{
  KMyMoneyPriceItem* item = dynamic_cast<KMyMoneyPriceItem*>(m_priceList->selectedItem());
  if (item) {
    QPointer<KEquityPriceUpdateDlg> dlg = new KEquityPriceUpdateDlg(this, (item->text(COMMODITY_COL) + ' ' + item->text(CURRENCY_COL)).toUtf8());
    if (dlg->exec() == Accepted)
      dlg->storePrices();
    delete dlg;
  } else {
    QPointer<KEquityPriceUpdateDlg> dlg = new KEquityPriceUpdateDlg(this);
    if (dlg->exec() == Accepted)
      dlg->storePrices();
    delete dlg;
  }
}

#if 0
// This function is not needed.  However, removing the KUpdateStockPriceDlg
// instantiation below causes link failures:

// This seems to be fixed, so I #if 0'ed it out. Let's see, if someone
// complains and if not, we get rid of this whole block one day. (2007-06-22 ipwizard)
//
// kmymonekmymoneypriceview.cpp:179: undefined reference to
// `KUpdateStockPriceDlg::KUpdateStockPriceDlg[in-charge](QWidget*, char const*)'
// kmymonekmymoneypriceview.cpp:204: undefined reference to
// `KUpdateStockPriceDlg::KUpdateStockPriceDlg[in-charge](QDate const&, QString const&, QWidget*, char const*)'
void KEditEquityEntryDlg_useless(void)
{
  delete new KUpdateStockPriceDlg();
}
#endif

// Make sure, that these definitions are only used within this file
// this does not seem to be necessary, but when building RPMs the
// build option 'final' is used and all CPP files are concatenated.
// So it could well be, that in another CPP file these definitions
// are also used.
#undef COMMODITY_COL
#undef CURRENCY_COL
#undef DATE_COL
#undef PRICE_COL
#undef SOURCE_COL


#include "kmymoneypricedlg.moc"
