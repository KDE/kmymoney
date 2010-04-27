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
#include <QPainter>
#include <QTimer>
#include <QApplication>
#include <QLayout>
#include <QPalette>
#include <QFrame>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kcombobox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoneydateinput.h>
#include <kmymoneyedit.h>
#include <kmymoneycategory.h>

#include "kmymoneyutils.h"
#include "kmymoneyglobalsettings.h"

using namespace KMyMoneyTransactionForm;

TabBar::TabBar(QWidget* parent) :
    KTabWidget(parent),
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

int TabBar::currentIndex(void) const
{
  QMap<int, int>::const_iterator it;
  it = m_idMap.find(KTabWidget::currentIndex());
  if (it != m_idMap.end())
    return *it;
  return -1;
}

void TabBar::setCurrentIndex(int id)
{
  if (widget(id)) // there are no tabs in an expense/income ledger
    if (widget(id)->isEnabled())
      setCurrentWidget(widget(id));
}

QWidget* TabBar::widget(int id) const
{
  /* if a QAccel calls setCurrentTab, id will be as set by qt.
  * however if we call it programmatically, id will
  * be our own id. We do tell QTab about our id but
  * in qt3.3 I (woro) am not able to make sure that
  * QAccel also gets it. See registeritem.h: We defined
  * new values for our own ids which should lie way
  * outside of the range that qt uses
  */
  QWidget* result = KTabWidget::widget(id);
  QMap<int, int>::const_iterator it;
  for (it = m_idMap.begin(); it != m_idMap.end(); ++it)
    if (*it == id)
      result = KTabWidget::widget(it.key());
  return result;
}


void TabBar::setCurrentWidget(QWidget* tab)
{
  if (m_signalType != SignalNormal)
    blockSignals(true);

  KTabWidget::setCurrentWidget(tab);

  if (m_signalType != SignalNormal)
    blockSignals(false);

  if (m_signalType == SignalAlways)
    emit currentChanged(indexOf(tab));
}

void TabBar::insertTab(int id, QWidget* tab, QString title)
{
  KTabWidget::insertTab(id, tab, title);
  setIdentifier(tab, id);
}

void TabBar::setIdentifier(QWidget* tab, int newId)
{
  m_idMap[indexOf(tab)] = newId;
}

void TransactionForm::enableTabBar(bool b)
{
  m_tabBar->setEnabled(b);
}

void TabBar::slotTabCurrentChanged(int id)
{
  QMap<int, int>::const_iterator it;
  it = m_idMap.constFind(id);
  if (it != m_idMap.constEnd())
    emit tabCurrentChanged(*it);
  else
    emit tabCurrentChanged(id);
}

void TabBar::showEvent(QShowEvent* event)
{
  // make sure we don't emit a signal when simply showing the widget
  if (m_signalType != SignalNormal)
    blockSignals(true);

  KTabWidget::showEvent(event);

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
  for (int i = 0; i < otabbar->count(); ++i) {
    QWidget* otab = otabbar->widget(i);
    QWidget* ntab = new QWidget();
    int nid = KTabWidget::addTab(ntab, otabbar->tabText(i));
    m_idMap[nid] = otabbar->m_idMap[i];
    ntab->setEnabled(otab->isEnabled());
    if (i == otabbar->currentIndex())
      setCurrentWidget(ntab);
  }
}

TransactionForm::TransactionForm(QWidget *parent) :
    TransactionEditorContainer(parent),
    m_transaction(0),
    m_tabBar(0)
{
  setFrameShape(Q3Table::NoFrame);
  setShowGrid(false);
  setSelectionMode(Q3Table::NoSelection);
  verticalHeader()->hide();
  horizontalHeader()->hide();
  setLeftMargin(0);
  setTopMargin(0);
  setReadOnly(true);    // display only

  // make sure, that the table is 'invisible' by setting up the right background
  // keep the original color group for painting the cells though
  QPalette p = palette();
  m_cellColorGroup = QColorGroup(p);
  p.setBrush(QPalette::Active, QPalette::Base, p.brush(QPalette::Background));
  p.setBrush(QPalette::Inactive, QPalette::Base, p.brush(QPalette::Background));
  p.setBrush(QPalette::Disabled, QPalette::Base, p.brush(QPalette::Background));
  setPalette(p);

  // never show vertical scroll bars
  setVScrollBarMode(Q3ScrollView::AlwaysOff);

  slotSetTransaction(0);
}

void TransactionForm::drawContents(QPainter *p, int cx, int cy, int cw, int ch)
{
  // the QTable::drawContents() method does not honor the block update flag
  // so we take care of it here
  if (testAttribute(Qt::WA_UpdatesDisabled))
    return;

  Q3Table::drawContents(p, cx, cy, cw, ch);
}

bool TransactionForm::focusNextPrevChild(bool next)
{
  return QFrame::focusNextPrevChild(next);
}

void TransactionForm::clear(void)
{
  slotSetTransaction(0);
}

void TransactionForm::slotSetTransaction(KMyMoneyRegister::Transaction* transaction)
{
  m_transaction = transaction;

  bool updatesNeedToBeDisabled = updatesEnabled();
  if (updatesNeedToBeDisabled)
    setUpdatesEnabled(false);

  if (m_transaction) {
    // the next call sets up a back pointer to the form and also sets up the col and row span
    // as well as the tab of the form
    m_transaction->setupForm(this);

  } else {
    setNumRows(5);
    setNumCols(1);
  }

  kMyMoneyDateInput dateInput(0);
  KMyMoneyCategory category(0, true);

  // extract the maximal sizeHint height
  int height = qMax(dateInput.sizeHint().height(), category.sizeHint().height());

  for (int row = 0; row < numRows(); ++row) {
    if (!transaction || transaction->showRowInForm(row)) {
      showRow(row);
      Q3Table::setRowHeight(row, height);
    } else
      hideRow(row);
  }

  // adjust vertical size of form table
  height *= numRows();
  setMaximumHeight(height);
  setMinimumHeight(height);

  if (updatesNeedToBeDisabled)
    setUpdatesEnabled(true); // see the call to setUpdatesEnabled(false) above

  // force resizeing of the columns
  QTimer::singleShot(0, this, SLOT(resize()));
}

