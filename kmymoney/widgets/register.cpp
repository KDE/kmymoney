/***************************************************************************
                             register.cpp  -  description
                             -------------------
    begin                : Fri Mar 10 2006
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

#include <config-kmymoney.h>

#include "register.h"

#include <typeinfo>

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QToolTip>
#include <QImage>
#include <QPixmap>
#include <QFocusEvent>
#include <QMouseEvent>
#include <QList>
#include <QKeyEvent>
#include <QEvent>
#include <QFrame>
#include <QDropEvent>
#include <QDragMoveEvent>
#include <QPaintEvent>
#include <QHeaderView>
#include <QStyleOptionViewItem>
#include <QApplication>
#include <QPushButton>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes
#include "kmymoneydateinput.h"
#include "kmymoneyedit.h"
#include "kmymoneycategory.h"
#include "transactionform.h"
#include "stdtransactiondownloaded.h"
#include "stdtransactionmatched.h"
#include "scheduledtransaction.h"
#include "kmymoneyglobalsettings.h"
#include "mymoneyfile.h"

static const char * sortOrderText[] = {
  I18N_NOOP2("Unknown sort order", "Unknown"),
  I18N_NOOP("Post date"),
  I18N_NOOP("Date entered"),
  I18N_NOOP("Payee"),
  I18N_NOOP("Amount"),
  I18N_NOOP("Number"),
  I18N_NOOP("Entry order"),
  I18N_NOOP("Type"),
  I18N_NOOP("Category"),
  I18N_NOOP("Reconcile state"),
  I18N_NOOP("Security")
  // add new values above this comment line
};

using namespace KMyMoneyRegister;

static unsigned char fancymarker_bg_image[] = {
  /* 200x49 */
  0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A,
  0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52,
  0x00, 0x00, 0x00, 0xC8, 0x00, 0x00, 0x00, 0x31,
  0x08, 0x06, 0x00, 0x00, 0x00, 0x9F, 0xC5, 0xE6,
  0x4F, 0x00, 0x00, 0x00, 0x06, 0x62, 0x4B, 0x47,
  0x44, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0xA0,
  0xBD, 0xA7, 0x93, 0x00, 0x00, 0x00, 0x09, 0x70,
  0x48, 0x59, 0x73, 0x00, 0x00, 0x0B, 0x13, 0x00,
  0x00, 0x0B, 0x13, 0x01, 0x00, 0x9A, 0x9C, 0x18,
  0x00, 0x00, 0x00, 0x86, 0x49, 0x44, 0x41, 0x54,
  0x78, 0xDA, 0xED, 0xDD, 0x31, 0x0A, 0x84, 0x40,
  0x10, 0x44, 0xD1, 0x1A, 0x19, 0x10, 0xCF, 0xE6,
  0xFD, 0x4F, 0xB2, 0x88, 0x08, 0x22, 0x9B, 0x18,
  0x4E, 0x1B, 0x2C, 0x1B, 0x18, 0xBC, 0x07, 0x7D,
  0x81, 0x82, 0x1F, 0x77, 0x4B, 0xB2, 0x06, 0x18,
  0xEA, 0x49, 0x3E, 0x66, 0x00, 0x81, 0x80, 0x40,
  0xE0, 0xDF, 0x81, 0x6C, 0x66, 0x80, 0x3A, 0x90,
  0xDD, 0x0C, 0x50, 0x07, 0x72, 0x98, 0x01, 0xEA,
  0x40, 0x4E, 0x33, 0x40, 0x1D, 0xC8, 0x65, 0x06,
  0x18, 0x6B, 0xF7, 0x01, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x16, 0x3E,
  0x4C, 0xC1, 0x83, 0x9E, 0x64, 0x32, 0x03, 0x08,
  0x04, 0x7E, 0x0A, 0xA4, 0x9B, 0x01, 0xEA, 0x40,
  0x66, 0x33, 0x40, 0x1D, 0xC8, 0x62, 0x06, 0x18,
  0xFB, 0x02, 0x05, 0x87, 0x08, 0x55, 0xFE, 0xDE,
  0xA2, 0x9D, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45,
  0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82
};

QPixmap* GroupMarker::m_bg = 0;
int GroupMarker::m_bgRefCnt = 0;

void ItemPtrVector::sort()
{
  if (count() > 0) {
    // get rid of 0 pointers in the list
    KMyMoneyRegister::ItemPtrVector::iterator it_l;
    RegisterItem *item;
    for (it_l = begin(); it_l != end(); ++it_l) {
      if (*it_l == 0) {
        item = last();
        *it_l = item;
        pop_back();
        --it_l;
      }
    }

    std::sort(begin(), end(), item_cmp);
  }
}

bool ItemPtrVector::item_cmp(RegisterItem* i1, RegisterItem* i2)
{
  const QList<TransactionSortField>& sortOrder = i1->parent()->sortOrder();
  QList<TransactionSortField>::const_iterator it;
  int rc = 0;
  bool ok1, ok2;
  qulonglong n1, n2;

  for (it = sortOrder.begin(); it != sortOrder.end(); ++it) {
    TransactionSortField sortField = static_cast<TransactionSortField>(*it);
    switch (qAbs(static_cast<int>(sortField))) {
      case PostDateSort:
        rc = i2->sortPostDate().daysTo(i1->sortPostDate());
        break;

      case EntryDateSort:
        rc = i2->sortEntryDate().daysTo(i1->sortEntryDate());
        break;

      case PayeeSort:
        rc = QString::localeAwareCompare(i1->sortPayee(), i2->sortPayee());
        break;

      case ValueSort:
        if (i1->sortValue() == i2->sortValue())
          rc = 0;
        else if (i1->sortValue() < i2->sortValue())
          rc = -1;
        else
          rc = 1;
        break;

      case NoSort:
        // convert both values to numbers
        n1 = i1->sortNumber().toULongLong(&ok1);
        n2 = i2->sortNumber().toULongLong(&ok2);
        // the following four cases exist:
        // a) both are converted correct
        //    compare them directly
        // b) n1 is numeric, n2 is not
        //    numbers come first, so return -1
        // c) n1 is not numeric, n2 is
        //    numbers come first, so return 1
        // d) both are non numbers
        //    compare using localeAwareCompare
        if (ok1 && ok2) { // case a)
          rc = (n1 > n2) ? 1 : ((n1 == n2) ? 0 : -1);
        } else if (ok1 && !ok2) {
          rc = -1;
        } else if (!ok1 && ok2) {
          rc = 1;
        } else
          rc = QString::localeAwareCompare(i1->sortNumber(), i2->sortNumber());
        break;

      case EntryOrderSort:
        rc = qstrcmp(i1->sortEntryOrder().toLatin1(), i2->sortEntryOrder().toLatin1());
        break;

      case TypeSort:
        rc = i1->sortType() - i2->sortType();
        break;

      case CategorySort:
        rc = QString::localeAwareCompare(i1->sortCategory(), i2->sortCategory());
        break;

      case ReconcileStateSort:
        rc = static_cast<int>(i1->sortReconcileState()) - static_cast<int>(i2->sortReconcileState());
        break;

      case SecuritySort:
        rc = QString::localeAwareCompare(i1->sortSecurity(), i2->sortSecurity());
        break;

      default:
        qDebug("Invalid sort key %d", *it);
        break;
    }

    // take care of group markers, but only first sort item
    if ((rc == 0) && (it == sortOrder.begin())) {
      rc = i1->sortSamePostDate() - i2->sortSamePostDate();
      if (rc) {
        return rc < 0;
      }
    }

    // the items differ for this sort key so we can return a result
    if (rc != 0) {
      return (*it < 0) ? rc >= 0 : rc < 0;
    }
  }

  if (rc == 0) {
    rc = qstrcmp(i1->sortEntryOrder().toLatin1(), i2->sortEntryOrder().toLatin1());
  }

  return rc < 0;
}

