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

#ifndef SCHEDULESJOURNALMODEL_H
#define SCHEDULESJOURNALMODEL_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "journalmodel.h"
#include "mymoneyenums.h"

#include "kmm_mymoney_export.h"


class QUndoStack;
/**
  */
class KMM_MYMONEY_EXPORT SchedulesJournalModel : public JournalModel
{
  Q_OBJECT

public:
  class Column {
  public:
    enum {
      Name
    } Columns;
  };

  explicit SchedulesJournalModel(QObject* parent = nullptr, QUndoStack* undoStack = nullptr);
  ~SchedulesJournalModel();

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  Qt::ItemFlags flags(const QModelIndex & index) const override;

  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

  void setPreviewPeriod(int days);
  void setShowPlannedDate(bool showPlannedDate = true);

public Q_SLOTS:
  void updateData();

private Q_SLOTS:
  /**
   * override the JournalModel::load() method here so that it cannot
   * be called, as it is useless in the context of this class
   */
  void load(const QMap<QString, MyMoneyTransaction>& list);
  void doLoad();

private:
  struct Private;
  QScopedPointer<Private> d;
};

#endif // SCHEDULESJOURNALMODEL_H

