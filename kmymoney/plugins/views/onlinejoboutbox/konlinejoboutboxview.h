/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * SPDX-FileCopyrightText: 2013 Christian Dávid <christian-david@web.de>
 *
 *SPDX-License-Identifier: GPL-2.0-or-laterrg/licenses/>.
 */

#ifndef KONLINEJOBOUTBOXVIEW_H
#define KONLINEJOBOUTBOXVIEW_H

// ----------------------------------------------------------------------------
// Qt Headers

// ----------------------------------------------------------------------------
// KDE Headers

class KXMLGUIClient;
class KXMLGUIFactory;

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

  void createActions(KXMLGUIFactory* guiFactory, KXMLGUIClient* guiClient);
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
