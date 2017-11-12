/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2015 Christian Dávid <christian-david@web.de>
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

#ifndef ONLINEJOBMESSAGESVIEW_H
#define ONLINEJOBMESSAGESVIEW_H

#include <QWidget>

#include "kmm_widgets_export.h"

class QAbstractItemModel;
namespace Ui
{
  class onlineJobMessageView;
}

class KMM_WIDGETS_EXPORT onlineJobMessagesView : public QWidget
{
  Q_OBJECT
  Q_DISABLE_COPY(onlineJobMessagesView)

public:
  explicit onlineJobMessagesView(QWidget* parent = nullptr);
  ~onlineJobMessagesView();
  void setModel(QAbstractItemModel* model);

protected:
  Ui::onlineJobMessageView* ui;
};

#endif // ONLINEJOBMESSAGESVIEW_H
