/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "parametersmodel.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDebug>
#include <QString>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes


ParameterItem::ParameterItem()
{
}

ParameterItem::ParameterItem(const QString& key, const QString& value)
    : m_id(key)
    , m_value(value)
{
}



struct ParametersModel::Private
{
    Private() {}
};

ParametersModel::ParametersModel(QObject* parent, QUndoStack* undoStack)
    : MyMoneyModel<ParameterItem>(parent, QStringLiteral("p"), ParametersModel::ID_SIZE, undoStack)
    , d(new Private)
{
    setObjectName(QLatin1String("ParametersModel"));
}

ParametersModel::~ParametersModel()
{
}

int ParametersModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 2;
}

QVariant ParametersModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch(section) {
        case ParameterKey:
            return i18nc("ParameterModel key", "Key");
        case ParameterValue:
            return i18nc("ParameterModel value", "Value");
        }
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

QVariant ParametersModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (index.row() < 0 || index.row() >= rowCount(index.parent()))
        return QVariant();

    const ParameterItem& parameter = static_cast<TreeItem<ParameterItem>*>(index.internalPointer())->constDataRef();
    switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        switch(index.column()) {
        case ParameterKey:
            // make sure to never return any displayable text for the dummy entry
            if (!parameter.id().isEmpty()) {
                return parameter.id();
            }
            return QVariant();

        case ParameterValue:
            // make sure to never return any displayable text for the dummy entry
            if (!parameter.id().isEmpty()) {
                return parameter.value();
            }
            return QVariant();
        }
        return QVariant();

    case Qt::TextAlignmentRole:
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);

    case eMyMoney::Model::IdRole:
    case eMyMoney::Model::ParameterKeyRole:
        return parameter.id();

    case eMyMoney::Model::ParameterValueRole:
        return parameter.value();
    }
    return QVariant();
}

bool ParametersModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if(!index.isValid()) {
        return false;
    }

    qDebug() << "setData(" << index.row() << index.column() << ")" << value << role;
    return QAbstractItemModel::setData(index, value, role);
}

void ParametersModel::addItem(const QString& key, const QString& val)
{
    const auto idx = indexById(key);
    int row;
    if (idx.isValid()) {
        row = idx.row();

    } else {
        row = rowCount();
        insertRow(row);
    }
    ParameterItem item(key, val);
    static_cast<TreeItem<ParameterItem>*>(index(row, 0).internalPointer())->dataRef() = item;
}

void ParametersModel::deleteItem(const QString& key)
{
    const auto idx = indexById(key);
    if (!idx.isValid()) {
        removeRow(idx.row());
    }
}

void ParametersModel::load(const QMap<QString, QString>& pairs)
{
    beginResetModel();
    // first get rid of any existing entries
    clearModelItems();

    // create the number of required items
    const auto itemCount = pairs.count();
    insertRows(0, itemCount);

    int row = 0;
    for (auto it = pairs.constBegin(); it != pairs.constEnd(); ++it) {
        ParameterItem item(it.key(), it.value());
        static_cast<TreeItem<ParameterItem>*>(index(row, 0).internalPointer())->dataRef() = item;
        ++row;
    }

    // and don't count loading as a modification
    setDirty(false);

    endResetModel();

    Q_EMIT modelLoaded();

    qDebug() << "Model for parameters loaded with" << itemCount << "items";
}

QMap<QString, QString> ParametersModel::pairs() const
{
    QMap<QString, QString> result;
    const int rows = rowCount();
    for (int row = 0; row < rows; ++row) {
        const auto& item = static_cast<TreeItem<ParameterItem>*>(index(row, 0).internalPointer())->constDataRef();
        result.insert(item.id(), item.value());
    }
    return result;
}
