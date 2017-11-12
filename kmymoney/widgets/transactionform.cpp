/***************************************************************************
                             transactionform.cpp
                             -------------------
    begin                : Sun May 14 2006
    copyright            : (C) 2006 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "transactionform.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QPalette>
#include <QFrame>
#include <QHeaderView>
#include <QPushButton>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyaccount.h"
#include "kmymoneydateinput.h"
#include "kmymoneyedit.h"
#include "kmymoneycategory.h"
#include "transaction.h"

#include "kmymoneyglobalsettings.h"

using namespace KMyMoneyTransactionForm;

TabBar::TabBar(QWidget* parent) :
    QTabBar(parent),
    m_signalType(SignalNormal)
{
  connect(this, SIGNAL(currentChanged(int)), this, SLOT(slotTabCurrentChanged(int)));
}

TabBar::SignalEmissionE TabBar::setSignalEmission(TabBar::SignalEmissionE type)
{
  TabBar::SignalEmissionE _type = m_signalType;
  m_signalType = type;
  return _type;
}

int TabBar::currentIndex() const
{
  QMap<int, int>::const_iterator it;
  int id = QTabBar::currentIndex();
  for (it = m_idMap.constBegin(); it != m_idMap.constEnd(); ++it) {
    if (*it == id) {
      return it.key();
    }
  }
  return -1;
}

void TabBar::setCurrentIndex(int id)
{
  if (m_signalType != SignalNormal)
    blockSignals(true);

  if (m_idMap.contains(id)) {
    QTabBar::setCurrentIndex(m_idMap[id]);
  }

  if (m_signalType != SignalNormal)
    blockSignals(false);

  if (m_signalType == SignalAlways)
    emit currentChanged(m_idMap[id]);
}

void TabBar::setTabEnabled(int id, bool enable)
{
  if (m_idMap.contains(id)) {
    QTabBar::setTabEnabled(m_idMap[id], enable);
  }
}

void TabBar::insertTab(int id, const QString& title)
{
  int newId = QTabBar::insertTab(id, title);
  m_idMap[id] = newId;
}

void TabBar::slotTabCurrentChanged(int id)
{
  QMap<int, int>::const_iterator it;
  for (it = m_idMap.constBegin(); it != m_idMap.constEnd(); ++it) {
    if (*it == id) {
      emit tabCurrentChanged(it.key());
      break;
    }
  }
  if (it == m_idMap.constEnd())
    emit tabCurrentChanged(id);
}

void TabBar::showEvent(QShowEvent* event)
{
  // make sure we don't emit a signal when simply showing the widget
  if (m_signalType != SignalNormal)
    blockSignals(true);

  QTabBar::showEvent(event);

  if (m_signalType != SignalNormal)
    blockSignals(false);
}

void TabBar::copyTabs(const TabBar* otabbar)
{
  // remove all existing tabs
  while (count()) {
    removeTab(0);
  }

  // now create new ones. copy text, icon and identifier
  m_idMap = otabbar->m_idMap;

  for (auto i = 0; i < otabbar->count(); ++i) {
    QTabBar::insertTab(i, otabbar->tabText(i));
    if (i == otabbar->QTabBar::currentIndex()) {
      QTabBar::setCurrentIndex(i);
    }
  }
}

int TabBar::indexAtPos(const QPoint& p) const
{
  if (tabRect(QTabBar::currentIndex()).contains(p))
    return QTabBar::currentIndex();
  for (auto i = 0; i < count(); ++i)
    if (isTabEnabled(i) && tabRect(i).contains(p))
      return i;
  return -1;
}

void TabBar::mousePressEvent(QMouseEvent *e)
{
  QTabBar::mousePressEvent(e);

  // in case we receive a mouse press event on the current
  // selected tab emit a signal no matter what as the base
  // class does not do that
  if (indexAtPos(e->pos()) == QTabBar::currentIndex()) {
    slotTabCurrentChanged(QTabBar::currentIndex());
  }
}

TransactionFormItemDelegate::TransactionFormItemDelegate(TransactionForm *parent) : QStyledItemDelegate(parent), m_transactionForm(parent)
{
}

TransactionFormItemDelegate::~TransactionFormItemDelegate()
{
}

void TransactionFormItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  m_transactionForm->paintCell(painter, option, index);
}

TransactionForm::TransactionForm(QWidget *parent) :
    TransactionEditorContainer(parent),
    m_transaction(0),
    m_tabBar(0)
{
  m_itemDelegate = new TransactionFormItemDelegate(this);
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
  m_tabBar->setEnabled(b);
}

void TransactionForm::slotSetTransaction(KMyMoneyRegister::Transaction* transaction)
{
  m_transaction = transaction;

  setUpdatesEnabled(false);

  if (m_transaction) {
    // the next call sets up a back pointer to the form and also sets up the col and row span
    // as well as the tab of the form
    m_transaction->setupForm(this);

  } else {
    setRowCount(5);
    setColumnCount(1);
  }

  kMyMoneyDateInput dateInput;
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
    setItemDelegateForRow(i, m_itemDelegate);
  }

  // force resizeing of the columns
  QMetaObject::invokeMethod(this, "resize", Qt::QueuedConnection, QGenericReturnArgument(), Q_ARG(int, ValueColumn1));
}

void TransactionForm::paintCell(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index)
{
  if (m_transaction) {
    m_transaction->paintFormCell(painter, option, index);
  }
}

TabBar* TransactionForm::tabBar(QWidget* parent)
{
  if (!m_tabBar && parent) {
    // determine the height of the objects in the table
    // create the tab bar
    m_tabBar = new TabBar(parent);
    m_tabBar->setSignalEmission(TabBar::SignalAlways);
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    sizePolicy.setHeightForWidth(m_tabBar->sizePolicy().hasHeightForWidth());
    m_tabBar->setSizePolicy(sizePolicy);
    connect(m_tabBar, SIGNAL(tabCurrentChanged(int)), this, SLOT(slotActionSelected(int)));
  }
  return m_tabBar;
}

void TransactionForm::slotActionSelected(int id)
{
  emit newTransaction(static_cast<KMyMoneyRegister::Action>(id));
}

void TransactionForm::setupForm(const MyMoneyAccount& acc)
{
  bool blocked = m_tabBar->blockSignals(true);

  // remove all tabs from the tabbar
  while (m_tabBar->count())
    m_tabBar->removeTab(0);

  m_tabBar->show();

  // important: one needs to add the new tabs first and then
  // change the identifier. Otherwise, addTab() will assign
  // a different value
  switch (acc.accountType()) {
    default:
      m_tabBar->insertTab(KMyMoneyRegister::ActionDeposit, i18n("&Deposit"));
      m_tabBar->insertTab(KMyMoneyRegister::ActionTransfer, i18n("&Transfer"));
      m_tabBar->insertTab(KMyMoneyRegister::ActionWithdrawal, i18n("&Withdrawal"));
      break;

    case eMyMoney::Account::CreditCard:
      m_tabBar->insertTab(KMyMoneyRegister::ActionDeposit, i18n("&Payment"));
      m_tabBar->insertTab(KMyMoneyRegister::ActionTransfer, i18n("&Transfer"));
      m_tabBar->insertTab(KMyMoneyRegister::ActionWithdrawal, i18n("&Charge"));
      break;

    case eMyMoney::Account::Liability:
    case eMyMoney::Account::Loan:
      m_tabBar->insertTab(KMyMoneyRegister::ActionDeposit, i18n("&Decrease"));
      m_tabBar->insertTab(KMyMoneyRegister::ActionTransfer, i18n("&Transfer"));
      m_tabBar->insertTab(KMyMoneyRegister::ActionWithdrawal, i18n("&Increase"));
      break;

    case eMyMoney::Account::Asset:
    case eMyMoney::Account::AssetLoan:
      m_tabBar->insertTab(KMyMoneyRegister::ActionDeposit, i18n("&Increase"));
      m_tabBar->insertTab(KMyMoneyRegister::ActionTransfer, i18n("&Transfer"));
      m_tabBar->insertTab(KMyMoneyRegister::ActionWithdrawal, i18n("&Decrease"));
      break;

    case eMyMoney::Account::Income:
    case eMyMoney::Account::Expense:
    case eMyMoney::Account::Investment:
    case eMyMoney::Account::Stock:
      m_tabBar->hide();
      break;
  }
  m_tabBar->blockSignals(blocked);
}

void TransactionForm::resize(int col)
{
  setUpdatesEnabled(false);

  // resize the register
  int w = viewport()->width();
  int nc = columnCount();

  // check which space we need
  if (nc >= LabelColumn1 && columnWidth(LabelColumn1))
    adjustColumn(LabelColumn1);
  if (nc >= ValueColumn1 && columnWidth(ValueColumn1))
    adjustColumn(ValueColumn1);
  if (nc >= LabelColumn2 && columnWidth(LabelColumn2))
    adjustColumn(LabelColumn2);
  if (nc >= ValueColumn2 && columnWidth(ValueColumn2))
    adjustColumn(ValueColumn2);

  for (auto i = 0; i < nc; ++i) {
    if (i == col)
      continue;

    w -= columnWidth(i);
  }
  if (col < nc && w >= 0)
    setColumnWidth(col, w);

  setUpdatesEnabled(true);
}

void TransactionForm::adjustColumn(Column col)
{
  int w = 0;

  // preset the width of the right value column with the width of
  // the possible edit widgets so that they fit if they pop up
  if (col == ValueColumn2) {
    kMyMoneyDateInput dateInput;
    kMyMoneyEdit valInput;
    w = qMax(dateInput.sizeHint().width(), valInput.sizeHint().width());
  }

  if (m_transaction) {
    QString txt;
    QFontMetrics fontMetrics(KMyMoneyGlobalSettings::listCellFont());

    // scan through the rows
    for (int i = rowCount() - 1; i >= 0; --i) {
      Qt::Alignment align;
      int spacing = 10;
      m_transaction->formCellText(txt, align, i, static_cast<int>(col), 0);
      QWidget* cw = cellWidget(i, col);
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

  if (col < columnCount())
    setColumnWidth(col, w);
}

void TransactionForm::arrangeEditWidgets(QMap<QString, QWidget*>& editWidgets, KMyMoneyRegister::Transaction* t)
{
  t->arrangeWidgetsInForm(editWidgets);
  resize(ValueColumn1);
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
  resize(ValueColumn1);

  // delete all remaining edit widgets   (e.g. tabbar)
  for (it = editWidgets.begin(); it != editWidgets.end();) {
    delete(*it);  // ->deleteLater();
    editWidgets.erase(it);
    it = editWidgets.begin();
  }
}
