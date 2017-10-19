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
#include <QPushButton>
#include <QIcon>
#include <QDialogButtonBox>
#include <QVBoxLayout>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KGuiItem>
#include <KMessageBox>
#include <KLocalizedString>
#include <KTreeWidgetSearchLine>
#include <KTreeWidgetSearchLineWidget>

// ----------------------------------------------------------------------------
// Project Includes

#include "kupdatestockpricedlg.h"
#include "kcurrencycalculator.h"
#include "mymoneyprice.h"
#include "kequitypriceupdatedlg.h"
#include "kmymoneycurrencyselector.h"
#include "mymoneyfile.h"
#include "kmymoneyutils.h"
#include "kpricetreeitem.h"
#include "icons/icons.h"

using namespace Icons;

KMyMoneyPriceDlg::KMyMoneyPriceDlg(QWidget* parent) :
    KMyMoneyPriceDlgDecl(parent)
{
  QVBoxLayout *mainLayout = new QVBoxLayout;
  setLayout(mainLayout);
  mainLayout->addWidget(m_layoutWidget);
  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  mainLayout->addWidget(buttonBox);

  // create the searchline widget
  // and insert it into the existing layout
  m_searchWidget = new KTreeWidgetSearchLineWidget(this, m_priceList);
  m_searchWidget->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
  m_listLayout->insertWidget(0, m_searchWidget);

  m_priceList->header()->setSortIndicator(0, Qt::AscendingOrder);
  m_priceList->header()->setStretchLastSection(true);
  m_priceList->setContextMenuPolicy(Qt::CustomContextMenu);

  KGuiItem removeButtonItem(i18n("&Delete"),
                            QIcon::fromTheme(g_Icons[Icon::EditDelete]),
                            i18n("Delete this entry"),
                            i18n("Remove this price item from the file"));
  KGuiItem::assign(m_deleteButton, removeButtonItem);

  KGuiItem newButtonItem(i18nc("New price entry", "&New"),
                         QIcon::fromTheme(g_Icons[Icon::DocumentNew]),
                         i18n("Add a new entry"),
                         i18n("Create a new price entry."));
  KGuiItem::assign(m_newButton, newButtonItem);

  KGuiItem editButtonItem(i18n("&Edit"),
                          QIcon::fromTheme(g_Icons[Icon::DocumentEdit]),
                          i18n("Modify the selected entry"),
                          i18n("Change the details of selected price information."));
  KGuiItem::assign(m_editButton, editButtonItem);

  m_onlineQuoteButton->setIcon(KMyMoneyUtils::overlayIcon(g_Icons[Icon::ViewInvestment], g_Icons[Icon::Download]));

  connect(m_editButton, SIGNAL(clicked()), this, SLOT(slotEditPrice()));
  connect(m_deleteButton, SIGNAL(clicked()), this, SLOT(slotDeletePrice()));
  connect(m_newButton, SIGNAL(clicked()), this, SLOT(slotNewPrice()));
  connect(m_priceList, SIGNAL(itemSelectionChanged()), this, SLOT(slotSelectPrice()));
  connect(m_onlineQuoteButton, SIGNAL(clicked()), this, SLOT(slotOnlinePriceUpdate()));
  connect(m_priceList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotOpenContextMenu(QPoint)));

  connect(m_showAllPrices, SIGNAL(toggled(bool)), this, SLOT(slotLoadWidgets()));
  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadWidgets()));

  slotLoadWidgets();
  slotSelectPrice();
}

KMyMoneyPriceDlg::~KMyMoneyPriceDlg()
{
}

