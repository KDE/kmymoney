/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2013 Christian Dávid <christian-david@web.de>
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

#ifndef KONLINEJOBOUTBOX_H
#define KONLINEJOBOUTBOX_H

#include "kmymoneyviewbase.h"

#include "onlinejob.h"

class QModelIndex;

namespace KMyMoneyPlugin { class OnlinePlugin; }

class KOnlineJobOutboxPrivate;
class KOnlineJobOutbox : public KMyMoneyViewBase
{
  Q_OBJECT

public:
  explicit KOnlineJobOutbox(QWidget *parent = 0);
  ~KOnlineJobOutbox() override;

  void executeCustomAction(eView::Action action) override;

  void updateActions(const MyMoneyObject& obj);

  QStringList selectedOnlineJobs() const;

  void setOnlinePlugins(QMap<QString, KMyMoneyPlugin::OnlinePlugin*>& plugins);

public Q_SLOTS:
  void slotSelectByObject(const MyMoneyObject& obj, eView::Intent intent) override;

Q_SIGNALS:
  void sendJobs(QList<onlineJob>);
  void editJob(QString);
  void newCreditTransfer();

protected:
  void showEvent(QShowEvent* event) override;
  void contextMenuEvent(QContextMenuEvent*) override;

private:
  Q_DECLARE_PRIVATE(KOnlineJobOutbox)

private Q_SLOTS:
  void updateNewCreditTransferButton();
  void updateButtonState() const;

  void slotRemoveJob();

  /** @brief If any job is selected, send it. Send all valid jobs otherwise. */
  void slotSendJobs();

  /** @brief Send all sendable online jobs */
  void slotSendAllSendableJobs();

  /** @brief Send only the selected jobs */
  void slotSendSelectedJobs();

  void slotEditJob();
  void slotEditJob(const QModelIndex&);

  void slotOnlineJobSave(onlineJob job);
  void slotOnlineJobSend(onlineJob job);
  void slotOnlineJobSend(QList<onlineJob> jobs);

  void slotOnlineJobLog();
  void slotOnlineJobLog(const QStringList& onlineJobIds);
  void slotNewCreditTransfer();
};

#endif // KONLINEJOBOUTBOX_H
