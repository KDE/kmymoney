/*
 * SPDX-FileCopyrightText: 2015-2016 Christian Dávid <christian-david@web.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "payeeidentifiermodel.h"

#include <limits>

#include <KLocalizedString>

#include "mymoneyfile.h"
#include "mymoneyexception.h"

/**
 * @brief create unique value for QModelIndex::internalId() to indicate "not set"
 */
static constexpr decltype(reinterpret_cast<QModelIndex*>(0)->internalId()) invalidParent = std::numeric_limits<decltype(reinterpret_cast<QModelIndex*>(0)->internalId())>::max();

payeeIdentifierModel::payeeIdentifierModel(QObject* parent)
    : QAbstractItemModel(parent),
    m_payeeIdentifierIds(),
    m_typeFilter()
{

}

void payeeIdentifierModel::setTypeFilter(QStringList filter)
{
  m_typeFilter = filter;
  loadData();
}

void payeeIdentifierModel::setTypeFilter(QString type)
{
  setTypeFilter(QStringList(type));
}


void payeeIdentifierModel::loadData()
{
  beginResetModel();

  const QList<MyMoneyPayee> payees = MyMoneyFile::instance()->payeeList();
  m_payeeIdentifierIds.clear();
  m_payeeIdentifierIds.reserve(payees.count());
  Q_FOREACH(const MyMoneyPayee& payee, payees) {
    m_payeeIdentifierIds.append(payee.id());
  }
  endResetModel();
}

MyMoneyPayee payeeIdentifierModel::payeeByIndex(const QModelIndex& index) const
{
  if (index.isValid() && index.row() >= 0 && index.row() < m_payeeIdentifierIds.count()) {
    try {
      return MyMoneyFile::instance()->payee(m_payeeIdentifierIds.at(index.row()));
    } catch (const MyMoneyException &) {
    }
  }

  return MyMoneyPayee();
}

QVariant payeeIdentifierModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return QVariant();

  const auto isPayeeIdentifierValid = index.parent().isValid();
  if (role == payeeIdentifierModel::isPayeeIdentifier)
    return isPayeeIdentifierValid;

  const MyMoneyPayee payee = (isPayeeIdentifierValid) ? payeeByIndex(index.parent()) : payeeByIndex(index);


  if (role == payeeName || (!isPayeeIdentifierValid && role == Qt::DisplayRole)) {
    // Return data for MyMoneyPayee
    return payee.name();
  } else if (isPayeeIdentifierValid) {
    // Return data for payeeIdentifier
    if (index.row() >= 0 && static_cast<unsigned int>(index.row()) < payee.payeeIdentifierCount()) {
      ::payeeIdentifier ident = payee.payeeIdentifier(index.row());

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
    }
  }

  return QVariant();
}

Qt::ItemFlags payeeIdentifierModel::flags(const QModelIndex &index) const
{
  Q_UNUSED(index)
#if 0
  if (!index.parent().isValid()) {
    if (payeeByIndex(index).payeeIdentifierCount() > 0)
      return Qt::ItemIsEnabled;
  }
#endif

  return (Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}

/**
 * @internal The internalId of QModelIndex is set to the row of the parent or invalidParent if there is no
 * parent.
 *
 * @todo Qt5: the type of the internal id changed!
 */
QModelIndex payeeIdentifierModel::index(int row, int column, const QModelIndex &parent) const
{
  if (parent.isValid())
    return createIndex(row, column, parent.row());
  return createIndex(row, column, invalidParent);
}

int payeeIdentifierModel::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  return 1;
}

int payeeIdentifierModel::rowCount(const QModelIndex& parent) const
{
  if (parent.isValid()) {
    if (parent.internalId() != invalidParent)
      return 0;
    return payeeByIndex(parent).payeeIdentifierCount();
  }
  return m_payeeIdentifierIds.count();
}

QModelIndex payeeIdentifierModel::parent(const QModelIndex& child) const
{
  if (child.internalId() != invalidParent)
    return createIndex(child.internalId(), 0, invalidParent);
  return QModelIndex();
}
