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

#ifndef SPECIALDATESMODEL_H
#define SPECIALDATESMODEL_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmm_mymoney_export.h"
#include "mymoneymodel.h"

class /* no export here on purpose */ SpecialDateEntry
{
public:
  explicit SpecialDateEntry() {}
  SpecialDateEntry(const QString& id, const QDate& date, const QString& txt)
  : m_id(id)
  , m_txt(txt)
  , m_date(date)
  {}

  inline const QString& id() const { return m_id; }
  inline const QString& txt() const { return m_txt; }
  inline const QDate& date() const { return m_date; }
  inline bool hasReferenceTo(const QString& id) const { return false; }

  /**
   * @copydoc MyMoneyObject::referencedObjects
   */
  inline QSet<QString> referencedObjects() const { return {}; }

private:
  QString     m_id;
  QString     m_txt;
  QDate       m_date;
};

class QUndoStack;
/**
  */
class KMM_MYMONEY_EXPORT SpecialDatesModel : public MyMoneyModel<SpecialDateEntry>
{
  Q_OBJECT

public:
  explicit SpecialDatesModel(QObject* parent = nullptr, QUndoStack* undoStack = nullptr);
  ~SpecialDatesModel();

  static const int ID_SIZE = 2;

  int columnCount (const QModelIndex & parent) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const final override;

public Q_SLOTS:
  void load();

private:
  struct Private;
  QScopedPointer<Private> d;
};

#endif // SPECIALDATESMODEL_H

