/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * SPDX-FileCopyrightText: 2014 Christian Dávid <christian-david@web.de>
 *
 *SPDX-License-Identifier: GPL-2.0-or-laterrg/licenses/>.
 */

#include "kpayeeidentifierview.h"
#include "ui_kpayeeidentifierview.h"

#include <algorithm>

#include <QPointer>
#include <QAbstractItemDelegate>
#include <QStyledItemDelegate>

#include "payeeidentifiercontainermodel.h"
#include "payeeidentifierselectiondelegate.h"
#include "widgets/payeeidentifier/ibanbic/ibanbicitemdelegate.h"
#include "widgets/payeeidentifier/nationalaccount/nationalaccountdelegate.h"

payeeIdentifierDelegate::payeeIdentifierDelegate(QObject* parent)
    : StyledItemDelegateForwarder(parent)
{
}

QAbstractItemDelegate* payeeIdentifierDelegate::getItemDelegate(const QModelIndex& index) const
{
  static QPointer<QAbstractItemDelegate> defaultDelegate;
  const QString type = (index.isValid()) ? index.model()->data(index, payeeIdentifierContainerModel::payeeIdentifierType).toString() : QString();

  if (type.isEmpty()) {
    QAbstractItemDelegate* delegate = new payeeIdentifierSelectionDelegate(this->parent());
    connectSignals(delegate);
    return delegate;
  }

  QAbstractItemDelegate* delegate = nullptr;
  // Use this->parent() as parent because "this" is const
  if (type == payeeIdentifiers::ibanBic::staticPayeeIdentifierIid()) {
    delegate = new ibanBicItemDelegate(this->parent());
  } else if (type == payeeIdentifiers::nationalAccount::staticPayeeIdentifierIid()) {
    delegate = new nationalAccountDelegate(this->parent());
  }

  if (delegate == 0) {
    if (defaultDelegate == 0)
      defaultDelegate = new QStyledItemDelegate(this->parent());
    delegate = defaultDelegate;
  }
  connectSignals(delegate, Qt::UniqueConnection);
  Q_CHECK_PTR(delegate);
  return delegate;
}

KPayeeIdentifierView::KPayeeIdentifierView(QWidget* parent)
    : QWidget(parent),
      ui(new Ui::KPayeeIdentifierView)
{
  ui->setupUi(this);
  ui->view->setItemDelegate(new payeeIdentifierDelegate(ui->view));
}

KPayeeIdentifierView::~KPayeeIdentifierView()
{
  delete ui;
}

void KPayeeIdentifierView::setSource(MyMoneyPayeeIdentifierContainer container)
{
  if (ui->view->model() == 0) {
    // this model must be closed after each KMyMoneyApp::fileLoaded signal
    // to limit includes, it is connected outside of this class
    auto model = new payeeIdentifierContainerModel(ui->view);
    connect(model, &payeeIdentifierContainerModel::dataChanged, this, &KPayeeIdentifierView::dataChanged);
    connect(model, &payeeIdentifierContainerModel::rowsRemoved, this, &KPayeeIdentifierView::dataChanged);
    ui->view->setModel(model);
  }

  Q_CHECK_PTR(qobject_cast<payeeIdentifierContainerModel*>(ui->view->model()));    // this should never fail but may help during debugging
  static_cast<payeeIdentifierContainerModel*>(ui->view->model())->setSource(container);

  // Open persistent editor for last row
  ui->view->openPersistentEditor(ui->view->model()->index(ui->view->model()->rowCount(QModelIndex()) - 1, 0));
}

void KPayeeIdentifierView::closeSource()
{
  auto model = ui->view->model();
  if (model)
    static_cast<payeeIdentifierContainerModel*>(model)->closeSource();
}

QList< payeeIdentifier > KPayeeIdentifierView::identifiers() const
{
  const QAbstractItemModel* model = ui->view->model();
  if (model != 0)
    return static_cast<const payeeIdentifierContainerModel*>(model)->identifiers();
  return QList< payeeIdentifier >();
}

/**
 * @brief Helper to sort QModelIndexList in decreasing order.
 */
inline bool QModelIndexRowComparison(const QModelIndex& first, const QModelIndex& second)
{
  return (first.row() > second.row());
}

/**
 * @bug If the last row is removed the type selection editor (which is always behind that last row) closes.
 * Maybe that is a Qt bug?!
 */
void KPayeeIdentifierView::removeSelected()
{
  QModelIndexList selectedRows = ui->view->selectionModel()->selectedRows();
  // To keep the items valid during remove the data must be removed from highest row
  // to the lowest. Unfortunately QList has no reverse iterator.
  std::sort(selectedRows.begin(), selectedRows.end(), QModelIndexRowComparison);

  QAbstractItemModel* model = ui->view->model();
  Q_CHECK_PTR(model);

  QModelIndexList::const_iterator end = selectedRows.constEnd();
  for (QModelIndexList::const_iterator iter = selectedRows.constBegin(); iter != end; ++iter)
    model->removeRow(iter->row(), iter->parent());
}
