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

#ifndef JOURNALMODEL_H
#define JOURNALMODEL_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QSharedDataPointer>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymodel.h"
#include "mymoneyenums.h"
#include "kmm_mymoney_export.h"

#include "mymoneyobject.h"
#include "mymoneytransaction.h"
#include "mymoneysplit.h"

/**
 * The class representing a single journal entry (one split of a transaction)
 */
class /* no export here on purpose */ JournalEntry
{
public:
  explicit JournalEntry() {}
  JournalEntry(QString id, QSharedPointer<MyMoneyTransaction> t, const MyMoneySplit& sp)
  : m_id(id)
  , m_transaction(t)
  , m_split(sp)
  {}

  inline const MyMoneyTransaction& transaction() const { return *m_transaction; }
  inline const MyMoneySplit& split() const { return m_split; }
  inline const QString& id() const { return m_id; }

private:
  QString                             m_id;
  QSharedPointer<MyMoneyTransaction>  m_transaction;
  MyMoneySplit                        m_split;

};



/**
  */
class KMM_MYMONEY_EXPORT JournalModel : public MyMoneyModel<JournalEntry>
{
  Q_OBJECT

public:
  class Column {
    enum {
      Name
    } Columns;
  };

  explicit JournalModel(QObject* parent = 0);
  virtual ~JournalModel();

  static const int ID_SIZE = 6;

  int columnCount(const QModelIndex& parent = QModelIndex()) const final override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const final override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const final override;

  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) final override;

  void load(const QMap<QString, MyMoneyTransaction>& list);

public Q_SLOTS:

private:
  struct Private;
  QScopedPointer<Private> d;
};

#endif // JOURNALMODEL_H

