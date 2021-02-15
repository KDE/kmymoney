/*
    SPDX-FileCopyrightText: 2015 Christian Dávid <christian-david@web.de>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTREEWIDGETFILTERLINEWIDGET_H
#define KTREEWIDGETFILTERLINEWIDGET_H

#include <KTreeWidgetSearchLineWidget>
#include "kmm_widgets_export.h"

class KMM_WIDGETS_EXPORT KTreeWidgetFilterLineWidget : public KTreeWidgetSearchLineWidget
{
  Q_OBJECT

public:
  explicit KTreeWidgetFilterLineWidget(QWidget* parent = nullptr, QTreeWidget *treeWidget = 0);

protected Q_SLOTS:
  /**
   * @copydoc KTreeWidgetSearchLineWidget::createWidgets()
   *
   * After widgets are created, this version finds the label and renames it to "Filter"
   */
  void createWidgets() final override;
};

#endif // KTREEWIDGETFILTERLINEWIDGET_H
