/*******************************************************************************
*                               kequitypriceupdateconfdlg.cpp
*                               ------------------
* begin                       : Sun May 21 2017
* copyright                   : (C) 2017 by Łukasz Wojnilowicz
* email                       : lukasz.wojnilowicz@gmail.com
********************************************************************************/

/*******************************************************************************
*                                                                              *
*   This program is free software; you can redistribute it and/or modify       *
*   it under the terms of the GNU General Public License as published by       *
*   the Free Software Foundation; either version 2 of the License, or          *
*   (at your option) any later version.                                        *
*                                                                              *
********************************************************************************/

#include "kequitypriceupdateconfdlg.h"

#include "ui_kequitypriceupdateconfdlg.h"

EquityPriceUpdateConfDlg::EquityPriceUpdateConfDlg(const updatingPricePolicyE policy) : ui(new Ui::EquityPriceUpdateConfDlg)
{
  ui->setupUi(this);
  switch(policy) {
    case eUpdateAllPrices:
      ui->m_updateAll->setChecked(true);
      break;
    case eUpdateMissingPrices:
      ui->m_updateMissing->setChecked(true);
      break;
    case eUpdateDownloadedPrices:
      ui->m_updateDownloaded->setChecked(true);
      break;
    case eUpdateSameSourcePrices:
      ui->m_updateSource->setChecked(true);
      break;
    case eAsk:
      ui->m_ask->setChecked(true);
      break;
    default:
      break;
  }

  m_updatingPricePolicy = policy;
  connect(ui->m_updateAll, &QAbstractButton::toggled, this, &EquityPriceUpdateConfDlg::updateAllToggled);
  connect(ui->m_updateMissing, &QAbstractButton::toggled, this, &EquityPriceUpdateConfDlg::updateMissingToggled);
  connect(ui->m_updateDownloaded, &QAbstractButton::toggled, this, &EquityPriceUpdateConfDlg::updateDownloadedToggled);
  connect(ui->m_updateSource, &QAbstractButton::toggled, this, &EquityPriceUpdateConfDlg::updateSameSourceToggled);
  connect(ui->m_ask, &QAbstractButton::toggled, this, &EquityPriceUpdateConfDlg::askToggled);
}

EquityPriceUpdateConfDlg::~EquityPriceUpdateConfDlg()
{
  delete ui;
}

void EquityPriceUpdateConfDlg::updateAllToggled(bool toggled)
{
  updatingPricePolicyChanged(eUpdateAllPrices, toggled);
}

void EquityPriceUpdateConfDlg::updateMissingToggled(bool toggled)
{
  updatingPricePolicyChanged(eUpdateMissingPrices, toggled);
}

void EquityPriceUpdateConfDlg::updateDownloadedToggled(bool toggled)
{
  updatingPricePolicyChanged(eUpdateDownloadedPrices, toggled);
}

void EquityPriceUpdateConfDlg::updateSameSourceToggled(bool toggled)
{
  updatingPricePolicyChanged(eUpdateSameSourcePrices, toggled);
}

void EquityPriceUpdateConfDlg::askToggled(bool toggled)
{
  updatingPricePolicyChanged(eAsk, toggled);
}

updatingPricePolicyE EquityPriceUpdateConfDlg::policy()
{
  return m_updatingPricePolicy;
}

void EquityPriceUpdateConfDlg::updatingPricePolicyChanged(const updatingPricePolicyE policy, bool toggled)
{
  if (!toggled)
    return;

  switch(policy) {
    case eUpdateAllPrices:
      ui->m_updateMissing->setChecked(false);
      ui->m_updateDownloaded->setChecked(false);
      ui->m_updateSource->setChecked(false);
      ui->m_ask->setChecked(false);
      break;
    case eUpdateMissingPrices:
      ui->m_updateAll->setChecked(false);
      ui->m_updateDownloaded->setChecked(false);
      ui->m_updateSource->setChecked(false);
      ui->m_ask->setChecked(false);
      break;
    case eUpdateDownloadedPrices:
      ui->m_updateAll->setChecked(false);
      ui->m_updateMissing->setChecked(false);
      ui->m_updateSource->setChecked(false);
      ui->m_ask->setChecked(false);
      break;
    case eUpdateSameSourcePrices:
      ui->m_updateAll->setChecked(false);
      ui->m_updateMissing->setChecked(false);
      ui->m_updateDownloaded->setChecked(false);
      ui->m_ask->setChecked(false);
      break;
    case eAsk:
      ui->m_updateAll->setChecked(false);
      ui->m_updateDownloaded->setChecked(false);
      ui->m_updateSource->setChecked(false);
      ui->m_updateMissing->setChecked(false);
      break;
    default:
      break;
  }
  m_updatingPricePolicy = policy;
}
