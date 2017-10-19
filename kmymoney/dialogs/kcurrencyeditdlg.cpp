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
                           (C) 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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
#include <QList>
#include <QTreeWidget>
#include <QStyledItemDelegate>
#include <QIcon>
#include <QPushButton>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KTreeWidgetSearchLineWidget>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kcurrencyeditdlg.h"
#include "ui_kcurrencyeditordlg.h"
#include "ui_kavailablecurrencydlg.h"

#include "mymoneysecurity.h"
#include "mymoneyfile.h"
#include "kavailablecurrencydlg.h"
#include "kcurrencyeditordlg.h"
#include "kmymoneyutils.h"
#include "icons/icons.h"

using namespace Icons;

// this delegate is needed to disable editing the currency id (column 1)
// since QTreeWidgetItem has only one set of flags for the whole row
// the column editable property couldn't be set in an easier way
class KCurrencyEditDelegate : public QStyledItemDelegate
{
public:
  explicit KCurrencyEditDelegate(QObject *parent = 0);

protected:
  QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

KCurrencyEditDelegate::KCurrencyEditDelegate(QObject* parent): QStyledItemDelegate(parent)
{
}

QWidget *KCurrencyEditDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  if (index.column() == 1)
    return 0;
  return QStyledItemDelegate::createEditor(parent, option, index);
}

KCurrencyEditDlg::KCurrencyEditDlg(QWidget *parent) : ui(new Ui::KCurrencyEditDlg)
{
  Q_UNUSED(parent);
  ui->setupUi(this);
  m_searchWidget = new KTreeWidgetSearchLineWidget(this, ui->m_currencyList);
  m_searchWidget->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
  m_searchWidget->setFocus();
  ui->verticalLayout->insertWidget(0, m_searchWidget);
  ui->m_currencyList->setItemDelegate(new KCurrencyEditDelegate(ui->m_currencyList));
  ui->m_closeButton->setIcon(QIcon::fromTheme(g_Icons[Icon::DialogClose]));
  ui->m_editCurrencyButton->setIcon(QIcon::fromTheme(g_Icons[Icon::DocumentEdit]));
  ui->m_selectBaseCurrencyButton->setIcon(QIcon::fromTheme(g_Icons[Icon::KMyMoney]));

  connect(ui->m_currencyList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotOpenContextMenu(QPoint)));
  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadCurrencies()));
  connect(ui->m_currencyList, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(slotUpdateCurrency(QTreeWidgetItem*)));
  connect(ui->m_currencyList, SIGNAL(itemSelectionChanged()), this, SLOT(slotItemSelectionChanged()));

  connect(ui->m_selectBaseCurrencyButton, SIGNAL(clicked()), this, SLOT(slotSelectBaseCurrency()));
  connect(ui->m_addCurrencyButton, SIGNAL(clicked()), this, SLOT(slotAddCurrency()));
  connect(ui->m_removeCurrencyButton, SIGNAL(clicked()), this, SLOT(slotRemoveCurrency()));
  connect(ui->m_editCurrencyButton, SIGNAL(clicked()), this, SLOT(slotEditCurrency()));
  connect(ui->m_removeUnusedCurrencyButton, SIGNAL(clicked()), this, SLOT(slotRemoveUnusedCurrency()));

  QTimer::singleShot(10, this, SLOT(timerDone()));
}

void KCurrencyEditDlg::timerDone()
{
  slotLoadCurrencies();

  //resize the column widths
  for (int i = 0; i < 3; ++i)
    ui->m_currencyList->resizeColumnToContents(i);

  if (!m_currency.id().isEmpty()) {
    QTreeWidgetItemIterator it(ui->m_currencyList);
    QTreeWidgetItem* q;
    while ((q = *it) != 0) {
      if (q->text(1) == m_currency.id()) {
        ui->m_currencyList->scrollToItem(q);
        break;
      }
      ++it;
    }
  }
}

KCurrencyEditDlg::~KCurrencyEditDlg()
{
}

