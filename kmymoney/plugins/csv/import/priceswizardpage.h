/*
 * Copyright 2017  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#ifndef PRICESWIZARDPAGE_H
#define PRICESWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QPointer>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "csvwizardpage.h"

// ----------------------------------------------------------------------------

class PricesProfile;
class SecurityDlg;
class CurrenciesDlg;

namespace Ui
{
class PricesPage;
}

class PricesPage : public CSVWizardPage
{
  Q_OBJECT

public:
  explicit PricesPage(CSVWizard *dlg, CSVImporterCore *imp);
  ~PricesPage();

private:
  void initializePage() final override;
  bool isComplete() const final override;
  bool validatePage() final override;

  void                resetComboBox(const Column comboBox);
  /**
  * This method is called column on prices page is selected.
  * It sets m_colTypeNum, m_colNumType and runs column validation.
  */
  bool                validateSelectedColumn(const int col, const Column type);

  /**
  * This method ensures that there is security for price import.
  */
  bool                validateSecurity();

  /**
  * This method ensures that there are currencies for price import.
  */
  bool                validateCurrencies();

  PricesProfile      *m_profile;
  Ui::PricesPage     *ui;

  QPointer<SecurityDlg>    m_securityDlg;
  QPointer<CurrenciesDlg>  m_currenciesDlg;

private Q_SLOTS:
  void                dateColSelected(int col);
  void                priceColSelected(int col);
  void                fractionChanged(int col);
  void                clearColumns();
};

#endif // PRICESWIZARDPAGE_H
