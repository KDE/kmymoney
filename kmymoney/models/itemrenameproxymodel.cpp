/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "itemrenameproxymodel.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyenums.h"
#include "mymoneyfile.h"

ItemRenameProxyModel::ItemRenameProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_renameColumn(0)
    , m_referenceFilter(eAllItem)
{
}

ItemRenameProxyModel::~ItemRenameProxyModel()
{
}

void ItemRenameProxyModel::setRenameColumn(int column)
{
    m_renameColumn = column;
}

bool ItemRenameProxyModel::setData(const QModelIndex& idx, const QVariant& value, int role)
{
    if (idx.column() == m_renameColumn && role == Qt::EditRole) {
        qDebug() << "Rename to" << value.toString();
        Q_EMIT renameItem(idx, value);
        return true;
    }
    return QSortFilterProxyModel::setData(idx, value, role);
}

Qt::ItemFlags ItemRenameProxyModel::flags(const QModelIndex& idx) const
{
    auto flags = QSortFilterProxyModel::flags(idx);
    if (idx.column() == m_renameColumn) {
        flags |= Qt::ItemIsEditable;
    }
    return flags;
}

void ItemRenameProxyModel::setReferenceFilter(const QVariant& filterType)
{
    const uint type = filterType.toUInt();
    if (type < eMaxItems) {
        setReferenceFilter(static_cast<ReferenceFilterType>(type));
    }
}

void ItemRenameProxyModel::setReferenceFilter(ItemRenameProxyModel::ReferenceFilterType filterType)
{
    if (m_referenceFilter != filterType) {
        m_referenceFilter = filterType;
        invalidateFilter();
    }
}

ItemRenameProxyModel::ReferenceFilterType ItemRenameProxyModel::referenceFilter() const
{
    return m_referenceFilter;
}

bool ItemRenameProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    if (m_referenceFilter != eAllItem) {
        const auto idx = sourceModel()->index(source_row, 0, source_parent);
        const auto itemId = idx.data(eMyMoney::Model::IdRole).toString();
        if (!itemId.isEmpty()) {
            QVariant rc;
            const auto objectList = MyMoneyFile::instance()->referencedObjects();
            switch(m_referenceFilter) {
            case eReferencedItems:
                if (!objectList.contains(itemId))
                    return false;
                break;
            case eUnReferencedItems:
                if (objectList.contains(itemId))
                    return false;
                break;
            case eOpenedItems:
                rc = idx.data(eMyMoney::Model::ClosedRole);
                if (rc.isValid() && (rc.toBool() == true)) {
                    return false;
                }
                break;
            case eClosedItems:
                rc = idx.data(eMyMoney::Model::ClosedRole);
                if (rc.isValid() && (rc.toBool() == false)) {
                    return false;
                }
                break;
            default:
                break;
            }
        }
    }
    return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}
