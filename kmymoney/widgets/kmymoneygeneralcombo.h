/*
 * Copyright 2009-2016  Cristian Oneț <onet.cristian@gmail.com>
 * Copyright 2009-2010  Alvaro Soliverez <asoliverez@gmail.com>
 * Copyright 2010-2017  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#ifndef KMYMONEYGENERALCOMBO_H
#define KMYMONEYGENERALCOMBO_H

#include "kmm_base_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <KComboBox>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class KMM_BASE_WIDGETS_EXPORT KMyMoneyGeneralCombo : public KComboBox
{
  Q_OBJECT
  Q_DISABLE_COPY(KMyMoneyGeneralCombo)
  Q_PROPERTY(int currentItem READ currentItem WRITE setCurrentItem STORED false)

public:
  explicit KMyMoneyGeneralCombo(QWidget* parent = nullptr);
  virtual ~KMyMoneyGeneralCombo();

  void insertItem(const QString& txt, int id, int idx = -1);

  void setCurrentItem(int id);
  int currentItem() const;

  void removeItem(int id);

public Q_SLOTS:
  void clear();

Q_SIGNALS:
  void itemSelected(int id);

protected:
  // prevent the caller to use the standard KComboBox insertItem function with a default idx
  void insertItem(const QString&);

protected Q_SLOTS:
  void slotChangeItem(int idx);

};

#endif