void KMyMoneyPriceDlg::slotLoadWidgets()
{
  MyMoneyFile* file = MyMoneyFile::instance();

  //clear the list and disable the sorting while it loads the widgets, for performance
  m_priceList->setSortingEnabled(false);
  m_priceList->clear();
  m_stockNameMap.clear();

  //load the currencies for investments, which we'll need later
  QList<MyMoneyAccount> accList;
  file->accountList(accList);
  QList<MyMoneyAccount>::const_iterator acc_it;
  for (acc_it = accList.constBegin(); acc_it != accList.constEnd(); ++acc_it) {
    if ((*acc_it).isInvest()) {
      if (m_stockNameMap.contains((*acc_it).currencyId())) {
        m_stockNameMap[(*acc_it).currencyId()] = QString(m_stockNameMap.value((*acc_it).currencyId()) + ", " + (*acc_it).name());
      } else {
        m_stockNameMap[(*acc_it).currencyId()] = (*acc_it).name();
      }
    }
  }

  //get the price list
  MyMoneyPriceList list = file->priceList();
  MyMoneyPriceList::ConstIterator it_allPrices;
  for (it_allPrices = list.constBegin(); it_allPrices != list.constEnd(); ++it_allPrices) {
    MyMoneyPriceEntries::ConstIterator it_priceItem;
    if (m_showAllPrices->isChecked()) {
      for (it_priceItem = (*it_allPrices).constBegin(); it_priceItem != (*it_allPrices).constEnd(); ++it_priceItem) {
        loadPriceItem(*it_priceItem);
      }
    } else {
      //if it doesn't show all prices, it only shows the most recent occurrence for each price
      if ((*it_allPrices).count() > 0) {
        //the prices for each currency are ordered by date in ascending order
        //it gets the last item of the item, which is supposed to be the most recent price
        it_priceItem = (*it_allPrices).constEnd();
        --it_priceItem;
        loadPriceItem(*it_priceItem);
      }
    }
  }
  //reenable sorting and sort by the commodity column
  m_priceList->setSortingEnabled(true);
  m_priceList->sortByColumn(KPriceTreeItem::ePriceCommodity);

  //update the search widget so the list gets refreshed correctly if it was being filtered
  if (!m_searchWidget->searchLine()->text().isEmpty())
    m_searchWidget->searchLine()->updateSearch(m_searchWidget->searchLine()->text());
}

QTreeWidgetItem* KMyMoneyPriceDlg::loadPriceItem(const MyMoneyPrice& basePrice)
{
  MyMoneySecurity from, to;
  MyMoneyPrice price = MyMoneyPrice(basePrice);

  KPriceTreeItem* priceTreeItem = new KPriceTreeItem(m_priceList);

  if (!price.isValid())
    price = MyMoneyFile::instance()->price(price.from(), price.to(), price.date());

  if (price.isValid()) {
    QString priceBase = price.to();
    from = MyMoneyFile::instance()->security(price.from());
    to = MyMoneyFile::instance()->security(price.to());
    if (!to.isCurrency()) {
      from = MyMoneyFile::instance()->security(price.to());
      to = MyMoneyFile::instance()->security(price.from());
      priceBase = price.from();
    }

    priceTreeItem->setData(KPriceTreeItem::ePriceCommodity, Qt::UserRole, QVariant::fromValue(price));
    priceTreeItem->setText(KPriceTreeItem::ePriceCommodity, (from.isCurrency()) ? from.id() : from.tradingSymbol());
    priceTreeItem->setText(KPriceTreeItem::ePriceStockName, (from.isCurrency()) ? QString() : m_stockNameMap.value(from.id()));
    priceTreeItem->setToolTip(KPriceTreeItem::ePriceStockName, (from.isCurrency()) ? QString() : m_stockNameMap.value(from.id()));
    priceTreeItem->setText(KPriceTreeItem::ePriceCurrency, to.id());
    priceTreeItem->setText(KPriceTreeItem::ePriceDate, QLocale().toString(price.date(), QLocale::ShortFormat));
    priceTreeItem->setData(KPriceTreeItem::ePriceDate, KPriceTreeItem::OrderRole, QVariant(price.date()));
    priceTreeItem->setText(KPriceTreeItem::ePricePrice, price.rate(priceBase).formatMoney("", from.pricePrecision()));
    priceTreeItem->setTextAlignment(KPriceTreeItem::ePricePrice, Qt::AlignRight | Qt::AlignVCenter);
    priceTreeItem->setData(KPriceTreeItem::ePricePrice, KPriceTreeItem::OrderRole, QVariant::fromValue(price.rate(priceBase)));
    priceTreeItem->setText(KPriceTreeItem::ePriceSource, price.source());
  }
  return priceTreeItem;
}

void KMyMoneyPriceDlg::slotSelectPrice()
{
  QTreeWidgetItem* item = 0;
  if (m_priceList->selectedItems().count() > 0) {
    item = m_priceList->selectedItems().at(0);
  }
  m_currentItem = item;
  m_editButton->setEnabled(item != 0);
  bool deleteEnabled = (item != 0);

  //if one of the selected entries is a default, then deleting is disabled
  QList<QTreeWidgetItem*> itemsList = m_priceList->selectedItems();
  QList<QTreeWidgetItem*>::const_iterator item_it;
  for (item_it = itemsList.constBegin(); item_it != itemsList.constEnd(); ++item_it) {
    MyMoneyPrice price = (*item_it)->data(0, Qt::UserRole).value<MyMoneyPrice>();
    if (price.source() == "KMyMoney")
      deleteEnabled = false;
  }
  m_deleteButton->setEnabled(deleteEnabled);

  // Modification of automatically added entries is not allowed
  // Multiple entries cannot be edited at once
  if (item) {
    MyMoneyPrice price = item->data(0, Qt::UserRole).value<MyMoneyPrice>();
    if (price.source() == "KMyMoney" || itemsList.count() > 1)
      m_editButton->setEnabled(false);
    emit selectObject(price);
  }
}

