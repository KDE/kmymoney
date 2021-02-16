/*
    SPDX-FileCopyrightText: 2013 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SEPAONLINETASKSLOADER_H
#define SEPAONLINETASKSLOADER_H

#include <kmymoney/plugins/onlinepluginextended.h>

class sepaOnlineTasksLoader : public QObject, public KMyMoneyPlugin::onlineTaskFactory
{
  Q_OBJECT
  Q_INTERFACES(KMyMoneyPlugin::onlineTaskFactory)

public:
  explicit sepaOnlineTasksLoader(QObject* parent = nullptr, const QVariantList& options = QVariantList{});
  onlineTask* createOnlineTask(const QString& taskId) const final override;

};

#endif // SEPAONLINETASKSLOADER_H
