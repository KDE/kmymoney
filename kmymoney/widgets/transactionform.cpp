/*
 * Copyright 2006-2018  Thomas Baumgart <tbaumgart@kde.org>
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

#include "transactionform.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QPalette>
#include <QFrame>
#include <QHeaderView>
#include <QPushButton>
#include <QMouseEvent>
#include <QKeyEvent>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "transactionformitemdelegate.h"
#include "tabbar.h"
#include "amountedit.h"
#include "mymoneyaccount.h"
#include "kmymoneydateinput.h"
#include "kmymoneycategory.h"
#include "transaction.h"

#include "kmymoneysettings.h"

#include "widgetenums.h"
#include "mymoneyenums.h"

using namespace eWidgets;
using namespace KMyMoneyTransactionForm;

namespace KMyMoneyTransactionForm
{
  class TransactionFormPrivate
  {
    Q_DISABLE_COPY(TransactionFormPrivate)

  public:
    TransactionFormPrivate() :
      m_transaction(nullptr),
      m_tabBar(nullptr),
      m_itemDelegate(nullptr)
    {
    }

    KMyMoneyRegister::Transaction   *m_transaction;
    KMyMoneyTransactionForm::TabBar *m_tabBar;
    TransactionFormItemDelegate     *m_itemDelegate;
  };
}

TransactionForm::TransactionForm(QWidget *parent) :
    TransactionEditorContainer(parent),
    d_ptr(new TransactionFormPrivate)
{
  Q_D(TransactionForm);
  d->m_itemDelegate = new TransactionFormItemDelegate(this);
  setFrameShape(QTableWidget::NoFrame);
  setShowGrid(false);
  setSelectionMode(QTableWidget::NoSelection);
  verticalHeader()->hide();
  horizontalHeader()->hide();

  setEditTriggers(QAbstractItemView::NoEditTriggers);

  // make sure, that the table is 'invisible' by setting up the right background
  // keep the original color group for painting the cells though
  QPalette p = palette();
  QBrush brush = p.brush(QPalette::Background);
  QColor color = brush.color();
  color.setAlpha(0);
  brush.setColor(color);
  p.setBrush(QPalette::Active, QPalette::Base, brush);
  p.setBrush(QPalette::Inactive, QPalette::Base, brush);
  p.setBrush(QPalette::Disabled, QPalette::Base, brush);
  setPalette(p);

  slotSetTransaction(0);
}

TransactionForm::~TransactionForm()
{
  Q_D(TransactionForm);
  delete d;
}

bool TransactionForm::focusNextPrevChild(bool next)
{
  return QFrame::focusNextPrevChild(next);
}

void TransactionForm::clear()
{
  slotSetTransaction(0);
}

void TransactionForm::enableTabBar(bool b)
{
  Q_D(TransactionForm);
  d->m_tabBar->setEnabled(b);
}

void TransactionForm::contentsMousePressEvent(QMouseEvent* ev)
{
  ev->ignore();
}

void TransactionForm::contentsMouseMoveEvent(QMouseEvent* ev)
{
  ev->ignore();
}

void TransactionForm::contentsMouseReleaseEvent(QMouseEvent* ev)
{
  ev->ignore();
}

void TransactionForm::contentsMouseDoubleClickEvent(QMouseEvent* ev)
{
  ev->ignore();
}

void TransactionForm::keyPressEvent(QKeyEvent* ev)
{
  ev->ignore();
}


void TransactionForm::slotSetTransaction(KMyMoneyRegister::Transaction* transaction)
{
  Q_D(TransactionForm);
  d->m_transaction = transaction;

  setUpdatesEnabled(false);

  if (d->m_transaction) {
    // the next call sets up a back pointer to the form and also sets up the col and row span
    // as well as the tab of the form
    d->m_transaction->setupForm(this);

  } else {
    setRowCount(5);
    setColumnCount(1);
  }

  KMyMoneyDateInput dateInput;
  KMyMoneyCategory category(true, nullptr);

  // extract the maximal sizeHint height
  int height = qMax(dateInput.sizeHint().height(), category.sizeHint().height());

  for (int row = 0; row < rowCount(); ++row) {
    if (!transaction || transaction->showRowInForm(row)) {
      showRow(row);
      QTableWidget::setRowHeight(row, height);
    } else
      hideRow(row);
  }

  // adjust vertical size of form table
  height *= rowCount();
  setMaximumHeight(height);
  setMinimumHeight(height);

  setUpdatesEnabled(true); // see the call to setUpdatesEnabled(false) above

  for (auto i = 0; i < rowCount(); ++i) {
    setItemDelegateForRow(i, d->m_itemDelegate);
  }

  // force resizeing of the columns
  QMetaObject::invokeMethod(this, "resize", Qt::QueuedConnection, QGenericReturnArgument(), Q_ARG(int, (int)eTransactionForm::Column::Value1));
}

void TransactionForm::paintCell(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index)
{
  Q_D(TransactionForm);
  if (d->m_transaction) {
    d->m_transaction->paintFormCell(painter, option, index);
  }
}

void TransactionForm::setCurrentCell(int, int)
{
}

KMyMoneyTransactionForm::TabBar* TransactionForm::getTabBar(QWidget* parent)
{
  Q_D(TransactionForm);
  if (!d->m_tabBar && parent) {
    // determine the height of the objects in the table
    // create the tab bar
    d->m_tabBar = new TabBar(parent);
    d->m_tabBar->setSignalEmission(eTabBar::SignalEmission::Always);
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    sizePolicy.setHeightForWidth(d->m_tabBar->sizePolicy().hasHeightForWidth());
    d->m_tabBar->setSizePolicy(sizePolicy);
    connect(d->m_tabBar, &TabBar::tabCurrentChanged, this, &TransactionForm::slotActionSelected);
  }
  return d->m_tabBar;
}

void TransactionForm::slotActionSelected(int id)
{
  emit newTransaction(static_cast<eRegister::Action>(id));
}

void TransactionForm::setupForm(const MyMoneyAccount& acc)
{
  Q_D(TransactionForm);
  bool blocked = d->m_tabBar->blockSignals(true);

  // remove all tabs from the tabbar
  while (d->m_tabBar->count())
    d->m_tabBar->removeTab(0);

  d->m_tabBar->show();

  // important: one needs to add the new tabs first and then
  // change the identifier. Otherwise, addTab() will assign
  // a different value
  switch (acc.accountType()) {
    default:
      d->m_tabBar->insertTab((int)eRegister::Action::Deposit, i18n("&Deposit"));
      d->m_tabBar->insertTab((int)eRegister::Action::Transfer, i18n("&Transfer"));
      d->m_tabBar->insertTab((int)eRegister::Action::Withdrawal, i18n("&Withdrawal"));
      break;

    case eMyMoney::Account::Type::CreditCard:
      d->m_tabBar->insertTab((int)eRegister::Action::Deposit, i18n("&Payment"));
      d->m_tabBar->insertTab((int)eRegister::Action::Transfer, i18n("&Transfer"));
      d->m_tabBar->insertTab((int)eRegister::Action::Withdrawal, i18n("&Charge"));
      break;

    case eMyMoney::Account::Type::Liability:
    case eMyMoney::Account::Type::Loan:
      d->m_tabBar->insertTab((int)eRegister::Action::Deposit, i18n("&Decrease"));
      d->m_tabBar->insertTab((int)eRegister::Action::Transfer, i18n("&Transfer"));
      d->m_tabBar->insertTab((int)eRegister::Action::Withdrawal, i18n("&Increase"));
      break;

    case eMyMoney::Account::Type::Asset:
    case eMyMoney::Account::Type::AssetLoan:
      d->m_tabBar->insertTab((int)eRegister::Action::Deposit, i18n("&Increase"));
      d->m_tabBar->insertTab((int)eRegister::Action::Transfer, i18n("&Transfer"));
      d->m_tabBar->insertTab((int)eRegister::Action::Withdrawal, i18n("&Decrease"));
      break;

    case eMyMoney::Account::Type::Income:
    case eMyMoney::Account::Type::Expense:
    case eMyMoney::Account::Type::Investment:
    case eMyMoney::Account::Type::Stock:
      d->m_tabBar->hide();
      break;
  }
  d->m_tabBar->blockSignals(blocked);
}

void TransactionForm::resize(int col)
{
  setUpdatesEnabled(false);

  // resize the register
  int w = viewport()->width();
  int nc = columnCount();

  // check which space we need
  if (nc >= (int)eTransactionForm::Column::Label1 && columnWidth((int)eTransactionForm::Column::Label1))
    adjustColumn(eTransactionForm::Column::Label1);
  if (nc >= (int)eTransactionForm::Column::Value1 && columnWidth((int)eTransactionForm::Column::Value1))
    adjustColumn(eTransactionForm::Column::Value1);
  if (nc >= (int)eTransactionForm::Column::Label2 && columnWidth((int)eTransactionForm::Column::Label2))
    adjustColumn(eTransactionForm::Column::Label2);
  if (nc >= (int)eTransactionForm::Column::Value2 && columnWidth((int)eTransactionForm::Column::Value2))
    adjustColumn(eTransactionForm::Column::Value2);

  for (auto i = 0; i < nc; ++i) {
    if (i == col)
      continue;

    w -= columnWidth(i);
  }
  if (col < nc && w >= 0)
    setColumnWidth(col, w);

  setUpdatesEnabled(true);
}

void TransactionForm::paintFocus(QPainter* /*p*/, const QRect& /*cr*/)
{
}

