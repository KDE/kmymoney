/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2015  Christian David <christian-david@web.de>
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

#include "ktreewidgetfilterlinewidget.h"

#include <QLabel>

#include <KLocalizedString>

KTreeWidgetFilterLineWidget::KTreeWidgetFilterLineWidget(QWidget* parent, QTreeWidget* treeWidget)
    : KTreeWidgetSearchLineWidget(parent, treeWidget)
{
}

void KTreeWidgetFilterLineWidget::createWidgets()
{
  KTreeWidgetSearchLineWidget::createWidgets();

  // The layout pointer is stored in the private class, so we do not have access to it directly
  // => use findChild()
  QLabel* label = findChild<QLabel*>();
  if (!label)
    return;

  label->setText(i18nc("Filter widget label", "Fi&lter:"));
}
