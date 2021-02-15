/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
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
  explicit ParameterItem(const QString& id, const ParameterItem& other)
  {
    Q_UNUSED(id)
    *this = other;
  }
  ParameterItem(const QString& key, const QString& value);

  const QString& id() const { return m_id; }
  const QString& value() const { return m_value; }

  QSet<QString> referencedObjects() const { return {}; }
  bool hasReferenceTo(const QString& id) const { Q_UNUSED(id) return false; }

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

