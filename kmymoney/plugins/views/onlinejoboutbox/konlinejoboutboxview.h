/*
    SPDX-FileCopyrightText: 2013 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KONLINEJOBOUTBOXVIEW_H
#define KONLINEJOBOUTBOXVIEW_H

#include "kmymoneyviewbase.h"

#include "onlinejob.h"

class QModelIndex;

namespace KMyMoneyPlugin {
class OnlinePlugin;
}

class KOnlineJobOutboxViewPrivate;
class KOnlineJobOutboxView : public KMyMoneyViewBase
{
    Q_OBJECT

public:
    explicit KOnlineJobOutboxView(QWidget *parent = 0);
    ~KOnlineJobOutboxView() override;

    void executeCustomAction(eView::Action action) override;

    void updateActions(const MyMoneyObject& obj);

    QStringList selectedOnlineJobs() const;

public Q_SLOTS:
    void slotSelectByObject(const MyMoneyObject& obj, eView::Intent intent) override;
    void slotSelectByVariant(const QVariantList& variant, eView::Intent intent) override;

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

#endif // KONLINEJOBOUTBOXVIEW_H
