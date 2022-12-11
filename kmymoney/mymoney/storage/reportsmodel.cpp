/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "reportsmodel.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDebug>
#include <QString>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes


struct ReportsModel::Private
{
    Private() {}
};

ReportsModel::ReportsModel(QObject* parent, QUndoStack* undoStack)
    : MyMoneyModel<MyMoneyReport>(parent, QStringLiteral("R"), ReportsModel::ID_SIZE, undoStack)
    , d(new Private)
{
    setObjectName(QLatin1String("ReportsModel"));
}

ReportsModel::~ReportsModel()
{
}

int ReportsModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant ReportsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch(section) {
        case 0:
            return i18nc("Reportname", "Name");
            break;
        }
    }
    return MyMoneyModelBase::headerData(section, orientation, role);
}

QVariant ReportsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (index.row() < 0 || index.row() >= rowCount(index.parent()))
        return QVariant();

    const MyMoneyReport& report = static_cast<TreeItem<MyMoneyReport>*>(index.internalPointer())->constDataRef();
    switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        switch(index.column()) {
        case ReportName:
            return report.name();
        default:
            return QStringLiteral("not yet implemented");
        }
        break;

    case Qt::TextAlignmentRole:
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        break;

    case eMyMoney::Model::Roles::IdRole:
        return report.id();
        break;
    }
    return QVariant();
}

bool ReportsModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if(!index.isValid()) {
        return false;
    }

    qDebug() << "setData(" << index.row() << index.column() << ")" << value << role;
    return QAbstractItemModel::setData(index, value, role);
}
