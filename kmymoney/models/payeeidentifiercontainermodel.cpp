/*
    SPDX-FileCopyrightText: 2014-2015 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "payeeidentifiercontainermodel.h"
#include "payeeidentifier/payeeidentifier.h"
#include "payeeidentifier/ibanbic/ibanbic.h"
#include "payeeidentifier/nationalaccount/nationalaccount.h"

#include <KLocalizedString>

payeeIdentifierContainerModel::payeeIdentifierContainerModel(QObject* parent)
    : QAbstractListModel(parent),
      m_data(QSharedPointer<MyMoneyPayeeIdentifierContainer>())
{
}

QVariant payeeIdentifierContainerModel::data(const QModelIndex& index, int role) const
{
    // Needed for the selection box and it prevents a crash if index is out of range
    if (m_data.isNull() || index.row() >= rowCount(index.parent()) - 1)
        return QVariant();

    const ::payeeIdentifier ident = m_data->payeeIdentifiers().at(index.row());

    if (role == payeeIdentifier) {
        return QVariant::fromValue< ::payeeIdentifier >(ident);
    } else if (ident.isNull()) {
        return QVariant();
    } else if (role == payeeIdentifierType) {
        return ident.iid();
    } else if (role == Qt::DisplayRole) {
        // The custom delegates won't ask for this role
        return QVariant::fromValue(i18n("The plugin to show this information could not be found."));
    }
    return QVariant();
}

bool payeeIdentifierContainerModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!m_data.isNull() && role == payeeIdentifier) {
        ::payeeIdentifier ident = value.value< ::payeeIdentifier >();
        if (index.row() == rowCount(index.parent()) - 1) {
            // The new row will be the last but one
            beginInsertRows(index.parent(), index.row() - 1, index.row() - 1);
            m_data->addPayeeIdentifier(ident);
            endInsertRows();
        } else {
            m_data->modifyPayeeIdentifier(index.row(), ident);
            Q_EMIT dataChanged(createIndex(index.row(), 0), createIndex(index.row(), 0));
        }
        return true;
    }
    return QAbstractItemModel::setData(index, value, role);
}

Qt::ItemFlags payeeIdentifierContainerModel::flags(const QModelIndex& index) const
{
    static const QVector<QString> editableDelegates {
        payeeIdentifiers::ibanBic::staticPayeeIdentifierIid(),
        payeeIdentifiers::nationalAccount::staticPayeeIdentifierIid()
    };
    auto flags = QAbstractItemModel::flags(index) | Qt::ItemIsDragEnabled;
    const auto type = data(index, payeeIdentifierType).toString();

    // type.isEmpty() means the type selection can be shown
    if (!type.isEmpty() && editableDelegates.contains(type))
        flags |= Qt::ItemIsEditable;
    return flags;
}

int payeeIdentifierContainerModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    if (m_data.isNull())
        return 0;
    // Always a row more which creates new entries
    return m_data->payeeIdentifiers().count() + 1;
}

/** @brief unused at the moment */
bool payeeIdentifierContainerModel::insertRows(int row, int count, const QModelIndex& parent)
{
    Q_UNUSED(row);
    Q_UNUSED(count);
    Q_UNUSED(parent);
    return false;
}

bool payeeIdentifierContainerModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (m_data.isNull())
        return false;

    if (count < 1 || row + count >= rowCount(parent))
        return false;

    beginRemoveRows(parent, row, row + count - 1);
    for (int i = row; i < row + count; ++i) {
        m_data->removePayeeIdentifier(i);
    }
    endRemoveRows();
    return true;
}

void payeeIdentifierContainerModel::setSource(const MyMoneyPayeeIdentifierContainer data)
{
    beginResetModel();
    m_data = QSharedPointer<MyMoneyPayeeIdentifierContainer>(new MyMoneyPayeeIdentifierContainer(data));
    endResetModel();
}

void payeeIdentifierContainerModel::closeSource()
{
    beginResetModel();
    m_data = QSharedPointer<MyMoneyPayeeIdentifierContainer>();
    endResetModel();
}

QList< ::payeeIdentifier > payeeIdentifierContainerModel::identifiers() const
{
    if (m_data.isNull())
        return QList< ::payeeIdentifier >();
    return m_data->payeeIdentifiers();
}
