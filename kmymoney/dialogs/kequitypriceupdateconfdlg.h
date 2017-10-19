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

#ifndef KEQUITYPRICEUPDATECONFDLG_H
#define KEQUITYPRICEUPDATECONFDLG_H

#include <QDialog>

enum updatingPricePolicyE : int {eUpdateAllPrices = 0, eUpdateMissingPrices, eUpdateDownloadedPrices, eUpdateSameSourcePrices, eAsk, eUpdatingPricePolicyEnd};

namespace Ui
{
class EquityPriceUpdateConfDlg;
}

class EquityPriceUpdateConfDlg : public QDialog
{
  Q_OBJECT

public:
  explicit EquityPriceUpdateConfDlg(const updatingPricePolicyE policy);
  ~EquityPriceUpdateConfDlg();

  Ui::EquityPriceUpdateConfDlg*   ui;

  updatingPricePolicyE policy();
private:
  void updatingPricePolicyChanged(const updatingPricePolicyE policy, bool toggled);

  updatingPricePolicyE m_updatingPricePolicy;
private slots:
  void updateAllToggled(bool toggled);
  void updateMissingToggled(bool toggled);
  void updateDownloadedToggled(bool toggled);
  void updateSameSourceToggled(bool toggled);
  void askToggled(bool toggled);
};

#endif // KEQUITYPRICEUPDATECONFDLG_H
