/*
    SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
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

