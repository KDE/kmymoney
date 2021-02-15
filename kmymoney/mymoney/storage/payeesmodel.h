/*
 * SPDX-FileCopyrightText: 2016-2017 Thomas Baumgart <tbaumgart@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef PAYEESMODEL_H
#define PAYEESMODEL_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QAbstractListModel>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmm_mymoney_export.h"

/**
  */
class KMM_MYMONEY_EXPORT PayeesModel : public QAbstractListModel
{
  Q_OBJECT

public:
  explicit PayeesModel(QObject* parent = 0);
  ~PayeesModel();

  enum Roles {
    PayeeIdRole = Qt::UserRole,      // must remain Qt::UserRole due to KMyMoneyMVCCombo::selectedItem,
  };

  int rowCount(const QModelIndex& parent = QModelIndex()) const final override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const final override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const final override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const final override;

  Qt::ItemFlags flags(const QModelIndex& index) const final override;
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) final override;

  /**
   * clears all objects currently in the model
   */
  void unload();

  /**
   * Loads the model with data from the engine
   */
  void load();

public Q_SLOTS:

private:
  struct Private;
  QScopedPointer<Private> d;
};

#endif // PAYEESMODEL_H

