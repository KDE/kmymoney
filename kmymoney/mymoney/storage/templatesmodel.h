/*
 * Copyright 2020       Thomas Baumgart <tbaumgart@kde.org>
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

#ifndef TEMPLATESMODEL_H
#define TEMPLATESMODEL_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneytemplate.h"
#include "mymoneymodel.h"
#include "mymoneyenums.h"
#include "kmm_mymoney_export.h"


class QUndoStack;
/**
  */
class KMM_MYMONEY_EXPORT TemplatesModel : public MyMoneyModel<MyMoneyTemplate>
{
  Q_OBJECT
  Q_DISABLE_COPY(TemplatesModel)

public:
  enum Column {
    Type = 0,
    Description,
    // insert new columns above this line
    MaxColumns
  };

  explicit TemplatesModel(QObject* parent = nullptr, QUndoStack* undoStack = nullptr);
  virtual ~TemplatesModel();

  static const int ID_SIZE = 6;

  int columnCount(const QModelIndex& parent = QModelIndex()) const final override;
  QVariant data(const QModelIndex& idx, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const final override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;

  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) final override;

  void addItem(MyMoneyTemplate& tmpl, const QModelIndex& parent);

protected:

public Q_SLOTS:

Q_SIGNALS:

private:
  struct Private;
  QScopedPointer<Private> d;
};

#endif // TEMPLATESMODEL_H

