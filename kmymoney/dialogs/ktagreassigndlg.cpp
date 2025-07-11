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

class KTagReassignDlgPrivate
{
public:
    KTagReassignDlgPrivate(KTagReassignDlg* q)
        : ui(new Ui::KTagReassignDlg)
        , model(new IdFilter(q))
    {
    }

    ~KTagReassignDlgPrivate()
    {
        delete ui;
    }

    void checkValidInput()
    {
        const auto idx = model->index(ui->tagCombo->currentIndex(), 0);
        const auto validInput = (!idx.data(eMyMoney::Model::IdRole).toString().isEmpty() || ui->removeCheckBox->isChecked());
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(validInput);
    }

    Ui::KTagReassignDlg* ui;
    QSortFilterProxyModel* model;
};

KTagReassignDlg::KTagReassignDlg(QWidget* parent)
    : QDialog(parent)
    , d(new KTagReassignDlgPrivate(this))
{
    d->ui->setupUi(this);

    d->model->setSourceModel(MyMoneyFile::instance()->tagsModel());
    d->model->setSortLocaleAware(true);
    d->ui->tagCombo->setModel(d->model);

    connect(d->ui->tagCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&]() {
        d->checkValidInput();
    });
    connect(d->ui->removeCheckBox, &QCheckBox::toggled, this, [&]() {
        d->checkValidInput();
    });

    d->ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

KTagReassignDlg::~KTagReassignDlg()
{
    delete d;
}

void KTagReassignDlg::setupFilter(const QList<QString>& tagslist)
{
    qobject_cast<IdFilter*>(d->model)->setFilterList(tagslist);
    d->model->sort(0);
    d->ui->tagCombo->setCurrentIndex(-1);
}

QString KTagReassignDlg::reassignTo() const
{
    if (d->ui->removeCheckBox->isChecked())
        return {};

    const auto idx = d->ui->tagCombo->model()->index(d->ui->tagCombo->currentIndex(), 0);
    return idx.data(eMyMoney::Model::IdRole).toString();
}
