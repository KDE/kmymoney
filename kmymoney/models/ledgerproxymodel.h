/***************************************************************************
                          ledgerproxymodel.h
                             -------------------
    begin                : Sat Aug 8 2015
    copyright            : (C) 2015 by Thomas Baumgart
    email                : Thomas Baumgart <tbaumgart@kde.org>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LEDGERPROXYMODEL_H
#define LEDGERPROXYMODEL_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QSortFilterProxyModel>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyenums.h"

class LedgerProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT
public:
  explicit LedgerProxyModel(QObject* parent = nullptr);
  virtual ~LedgerProxyModel();

  void setShowNewTransaction(bool show);
  void setAccountType(eMyMoney::Account::Type type);

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
  bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

private:
  bool                 m_showNewTransaction;
  eMyMoney::Account::Type    m_accountType;
};

#endif // LEDGERPROXYMODEL_H

