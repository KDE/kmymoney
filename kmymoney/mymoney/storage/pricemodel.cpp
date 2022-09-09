/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "pricemodel.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDebug>
#include <QString>
#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"

PriceEntry::PriceEntry(const MyMoneyPrice& price)
    : MyMoneyPrice(price)
    , m_id(PriceModel::createId(price.from(), price.to(), price.date()))
{
}


struct PriceModel::Private
{
    Private()
        : headerData(QHash<Column, QString> ({
        { Commodity, i18n("Commodity") },
        { StockName, i18n("Stock name") },
        { Currency, i18n("Currency") },
        { Date, i18n("Date") },
        { Price, i18n("Price") },
        { Source, i18n("Source") },
    }))
    {
    }


    QHash<Column, QString>          headerData;
};



PriceModel::PriceModel(QObject* parent, QUndoStack* undoStack)
    : MyMoneyModel<PriceEntry>(parent, QStringLiteral("p"), PriceModel::ID_SIZE, undoStack)
    , d(new Private)
{
    setObjectName(QLatin1String("PriceModel"));
}

PriceModel::~PriceModel()
{
}

QString PriceModel::createId(const QString& from, const QString& to, const QDate& date)
{
    return QString("%1-%2-%3").arg(from, to, date.toString(Qt::ISODate));
}

QPair<QString,QString> PriceEntry::pricePair() const
{
    return qMakePair<QString,QString>(from(), to());
}

int PriceModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return d->headerData.count();
}

QVariant PriceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return d->headerData.value(static_cast<Column>(section));
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

QVariant PriceModel::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid())
        return QVariant();
    if (idx.row() < 0 || idx.row() >= rowCount(idx.parent()))
        return QVariant();

    const PriceEntry& priceEntry = static_cast<TreeItem<PriceEntry>*>(idx.internalPointer())->constDataRef();

    switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        switch(idx.column()) {
        case Commodity:
            return QString("From");

        case StockName:
            return QString("Stockname");

        case Currency:
            return QString("To");

        case Date:
            return QLocale().toString(priceEntry.date(), QLocale::ShortFormat);

        case Price:
            /// @todo fix precision handling
            return priceEntry.rate(priceEntry.to()).formatMoney("", 2);

        case Source:
            return priceEntry.source();
        }
        break;

    case Qt::TextAlignmentRole:
        switch( idx.column()) {
        case Price:
            return QVariant(Qt::AlignRight | Qt::AlignTop);

        default:
            break;
        }
        return QVariant(Qt::AlignLeft | Qt::AlignTop);

    case eMyMoney::Model::Roles::IdRole:
        return priceEntry.id();


    default:
        if (role >= Qt::UserRole)
            qDebug() << "PriceModel::data(), role" << role << "offset" << role-Qt::UserRole << "not implemented";
        break;
    }
    return QVariant();
}

bool PriceModel::setData(const QModelIndex& idx, const QVariant& value, int role)
{
    if(!idx.isValid()) {
        return false;
    }

    qDebug() << "setData(" << idx.row() << idx.column() << ")" << value << role;
    return QAbstractItemModel::setData(idx, value, role);
}

void PriceModel::load(const QMap<MyMoneySecurityPair, MyMoneyPriceEntries>& list)
{
    QElapsedTimer t;

    t.start();
    beginResetModel();
    // first get rid of any existing entries
    clearModelItems();

    // create the number of required items
    int itemCount = 0;
    for (const auto& item : list) {
        itemCount += item.count();
    }
    insertRows(0, itemCount);

    int row = 0;
    QMap<MyMoneySecurityPair, MyMoneyPriceEntries>::const_iterator itPairs;
    for (itPairs = list.constBegin(); itPairs != list.constEnd(); ++itPairs) {
        QDate lastDate(1900, 1, 1);
        for (const auto& priceInfo : *itPairs) {
            if (lastDate > priceInfo.date()) {
                qDebug() << "Price loader: dates not sorted as needed" << priceInfo.date() << "older than" << lastDate;
            }
            PriceEntry newEntry(priceInfo);
            static_cast<TreeItem<PriceEntry>*>(index(row, 0).internalPointer())->dataRef() = newEntry;
            lastDate = priceInfo.date();
            ++row;
        }
    }
    endResetModel();

    Q_EMIT modelLoaded();

    // and don't count loading as a modification
    setDirty(false);

    qDebug() << "Model for prices loaded with" << row << "items in" << t.elapsed() << "ms";
}

