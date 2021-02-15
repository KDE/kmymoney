/*
 * SPDX-FileCopyrightText: 2009-2016 Cristian One ț <onet.cristian@gmail.com>
 * SPDX-FileCopyrightText: 2009-2010 Alvaro Soliverez <asoliverez@gmail.com>
 * SPDX-FileCopyrightText: 2010-2020 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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

#include "kmm_base_widgets_export.h"

class MyMoneyTag;

/**
  * This widget contains a QComboBox widget and 0 or more KTagLabel widgets
  *
  * @author Alessandro Russo, Thomas Baumgart
  */
class KTagContainerPrivate;
class KMM_BASE_WIDGETS_EXPORT KTagContainer : public QWidget
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

Q_SIGNALS:
  void tagsChanged(const QStringList& tagIdList);

private:
  KTagContainerPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KTagContainer)
};

#endif
