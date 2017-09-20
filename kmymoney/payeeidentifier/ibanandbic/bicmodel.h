/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2014 Christian Dávid <christian-david@web.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BICMODEL_H
#define BICMODEL_H

#include <QtSql/QSqlQueryModel>

#include "iban_bic_identifier_export.h"

class IBAN_BIC_IDENTIFIER_EXPORT bicModel : public QSqlQueryModel
{
  Q_OBJECT

public:
  enum DisplayRole {
    InstitutionNameRole = Qt::UserRole
  };

  explicit bicModel(QObject* parent = 0);
  virtual QVariant data(const QModelIndex& item, int role = Qt::DisplayRole) const;

};

#endif // BICMODEL_H
