/*
 * Copyright 2010-2014  Cristian Oneț <onet.cristian@gmail.com>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#include "kmymoneyaccounttreeview.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QHeaderView>
#include <QMouseEvent>
#include <QPoint>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KConfigGroup>
#include <KSharedConfig>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyinstitution.h"
#include "mymoneyaccount.h"
#include "models.h"
#include "accountsmodel.h"
#include "accountsviewproxymodel.h"
#include "budgetviewproxymodel.h"
#include "modelenums.h"
#include "mymoneyenums.h"
#include "viewenums.h"

class KMyMoneyAccountTreeViewPrivate
{
  Q_DISABLE_COPY(KMyMoneyAccountTreeViewPrivate)
  Q_DECLARE_PUBLIC(KMyMoneyAccountTreeView)

public:
  explicit KMyMoneyAccountTreeViewPrivate(KMyMoneyAccountTreeView *qq) :
    q_ptr(qq),
    m_model(nullptr),
    m_view(View::None)
  {
  }

  ~KMyMoneyAccountTreeViewPrivate()
  {
  }

  QVector<eMyMoney::Account::Type> getVisibleGroups(const View view)
  {
    switch (view) {
      case View::Institutions:
      case View::Accounts:
        return QVector<eMyMoney::Account::Type> {eMyMoney::Account::Type::Asset, eMyMoney::Account::Type::Liability, eMyMoney::Account::Type::Equity};
      case View::Categories:
      case View::Budget:
        return QVector<eMyMoney::Account::Type> {eMyMoney::Account::Type::Income, eMyMoney::Account::Type::Expense};
      default:
        return QVector<eMyMoney::Account::Type> ();
    }
  }

  QSet<eAccountsModel::Column> readVisibleColumns(const View view)
  {
    QSet<eAccountsModel::Column> columns;

    const auto grp = KSharedConfig::openConfig()->group(getConfGrpName(view));
    const auto cfgColumns = grp.readEntry("ColumnsSelection", QList<int>());
    columns.insert(eAccountsModel::Column::Account);
    foreach (const auto column, cfgColumns)
      columns.insert(static_cast<eAccountsModel::Column>(column));
    return columns;
  }

  void openIndex(const QModelIndex &index)
  {
    Q_Q(KMyMoneyAccountTreeView);
    if (index.isValid()) {
      QVariant data = q->model()->data(index, (int)eAccountsModel::Role::Account);
      if (data.isValid()) {
        if (data.canConvert<MyMoneyAccount>()) {
          emit q->selectByObject(data.value<MyMoneyAccount>(), eView::Intent::OpenObject);
        }
        if (data.canConvert<MyMoneyInstitution>()) {
          emit q->selectByObject(data.value<MyMoneyInstitution>(), eView::Intent::OpenObject);
        }
      }
    }
  }

  static QString getConfGrpName(const View view)
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

  KMyMoneyAccountTreeView *q_ptr;
  AccountsViewProxyModel  *m_model;
  View                     m_view;
};

KMyMoneyAccountTreeView::KMyMoneyAccountTreeView(QWidget *parent) :
  QTreeView(parent),
  d_ptr(new KMyMoneyAccountTreeViewPrivate(this))
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
  Q_D(KMyMoneyAccountTreeView);
  if (d->m_view != View::None) {
    auto grp = KSharedConfig::openConfig()->group(d->getConfGrpName(d->m_view));
    const auto columns = header()->saveState();
    grp.writeEntry("HeaderState", columns);
    QList<int> visColumns;
    foreach (const auto column, d->m_model->getVisibleColumns())
      visColumns.append(static_cast<int>(column));
    grp.writeEntry("ColumnsSelection", visColumns);
    grp.sync();
  }
  delete d;
}

AccountsViewProxyModel *KMyMoneyAccountTreeView::init(View view)
{
  Q_D(KMyMoneyAccountTreeView);
  d->m_view = view;
  if (view != View::Budget)
    d->m_model = new AccountsViewProxyModel(this);
  else
    d->m_model = new BudgetViewProxyModel(this);

  d->m_model->addAccountGroup(d->getVisibleGroups(view));

  const auto accountsModel = Models::instance()->accountsModel();
  const auto institutionsModel = Models::instance()->institutionsModel();

  AccountsModel *sourceModel;
  if (view != View::Institutions)
    sourceModel = accountsModel;
  else
    sourceModel = institutionsModel;

  foreach (const auto column, d->readVisibleColumns(view)) {
    d->m_model->setColumnVisibility(column, true);
    accountsModel->setColumnVisibility(column, true);
    institutionsModel->setColumnVisibility(column, true);
  }

  d->m_model->setSourceModel(sourceModel);
  d->m_model->setSourceColumns(sourceModel->getColumns());
  d->m_model->setFilterCaseSensitivity(Qt::CaseInsensitive);
  d->m_model->setSortLocaleAware(true);
  setModel(d->m_model);

  connect(this->header(), &QWidget::customContextMenuRequested, d->m_model, &AccountsViewProxyModel::slotColumnsMenu);
  connect(d->m_model, &AccountsViewProxyModel::columnToggled, this, &KMyMoneyAccountTreeView::slotColumnToggled);

  // restore the headers
  const auto grp = KSharedConfig::openConfig()->group(d->getConfGrpName(view));
  const auto columnNames = grp.readEntry("HeaderState", QByteArray());
  header()->restoreState(columnNames);

  return d->m_model;
}

void KMyMoneyAccountTreeView::mouseDoubleClickEvent(QMouseEvent *event)
{
  Q_D(KMyMoneyAccountTreeView);
  d->openIndex(currentIndex());
  event->accept();
}

void KMyMoneyAccountTreeView::keyPressEvent(QKeyEvent *event)
{
  Q_D(KMyMoneyAccountTreeView);
  if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
    d->openIndex(currentIndex());
    event->accept();
  } else {
    QTreeView::keyPressEvent(event);
  }
}

void KMyMoneyAccountTreeView::customContextMenuRequested(const QPoint)
{
  const auto index = model()->index(currentIndex().row(), (int)eAccountsModel::Column::Account, currentIndex().parent());
  if (index.isValid() && (model()->flags(index) & Qt::ItemIsSelectable)) {
    const auto dataVariant = model()->data(index, (int)eAccountsModel::Role::Account);
    if (dataVariant.isValid()) {
      if (dataVariant.canConvert<MyMoneyAccount>()) {
        emit selectByObject(dataVariant.value<MyMoneyAccount>(), eView::Intent::None);
        emit selectByObject(dataVariant.value<MyMoneyAccount>(), eView::Intent::OpenContextMenu);
      }
      if (dataVariant.canConvert<MyMoneyInstitution>()) {
        emit selectByObject(dataVariant.value<MyMoneyInstitution>(), eView::Intent::None);
        emit selectByObject(dataVariant.value<MyMoneyInstitution>(), eView::Intent::OpenContextMenu);
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
      const auto dataVariant = model()->data(model()->index(indexes.front().row(), (int)eAccountsModel::Column::Account, indexes.front().parent()), (int)eAccountsModel::Role::Account);
      if (dataVariant.isValid()) {
        if (dataVariant.canConvert<MyMoneyAccount>())
          emit selectByObject(dataVariant.value<MyMoneyAccount>(), eView::Intent::None);

        if (dataVariant.canConvert<MyMoneyInstitution>())
          emit selectByObject(dataVariant.value<MyMoneyInstitution>(), eView::Intent::None);

        // an object was successfully selected
        return;
      }
    }
  }
  // since no object was selected reset the object selection
  emit selectByObject(MyMoneyAccount(), eView::Intent::None);
  emit selectByObject(MyMoneyInstitution(), eView::Intent::None);
}

void KMyMoneyAccountTreeView::slotColumnToggled(const eAccountsModel::Column column, const bool show)
{
  emit selectByVariant(QVariantList {QVariant::fromValue(column), QVariant(show)}, eView::Intent::ToggleColumn);
}