GroupMarker::GroupMarker(Register *parent, const QString& txt) :
    RegisterItem(parent),
    m_txt(txt),
    m_showDate(false),
    m_erroneous(false)
{
  int h;
  if (m_parent) {
    h = m_parent->rowHeightHint();
  } else {
    QFontMetrics fm(KMyMoneyGlobalSettings::listCellFont());
    h = fm.lineSpacing() + 6;
  }

  if (m_bg && (m_bg->height() != h)) {
    delete m_bg;
    m_bg = 0;
  }

  // convert the backgroud once
  if (m_bg == 0) {
    m_bg = new QPixmap;
    m_bg->loadFromData(fancymarker_bg_image, sizeof(fancymarker_bg_image));
    *m_bg = m_bg->scaled(m_bg->width(), h, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
  }

  ++m_bgRefCnt;
}

GroupMarker::~GroupMarker()
{
  --m_bgRefCnt;
  if (!m_bgRefCnt) {
    delete m_bg;
    m_bg = 0;
  }
}

void GroupMarker::paintRegisterCell(QPainter *painter, QStyleOptionViewItem &option, const QModelIndex &index)
{
  QRect r(option.rect);
  painter->save();

  // the group marker always uses all cols
  r.setX(m_parent->horizontalHeader()->sectionPosition(0));
  r.setWidth(m_parent->viewport()->width());
  painter->translate(r.x(), r.y());

  QRect cellRect;
  cellRect.setX(0);
  cellRect.setY(0);
  cellRect.setWidth(m_parent->viewport()->width());
  cellRect.setHeight(m_parent->rowHeight(index.row()));

  option.palette.setColor(QPalette::Base, isErroneous() ? KMyMoneyGlobalSettings::listErroneousTransactionColor() : KMyMoneyGlobalSettings::groupMarkerColor());

  QBrush backgroundBrush(option.palette.color(QPalette::Base));
  painter->fillRect(cellRect, backgroundBrush);
  painter->setPen(KMyMoneyGlobalSettings::listGridColor());
  painter->drawLine(cellRect.x(), cellRect.height() - 1, cellRect.width(), cellRect.height() - 1);

  // now write the text
  painter->setPen(option.palette.color(isErroneous() ? QPalette::HighlightedText : QPalette::Text));
  QFont font = painter->font();
  font.setBold(true);
  painter->setFont(font);

  painter->drawText(cellRect, Qt::AlignVCenter | Qt::AlignCenter, m_txt);

  cellRect.setHeight(m_bg->height());

  // now it's time to draw the background
  painter->drawPixmap(cellRect, *m_bg);

  // in case we need to show the date, we just paint it in col 1
  if (m_showDate) {
    font.setBold(false);
    cellRect.setX(m_parent->horizontalHeader()->sectionPosition(DateColumn));
    cellRect.setWidth(m_parent->horizontalHeader()->sectionSize(DateColumn));
    painter->setFont(font);
    painter->drawText(cellRect, Qt::AlignVCenter | Qt::AlignCenter, QLocale().toString(sortPostDate(), QLocale::ShortFormat));
  }

  painter->restore();
}

void GroupMarker::paintFormCell(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index)
{
  Q_UNUSED(painter);
  Q_UNUSED(option);
  Q_UNUSED(index);
}

int GroupMarker::rowHeightHint() const
{
  if (!m_visible)
    return 0;

  return m_bg->height();
}

StatementGroupMarker::StatementGroupMarker(Register* parent, CashFlowDirection dir, const QDate& date, const QString& txt) :
    FancyDateGroupMarker(parent, date, txt),
    m_dir(dir)
{
  m_showDate = true;
}

FancyDateGroupMarker::FancyDateGroupMarker(Register* parent, const QDate& date, const QString& txt) :
    GroupMarker(parent, txt),
    m_date(date)
{
}

FiscalYearGroupMarker::FiscalYearGroupMarker(Register* parent, const QDate& date, const QString& txt) :
    FancyDateGroupMarker(parent, date, txt)
{
}

SimpleDateGroupMarker::SimpleDateGroupMarker(Register* parent, const QDate& date, const QString& txt) :
    FancyDateGroupMarker(parent, date, txt)
{
}

int SimpleDateGroupMarker::rowHeightHint() const
{
  if (!m_visible)
    return 0;

  return RegisterItem::rowHeightHint() / 2;
}

void SimpleDateGroupMarker::paintRegisterCell(QPainter *painter, QStyleOptionViewItem &option, const QModelIndex &index)
{
  QRect cellRect = option.rect;
  painter->save();
  cellRect.setWidth(m_parent->viewport()->width());
  cellRect.setHeight(m_parent->rowHeight(index.row() + m_startRow));

  if (m_alternate)
    option.palette.setColor(QPalette::Base, KMyMoneyGlobalSettings::listColor());
  else
    option.palette.setColor(QPalette::Base, KMyMoneyGlobalSettings::listBGColor());
  QBrush backgroundBrush(option.palette.color(QPalette::Base));
  backgroundBrush.setStyle(Qt::Dense5Pattern);
  backgroundBrush.setColor(KMyMoneyGlobalSettings::listGridColor());
  painter->eraseRect(cellRect);
  painter->fillRect(cellRect, backgroundBrush);
  painter->setPen(KMyMoneyGlobalSettings::listGridColor());
  painter->restore();
}

TypeGroupMarker::TypeGroupMarker(Register* parent, CashFlowDirection dir, MyMoneyAccount::accountTypeE accType) :
    GroupMarker(parent),
    m_dir(dir)
{
  switch (dir) {
    case Deposit:
      m_txt = i18nc("Deposits onto account", "Deposits");
      if (accType == MyMoneyAccount::CreditCard) {
        m_txt = i18nc("Payments towards credit card", "Payments");
      }
      break;
    case Payment:
      m_txt = i18nc("Payments made from account", "Payments");
      if (accType == MyMoneyAccount::CreditCard) {
        m_txt = i18nc("Payments made with credit card", "Charges");
      }
      break;
    default:
      qDebug("Unknown CashFlowDirection %d for TypeGroupMarker constructor", dir);
      break;
  }
}

PayeeGroupMarker::PayeeGroupMarker(Register* parent, const QString& name) :
    GroupMarker(parent, name)
{
}

CategoryGroupMarker::CategoryGroupMarker(Register* parent, const QString& category) :
    GroupMarker(parent, category)
{
}

ReconcileGroupMarker::ReconcileGroupMarker(Register* parent, MyMoneySplit::reconcileFlagE state) :
    GroupMarker(parent),
    m_state(state)
{
  switch (state) {
    case MyMoneySplit::NotReconciled:
      m_txt = i18nc("Reconcile state 'Not reconciled'", "Not reconciled");
      break;
    case MyMoneySplit::Cleared:
      m_txt = i18nc("Reconcile state 'Cleared'", "Cleared");
      break;
    case MyMoneySplit::Reconciled:
      m_txt = i18nc("Reconcile state 'Reconciled'", "Reconciled");
      break;
    case MyMoneySplit::Frozen:
      m_txt = i18nc("Reconcile state 'Frozen'", "Frozen");
      break;
    default:
      m_txt = i18nc("Unknown reconcile state", "Unknown");
      break;
  }
}

RegisterItemDelegate::RegisterItemDelegate(Register *parent) : QStyledItemDelegate(parent), m_register(parent)
{
}

RegisterItemDelegate::~RegisterItemDelegate()
{
}

void RegisterItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  RegisterItem* const item = m_register->itemAtRow(index.row());
  if (item && m_register->updatesEnabled()) {
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    item->paintRegisterCell(painter, opt, index);
  }
}

