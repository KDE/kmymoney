/*
 * Copyright 2019-2020  Thomas Baumgart <tbaumgart@kde.org>
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

#include "mymoneymodelbase.h"

// ----------------------------------------------------------------------------
// Qt Includes

#include <QSortFilterProxyModel>
#include <QIdentityProxyModel>
#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KConcatenateRowsProxyModel>           // Qt5.13+ use QConcatenateTablesProxyModel
#include <KDescendantsProxyModel>

// ----------------------------------------------------------------------------
// Project Includes


MyMoneyModelBase::MyMoneyModelBase(QObject* parent, const QString& idLeadin, quint8 idSize)
  : QAbstractItemModel(parent)
  , m_nextId(0)
  , m_idLeadin(idLeadin)
  , m_idSize(idSize)
  , m_dirty(false)
  , m_blockedSignals(false)
  , m_idMatchExp(QStringLiteral("^%1(\\d+)$").arg(m_idLeadin))
{
  connect(this, &QAbstractItemModel::modelReset, this, &MyMoneyModelBase::updateReferencedObjects);
}

MyMoneyModelBase::~MyMoneyModelBase()
{
}

QModelIndexList MyMoneyModelBase::indexListByName(const QString& name, const QModelIndex& parent) const
{
  return match(index(0, 0, parent), Qt::DisplayRole, name, 1, Qt::MatchFixedString | Qt::MatchCaseSensitive);
}

const QAbstractItemModel* MyMoneyModelBase::baseModel(const QModelIndex& idx)
{
  return mapToBaseSource(idx).model();
}

QModelIndex MyMoneyModelBase::mapToBaseSource(const QModelIndex& _idx)
{
  QModelIndex                       idx(_idx);
  const QSortFilterProxyModel*      sortFilterModel;
  const QIdentityProxyModel*        identityModel;
  const KConcatenateRowsProxyModel* concatModel;
  const KDescendantsProxyModel*     descendantsModel;
  do {
    if (( sortFilterModel = qobject_cast<const QSortFilterProxyModel*>(idx.model())) != nullptr) {
      // qDebug() << "QSortFilterProxyModel";
      idx = sortFilterModel->mapToSource(idx);
    } else if ((concatModel = qobject_cast<const KConcatenateRowsProxyModel*>(idx.model())) != nullptr) {
      // qDebug() << "KConcatenateRowsProxyModel";
      idx = concatModel->mapToSource(idx);
    } else if ((identityModel = qobject_cast<const QIdentityProxyModel*>(idx.model())) != nullptr) {
      // qDebug() << "QIdentityProxyModel";
      idx = identityModel->mapToSource(idx);
    } else if ((descendantsModel = qobject_cast<const KDescendantsProxyModel*>(idx.model())) != nullptr) {
      // qDebug() << "KDescendantsProxyModel";
      idx = descendantsModel->mapToSource(idx);
    } else if ((qobject_cast<const MyMoneyModelBase*>(idx.model())) == nullptr) {
      qDebug() << "Unknown model type in mapToBaseSource:" << idx.model()->metaObject()->className();
    }
  } while (sortFilterModel || concatModel || identityModel || descendantsModel);
  return idx;
}

QModelIndex MyMoneyModelBase::mapFromBaseSource(QAbstractItemModel* proxyModel, const QModelIndex& _idx)
{
  QModelIndex                       idx(_idx);
  const QSortFilterProxyModel*      sortFilterModel;
  const QIdentityProxyModel*        identityModel;
  const KConcatenateRowsProxyModel* concatModel;
  const KDescendantsProxyModel*     descendantsModel;

  if (( sortFilterModel = qobject_cast<const QSortFilterProxyModel*>(proxyModel)) != nullptr) {
    if (sortFilterModel->sourceModel() != idx.model()) {
      idx = mapFromBaseSource(sortFilterModel->sourceModel(), idx);
    }
    idx = sortFilterModel->mapFromSource(idx);

  } else if((concatModel = qobject_cast<const KConcatenateRowsProxyModel*>(proxyModel)) != nullptr) {
    idx = concatModel->mapFromSource(idx);

  } else if((identityModel = qobject_cast<const QIdentityProxyModel*>(proxyModel)) != nullptr) {
    if (identityModel->sourceModel() != idx.model()) {
      idx = mapFromBaseSource(identityModel->sourceModel(), idx);
    }
    idx = identityModel->mapFromSource(idx);
  } else if((descendantsModel = qobject_cast<const KDescendantsProxyModel*>(idx.model())) != nullptr) {
    if (descendantsModel->sourceModel() != idx.model()) {
      idx = mapFromBaseSource(descendantsModel->sourceModel(), idx);
    }
    idx = descendantsModel->mapFromSource(idx);
  } else {
    qDebug() << proxyModel->metaObject()->className() << "not supported in" << Q_FUNC_INFO;
    idx = QModelIndex();
  }
  return idx;
}

QModelIndex MyMoneyModelBase::lowerBound(const QString& id) const
{
  return lowerBound(id, 0, rowCount()-1);
}

QModelIndex MyMoneyModelBase::upperBound(const QString& id) const
{
  return upperBound(id, 0, rowCount()-1);
}

QString MyMoneyModelBase::peekNextId() const
{
  return QString("%1%2").arg(m_idLeadin).arg(m_nextId+1, m_idSize, 10, QLatin1Char('0'));
}

QString MyMoneyModelBase::nextId()
{
  return QString("%1%2").arg(m_idLeadin).arg(++m_nextId, m_idSize, 10, QLatin1Char('0'));
}

void MyMoneyModelBase::updateNextObjectId(const QString& id)
{
  QRegularExpressionMatch m = m_idMatchExp.match(id);
  if (m.hasMatch()) {
    const quint64 itemId = m.captured(1).toUInt();
    if (itemId > m_nextId) {
      m_nextId = itemId;
    }
  }
}

bool MyMoneyModelBase::isValidId(const QString& id) const
{
  QRegularExpressionMatch m = m_idMatchExp.match(id);
  if (m.hasMatch()) {
    return m.captured(1).length() == m_idSize;
  }
  return false;
}

void MyMoneyModelBase::setDirty(bool dirty)
{
  m_dirty = dirty;
}

bool MyMoneyModelBase::isDirty() const
{
  return m_dirty;
}

void MyMoneyModelBase::updateReferencedObjects()
{
  doUpdateReferencedObjects();
}

void MyMoneyModelBase::beginResetModel()
{
  m_blockedSignals = signalsBlocked();
  QAbstractItemModel::beginResetModel();
  blockSignals(true);
}

void MyMoneyModelBase::endResetModel()
{
  blockSignals(m_blockedSignals);
  QAbstractItemModel::endResetModel();
}
