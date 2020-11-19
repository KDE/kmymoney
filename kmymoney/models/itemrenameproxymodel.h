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

#ifndef ITEMRENAMEPROXYMODEL_H
#define ITEMRENAMEPROXYMODEL_H

#include "kmm_models_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QSortFilterProxyModel>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes


/**
  * A proxy model to provide in view rename for an item .
  *
  * /// @todo update description
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
  * @author Thomas Baumgart
  *
  */

class KMM_MODELS_EXPORT ItemRenameProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT
  Q_DISABLE_COPY(ItemRenameProxyModel)

public:
  typedef enum  {
    eAllItem = 0,
    eReferencedItems,
    eUnReferencedItems,
    eOpenedItems,
    eClosedItems,
    // insert new values above this line
    eMaxItems
  } ReferenceFilterType;

  explicit ItemRenameProxyModel(QObject *parent = nullptr);
  virtual ~ItemRenameProxyModel();

  bool setData(const QModelIndex& idx, const QVariant& value, int role) override;

  Qt::ItemFlags flags(const QModelIndex& idx) const override;
  void setRenameColumn(int column);

  void setReferenceFilter(ReferenceFilterType filterType);
  void setReferenceFilter(const QVariant& filterType);

protected:
  virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

Q_SIGNALS:
  void renameItem(const QModelIndex& idx, const QVariant& value);

private:
  int                   m_renameColumn;
  ReferenceFilterType   m_referenceFilter;
};

#endif