Register::Register(QWidget *parent) :
    TransactionEditorContainer(parent),
    m_selectAnchor(0),
    m_focusItem(0),
    m_firstItem(0),
    m_lastItem(0),
    m_firstErroneous(0),
    m_lastErroneous(0),
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
    m_detailsColumnType(PayeeFirst)
{
  // used for custom coloring with the help of the application's stylesheet
  setObjectName(QLatin1String("register"));
  setItemDelegate(new RegisterItemDelegate(this));

  setEditTriggers(QAbstractItemView::NoEditTriggers);
  setColumnCount(MaxColumns);
  setSelectionBehavior(QAbstractItemView::SelectRows);
  setAcceptDrops(true);
  setShowGrid(false);
  setContextMenuPolicy(Qt::DefaultContextMenu);

  setHorizontalHeaderItem(NumberColumn, new QTableWidgetItem());
  setHorizontalHeaderItem(DateColumn, new QTableWidgetItem());
  setHorizontalHeaderItem(AccountColumn, new QTableWidgetItem());
  setHorizontalHeaderItem(SecurityColumn, new QTableWidgetItem());
  setHorizontalHeaderItem(DetailColumn, new QTableWidgetItem());
  setHorizontalHeaderItem(ReconcileFlagColumn, new QTableWidgetItem());
  setHorizontalHeaderItem(PaymentColumn, new QTableWidgetItem());
  setHorizontalHeaderItem(DepositColumn, new QTableWidgetItem());
  setHorizontalHeaderItem(QuantityColumn, new QTableWidgetItem());
  setHorizontalHeaderItem(PriceColumn, new QTableWidgetItem());
  setHorizontalHeaderItem(ValueColumn, new QTableWidgetItem());
  setHorizontalHeaderItem(BalanceColumn, new QTableWidgetItem());

  // keep the following list in sync with KMyMoneyRegister::Column in transaction.h
  horizontalHeaderItem(NumberColumn)->setText(i18nc("Cheque Number", "No."));
  horizontalHeaderItem(DateColumn)->setText(i18n("Date"));
  horizontalHeaderItem(AccountColumn)->setText(i18n("Account"));
  horizontalHeaderItem(SecurityColumn)->setText(i18n("Security"));
  horizontalHeaderItem(DetailColumn)->setText(i18n("Details"));
  horizontalHeaderItem(ReconcileFlagColumn)->setText(i18n("C"));
  horizontalHeaderItem(PaymentColumn)->setText(i18n("Payment"));
  horizontalHeaderItem(DepositColumn)->setText(i18n("Deposit"));
  horizontalHeaderItem(QuantityColumn)->setText(i18n("Quantity"));
  horizontalHeaderItem(PriceColumn)->setText(i18n("Price"));
  horizontalHeaderItem(ValueColumn)->setText(i18n("Value"));
  horizontalHeaderItem(BalanceColumn)->setText(i18n("Balance"));

  verticalHeader()->hide();

  horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
  horizontalHeader()->setSortIndicatorShown(false);
  horizontalHeader()->setSectionsMovable(false);
  horizontalHeader()->setSectionsClickable(false);
  horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);

  connect(this, SIGNAL(cellClicked(int,int)), this, SLOT(selectItem(int,int)));
  connect(this, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(slotDoubleClicked(int,int)));
}

Register::~Register()
{
  clear();
}

bool Register::eventFilter(QObject* o, QEvent* e)
{
  if (o == this && e->type() == QEvent::KeyPress) {
    QKeyEvent* ke = dynamic_cast<QKeyEvent*>(e);
    if (ke->key() == Qt::Key_Menu) {
      emit openContextMenu();
      return true;
    }
  }
  return QTableWidget::eventFilter(o, e);
}