void TransactionForm::paintCell(QPainter* painter, int row, int col, const QRect& r, bool selected, const QColorGroup& /* cg */)
{
  if (m_transaction) {
    m_transaction->paintFormCell(painter, row, col, r, selected, m_cellColorGroup);
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
  // remove all tabs from the tabbar
  QWidget* tab;
  for (tab = m_tabBar->widget(0); tab; tab = m_tabBar->widget(0)) {
    m_tabBar->removeTab(0);
  }

  m_tabBar->show();

  // important: one needs to add the new tabs first and then
  // change the identifier. Otherwise, addTab() will assign
  // a different value
  switch (acc.accountType()) {
  default:
    tab = new QWidget();
    m_tabBar->insertTab(KMyMoneyRegister::ActionDeposit, tab, i18n("&Deposit"));
    tab = new QWidget();
    m_tabBar->insertTab(KMyMoneyRegister::ActionTransfer, tab, i18n("&Transfer"));
    tab = new QWidget();
    m_tabBar->insertTab(KMyMoneyRegister::ActionWithdrawal, tab, i18n("&Withdrawal"));
    break;

  case MyMoneyAccount::CreditCard:
    tab = new QWidget();
    m_tabBar->insertTab(KMyMoneyRegister::ActionDeposit, tab, i18n("&Payment"));
    tab = new QWidget();
    m_tabBar->insertTab(KMyMoneyRegister::ActionTransfer, tab, i18n("&Transfer"));
    tab = new QWidget();
    m_tabBar->insertTab(KMyMoneyRegister::ActionWithdrawal, tab, i18n("&Charge"));
    break;

  case MyMoneyAccount::Liability:
  case MyMoneyAccount::Loan:
    tab = new QWidget();
    m_tabBar->insertTab(KMyMoneyRegister::ActionDeposit, tab, i18n("&Decrease"));
    tab = new QWidget();
    m_tabBar->insertTab(KMyMoneyRegister::ActionTransfer, tab, i18n("&Transfer"));
    tab = new QWidget();
    m_tabBar->insertTab(KMyMoneyRegister::ActionWithdrawal, tab, i18n("&Increase"));
    break;

  case MyMoneyAccount::Asset:
  case MyMoneyAccount::AssetLoan:
    tab = new QWidget();
    m_tabBar->insertTab(KMyMoneyRegister::ActionDeposit, tab, i18n("&Increase"));
    tab = new QWidget();
    m_tabBar->insertTab(KMyMoneyRegister::ActionTransfer, tab, i18n("&Transfer"));
    tab = new QWidget();
    m_tabBar->insertTab(KMyMoneyRegister::ActionWithdrawal, tab, i18n("&Decrease"));
    break;

  case MyMoneyAccount::Income:
  case MyMoneyAccount::Expense:
  case MyMoneyAccount::Investment:
  case MyMoneyAccount::Stock:
    m_tabBar->hide();
    break;
  }
}

void TransactionForm::resize(void)
{
  resize(ValueColumn1);
}

void TransactionForm::resize(int col)
{
  bool updatesNeedToBeDisabled = updatesEnabled();
  if (updatesNeedToBeDisabled)
    setUpdatesEnabled(false);

  // resize the register
  int w = visibleWidth();
  int nc = numCols();

  // check which space we need
  if (nc >= LabelColumn1 && columnWidth(LabelColumn1))
    adjustColumn(LabelColumn1);
  if (nc >= LabelColumn2 && columnWidth(LabelColumn2))
    adjustColumn(LabelColumn2);
  if (nc >= ValueColumn2 && columnWidth(ValueColumn2))
    adjustColumn(ValueColumn2);

  for (int i = 0; i < nc; ++i) {
    if (i == col)
      continue;

    w -= columnWidth(i);
  }
  if (col < nc && w >= 0)
    setColumnWidth(col, w);

  if (updatesNeedToBeDisabled)
    setUpdatesEnabled(true); // see the call to setUpdatesEnabled(false) above
  updateContents();
}

// needed to duplicate this here, as the QTable::tableSize method is private :-(
QSize TransactionForm::tableSize(void) const
{
  return QSize(columnPos(numCols() - 1) + columnWidth(numCols() - 1) + 10,
               rowPos(numRows() - 1) + rowHeight(numRows() - 1) + 10);
}

QSize TransactionForm::sizeHint(void) const
{
  // I've taken this from qtable.cpp, QTable::sizeHint()
  int vmargin = QApplication::isRightToLeft() ? rightMargin() : leftMargin();
  return QSize(tableSize().width() + vmargin + 5, tableSize().height() + topMargin() + 10);
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
    for (int i = numRows() - 1; i >= 0; --i) {
      int align;
      m_transaction->formCellText(txt, align, i, static_cast<int>(col), 0);
      QWidget* cw = cellWidget(i, col);
      if (cw) {
        w = qMax(w, cw->sizeHint().width() + 10);
      }
      w = qMax(w, fontMetrics.width(txt) + 10);
    }
  }

  if (col < numCols())
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

  for (int row = 0; row < numRows(); ++row) {
    for (int col = 0; col < numCols(); ++col) {
      if (cellWidget(row, col))
        clearCellWidget(row, col);
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

#include "transactionform.moc"
