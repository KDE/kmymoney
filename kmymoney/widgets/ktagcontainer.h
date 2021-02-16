/*
    SPDX-FileCopyrightText: 2009-2016 Cristian One ț <onet.cristian@gmail.com>
    SPDX-FileCopyrightText: 2009-2010 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-FileCopyrightText: 2010-2016 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTAGCONTAINER_H
#define KTAGCONTAINER_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmm_widgets_export.h"

class KMyMoneyTagCombo;
class MyMoneyTag;

/**
  * This widget contain a KMyMoneyTagCombo widget and 0 or more KTagLabel widgets
  * call KMyMoneyTagCombo.loadTags with the correct list whenever a new KTagLabel is created or
  * deleted by removing or adding the relative tag
  *
  * @author Alessandro Russo
  */
class KTagContainerPrivate;
class KMM_WIDGETS_EXPORT KTagContainer : public QWidget
{
  Q_OBJECT
  Q_DISABLE_COPY(KTagContainer)

public:
  explicit KTagContainer(QWidget* parent = nullptr);
  ~KTagContainer();

  void loadTags(const QList<MyMoneyTag>& list);
  KMyMoneyTagCombo* tagCombo();
  const QList<QString> selectedTags();
  void addTagWidget(const QString& id);
  void RemoveAllTagWidgets();

protected Q_SLOTS:
  void slotRemoveTagWidget();
  void slotAddTagWidget();

private:
  KTagContainerPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KTagContainer)
};

#endif
