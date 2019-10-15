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

#include "securitiesmodel.h"

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

struct SecuritiesModel::Private
{
  Private(SecuritiesModel* qq, QObject* parent)
    : q_ptr(qq)
    , parentObject(parent)
  {
  }

  QModelIndex tradingCurrencyIndex(const MyMoneySecurity& security)
  {
    QModelIndex idx;
    if (!security.isCurrency()) {
      idx = MyMoneyFile::instance()->currenciesModel()->indexById(security.tradingCurrency());
      if (!idx.isValid()) {
        idx = q_ptr->indexById(security.tradingCurrency());
      }
    }
    if (!idx.isValid()) {
      qDebug() << "Trading currency" << security.tradingCurrency() << "not found";
    }
    return idx;
  }

  QString tradingCurrency(const MyMoneySecurity& security)
  {
    const auto idx = tradingCurrencyIndex(security);
    return idx.data(eMyMoney::Model::Roles::SecuritySymbolRole).toString();
  }

  SecuritiesModel*    q_ptr;
  QObject*            parentObject;
};

SecuritiesModel::SecuritiesModel(QObject* parent, QUndoStack* undoStack)
  : MyMoneyModel<MyMoneySecurity>(parent, QStringLiteral("E"), SecuritiesModel::ID_SIZE, undoStack)
  , d(new Private(this, parent))
{
  setObjectName(QLatin1String("SecuritiesModel"));
}

SecuritiesModel::~SecuritiesModel()
{
}

int SecuritiesModel::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  return Column::MaxColumns;
}

QVariant SecuritiesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch(section) {
      case Column::Security:
        return i18n("Security");
      case Column::Symbol:
        return i18n("Symbol");
      case Column::Type:
        return i18n("Type");
      case Column::Market:
        return i18n("Market");
      case Column::Currency:
        return i18n("Currency");
      case Column::Fraction:
        return i18n("Fraction");
      default:
        return QVariant();
    }
  }
  return QAbstractItemModel::headerData(section, orientation, role);
}

QVariant SecuritiesModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return QVariant();
  if (index.row() < 0 || index.row() >= rowCount(index.parent()))
    return QVariant();

  QVariant rc;
  const MyMoneySecurity& security = static_cast<TreeItem<MyMoneySecurity>*>(index.internalPointer())->constDataRef();
  MyMoneySecurity tradingCurrency;
  switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
      switch(index.column()) {
        case Column::Security:
          // make sure to never return any displayable text for the dummy entry
          if (!security.id().isEmpty()) {
            return security.name();
          }
          break;

        case Column::Symbol:
          return security.tradingSymbol();

        case Column::Type:
          return security.securityTypeToString(security.securityType());

        case Column::Market:
          if (security.isCurrency())
            return QStringLiteral("ISO 4217");
          return security.tradingMarket();

        case Column::Currency:
          if (!security.isCurrency()) {
            return d->tradingCurrency(security);
          }
          break;

        case Column::Fraction:
          return QStringLiteral("%1").arg(security.smallestAccountFraction());

        default:
          break;
      }
      break;

    case Qt::TextAlignmentRole:
      return QVariant(Qt::AlignLeft | Qt::AlignVCenter);

    case eMyMoney::Model::Roles::IdRole:
      return security.id();

    case eMyMoney::Model::Roles::SecuritySymbolRole:
      return security.tradingSymbol();

    case eMyMoney::Model::SecurityTradingCurrencyIdRole:
      return security.tradingCurrency();

    case eMyMoney::Model::SecurityTradingCurrencyIndexRole:
      return d->tradingCurrencyIndex(security);

    case eMyMoney::Model::SecurityPricePrecisionRole:
      return security.pricePrecision();

    case eMyMoney::Model::SecuritySmallestAccountFractionRole:
      return security.smallestAccountFraction();

    case eMyMoney::Model::SecuritySmallestCashFractionRole:
      return security.smallestCashFraction();
  }
  return QVariant();
}

bool SecuritiesModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if(!index.isValid()) {
    return false;
  }

  qDebug() << "setData(" << index.row() << index.column() << ")" << value << role;
  return QAbstractItemModel::setData(index, value, role);
}

void SecuritiesModel::loadCurrencies(const QMap<QString, MyMoneySecurity>& list)
{
  // make sure this is a currency model
  m_idLeadin.clear();
  m_idMatchExp.setPattern(QStringLiteral("^(\\d+)$"));
  setObjectName(QLatin1String("CurrenciesModel"));

  beginResetModel();
  // first get rid of any existing entries
  clearModelItems();

  const auto itemCount = list.count();
  // reserve an extra slot for the null item
  insertRows(0, itemCount);

  // and don't count loading as a modification
  setDirty(false);

  int row = 0;
  foreach (const auto item, list) {
    static_cast<TreeItem<MyMoneySecurity>*>(index(row, 0).internalPointer())->dataRef() = item;
    ++row;
  }
  endResetModel();

  qDebug() << "Model for currencies loaded with" << rowCount() << "items";
}



void SecuritiesModel::addCurrency(const MyMoneySecurity& currency)
{
  // make sure this is a currency model
  m_idLeadin.clear();
  m_idMatchExp.setPattern(QStringLiteral("^(\\d+)$"));

  if (!currency.id().isEmpty()) {
    const auto idx = indexById(currency.id());
    if (!idx.isValid()) {
      const int row = rowCount();
      insertRows(row, 1);
      const QModelIndex index = SecuritiesModel::index(row, 0);
      static_cast<TreeItem<MyMoneySecurity>*>(index.internalPointer())->dataRef() = currency;
      setDirty();
      emit dataChanged(index, SecuritiesModel::index(row, columnCount()-1));
    } else {
      qDebug() << "Currency with ID" << currency.id() << "already exists";
    }
  } else {
    qDebug() << "Unable to add currency without existing ID";
  }
}
