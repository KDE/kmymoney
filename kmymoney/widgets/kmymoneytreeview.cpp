/*
 * Copyright 2020       Thomas Baumgart <tbaumgart@kde.org>
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

#include "kmymoneytreeview.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QMouseEvent>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

KMyMoneyTreeView::KMyMoneyTreeView(QWidget *parent)
  : QTreeView(parent)
{
}

KMyMoneyTreeView::~KMyMoneyTreeView()
{
}

void KMyMoneyTreeView::mouseDoubleClickEvent(QMouseEvent *event)
{
  emit startEdit(currentIndex());
  event->accept();
}

void KMyMoneyTreeView::keyPressEvent(QKeyEvent* event)
{
  if (event->modifiers() == Qt::NoModifier) {
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
      emit startEdit(currentIndex());
      event->accept();
      return;
    }
  }
  QTreeView::keyPressEvent(event);
}