void TransactionForm::adjustColumn(eTransactionForm::Column col)
{
  Q_D(TransactionForm);
  int w = 0;

  // preset the width of the right value column with the width of
  // the possible edit widgets so that they fit if they pop up
  if (col == eTransactionForm::Column::Value2) {
    KMyMoneyDateInput dateInput;
    AmountEdit valInput;
    w = qMax(dateInput.sizeHint().width(), valInput.sizeHint().width());
  }

  if (d->m_transaction) {
    QString txt;
    QFontMetrics fontMetrics(KMyMoneySettings::listCellFontEx());

    // scan through the rows
    for (int i = rowCount() - 1; i >= 0; --i) {
      Qt::Alignment align;
      int spacing = 10;
      d->m_transaction->formCellText(txt, align, i, static_cast<int>(col), 0);
      QWidget* cw = cellWidget(i, (int)col);
      if (cw) {
        w = qMax(w, cw->sizeHint().width() + spacing);
        // if the cell widget contains a push button increase the spacing used
        // for the cell text value to consider the size of the push button
        if (QPushButton *pushButton = cw->findChild<QPushButton *>()) {
          spacing += pushButton->sizeHint().width() + 5;
        }
      }
      w = qMax(w, fontMetrics.width(txt) + spacing);
    }
  }

  if ((int)col < columnCount())
    setColumnWidth((int)col, w);
}

