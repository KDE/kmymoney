/*
 * Copyright 2017       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
