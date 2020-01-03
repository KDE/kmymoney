/*
 * Copyright 2009-2016  Cristian Oneț <onet.cristian@gmail.com>
 * Copyright 2009-2010  Alvaro Soliverez <asoliverez@gmail.com>
 * Copyright 2010-2020  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#ifndef KTAGCONTAINER_H
#define KTAGCONTAINER_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>
class QComboBox;
class QAbstractItemModel;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmm_widgets_export.h"

class MyMoneyTag;

/**
  * This widget contains a QComboBox widget and 0 or more KTagLabel widgets
  *
  * @author Alessandro Russo, Thomas Baumgart
  */
class KTagContainerPrivate;
class KMM_WIDGETS_EXPORT KTagContainer : public QWidget
{
  Q_OBJECT
  Q_DISABLE_COPY(KTagContainer)

public:
  explicit KTagContainer(QWidget* parent = nullptr);
  ~KTagContainer();

  void setModel(QAbstractItemModel* model);
  void loadTags(const QList<QString>& idList);

  QComboBox* tagCombo();
  const QList<QString> selectedTags();

protected Q_SLOTS:
  void slotRemoveTagWidget();

private:
  KTagContainerPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KTagContainer)
};

#endif
