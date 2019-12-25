/*
 * Copyright 2019       Thomas Baumgart <tbaumgart@kde.org>
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

#ifndef TRANSACTIONEDITORBASE_H
#define TRANSACTIONEDITORBASE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QFrame>
class QWidget;

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymoney.h"

class TransactionEditorBase : public QFrame
{
  Q_OBJECT

public:
  explicit TransactionEditorBase(QWidget* parent = 0, const QString& accountId = QString());
  virtual ~TransactionEditorBase();

  virtual bool accepted() const = 0;
  virtual void loadTransaction(const QModelIndex& index) = 0;
  virtual void saveTransaction() = 0;

Q_SIGNALS:
  void done();

private:
  static QDate  m_lastPostDateUsed;
};

#endif // TRANSACTIONEDITORBASE_H

