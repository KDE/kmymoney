/***************************************************************************
 *   Copyright 2010  Cristian Onet onet.cristian@gmail.com                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>  *
 ***************************************************************************/

#include "kmymoneyaccounttreeview.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QHeaderView>
#include <QMouseEvent>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KGlobal>
#include <KConfig>
#include <KConfigGroup>

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyinstitution.h>

KMyMoneyAccountTreeView::KMyMoneyAccountTreeView(QWidget *parent)
    : QTreeView(parent)
{
  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(customContextMenuRequested(QPoint)));
  setAllColumnsShowFocus(true);
}

KMyMoneyAccountTreeView::~KMyMoneyAccountTreeView()
{
  if (!m_groupName.isEmpty()) {
    KConfigGroup grp = KGlobal::config()->group(m_groupName);
    QByteArray columns = header()->saveState();
    grp.writeEntry("HeaderState", columns);
  }
}

/**
  * Set the name of the configuration group where the view's persistent data is saved to @param group .
  */
void KMyMoneyAccountTreeView::setConfigGroupName(const QString& group)
{
  if (!group.isEmpty()) {
    m_groupName = group;
    // restore the headers
    KConfigGroup grp = KGlobal::config()->group(m_groupName);
    QByteArray columns;
    columns = grp.readEntry("HeaderState", columns);
    header()->restoreState(columns);
  }
}

void KMyMoneyAccountTreeView::mouseDoubleClickEvent(QMouseEvent *event)
{
  openIndex(currentIndex());
  event->accept();
}

void KMyMoneyAccountTreeView::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
    openIndex(currentIndex());
    event->accept();
  } else {
    QTreeView::keyPressEvent(event);
  }
}

void KMyMoneyAccountTreeView::openIndex(const QModelIndex &index)
{
  if (index.isValid()) {
    QVariant data = model()->data(index, AccountsModel::AccountRole);
    if (data.isValid()) {
      if (data.canConvert<MyMoneyAccount>()) {
        emit openObject(data.value<MyMoneyAccount>());
      }
      if (data.canConvert<MyMoneyInstitution>()) {
        emit openObject(data.value<MyMoneyInstitution>());
      }
    }
  }
}

void KMyMoneyAccountTreeView::customContextMenuRequested(const QPoint &pos)
{
  Q_UNUSED(pos)
  QModelIndex index = model()->index(currentIndex().row(), AccountsModel::Account, currentIndex().parent());
  if (index.isValid() && (model()->flags(index) & Qt::ItemIsSelectable)) {
    QVariant data = model()->data(index, AccountsModel::AccountRole);
    if (data.isValid()) {
      if (data.canConvert<MyMoneyAccount>()) {
        emit selectObject(data.value<MyMoneyAccount>());
        emit openContextMenu(data.value<MyMoneyAccount>());
      }
      if (data.canConvert<MyMoneyInstitution>()) {
        emit selectObject(data.value<MyMoneyInstitution>());
        emit openContextMenu(data.value<MyMoneyInstitution>());
      }
    }
  }
}

void KMyMoneyAccountTreeView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
  QTreeView::selectionChanged(selected, deselected);
  if (!selected.empty()) {
    QModelIndexList indexes = selected.front().indexes();
    if (!indexes.empty()) {
      QVariant data = model()->data(model()->index(indexes.front().row(), AccountsModel::Account, indexes.front().parent()), AccountsModel::AccountRole);
      if (data.isValid()) {
        if (data.canConvert<MyMoneyAccount>()) {
          emit selectObject(data.value<MyMoneyAccount>());
        }
        if (data.canConvert<MyMoneyInstitution>()) {
          emit selectObject(data.value<MyMoneyInstitution>());
        }
        // an object was successfully selected
        return;
      }
    }
  }
  // since no object was selected reset the object selection
  emit selectObject(MyMoneyAccount());
  emit selectObject(MyMoneyInstitution());
}

void KMyMoneyAccountTreeView::collapseAll(void)
{
  QTreeView::collapseAll();
  emit collapsedAll();
}

void KMyMoneyAccountTreeView::expandAll(void)
{
  QTreeView::expandAll();
  emit expandedAll();
}


class AccountsViewFilterProxyModel::Private
{
public:
  Private() {
  }

