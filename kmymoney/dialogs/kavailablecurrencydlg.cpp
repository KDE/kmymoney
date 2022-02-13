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

KAvailableCurrencyDlg::KAvailableCurrencyDlg(const QList<QString>& usedCurrencies, QWidget* parent)
    : ui(new Ui::KAvailableCurrencyDlg)
{
    Q_UNUSED(parent);
    ui->setupUi(this);
    m_searchWidget = new KTreeWidgetSearchLineWidget(this, ui->m_currencyList);
    m_searchWidget->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
    ui->verticalLayout->insertWidget(0, m_searchWidget);
    connect(ui->m_currencyList, &QTreeWidget::itemSelectionChanged, this, &KAvailableCurrencyDlg::slotItemSelectionChanged);

    slotLoadCurrencies(usedCurrencies);

    //resize the column widths
    for (auto i = 0; i < ui->m_currencyList->columnCount(); ++i)
        ui->m_currencyList->resizeColumnToContents(i);

    m_searchWidget->setFocus();
}

KAvailableCurrencyDlg::~KAvailableCurrencyDlg()
{
    delete ui;
}

void KAvailableCurrencyDlg::slotLoadCurrencies(const QList<QString>& usedCurrencies)
{
    QList<MyMoneySecurity> list = MyMoneyFile::instance()->availableCurrencyList();

    // construct a transparent 16x16 pixmap
    QPixmap empty(16, 16);
    QBitmap mask(16, 16);
    mask.clear();
    empty.setMask(mask);

    ui->m_currencyList->clear();
    // remove the used currencies from the list
    for (const auto& currency : list) {
        if (!usedCurrencies.contains(currency.id())) {
            const auto item = new QTreeWidgetItem(ui->m_currencyList);
            item->setText(0, currency.name());
            item->setData(0, Qt::UserRole, QVariant::fromValue(currency));
            item->setData(0, Qt::DecorationRole, empty);
            item->setFlags(item->flags() | Qt::ItemIsEditable);
            item->setText(1, currency.id());
            item->setText(2, currency.tradingSymbol());
        }
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
