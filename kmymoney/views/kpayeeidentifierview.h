/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2014 Christian Dávid <christian-david@web.de>
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

#ifndef KPAYEEIDENTIFIERVIEW_H
#define KPAYEEIDENTIFIERVIEW_H

#include <QWidget>

#include "widgets/styleditemdelegateforwarder.h"

class payeeIdentifier;
class MyMoneyPayeeIdentifierContainer;

namespace Ui { class KPayeeIdentifierView; }

class KPayeeIdentifierView : public QWidget
{
  Q_OBJECT

public:
  explicit KPayeeIdentifierView(QWidget* parent);
  ~KPayeeIdentifierView();
  QList<payeeIdentifier> identifiers() const;

  void closeSource();

Q_SIGNALS:
  void dataChanged();

public Q_SLOTS:
  void setSource(MyMoneyPayeeIdentifierContainer data);

private Q_SLOTS:
  void removeSelected();

private:
  Ui::KPayeeIdentifierView* ui;
};

class payeeIdentifierDelegate : public StyledItemDelegateForwarder
{
  Q_OBJECT
public:
  explicit payeeIdentifierDelegate(QObject* parent = 0);
  virtual QAbstractItemDelegate* getItemDelegate(const QModelIndex& index) const override;
};

#endif // KPAYEEIDENTIFIERVIEW_H
