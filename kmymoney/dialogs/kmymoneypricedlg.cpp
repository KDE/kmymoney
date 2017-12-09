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

#include "kmymoneypricedlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QCheckBox>
#include <QPushButton>
#include <QIcon>
#include <QVBoxLayout>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>
#include <KLocalizedString>
#include <KTreeWidgetSearchLine>
#include <KTreeWidgetSearchLineWidget>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kmymoneypricedlg.h"
#include "ui_kupdatestockpricedlg.h"

#include "kupdatestockpricedlg.h"
#include "kcurrencycalculator.h"
#include "mymoneyprice.h"
#include "kequitypriceupdatedlg.h"
#include "kmymoneycurrencyselector.h"
#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneysecurity.h"
#include "mymoneymoney.h"
#include "mymoneyexception.h"
#include "kmymoneyutils.h"
#include "kpricetreeitem.h"
#include "icons/icons.h"

using namespace Icons;

class KMyMoneyPriceDlgPrivate
{
  Q_DISABLE_COPY(KMyMoneyPriceDlgPrivate)
  Q_DECLARE_PUBLIC(KMyMoneyPriceDlg)

public:
  explicit KMyMoneyPriceDlgPrivate(KMyMoneyPriceDlg *qq) :
    q_ptr(qq),
    ui(new Ui::KMyMoneyPriceDlg),
    m_searchWidget(nullptr)
  {
  }

  ~KMyMoneyPriceDlgPrivate()
  {
    delete ui;
  }

  KMyMoneyPriceDlg      *q_ptr;
  Ui::KMyMoneyPriceDlg  *ui;
  QTreeWidgetItem*       m_currentItem;
  /**
    * Search widget for the list
    */
  KTreeWidgetSearchLineWidget*  m_searchWidget;
  QMap<QString, QString>        m_stockNameMap;
};

KMyMoneyPriceDlg::KMyMoneyPriceDlg(QWidget* parent) :
  QDialog(parent),
  d_ptr(new KMyMoneyPriceDlgPrivate(this))
{
  Q_D(KMyMoneyPriceDlg);
  d->ui->setupUi(this);

  // create the searchline widget
  // and insert it into the existing layout
  d->m_searchWidget = new KTreeWidgetSearchLineWidget(this, d->ui->m_priceList);
  d->m_searchWidget->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));

  d->ui->m_listLayout->insertWidget(0, d->m_searchWidget);

  d->ui->m_priceList->header()->setSortIndicator(0, Qt::AscendingOrder);
  d->ui->m_priceList->header()->setStretchLastSection(true);
  d->ui->m_priceList->setContextMenuPolicy(Qt::CustomContextMenu);

  d->ui->m_deleteButton->setIcon(Icons::get(Icon::EditDelete));
  d->ui->m_newButton->setIcon(Icons::get(Icon::DocumentNew));
  d->ui->m_editButton->setIcon(Icons::get(Icon::DocumentEdit));

  d->ui->m_onlineQuoteButton->setIcon(Icons::get(Icon::ToolUpdatePrices));

  connect(d->ui->m_editButton, &QAbstractButton::clicked, this, &KMyMoneyPriceDlg::slotEditPrice);
  connect(d->ui->m_deleteButton, &QAbstractButton::clicked, this, &KMyMoneyPriceDlg::slotDeletePrice);
  connect(d->ui->m_newButton, &QAbstractButton::clicked, this, &KMyMoneyPriceDlg::slotNewPrice);
  connect(d->ui->m_priceList, &QTreeWidget::itemSelectionChanged, this, &KMyMoneyPriceDlg::slotSelectPrice);
  connect(d->ui->m_onlineQuoteButton, &QAbstractButton::clicked, this, &KMyMoneyPriceDlg::slotOnlinePriceUpdate);
  connect(d->ui->m_priceList, &QWidget::customContextMenuRequested, this, &KMyMoneyPriceDlg::slotOpenContextMenu);

  connect(d->ui->m_showAllPrices, &QAbstractButton::toggled, this, &KMyMoneyPriceDlg::slotLoadWidgets);
  connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, this, &KMyMoneyPriceDlg::slotLoadWidgets);

  slotLoadWidgets();
  slotSelectPrice();
}

KMyMoneyPriceDlg::~KMyMoneyPriceDlg()
{
  Q_D(KMyMoneyPriceDlg);
  delete d;
}