void Register::setupRegister(const MyMoneyAccount& account, const QList<Column>& cols)
{
  m_account = account;
  setUpdatesEnabled(false);

  for (int i = 0; i < MaxColumns; ++i)
    hideColumn(i);

  m_needInitialColumnResize = true;

  m_lastCol = static_cast<Column>(0);
  QList<Column>::const_iterator it_c;
  for (it_c = cols.begin(); it_c != cols.end(); ++it_c) {
    if ((*it_c) > MaxColumns)
      continue;
    showColumn(*it_c);
    if (*it_c > m_lastCol)
      m_lastCol = *it_c;
  }

  setUpdatesEnabled(true);
}

void Register::setupRegister(const MyMoneyAccount& account, bool showAccountColumn)
{
  m_account = account;
  setUpdatesEnabled(false);

  for (int i = 0; i < MaxColumns; ++i)
    hideColumn(i);

  horizontalHeaderItem(PaymentColumn)->setText(i18nc("Payment made from account", "Payment"));
  horizontalHeaderItem(DepositColumn)->setText(i18nc("Deposit into account", "Deposit"));

  if (account.id().isEmpty()) {
    setUpdatesEnabled(true);
    return;
  }

  m_needInitialColumnResize = true;

  // turn on standard columns
  showColumn(DateColumn);
  showColumn(DetailColumn);
  showColumn(ReconcileFlagColumn);

  // balance
  switch (account.accountType()) {
    case MyMoneyAccount::Stock:
      break;
    default:
      showColumn(BalanceColumn);
      break;
  }

  // Number column
  switch (account.accountType()) {
    case MyMoneyAccount::Savings:
    case MyMoneyAccount::Cash:
    case MyMoneyAccount::Loan:
    case MyMoneyAccount::AssetLoan:
    case MyMoneyAccount::Asset:
    case MyMoneyAccount::Liability:
    case MyMoneyAccount::Equity:
      if (KMyMoneyGlobalSettings::alwaysShowNrField())
        showColumn(NumberColumn);
      break;

    case MyMoneyAccount::Checkings:
    case MyMoneyAccount::CreditCard:
      showColumn(NumberColumn);
      break;

    default:
      hideColumn(NumberColumn);
      break;
  }

  switch (account.accountType()) {
    case MyMoneyAccount::Income:
    case MyMoneyAccount::Expense:
      showAccountColumn = true;
      break;
    default:
      break;
  }

  if (showAccountColumn)
    showColumn(AccountColumn);

  // Security, activity, payment, deposit, amount, price and value column
  switch (account.accountType()) {
    default:
      showColumn(PaymentColumn);
      showColumn(DepositColumn);
      break;

    case MyMoneyAccount::Investment:
      showColumn(SecurityColumn);
      showColumn(QuantityColumn);
      showColumn(PriceColumn);
      showColumn(ValueColumn);
      break;
  }

  // headings
  switch (account.accountType()) {
    case MyMoneyAccount::CreditCard:
      horizontalHeaderItem(PaymentColumn)->setText(i18nc("Payment made with credit card", "Charge"));
      horizontalHeaderItem(DepositColumn)->setText(i18nc("Payment towards credit card", "Payment"));
      break;
    case MyMoneyAccount::Asset:
    case MyMoneyAccount::AssetLoan:
      horizontalHeaderItem(PaymentColumn)->setText(i18nc("Decrease of asset/liability value", "Decrease"));
      horizontalHeaderItem(DepositColumn)->setText(i18nc("Increase of asset/liability value", "Increase"));
      break;
    case MyMoneyAccount::Liability:
    case MyMoneyAccount::Loan:
      horizontalHeaderItem(PaymentColumn)->setText(i18nc("Increase of asset/liability value", "Increase"));
      horizontalHeaderItem(DepositColumn)->setText(i18nc("Decrease of asset/liability value", "Decrease"));
      break;
    case MyMoneyAccount::Income:
    case MyMoneyAccount::Expense:
      horizontalHeaderItem(PaymentColumn)->setText(i18n("Income"));
      horizontalHeaderItem(DepositColumn)->setText(i18n("Expense"));
      break;

    default:
      break;
  }

  m_lastCol = BalanceColumn;

  setUpdatesEnabled(true);
}

bool Register::focusNextPrevChild(bool next)
{
  return QFrame::focusNextPrevChild(next);
}

void Register::setSortOrder(const QString& order)
{
  const QStringList orderList = order.split(',', QString::SkipEmptyParts);
  QStringList::const_iterator it;
  m_sortOrder.clear();
  for (it = orderList.constBegin(); it != orderList.constEnd(); ++it) {
    m_sortOrder << static_cast<TransactionSortField>((*it).toInt());
  }
}

void Register::sortItems()
{
  if (m_items.count() == 0)
    return;

  // sort the array of pointers to the transactions
  m_items.sort();

  // update the next/prev item chains
  RegisterItem* prev = 0;
  RegisterItem* item;
  m_firstItem = m_lastItem = 0;
  for (QVector<RegisterItem*>::size_type i = 0; i < m_items.size(); ++i) {
    item = m_items[i];
    if (!item)
      continue;

    if (!m_firstItem)
      m_firstItem = item;
    m_lastItem = item;
    if (prev)
      prev->setNextItem(item);
    item->setPrevItem(prev);
    item->setNextItem(0);
    prev = item;
  }

  // update the balance visibility settings
  item = m_lastItem;
  bool showBalance = true;
  while (item) {
    Transaction* t = dynamic_cast<Transaction*>(item);
    if (t) {
      t->setShowBalance(showBalance);
      if (!t->isVisible()) {
        showBalance = false;
      }
    }
    item = item->prevItem();
  }

  // force update of the item index (row to item array)
  m_listsDirty = true;
}

TransactionSortField Register::primarySortKey() const
{
  if (!m_sortOrder.isEmpty())
    return static_cast<KMyMoneyRegister::TransactionSortField>(m_sortOrder.first());
  return UnknownSort;
}


void Register::clear()
{
  m_firstErroneous = m_lastErroneous = 0;
  m_ensureVisibleItem = 0;

  m_items.clear();

  RegisterItem* p;
  while ((p = firstItem()) != 0) {
    delete p;
  }

  m_firstItem = m_lastItem = 0;

  m_listsDirty = true;
  m_selectAnchor = 0;
  m_focusItem = 0;

#ifndef KMM_DESIGNER
  // recalculate row height hint
  QFontMetrics fm(KMyMoneyGlobalSettings::listCellFont());
  m_rowHeightHint = fm.lineSpacing() + 6;
#endif

  m_needInitialColumnResize = true;
  m_needResize = true;
  updateRegister(true);
}

