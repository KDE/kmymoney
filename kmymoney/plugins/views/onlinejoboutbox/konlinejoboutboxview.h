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

#ifndef KONLINEJOBOUTBOXVIEW_H
#define KONLINEJOBOUTBOXVIEW_H

// ----------------------------------------------------------------------------
// Qt Headers

// ----------------------------------------------------------------------------
// KDE Headers

class KXMLGUIClient;

// ----------------------------------------------------------------------------
// Project Headers

#include "kmymoneyviewbase.h"
#include "onlinejob.h"

class QModelIndex;

namespace KMyMoneyPlugin { class OnlinePlugin; }

namespace eMenu {
  enum class OnlineAction {
    LogOnlineJob,
    AccountCreditTransfer,
    DeleteOnlineJob,
    EditOnlineJob,
    SendOnlineJobs
  };
  inline uint qHash(const OnlineAction key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
};

class KOnlineJobOutboxViewPrivate;
class SelectedObjects;
class KOnlineJobOutboxView : public KMyMoneyViewBase
{
  Q_OBJECT

public:
  explicit KOnlineJobOutboxView(QWidget *parent = 0);
  ~KOnlineJobOutboxView() override;

  void executeCustomAction(eView::Action action) override;

  void createActions(KXMLGUIClient* guiClient);
  void removeActions();

  QStringList selectedOnlineJobs() const;

public Q_SLOTS:
  void slotSelectByVariant(const QVariantList& variant, eView::Intent intent) override;

  void updateActions(const SelectedObjects& selections) override;

Q_SIGNALS:
  void sendJobs(QList<onlineJob>);
  void editJob(QString);
  void newCreditTransfer();

protected:
  void showEvent(QShowEvent* event) override;
  void contextMenuEvent(QContextMenuEvent*) override;

private:
  Q_DECLARE_PRIVATE(KOnlineJobOutboxView)

private Q_SLOTS:
  void updateSelection();

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

#endif // KONLINEJOBOUTBOXVIEW_H
