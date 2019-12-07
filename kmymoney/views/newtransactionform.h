/*
 * Copyright 2015-2019  Thomas Baumgart <tbaumgart@kde.org>
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
  explicit NewTransactionForm(QWidget* parent = 0);
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