void KCurrencyEditDlg::slotLoadCurrencies()
{
  disconnect(ui->m_currencyList, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), this, SLOT(slotSelectCurrency(QTreeWidgetItem*)));
  disconnect(ui->m_currencyList, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(slotUpdateCurrency(QTreeWidgetItem*)));
  QList<MyMoneySecurity> list = MyMoneyFile::instance()->currencyList();
  QList<MyMoneySecurity>::ConstIterator it;
  QTreeWidgetItem *first = 0;

  QString localCurrency(localeconv()->int_curr_symbol);
  localCurrency.truncate(3);

  QString baseCurrency;
  try {
    baseCurrency = MyMoneyFile::instance()->baseCurrency().id();
  } catch (const MyMoneyException &e) {
    qDebug("%s", qPrintable(e.what()));
  }

  // construct a transparent 16x16 pixmap
  QPixmap empty(16, 16);
  QBitmap mask(16, 16);
  mask.clear();
  empty.setMask(mask);

  ui->m_currencyList->clear();
  for (it = list.constBegin(); it != list.constEnd(); ++it) {
    QTreeWidgetItem *p = new QTreeWidgetItem(ui->m_currencyList);
    p->setText(0, (*it).name());
    p->setData(0, Qt::UserRole, QVariant::fromValue(*it));
    p->setFlags(p->flags() | Qt::ItemIsEditable);
    p->setText(1, (*it).id());
    p->setText(2, (*it).tradingSymbol());

    if ((*it).id() == baseCurrency) {
      p->setData(0, Qt::DecorationRole, QIcon::fromTheme(g_Icons[Icon::KMyMoney]));
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
  ui->m_removeUnusedCurrencyButton->setDisabled(list.count() <= 1);
  ui->m_currencyList->sortItems(0, Qt::AscendingOrder);

  connect(ui->m_currencyList, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), this, SLOT(slotSelectCurrency(QTreeWidgetItem*)));
  connect(ui->m_currencyList, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(slotUpdateCurrency(QTreeWidgetItem*)));

  if (first == 0)
    first = ui->m_currencyList->invisibleRootItem()->child(0);
  if (first != 0) {
    ui->m_currencyList->setCurrentItem(first);
    ui->m_currencyList->scrollToItem(first);
  }

  slotSelectCurrency(first);
}

void KCurrencyEditDlg::slotUpdateCurrency(QTreeWidgetItem* item)
{
  //if there is no current item selected, exit
  if (!ui->m_currencyList->currentItem() || item != ui->m_currencyList->currentItem())
    return;

  //verify that the stored currency id is not empty and the edited fields are not empty either
  if (!m_currency.id().isEmpty()
      && !ui->m_currencyList->currentItem()->text(2).isEmpty()
      && !ui->m_currencyList->currentItem()->text(0).isEmpty()) {
    //check that either the name or the id have changed
    if (ui->m_currencyList->currentItem()->text(2) != m_currency.tradingSymbol()
        || ui->m_currencyList->currentItem()->text(0) != m_currency.name()) {
      //update the name and the id
      m_currency.setName(ui->m_currencyList->currentItem()->text(0));
      m_currency.setTradingSymbol(ui->m_currencyList->currentItem()->text(2));

      emit updateCurrency(m_currency.id(), m_currency.name(), m_currency.tradingSymbol());
    }
  }
}

