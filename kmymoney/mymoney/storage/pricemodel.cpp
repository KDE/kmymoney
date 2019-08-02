/*
 * Copyright 2019       Thomas Baumgart <tbaumgart@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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



PriceModel::PriceModel(QObject* parent)
  : MyMoneyModel<PriceEntry>(parent, QStringLiteral("p"), PriceModel::ID_SIZE)
  , d(new Private)
{
  setObjectName(QLatin1String("PriceModel"));
}

PriceModel::~PriceModel()
{
}

QString PriceModel::createId(const QString& from, const QString& to, const QDate& date)
{
  return QString("%1-%2-%3").arg(date.toString(Qt::ISODate), from, to);
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

  const MyMoneyPrice& priceEntry = static_cast<TreeItem<MyMoneyPrice>*>(idx.internalPointer())->constDataRef();

  switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
      switch(idx.column()) {
        case Commodity:
          return QString("From");

        case Currency:
          return QString("To");

        case Date:
          return QLocale().toString(priceEntry.date(), QLocale::ShortFormat);

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
      /// @todo return an id
      return QString();


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
  beginResetModel();
  // first get rid of any existing entries
  clearModelItems();

  // create the number of required items
  int itemCount = 0;
  foreach (const auto& item, list) {
    itemCount += item.count();
  }
  insertRows(0, itemCount);

  int row = 0;
  QMap<MyMoneySecurityPair, MyMoneyPriceEntries>::const_iterator itPairs;
  for (itPairs = list.constBegin(); itPairs != list.constEnd(); ++itPairs) {
    QDate lastDate(1900, 1, 1);
    foreach(auto priceInfo, *itPairs) {
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

  emit modelLoaded();

  // and don't count loading as a modification
  setDirty(false);

  qDebug() << "Model for prices loaded with" << rowCount() << "items";
}

void PriceModel::addPrice(MyMoneyPrice& price)
{
  PriceEntry newEntry(price);

  // find the entry or the position where to insert a new one
  QModelIndex idx = MyMoneyModelBase::lowerBound(newEntry.id());
  if (idx.data(eMyMoney::Model::IdRole).toString() != newEntry.id()) {
    insertRow(idx.row());
  }

  if (static_cast<TreeItem<PriceEntry>*>(index(idx.row(), 0).internalPointer())->dataRef() != newEntry) {
    static_cast<TreeItem<PriceEntry>*>(index(idx.row(), 0).internalPointer())->dataRef() = newEntry;
    emit dataChanged(idx, index(idx.row(), columnCount()));
  }
}

void PriceModel::removePrice(const MyMoneyPrice& price)
{
  PriceEntry newEntry(price);

  // find the entry or the position where to insert a new one
  QModelIndex idx = MyMoneyModelBase::lowerBound(newEntry.id());
  if (idx.data(eMyMoney::Model::IdRole).toString() != newEntry.id()) {
    removeRow(idx.row());
  }
}

MyMoneyPrice PriceModel::price(const QString& from, const QString& to, const QDate& _date, bool exactDate) const
{
  // if no valid date is passed, we use today's date.
  const QDate &date = _date.isValid() ? _date : QDate::currentDate();
  QString id(createId(from, to, date));

  // find the last matching price
  QModelIndex idx = MyMoneyModelBase::lowerBound(id);
  if (!idx.isValid()) {
    return MyMoneyPrice();
  }
  auto priceEntry = static_cast<TreeItem<PriceEntry>*>(index(idx.row(), 0).internalPointer())->data();

  // in case we have an exact match, we report it
  if (priceEntry.id() == id) {
    return std::move(priceEntry);
  }

  // in case the caller wanted the exact date we are done
  if (exactDate) {
    return MyMoneyPrice();
  }

  // since lower bound returns the first item with a larger key (we already know that key is not present)
  // we need to return the previous item (the model is not empty so there is one)
  idx = index(idx.row()-1, 0);
  if (idx.isValid()) {
    priceEntry = static_cast<TreeItem<PriceEntry>*>(index(idx.row(), 0).internalPointer())->data();
    if ((priceEntry.from() == from) && (priceEntry.to() == to)) {
      return std::move(priceEntry);
    }
  }
  return MyMoneyPrice();
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
