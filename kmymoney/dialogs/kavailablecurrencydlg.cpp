/*
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kavailablecurrencydlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPixmap>
#include <QBitmap>
#include <QList>
#include <QTreeWidget>
#include <QPushButton>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KTreeWidgetSearchLineWidget>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include <ui_kavailablecurrencydlg.h>
#include "mymoneysecurity.h"

KAvailableCurrencyDlg::KAvailableCurrencyDlg(QWidget *parent)
  : ui(new Ui::KAvailableCurrencyDlg)
{
  Q_UNUSED(parent);
  ui->setupUi(this);
  m_searchWidget = new KTreeWidgetSearchLineWidget(this, ui->m_currencyList);
  m_searchWidget->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
  ui->verticalLayout->insertWidget(0, m_searchWidget);
  connect(ui->m_currencyList, &QTreeWidget::itemSelectionChanged, this, &KAvailableCurrencyDlg::slotItemSelectionChanged);

  slotLoadCurrencies();

  //resize the column widths
  for (auto i = 0; i < 3; ++i)
    ui->m_currencyList->resizeColumnToContents(i);

  m_searchWidget->setFocus();
}

KAvailableCurrencyDlg::~KAvailableCurrencyDlg()
{
  delete ui;
}

void KAvailableCurrencyDlg::slotLoadCurrencies()
{
  QList<MyMoneySecurity> list = MyMoneyFile::instance()->availableCurrencyList();
  QList<MyMoneySecurity> currencies = MyMoneyFile::instance()->currencyList();
  foreach (auto currency, currencies) {
    int idx = list.indexOf(currency);
    if (idx != -1)
      list.removeAt(idx);
  }

  QList<MyMoneySecurity>::ConstIterator it;

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
    p->setData(0, Qt::DecorationRole, empty);
    p->setFlags(p->flags() | Qt::ItemIsEditable);
    p->setText(1, (*it).id());
    p->setText(2, (*it).tradingSymbol());
  }

  ui->m_currencyList->sortItems(0, Qt::AscendingOrder);
}

void KAvailableCurrencyDlg::slotItemSelectionChanged()
{
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!ui->m_currencyList->selectedItems().isEmpty());
}

QList<QTreeWidgetItem*> KAvailableCurrencyDlg::selectedItems() const
{
  return ui->m_currencyList->selectedItems();
}
