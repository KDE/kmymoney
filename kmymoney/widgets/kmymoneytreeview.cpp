/*
 * SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
