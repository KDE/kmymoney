/*
 * Copyright 2009       Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#ifndef KBALANCEWARNING_H
#define KBALANCEWARNING_H
// ----------------------------------------------------------------------------
// QT Includes

#include "kmm_base_dialogs_export.h"

#include <QObject>
class QString;
class QWidget;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyAccount;

class KMM_BASE_DIALOGS_EXPORT KBalanceWarning : public QObject
{
  Q_OBJECT
public:
  explicit KBalanceWarning(QObject* parent);
  virtual ~KBalanceWarning();

public Q_SLOTS:
  void slotShowMessage(QWidget* parent, const MyMoneyAccount& account, const QString& msg);

private:
  class Private;
  Private* d;
};

#endif
