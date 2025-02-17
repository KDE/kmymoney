/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "onlinepricemodel.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>
#include <QDebug>
#include <QString>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyenums.h"
#include "mymoneyutils.h"

QString OnlinePriceEntry::id() const
{
    return m_id;
}

void OnlinePriceEntry::setSymbol(const QString& symbol)
{
    m_symbol = symbol;
}

QString OnlinePriceEntry::symbol() const
{
    return m_symbol;
}

void OnlinePriceEntry::setName(const QString& name)
{
    m_name = name;
}

QString OnlinePriceEntry::name() const
{
    return m_name;
}

void OnlinePriceEntry::setDate(const QDate& date)
{
    m_date = date;
}

QDate OnlinePriceEntry::date() const
{
    return m_date;
}

void OnlinePriceEntry::setPrice(const QString& price)
{
    m_price = price;
}

QString OnlinePriceEntry::price() const
{
    return m_price;
}

void OnlinePriceEntry::setSource(const QString& source)
{
    m_source = source;
}

QString OnlinePriceEntry::source() const
{
    return m_source;
}

void OnlinePriceEntry::setDirty()
{
    m_dirty = true;
}

bool OnlinePriceEntry::isDirty() const
{
    return m_dirty;
}

bool OnlinePriceEntry::operator!=(const OnlinePriceEntry& right) const
{
    return (m_id != right.id()) || (m_date != right.date()) || (m_source != right.source()) || (m_price != right.price()) || (m_symbol != right.symbol())
        || (m_name != right.name());
}

struct OnlinePriceModel::Private {
    Private()
        : headerData(QHash<Column, QString>({{Symbol, i18nc("@title:column External identifier (symbol)", "ID")},
                                             {Name, i18nc("@title:column Name of security", "Name")},
                                             {Price, i18nc("@title:column Price", "Price")},
                                             {Date, i18nc("@title:column Date for price", "Date")},
                                             {Id, i18nc("@title:column KMyMoney internal identifier", "Internal ID")},
                                             {Source, i18nc("@title:column Online quote source", "Source")}}))
    {
    }

    QHash<Column, QString> headerData;
};

OnlinePriceModel::OnlinePriceModel(QObject* parent, QUndoStack* undoStack)
    : MyMoneyModel<OnlinePriceEntry>(parent, QStringLiteral("p"), OnlinePriceModel::ID_SIZE, undoStack)
    , d(new Private)
{
    setObjectName(QLatin1String("OnlinePriceModel"));
}

OnlinePriceModel::~OnlinePriceModel()
{
}

int OnlinePriceModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return d->headerData.count();
}

QVariant OnlinePriceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return d->headerData.value(static_cast<Column>(section));
    }
    return MyMoneyModelBase::headerData(section, orientation, role);
}

Qt::ItemFlags OnlinePriceModel::flags(const QModelIndex& index) const
{
    auto currentFlags = MyMoneyModelBase::flags(index);

    if (index.column() == Source) {
        // if the ID contains a blank, it is a currency rate and we allow
        // to modify the source
        if (index.data(eMyMoney::Model::IdRole).toString().contains(' ')) {
            currentFlags |= Qt::ItemIsEditable;
        }
    }
    return currentFlags;
}

QVariant OnlinePriceModel::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid())
        return QVariant();
    if (idx.row() < 0 || idx.row() >= rowCount(idx.parent()))
        return QVariant();

    const OnlinePriceEntry& onlinePriceEntry = static_cast<TreeItem<OnlinePriceEntry>*>(idx.internalPointer())->constDataRef();

    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        switch (idx.column()) {
        case Symbol:
            return onlinePriceEntry.symbol();

        case Id:
            return onlinePriceEntry.id();

        case Name:
            return onlinePriceEntry.name();

        case Date:
            return MyMoneyUtils::formatDate(onlinePriceEntry.date());

        case Price:
            return onlinePriceEntry.price();

        case Source:
            return onlinePriceEntry.source();
        }
        break;

    case Qt::TextAlignmentRole:
        switch (idx.column()) {
        case Price:
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);

        default:
            break;
        }
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);

    case eMyMoney::Model::IdRole:
        return onlinePriceEntry.id();

    case eMyMoney::Model::IsDirtyRole:
        return onlinePriceEntry.isDirty();

    case eMyMoney::Model::PriceDateRole:
        return onlinePriceEntry.date();

    default:
        if (role >= Qt::UserRole)
            qDebug() << "OnlinePriceModel::data(), role" << role << "offset" << role - Qt::UserRole << "not implemented";
        break;
    }
    return QVariant();
}

bool OnlinePriceModel::setData(const QModelIndex& idx, const QVariant& value, int role)
{
    if (!idx.isValid()) {
        return false;
    }

    OnlinePriceEntry& onlinePriceEntry = static_cast<TreeItem<OnlinePriceEntry>*>(idx.internalPointer())->dataRef();
    const auto newValue = value.toString();
    const auto newDate = value.toDate();

    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        switch (idx.column()) {
        case Date:
            if (onlinePriceEntry.date() != newDate) {
                onlinePriceEntry.setDate(newDate);
                onlinePriceEntry.setDirty();
                Q_EMIT dataChanged(idx, idx);
            }
            return true;

        case Price:
            if (onlinePriceEntry.price() != newValue) {
                onlinePriceEntry.setPrice(newValue);
                onlinePriceEntry.setDirty();
                Q_EMIT dataChanged(idx, idx);
            }
            return true;

        case Source:
            if (onlinePriceEntry.source() != newValue) {
                onlinePriceEntry.setSource(newValue);
                onlinePriceEntry.setDirty();
                Q_EMIT dataChanged(idx, idx);
            }
            return true;

        default:
            break;
        }
    }

    qDebug() << "OnlinePriceModel::setData(" << idx.row() << idx.column() << ")" << value << role;
    return QAbstractItemModel::setData(idx, value, role);
}

void OnlinePriceModel::addOnlinePrice(const QString& id,
                                      const QString& symbol,
                                      const QString& name,
                                      const QString& price,
                                      const QDate& date,
                                      const QString& source)
{
    OnlinePriceEntry newEntry(id, OnlinePriceEntry());
    newEntry.setSymbol(symbol);
    newEntry.setName(name);
    newEntry.setDate(date);
    newEntry.setPrice(price);
    newEntry.setSource(source);

    // find the entry or the position where to insert a new one
    QModelIndex idx = MyMoneyModelBase::lowerBound(newEntry.id());
    int row = idx.row();
    if (!idx.isValid()) {
        row = rowCount();
    }
    if (idx.data(eMyMoney::Model::IdRole).toString() != newEntry.id()) {
        insertRow(row);
    }

    if (static_cast<TreeItem<OnlinePriceEntry>*>(index(row, 0).internalPointer())->dataRef() != newEntry) {
        static_cast<TreeItem<OnlinePriceEntry>*>(index(row, 0).internalPointer())->dataRef() = newEntry;
        Q_EMIT dataChanged(idx, index(row, columnCount() - 1));
        setDirty();
    }
}
