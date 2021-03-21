/*
    SPDX-FileCopyrightText: 2009 Cristian Onet onet.cristian @gmail.com
    SPDX-FileCopyrightText: 2018 Martin Preuss aquamaniac @users.sourceforge.net
    SPDX-FileCopyrightText: 2010 Thomas Baumgart ipwizard @users.sourceforge.net
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifdef HAVE_CONFIG_H
# include <config-kmymoney.h>
#endif

#include "kbjoblist.h"
#include <assert.h>

#include <QString>
#include <QWidget>
#include <QHeaderView>

#include <KLocalizedString>


KBJobListViewItem::KBJobListViewItem(KBJobListView *parent,
                                     AB_TRANSACTION *j)
    : QTreeWidgetItem(parent)
    , _job(j)
{
    assert(j);
    _populate();
}



KBJobListViewItem::KBJobListViewItem(const KBJobListViewItem &item)
    : QTreeWidgetItem(item)
    , _job(0)
{

    if (item._job) {
        _job = item._job;
    }
}

KBJobListViewItem::~KBJobListViewItem()
{
}



AB_TRANSACTION *KBJobListViewItem::getJob()
{
    return _job;
}


void KBJobListViewItem::_populate()
{
    QString tmp;
    int i;

    assert(_job);

    i = 0;

    // job id
    setText(i++, QString::number(AB_Transaction_GetIdForApplication(_job)));

    // job type
    switch (AB_Transaction_GetCommand(_job)) {
    case AB_Transaction_CommandGetBalance:
        tmp = i18n("Get Balance");
        break;
    case AB_Transaction_CommandGetTransactions:
        tmp = i18n("Get Transactions");
        break;
    case AB_Transaction_CommandTransfer:
        tmp = i18n("Transfer");
        break;
    case AB_Transaction_CommandDebitNote:
        tmp = i18n("Debit Note");
        break;
    default:
        tmp = i18nc("Unknown job type", "(unknown)");
        break;
    }
    setText(i++, tmp);

    // bank name
    tmp = AB_Transaction_GetLocalBankCode(_job);
    if (tmp.isEmpty())
        tmp = i18nc("Unknown bank code", "(unknown)");
    setText(i++, tmp);

    // account name
    tmp = AB_Transaction_GetLocalAccountNumber(_job);
    if (tmp.isEmpty())
        tmp = i18nc("Unknown account number", "(unknown)");
    setText(i++, tmp);

    // status
    switch (AB_Transaction_GetStatus(_job)) {
    case AB_Transaction_StatusNone:
    case AB_Transaction_StatusUnknown:
        tmp = i18nc("Status of the job", "new");
        break;
    case AB_Transaction_StatusAccepted:
        tmp = i18nc("Status of the job", "accepted");
        break;
    case AB_Transaction_StatusRejected:
        tmp = i18nc("Status of the job", "rejected");
        break;
    case AB_Transaction_StatusPending:
        tmp = i18nc("Status of the job", "pending");
        break;
    case AB_Transaction_StatusSending:
        tmp = i18nc("Status of the job", "sending");
        break;
    case AB_Transaction_StatusAutoReconciled:
        tmp = i18nc("Status of the job", "reconciled (auto)");
        break;
    case AB_Transaction_StatusManuallyReconciled:
        tmp = i18nc("Status of the job", "reconciled (manual)");
        break;
    case AB_Transaction_StatusRevoked:
        tmp = i18nc("Status of the job", "revoked");
        break;
    case AB_Transaction_StatusAborted:
        tmp = i18nc("Status of the job", "aborted");
        break;
    case AB_Transaction_StatusEnqueued:
        tmp = i18nc("Status of the job", "enqueued");
        break;
    case AB_Transaction_StatusError:
        tmp = i18nc("Status of the job", "error");
        break;
    default:
        tmp = i18nc("Status of the job", "(unknown)");
        break;
    }
    setText(i++, tmp);
}









KBJobListView::KBJobListView(QWidget *parent)
    : QTreeWidget(parent)
{
    setColumnCount(5);
    setAllColumnsShowFocus(true);
    setHeaderLabels(QStringList() << i18n("Job Id")
                    << i18n("Job Type")
                    << i18n("Institute")
                    << i18n("Account")
                    << i18n("Status"));

    header()->setSortIndicatorShown(true);
}



KBJobListView::~KBJobListView()
{
}



void KBJobListView::addJob(AB_TRANSACTION *j)
{
    new KBJobListViewItem(this, j);
}



void KBJobListView::addJobs(const std::list<AB_TRANSACTION*> &js)
{
    std::list<AB_TRANSACTION*>::const_iterator it;

    for (it = js.begin(); it != js.end(); ++it) {
        new KBJobListViewItem(this, *it);
    } /* for */
}



AB_TRANSACTION *KBJobListView::getCurrentJob()
{
    KBJobListViewItem *entry;

    entry = dynamic_cast<KBJobListViewItem*>(currentItem());
    if (!entry) {
        fprintf(stderr, "No item selected in list.\n");
        return 0;
    }
    return entry->getJob();
}



std::list<AB_TRANSACTION*> KBJobListView::getSelectedJobs()
{
    std::list<AB_TRANSACTION*> js;
    KBJobListViewItem *entry;
    // Create an iterator and give the listview as argument
    QTreeWidgetItemIterator it(this, QTreeWidgetItemIterator::Selected);
    // iterate through all selected items of the listview
    for (; *it; ++it) {
        entry = dynamic_cast<KBJobListViewItem*>(*it);
        if (entry)
            js.push_back(entry->getJob());
    } // for

    return js;
}

