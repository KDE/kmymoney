/*
    SPDX-FileCopyrightText: 2022 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// ----------------------------------------------------------------------------
// QT Includes

#include <QCheckBox>
#include <QList>
#include <QPushButton>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "idfilter.h"
#include "ktagreassigndlg.h"
#include "mymoneyfile.h"
#include "tagsmodel.h"

#include "ui_ktagreassigndlg.h"

KTagReassignDlg::KTagReassignDlg(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::KTagReassignDlg)
    , model(new IdFilter(this))
{
    auto checkValidInput = [&]() {
        const auto idx = model->index(ui->tagCombo->currentIndex(), 0);
        const auto validInput = (!idx.data(eMyMoney::Model::IdRole).toString().isEmpty() || ui->removeCheckBox->isChecked());
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(validInput);
    };

    ui->setupUi(this);

    model->setSourceModel(MyMoneyFile::instance()->tagsModel());
    model->setSortLocaleAware(true);
    ui->tagCombo->setModel(model);

    connect(ui->tagCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&]() {
        checkValidInput();
    });
    connect(ui->removeCheckBox, &QCheckBox::toggled, this, [&]() {
        checkValidInput();
    });

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

KTagReassignDlg::~KTagReassignDlg()
{
    delete ui;
}

void KTagReassignDlg::setupFilter(const QList<QString>& tagslist)
{
    qobject_cast<IdFilter*>(model)->setFilterList(tagslist);
    model->sort(0);
    ui->tagCombo->setCurrentIndex(-1);
}

QString KTagReassignDlg::reassignTo() const
{
    if (ui->removeCheckBox->isChecked())
        return {};

    const auto idx = ui->tagCombo->model()->index(ui->tagCombo->currentIndex(), 0);
    return idx.data(eMyMoney::Model::IdRole).toString();
}
