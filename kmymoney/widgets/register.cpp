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

#include "register.h"

#include <typeinfo>

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QToolTip>
#include <QMouseEvent>
#include <QList>
#include <QKeyEvent>
#include <QEvent>
#include <QFrame>
#include <QHeaderView>
#include <QApplication>
#include <QPushButton>
#include <QTimer>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "mymoneyexception.h"
#include "mymoneyaccount.h"
#include "stdtransactiondownloaded.h"
#include "stdtransactionmatched.h"
#include "selectedtransactions.h"
#include "scheduledtransaction.h"
#include "kmymoneysettings.h"
#include "mymoneymoney.h"
#include "mymoneyfile.h"
#include "groupmarkers.h"
#include "fancydategroupmarkers.h"
#include "registeritemdelegate.h"
#include "itemptrvector.h"
#include "mymoneyenums.h"
#include "widgetenums.h"

using namespace KMyMoneyRegister;
using namespace eWidgets;
using namespace eMyMoney;

namespace KMyMoneyRegister
{
  class RegisterPrivate
  {
  public:
    RegisterPrivate() :
      m_selectAnchor(nullptr),
      m_focusItem(nullptr),
      m_ensureVisibleItem(nullptr),
      m_firstItem(nullptr),
      m_lastItem(nullptr),
      m_firstErroneous(nullptr),
      m_lastErroneous(nullptr),
      m_rowHeightHint(0),
      m_ledgerLensForced(false),
      m_selectionMode(QTableWidget::MultiSelection),
      m_needResize(true),
      m_listsDirty(false),
      m_ignoreNextButtonRelease(false),
      m_needInitialColumnResize(false),
      m_usedWithEditor(false),
      m_mouseButton(Qt::MouseButtons(Qt::NoButton)),
      m_modifiers(Qt::KeyboardModifiers(Qt::NoModifier)),
      m_lastCol(eTransaction::Column::Account),
      m_detailsColumnType(eRegister::DetailColumn::PayeeFirst)
    {
    }

    ~RegisterPrivate()
    {
    }

    ItemPtrVector                m_items;
    QVector<RegisterItem*>       m_itemIndex;
    RegisterItem*                m_selectAnchor;
    RegisterItem*                m_focusItem;
    RegisterItem*                m_ensureVisibleItem;
    RegisterItem*                m_firstItem;
    RegisterItem*                m_lastItem;
    RegisterItem*                m_firstErroneous;
    RegisterItem*                m_lastErroneous;

    int                          m_markErroneousTransactions;
    int                          m_rowHeightHint;

    MyMoneyAccount               m_account;

    bool                         m_ledgerLensForced;
    QAbstractItemView::SelectionMode m_selectionMode;

    bool                         m_needResize;
    bool                         m_listsDirty;
    bool                         m_ignoreNextButtonRelease;
    bool                         m_needInitialColumnResize;
    bool                         m_usedWithEditor;
    Qt::MouseButtons             m_mouseButton;
    Qt::KeyboardModifiers        m_modifiers;
    eTransaction::Column         m_lastCol;
    QList<SortField>             m_sortOrder;
    QRect                        m_lastRepaintRect;
    eRegister::DetailColumn      m_detailsColumnType;

  };

