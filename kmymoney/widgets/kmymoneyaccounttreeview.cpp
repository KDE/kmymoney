/*
 * Copyright 2010-2014  Cristian Oneț <onet.cristian@gmail.com>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#include "kmymoneyaccounttreeview.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QObject>
#include <QHeaderView>
#include <QMouseEvent>
#include <QPoint>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KConfigGroup>
#include <KSharedConfig>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "mymoneyinstitution.h"
#include "mymoneyaccount.h"
#include "accountsmodel.h"
#include "institutionsmodel.h"
#include "accountsproxymodel.h"
#include "modelenums.h"
#include "menuenums.h"
#include "selectedobjects.h"

class KMyMoneyAccountTreeViewPrivate
{
  Q_DECLARE_PUBLIC(KMyMoneyAccountTreeView)

public:
  KMyMoneyAccountTreeViewPrivate(KMyMoneyAccountTreeView *qq)
    : q_ptr(qq),
    proxyModel(new AccountsProxyModel(qq))
    {}

  void openIndex(const QModelIndex &index)
  {
    Q_Q(KMyMoneyAccountTreeView);
    if (index.isValid()) {
      QModelIndex baseIdx = MyMoneyFile::baseModel()->mapToBaseSource(index);
      // baseIdx could point into the accountsModel or the institutionsModel.
      // in case of the institutionsModel it is unclear if it is an account or
      // an institution. So we simply extract the id and check where we find
      // the object.
      const auto objId = baseIdx.data(eMyMoney::Model::IdRole).toString();
      auto idx = MyMoneyFile::instance()->accountsModel()->indexById(objId);
      if (idx.isValid()) {
        emit q->requestActionTrigger(eMenu::Action::EditAccount);
      } else {
        idx = MyMoneyFile::instance()->institutionsModel()->indexById(objId);
        if (idx.isValid()) {
          emit q->requestActionTrigger(eMenu::Action::EditInstitution);
        }
      }
    }
  }

  KMyMoneyAccountTreeView*  q_ptr;
  AccountsProxyModel*       proxyModel;
};

KMyMoneyAccountTreeView::KMyMoneyAccountTreeView(QWidget *parent)
  : QTreeView(parent)
  , d_ptr(new KMyMoneyAccountTreeViewPrivate(this))
{
  Q_D(KMyMoneyAccountTreeView);
  setContextMenuPolicy(Qt::CustomContextMenu);            // allow context menu to be opened on tree items
  connect(this, &QWidget::customContextMenuRequested, this, &KMyMoneyAccountTreeView::customContextMenuRequested);
  setAllColumnsShowFocus(true);
  setAlternatingRowColors(true);
  setIconSize(QSize(22, 22));
  setSortingEnabled(true);
}

KMyMoneyAccountTreeView::~KMyMoneyAccountTreeView()
{
}


void KMyMoneyAccountTreeView::setModel(QAbstractItemModel* model)
{
  Q_D(KMyMoneyAccountTreeView);
  d->proxyModel->setSourceModel(model);
  QTreeView::setModel(d->proxyModel);
}

void KMyMoneyAccountTreeView::setProxyModel(AccountsProxyModel* model)
{
  Q_D(KMyMoneyAccountTreeView);
  // unlink a possible sourceModel
  QAbstractItemModel* sourceModel = d->proxyModel->sourceModel();
  if (sourceModel) {
    d->proxyModel->setSourceModel(nullptr);
  }

  // delete the old proxy
  d->proxyModel->deleteLater();

  // reparent the new proxy
  model->setParent(this);

  // and insert it into the chain
  d->proxyModel = model;
  d->proxyModel->setSourceModel(sourceModel);
  QTreeView::setModel(d->proxyModel);

}

AccountsProxyModel* KMyMoneyAccountTreeView::proxyModel() const
{
  Q_D(const KMyMoneyAccountTreeView);
  return d->proxyModel;
}

void KMyMoneyAccountTreeView::mouseDoubleClickEvent(QMouseEvent *event)
{
  Q_D(KMyMoneyAccountTreeView);
  d->openIndex(currentIndex());
  event->accept();
}

void KMyMoneyAccountTreeView::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
    Q_D(KMyMoneyAccountTreeView);
    d->openIndex(currentIndex());
    event->accept();
  } else {
    QTreeView::keyPressEvent(event);
  }
}

void KMyMoneyAccountTreeView::customContextMenuRequested(const QPoint pos)
{
  if (currentIndex().isValid() && (model()->flags(currentIndex()) & Qt::ItemIsSelectable)) {
    auto objId = currentIndex().data(eMyMoney::Model::IdRole).toString();
    const auto account = MyMoneyFile::instance()->accountsModel()->itemById(objId);
    if (!account.id().isEmpty()) {
      if (account.isIncomeExpense()) {
        emit requestCustomContextMenu(eMenu::Menu::Category, mapToGlobal(pos));
      } else {
        emit requestCustomContextMenu(eMenu::Menu::Account, mapToGlobal(pos));
      }
    }
    const auto institution = MyMoneyFile::instance()->institutionsModel()->itemById(objId);
    if (!institution.id().isEmpty()) {
      // the institutions model also reports accounts as institutions at this point.
      // We can differentiate between the two objects by looking at the parent of
      // the index. If it is valid, we have been called via an account,
      // if it is invalid the source is an institution.
      if (!currentIndex().parent().isValid()) {
        emit requestCustomContextMenu(eMenu::Menu::Institution, mapToGlobal(pos));
      }
    }
  }
}

void KMyMoneyAccountTreeView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
  SelectedObjects selections;

  QTreeView::selectionChanged(selected, deselected);
  if (!selected.isEmpty()) {
    QModelIndexList idxList = selected.indexes();
    if (!idxList.isEmpty()) {
      auto objId = selected.indexes().front().data(eMyMoney::Model::IdRole).toString();
      const auto account = MyMoneyFile::instance()->accountsModel()->itemById(objId);
      if (!account.id().isEmpty()) {
        selections.addSelection(SelectedObjects::Account, account.id());
        selections.addSelection(SelectedObjects::Institution, account.institutionId());
      } else {

        const auto institution = MyMoneyFile::instance()->institutionsModel()->itemById(objId);
        if (!institution.id().isEmpty()) {
          selections.addSelection(SelectedObjects::Institution, institution.id());
        }
      }
    }
  }
  // since no object was selected reset the object selection
  emit requestSelectionChange(selections);
}
