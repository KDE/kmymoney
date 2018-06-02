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
