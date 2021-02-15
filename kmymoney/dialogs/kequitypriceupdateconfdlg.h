/*
 * SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KEQUITYPRICEUPDATECONFDLG_H
#define KEQUITYPRICEUPDATECONFDLG_H

#include <QDialog>

namespace eDialogs { enum class UpdatePrice; }

class EquityPriceUpdateConfDlgPrivate;
class EquityPriceUpdateConfDlg : public QDialog
{
  Q_OBJECT
  Q_DISABLE_COPY(EquityPriceUpdateConfDlg)

public:
  explicit EquityPriceUpdateConfDlg(eDialogs::UpdatePrice policy);
  ~EquityPriceUpdateConfDlg();

  eDialogs::UpdatePrice policy() const;

private Q_SLOTS:
  void updateAllToggled(bool toggled);
  void updateMissingToggled(bool toggled);
  void updateDownloadedToggled(bool toggled);
  void updateSameSourceToggled(bool toggled);
  void askToggled(bool toggled);

private:
  EquityPriceUpdateConfDlgPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(EquityPriceUpdateConfDlg)
};

#endif // KEQUITYPRICEUPDATECONFDLG_H
