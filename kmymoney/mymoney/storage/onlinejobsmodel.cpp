/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "onlinejobsmodel.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDebug>
#include <QString>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "accountsmodel.h"
#include "mymoneyaccount.h"

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
    return 1;
}

QVariant OnlineJobsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch(section) {
        case 0:
            return i18nc("OnlineJob", "Name");
            break;
        }
    }
    return QAbstractItemModel::headerData(section, orientation, role);
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
        case AccountName:
            return MyMoneyFile::instance()->accountsModel()->itemById(job.responsibleAccount()).name();
        default:
            return QStringLiteral("not yet implemented");
        }
        break;

    case Qt::TextAlignmentRole:
        if (index.column() == Columns::Value) {
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        }
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        break;

    case eMyMoney::Model::Roles::IdRole:
        return job.id();
        break;
    }
    return QVariant();
}

bool OnlineJobsModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if(!index.isValid()) {
        return false;
    }

    qDebug() << "setData(" << index.row() << index.column() << ")" << value << role;
    return QAbstractItemModel::setData(index, value, role);
}
