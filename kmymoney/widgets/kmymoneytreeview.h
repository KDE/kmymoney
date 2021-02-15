/*
 * SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
