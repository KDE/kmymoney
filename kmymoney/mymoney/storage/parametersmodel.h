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

#ifndef PARAMETERSMODEL_H
#define PARAMETERSMODEL_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymodel.h"
#include "mymoneyenums.h"
#include "kmm_mymoney_export.h"

class ParametersModel;
class QUndoStack;

class KMM_MYMONEY_EXPORT  ParameterItem
{
  friend ParametersModel;
public:
  explicit ParameterItem();
  ParameterItem(const QString& key, const QString& value);

  const QString& id() const { return m_id; }
  const QString& value() const { return m_value; }

  QSet<QString> referencedObjects() const { return {}; }
private:
  QString       m_id;
  QString       m_value;
};

/**
  */
class KMM_MYMONEY_EXPORT ParametersModel : public MyMoneyModel<ParameterItem>
{
  Q_OBJECT

public:
  class Column {
    enum {
      Name
    } Columns;
  };

  explicit ParametersModel(QObject* parent = nullptr, QUndoStack* undoStack = nullptr);
  virtual ~ParametersModel();

  static const int ID_SIZE = 6;

  int columnCount(const QModelIndex& parent = QModelIndex()) const final override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const final override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const final override;

  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) final override;

  void addItem(const QString& key, const QString& val);
  void deleteItem(const QString& key);
  void load(const QMap<QString, QString>& pairs);
  QMap<QString, QString> pairs() const;

public Q_SLOTS:

private:
  struct Private;
  QScopedPointer<Private> d;
};

#endif // PARAMETERSMODEL_H

