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

#include <KConfigGroup>
#include <KSharedConfig>

// ----------------------------------------------------------------------------
// Project Includes

#include "models.h"
#include "accountsmodel.h"
#include "accountsviewproxymodel.h"
#include "budgetviewproxymodel.h"
#include "modelenums.h"

KMyMoneyAccountTreeView::KMyMoneyAccountTreeView(QWidget *parent)
    : QTreeView(parent), m_view(View::None)
{
  setContextMenuPolicy(Qt::CustomContextMenu);            // allow context menu to be opened on tree items
  header()->setContextMenuPolicy(Qt::CustomContextMenu);  // allow context menu to be opened on tree header for columns selection
  connect(this, &QWidget::customContextMenuRequested, this, &KMyMoneyAccountTreeView::customContextMenuRequested);
  setAllColumnsShowFocus(true);
  setAlternatingRowColors(true);
  setIconSize(QSize(22, 22));
  setSortingEnabled(true);
}

KMyMoneyAccountTreeView::~KMyMoneyAccountTreeView()
{
  if (m_view != View::None) {
    auto grp = KSharedConfig::openConfig()->group(getConfGrpName(m_view));
    const auto columns = header()->saveState();
    grp.writeEntry("HeaderState", columns);
    QList<int> visColumns;
    foreach (const auto column, m_model->getVisibleColumns())
      visColumns.append(static_cast<int>(column));
    grp.writeEntry("ColumnsSelection", visColumns);
    grp.sync();
  }
}

AccountsViewProxyModel *KMyMoneyAccountTreeView::init(View view)
{
  m_view = view;
  if (view != View::Budget)
    m_model = new AccountsViewProxyModel(this);
  else
    m_model = new BudgetViewProxyModel(this);

  m_model->addAccountGroup(getVisibleGroups(view));

  const auto accountsModel = Models::instance()->accountsModel();
  const auto institutionsModel = Models::instance()->institutionsModel();

  AccountsModel *sourceModel;
  if (view != View::Institutions)
    sourceModel = accountsModel;
  else
    sourceModel = institutionsModel;

  foreach (const auto column, readVisibleColumns(view)) {
   m_model->setColumnVisibility(column, true);
   accountsModel->setColumnVisibility(column, true);
   institutionsModel->setColumnVisibility(column, true);
  }

  m_model->setSourceModel(sourceModel);
  m_model->setSourceColumns(sourceModel->getColumns());
  setModel(m_model);

  connect(this->header(), &QWidget::customContextMenuRequested, m_model, &AccountsViewProxyModel::slotColumnsMenu);
  connect(m_model, &AccountsViewProxyModel::columnToggled, this, &KMyMoneyAccountTreeView::columnToggled);

  // restore the headers
  const auto grp = KSharedConfig::openConfig()->group(getConfGrpName(view));
  const auto columnNames = grp.readEntry("HeaderState", QByteArray());
  header()->restoreState(columnNames);

  return m_model;
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
    QVariant data = model()->data(index, (int)eAccountsModel::Role::Account);
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

QString KMyMoneyAccountTreeView::getConfGrpName(const View view)
{
  switch (view) {
    case View::Institutions:
      return QStringLiteral("KInstitutionsView");
    case View::Accounts:
      return QStringLiteral("KAccountsView");
    case View::Categories:
      return QStringLiteral("KCategoriesView");
    case View::Budget:
      return QStringLiteral("KBudgetsView");
    default:
      return QString();
  }
}

void KMyMoneyAccountTreeView::customContextMenuRequested(const QPoint)
{
  const auto index = model()->index(currentIndex().row(), (int)eAccountsModel::Column::Account, currentIndex().parent());
  if (index.isValid() && (model()->flags(index) & Qt::ItemIsSelectable)) {
    const auto data = model()->data(index, (int)eAccountsModel::Role::Account);
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
    auto indexes = selected.front().indexes();
    if (!indexes.empty()) {
      const auto data = model()->data(model()->index(indexes.front().row(), (int)eAccountsModel::Column::Account, indexes.front().parent()), (int)eAccountsModel::Role::Account);
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

QVector<MyMoneyAccount::_accountTypeE> KMyMoneyAccountTreeView::getVisibleGroups(const View view)
{
  switch (view) {
    case View::Institutions:
    case View::Accounts:
      return QVector<MyMoneyAccount::_accountTypeE> {MyMoneyAccount::Asset, MyMoneyAccount::Liability, MyMoneyAccount::Equity};
    case View::Categories:
    case View::Budget:
      return QVector<MyMoneyAccount::_accountTypeE> {MyMoneyAccount::Income, MyMoneyAccount::Expense};
    default:
      return QVector<MyMoneyAccount::_accountTypeE> ();
  }
}

QSet<eAccountsModel::Column> KMyMoneyAccountTreeView::readVisibleColumns(const View view)
{
  QSet<eAccountsModel::Column> columns;

  const auto grp = KSharedConfig::openConfig()->group(getConfGrpName(view));
  const auto cfgColumns = grp.readEntry("ColumnsSelection", QList<int>());
  columns.insert(eAccountsModel::Column::Account);
  foreach (const auto column, cfgColumns)
    columns.insert(static_cast<eAccountsModel::Column>(column));
  return columns;
}
