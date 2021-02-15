/*
 * SPDX-FileCopyrightText: 2015-2019 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2020 Robert Szczesiak <dev.rszczesiak@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef NEWTRANSACTIONFORM_H
#define NEWTRANSACTIONFORM_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QFrame>
class QModelIndex;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes


class NewTransactionForm : public QFrame
{
  Q_OBJECT
public:
  explicit NewTransactionForm(QWidget* parent = nullptr);
  virtual ~NewTransactionForm();

public Q_SLOTS:
  void showTransaction(const QModelIndex& idx);
  void modelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);

protected Q_SLOTS:
  void rowsInserted(const QModelIndex& parent, int first, int last);
  void rowsRemoved(const QModelIndex& parent, int first, int last);

private:
  class Private;
  Private * const d;
};

#endif // NEWTRANSACTIONFORM_H