void Register::insertItemAfter(RegisterItem*p, RegisterItem* prev)
{
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

  if (!m_firstItem)
    m_firstItem = p;
  if (!m_lastItem)
    m_lastItem = p;

  if (prev == m_lastItem)
    m_lastItem = p;

  m_listsDirty = true;
  m_needResize = true;
}

void Register::addItem(RegisterItem* p)
{
  RegisterItem* q = lastItem();
  if (q)
    q->setNextItem(p);
  p->setPrevItem(q);
  p->setNextItem(0);

  m_items.append(p);

  if (!m_firstItem)
    m_firstItem = p;
  m_lastItem = p;
  m_listsDirty = true;
  m_needResize = true;
}

void Register::removeItem(RegisterItem* p)
{
  // remove item from list
  if (p->prevItem())
    p->prevItem()->setNextItem(p->nextItem());
  if (p->nextItem())
    p->nextItem()->setPrevItem(p->prevItem());

  // update first and last pointer if required
  if (p == m_firstItem)
    m_firstItem = p->nextItem();
  if (p == m_lastItem)
    m_lastItem = p->prevItem();

  // make sure we don't do it twice
  p->setNextItem(0);
  p->setPrevItem(0);

  // remove it from the m_items array
  int i = m_items.indexOf(p);
  if (-1 != i) {
    m_items[i] = 0;
  }
  m_listsDirty = true;
  m_needResize = true;
}

RegisterItem* Register::firstItem() const
{
  return m_firstItem;
}

RegisterItem* Register::nextItem(RegisterItem* item) const
{
  return item->nextItem();
}

RegisterItem* Register::lastItem() const
{
  return m_lastItem;
}

void Register::setupItemIndex(int rowCount)
{
  // setup index array
  m_itemIndex.clear();
  m_itemIndex.reserve(rowCount);

  // fill index array
  rowCount = 0;
  RegisterItem* prev = 0;
  m_firstItem = m_lastItem = 0;
  for (QVector<RegisterItem*>::size_type i = 0; i < m_items.size(); ++i) {
    RegisterItem* item = m_items[i];
    if (!item)
      continue;
    if (!m_firstItem)
      m_firstItem = item;
    m_lastItem = item;
    if (prev)
      prev->setNextItem(item);
    item->setPrevItem(prev);
    item->setNextItem(0);
    prev = item;
    for (int j = item->numRowsRegister(); j; --j) {
      m_itemIndex.push_back(item);
    }
  }
}