  Register::Register(QWidget *parent) :
    TransactionEditorContainer(parent),
    d_ptr(new RegisterPrivate)
  {
    // used for custom coloring with the help of the application's stylesheet
    setObjectName(QLatin1String("register"));
    setItemDelegate(new RegisterItemDelegate(this));

    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setColumnCount((int)eTransaction::Column::LastColumn);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setAcceptDrops(true);
    setShowGrid(false);
    setContextMenuPolicy(Qt::DefaultContextMenu);

    setHorizontalHeaderItem((int)eTransaction::Column::Number, new QTableWidgetItem());
    setHorizontalHeaderItem((int)eTransaction::Column::Date, new QTableWidgetItem());
    setHorizontalHeaderItem((int)eTransaction::Column::Account, new QTableWidgetItem());
    setHorizontalHeaderItem((int)eTransaction::Column::Security, new QTableWidgetItem());
    setHorizontalHeaderItem((int)eTransaction::Column::Detail, new QTableWidgetItem());
    setHorizontalHeaderItem((int)eTransaction::Column::ReconcileFlag, new QTableWidgetItem());
    setHorizontalHeaderItem((int)eTransaction::Column::Payment, new QTableWidgetItem());
    setHorizontalHeaderItem((int)eTransaction::Column::Deposit, new QTableWidgetItem());
    setHorizontalHeaderItem((int)eTransaction::Column::Quantity, new QTableWidgetItem());
    setHorizontalHeaderItem((int)eTransaction::Column::Price, new QTableWidgetItem());
    setHorizontalHeaderItem((int)eTransaction::Column::Value, new QTableWidgetItem());
    setHorizontalHeaderItem((int)eTransaction::Column::Balance, new QTableWidgetItem());

    // keep the following list in sync with KMyMoneyRegister::Column in transaction.h
    horizontalHeaderItem((int)eTransaction::Column::Number)->setText(i18nc("Cheque Number", "No."));
    horizontalHeaderItem((int)eTransaction::Column::Date)->setText(i18n("Date"));
    horizontalHeaderItem((int)eTransaction::Column::Account)->setText(i18n("Account"));
    horizontalHeaderItem((int)eTransaction::Column::Security)->setText(i18n("Security"));
    horizontalHeaderItem((int)eTransaction::Column::Detail)->setText(i18n("Details"));
    horizontalHeaderItem((int)eTransaction::Column::ReconcileFlag)->setText(i18n("C"));
    horizontalHeaderItem((int)eTransaction::Column::Payment)->setText(i18n("Payment"));
    horizontalHeaderItem((int)eTransaction::Column::Deposit)->setText(i18n("Deposit"));
    horizontalHeaderItem((int)eTransaction::Column::Quantity)->setText(i18n("Quantity"));
    horizontalHeaderItem((int)eTransaction::Column::Price)->setText(i18n("Price"));
    horizontalHeaderItem((int)eTransaction::Column::Value)->setText(i18n("Value"));
    horizontalHeaderItem((int)eTransaction::Column::Balance)->setText(i18n("Balance"));

    verticalHeader()->hide();

    horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    horizontalHeader()->setSortIndicatorShown(false);
    horizontalHeader()->setSectionsMovable(false);
    horizontalHeader()->setSectionsClickable(false);
    horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, &QTableWidget::cellClicked, this, static_cast<void (Register::*)(int, int)>(&Register::selectItem));
    connect(this, &QTableWidget::cellDoubleClicked, this, &Register::slotDoubleClicked);
  }

  Register::~Register()
  {
    Q_D(Register);
    clear();
    delete d;
  }

  bool Register::eventFilter(QObject* o, QEvent* e)
  {
    if (o == this && e->type() == QEvent::KeyPress) {
      auto ke = dynamic_cast<QKeyEvent*>(e);
      if (ke && ke->key() == Qt::Key_Menu) {
        emit openContextMenu();
        return true;
      }
    }
    return QTableWidget::eventFilter(o, e);
  }

  void Register::setupRegister(const MyMoneyAccount& account, const QList<eTransaction::Column>& cols)
  {
    Q_D(Register);
    d->m_account = account;
    setUpdatesEnabled(false);

    for (auto i = 0; i < (int)eTransaction::Column::LastColumn; ++i)
      hideColumn(i);

    d->m_needInitialColumnResize = true;

    d->m_lastCol = static_cast<eTransaction::Column>(0);
    QList<eTransaction::Column>::const_iterator it_c;
    for (it_c = cols.begin(); it_c != cols.end(); ++it_c) {
      if ((*it_c) > eTransaction::Column::LastColumn)
        continue;
      showColumn((int)*it_c);
      if (*it_c > d->m_lastCol)
        d->m_lastCol = *it_c;
    }

    setUpdatesEnabled(true);
  }

  void Register::setupRegister(const MyMoneyAccount& account, bool showAccountColumn)
  {
    Q_D(Register);
    d->m_account = account;
    setUpdatesEnabled(false);

    for (auto i = 0; i < (int)eTransaction::Column::LastColumn; ++i)
      hideColumn(i);

    horizontalHeaderItem((int)eTransaction::Column::Payment)->setText(i18nc("Payment made from account", "Payment"));
    horizontalHeaderItem((int)eTransaction::Column::Deposit)->setText(i18nc("Deposit into account", "Deposit"));

    if (account.id().isEmpty()) {
      setUpdatesEnabled(true);
      return;
    }

    d->m_needInitialColumnResize = true;

    // turn on standard columns
    showColumn((int)eTransaction::Column::Date);
    showColumn((int)eTransaction::Column::Detail);
    showColumn((int)eTransaction::Column::ReconcileFlag);

    // balance
    switch (account.accountType()) {
      case Account::Type::Stock:
        break;
      default:
        showColumn((int)eTransaction::Column::Balance);
        break;
    }

    // Number column
    switch (account.accountType()) {
      case Account::Type::Savings:
      case Account::Type::Cash:
      case Account::Type::Loan:
      case Account::Type::AssetLoan:
      case Account::Type::Asset:
      case Account::Type::Liability:
      case Account::Type::Equity:
        if (KMyMoneySettings::alwaysShowNrField())
          showColumn((int)eTransaction::Column::Number);
        break;

      case Account::Type::Checkings:
      case Account::Type::CreditCard:
        showColumn((int)eTransaction::Column::Number);
        break;

      default:
        hideColumn((int)eTransaction::Column::Number);
        break;
    }

    switch (account.accountType()) {
      case Account::Type::Income:
      case Account::Type::Expense:
        showAccountColumn = true;
        break;
      default:
        break;
    }

    if (showAccountColumn)
      showColumn((int)eTransaction::Column::Account);

    // Security, activity, payment, deposit, amount, price and value column
    switch (account.accountType()) {
      default:
        showColumn((int)eTransaction::Column::Payment);
        showColumn((int)eTransaction::Column::Deposit);
        break;

      case Account::Type::Investment:
        showColumn((int)eTransaction::Column::Security);
        showColumn((int)eTransaction::Column::Quantity);
        showColumn((int)eTransaction::Column::Price);
        showColumn((int)eTransaction::Column::Value);
        break;
    }

    // headings
    switch (account.accountType()) {
      case Account::Type::CreditCard:
        horizontalHeaderItem((int)eTransaction::Column::Payment)->setText(i18nc("Payment made with credit card", "Charge"));
        horizontalHeaderItem((int)eTransaction::Column::Deposit)->setText(i18nc("Payment towards credit card", "Payment"));
        break;
      case Account::Type::Asset:
      case Account::Type::AssetLoan:
        horizontalHeaderItem((int)eTransaction::Column::Payment)->setText(i18nc("Decrease of asset/liability value", "Decrease"));
        horizontalHeaderItem((int)eTransaction::Column::Deposit)->setText(i18nc("Increase of asset/liability value", "Increase"));
        break;
      case Account::Type::Liability:
      case Account::Type::Loan:
        horizontalHeaderItem((int)eTransaction::Column::Payment)->setText(i18nc("Increase of asset/liability value", "Increase"));
        horizontalHeaderItem((int)eTransaction::Column::Deposit)->setText(i18nc("Decrease of asset/liability value", "Decrease"));
        break;
      case Account::Type::Income:
      case Account::Type::Expense:
        horizontalHeaderItem((int)eTransaction::Column::Payment)->setText(i18n("Income"));
        horizontalHeaderItem((int)eTransaction::Column::Deposit)->setText(i18n("Expense"));
        break;

      default:
        break;
    }

    d->m_lastCol = eTransaction::Column::Balance;

    setUpdatesEnabled(true);
  }

  bool Register::focusNextPrevChild(bool next)
  {
    return QFrame::focusNextPrevChild(next);
  }

  void Register::setSortOrder(const QString& order)
  {
    Q_D(Register);
    const QStringList orderList = order.split(',', QString::SkipEmptyParts);
    QStringList::const_iterator it;
    d->m_sortOrder.clear();
    for (it = orderList.constBegin(); it != orderList.constEnd(); ++it) {
      d->m_sortOrder << static_cast<SortField>((*it).toInt());
    }
  }

  const QList<SortField>& Register::sortOrder() const
  {
    Q_D(const Register);
    return d->m_sortOrder;
  }

  void Register::sortItems()
  {
    Q_D(Register);
    if (d->m_items.count() == 0)
      return;

    // sort the array of pointers to the transactions
    d->m_items.sort();

    // update the next/prev item chains
    RegisterItem* prev = 0;
    RegisterItem* item;
    d->m_firstItem = d->m_lastItem = 0;
    for (QVector<RegisterItem*>::size_type i = 0; i < d->m_items.size(); ++i) {
      item = d->m_items[i];
      if (!item)
        continue;

      if (!d->m_firstItem)
        d->m_firstItem = item;
      d->m_lastItem = item;
      if (prev)
        prev->setNextItem(item);
      item->setPrevItem(prev);
      item->setNextItem(0);
      prev = item;
    }

    // update the balance visibility settings
    item = d->m_lastItem;
    bool showBalance = true;
    while (item) {
      auto t = dynamic_cast<Transaction*>(item);
      if (t) {
        t->setShowBalance(showBalance);
        if (!t->isVisible()) {
          showBalance = false;
        }
      }
      item = item->prevItem();
    }

    // force update of the item index (row to item array)
    d->m_listsDirty = true;
  }

  eTransaction::Column Register::lastCol() const
  {
    Q_D(const Register);
    return d->m_lastCol;
  }

  SortField Register::primarySortKey() const
  {
    Q_D(const Register);
    if (!d->m_sortOrder.isEmpty())
      return static_cast<SortField>(d->m_sortOrder.first());
    return SortField::Unknown;
  }


  void Register::clear()
  {
    Q_D(Register);
    d->m_firstErroneous = d->m_lastErroneous = 0;
    d->m_ensureVisibleItem = 0;

    d->m_items.clear();

    RegisterItem* p;
    while ((p = firstItem()) != 0) {
      delete p;
    }

    d->m_firstItem = d->m_lastItem = 0;

    d->m_listsDirty = true;
    d->m_selectAnchor = 0;
    d->m_focusItem = 0;

#ifndef KMM_DESIGNER
    // recalculate row height hint
    QFontMetrics fm(KMyMoneySettings::listCellFontEx());
    d->m_rowHeightHint = fm.lineSpacing() + 6;
#endif

    d->m_needInitialColumnResize = true;
    d->m_needResize = true;
    updateRegister(true);
  }

  void Register::insertItemAfter(RegisterItem*p, RegisterItem* prev)
  {
    Q_D(Register);
    RegisterItem* next = 0;
    if (!prev)
      prev = lastItem();

    if (prev) {
      next = prev->nextItem();
      prev->setNextItem(p);
    }
    if (next)
      next->setPrevItem(p);

    p->setPrevItem(prev);
    p->setNextItem(next);

    if (!d->m_firstItem)
      d->m_firstItem = p;
    if (!d->m_lastItem)
      d->m_lastItem = p;

    if (prev == d->m_lastItem)
      d->m_lastItem = p;

    d->m_listsDirty = true;
    d->m_needResize = true;
  }

  void Register::addItem(RegisterItem* p)
  {
    Q_D(Register);
    RegisterItem* q = lastItem();
    if (q)
      q->setNextItem(p);
    p->setPrevItem(q);
    p->setNextItem(0);

    d->m_items.append(p);

    if (!d->m_firstItem)
      d->m_firstItem = p;
    d->m_lastItem = p;
    d->m_listsDirty = true;
    d->m_needResize = true;
  }

  void Register::removeItem(RegisterItem* p)
  {
    Q_D(Register);
    // remove item from list
    if (p->prevItem())
      p->prevItem()->setNextItem(p->nextItem());
    if (p->nextItem())
      p->nextItem()->setPrevItem(p->prevItem());

    // update first and last pointer if required
    if (p == d->m_firstItem)
      d->m_firstItem = p->nextItem();
    if (p == d->m_lastItem)
      d->m_lastItem = p->prevItem();

    // make sure we don't do it twice
    p->setNextItem(0);
    p->setPrevItem(0);

    // remove it from the m_items array
    int i = d->m_items.indexOf(p);
    if (-1 != i) {
      d->m_items[i] = 0;
    }
    d->m_listsDirty = true;
    d->m_needResize = true;
  }

  RegisterItem* Register::firstItem() const
  {
    Q_D(const Register);
    return d->m_firstItem;
  }

  RegisterItem* Register::nextItem(RegisterItem* item) const
  {
    return item->nextItem();
  }

  RegisterItem* Register::lastItem() const
  {
    Q_D(const Register);
    return d->m_lastItem;
  }

  void Register::setupItemIndex(int rowCount)
  {
    Q_D(Register);
    // setup index array
    d->m_itemIndex.clear();
    d->m_itemIndex.reserve(rowCount);

    // fill index array
    rowCount = 0;
    RegisterItem* prev = 0;
    d->m_firstItem = d->m_lastItem = 0;
    for (QVector<RegisterItem*>::size_type i = 0; i < d->m_items.size(); ++i) {
      RegisterItem* item = d->m_items[i];
      if (!item)
        continue;
      if (!d->m_firstItem)
        d->m_firstItem = item;
      d->m_lastItem = item;
      if (prev)
        prev->setNextItem(item);
      item->setPrevItem(prev);
      item->setNextItem(0);
      prev = item;
      for (int j = item->numRowsRegister(); j; --j) {
        d->m_itemIndex.push_back(item);
      }
    }
  }

  void Register::updateAlternate() const
  {
    Q_D(const Register);
    bool alternate = false;
    for (QVector<RegisterItem*>::size_type i = 0; i < d->m_items.size(); ++i) {
      RegisterItem* item = d->m_items[i];
      if (!item)
        continue;
      if (item->isVisible()) {
        item->setAlternate(alternate);
        alternate ^= true;
      }
    }
  }

  void Register::suppressAdjacentMarkers()
  {
    bool lastWasGroupMarker = false;
    KMyMoneyRegister::RegisterItem* p = lastItem();
    auto t = dynamic_cast<KMyMoneyRegister::Transaction*>(p);
    if (t && t->transaction().id().isEmpty()) {
      lastWasGroupMarker = true;
      p = p->prevItem();
    }
    while (p) {
      auto m = dynamic_cast<KMyMoneyRegister::GroupMarker*>(p);
      if (m) {
        // make adjacent group marker invisible except those that show statement information
        if (lastWasGroupMarker && (dynamic_cast<KMyMoneyRegister::StatementGroupMarker*>(m) == 0)) {
          m->setVisible(false);
        }
        lastWasGroupMarker = true;
      } else if (p->isVisible())
        lastWasGroupMarker = false;
      p = p->prevItem();
    }
  }

  void Register::updateRegister(bool forceUpdateRowHeight)
  {
    Q_D(Register);
    if (d->m_listsDirty || forceUpdateRowHeight) {
      // don't get in here recursively
      d->m_listsDirty = false;

      int rowCount = 0;
      // determine the number of rows we need to display all items
      // while going through the list, check for erroneous transactions
      for (QVector<RegisterItem*>::size_type i = 0; i < d->m_items.size(); ++i) {
        RegisterItem* item = d->m_items[i];
        if (!item)
          continue;
        item->setStartRow(rowCount);
        item->setNeedResize();
        rowCount += item->numRowsRegister();

        if (item->isErroneous()) {
          if (!d->m_firstErroneous)
            d->m_firstErroneous = item;
          d->m_lastErroneous = item;
        }
      }

      updateAlternate();

      // create item index
      setupItemIndex(rowCount);

      bool needUpdateHeaders = (QTableWidget::rowCount() != rowCount) | forceUpdateRowHeight;

      // setup QTable.  Make sure to suppress screen updates for now
      setRowCount(rowCount);

      // if we need to update the headers, we do it now for all rows
      // again we make sure to suppress screen updates
      if (needUpdateHeaders) {
        for (auto i = 0; i < rowCount; ++i) {
          RegisterItem* item = itemAtRow(i);
          if (item->isVisible()) {
            showRow(i);
          } else {
            hideRow(i);
          }
          verticalHeader()->resizeSection(i, item->rowHeightHint());
        }
        verticalHeader()->setUpdatesEnabled(true);
      }

      // force resizeing of the columns if necessary
      if (d->m_needInitialColumnResize) {
        QTimer::singleShot(0, this, SLOT(resize()));
        d->m_needInitialColumnResize = false;
      } else {
        update();

        // if the number of rows changed, we might need to resize the register
        // to make sure we reflect the current visibility of the scrollbars.
        if (needUpdateHeaders)
          QTimer::singleShot(0, this, SLOT(resize()));
      }
    }
  }

  int Register::rowHeightHint() const
  {
    Q_D(const Register);
    if (!d->m_rowHeightHint) {
      qDebug("Register::rowHeightHint(): m_rowHeightHint is zero!!");
    }
    return d->m_rowHeightHint;
  }

  void Register::focusInEvent(QFocusEvent* ev)
  {
    Q_D(const Register);
    QTableWidget::focusInEvent(ev);
    if (d->m_focusItem) {
      d->m_focusItem->setFocus(true, false);
    }
  }

  bool Register::event(QEvent* event)
  {
    if (event->type() == QEvent::ToolTip) {
      QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);

      // get the row, if it's the header, then we're done
      // otherwise, adjust the row to be 0 based.
      int row = rowAt(helpEvent->y());
      if (!row)
        return true;
      --row;

      int col = columnAt(helpEvent->x());
      RegisterItem* item = itemAtRow(row);
      if (!item)
        return true;

      row = row - item->startRow();

      QString msg;
      QRect rect;
      if (!item->maybeTip(helpEvent->pos(), row, col, rect, msg))
        return true;

      if (!msg.isEmpty()) {
        QToolTip::showText(helpEvent->globalPos(), msg);
      } else {
        QToolTip::hideText();
        event->ignore();
      }
      return true;
    }
    return TransactionEditorContainer::event(event);
  }

  void Register::focusOutEvent(QFocusEvent* ev)
  {
    Q_D(Register);
    if (d->m_focusItem) {
      d->m_focusItem->setFocus(false, false);
    }
    QTableWidget::focusOutEvent(ev);
  }

  void Register::resizeEvent(QResizeEvent* ev)
  {
    TransactionEditorContainer::resizeEvent(ev);
    resize((int)eTransaction::Column::Detail, true);
  }

  void Register::resize()
  {
    resize((int)eTransaction::Column::Detail);
  }

  void Register::resize(int col, bool force)
  {
    Q_D(Register);
    if (!d->m_needResize && !force)
      return;

    d->m_needResize = false;

    // resize the register
    int w = viewport()->width();

    // TODO I was playing a bit with manual ledger resizing but could not get
    // a good solution. I just leave the code around, so that maybe others
    // pick it up again.  So far, it's not clear to me where to store the
    // size of the sections:
    //
    // a) with the account (as it is done now)
    // b) with the application for the specific account type
    // c) ????
    //
    // Ideas are welcome (ipwizard: 2007-07-19)
    // Note: currently there's no way to switch back to automatic
    // column sizing once the manual sizing option has been saved
#if 0
    if (m_account.value("kmm-ledger-column-width").isEmpty()) {
#endif

      // check which space we need
      if (columnWidth((int)eTransaction::Column::Number))
        adjustColumn((int)eTransaction::Column::Number);
      if (columnWidth((int)eTransaction::Column::Account))
        adjustColumn((int)eTransaction::Column::Account);
      if (columnWidth((int)eTransaction::Column::Payment))
        adjustColumn((int)eTransaction::Column::Payment);
      if (columnWidth((int)eTransaction::Column::Deposit))
        adjustColumn((int)eTransaction::Column::Deposit);
      if (columnWidth((int)eTransaction::Column::Quantity))
        adjustColumn((int)eTransaction::Column::Quantity);
      if (columnWidth((int)eTransaction::Column::Balance))
        adjustColumn((int)eTransaction::Column::Balance);
      if (columnWidth((int)eTransaction::Column::Price))
        adjustColumn((int)eTransaction::Column::Price);
      if (columnWidth((int)eTransaction::Column::Value))
        adjustColumn((int)eTransaction::Column::Value);

      // make amount columns all the same size
      // only extend the entry columns to make sure they fit
      // the widget
      int dwidth = 0;
      int ewidth = 0;
      if (ewidth < columnWidth((int)eTransaction::Column::Payment))
        ewidth = columnWidth((int)eTransaction::Column::Payment);
      if (ewidth < columnWidth((int)eTransaction::Column::Deposit))
        ewidth = columnWidth((int)eTransaction::Column::Deposit);
      if (ewidth < columnWidth((int)eTransaction::Column::Quantity))
        ewidth = columnWidth((int)eTransaction::Column::Quantity);
      if (dwidth < columnWidth((int)eTransaction::Column::Balance))
        dwidth = columnWidth((int)eTransaction::Column::Balance);
      if (ewidth < columnWidth((int)eTransaction::Column::Price))
        ewidth = columnWidth((int)eTransaction::Column::Price);
      if (dwidth < columnWidth((int)eTransaction::Column::Value))
        dwidth = columnWidth((int)eTransaction::Column::Value);
      int swidth = columnWidth((int)eTransaction::Column::Security);
      if (swidth > 0) {
        adjustColumn((int)eTransaction::Column::Security);
        swidth = columnWidth((int)eTransaction::Column::Security);
      }

      adjustColumn((int)eTransaction::Column::Date);

#ifndef KMM_DESIGNER
      // Resize the date and money fields to either
      // a) the size required by the input widget if no transaction form is shown and the register is used with an editor
      // b) the adjusted value for the input widget if the transaction form is visible or an editor is not used
      if (d->m_usedWithEditor && !KMyMoneySettings::transactionForm()) {
        QPushButton *pushButton = new QPushButton;
        const int pushButtonSpacing = pushButton->sizeHint().width() + 5;
        setColumnWidth((int)eTransaction::Column::Date, columnWidth((int)eTransaction::Column::Date) + pushButtonSpacing + 4/* space for the spinbox arrows */);
        ewidth += pushButtonSpacing;

        if (swidth > 0) {
          // extend the security width to make space for the selector arrow
          swidth = columnWidth((int)eTransaction::Column::Security) + 40;
        }
        delete pushButton;
      }
#endif

      if (columnWidth((int)eTransaction::Column::Payment))
        setColumnWidth((int)eTransaction::Column::Payment, ewidth);
      if (columnWidth((int)eTransaction::Column::Deposit))
        setColumnWidth((int)eTransaction::Column::Deposit, ewidth);
      if (columnWidth((int)eTransaction::Column::Quantity))
        setColumnWidth((int)eTransaction::Column::Quantity, ewidth);
      if (columnWidth((int)eTransaction::Column::Balance))
        setColumnWidth((int)eTransaction::Column::Balance, dwidth);
      if (columnWidth((int)eTransaction::Column::Price))
        setColumnWidth((int)eTransaction::Column::Price, ewidth);
      if (columnWidth((int)eTransaction::Column::Value))
        setColumnWidth((int)eTransaction::Column::Value, dwidth);

      if (columnWidth((int)eTransaction::Column::ReconcileFlag))
        setColumnWidth((int)eTransaction::Column::ReconcileFlag, 20);

      if (swidth > 0)
        setColumnWidth((int)eTransaction::Column::Security, swidth);
#if 0
      // see comment above
    } else {
      QStringList colSizes = QStringList::split(",", m_account.value("kmm-ledger-column-width"), true);
      for (int i; i < colSizes.count(); ++i) {
        int colWidth = colSizes[i].toInt();
        if (colWidth == 0)
          continue;
        setColumnWidth(i, w * colWidth / 100);
      }
    }
#endif

    for (auto i = 0; i < columnCount(); ++i) {
      if (i == col)
        continue;

      w -= columnWidth(i);
    }
    setColumnWidth(col, w);
  }

  void Register::forceUpdateLists()
  {
    Q_D(Register);
    d->m_listsDirty = true;
  }

  int Register::minimumColumnWidth(int col)
  {
    Q_D(Register);
    QHeaderView *topHeader = horizontalHeader();
    int w = topHeader->fontMetrics().width(horizontalHeaderItem(col) ? horizontalHeaderItem(col)->text() : QString()) + 10;
    w = qMax(w, 20);
#ifdef KMM_DESIGNER
    return w;
#else
    int maxWidth = 0;
    int minWidth = 0;
    QFontMetrics cellFontMetrics(KMyMoneySettings::listCellFontEx());
    switch (col) {
      case (int)eTransaction::Column::Date:
        minWidth = cellFontMetrics.width(QLocale().toString(QDate(6999, 12, 29), QLocale::ShortFormat) + "  ");
        break;
      default:
        break;
    }

    // scan through the transactions
    for (auto i = 0; i < d->m_items.size(); ++i) {
      RegisterItem* const item = d->m_items[i];
      if (!item)
        continue;
      auto t = dynamic_cast<Transaction*>(item);
      if (t) {
        int nw = 0;
        try {
          nw = t->registerColWidth(col, cellFontMetrics);
        } catch (const MyMoneyException &) {
          // This should only be reached if the data in the file disappeared
          // from under us, such as when the account was deleted from a
          // different view, then this view is restored. In this case, new
          // data is about to be loaded into the view anyway, so just remove
          // the item from the register and swallow the exception.
          //qDebug("%s", e.what());
          removeItem(t);
        }
        w = qMax(w, nw);
        if (maxWidth) {
          if (w > maxWidth) {
            w = maxWidth;
            break;
          }
        }
        if (w < minWidth) {
          w = minWidth;
          break;
        }
      }
    }

    return w;
#endif
  }

  void Register::adjustColumn(int col)
  {
    setColumnWidth(col, minimumColumnWidth(col));
  }

  void Register::clearSelection()
  {
    unselectItems();
    TransactionEditorContainer::clearSelection();
  }

  void Register::doSelectItems(int from, int to, bool selected)
  {
    Q_D(Register);
    int start, end;
    // make sure start is smaller than end
    if (from <= to) {
      start = from;
      end = to;
    } else {
      start = to;
      end = from;
    }
    // make sure we stay in bounds
    if (start < 0)
      start = 0;
    if ((end <= -1) || (end > (d->m_items.size() - 1)))
      end = d->m_items.size() - 1;

    RegisterItem* firstItem;
    RegisterItem* lastItem;
    firstItem = lastItem = 0;
    for (int i = start; i <= end; ++i) {
      RegisterItem* const item = d->m_items[i];
      if (item) {
        if (selected != item->isSelected()) {
          if (!firstItem)
            firstItem = item;
          item->setSelected(selected);
          lastItem = item;
        }
      }
    }
  }

  RegisterItem* Register::itemAtRow(int row) const
  {
    Q_D(const Register);
    if (row >= 0 && row < d->m_itemIndex.size()) {
      return d->m_itemIndex[row];
    }
    return 0;
  }

  int Register::rowToIndex(int row) const
  {
    Q_D(const Register);
    for (auto i = 0; i < d->m_items.size(); ++i) {
      RegisterItem* const item = d->m_items[i];
      if (!item)
        continue;
      if (row >= item->startRow() && row < (item->startRow() + item->numRowsRegister()))
        return i;
    }
    return -1;
  }

  void Register::selectedTransactions(SelectedTransactions& list) const
  {
    Q_D(const Register);
    if (d->m_focusItem && d->m_focusItem->isSelected() && d->m_focusItem->isVisible()) {
      auto t = dynamic_cast<Transaction*>(d->m_focusItem);
      if (t) {
        QString id;
        if (t->isScheduled())
          id = t->transaction().id();
        SelectedTransaction s(t->transaction(), t->split(), id);
        list << s;
      }
    }

    for (auto i = 0; i < d->m_items.size(); ++i) {
      RegisterItem* const item = d->m_items[i];
      // make sure, we don't include the focus item twice
      if (item == d->m_focusItem)
        continue;
      if (item && item->isSelected() && item->isVisible()) {
        auto t = dynamic_cast<Transaction*>(item);
        if (t) {
          QString id;
          if (t->isScheduled())
            id = t->transaction().id();
          SelectedTransaction s(t->transaction(), t->split(), id);
          list << s;
        }
      }
    }
  }

  QList<RegisterItem*> Register::selectedItems() const
  {
    Q_D(const Register);
    QList<RegisterItem*> list;

    RegisterItem* item = d->m_firstItem;
    while (item) {
      if (item && item->isSelected() && item->isVisible()) {
        list << item;
      }
      item = item->nextItem();
    }
    return list;
  }

  int Register::selectedItemsCount() const
  {
    Q_D(const Register);
    auto cnt = 0;
    RegisterItem* item = d->m_firstItem;
    while (item) {
      if (item->isSelected() && item->isVisible())
        ++cnt;
      item = item->nextItem();
    }
    return cnt;
  }

  void Register::mouseReleaseEvent(QMouseEvent *e)
  {
    Q_D(Register);
    if (e->button() == Qt::RightButton) {
      // see the comment in Register::contextMenuEvent
      // on Linux we never get here but on Windows this
      // event is fired before the contextMenuEvent which
      // causes the loss of the multiple selection; to avoid
      // this just ignore the event and act like on Linux
      return;
    }
    if (d->m_ignoreNextButtonRelease) {
      d->m_ignoreNextButtonRelease = false;
      return;
    }
    d->m_mouseButton = e->button();
    d->m_modifiers = QApplication::keyboardModifiers();
    QTableWidget::mouseReleaseEvent(e);
  }

  void Register::contextMenuEvent(QContextMenuEvent *e)
  {
    Q_D(Register);
    if (e->reason() == QContextMenuEvent::Mouse) {
      // since mouse release event is not called, we need
      // to reset the mouse button and the modifiers here
      d->m_mouseButton = Qt::NoButton;
      d->m_modifiers = Qt::NoModifier;

      // if a selected item is clicked don't change the selection
      RegisterItem* item = itemAtRow(rowAt(e->y()));
      if (item && !item->isSelected())
        selectItem(rowAt(e->y()), columnAt(e->x()));
    }
    openContextMenu();
  }

  void Register::unselectItems(int from, int to)
  {
    doSelectItems(from, to, false);
  }

  void Register::selectItems(int from, int to)
  {
    doSelectItems(from, to, true);
  }

  void Register::selectItem(int row, int col)
  {
    Q_D(Register);
    if (row >= 0 && row < d->m_itemIndex.size()) {
      RegisterItem* item = d->m_itemIndex[row];

      // don't support selecting when the item has an editor
      // or the item itself is not selectable
      if (item->hasEditorOpen() || !item->isSelectable()) {
        d->m_mouseButton = Qt::NoButton;
        return;
      }
      QString id = item->id();
      selectItem(item);
      // selectItem() might have changed the pointers, so we
      // need to reconstruct it here
      item = itemById(id);
      auto t = dynamic_cast<Transaction*>(item);
      if (t) {
        if (!id.isEmpty()) {
          if (t && col == (int)eTransaction::Column::ReconcileFlag && selectedItemsCount() == 1 && !t->isScheduled())
            emit reconcileStateColumnClicked(t);
        } else {
          emit emptyItemSelected();
        }
      }
    }
  }

  void Register::setAnchorItem(RegisterItem* anchorItem)
  {
    Q_D(Register);
    d->m_selectAnchor = anchorItem;
  }

  bool Register::setFocusItem(RegisterItem* focusItem)
  {
    Q_D(Register);
    if (focusItem && focusItem->canHaveFocus()) {
      if (d->m_focusItem) {
        d->m_focusItem->setFocus(false);
      }
      auto item = dynamic_cast<Transaction*>(focusItem);
      if (d->m_focusItem != focusItem && item) {
        emit focusChanged(item);
      }

      d->m_focusItem = focusItem;
      d->m_focusItem->setFocus(true);
      if (d->m_listsDirty)
        updateRegister(KMyMoneySettings::ledgerLens() | !KMyMoneySettings::transactionForm());
      ensureItemVisible(d->m_focusItem);
      return true;
    } else
      return false;
  }

  bool Register::setFocusToTop()
  {
    Q_D(Register);
    RegisterItem* rgItem = d->m_firstItem;
    while (rgItem) {
      if (setFocusItem(rgItem))
        return true;
      rgItem = rgItem->nextItem();
    }
    return false;
  }

  void Register::selectItem(RegisterItem* item, bool dontChangeSelections)
  {
    Q_D(Register);
    if (!item)
      return;

    Qt::MouseButtons buttonState = d->m_mouseButton;
    Qt::KeyboardModifiers modifiers = d->m_modifiers;
    d->m_mouseButton = Qt::NoButton;
    d->m_modifiers = Qt::NoModifier;

    if (d->m_selectionMode == NoSelection)
      return;

    if (item->isSelectable()) {
      QString id = item->id();
      QList<RegisterItem*> itemList = selectedItems();
      bool okToSelect = true;
      auto cnt = itemList.count();
      auto scheduledTransactionSelected = false;
      if (cnt > 0) {
        auto& r = *(itemList.front());
        scheduledTransactionSelected = (typeid(r) == typeid(StdTransactionScheduled));
      }
      if (buttonState & Qt::LeftButton) {
        if (!(modifiers & (Qt::ShiftModifier | Qt::ControlModifier))
            || (d->m_selectAnchor == 0)) {
          if ((cnt != 1) || ((cnt == 1) && !item->isSelected())) {
            emit aboutToSelectItem(item, okToSelect);
            if (okToSelect) {
              // pointer 'item' might have changed. reconstruct it.
              item = itemById(id);
              unselectItems();
              item->setSelected(true);
              setFocusItem(item);
            }
          }
          if (okToSelect)
            d->m_selectAnchor = item;
        }

        if (d->m_selectionMode == MultiSelection) {
          switch (modifiers & (Qt::ShiftModifier | Qt::ControlModifier)) {
            case Qt::ControlModifier:
              if (scheduledTransactionSelected || typeid(*item) == typeid(StdTransactionScheduled))
                okToSelect = false;
              // toggle selection state of current item
              emit aboutToSelectItem(item, okToSelect);
              if (okToSelect) {
                // pointer 'item' might have changed. reconstruct it.
                item = itemById(id);
                item->setSelected(!item->isSelected());
                setFocusItem(item);
              }
              break;

            case Qt::ShiftModifier:
              if (scheduledTransactionSelected || typeid(*item) == typeid(StdTransactionScheduled))
                okToSelect = false;
              emit aboutToSelectItem(item, okToSelect);
              if (okToSelect) {
                // pointer 'item' might have changed. reconstruct it.
                item = itemById(id);
                unselectItems();
                if (d->m_selectAnchor)
                  selectItems(rowToIndex(d->m_selectAnchor->startRow()), rowToIndex(item->startRow()));
                setFocusItem(item);
              }
              break;
          }
        }
      } else {
        // we get here when called by application logic
        emit aboutToSelectItem(item, okToSelect);
        if (okToSelect) {
          // pointer 'item' might have changed. reconstruct it.
          item = itemById(id);
          if (!dontChangeSelections)
            unselectItems();
          item->setSelected(true);
          setFocusItem(item);
          d->m_selectAnchor = item;
        }
      }
      if (okToSelect) {
        SelectedTransactions list(this);
        emit transactionsSelected(list);
      }
    }
  }

  void Register::ensureFocusItemVisible()
  {
    Q_D(Register);
    ensureItemVisible(d->m_focusItem);
  }

  void Register::ensureItemVisible(RegisterItem* item)
  {
    Q_D(Register);
    if (!item)
      return;

    d->m_ensureVisibleItem = item;
    QTimer::singleShot(0, this, SLOT(slotEnsureItemVisible()));
  }

  void Register::slotDoubleClicked(int row, int)
  {
    Q_D(Register);
    if (row >= 0 && row < d->m_itemIndex.size()) {
      RegisterItem* p = d->m_itemIndex[row];
      if (p->isSelectable()) {
        d->m_ignoreNextButtonRelease = true;
        // double click to start editing only works if the focus
        // item is among the selected ones
        if (!focusItem()) {
          setFocusItem(p);
          if (d->m_selectionMode != NoSelection)
            p->setSelected(true);
        }

        if (d->m_focusItem->isSelected()) {
          // don't emit the signal right away but wait until
          // we come back to the Qt main loop
          QTimer::singleShot(0, this, SIGNAL(editTransaction()));
        }
      }
    }
  }

  void Register::slotEnsureItemVisible()
  {
    Q_D(Register);
    // if clear() has been called since the timer was
    // started, we just ignore the call
    if (!d->m_ensureVisibleItem)
      return;

    // make sure to catch latest changes
    setUpdatesEnabled(false);
    updateRegister();
    setUpdatesEnabled(true);
    // since the item will be made visible at the top of the viewport make the bottom index visible first to make the whole item visible
    scrollTo(model()->index(d->m_ensureVisibleItem->startRow() + d->m_ensureVisibleItem->numRowsRegister() - 1, (int)eTransaction::Column::Detail));
    scrollTo(model()->index(d->m_ensureVisibleItem->startRow(), (int)eTransaction::Column::Detail));
  }

  QString Register::text(int /*row*/, int /*col*/) const
  {
    return QString("a");
  }

  QWidget* Register::createEditor(int /*row*/, int /*col*/, bool /*initFromCell*/) const
  {
    return 0;
  }

  void Register::setCellContentFromEditor(int /*row*/, int /*col*/)
  {
  }

  void Register::endEdit(int /*row*/, int /*col*/, bool /*accept*/, bool /*replace*/)
  {
  }

  RegisterItem* Register::focusItem() const
  {
    Q_D(const Register);
    return d->m_focusItem;
  }

  RegisterItem* Register::anchorItem() const
  {
    Q_D(const Register);
    return d->m_selectAnchor;
  }

  void Register::arrangeEditWidgets(QMap<QString, QWidget*>& editWidgets, KMyMoneyRegister::Transaction* t)
  {
    t->arrangeWidgetsInRegister(editWidgets);
    ensureItemVisible(t);
    // updateContents();
  }

  void Register::tabOrder(QWidgetList& tabOrderWidgets, KMyMoneyRegister::Transaction* t) const
  {
    t->tabOrderInRegister(tabOrderWidgets);
  }

  void Register::removeEditWidgets(QMap<QString, QWidget*>& editWidgets)
  {
    // remove pointers from map
    QMap<QString, QWidget*>::iterator it;
    for (it = editWidgets.begin(); it != editWidgets.end();) {
      if ((*it)->parentWidget() == this) {
        editWidgets.erase(it);
        it = editWidgets.begin();
      } else
        ++it;
    }

    // now delete the widgets
    if (auto t = dynamic_cast<KMyMoneyRegister::Transaction*>(focusItem())) {
      for (int row = t->startRow(); row < t->startRow() + t->numRowsRegister(true); ++row) {
        for (int col = 0; col < columnCount(); ++col) {
          if (cellWidget(row, col)) {
            cellWidget(row, col)->hide();
            setCellWidget(row, col, 0);
          }
        }
        // make sure to reduce the possibly size to what it was before editing started
        setRowHeight(row, t->rowHeightHint());
      }
    }
  }

  RegisterItem* Register::itemById(const QString& id) const
  {
    Q_D(const Register);
    if (id.isEmpty())
      return d->m_lastItem;

    for (QVector<RegisterItem*>::size_type i = 0; i < d->m_items.size(); ++i) {
      RegisterItem* item = d->m_items[i];
      if (!item)
        continue;
      if (item->id() == id)
        return item;
    }
    return 0;
  }

  void Register::handleItemChange(RegisterItem* old, bool shift, bool control)
  {
    Q_D(Register);
    if (d->m_selectionMode == MultiSelection) {
      if (shift) {
        selectRange(d->m_selectAnchor ? d->m_selectAnchor : old,
                    d->m_focusItem, false, true, (d->m_selectAnchor && !control) ? true : false);
      } else if (!control) {
        selectItem(d->m_focusItem, false);
      }
    }
  }

  void Register::selectRange(RegisterItem* from, RegisterItem* to, bool invert, bool includeFirst, bool clearSel)
  {
    if (!from || !to)
      return;
    if (from == to && !includeFirst)
      return;
    bool swap = false;
    if (to == from->prevItem())
      swap = true;

    RegisterItem* item;
    if (!swap && from != to && from != to->prevItem()) {
      bool found = false;
      for (item = from; item; item = item->nextItem()) {
        if (item == to) {
          found = true;
          break;
        }
      }
      if (!found)
        swap = true;
    }

    if (swap) {
      item = from;
      from = to;
      to = item;
      if (!includeFirst)
        to = to->prevItem();

    } else if (!includeFirst) {
      from = from->nextItem();
    }

    if (clearSel) {
      for (item = firstItem(); item; item = item->nextItem()) {
        if (item->isSelected() && item->isVisible()) {
          item->setSelected(false);
        }
      }
    }

    for (item = from; item; item = item->nextItem()) {
      if (item->isSelectable()) {
        if (!invert) {
          if (!item->isSelected() && item->isVisible()) {
            item->setSelected(true);
          }
        } else {
          bool sel = !item->isSelected();
          if ((item->isSelected() != sel) && item->isVisible()) {
            item->setSelected(sel);
          }
        }
      }
      if (item == to)
        break;
    }
  }

  void Register::scrollPage(int key, Qt::KeyboardModifiers modifiers)
  {
    Q_D(Register);
    RegisterItem* oldFocusItem = d->m_focusItem;

    // make sure we have a focus item
    if (!d->m_focusItem)
      setFocusItem(d->m_firstItem);
    if (!d->m_focusItem && d->m_firstItem)
      setFocusItem(d->m_firstItem->nextItem());
    if (!d->m_focusItem)
      return;

    RegisterItem* item = d->m_focusItem;
    int height = 0;

    switch (key) {
      case Qt::Key_PageUp:
        while (height < viewport()->height() && item->prevItem()) {
          do {
            item = item->prevItem();
            if (item->isVisible())
              height += item->rowHeightHint();
          } while ((!item->isSelectable() || !item->isVisible()) && item->prevItem());
          while ((!item->isSelectable() || !item->isVisible()) && item->nextItem())
            item = item->nextItem();
        }
        break;
      case Qt::Key_PageDown:
        while (height < viewport()->height() && item->nextItem()) {
          do {
            if (item->isVisible())
              height += item->rowHeightHint();
            item = item->nextItem();
          } while ((!item->isSelectable() || !item->isVisible()) && item->nextItem());
          while ((!item->isSelectable() || !item->isVisible()) && item->prevItem())
            item = item->prevItem();
        }
        break;

      case Qt::Key_Up:
        if (item->prevItem()) {
          do {
            item = item->prevItem();
          } while ((!item->isSelectable() || !item->isVisible()) && item->prevItem());
        }
        break;

      case Qt::Key_Down:
        if (item->nextItem()) {
          do {
            item = item->nextItem();
          } while ((!item->isSelectable() || !item->isVisible()) && item->nextItem());
        }
        break;

      case Qt::Key_Home:
        item = d->m_firstItem;
        while ((!item->isSelectable() || !item->isVisible()) && item->nextItem())
          item = item->nextItem();
        break;

      case Qt::Key_End:
        item = d->m_lastItem;
        while ((!item->isSelectable() || !item->isVisible()) && item->prevItem())
          item = item->prevItem();
        break;
    }

    // make sure to avoid selecting a possible empty transaction at the end
    auto t = dynamic_cast<Transaction*>(item);
    if (t && t->transaction().id().isEmpty()) {
      if (t->prevItem()) {
        item = t->prevItem();
      }
    }

    if (!(modifiers & Qt::ShiftModifier) || !d->m_selectAnchor)
      d->m_selectAnchor = item;

    setFocusItem(item);

    if (item->isSelectable()) {
      handleItemChange(oldFocusItem, modifiers & Qt::ShiftModifier, modifiers & Qt::ControlModifier);
      // tell the world about the changes in selection
      SelectedTransactions list(this);
      emit transactionsSelected(list);
    }

    if (d->m_focusItem && !d->m_focusItem->isSelected() && d->m_selectionMode == SingleSelection)
      selectItem(item);

  }

  void Register::keyPressEvent(QKeyEvent* ev)
  {
    Q_D(Register);
    switch (ev->key()) {
      case Qt::Key_Space:
        if (d->m_selectionMode != NoSelection) {
          // get the state out of the event ...
          d->m_modifiers = ev->modifiers();
          // ... and pretend that we have pressed the left mouse button ;)
          d->m_mouseButton = Qt::LeftButton;
          selectItem(d->m_focusItem);
        }
        break;

      case Qt::Key_PageUp:
      case Qt::Key_PageDown:
      case Qt::Key_Home:
      case Qt::Key_End:
      case Qt::Key_Down:
      case Qt::Key_Up:
        scrollPage(ev->key(), ev->modifiers());
        break;
      case Qt::Key_Enter:
      case Qt::Key_Return:
        // don't emit the signal right away but wait until
        // we come back to the Qt main loop
        QTimer::singleShot(0, this, SIGNAL(editTransaction()));
        break;

      default:
        QTableWidget::keyPressEvent(ev);
        break;
    }
  }

  Transaction* Register::transactionFactory(Register *parent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId)
  {
    Transaction* t = 0;
    MyMoneySplit s = split;

    if (parent->account() == MyMoneyAccount()) {
      t = new KMyMoneyRegister::StdTransaction(parent, transaction, s, uniqueId);
      return t;
    }

    switch (parent->account().accountType()) {
      case Account::Type::Checkings:
      case Account::Type::Savings:
      case Account::Type::Cash:
      case Account::Type::CreditCard:
      case Account::Type::Loan:
      case Account::Type::Asset:
      case Account::Type::Liability:
      case Account::Type::Currency:
      case Account::Type::Income:
      case Account::Type::Expense:
      case Account::Type::AssetLoan:
      case Account::Type::Equity:
        if (s.accountId().isEmpty())
          s.setAccountId(parent->account().id());
        if (s.isMatched())
          t = new KMyMoneyRegister::StdTransactionMatched(parent, transaction, s, uniqueId);
        else if (transaction.isImported())
          t = new KMyMoneyRegister::StdTransactionDownloaded(parent, transaction, s, uniqueId);
        else
          t = new KMyMoneyRegister::StdTransaction(parent, transaction, s, uniqueId);
        break;

      case Account::Type::Investment:
        if (s.isMatched())
          t = new KMyMoneyRegister::InvestTransaction/* Matched */(parent, transaction, s, uniqueId);
        else if (transaction.isImported())
          t = new KMyMoneyRegister::InvestTransactionDownloaded(parent, transaction, s, uniqueId);
        else
          t = new KMyMoneyRegister::InvestTransaction(parent, transaction, s, uniqueId);
        break;

      case Account::Type::CertificateDep:
      case Account::Type::MoneyMarket:
      case Account::Type::Stock:
      default:
        qDebug("Register::transactionFactory: invalid accountTypeE %d", (int)parent->account().accountType());
        break;
    }
    return t;
  }

  const MyMoneyAccount& Register::account() const
  {
    Q_D(const Register);
    return d->m_account;
  }

  void Register::addGroupMarkers()
  {
    Q_D(Register);
    QMap<QString, int> list;
    QMap<QString, int>::const_iterator it;
    KMyMoneyRegister::RegisterItem* p = firstItem();
    KMyMoneyRegister::Transaction* t;
    QString name;
    QDate today;
    QDate yesterday, thisWeek, lastWeek;
    QDate thisMonth, lastMonth;
    QDate thisYear;
    int weekStartOfs;

    switch (primarySortKey()) {
      case SortField::PostDate:
      case SortField::EntryDate:
        today = QDate::currentDate();
        thisMonth.setDate(today.year(), today.month(), 1);
        lastMonth = thisMonth.addMonths(-1);
        yesterday = today.addDays(-1);
        // a = QDate::dayOfWeek()         todays weekday (1 = Monday, 7 = Sunday)
        // b = QLocale().firstDayOfWeek() first day of week (1 = Monday, 7 = Sunday)
        weekStartOfs = today.dayOfWeek() - QLocale().firstDayOfWeek();
        if (weekStartOfs < 0) {
          weekStartOfs = 7 + weekStartOfs;
        }
        thisWeek = today.addDays(-weekStartOfs);
        lastWeek = thisWeek.addDays(-7);
        thisYear.setDate(today.year(), 1, 1);
        if (KMyMoneySettings::startDate().date() != QDate(1900, 1, 1))
          new KMyMoneyRegister::FancyDateGroupMarker(this, KMyMoneySettings::startDate().date(), i18n("Prior transactions possibly filtered"));

        if (KMyMoneySettings::showFancyMarker()) {
          if (d->m_account.lastReconciliationDate().isValid())
            new KMyMoneyRegister::StatementGroupMarker(this, eRegister::CashFlowDirection::Deposit, d->m_account.lastReconciliationDate(), i18n("Last reconciliation"));

          if (!d->m_account.value("lastImportedTransactionDate").isEmpty()
              && !d->m_account.value("lastStatementBalance").isEmpty()) {
            MyMoneyMoney balance(d->m_account.value("lastStatementBalance"));
            if (d->m_account.accountGroup() == Account::Type::Liability)
              balance = -balance;
            auto txt = i18n("Online Statement Balance: %1", balance.formatMoney(d->m_account.fraction()));

            KMyMoneyRegister::StatementGroupMarker *pGroupMarker = new KMyMoneyRegister::StatementGroupMarker(this, eRegister::CashFlowDirection::Deposit, QDate::fromString(d->m_account.value("lastImportedTransactionDate"), Qt::ISODate), txt);

            pGroupMarker->setErroneous(!MyMoneyFile::instance()->hasMatchingOnlineBalance(d->m_account));
          }

          new KMyMoneyRegister::FancyDateGroupMarker(this, thisYear, i18n("This year"));
          new KMyMoneyRegister::FancyDateGroupMarker(this, lastMonth, i18n("Last month"));
          new KMyMoneyRegister::FancyDateGroupMarker(this, thisMonth, i18n("This month"));
          new KMyMoneyRegister::FancyDateGroupMarker(this, lastWeek, i18n("Last week"));
          new KMyMoneyRegister::FancyDateGroupMarker(this, thisWeek, i18n("This week"));
          new KMyMoneyRegister::FancyDateGroupMarker(this, yesterday, i18n("Yesterday"));
          new KMyMoneyRegister::FancyDateGroupMarker(this, today, i18n("Today"));
          new KMyMoneyRegister::FancyDateGroupMarker(this, today.addDays(1), i18n("Future transactions"));
          new KMyMoneyRegister::FancyDateGroupMarker(this, thisWeek.addDays(7), i18n("Next week"));
          new KMyMoneyRegister::FancyDateGroupMarker(this, thisMonth.addMonths(1), i18n("Next month"));

        } else {
          new KMyMoneyRegister::SimpleDateGroupMarker(this, today.addDays(1), i18n("Future transactions"));
        }
        if (KMyMoneySettings::showFiscalMarker()) {
          QDate currentFiscalYear = KMyMoneySettings::firstFiscalDate();
          new KMyMoneyRegister::FiscalYearGroupMarker(this, currentFiscalYear, i18n("Current fiscal year"));
          new KMyMoneyRegister::FiscalYearGroupMarker(this, currentFiscalYear.addYears(-1), i18n("Previous fiscal year"));
          new KMyMoneyRegister::FiscalYearGroupMarker(this, currentFiscalYear.addYears(1), i18n("Next fiscal year"));
        }
        break;

      case SortField::Type:
        if (KMyMoneySettings::showFancyMarker()) {
          new KMyMoneyRegister::TypeGroupMarker(this, eRegister::CashFlowDirection::Deposit, d->m_account.accountType());
          new KMyMoneyRegister::TypeGroupMarker(this, eRegister::CashFlowDirection::Payment, d->m_account.accountType());
        }
        break;

      case SortField::ReconcileState:
        if (KMyMoneySettings::showFancyMarker()) {
          new KMyMoneyRegister::ReconcileGroupMarker(this, eMyMoney::Split::State::NotReconciled);
          new KMyMoneyRegister::ReconcileGroupMarker(this, eMyMoney::Split::State::Cleared);
          new KMyMoneyRegister::ReconcileGroupMarker(this, eMyMoney::Split::State::Reconciled);
          new KMyMoneyRegister::ReconcileGroupMarker(this, eMyMoney::Split::State::Frozen);
        }
        break;

      case SortField::Payee:
        if (KMyMoneySettings::showFancyMarker()) {
          while (p) {
            if ((t = dynamic_cast<KMyMoneyRegister::Transaction*>(p)))
              list[t->sortPayee()] = 1;
            p = p->nextItem();
          }
          for (it = list.constBegin(); it != list.constEnd(); ++it) {
            name = it.key();
            if (name.isEmpty()) {
              name = i18nc("Unknown payee", "Unknown");
            }
            new KMyMoneyRegister::PayeeGroupMarker(this, name);
          }
        }
        break;

      case SortField::Category:
        if (KMyMoneySettings::showFancyMarker()) {
          while (p) {
            if ((t = dynamic_cast<KMyMoneyRegister::Transaction*>(p)))
              list[t->sortCategory()] = 1;
            p = p->nextItem();
          }
          for (it = list.constBegin(); it != list.constEnd(); ++it) {
            name = it.key();
            if (name.isEmpty()) {
              name = i18nc("Unknown category", "Unknown");
            }
            new KMyMoneyRegister::CategoryGroupMarker(this, name);
          }
        }
        break;

      case SortField::Security:
        if (KMyMoneySettings::showFancyMarker()) {
          while (p) {
            if ((t = dynamic_cast<KMyMoneyRegister::InvestTransaction*>(p)))
              list[t->sortSecurity()] = 1;
            p = p->nextItem();
          }
          for (it = list.constBegin(); it != list.constEnd(); ++it) {
            name = it.key();
            if (name.isEmpty()) {
              name = i18nc("Unknown security", "Unknown");
            }
            new KMyMoneyRegister::CategoryGroupMarker(this, name);
          }
        }
        break;

      default: // no markers supported
        break;
    }
  }

  void Register::removeUnwantedGroupMarkers()
  {
    // remove all trailing group markers except statement markers
    KMyMoneyRegister::RegisterItem* q;
    KMyMoneyRegister::RegisterItem* p = lastItem();
    while (p) {
      q = p;
      if (dynamic_cast<KMyMoneyRegister::Transaction*>(p)
          || dynamic_cast<KMyMoneyRegister::StatementGroupMarker*>(p))
        break;

      p = p->prevItem();
      delete q;
    }

    // remove all adjacent group markers
    bool lastWasGroupMarker = false;
    p = lastItem();
    while (p) {
      q = p;
      auto m = dynamic_cast<KMyMoneyRegister::GroupMarker*>(p);
      p = p->prevItem();
      if (m) {
        m->markVisible(true);
        // make adjacent group marker invisible except those that show statement information
        if (lastWasGroupMarker && (dynamic_cast<KMyMoneyRegister::StatementGroupMarker*>(m) == 0)) {
          m->markVisible(false);
        }
        lastWasGroupMarker = true;
      } else if (q->isVisible())
        lastWasGroupMarker = false;
    }
  }

  void Register::setLedgerLensForced(bool forced)
  {
    Q_D(Register);
    d->m_ledgerLensForced = forced;
  }

  bool Register::ledgerLens() const
  {
    Q_D(const Register);
    return d->m_ledgerLensForced;
  }

  void Register::setSelectionMode(SelectionMode mode)
  {
    Q_D(Register);
    d->m_selectionMode = mode;
  }

  void Register::setUsedWithEditor(bool value)
  {
    Q_D(Register);
    d->m_usedWithEditor = value;
  }

  eRegister::DetailColumn Register::getDetailsColumnType() const
  {
    Q_D(const Register);
    return d->m_detailsColumnType;
  }

  void Register::setDetailsColumnType(eRegister::DetailColumn detailsColumnType)
  {
    Q_D(Register);
    d->m_detailsColumnType = detailsColumnType;
  }
}