void PriceModel::addPrice(const MyMoneyPrice& price)
{
    PriceEntry newEntry(price);

    // find the entry or the position where to insert a new one
    QModelIndex idx = MyMoneyModelBase::lowerBound(newEntry.id());
    int row = idx.row();
    if (!idx.isValid()) {
        row = rowCount();
    }
    if (idx.data(eMyMoney::Model::IdRole).toString() != newEntry.id()) {
        insertRow(row);
    }

    if (static_cast<TreeItem<PriceEntry>*>(index(row, 0).internalPointer())->dataRef() != newEntry) {
        static_cast<TreeItem<PriceEntry>*>(index(row, 0).internalPointer())->dataRef() = newEntry;
        Q_EMIT dataChanged(idx, index(row, columnCount()-1));
        setDirty();
    }
}

void PriceModel::removePrice(const MyMoneyPrice& price)
{
    PriceEntry newEntry(price);

    // find the entry or the position where to remove the entry
    QModelIndex idx = MyMoneyModelBase::lowerBound(newEntry.id());
    if (idx.data(eMyMoney::Model::IdRole).toString() == newEntry.id()) {
        removeRow(idx.row());
        setDirty();
    }
}

MyMoneyPrice PriceModel::price(const QString& from, const QString& to, const QDate& _date, bool exactDate) const
{
    // if no valid date is passed, we use today's date.
    const QDate &date = _date.isValid() ? _date : QDate::currentDate();
    QString fullId(createId(from, to, date));
    QString pricePair(createId(from, to, QDate()));

    // check if we have a price for this price pair at all
    QModelIndex idx = MyMoneyModelBase::lowerBound(pricePair);
    if (!idx.isValid()) {
        return {};
    }

    // find the last matching price
    idx = lowerBound(fullId, idx.row(), rowCount()-1);
    int row = idx.row();
    if (!idx.isValid()) {
        // if we found a price pair but get an invalid index here
        // we don't have an exact match. In case we look for the
        // exact match, we can return right away.
        if (exactDate) {
            return {};
        }
        // the last row is the one we look for. We
        row = rowCount()-1;
        auto priceEntry = static_cast<TreeItem<PriceEntry>*>(index(row, 0).internalPointer())->data();
        return std::move(priceEntry);
    }

    auto priceEntry = static_cast<TreeItem<PriceEntry>*>(index(row, 0).internalPointer())->data();

    // in case we have an exact match, we report it
    if (priceEntry.id() == fullId) {
        return std::move(priceEntry);
    }

    // in case the caller wanted the exact date we are done
    if (exactDate) {
        return {};
    }

    // since lower bound returns the first item with a larger key (we already know that
    // the exact key is not present) we need to return the previous item (the model
    // is not empty so there is one)
    if (row > 0) {
        priceEntry = static_cast<TreeItem<PriceEntry>*>(index(row-1, 0).internalPointer())->data();
        if ((priceEntry.from() == from) && (priceEntry.to() == to)) {
            return std::move(priceEntry);
        }
    }
    return {};
}

MyMoneyPriceList PriceModel::priceList() const
{
    MyMoneyPriceList priceList;
    MyMoneyPriceEntries entries;
    QPair<QString,QString> pricePair;

    QModelIndex idx;
    const auto rows = rowCount();
    for (int row = 0; row < rows; ++row) {
        idx = index(row, 0);
        const auto priceEntry = static_cast<TreeItem<PriceEntry>*>(index(idx.row(), 0).internalPointer())->constDataRef();
        if (priceEntry.pricePair() != pricePair) {
            if (!entries.isEmpty() && !pricePair.first.isEmpty()) {
                priceList[pricePair] = entries;
            }
            pricePair = priceEntry.pricePair();
            entries.clear();
        }
        entries[priceEntry.date()] = priceEntry;
    }
    if (!entries.isEmpty() && !pricePair.first.isEmpty()) {
        priceList[pricePair] = entries;
    }
    return priceList;
}