void Register::updateAlternate() const
{
  bool alternate = false;
  for (QVector<RegisterItem*>::size_type i = 0; i < m_items.size(); ++i) {
    RegisterItem* item = m_items[i];
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
  KMyMoneyRegister::Transaction* t = dynamic_cast<KMyMoneyRegister::Transaction*>(p);
  if (t && t->transaction().id().isEmpty()) {
    lastWasGroupMarker = true;
    p = p->prevItem();
  }
  while (p) {
    KMyMoneyRegister::GroupMarker* m = dynamic_cast<KMyMoneyRegister::GroupMarker*>(p);
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
  if (m_listsDirty || forceUpdateRowHeight) {
    // don't get in here recursively
    m_listsDirty = false;

    int rowCount = 0;
    // determine the number of rows we need to display all items
    // while going through the list, check for erroneous transactions
    for (QVector<RegisterItem*>::size_type i = 0; i < m_items.size(); ++i) {
      RegisterItem* item = m_items[i];
      if (!item)
        continue;
      item->setStartRow(rowCount);
      item->setNeedResize();
      rowCount += item->numRowsRegister();

      if (item->isErroneous()) {
        if (!m_firstErroneous)
          m_firstErroneous = item;
        m_lastErroneous = item;
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
      for (int i = 0; i < rowCount; ++i) {
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
    if (m_needInitialColumnResize) {
      QTimer::singleShot(0, this, SLOT(resize()));
      m_needInitialColumnResize = false;
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
  if (!m_rowHeightHint) {
    qDebug("Register::rowHeightHint(): m_rowHeightHint is zero!!");
  }
  return m_rowHeightHint;
}

void Register::focusInEvent(QFocusEvent* ev)
{
  QTableWidget::focusInEvent(ev);
  if (m_focusItem) {
    m_focusItem->setFocus(true, false);
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
  if (m_focusItem) {
    m_focusItem->setFocus(false, false);
  }
  QTableWidget::focusOutEvent(ev);
}

void Register::resizeEvent(QResizeEvent* ev)
{
  TransactionEditorContainer::resizeEvent(ev);
  resize(DetailColumn, true);
}

void Register::resize()
{
  resize(DetailColumn);
}

void Register::resize(int col, bool force)
{
  if (!m_needResize && !force)
    return;

  m_needResize = false;

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
    if (columnWidth(NumberColumn))
      adjustColumn(NumberColumn);
    if (columnWidth(AccountColumn))
      adjustColumn(AccountColumn);
    if (columnWidth(PaymentColumn))
      adjustColumn(PaymentColumn);
    if (columnWidth(DepositColumn))
      adjustColumn(DepositColumn);
    if (columnWidth(QuantityColumn))
      adjustColumn(QuantityColumn);
    if (columnWidth(BalanceColumn))
      adjustColumn(BalanceColumn);
    if (columnWidth(PriceColumn))
      adjustColumn(PriceColumn);
    if (columnWidth(ValueColumn))
      adjustColumn(ValueColumn);

    // make amount columns all the same size
    // only extend the entry columns to make sure they fit
    // the widget
    int dwidth = 0;
    int ewidth = 0;
    if (ewidth < columnWidth(PaymentColumn))
      ewidth = columnWidth(PaymentColumn);
    if (ewidth < columnWidth(DepositColumn))
      ewidth = columnWidth(DepositColumn);
    if (ewidth < columnWidth(QuantityColumn))
      ewidth = columnWidth(QuantityColumn);
    if (dwidth < columnWidth(BalanceColumn))
      dwidth = columnWidth(BalanceColumn);
    if (ewidth < columnWidth(PriceColumn))
      ewidth = columnWidth(PriceColumn);
    if (dwidth < columnWidth(ValueColumn))
      dwidth = columnWidth(ValueColumn);
    int swidth = columnWidth(SecurityColumn);
    if (swidth > 0) {
      adjustColumn(SecurityColumn);
      swidth = columnWidth(SecurityColumn);
    }

    adjustColumn(DateColumn);

#ifndef KMM_DESIGNER
    // Resize the date and money fields to either
    // a) the size required by the input widget if no transaction form is shown and the register is used with an editor
    // b) the adjusted value for the input widget if the transaction form is visible or an editor is not used
    if (m_usedWithEditor && !KMyMoneyGlobalSettings::transactionForm()) {
      QPushButton *pushButton = new QPushButton;
      const int pushButtonSpacing = pushButton->sizeHint().width() + 5;
      setColumnWidth(DateColumn, columnWidth(DateColumn) + pushButtonSpacing + 4/* space for the spinbox arrows */);
      ewidth += pushButtonSpacing;

      if (swidth > 0) {
        // extend the security width to make space for the selector arrow
        swidth = columnWidth(SecurityColumn) + 40;
      }
      delete pushButton;
    }
#endif

    if (columnWidth(PaymentColumn))
      setColumnWidth(PaymentColumn, ewidth);
    if (columnWidth(DepositColumn))
      setColumnWidth(DepositColumn, ewidth);
    if (columnWidth(QuantityColumn))
      setColumnWidth(QuantityColumn, ewidth);
    if (columnWidth(BalanceColumn))
      setColumnWidth(BalanceColumn, dwidth);
    if (columnWidth(PriceColumn))
      setColumnWidth(PriceColumn, ewidth);
    if (columnWidth(ValueColumn))
      setColumnWidth(ValueColumn, dwidth);

    if (columnWidth(ReconcileFlagColumn))
      setColumnWidth(ReconcileFlagColumn, 20);

    if (swidth > 0)
      setColumnWidth(SecurityColumn, swidth);
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

  for (int i = 0; i < columnCount(); ++i) {
    if (i == col)
      continue;

    w -= columnWidth(i);
  }
  setColumnWidth(col, w);
}

int Register::minimumColumnWidth(int col)
{
  QHeaderView *topHeader = horizontalHeader();
  int w = topHeader->fontMetrics().width(horizontalHeaderItem(col) ? horizontalHeaderItem(col)->text() : QString()) + 10;
  w = qMax(w, 20);
#ifdef KMM_DESIGNER
  return w;
#else
  int maxWidth = 0;
  int minWidth = 0;
  QFontMetrics cellFontMetrics(KMyMoneyGlobalSettings::listCellFont());
  switch (col) {
    case DateColumn:
      minWidth = cellFontMetrics.width(QLocale().toString(QDate(6999, 12, 29), QLocale::ShortFormat) + "  ");
      break;
    default:
      break;
  }

  // scan through the transactions
  for (int i = 0; i < m_items.size(); ++i) {
    RegisterItem* const item = m_items[i];
    if (!item)
      continue;
    Transaction* t = dynamic_cast<Transaction*>(item);
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
        //qDebug("%s", qPrintable(e.what()));
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
  if ((end <= -1) || (end > (m_items.size() - 1)))
    end = m_items.size() - 1;

  RegisterItem* firstItem;
  RegisterItem* lastItem;
  firstItem = lastItem = 0;
  for (int i = start; i <= end; ++i) {
    RegisterItem* const item = m_items[i];
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
  if (row >= 0 && row < m_itemIndex.size()) {
    return m_itemIndex[row];
  }
  return 0;
}

int Register::rowToIndex(int row) const
{
  for (int i = 0; i < m_items.size(); ++i) {
    RegisterItem* const item = m_items[i];
    if (!item)
      continue;
    if (row >= item->startRow() && row < (item->startRow() + item->numRowsRegister()))
      return i;
  }
  return -1;
}

void Register::selectedTransactions(SelectedTransactions& list) const
{
  if (m_focusItem && m_focusItem->isSelected() && m_focusItem->isVisible()) {
    Transaction* t = dynamic_cast<Transaction*>(m_focusItem);
    if (t) {
      QString id;
      if (t->isScheduled())
        id = t->transaction().id();
      SelectedTransaction s(t->transaction(), t->split(), id);
      list << s;
    }
  }

  for (int i = 0; i < m_items.size(); ++i) {
    RegisterItem* const item = m_items[i];
    // make sure, we don't include the focus item twice
    if (item == m_focusItem)
      continue;
    if (item && item->isSelected() && item->isVisible()) {
      Transaction* t = dynamic_cast<Transaction*>(item);
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
  QList<RegisterItem*> list;

  RegisterItem* item = m_firstItem;
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
  int cnt = 0;
  RegisterItem* item = m_firstItem;
  while (item) {
    if (item->isSelected() && item->isVisible())
      ++cnt;
    item = item->nextItem();
  }
  return cnt;
}

void Register::mouseReleaseEvent(QMouseEvent *e)
{
  if (e->button() == Qt::RightButton) {
    // see the comment in Register::contextMenuEvent
    // on Linux we never get here but on Windows this
    // event is fired before the contextMenuEvent which
    // causes the loss of the multiple selection; to avoid
    // this just ignore the event and act like on Linux
    return;
  }
  if (m_ignoreNextButtonRelease) {
    m_ignoreNextButtonRelease = false;
    return;
  }
  m_mouseButton = e->button();
  m_modifiers = QApplication::keyboardModifiers();
  QTableWidget::mouseReleaseEvent(e);
}

void Register::contextMenuEvent(QContextMenuEvent *e)
{
  if (e->reason() == QContextMenuEvent::Mouse) {
    // since mouse release event is not called, we need
    // to reset the mouse button and the modifiers here
    m_mouseButton = Qt::NoButton;
    m_modifiers = Qt::NoModifier;

    // if a selected item is clicked don't change the selection
    RegisterItem* item = itemAtRow(rowAt(e->y()));
    if (item && !item->isSelected())
      selectItem(rowAt(e->y()), columnAt(e->x()));
  }
  openContextMenu();
}

void Register::selectItem(int row, int col)
{
  if (row >= 0 && row < m_itemIndex.size()) {
    RegisterItem* item = m_itemIndex[row];

    // don't support selecting when the item has an editor
    // or the item itself is not selectable
    if (item->hasEditorOpen() || !item->isSelectable()) {
      m_mouseButton = Qt::NoButton;
      return;
    }
    QString id = item->id();
    selectItem(item);
    // selectItem() might have changed the pointers, so we
    // need to reconstruct it here
    item = itemById(id);
    Transaction* t = dynamic_cast<Transaction*>(item);
    if (t) {
      if (!id.isEmpty()) {
        if (t && col == ReconcileFlagColumn && selectedItemsCount() == 1 && !t->isScheduled())
          emit reconcileStateColumnClicked(t);
      } else {
        emit emptyItemSelected();
      }
    }
  }
}

void Register::setAnchorItem(RegisterItem* anchorItem)
{
  m_selectAnchor = anchorItem;
}

bool Register::setFocusItem(RegisterItem* focusItem)
{
  if (focusItem && focusItem->canHaveFocus()) {
    if (m_focusItem) {
      m_focusItem->setFocus(false);
    }
    Transaction* item = dynamic_cast<Transaction*>(focusItem);
    if (m_focusItem != focusItem && item) {
      emit focusChanged(item);
    }

    m_focusItem = focusItem;
    m_focusItem->setFocus(true);
    if (m_listsDirty)
      updateRegister(KMyMoneyGlobalSettings::ledgerLens() | !KMyMoneyGlobalSettings::transactionForm());
    ensureItemVisible(m_focusItem);
    return true;
  } else
    return false;
}

bool Register::setFocusToTop()
{
  RegisterItem* rgItem = m_firstItem;
  while (rgItem) {
    if (setFocusItem(rgItem))
      return true;
    rgItem = rgItem->nextItem();
  }
  return false;
}

void Register::selectItem(RegisterItem* item, bool dontChangeSelections)
{
  if (!item)
    return;

  Qt::MouseButtons buttonState = m_mouseButton;
  Qt::KeyboardModifiers modifiers = m_modifiers;
  m_mouseButton = Qt::NoButton;
  m_modifiers = Qt::NoModifier;

  if (m_selectionMode == NoSelection)
    return;

  if (item->isSelectable()) {
    QString id = item->id();
    QList<RegisterItem*> itemList = selectedItems();
    bool okToSelect = true;
    int cnt = itemList.count();
    const bool scheduledTransactionSelected = (cnt > 0 && itemList.front() && (typeid(*(itemList.front())) == typeid(StdTransactionScheduled)));
    if (buttonState & Qt::LeftButton) {
      if (!(modifiers & (Qt::ShiftModifier | Qt::ControlModifier))
          || (m_selectAnchor == 0)) {
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
          m_selectAnchor = item;
      }

      if (m_selectionMode == MultiSelection) {
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
              selectItems(rowToIndex(m_selectAnchor->startRow()), rowToIndex(item->startRow()));
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
        m_selectAnchor = item;
      }
    }
    if (okToSelect) {
      SelectedTransactions list(this);
      emit transactionsSelected(list);
    }
  }
}

void Register::ensureItemVisible(RegisterItem* item)
{
  if (!item)
    return;

  m_ensureVisibleItem = item;
  QTimer::singleShot(0, this, SLOT(slotEnsureItemVisible()));
}

void Register::slotDoubleClicked(int row, int)
{
  if (row >= 0 && row < m_itemIndex.size()) {
    RegisterItem* p = m_itemIndex[row];
    if (p->isSelectable()) {
      m_ignoreNextButtonRelease = true;
      // double click to start editing only works if the focus
      // item is among the selected ones
      if (!focusItem()) {
        setFocusItem(p);
        if (m_selectionMode != NoSelection)
          p->setSelected(true);
      }

      if (m_focusItem->isSelected()) {
        // don't emit the signal right away but wait until
        // we come back to the Qt main loop
        QTimer::singleShot(0, this, SIGNAL(editTransaction()));
      }
    }
  }
}

void Register::slotEnsureItemVisible()
{
  // if clear() has been called since the timer was
  // started, we just ignore the call
  if (!m_ensureVisibleItem)
    return;

  // make sure to catch latest changes
  setUpdatesEnabled(false);
  updateRegister();
  setUpdatesEnabled(true);
  // since the item will be made visible at the top of the viewport make the bottom index visible first to make the whole item visible
  scrollTo(model()->index(m_ensureVisibleItem->startRow() + m_ensureVisibleItem->numRowsRegister() - 1, DetailColumn));
  scrollTo(model()->index(m_ensureVisibleItem->startRow(), DetailColumn));
}

TransactionSortField KMyMoneyRegister::textToSortOrder(const QString& text)
{
  for (int idx = 1; idx < static_cast<int>(MaxSortFields); ++idx) {
    if (text == i18n(sortOrderText[idx])) {
      return static_cast<TransactionSortField>(idx);
    }
  }
  return UnknownSort;
}

const QString KMyMoneyRegister::sortOrderToText(TransactionSortField idx)
{
  if (idx < PostDateSort || idx >= MaxSortFields)
    idx = UnknownSort;
  return i18n(sortOrderText[idx]);
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
  KMyMoneyRegister::Transaction* t = dynamic_cast<KMyMoneyRegister::Transaction*>(focusItem());
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

RegisterItem* Register::itemById(const QString& id) const
{
  if (id.isEmpty())
    return m_lastItem;

  for (QVector<RegisterItem*>::size_type i = 0; i < m_items.size(); ++i) {
    RegisterItem* item = m_items[i];
    if (!item)
      continue;
    if (item->id() == id)
      return item;
  }
  return 0;
}

void Register::handleItemChange(RegisterItem* old, bool shift, bool control)
{
  if (m_selectionMode == MultiSelection) {
    if (shift) {
      selectRange(m_selectAnchor ? m_selectAnchor : old,
                  m_focusItem, false, true, (m_selectAnchor && !control) ? true : false);
    } else if (!control) {
      selectItem(m_focusItem, false);
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
  RegisterItem* oldFocusItem = m_focusItem;

  // make sure we have a focus item
  if (!m_focusItem)
    setFocusItem(m_firstItem);
  if (!m_focusItem && m_firstItem)
    setFocusItem(m_firstItem->nextItem());
  if (!m_focusItem)
    return;

  RegisterItem* item = m_focusItem;
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
      item = m_firstItem;
      while ((!item->isSelectable() || !item->isVisible()) && item->nextItem())
        item = item->nextItem();
      break;

    case Qt::Key_End:
      item = m_lastItem;
      while ((!item->isSelectable() || !item->isVisible()) && item->prevItem())
        item = item->prevItem();
      break;
  }

  // make sure to avoid selecting a possible empty transaction at the end
  Transaction* t = dynamic_cast<Transaction*>(item);
  if (t && t->transaction().id().isEmpty()) {
    if (t->prevItem()) {
      item = t->prevItem();
    }
  }

  if (!(modifiers & Qt::ShiftModifier) || !m_selectAnchor)
    m_selectAnchor = item;

  setFocusItem(item);

  if (item->isSelectable()) {
    handleItemChange(oldFocusItem, modifiers & Qt::ShiftModifier, modifiers & Qt::ControlModifier);
    // tell the world about the changes in selection
    SelectedTransactions list(this);
    emit transactionsSelected(list);
  }

  if (m_focusItem && !m_focusItem->isSelected() && m_selectionMode == SingleSelection)
    selectItem(item);

}

void Register::keyPressEvent(QKeyEvent* ev)
{
  switch (ev->key()) {
    case Qt::Key_Space:
      if (m_selectionMode != NoSelection) {
        // get the state out of the event ...
        m_modifiers = ev->modifiers();
        // ... and pretend that we have pressed the left mouse button ;)
        m_mouseButton = Qt::LeftButton;
        selectItem(m_focusItem);
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
    case MyMoneyAccount::Checkings:
    case MyMoneyAccount::Savings:
    case MyMoneyAccount::Cash:
    case MyMoneyAccount::CreditCard:
    case MyMoneyAccount::Loan:
    case MyMoneyAccount::Asset:
    case MyMoneyAccount::Liability:
    case MyMoneyAccount::Currency:
    case MyMoneyAccount::Income:
    case MyMoneyAccount::Expense:
    case MyMoneyAccount::AssetLoan:
    case MyMoneyAccount::Equity:
      if (s.accountId().isEmpty())
        s.setAccountId(parent->account().id());
      if (s.isMatched())
        t = new KMyMoneyRegister::StdTransactionMatched(parent, transaction, s, uniqueId);
      else if (transaction.isImported())
        t = new KMyMoneyRegister::StdTransactionDownloaded(parent, transaction, s, uniqueId);
      else
        t = new KMyMoneyRegister::StdTransaction(parent, transaction, s, uniqueId);
      break;

    case MyMoneyAccount::Investment:
      if (s.isMatched())
        t = new KMyMoneyRegister::InvestTransaction/* Matched */(parent, transaction, s, uniqueId);
      else if (transaction.isImported())
        t = new KMyMoneyRegister::InvestTransactionDownloaded(parent, transaction, s, uniqueId);
      else
        t = new KMyMoneyRegister::InvestTransaction(parent, transaction, s, uniqueId);
      break;

    case MyMoneyAccount::CertificateDep:
    case MyMoneyAccount::MoneyMarket:
    case MyMoneyAccount::Stock:
    default:
      qDebug("Register::transactionFactory: invalid accountTypeE %d", parent->account().accountType());
      break;
  }
  return t;
}

void Register::addGroupMarkers()
{
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
    case KMyMoneyRegister::PostDateSort:
    case KMyMoneyRegister::EntryDateSort:
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
      if (KMyMoneyGlobalSettings::startDate().date() != QDate(1900, 1, 1))
        new KMyMoneyRegister::FancyDateGroupMarker(this, KMyMoneyGlobalSettings::startDate().date(), i18n("Prior transactions possibly filtered"));

      if (KMyMoneyGlobalSettings::showFancyMarker()) {
        if (m_account.lastReconciliationDate().isValid())
          new KMyMoneyRegister::StatementGroupMarker(this, KMyMoneyRegister::Deposit, m_account.lastReconciliationDate(), i18n("Last reconciliation"));

        if (!m_account.value("lastImportedTransactionDate").isEmpty()
            && !m_account.value("lastStatementBalance").isEmpty()) {
          MyMoneyMoney balance(m_account.value("lastStatementBalance"));
          if (m_account.accountGroup() == MyMoneyAccount::Liability)
            balance = -balance;
          QString txt = i18n("Online Statement Balance: %1", balance.formatMoney(m_account.fraction()));

          KMyMoneyRegister::StatementGroupMarker *p = new KMyMoneyRegister::StatementGroupMarker(this, KMyMoneyRegister::Deposit, QDate::fromString(m_account.value("lastImportedTransactionDate"), Qt::ISODate), txt);

          p->setErroneous(!MyMoneyFile::instance()->hasMatchingOnlineBalance(m_account));
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
      if (KMyMoneyGlobalSettings::showFiscalMarker()) {
        QDate currentFiscalYear = KMyMoneyGlobalSettings::firstFiscalDate();
        new KMyMoneyRegister::FiscalYearGroupMarker(this, currentFiscalYear, i18n("Current fiscal year"));
        new KMyMoneyRegister::FiscalYearGroupMarker(this, currentFiscalYear.addYears(-1), i18n("Previous fiscal year"));
        new KMyMoneyRegister::FiscalYearGroupMarker(this, currentFiscalYear.addYears(1), i18n("Next fiscal year"));
      }
      break;

    case KMyMoneyRegister::TypeSort:
      if (KMyMoneyGlobalSettings::showFancyMarker()) {
        new KMyMoneyRegister::TypeGroupMarker(this, KMyMoneyRegister::Deposit, m_account.accountType());
        new KMyMoneyRegister::TypeGroupMarker(this, KMyMoneyRegister::Payment, m_account.accountType());
      }
      break;

    case KMyMoneyRegister::ReconcileStateSort:
      if (KMyMoneyGlobalSettings::showFancyMarker()) {
        new KMyMoneyRegister::ReconcileGroupMarker(this, MyMoneySplit::NotReconciled);
        new KMyMoneyRegister::ReconcileGroupMarker(this, MyMoneySplit::Cleared);
        new KMyMoneyRegister::ReconcileGroupMarker(this, MyMoneySplit::Reconciled);
        new KMyMoneyRegister::ReconcileGroupMarker(this, MyMoneySplit::Frozen);
      }
      break;

    case KMyMoneyRegister::PayeeSort:
      if (KMyMoneyGlobalSettings::showFancyMarker()) {
        while (p) {
          t = dynamic_cast<KMyMoneyRegister::Transaction*>(p);
          if (t) {
            list[t->sortPayee()] = 1;
          }
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

    case KMyMoneyRegister::CategorySort:
      if (KMyMoneyGlobalSettings::showFancyMarker()) {
        while (p) {
          t = dynamic_cast<KMyMoneyRegister::Transaction*>(p);
          if (t) {
            list[t->sortCategory()] = 1;
          }
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

    case KMyMoneyRegister::SecuritySort:
      if (KMyMoneyGlobalSettings::showFancyMarker()) {
        while (p) {
          t = dynamic_cast<KMyMoneyRegister::InvestTransaction*>(p);
          if (t) {
            list[t->sortSecurity()] = 1;
          }
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
    KMyMoneyRegister::GroupMarker* m = dynamic_cast<KMyMoneyRegister::GroupMarker*>(p);
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

DetailsColumnType Register::getDetailsColumnType() const
{
  return m_detailsColumnType;
}

void Register::setDetailsColumnType(DetailsColumnType detailsColumnType)
{
  m_detailsColumnType = detailsColumnType;
}
