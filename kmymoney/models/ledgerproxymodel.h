/*
 * SPDX-FileCopyrightText: 2016-2018 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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

  /**
   * Set if the attached views shall show an empty transaction at
   * the end of the ledger which clicked starts editing a new transaction.
   *
   * @note This must be called before setAccount().
   */
  void setShowNewTransaction(bool show);

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

