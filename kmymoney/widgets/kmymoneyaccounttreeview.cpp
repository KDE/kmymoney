/***************************************************************************
 *   Copyright 2010  Cristian Onet onet.cristian@gmail.com                 *
 *   Copyright 2017  Łukasz Wojniłowicz lukasz.wojnilowicz@gmail.com       *
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

#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyinstitution.h>
#include <models.h>

KMyMoneyAccountTreeView::KMyMoneyAccountTreeView(QWidget *parent)
    : QTreeView(parent)
{
  setContextMenuPolicy(Qt::CustomContextMenu);
  header()->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, &QWidget::customContextMenuRequested, this, &KMyMoneyAccountTreeView::customContextMenuRequested);
  connect(this->header(), &QWidget::customContextMenuRequested, this, &KMyMoneyAccountTreeView::customHeaderContextMenuRequested);
  setAllColumnsShowFocus(true);
  setAlternatingRowColors(true);
  setIconSize(QSize(22, 22));
  setSortingEnabled(true);
  m_view = KMyMoneyView::View::None;
}

KMyMoneyAccountTreeView::~KMyMoneyAccountTreeView()
{
  if (m_view != KMyMoneyView::View::None) {
    auto grp = KSharedConfig::openConfig()->group(getConfGrpName(m_view));
    const auto columns = header()->saveState();
    grp.writeEntry("HeaderState", columns);
    QList<int> visColumns;
    foreach (const auto column, m_visColumns)
      visColumns.append(static_cast<int>(column));
    grp.writeEntry("ColumnsSelection", visColumns);
    grp.sync();
  }
}

void KMyMoneyAccountTreeView::init(QAbstractItemModel *model, QList<AccountsModel::Columns> *columns)
{
  setModel(model);
  m_mdlColumns = columns;
  if (m_view != KMyMoneyView::View::None) {
    // restore the headers
    const auto grp = KSharedConfig::openConfig()->group(getConfGrpName(m_view));
    const auto columnNames = grp.readEntry("HeaderState", QByteArray());
    header()->restoreState(columnNames);
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

QString KMyMoneyAccountTreeView::getConfGrpName(const KMyMoneyView::View view)
{
  switch (view) {
    case KMyMoneyView::View::Institutions:
      return QStringLiteral("KInstitutionsView");
    case KMyMoneyView::View::Accounts:
      return QStringLiteral("KAccountsView");
    case KMyMoneyView::View::Categories:
        return QStringLiteral("KCategoriesView");
    case KMyMoneyView::View::Budget:
      return QStringLiteral("KBudgetsView");
    default:
      return QString();
  }
}

void KMyMoneyAccountTreeView::customContextMenuRequested(const QPoint)
{
  const auto index = model()->index(currentIndex().row(), AccountsModel::Account, currentIndex().parent());
  if (index.isValid() && (model()->flags(index) & Qt::ItemIsSelectable)) {
    const auto data = model()->data(index, AccountsModel::AccountRole);
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

void KMyMoneyAccountTreeView::customHeaderContextMenuRequested(const QPoint)
{
  const QList<AccountsModel::Columns> defColumns = { AccountsModel::Type, AccountsModel::Tax,
                                                     AccountsModel::VAT, AccountsModel::CostCenter,
                                                     AccountsModel::TotalBalance, AccountsModel::PostedValue,
                                                     AccountsModel::TotalValue, AccountsModel::AccountNumber,
                                                     AccountsModel::AccountSortCode
                                                   };

  m_menu = new QMenu(i18n("Displayed columns"));
  QList<QAction *> actions;
  foreach (const auto column, defColumns) {
    auto a = new QAction(0);
    a->setObjectName(QString::number(column));
    a->setText(AccountsModel::getHeaderName(column));
    a->setCheckable(true);
    a->setChecked(m_visColumns.contains(column));
    connect(a, &QAction::toggled, this, &KMyMoneyAccountTreeView::slotColumnToggled);
    actions.append(a);
  }

  m_menu->addActions(actions);
  m_menu->exec(QCursor::pos());
  connect(m_menu, &QMenu::aboutToHide, this, &QMenu::deleteLater);
}

void KMyMoneyAccountTreeView::slotColumnToggled(bool)
{
  QList<AccountsModel::Columns> selColumns;
  selColumns.append(AccountsModel::Account);
  const auto actions = m_menu->actions();
  foreach (const auto action, actions) {
    const auto columnID = static_cast<AccountsModel::Columns>(action->objectName().toInt());
    const auto isChecked = action->isChecked();
    const auto contains = m_visColumns.contains(columnID);
    if (isChecked && !contains) {
      m_visColumns.append(columnID);
      emit columnToggled(columnID, true);
      break;
    } else if (!isChecked && contains) {
      m_visColumns.removeOne(columnID);
      emit columnToggled(columnID, false);
      break;
    }
  }
  auto mdl = qobject_cast<KRecursiveFilterProxyModel *>(model());
  mdl->invalidate();
}

void KMyMoneyAccountTreeView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
  QTreeView::selectionChanged(selected, deselected);
  if (!selected.empty()) {
    auto indexes = selected.front().indexes();
    if (!indexes.empty()) {
      const auto data = model()->data(model()->index(indexes.front().row(), AccountsModel::Account, indexes.front().parent()), AccountsModel::AccountRole);
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

void KMyMoneyAccountTreeView::collapseAll()
{
  QTreeView::collapseAll();
  emit collapsedAll();
}

void KMyMoneyAccountTreeView::expandAll()
{
  QTreeView::expandAll();
  emit expandedAll();
}

QList<AccountsModel::Columns> *KMyMoneyAccountTreeView::getColumns(const KMyMoneyView::View view)
{
  m_view = view;
  if (m_view != KMyMoneyView::View::None && m_visColumns.isEmpty()) {
    auto const mdlAccounts = Models::instance()->accountsModel();
    auto const mdlInstitutions = Models::instance()->institutionsModel();
    const auto grp = KSharedConfig::openConfig()->group(getConfGrpName(m_view));
    const auto cfgColumns = grp.readEntry("ColumnsSelection", QList<int>());
    m_visColumns.append(AccountsModel::Account);
    foreach (const auto column, cfgColumns) {
      const auto visColumn = static_cast<AccountsModel::Columns>(column);
      if (!m_visColumns.contains(visColumn)) {
        m_visColumns.append(visColumn);
        mdlAccounts->setColumnVisibility(visColumn, true);
        mdlInstitutions->setColumnVisibility(visColumn, true);
      }
    }
  }
  return &m_visColumns;
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
    QModelIndexList list = model->match(model->index(0, 0), AccountsModel::AccountIdRole, QVariant(QLatin1String("*")), -1, Qt::MatchFlags(Qt::MatchWildcard | Qt::MatchRecursive));
    foreach (const QModelIndex &index, list) {
      markAccountExpanded(index.data(AccountsModel::AccountIdRole).toString());
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
  return AccountsFilterProxyModel::data(index, role);
}

  /*
  * Reimplemented to filter all but the account displayed in the accounts view.
  */
bool AccountsViewFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
  if (!source_parent.isValid()) {
    const auto data = sourceModel()->index(source_row, AccountsModel::Account, source_parent).data(AccountsModel::AccountIdRole);
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
  const auto accountId = mapToSource(index).data(AccountsModel::AccountIdRole);
  if (accountId.isValid())
    d->markAccountCollapsed(accountId.toString());
}

/**
  * The model is notified that the representation of the item at index was expanded.
  * @param index The index of the item which was expanded.
  */
void AccountsViewFilterProxyModel::expanded(const QModelIndex &index)
{
  const auto accountId = mapToSource(index).data(AccountsModel::AccountIdRole);
  if (accountId.isValid())
    d->markAccountExpanded(accountId.toString());
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