void KMyMoneyPriceDlg::slotLoadWidgets()
{
  Q_D(KMyMoneyPriceDlg);
  auto file = MyMoneyFile::instance();

  //clear the list and disable the sorting while it loads the widgets, for performance
  d->ui->m_priceList->setSortingEnabled(false);
  d->ui->m_priceList->clear();
  d->m_stockNameMap.clear();

  //load the currencies for investments, which we'll need later
  QList<MyMoneyAccount> accList;
  file->accountList(accList);
  QList<MyMoneyAccount>::const_iterator acc_it;
  for (acc_it = accList.constBegin(); acc_it != accList.constEnd(); ++acc_it) {
    if ((*acc_it).isInvest()) {
      if (d->m_stockNameMap.contains((*acc_it).currencyId())) {
        d->m_stockNameMap[(*acc_it).currencyId()] = QString(d->m_stockNameMap.value((*acc_it).currencyId()) + ", " + (*acc_it).name());
      } else {
        d->m_stockNameMap[(*acc_it).currencyId()] = (*acc_it).name();
      }
    }
  }

  //get the price list
  MyMoneyPriceList list = file->priceList();
  MyMoneyPriceList::ConstIterator it_allPrices;
  for (it_allPrices = list.constBegin(); it_allPrices != list.constEnd(); ++it_allPrices) {
    MyMoneyPriceEntries::ConstIterator it_priceItem;
    if (d->ui->m_showAllPrices->isChecked()) {
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
  d->ui->m_priceList->setSortingEnabled(true);
  d->ui->m_priceList->sortByColumn(KPriceTreeItem::ePriceCommodity);

  //update the search widget so the list gets refreshed correctly if it was being filtered
  if (!d->m_searchWidget->searchLine()->text().isEmpty())
    d->m_searchWidget->searchLine()->updateSearch(d->m_searchWidget->searchLine()->text());
}

QTreeWidgetItem* KMyMoneyPriceDlg::loadPriceItem(const MyMoneyPrice& basePrice)
{
  Q_D(KMyMoneyPriceDlg);
  MyMoneySecurity from, to;
  auto price = MyMoneyPrice(basePrice);

  auto priceTreeItem = new KPriceTreeItem(d->ui->m_priceList);

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
    priceTreeItem->setText(KPriceTreeItem::ePriceStockName, (from.isCurrency()) ? QString() : d->m_stockNameMap.value(from.id()));
    priceTreeItem->setToolTip(KPriceTreeItem::ePriceStockName, (from.isCurrency()) ? QString() : d->m_stockNameMap.value(from.id()));
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
  Q_D(KMyMoneyPriceDlg);
  QTreeWidgetItem* item = 0;
  if (d->ui->m_priceList->selectedItems().count() > 0) {
    item = d->ui->m_priceList->selectedItems().at(0);
  }
  d->m_currentItem = item;
  d->ui->m_editButton->setEnabled(item != 0);
  bool deleteEnabled = (item != 0);

  //if one of the selected entries is a default, then deleting is disabled
  QList<QTreeWidgetItem*> itemsList = d->ui->m_priceList->selectedItems();
  QList<QTreeWidgetItem*>::const_iterator item_it;
  for (item_it = itemsList.constBegin(); item_it != itemsList.constEnd(); ++item_it) {
    MyMoneyPrice price = (*item_it)->data(0, Qt::UserRole).value<MyMoneyPrice>();
    if (price.source() == "KMyMoney")
      deleteEnabled = false;
  }
  d->ui->m_deleteButton->setEnabled(deleteEnabled);

  // Modification of automatically added entries is not allowed
  // Multiple entries cannot be edited at once
  if (item) {
    MyMoneyPrice price = item->data(0, Qt::UserRole).value<MyMoneyPrice>();
    if (price.source() == "KMyMoney" || itemsList.count() > 1)
      d->ui->m_editButton->setEnabled(false);
    emit selectObject(price);
  }
}

void KMyMoneyPriceDlg::slotNewPrice()
{
  Q_D(KMyMoneyPriceDlg);
  QPointer<KUpdateStockPriceDlg> dlg = new KUpdateStockPriceDlg(this);
  try {
    auto item = d->ui->m_priceList->currentItem();
    if (item) {
      MyMoneySecurity security;
      security = MyMoneyFile::instance()->security(item->data(0, Qt::UserRole).value<MyMoneyPrice>().from());
      dlg->ui->m_security->setSecurity(security);
      security = MyMoneyFile::instance()->security(item->data(0, Qt::UserRole).value<MyMoneyPrice>().to());
      dlg->ui->m_currency->setSecurity(security);
    }

    if (dlg->exec()) {
      MyMoneyPrice price(dlg->ui->m_security->security().id(), dlg->ui->m_currency->security().id(), dlg->date(), MyMoneyMoney::ONE, QString());
      QTreeWidgetItem* p = loadPriceItem(price);
      d->ui->m_priceList->setCurrentItem(p, true);
      // If the user cancels the following operation, we delete the new item
      // and re-select any previously selected one
      if (slotEditPrice() == Rejected) {
        delete p;
        if (item)
          d->ui->m_priceList->setCurrentItem(item, true);
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
  Q_D(KMyMoneyPriceDlg);
  int rc = Rejected;
  auto item = d->ui->m_priceList->currentItem();
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
  Q_D(KMyMoneyPriceDlg);
  QList<QTreeWidgetItem*> listItems = d->ui->m_priceList->selectedItems();
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
  Q_D(KMyMoneyPriceDlg);
  auto item = d->ui->m_priceList->itemAt(p);
  if (item) {
    d->ui->m_priceList->setCurrentItem(item, QItemSelectionModel::ClearAndSelect);
    emit openContextMenu(item->data(0, Qt::UserRole).value<MyMoneyPrice>());
  }
}
