/*
    SPDX-FileCopyrightText: 2019-2022 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "onlinejobsmodel.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>
#include <QDebug>
#include <QIcon>
#include <QString>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "accountsmodel.h"
#include "icons.h"
#include "mymoneyaccount.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneymoney.h"
#include "mymoneysecurity.h"
#include "mymoneyutils.h"
#include "onlinejobtyped.h"

#include "ibanbic/ibanbic.h"
#include "payeeidentifier/payeeidentifiertyped.h"
#include "tasks/credittransfer.h"
#include "tasks/onlinetask.h"

using namespace Icons;

struct OnlineJobsModel::Private
{
    Private() {}
};

OnlineJobsModel::OnlineJobsModel(QObject* parent, QUndoStack* undoStack)
    : MyMoneyModel<onlineJob>(parent, QStringLiteral("O"), OnlineJobsModel::ID_SIZE, undoStack)
    , d(new Private)
{
    setObjectName(QLatin1String("OnlineJobsModel"));
}

OnlineJobsModel::~OnlineJobsModel()
{
}

int OnlineJobsModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return MaxColumns;
}

QVariant OnlineJobsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case PostDate:
            return i18nc("@title:column", "Date");
        case AccountName:
            return i18nc("@title:column", "Account");
        case Action:
            return i18nc("@title:column", "Action");
        case Destination:
            return i18nc("@title:column", "Destination");
        case Value:
            return i18nc("@title:column", "Value");
        }
    }
    return {};
}

QVariant OnlineJobsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (index.row() < 0 || index.row() >= rowCount(index.parent()))
        return QVariant();

    const onlineJob& job = static_cast<TreeItem<onlineJob>*>(index.internalPointer())->constDataRef();
    switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        switch(index.column()) {
        case PostDate:
            return MyMoneyUtils::formatDate(job.sendDate().date());

        case AccountName:
            return MyMoneyFile::instance()->accountsModel()->itemById(job.responsibleAccount()).name();

        case Action:
            return job.task()->jobTypeName();

        case Value:
            // Show credit transfer data
            try {
                onlineJobTyped<creditTransfer> transfer(job);
                return MyMoneyUtils::formatMoney(transfer.task()->value(), transfer.task()->currency());

            } catch (const onlineJob::badTaskCast&) {
            } catch (const MyMoneyException&) {
            }

        case Destination:
            try {
                onlineJobTyped<creditTransfer> transfer(job);
                const payeeIdentifierTyped<payeeIdentifiers::ibanBic> ibanBic(transfer.constTask()->beneficiary());
                return ibanBic->ownerName();
            } catch (const onlineJob::badTaskCast&) {
                return i18nc("Unknown payee in online task", "Unknown");
            } catch (const MyMoneyException&) {
            }

        default:
            return QStringLiteral("not yet implemented");
        }
        break;

    case Qt::DecorationRole:
        if (index.column() == Columns::AccountName) {
            if (job.isLocked()) {
                return Icons::get(Icon::TaskOngoing);
            }

            switch (job.bankAnswerState()) {
            case eMyMoney::OnlineJob::sendingState::acceptedByBank:
                return Icons::get(Icon::TaskComplete);

            case eMyMoney::OnlineJob::sendingState::sendingError:
            case eMyMoney::OnlineJob::sendingState::abortedByUser:
            case eMyMoney::OnlineJob::sendingState::rejectedByBank:
                return Icons::get(Icon::TaskReject);

            case eMyMoney::OnlineJob::sendingState::noBankAnswer:
                break;
            }

            if (job.sendDate().isValid()) {
                return Icons::get(Icon::TaskAccepted);
            } else if (!job.isValid()) {
                return Icons::get(Icon::DialogWarning);
            }
        }
        break;

    case Qt::ToolTipRole:
        if (index.column() == Columns::AccountName) {
            if (job.isLocked())
                return i18nc("@info:tooltip online job list status", "Job is being processed at the moment.");

            switch (job.bankAnswerState()) {
            case eMyMoney::OnlineJob::sendingState::acceptedByBank:
                return i18nc("@info:tooltip online job list status Arg 1 is a date/time",
                             "This job was accepted by the bank on %1.",
                             job.bankAnswerDate().toString(Qt::DefaultLocaleShortDate));
            case eMyMoney::OnlineJob::sendingState::sendingError:
                return i18nc("@info:tooltip online job list status Arg 1 is a date/time",
                             "Sending this job failed (tried on %1).",
                             job.sendDate().toString(Qt::DefaultLocaleShortDate));
            case eMyMoney::OnlineJob::sendingState::abortedByUser:
                return i18nc("@info:tooltip online job list status", "Sending this job was manually aborted.");
            case eMyMoney::OnlineJob::sendingState::rejectedByBank:
                return i18nc("@info:tooltip online job list status Arg 1 is a date/time",
                             "The bank rejected this job on %1.",
                             job.bankAnswerDate().toString(Qt::DefaultLocaleShortDate));
            case eMyMoney::OnlineJob::sendingState::noBankAnswer:
                if (job.sendDate().isValid())
                    return i18nc("@info:tooltip online job list status Arg 1 is a date/time",
                                 "The bank accepted this job on %1.",
                                 job.sendDate().toString(Qt::DefaultLocaleShortDate));
                else if (!job.isValid())
                    return i18nc("@info:tooltip online job list status", "This job needs further editing and cannot be sent therefore.");
                else
                    return i18nc("@info:tooltip online job list status", "This job is ready for sending.");
            }
        }
        break;

    case Qt::TextAlignmentRole:
        if (index.column() == Columns::Value) {
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        }
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);

    case eMyMoney::Model::Roles::IdRole:
        return job.id();

    case eMyMoney::Model::OnlineJobRole:
        return QVariant::fromValue(job);

    case eMyMoney::Model::OnlineJobEditableRole:
        return job.isEditable();

    case eMyMoney::Model::OnlineJobLockedRole:
        return job.isLocked();

    case eMyMoney::Model::OnlineJobSendableRole:
        return (job.bankAnswerState() == eMyMoney::OnlineJob::sendingState::noBankAnswer) && !job.sendDate().isValid() && job.isValid();

    case eMyMoney::Model::OnlineJobTaskIidRole:
        return job.taskIid();

    case eMyMoney::Model::OnlineJobSendDateRole:
        return job.sendDate();
    }
    return {};
}

bool OnlineJobsModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if(!index.isValid()) {
        return false;
    }

    qDebug() << "setData(" << index.row() << index.column() << ")" << value << role;
    return QAbstractItemModel::setData(index, value, role);
}
