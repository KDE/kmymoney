/*
    SPDX-FileCopyrightText: 2002-2004 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2010 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kupdatestockpricedlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>
#include <QPushButton>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kupdatestockpricedlg.h"

#include "kmymoneydateinput.h"
#include "kmymoneycurrencyselector.h"

KUpdateStockPriceDlg::KUpdateStockPriceDlg(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::KUpdateStockPriceDlg)
{
    ui->setupUi(this);
    setModal(true);
    ui->m_date->setDate(QDate::currentDate());

    connect(ui->m_security, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, static_cast<void (KUpdateStockPriceDlg::*)(int)>(&KUpdateStockPriceDlg::slotCheckData));
    connect(ui->m_currency, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, static_cast<void (KUpdateStockPriceDlg::*)(int)>(&KUpdateStockPriceDlg::slotCheckData));

    // load initial values into the selection widgets
    ui->m_currency->update(QString());
    ui->m_security->update(QString());

    slotCheckData();
}

KUpdateStockPriceDlg::~KUpdateStockPriceDlg()
{
    delete  ui;
}

int KUpdateStockPriceDlg::exec()
{
    slotCheckData();
    return QDialog::exec();
}

QDate KUpdateStockPriceDlg::date() const
{
    return ui->m_date->date();
}

void KUpdateStockPriceDlg::slotCheckData()
{
    auto from = ui->m_security->security().id();
    auto to   = ui->m_currency->security().id();

    ui->m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!from.isEmpty() && !to.isEmpty() && from != to);
}

void KUpdateStockPriceDlg::slotCheckData(int)
{
    slotCheckData();
}