void TransactionForm::arrangeEditWidgets(QMap<QString, QWidget*>& editWidgets, KMyMoneyRegister::Transaction* t)
{
  t->arrangeWidgetsInForm(editWidgets);
  resize((int)eTransactionForm::Column::Value1);
}

void TransactionForm::tabOrder(QWidgetList& tabOrderWidgets, KMyMoneyRegister::Transaction* t) const
{
  t->tabOrderInForm(tabOrderWidgets);
}

void TransactionForm::removeEditWidgets(QMap<QString, QWidget*>& editWidgets)
{
  QMap<QString, QWidget*>::iterator it;
  for (it = editWidgets.begin(); it != editWidgets.end();) {
    if ((*it)->parentWidget() == this) {
      editWidgets.erase(it);
      it = editWidgets.begin();
    } else
      ++it;
  }

  for (int row = 0; row < rowCount(); ++row) {
    for (int col = 0; col < columnCount(); ++col) {
      if (cellWidget(row, col)) {
        cellWidget(row, col)->hide();
        setCellWidget(row, col, 0);
      }
    }
  }
  resize((int)eTransactionForm::Column::Value1);

  // delete all remaining edit widgets   (e.g. tabbar)
  for (it = editWidgets.begin(); it != editWidgets.end();) {
    delete(*it);  // ->deleteLater();
    editWidgets.erase(it);
    it = editWidgets.begin();
  }
}