void KMyMoneyPriceDlg::slotNewPrice()
{
  QPointer<KUpdateStockPriceDlg> dlg = new KUpdateStockPriceDlg(this);
  try {
    QTreeWidgetItem* item = m_priceList->currentItem();
    if (item) {
      MyMoneySecurity security;
      security = MyMoneyFile::instance()->security(item->data(0, Qt::UserRole).value<MyMoneyPrice>().from());
      dlg->m_security->setSecurity(security);
      security = MyMoneyFile::instance()->security(item->data(0, Qt::UserRole).value<MyMoneyPrice>().to());
      dlg->m_currency->setSecurity(security);
    }

    if (dlg->exec()) {
      MyMoneyPrice price(dlg->m_security->security().id(), dlg->m_currency->security().id(), dlg->date(), MyMoneyMoney::ONE);
      QTreeWidgetItem* p = loadPriceItem(price);
      m_priceList->setCurrentItem(p, true);
      // If the user cancels the following operation, we delete the new item
      // and re-select any previously selected one
      if (slotEditPrice() == Rejected) {
        delete p;
        if (item)
          m_priceList->setCurrentItem(item, true);
      }
    }
  } catch (...) {
    delete dlg;
    throw;
  }
  delete dlg;
}

int KMyMoneyPriceDlg::slotEditPrice()
{
  int rc = Rejected;
  QTreeWidgetItem* item = m_priceList->currentItem();
  if (item) {
    MyMoneySecurity from(MyMoneyFile::instance()->security(item->data(0, Qt::UserRole).value<MyMoneyPrice>().from()));
    MyMoneySecurity to(MyMoneyFile::instance()->security(item->data(0, Qt::UserRole).value<MyMoneyPrice>().to()));
    signed64 fract = MyMoneyMoney::precToDenom(from.pricePrecision());

    QPointer<KCurrencyCalculator> calc =
      new KCurrencyCalculator(from,
                              to,
                              MyMoneyMoney::ONE,
                              item->data(0, Qt::UserRole).value<MyMoneyPrice>().rate(to.id()),
                              item->data(0, Qt::UserRole).value<MyMoneyPrice>().date(),
                              fract,
                              this);
    calc->setupPriceEditor();

    rc = calc->exec();
    delete calc;
  }
  return rc;
}


void KMyMoneyPriceDlg::slotDeletePrice()
{
  QList<QTreeWidgetItem*> listItems = m_priceList->selectedItems();
  if (listItems.count() > 0) {
    if (KMessageBox::questionYesNo(this, i18np("Do you really want to delete the selected price entry?", "Do you really want to delete the selected price entries?", listItems.count()), i18n("Delete price information"), KStandardGuiItem::yes(), KStandardGuiItem::no(), "DeletePrice") == KMessageBox::Yes) {
      MyMoneyFileTransaction ft;
      try {
        QList<QTreeWidgetItem*>::const_iterator price_it;
        for (price_it = listItems.constBegin(); price_it != listItems.constEnd(); ++price_it) {
          MyMoneyFile::instance()->removePrice((*price_it)->data(0, Qt::UserRole).value<MyMoneyPrice>());
        }
        ft.commit();
      } catch (const MyMoneyException &) {
        qDebug("Cannot delete price");
      }
    }
  }
}

void KMyMoneyPriceDlg::slotOnlinePriceUpdate()
{
  QPointer<KEquityPriceUpdateDlg> dlg = new KEquityPriceUpdateDlg(this);
  if (dlg->exec() == Accepted && dlg)
    dlg->storePrices();
  delete dlg;
}

void KMyMoneyPriceDlg::slotOpenContextMenu(const QPoint& p)
{
  QTreeWidgetItem* item = m_priceList->itemAt(p);
  if (item) {
    m_priceList->setCurrentItem(item, QItemSelectionModel::ClearAndSelect);
    emit openContextMenu(item->data(0, Qt::UserRole).value<MyMoneyPrice>());
  }
}
