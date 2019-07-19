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


#ifndef MYMONEYMODELBASE_H
#define MYMONEYMODELBASE_H

#include <QAbstractItemModel>

#include "kmm_models_export.h"


class KMM_MODELS_EXPORT MyMoneyModelBase : public QAbstractItemModel
{
  Q_OBJECT
public:
  explicit MyMoneyModelBase(QObject* parent);
  virtual ~MyMoneyModelBase();

Q_SIGNALS:
  void modelLoaded() const;

};

#endif // MYMONEYMODELBASE_H
