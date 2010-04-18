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

KMyMoneyAccountTreeView::KMyMoneyAccountTreeView(QWidget *parent)
    : QTreeView(parent)
{
  // restore the headers
  KConfigGroup grp = KGlobal::config()->group("KAccountsView");
  QByteArray columns;
  columns = grp.readEntry("HeaderState", columns);
  header()->restoreState(columns);

  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(customContextMenuRequested(const QPoint &)));
}

KMyMoneyAccountTreeView::~KMyMoneyAccountTreeView()
{
  KConfigGroup grp = KGlobal::config()->group("KAccountsView");
  QByteArray columns = header()->saveState();
  grp.writeEntry("HeaderState", columns);
}

void KMyMoneyAccountTreeView::mouseDoubleClickEvent(QMouseEvent *event)
{
  QModelIndex index = currentIndex();
  if (index.isValid()) {
    QVariant data = model()->data(currentIndex(), AccountsModel::AccountRole);
    if (data.isValid()) {
      emit openObject(data.value<MyMoneyAccount>());
    }
  }
  event->accept();
}

void KMyMoneyAccountTreeView::customContextMenuRequested(const QPoint &pos)
{
  Q_UNUSED(pos)
  QModelIndex index = currentIndex();
  if (index.isValid()) {
    QVariant data = model()->data(currentIndex(), AccountsModel::AccountRole);
    if (data.isValid()) {
      emit selectObject(data.value<MyMoneyAccount>());
      emit openContextMenu(data.value<MyMoneyAccount>());
    }
  }
}

void KMyMoneyAccountTreeView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
  QTreeView::currentChanged(current, previous);

  QVariant data = model()->data(current, AccountsModel::AccountRole);
  if (data.isValid()) {
    emit selectObject(data.value<MyMoneyAccount>());
  }
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
  if (index.isValid() && index.column() == AccountsModel::TotalValue && role == Qt::DisplayRole) {
    QVariant accountId = sourceModel()->data(mapToSource(AccountsViewFilterProxyModel::index(index.row(), 0, index.parent())), AccountsModel::AccountIdRole);
    if (d->isAccountExpanded(accountId.toString()) && index.parent().isValid()) {
      // if an account is not a top-level account and it is expanded display it's value
      return data(index, AccountsModel::AccountValueDisplayRole);
    } else {
      // if an account is a top-level account or it is collapsed display it's total value
      return data(index, AccountsModel::AccountTotalValueDisplayRole);
    }
  } else {
    return AccountsFilterProxyModel::data(index, role);
  }
}

/**
  * Reimplemented to filter all but the account displayed in the accounts view.
  */
bool AccountsViewFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
  if (!source_parent.isValid() && source_row == 0) {
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

#include "kmymoneyaccounttreeview.moc"
