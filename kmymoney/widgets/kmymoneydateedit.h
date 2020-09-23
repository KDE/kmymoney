/*
 * Copyright 2016-2017  Thomas Baumgart <tbaumgart@kde.org>
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

#ifndef KMYMONEYDATEEDIT_H
#define KMYMONEYDATEEDIT_H

#include "kmm_base_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDateEdit>

// TODO: check if this class is really necessary
class KMM_BASE_WIDGETS_EXPORT KMyMoneyDateEdit : public QDateEdit
{
  Q_OBJECT

public:
  explicit KMyMoneyDateEdit(QWidget* parent = nullptr);

};
#endif // KMYMONEYDATEEDIT_H
