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

#ifndef STATUSMODEL_H
#define STATUSMODEL_H

#include "kmm_mymoney_export.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneymodel.h>
#include <mymoneyenums.h>

class StatusEntry {
public:
  explicit StatusEntry() {}
  explicit StatusEntry(const QString& id, eMyMoney::Split::State state, const QString& shortName, const QString& longName)
  : m_id(id)
  , m_shortName(shortName)
  , m_longName(longName)
  , m_state(state)
  {}

  inline const QString& id() const { return m_id; }
  inline const QString& shortName() const { return m_shortName; }
  inline const QString& longName() const { return m_longName; }
  inline eMyMoney::Split::State state() const { return m_state; }
  inline QSet<QString> referencedObjects() const { return {}; }

private:
  QString                 m_id;
  QString                 m_shortName;
  QString                 m_longName;
  eMyMoney::Split::State  m_state;
};

class KMM_MYMONEY_EXPORT StatusModel : public MyMoneyModel<StatusEntry>
{
  Q_OBJECT

public:
  explicit StatusModel(QObject* parent = nullptr);
  virtual ~StatusModel();

  int columnCount(const QModelIndex& parent = QModelIndex()) const final override;
  QVariant data(const QModelIndex& idx, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const final override;
  Qt::ItemFlags flags(const QModelIndex & index) const override;

private:

};
#endif // STATUSMODEL_H

