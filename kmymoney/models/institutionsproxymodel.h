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

#ifndef INSTITUTIONSPROXYMODEL_H
#define INSTITUTIONSPROXYMODEL_H

#include "kmm_models_export.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "accountsproxymodel.h"

/**
  * A proxy model to provide various sorting and filtering operations for @ref institutionsModel.
  *
  * Here is an example of how to use this class in combination with the @ref institutionsModel.
  * (in the example @a widget is a pointer to a model/view widget):
  *
  * @code
  *   InstitutionsFilterProxyModel *filterModel = new InstitutionsFilterProxyModel(widget);
  *   filterModel->setSourceModel(MyMoneyFile::instance()->institutionsModel());
  *   filterModel->sort(0);
  *
  *   widget->setModel(filterModel);
  * @endcode
  *
  * @sa institutionsModel
  *
  * @author Thomas Baumgart
  *
  */

class KMM_MODELS_EXPORT InstitutionsProxyModel : public AccountsProxyModel
{
  Q_OBJECT
  Q_DISABLE_COPY(InstitutionsProxyModel)

public:
  explicit InstitutionsProxyModel(QObject *parent = nullptr);
  virtual ~InstitutionsProxyModel();

protected:
  bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
  bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

  int visibleItems(const QModelIndex& index) const;

Q_SIGNALS:
  void unusedIncomeExpenseAccountHidden();

};

#endif
