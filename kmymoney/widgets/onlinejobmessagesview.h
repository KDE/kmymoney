/*
 * SPDX-FileCopyrightText: 2015 Christian Dávid <christian-david@web.de>
 * SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

// krazy:excludeall=dpointer

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