  ~Private() {
  }

  void markAccountExpanded(const QString &accountId) {
    m_expandedAccountIds.insert(accountId);
  }

  void markAccountCollapsed(const QString &accountId) {
    m_expandedAccountIds.remove(accountId);
  }

  void markAllExpanded(QAbstractItemModel *model) {
    QModelIndexList list = model->match(model->index(0, 0), AccountsModel::AccountIdRole, "*", -1, Qt::MatchFlags(Qt::MatchWildcard | Qt::MatchRecursive));
    foreach (const QModelIndex &index, list) {
      markAccountExpanded(model->data(index, AccountsModel::AccountIdRole).toString());
    }
  }

  void markAllCollapsed(QAbstractItemModel *model) {
    Q_UNUSED(model);
    m_expandedAccountIds.clear();
  }

  bool isAccountExpanded(const QString &accountId) {
    return m_expandedAccountIds.contains(accountId);
  }

  QSet<QString> m_expandedAccountIds;
};

AccountsViewFilterProxyModel::AccountsViewFilterProxyModel(QObject *parent)
    : AccountsFilterProxyModel(parent), d(new Private)
{
}

AccountsViewFilterProxyModel::~AccountsViewFilterProxyModel()
{
  delete d;
}

/**
  * This function was reimplemented to add the data needed by the other columns that this model
  * is adding besides the columns of the @ref AccountsModel.
  */
QVariant AccountsViewFilterProxyModel::data(const QModelIndex &index, int role) const
{
  if (index.isValid() && role == Qt::DisplayRole) {
    int sourceColumn = mapToSource(index).column();
    if (sourceColumn == AccountsModel::TotalValue) {
      QVariant accountId = sourceModel()->data(mapToSource(AccountsViewFilterProxyModel::index(index.row(), 0, index.parent())), AccountsModel::AccountIdRole);
      if (d->isAccountExpanded(accountId.toString()) && index.parent().isValid()) {
        // if an account is not a top-level account and it is expanded display it's value
        return data(index, AccountsModel::AccountValueDisplayRole);
      } else {
        // if an account is a top-level account or it is collapsed display it's total value
        return data(index, AccountsModel::AccountTotalValueDisplayRole);
      }
    }
    if (sourceColumn == AccountsModel::TotalBalance) {
      return data(index, AccountsModel::AccountBalanceDisplayRole);
    }
  }
  return AccountsFilterProxyModel::data(index, role);
}

/**
  * Reimplemented to filter all but the account displayed in the accounts view.
  */
bool AccountsViewFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
  if (!source_parent.isValid()) {
    QVariant data = sourceModel()->data(sourceModel()->index(source_row, 0, source_parent), AccountsModel::AccountIdRole);
    if (data.isValid() && data.toString() == AccountsModel::favoritesAccountId)
      return false;
  }
  return AccountsFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

/**
  * The model is notified that the representation of the item at index was collapsed.
  * @param index The index of the item which was collapsed.
  */
void AccountsViewFilterProxyModel::collapsed(const QModelIndex &index)
{
  QVariant accountId = sourceModel()->data(mapToSource(index), AccountsModel::AccountIdRole);
  if (accountId.isValid()) {
    d->markAccountCollapsed(accountId.toString());
  }
}

/**
  * The model is notified that the representation of the item at index was expanded.
  * @param index The index of the item which was expanded.
  */
void AccountsViewFilterProxyModel::expanded(const QModelIndex &index)
{
  QVariant accountId = sourceModel()->data(mapToSource(index), AccountsModel::AccountIdRole);
  if (accountId.isValid()) {
    d->markAccountExpanded(accountId.toString());
  }
}

/**
  * The model is notified that all the items where collapsed (unfortunately we can't use the above
  * for that since Qt does not emit the appropriate signal when collapsing all the items).
  */
void AccountsViewFilterProxyModel::collapseAll()
{
  d->markAllCollapsed(this);
}

/**
  * The model is notified that all the items where expanded (unfortunately we can't use the above
  * for that since Qt does not emit the appropriate signal when expanding all the items).
  */
void AccountsViewFilterProxyModel::expandAll()
{
  d->markAllExpanded(this);
}

#include "kmymoneyaccounttreeview.moc"
