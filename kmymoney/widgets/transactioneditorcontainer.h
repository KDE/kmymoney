/*
 * Copyright 2006       Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#ifndef TRANSACTIONEDITORCONTAINER_H
#define TRANSACTIONEDITORCONTAINER_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QTableWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace KMyMoneyRegister { class Transaction; }

class TransactionEditorContainer : public QTableWidget
{
  Q_OBJECT
  Q_DISABLE_COPY(TransactionEditorContainer)

public:
  explicit TransactionEditorContainer(QWidget* parent);
  virtual ~TransactionEditorContainer();

  virtual void arrangeEditWidgets(QMap<QString, QWidget*>& editWidgets, KMyMoneyRegister::Transaction* t) = 0;
  virtual void removeEditWidgets(QMap<QString, QWidget*>& editWidgets) = 0;
  virtual void tabOrder(QWidgetList& tabOrderWidgets, KMyMoneyRegister::Transaction* t) const = 0;
  // FIXME remove tabbar
  // virtual int action(QMap<QString, QWidget*>& editWidgets) const = 0;
  // virtual void setProtectedAction(QMap<QString, QWidget*>& editWidgets, ProtectedAction action) = 0;

Q_SIGNALS:
  void geometriesUpdated();

protected Q_SLOTS:
  void updateGeometries() final override;
};

#endif
