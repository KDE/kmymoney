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

#ifndef KMYMONEYTREEVIEW_H
#define KMYMONEYTREEVIEW_H

#include "kmm_base_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QTreeView>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

/**
  * This class overrides a standard QTreeView in such a way that
  * it emits the signal startEdit upon a mouse double click event
  * or a Return/Enter key press and suppresses further processing
  * of the event.
  */
class KMM_BASE_WIDGETS_EXPORT KMyMoneyTreeView : public QTreeView
{
  Q_OBJECT
  Q_DISABLE_COPY(KMyMoneyTreeView)

public:
  explicit KMyMoneyTreeView(QWidget* parent = nullptr);
  ~KMyMoneyTreeView();

protected:
  void mouseDoubleClickEvent(QMouseEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;

Q_SIGNALS:
  void startEdit(const QModelIndex& idx) const;
};

#endif // KMYMONEYTREEVIEW_H
