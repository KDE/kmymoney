/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "schedulesjournalmodel.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QDate>
#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyutils.h"
#include "mymoneyfile.h"
#include "schedulesmodel.h"

struct SchedulesJournalModel::Private
{
    Private()
        : previewPeriod(0)
        , updateRequested(false)
        , showPlannedDate(false)
    {}

    int  previewPeriod;
    bool updateRequested;
    bool showPlannedDate;
};

SchedulesJournalModel::SchedulesJournalModel(QObject* parent, QUndoStack* undoStack)
    : JournalModel(QLatin1String("SCH"), parent, undoStack)
    , d(new Private)
{
    setObjectName(QLatin1String("SchedulesJournalModel"));
}

SchedulesJournalModel::~SchedulesJournalModel()
{
}

Qt::ItemFlags SchedulesJournalModel::flags(const QModelIndex& index) const
{
    if (index.isValid()) {
        return (Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    }
    return Qt::NoItemFlags;
}

QVariant SchedulesJournalModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (index.row() < 0 || index.row() >= rowCount(index.parent()))
        return QVariant();


    const JournalEntry& entry = static_cast<TreeItem<JournalEntry>*>(index.internalPointer())->constDataRef();
    switch (role) {
    case eMyMoney::Model::ScheduleIsOverdueRole:
        return entry.transaction().value(QStringLiteral("kmm-is-overdue")).compare(QStringLiteral("yes")) == 0;

    case eMyMoney::Model::ScheduleIsOverdueSinceRole:
        return QDate::fromString(entry.transaction().value(QLatin1String("kmm-overdue-since")), Qt::ISODate);

    case eMyMoney::Model::TransactionScheduleRole:
        return true;

    case eMyMoney::Model::TransactionScheduleIdRole:
        return entry.transaction().id();

    case eMyMoney::Model::TransactionIsImportedRole:
    case eMyMoney::Model::JournalSplitIsMatchedRole:
        return false;

    default:
        break;
    }
    return JournalModel::data(index, role);
}

bool SchedulesJournalModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    return JournalModel::setData(index, value, role);
#if 0
    if (!index.isValid()) {
        return false;
    }

    if (!index.isValid())
        return false;
    if (index.row() < 0 || index.row() >= rowCount(index.parent()))
        return false;

    JournalEntry& entry = static_cast<TreeItem<JournalEntry>*>(index.internalPointer())->dataRef();

    bool rc = true;
    switch(role) {
    case eMyMoney::Model::PayeeNameRole:
    case Qt::DisplayRole:
    case Qt::EditRole:
        // make sure to never return any displayable text for the dummy entry
        break;

    default:
        rc = false;
        break;
    }

    if (rc) {
        setDirty();
        const auto topLeft = SchedulesJournalModel::index(index.row(), 0);
        const auto bottomRight = SchedulesJournalModel::index(index.row(), columnCount()-1);
        Q_EMIT dataChanged(topLeft, bottomRight);
    }
    return rc;
#endif
}

void SchedulesJournalModel::load(const QMap<QString, MyMoneyTransaction>& list)
{
    Q_UNUSED(list);
    qDebug() << Q_FUNC_INFO << "must never be called";
}

void SchedulesJournalModel::updateData()
{
    // register the update of the model for the
    // next event loop run so that the model is
    // updated only once even if load() is called
    // multiple times in a row. This is enough,
    // since the model is always loaded completely
    // (no partial updates)
    if (!d->updateRequested) {
        d->updateRequested = true;
        QMetaObject::invokeMethod(this, "doLoad", Qt::QueuedConnection);
    }
}

void SchedulesJournalModel::doLoad()
{
    // create scheduled transactions which have a scheduled postdate
    // within the next 'period' days.
    const auto endDate = QDate::currentDate().addDays(d->previewPeriod);
    QList<MyMoneySchedule> scheduleList = MyMoneyFile::instance()->scheduleList();

    // in case we don't have a single schedule, there are certainly no transactions
    if (scheduleList.isEmpty()) {
        JournalModel::unload();

    } else {
        QMap<QString, QSharedPointer<MyMoneyTransaction>> transactionList;

        while (!scheduleList.isEmpty()) {
            MyMoneySchedule& s = scheduleList.first();
            for (;;) {
                if (s.isFinished() || s.adjustedNextDueDate() > endDate) {
                    break;
                }

                MyMoneyTransaction t(s.id(), MyMoneyFile::instance()->scheduledTransaction(s));
                if (s.isOverdue()) {
                    if (!d->showPlannedDate) {
                        // if the transaction is scheduled and overdue, it can't
                        // certainly be posted in the past. So we take today's date
                        // as the alternative
                        qDebug() << "Adjust scheduled transaction" << s.name() << "from" << t.postDate() << "to" << s.adjustedDate(QDate::currentDate(), eMyMoney::Schedule::WeekendOption::MoveAfter) << s.weekendOptionToString(eMyMoney::Schedule::WeekendOption::MoveAfter);
                        t.setPostDate(s.adjustedDate(QDate::currentDate(), eMyMoney::Schedule::WeekendOption::MoveAfter));
                    } else {
                        t.setPostDate(s.adjustedNextDueDate());
                    }
                    t.setValue(QLatin1String("kmm-is-overdue"), QLatin1String("yes"));
                    t.setValue(QLatin1String("kmm-overdue-since"), s.adjustedNextDueDate().toString(Qt::ISODate));
                } else {
                    t.setPostDate(s.adjustedNextDueDate());
                }

                // add transaction to the list
                transactionList.insert(t.uniqueSortKey(), QSharedPointer<MyMoneyTransaction>(new MyMoneyTransaction(t)));

                // keep track of this payment locally (not in the engine)
                if (s.isOverdue() && !d->showPlannedDate) {
                    s.setLastPayment(QDate::currentDate());
                } else {
                    s.setLastPayment(s.nextDueDate());
                }

                // if this is a one time schedule, we can bail out here as we're done
                if (s.occurrence() == eMyMoney::Schedule::Occurrence::Once)
                    break;

                // for all others, we check if the next payment date is still 'in range'
                QDate nextDueDate = s.nextPayment(s.nextDueDate());
                if (nextDueDate.isValid()) {
                    s.setNextDueDate(nextDueDate);
                } else {
                    break;
                }
            }
            scheduleList.pop_front();
        }
        JournalModel::load(transactionList);
    }
    // free up the lock for the next update
    d->updateRequested = false;
}

void SchedulesJournalModel::setPreviewPeriod(int days)
{
    if (d->previewPeriod != days) {
        d->previewPeriod = days;
        updateData();
    }
}

void SchedulesJournalModel::setShowPlannedDate(bool showPlannedDate)
{
    if (d->showPlannedDate != showPlannedDate) {
        d->showPlannedDate = showPlannedDate;
        updateData();
    }
}
