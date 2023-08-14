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
    Private(OnlineJobsModel* qq)
        : q(qq)
        , headerData(QHash<Columns, QString>({
              {PostDate, i18nc("@title:column", "Date")},
              {AccountName, i18nc("@title:column", "Account")},
              {Action, i18nc("@title:column", "Action")},
              {Destination, i18nc("@title:column", "Destination")},
              {Value, i18nc("@title:column", "Value")},
              {Purpose, i18nc("@title:column", "Purpose")},
              {DestinationAccount, i18nc("@title:column", "Destination Account")},
              {DestinationBic, i18nc("@title:column", "Destination BIC")},
          }))
    {
    }

    QDate youngestDateOnFile()
    {
        QDate date;

        const auto rows = q->rowCount();
        for (int row = rows - 1; row >= 0; --row) {
            const auto idx = q->index(row, 0);
            const onlineJob& job = static_cast<TreeItem<onlineJob>*>(idx.internalPointer())->constDataRef();
            if (job.sendDate().isValid()) {
                if (job.sendDate().date() > date) {
                    date = job.sendDate().date();
                }
            }
        }
        return date;
    }

    OnlineJobsModel* q;
    QHash<Columns, QString> headerData;
};

OnlineJobsModel::OnlineJobsModel(QObject* parent, QUndoStack* undoStack)
    : MyMoneyModel<onlineJob>(parent, QStringLiteral("O"), OnlineJobsModel::ID_SIZE, undoStack)
    , d(new Private(this))
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
    if (orientation == Qt::Horizontal) {
        switch (role) {
        case Qt::DisplayRole:
            return d->headerData.value(static_cast<Columns>(section));
        }
    }
    if (orientation == Qt::Vertical && role == Qt::SizeHintRole) {
        return QSize(10, 10);
    }
    return MyMoneyModelBase::headerData(section, orientation, role);
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
            try {
                return job.task()->jobTypeName();
            } catch (const onlineJob::badTaskCast&) {
            } catch (const onlineJob::emptyTask&) {
            } catch (const MyMoneyException&) {
            }
            break;

        case Value:
            // Show credit transfer data
            try {
                onlineJobTyped<creditTransfer> transfer(job);
                return MyMoneyUtils::formatMoney(transfer.task()->value(), transfer.task()->currency());

            } catch (const onlineJob::badTaskCast&) {
            } catch (const onlineJob::emptyTask&) {
            } catch (const MyMoneyException&) {
            }
            break;

        case Destination:
            try {
                onlineJobTyped<creditTransfer> transfer(job);
                const payeeIdentifierTyped<payeeIdentifiers::ibanBic> ibanBic(transfer.constTask()->beneficiary());
                return ibanBic->ownerName();
            } catch (const onlineJob::badTaskCast&) {
                return i18nc("Unknown payee in online task", "Unknown");
            } catch (const onlineJob::emptyTask&) {
            } catch (const MyMoneyException&) {
            }
            break;

        case Purpose:
            try {
                return job.purpose().remove(QLatin1Char('\n'));
            } catch (const onlineJob::badTaskCast&) {
            } catch (const onlineJob::emptyTask&) {
            } catch (const MyMoneyException&) {
            }
            break;

        case DestinationAccount:
            try {
                onlineJobTyped<creditTransfer> transfer(job);
                const payeeIdentifierTyped<payeeIdentifiers::ibanBic> ibanBic(transfer.constTask()->beneficiary());
                return ibanBic->paperformatIban();
            } catch (const onlineJob::badTaskCast&) {
                return i18nc("Unknown payee in online task", "Unknown");
            } catch (const onlineJob::emptyTask&) {
            } catch (const MyMoneyException&) {
            }
            break;

        case DestinationBic:
            try {
                onlineJobTyped<creditTransfer> transfer(job);
                const payeeIdentifierTyped<payeeIdentifiers::ibanBic> ibanBic(transfer.constTask()->beneficiary());
                return ibanBic->bic();
            } catch (const onlineJob::badTaskCast&) {
                return i18nc("Unknown payee in online task", "Unknown");
            } catch (const onlineJob::emptyTask&) {
            } catch (const MyMoneyException&) {
            }
            break;
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
            } else {
                return Icons::get(Icon::TaskPending);
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
                             MyMoneyUtils::formatDateTime(job.bankAnswerDate()));
            case eMyMoney::OnlineJob::sendingState::sendingError:
                return i18nc("@info:tooltip online job list status Arg 1 is a date/time",
                             "Sending this job failed (tried on %1).",
                             MyMoneyUtils::formatDateTime(job.sendDate()));
            case eMyMoney::OnlineJob::sendingState::abortedByUser:
                return i18nc("@info:tooltip online job list status", "Sending this job was manually aborted.");
            case eMyMoney::OnlineJob::sendingState::rejectedByBank:
                return i18nc("@info:tooltip online job list status Arg 1 is a date/time",
                             "The bank rejected this job on %1.",
                             MyMoneyUtils::formatDateTime(job.bankAnswerDate()));
            case eMyMoney::OnlineJob::sendingState::noBankAnswer:
                if (job.sendDate().isValid())
                    return i18nc("@info:tooltip online job list status Arg 1 is a date/time",
                                 "The bank accepted this job on %1.",
                                 MyMoneyUtils::formatDateTime(job.sendDate()));
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

    case eMyMoney::Model::OnlineJobPurposeRole:
        return job.purpose();

    case eMyMoney::Model::OnlineJobPostDateRole:
        if ((job.bankAnswerState() == eMyMoney::OnlineJob::sendingState::noBankAnswer) && !job.sendDate().isValid() && job.isValid()) {
            return d->youngestDateOnFile().addDays(1);
        }
        return job.sendDate().date();

    case eMyMoney::Model::OnlineJobValueAsDoubleRole:
        try {
            onlineJobTyped<creditTransfer> transfer(job);
            return transfer.task()->value().toDouble();

        } catch (const onlineJob::badTaskCast&) {
        } catch (const MyMoneyException&) {
        }
        break;
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
