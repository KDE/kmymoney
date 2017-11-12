/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2015  Christian David <christian-david@web.de>
 * (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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
  virtual void createWidgets();
};

#endif // KTREEWIDGETFILTERLINEWIDGET_H
