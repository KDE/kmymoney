/*
 * Copyright 2016-2018  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#ifndef LEDGERPROXYMODEL_H
#define LEDGERPROXYMODEL_H

#include "kmm_models_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QSortFilterProxyModel>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyenums.h"

class KMM_MODELS_EXPORT LedgerProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT
public:
  explicit LedgerProxyModel(QObject* parent = nullptr);
  virtual ~LedgerProxyModel();

  void setAccountType(eMyMoney::Account::Type type);

  void setFilterFixedString(const QString& filter);

  void setFilterRole(int role);

  int filterRole() const;

  /**
   * This method maps the @a index to the base model and calls setData on it
   */
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

  /**
   * This method returns the headerData adjusted to the current
   * accountType
   */
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

protected:
  bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;
  /**
   * Currently filtering only on account id provided by @sa setAccount()
   *
   * @note This does not call the base class implementation for speed purposes
   */
  bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

private:
  bool                      m_showNewTransaction;
  eMyMoney::Account::Type   m_accountType;
  QString                   m_filterId;
  int                       m_filterRole;
};

#endif // LEDGERPROXYMODEL_H