void KCurrencyEditDlg::slotSelectCurrency(const QString& id)
{
  QTreeWidgetItemIterator it(ui->m_currencyList);

  while (*it) {
    if ((*it)->text(1) == id) {
      ui->m_currencyList->blockSignals(true);
      slotSelectCurrency(*it);
      ui->m_currencyList->setCurrentItem(*it);
      ui->m_currencyList->scrollToItem(*it);
      ui->m_currencyList->blockSignals(false);
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
  } catch (const MyMoneyException &) {
  }

  if (item) {
    try {
      m_currency = file->security(item->text(1));

    } catch (const MyMoneyException &) {
      m_currency = MyMoneySecurity();
    }

    MyMoneyFileBitArray skip(IMyMoneyStorage::MaxRefCheckBits);
    skip.fill(false);
    skip.setBit(IMyMoneyStorage::RefCheckPrice);

    const bool rc1 = m_currency.id() == baseId;
    const bool rc2 = file->isReferenced(m_currency, skip);
    const int count = ui->m_currencyList->selectedItems().count();

    ui->m_selectBaseCurrencyButton->setDisabled(rc1 || count != 1);
    ui->m_editCurrencyButton->setDisabled(count != 1);
    ui->m_removeCurrencyButton->setDisabled((rc1 || rc2) && count <= 1);
    emit selectObject(m_currency);
  }
}

void KCurrencyEditDlg::slotItemSelectionChanged()
{
  int count = ui->m_currencyList->selectedItems().count();
  if (!ui->m_selectBaseCurrencyButton->isEnabled() && count == 1)
    slotSelectCurrency(ui->m_currencyList->currentItem());
  if (count > 1)
    ui->m_removeCurrencyButton->setEnabled(true);
}

void KCurrencyEditDlg::slotStartRename()
{
  QTreeWidgetItemIterator it_l(ui->m_currencyList, QTreeWidgetItemIterator::Selected);
  QTreeWidgetItem* it_v;
  if ((it_v = *it_l) != 0) {
    ui->m_currencyList->editItem(it_v, 0);
  }
}

void KCurrencyEditDlg::slotOpenContextMenu(const QPoint& p)
{
  QTreeWidgetItem* item = ui->m_currencyList->itemAt(p);
  if (item)
    emit openContextMenu(item->data(0, Qt::UserRole).value<MyMoneySecurity>());
}

void KCurrencyEditDlg::slotSelectBaseCurrency()
{
  if (!m_currency.id().isEmpty()) {
    QTreeWidgetItem* p = ui->m_currencyList->currentItem();
    emit selectBaseCurrency(m_currency);
    // in case the dataChanged() signal was not sent out (nested FileTransaction)
    // we update the list manually
    if (p == ui->m_currencyList->currentItem())
      slotLoadCurrencies();
  }
}

void KCurrencyEditDlg::slotAddCurrency()
{
  m_availableCurrencyDlg = new KAvailableCurrencyDlg;                                   // create new dialog for selecting currencies to add
  if (m_availableCurrencyDlg->exec() != QDialog::Rejected) {
    MyMoneyFile* file = MyMoneyFile::instance();
    QMap<MyMoneySecurity, MyMoneyPrice> ancientCurrencies = file->ancientCurrencies();
    MyMoneyFileTransaction ft;
    QList<QTreeWidgetItem *> currencyRows = m_availableCurrencyDlg->ui->m_currencyList->selectedItems(); // get selected currencies from new dialog
    foreach (auto currencyRow, currencyRows) {
      MyMoneySecurity currency = currencyRow->data(0, Qt::UserRole).value<MyMoneySecurity>();
      file->addCurrency(currency);
      if (ancientCurrencies.value(currency, MyMoneyPrice()) != MyMoneyPrice()) // if ancient currency is added...
        file->addPrice(ancientCurrencies[currency]);                           // ...we want to add last known exchange rate as well
    }
    ft.commit();
    ui->m_removeUnusedCurrencyButton->setDisabled(file->currencyList().count() <= 1);
  }
  delete m_availableCurrencyDlg;
}

void KCurrencyEditDlg::removeCurrency(const removalModeE& mode)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyFileTransaction ft;
  MyMoneyFileBitArray skip(IMyMoneyStorage::MaxRefCheckBits);
  skip.fill(false);                                 // check reference to all...
  skip.setBit(IMyMoneyStorage::RefCheckPrice);      // ...except price

  QTreeWidgetItemIterator it (ui->m_currencyList);  // iterate over whole tree
  if (mode == RemoveUnused) {
    while (*it) {
      MyMoneySecurity currency = (*it)->data(0, Qt::UserRole).value<MyMoneySecurity>();
      if (file->baseCurrency() != currency && !file->isReferenced(currency, skip))
        KMyMoneyUtils::deleteSecurity(currency, this);
      ++it;
    }
  } else if (mode == RemoveSelected) {
    QList<QTreeWidgetItem*> currencyRows = ui->m_currencyList->selectedItems();
    foreach(auto currencyRow, currencyRows) {
      MyMoneySecurity currency = currencyRow->data(0, Qt::UserRole).value<MyMoneySecurity>();
      if (file->baseCurrency() != currency && !file->isReferenced(currency, skip))
        KMyMoneyUtils::deleteSecurity(currency, this);
    }
  }
  ft.commit();
  ui->m_removeUnusedCurrencyButton->setDisabled(file->currencyList().count() <= 1);
}

void KCurrencyEditDlg::slotRemoveCurrency()
{
  removeCurrency(RemoveSelected);
}

void KCurrencyEditDlg::slotRemoveUnusedCurrency()
{
  removeCurrency(RemoveUnused);
}

void KCurrencyEditDlg::slotEditCurrency()
{
  MyMoneySecurity currency = ui->m_currencyList->currentItem()->data(0, Qt::UserRole).value<MyMoneySecurity>();
  m_currencyEditorDlg = new KCurrencyEditorDlg(currency);                                   // create new dialog for editing currency
  if (m_currencyEditorDlg->exec() != QDialog::Rejected) {
    MyMoneyFile* file = MyMoneyFile::instance();
    MyMoneyFileTransaction ft;
    currency.setPricePrecision(m_currencyEditorDlg->ui->m_pricePrecision->value());
    try {
      file->modifyCurrency(currency);
      ft.commit();
    } catch (const MyMoneyException &e) {
      qDebug("%s", qPrintable(e.what()));
    }
  }
  delete m_currencyEditorDlg;
}
