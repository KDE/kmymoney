/***************************************************************************
                          ktagcontainer.h  -  description
                             -------------------
    begin                : Mon Jan 09 2010
    copyright            : (C) 2010 by Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Cristian Onet <cristian.onet@gmail.com>
                           Alvaro Soliverez <asoliverez@gmail.com>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
