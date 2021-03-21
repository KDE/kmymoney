/*
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kequitypriceupdateconfdlg.h"

#include "ui_kequitypriceupdateconfdlg.h"

#include "dialogenums.h"

class EquityPriceUpdateConfDlgPrivate
{
    Q_DISABLE_COPY(EquityPriceUpdateConfDlgPrivate)

public:
    EquityPriceUpdateConfDlgPrivate() :
        ui(new Ui::EquityPriceUpdateConfDlg),
        m_updatingPricePolicy(eDialogs::UpdatePrice::All)
    {
    }

    ~EquityPriceUpdateConfDlgPrivate()
    {
        delete ui;
    }

    void updatingPricePolicyChanged(const eDialogs::UpdatePrice policy, bool toggled)
    {
        if (!toggled)
            return;

        switch(policy) {
        case eDialogs::UpdatePrice::All:
            ui->m_updateMissing->setChecked(false);
            ui->m_updateDownloaded->setChecked(false);
            ui->m_updateSource->setChecked(false);
            ui->m_ask->setChecked(false);
            break;
        case eDialogs::UpdatePrice::Missing:
            ui->m_updateAll->setChecked(false);
            ui->m_updateDownloaded->setChecked(false);
            ui->m_updateSource->setChecked(false);
            ui->m_ask->setChecked(false);
            break;
        case eDialogs::UpdatePrice::Downloaded:
            ui->m_updateAll->setChecked(false);
            ui->m_updateMissing->setChecked(false);
            ui->m_updateSource->setChecked(false);
            ui->m_ask->setChecked(false);
            break;
        case eDialogs::UpdatePrice::SameSource:
            ui->m_updateAll->setChecked(false);
            ui->m_updateMissing->setChecked(false);
            ui->m_updateDownloaded->setChecked(false);
            ui->m_ask->setChecked(false);
            break;
        case eDialogs::UpdatePrice::Ask:
            ui->m_updateAll->setChecked(false);
            ui->m_updateDownloaded->setChecked(false);
            ui->m_updateSource->setChecked(false);
            ui->m_updateMissing->setChecked(false);
            break;
        }
        m_updatingPricePolicy = policy;
    }

    Ui::EquityPriceUpdateConfDlg  *ui;
    eDialogs::UpdatePrice          m_updatingPricePolicy;
};

EquityPriceUpdateConfDlg::EquityPriceUpdateConfDlg(eDialogs::UpdatePrice policy) :
    QDialog(nullptr),
    d_ptr(new EquityPriceUpdateConfDlgPrivate)
{
    Q_D(EquityPriceUpdateConfDlg);
    d->ui->setupUi(this);
    switch(policy) {
    case eDialogs::UpdatePrice::All:
        d->ui->m_updateAll->setChecked(true);
        break;
    case eDialogs::UpdatePrice::Missing:
        d->ui->m_updateMissing->setChecked(true);
        break;
    case eDialogs::UpdatePrice::Downloaded:
        d->ui->m_updateDownloaded->setChecked(true);
        break;
    case eDialogs::UpdatePrice::SameSource:
        d->ui->m_updateSource->setChecked(true);
        break;
    case eDialogs::UpdatePrice::Ask:
        d->ui->m_ask->setChecked(true);
        break;
    }

    d->m_updatingPricePolicy = policy;
    connect(d->ui->m_updateAll, &QAbstractButton::toggled, this, &EquityPriceUpdateConfDlg::updateAllToggled);
    connect(d->ui->m_updateMissing, &QAbstractButton::toggled, this, &EquityPriceUpdateConfDlg::updateMissingToggled);
    connect(d->ui->m_updateDownloaded, &QAbstractButton::toggled, this, &EquityPriceUpdateConfDlg::updateDownloadedToggled);
    connect(d->ui->m_updateSource, &QAbstractButton::toggled, this, &EquityPriceUpdateConfDlg::updateSameSourceToggled);
    connect(d->ui->m_ask, &QAbstractButton::toggled, this, &EquityPriceUpdateConfDlg::askToggled);
}

EquityPriceUpdateConfDlg::~EquityPriceUpdateConfDlg()
{
    Q_D(EquityPriceUpdateConfDlg);
    delete d;
}

void EquityPriceUpdateConfDlg::updateAllToggled(bool toggled)
{
    Q_D(EquityPriceUpdateConfDlg);
    d->updatingPricePolicyChanged(eDialogs::UpdatePrice::All, toggled);
}

void EquityPriceUpdateConfDlg::updateMissingToggled(bool toggled)
{
    Q_D(EquityPriceUpdateConfDlg);
    d->updatingPricePolicyChanged(eDialogs::UpdatePrice::Missing, toggled);
}

void EquityPriceUpdateConfDlg::updateDownloadedToggled(bool toggled)
{
    Q_D(EquityPriceUpdateConfDlg);
    d->updatingPricePolicyChanged(eDialogs::UpdatePrice::Downloaded, toggled);
}

void EquityPriceUpdateConfDlg::updateSameSourceToggled(bool toggled)
{
    Q_D(EquityPriceUpdateConfDlg);
    d->updatingPricePolicyChanged(eDialogs::UpdatePrice::SameSource, toggled);
}

void EquityPriceUpdateConfDlg::askToggled(bool toggled)
{
    Q_D(EquityPriceUpdateConfDlg);
    d->updatingPricePolicyChanged(eDialogs::UpdatePrice::Ask, toggled);
}

eDialogs::UpdatePrice EquityPriceUpdateConfDlg::policy() const
{
    Q_D(const EquityPriceUpdateConfDlg);
    return d->m_updatingPricePolicy;
}
