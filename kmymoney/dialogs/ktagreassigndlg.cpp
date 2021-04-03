/*
    SPDX-FileCopyrightText: 2011-2012 Alessandro Russo <axela74@yahoo.it>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ktagreassigndlg.h"
#include "ui_ktagreassigndlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "tagsmodel.h"
#include "idfilter.h"

KTagReassignDlg::KTagReassignDlg(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::KTagReassignDlg)
{
    ui->setupUi(this);
}

KTagReassignDlg::~KTagReassignDlg()
{
    delete ui;
}

QString KTagReassignDlg::show(const QList<QString>& tagslist)
{
    auto filter = new IdFilter(this);
    filter->setFilterList(tagslist);
    filter->setSourceModel(MyMoneyFile::instance()->tagsModel());
    filter->setSortLocaleAware(true);
    filter->sort(0);
    ui->tagCombo->setModel(filter);

    // execute dialog and if aborted, return empty string
    if (this->exec() == QDialog::Rejected)
        return QString();

    // otherwise return id of selected tag
    const auto idx = filter->index(ui->tagCombo->currentIndex(), 0);
    return idx.data(eMyMoney::Model::IdRole).toString();
}
