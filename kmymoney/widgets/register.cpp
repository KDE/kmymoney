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

#include <typeinfo>

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QTimer>
#include <QApplication>
#include <QEventLoop>
#include <QToolTip>
#include <QImage>
//Added by qt3to4:
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

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>
#include <k3urldrag.h>

// ----------------------------------------------------------------------------
// Project Includes
#include <config-kmymoney.h>
#include <kmymoneydateinput.h>
#include <kmymoneyedit.h>
#include <kmymoneycategory.h>
#include <register.h>
#include <transactionform.h>
#include <stdtransactiondownloaded.h>
#include <stdtransactionmatched.h>
#include "scheduledtransaction.h"
#include "kmymoneyglobalsettings.h"

const int LinesPerMemo = 3;

static QString sortOrderText[] = {
  I18N_NOOP("Unknown"),
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
  0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A,
  0x00,0x00,0x00,0x0D,0x49,0x48,0x44,0x52,
  0x00,0x00,0x00,0xC8,0x00,0x00,0x00,0x31,
  0x08,0x06,0x00,0x00,0x00,0x9F,0xC5,0xE6,
  0x4F,0x00,0x00,0x00,0x06,0x62,0x4B,0x47,
  0x44,0x00,0xFF,0x00,0xFF,0x00,0xFF,0xA0,
  0xBD,0xA7,0x93,0x00,0x00,0x00,0x09,0x70,
  0x48,0x59,0x73,0x00,0x00,0x0B,0x13,0x00,
  0x00,0x0B,0x13,0x01,0x00,0x9A,0x9C,0x18,
  0x00,0x00,0x00,0x86,0x49,0x44,0x41,0x54,
  0x78,0xDA,0xED,0xDD,0x31,0x0A,0x84,0x40,
  0x10,0x44,0xD1,0x1A,0x19,0x10,0xCF,0xE6,
  0xFD,0x4F,0xB2,0x88,0x08,0x22,0x9B,0x18,
  0x4E,0x1B,0x2C,0x1B,0x18,0xBC,0x07,0x7D,
  0x81,0x82,0x1F,0x77,0x4B,0xB2,0x06,0x18,
  0xEA,0x49,0x3E,0x66,0x00,0x81,0x80,0x40,
  0xE0,0xDF,0x81,0x6C,0x66,0x80,0x3A,0x90,
  0xDD,0x0C,0x50,0x07,0x72,0x98,0x01,0xEA,
  0x40,0x4E,0x33,0x40,0x1D,0xC8,0x65,0x06,
  0x18,0x6B,0xF7,0x01,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0xF0,0x16,0x3E,
  0x4C,0xC1,0x83,0x9E,0x64,0x32,0x03,0x08,
  0x04,0x7E,0x0A,0xA4,0x9B,0x01,0xEA,0x40,
  0x66,0x33,0x40,0x1D,0xC8,0x62,0x06,0x18,
  0xFB,0x02,0x05,0x87,0x08,0x55,0xFE,0xDE,
  0xA2,0x9D,0x00,0x00,0x00,0x00,0x49,0x45,
  0x4E,0x44,0xAE,0x42,0x60,0x82
};

QPixmap* GroupMarker::m_bg = 0;
int GroupMarker::m_bgRefCnt = 0;

void ItemPtrVector::sort(void)
{
  if(count() > 0) {
    // get rid of 0 pointers in the list
    KMyMoneyRegister::ItemPtrVector::iterator it_l;
    RegisterItem *item;
    for(it_l = begin(); it_l != end(); ++it_l) {
      if(*it_l == 0) {
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

  MyMoneyMoney tmp;

  for(it = sortOrder.begin(); it != sortOrder.end(); ++it) {
    TransactionSortField sortField = static_cast<TransactionSortField>(abs(*it));
    switch(sortField) {
      case PostDateSort:
        rc = i2->sortPostDate().daysTo(i1->sortPostDate());
        if(rc == 0) {
          rc = i1->sortSamePostDate() - i2->sortSamePostDate();
        }
        break;

      case EntryDateSort:
        rc = i2->sortEntryDate().daysTo(i1->sortEntryDate());
        break;

      case PayeeSort:
        rc = QString::localeAwareCompare(i1->sortPayee(), i2->sortPayee());
        break;

      case ValueSort:
        tmp = i1->sortValue() - i2->sortValue();
        if(tmp.isZero())
          rc = 0;
        else if(tmp.isNegative())
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
        if(ok1 && ok2) {  // case a)
          rc = (n1 > n2) ? 1 : ((n1 == n2 ) ? 0 : -1);
        } else if(ok1 && !ok2) {
          rc = -1;
        } else if(!ok1 && ok2) {
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

    // the items differ for this sort key so we can return a result
    if(rc != 0)
      return (*it < 0) ? rc >= 0 : rc < 0;
  }

  if(rc == 0) {
    rc = qstrcmp(i1->sortEntryOrder().toLatin1(), i2->sortEntryOrder().toLatin1());
  }

  return rc < 0;
}

GroupMarker::GroupMarker(Register *parent, const QString& txt) :
  RegisterItem(parent),
  m_txt(txt),
  m_drawCounter(parent->drawCounter()-1),   // make sure we get painted the first time around
  m_showDate(false)
{
  int h;
  if(m_parent) {
    h = m_parent->rowHeightHint();
  } else {
    QFontMetrics fm( KMyMoneyGlobalSettings::listCellFont() );
    h = fm.lineSpacing()+6;
  }

  if(m_bg && (m_bg->height() != h)) {
    delete m_bg;
    m_bg = 0;
  }

  // convert the backgroud once
  if(m_bg == 0) {
    m_bg = new QPixmap;
    m_bg->loadFromData(fancymarker_bg_image, sizeof(fancymarker_bg_image));
    *m_bg = m_bg->scaled(m_bg->width(), h, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
  }

  ++m_bgRefCnt;
}

GroupMarker::~GroupMarker()
{
  --m_bgRefCnt;
  if(!m_bgRefCnt) {
    delete m_bg;
    m_bg = 0;
  }
}

void GroupMarker::paintRegisterCell(QPainter* painter, int row, int /* col */, const QRect& _r, bool /*selected*/, const QColorGroup& _cg)
{
  // avoid painting the marker twice for the same update round
  unsigned int drawCounter = m_parent->drawCounter();
  if(m_drawCounter == drawCounter) {
    return;
  }
  m_drawCounter = drawCounter;

  QRect r(_r);
  painter->save();
  painter->translate(-r.x(), -r.y());

  // the group marker always uses all cols
  r.setX(m_parent->columnPos(0));
  r.setWidth(m_parent->visibleWidth());
  painter->translate(r.x(), r.y());

  QRect cellRect;
  cellRect.setX(0);
  cellRect.setY(0);
  cellRect.setWidth(m_parent->visibleWidth());
  cellRect.setHeight(m_parent->rowHeight(row + m_startRow));

  // clear out cell rectangle
  QColorGroup cg(_cg);
  setupColors(cg);

  QBrush backgroundBrush(cg.base());
  painter->fillRect(cellRect, backgroundBrush);
  painter->setPen(KMyMoneyGlobalSettings::listGridColor());
  painter->drawLine(cellRect.x(), cellRect.height()-1, cellRect.width(), cellRect.height()-1);

  // now write the text
  painter->setPen(cg.text());
  QFont font = painter->font();
  font.setBold(true);
  painter->setFont(font);

  painter->drawText(cellRect, Qt::AlignVCenter | Qt::AlignCenter, m_txt);

  cellRect.setHeight(m_bg->height());
  int curWidth = m_bg->width();

  // now it's time to draw the background
  painter->drawPixmap(cellRect, *m_bg);

  // translate back
  painter->translate(-r.x(), -r.y());

  // in case we need to show the date, we just paint it in col 1
  if(m_showDate) {
    r.setX(m_parent->columnPos(1));
    r.setWidth(m_parent->columnWidth(1));
    painter->translate(r.x(), r.y());

    cellRect.setX(0);
    cellRect.setY(0);
    cellRect.setWidth(m_parent->columnWidth(1));
    cellRect.setHeight(m_parent->rowHeight(row + m_startRow));

    font.setBold(false);
    painter->setFont(font);
    painter->drawText(cellRect, Qt::AlignVCenter | Qt::AlignCenter, KGlobal::locale()->formatDate(sortPostDate(), KLocale::ShortDate));
  }

  painter->restore();
}

void GroupMarker::setupColors(QColorGroup& cg)
{
  cg.setColor(QColorGroup::Base, KMyMoneyGlobalSettings::groupMarkerColor());
}

int GroupMarker::rowHeightHint(void) const
{
  if(!m_visible)
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

void FiscalYearGroupMarker::setupColors(QColorGroup& cg)
{
  cg.setColor(QColorGroup::Base, KMyMoneyGlobalSettings::groupMarkerColor());
}


SimpleDateGroupMarker::SimpleDateGroupMarker(Register* parent, const QDate& date, const QString& txt) :
  FancyDateGroupMarker(parent, date, txt)
{
}

int SimpleDateGroupMarker::rowHeightHint(void) const
{
  if(!m_visible)
    return 0;

  return RegisterItem::rowHeightHint() / 2;
}

void SimpleDateGroupMarker::paintRegisterCell(QPainter* painter, int row, int /*col*/, const QRect& _r, bool /*selected*/, const QColorGroup& _cg)
{
  QRect r(_r);
  painter->save();
  painter->translate(-r.x(), -r.y());

  // the group marker always uses all cols
  r.setX(m_parent->columnPos(0));
  r.setWidth(m_parent->visibleWidth());
  painter->translate(r.x(), r.y());

  QRect cellRect;
  cellRect.setX(0);
  cellRect.setY(0);
  cellRect.setWidth(m_parent->visibleWidth());
  cellRect.setHeight(m_parent->rowHeight(row + m_startRow));

  // clear out cell rectangle
  QColorGroup cg(_cg);
  if(m_alternate)
    cg.setColor(QColorGroup::Base, KMyMoneyGlobalSettings::listColor());
  else
    cg.setColor(QColorGroup::Base, KMyMoneyGlobalSettings::listBGColor());
  QBrush backgroundBrush(cg.base());
  // backgroundBrush.setStyle(Qt::DiagCrossPattern);
  backgroundBrush.setStyle(Qt::Dense5Pattern);
  backgroundBrush.setColor(KMyMoneyGlobalSettings::listGridColor());
  painter->eraseRect(cellRect);
  painter->fillRect(cellRect, backgroundBrush);
  painter->setPen(KMyMoneyGlobalSettings::listGridColor());
  painter->drawLine(cellRect.x(), cellRect.height()-1, cellRect.width(), cellRect.height()-1);

  painter->restore();
}

TypeGroupMarker::TypeGroupMarker(Register* parent, CashFlowDirection dir, MyMoneyAccount::accountTypeE accType) :
  GroupMarker(parent),
  m_dir(dir)
{
  switch(dir) {
    case Deposit:
      m_txt = i18nc("Deposits onto account", "Deposits");
      if(accType == MyMoneyAccount::CreditCard) {
        m_txt = i18nc("Payments towards credit card", "Payments");
      }
      break;
    case Payment:
      m_txt = i18nc("Payments made from account", "Payments");
      if(accType == MyMoneyAccount::CreditCard) {
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
  switch(state) {
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
#if 0
class RegisterToolTip : public QToolTip
{
public:
  RegisterToolTip(QWidget* parent, Register* reg);
  void maybeTip(const QPoint& pos);
  virtual ~RegisterToolTip() {}

private:
  Register* m_register;
};

RegisterToolTip::RegisterToolTip(QWidget* parent, Register * reg) :
  QToolTip(parent),
  m_register(reg)
{
}

void RegisterToolTip::maybeTip(const QPoint& pos)
{
  // if we update the register, there's no need to show tooltips
  if(!m_register->isUpdatesEnabled())
    return;

  QPoint cpos = m_register->viewportToContents(pos);
  // qDebug("RegisterToolTip::mayBeTip(%d,%d)", cpos.x(), cpos.y());
  int row = m_register->rowAt(cpos.y());
  int col = m_register->columnAt(cpos.x());
  RegisterItem* item = m_register->itemAtRow(row);
  if(!item)
    return;

  QPoint relPos(cpos.x() - m_register->columnPos(0), cpos.y() - m_register->rowPos(item->startRow()));
  row = row - item->startRow();

  // qDebug("row = %d, col = %d", row, col);
  // qDebug("relpos = %d,%d", relPos.x(), relPos.y());
  QString msg;
  QRect rect;
  if(!item->maybeTip(cpos, row, col, rect, msg))
    return;

  QPoint tl(rect.topLeft());
  QPoint br(rect.bottomRight());
  QRect r = QRect(m_register->contentsToViewport(tl), m_register->contentsToViewport(br));
  tip(r, msg);
  return;
}
#endif

Register::Register(QWidget *parent) :
  TransactionEditorContainer(parent),
  m_selectAnchor(0),
  m_focusItem(0),
  m_firstItem(0),
  m_lastItem(0),
  m_firstErronous(0),
  m_lastErronous(0),
  m_markErronousTransactions(0),
  m_rowHeightHint(0),
  m_ledgerLensForced(false),
  m_selectionMode(Multi),
  m_listsDirty(false),
  m_ignoreNextButtonRelease(false),
  m_needInitialColumnResize(false),
  m_buttonState(Qt::ButtonState(0)),
  m_drawCounter(0)
{
//FIXME: Port to KDE4	
  //m_tooltip = new RegisterToolTip(viewport(), this);

  setNumCols(MaxColumns);
  setCurrentCell(0, 1);
  // we do our own sorting
  setSorting(false);

  // keep the following list in sync with KMyMoneyRegister::Column in transaction.h
  horizontalHeader()->setLabel(NumberColumn, i18nc("Cheque Number", "No."));
  horizontalHeader()->setLabel(DateColumn, i18n("Date"));
  horizontalHeader()->setLabel(AccountColumn, i18n("Account"));
  horizontalHeader()->setLabel(SecurityColumn, i18n("Security"));
  horizontalHeader()->setLabel(DetailColumn, i18n("Details"));
  horizontalHeader()->setLabel(ReconcileFlagColumn, i18n("C"));
  horizontalHeader()->setLabel(PaymentColumn, i18n("Payment"));
  horizontalHeader()->setLabel(DepositColumn, i18n("Deposit"));
  horizontalHeader()->setLabel(QuantityColumn, i18n("Quantity"));
  horizontalHeader()->setLabel(PriceColumn, i18n("Price"));
  horizontalHeader()->setLabel(ValueColumn, i18n("Value"));
  horizontalHeader()->setLabel(BalanceColumn, i18n("Balance"));

  setLeftMargin(0);
  verticalHeader()->hide();

  for(int i = 0; i < numCols(); ++i)
    setColumnStretchable(i, false);

  horizontalHeader()->setResizeEnabled(false);
  horizontalHeader()->setMovingEnabled(false);
  horizontalHeader()->setClickEnabled(false);

  horizontalHeader()->installEventFilter(this);

  // never show horizontal scroll bars
  setHScrollBarMode(Q3ScrollView::AlwaysOff);

  connect(this, SIGNAL(clicked(int, int, int, const QPoint&)), this, SLOT(selectItem(int, int, int, const QPoint&)));
  connect(this, SIGNAL(doubleClicked(int, int, int, const QPoint&)), this, SLOT(slotDoubleClicked(int, int, int, const QPoint&)));

  // double clicking the header turns on auto column sizing
  connect(horizontalHeader(), SIGNAL(sectionSizeChanged(int)), this, SLOT(slotAutoColumnSizing(int)));

  //DND
  setAcceptDrops(true);
}

// DND
Transaction* Register::dropTransaction(QPoint cPos) const
{
  Transaction* t = 0;
  cPos -= QPoint( verticalHeader()->width(), horizontalHeader()->height() );
  if(cPos.y() >= 0) {
    cPos += QPoint(contentsX(), contentsY());
    int row = rowAt(cPos.y());
    t = dynamic_cast<Transaction*>(itemAtRow(row));
  }
  return t;
}

void Register::dragMoveEvent(QDragMoveEvent* event)
{
  if ( K3URLDrag::canDecode(event) ) {
    event->ignore();
    Transaction* t = dropTransaction(event->pos());
    if(t && !t->isScheduled()) {
      event->accept();
    }
  }
}

void Register::dropEvent(QDropEvent* event)
{
  qDebug("Register::dropEvent");
  if ( K3URLDrag::canDecode(event) ) {
    event->ignore();
    Transaction* t = dropTransaction(event->pos());
    if(t && !t->isScheduled()) {
      qDebug("Drop was ok");
      KUrl::List urls;
      K3URLDrag::decode(event, urls);
      qDebug("List is '%s'", qPrintable(urls.toStringList().join(";")));
      event->accept();
    }
  }
}
// DND end


Register::~Register()
{
  clear();
  //delete m_tooltip;
  //m_tooltip = 0;
}

void Register::slotAutoColumnSizing(int section)
{
  Q_UNUSED(section)
#if 0
  // this is some trial code to make the col sizes adjustable
  // there are some drawbacks though: what when we have a register
  // but no account? (ipwizard 2007-11-06)
  if(isUpdatesEnabled()) {
    int w = visibleWidth();
    QString size;
    for(int i=0; i < numCols(); ++i) {
      if(i)
        size += ",";
      if(i == DetailColumn) {
        size += "0";
        continue;
      }
      size += QString("%1").arg((columnWidth(i) * 100) / w);
    }
    qDebug("size = %s", size.data());
    m_account.setValue("kmm-ledger-column-width", size);
  }
#endif
}

bool Register::eventFilter(QObject* o, QEvent* e)
{
  if(o == horizontalHeader() && e->type() == QEvent::MouseButtonPress) {
    QMouseEvent *me = dynamic_cast<QMouseEvent*>(e);
    if(me->button() == Qt::RightButton) {
      emit headerClicked();
    }
    // eat up left mouse button press for now
    return true;

  } else if(o == horizontalHeader() && e->type() == QEvent::Paint) {
    // always show the header in bold (to suppress cell selection)
    QFont f(horizontalHeader()->font());
    f.setBold(true);
    horizontalHeader()->setFont(f);

  } else if(o == this && e->type() == QEvent::KeyPress) {
    QKeyEvent* ke = dynamic_cast<QKeyEvent*>(e);
    if(ke->key() == Qt::Key_Menu) {
      emit openContextMenu();
      return true;
    }
  }

  return Q3Table::eventFilter(o, e);
}

void Register::setupRegister(const MyMoneyAccount& account, const QList<Column>& cols)
{
  m_account = account;
  bool enabled = isUpdatesEnabled();
  setUpdatesEnabled(false);

  for(int i = 0; i < MaxColumns; ++i)
    hideColumn(i);

  m_needInitialColumnResize = true;

  m_lastCol = static_cast<Column>(0);
  QList<Column>::const_iterator it_c;
  for(it_c = cols.begin(); it_c != cols.end(); ++it_c) {
    if((*it_c) > MaxColumns)
      continue;
    showColumn(*it_c);
    if(*it_c > m_lastCol)
      m_lastCol = *it_c;
  }

  setUpdatesEnabled(enabled);
}

void Register::setupRegister(const MyMoneyAccount& account, bool showAccountColumn)
{
  m_account = account;
  bool enabled = isUpdatesEnabled();
  setUpdatesEnabled(false);

  for(int i = 0; i < MaxColumns; ++i)
    hideColumn(i);

  horizontalHeader()->setLabel(PaymentColumn, i18nc("Payment made from account", "Payment"));
  horizontalHeader()->setLabel(DepositColumn, i18nc("Deposit into account", "Deposit"));

  if(account.id().isEmpty()) {
    setUpdatesEnabled(enabled);
    return;
  }

  m_needInitialColumnResize = true;

  // turn on standard columns
  showColumn(DateColumn);
  showColumn(DetailColumn);
  showColumn(ReconcileFlagColumn);

  // balance
  switch(account.accountType()) {
    case MyMoneyAccount::Stock:
      break;
    default:
      showColumn(BalanceColumn);
      break;
  }

  // Number column
  switch(account.accountType()) {
    case MyMoneyAccount::Savings:
    case MyMoneyAccount::Cash:
    case MyMoneyAccount::Loan:
    case MyMoneyAccount::AssetLoan:
    case MyMoneyAccount::Asset:
    case MyMoneyAccount::Liability:
    case MyMoneyAccount::Equity:
      if(KMyMoneyGlobalSettings::alwaysShowNrField())
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

  switch(account.accountType()) {
    case MyMoneyAccount::Income:
    case MyMoneyAccount::Expense:
      showAccountColumn = true;
      break;
    default:
      break;
  }

  if(showAccountColumn)
    showColumn(AccountColumn);

  // Security, activity, payment, deposit, amount, price and value column
  switch(account.accountType()) {
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
  switch(account.accountType()) {
    case MyMoneyAccount::CreditCard:
      horizontalHeader()->setLabel(PaymentColumn, i18nc("Payment made with credit card", "Charge"));
      horizontalHeader()->setLabel(DepositColumn, i18nc("Payment towards credit card", "Payment"));
      break;
    case MyMoneyAccount::Asset:
    case MyMoneyAccount::AssetLoan:
      horizontalHeader()->setLabel(PaymentColumn, i18nc("Decrease of asset/liability value", "Decrease"));
      horizontalHeader()->setLabel(DepositColumn, i18nc("Increase of asset/liability value", "Increase"));
      break;
    case MyMoneyAccount::Liability:
    case MyMoneyAccount::Loan:
      horizontalHeader()->setLabel(PaymentColumn, i18nc("Increase of asset/liability value", "Increase"));
      horizontalHeader()->setLabel(DepositColumn, i18nc("Decrease of asset/liability value", "Decrease"));
      break;
    case MyMoneyAccount::Income:
    case MyMoneyAccount::Expense:
      horizontalHeader()->setLabel(PaymentColumn, i18n("Income"));
      horizontalHeader()->setLabel(DepositColumn, i18n("Expense"));
      break;

    default:
      break;
  }

  switch(account.accountType()) {
    default:
      m_lastCol = BalanceColumn;
      break;
  }

  setUpdatesEnabled(enabled);
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
  for(it = orderList.constBegin(); it != orderList.constEnd(); ++it) {
    m_sortOrder << static_cast<TransactionSortField>((*it).toInt());
  }
}

void Register::sortItems(void)
{
  if(m_items.count() == 0)
    return;

  // sort the array of pointers to the transactions
  m_items.sort();

  // update the next/prev item chains
  RegisterItem* prev = 0;
  RegisterItem* item;
  m_firstItem = m_lastItem = 0;
  for(Q3ValueVector<RegisterItem*>::size_type i = 0; i < m_items.size(); ++i) {
    item = m_items[i];
    if(!item)
      continue;

    if(!m_firstItem)
      m_firstItem = item;
    m_lastItem = item;
    if(prev)
      prev->setNextItem(item);
    item->setPrevItem(prev);
    item->setNextItem(0);
    prev = item;
  }

  // update the balance visibility settings
  item = m_lastItem;
  bool showBalance = true;
  while(item) {
    Transaction* t = dynamic_cast<Transaction*>(item);
    if(t) {
      t->setShowBalance(showBalance);
      if(!t->isVisible()) {
        showBalance = false;
      }
    }
    item = item->prevItem();
  }

  // force update of the item index (row to item array)
  m_listsDirty = true;
}

TransactionSortField Register::primarySortKey(void) const
{
  if(!m_sortOrder.isEmpty())
    return static_cast<KMyMoneyRegister::TransactionSortField>(abs(m_sortOrder.first()));
  return UnknownSort;
}


void Register::clear(void)
{
  m_firstErronous = m_lastErronous = 0;
  m_ensureVisibleItem = 0;

  RegisterItem* p;
  while((p = firstItem()) != 0) {
    delete p;
  }
  m_items.clear();

  m_firstItem = m_lastItem = 0;

  m_listsDirty = true;
  m_selectAnchor = 0;
  m_focusItem = 0;

#ifndef KMM_DESIGNER
  // recalculate row height hint
  QFontMetrics fm( KMyMoneyGlobalSettings::listCellFont() );
  m_rowHeightHint = fm.lineSpacing()+6;
#endif

  m_needInitialColumnResize = true;
}

void Register::insertItemAfter(RegisterItem*p, RegisterItem* prev)
{
  RegisterItem* next = 0;
  if(!prev)
    prev = lastItem();

  if(prev) {
    next = prev->nextItem();
    prev->setNextItem(p);
  }
  if(next)
    next->setPrevItem(p);

  p->setPrevItem(prev);
  p->setNextItem(next);

  if(!m_firstItem)
    m_firstItem = p;
  if(!m_lastItem)
    m_lastItem = p;

  if(prev == m_lastItem)
    m_lastItem = p;

  m_listsDirty = true;
}

void Register::addItem(RegisterItem* p)
{
  RegisterItem* q = lastItem();
  if(q)
    q->setNextItem(p);
  p->setPrevItem(q);
  p->setNextItem(0);

  m_items.append(p);

  if(!m_firstItem)
    m_firstItem = p;
  m_lastItem = p;
  m_listsDirty = true;
}

void Register::removeItem(RegisterItem* p)
{
  // remove item from list
  if(p->prevItem())
    p->prevItem()->setNextItem(p->nextItem());
  if(p->nextItem())
    p->nextItem()->setPrevItem(p->prevItem());

  // update first and last pointer if required
  if(p == m_firstItem)
    m_firstItem = p->nextItem();
  if(p == m_lastItem)
    m_lastItem = p->prevItem();

  // make sure we don't do it twice
  p->setNextItem(0);
  p->setPrevItem(0);

  // remove it from the m_items array
  for(Q3ValueVector<RegisterItem*>::size_type i = 0; i < m_items.size(); ++i) {
    RegisterItem* item = m_items[i];
    if(!item)
      continue;
    if(item == p) {
      m_items[i] = 0;
      break;
    }
  }
  m_listsDirty = true;
}

RegisterItem* Register::firstItem(void) const
{
  return m_firstItem;
}

RegisterItem* Register::lastItem(void) const
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
  for(Q3ValueVector<RegisterItem*>::size_type i = 0; i < m_items.size(); ++i) {
    RegisterItem* item = m_items[i];
    if(!item)
      continue;
    if(!m_firstItem)
      m_firstItem = item;
    m_lastItem = item;
    if(prev)
      prev->setNextItem(item);
    item->setPrevItem(prev);
    item->setNextItem(0);
    prev = item;
    for(int j = item->numRowsRegister(); j; --j) {
      m_itemIndex.push_back(item);
    }
  }
}

void Register::drawContents( QPainter *p, int cx, int cy, int cw, int ch )
{
  // the QTable::drawContents() method does not honor the block update flag
  // so we take care of it here
  if (testAttribute(Qt::WA_UpdatesDisabled))
    return;

  if(m_listsDirty) {
    updateRegister(KMyMoneyGlobalSettings::ledgerLens() | !KMyMoneyGlobalSettings::transactionForm());
  }

  ++m_drawCounter;
  Q3Table::drawContents(p, cx, cy, cw, ch);
}

void Register::updateAlternate(void) const
{
  bool alternate = false;
  for(Q3ValueVector<RegisterItem*>::size_type i = 0; i < m_items.size(); ++i) {
    RegisterItem* item = m_items[i];
    if(!item)
      continue;
    if(item->isVisible()) {
      item->setAlternate(alternate);
      alternate ^= true;
    }
  }
}

void Register::suppressAdjacentMarkers(void)
{
  bool lastWasGroupMarker = false;
  KMyMoneyRegister::RegisterItem* p = lastItem();
  KMyMoneyRegister::Transaction* t = dynamic_cast<KMyMoneyRegister::Transaction*>(p);
  if(t && t->transaction().id().isEmpty()) {
    lastWasGroupMarker = true;
    p = p->prevItem();
  }
  while(p) {
    KMyMoneyRegister::GroupMarker* m = dynamic_cast<KMyMoneyRegister::GroupMarker*>(p);
    if(m) {
      // make adjacent group marker invisible except those that show statement information
      if(lastWasGroupMarker && (dynamic_cast<KMyMoneyRegister::StatementGroupMarker*>(m) == 0)) {
        m->setVisible(false);
      }
      lastWasGroupMarker = true;
    } else if(p->isVisible())
      lastWasGroupMarker = false;
    p = p->prevItem();
  }
}

void Register::updateRegister(bool forceUpdateRowHeight)
{
//FIXME: Port to KDE4	
  //::timetrace("Update register");
  if(m_listsDirty || forceUpdateRowHeight) {
    // don't get in here recursively
    m_listsDirty = false;

    int rowCount = 0;
    // determine the number of rows we need to display all items
    // while going through the list, check for erronous transactions
    for(Q3ValueVector<RegisterItem*>::size_type i = 0; i < m_items.size(); ++i) {
      RegisterItem* item = m_items[i];
      if(!item)
        continue;
      item->setStartRow(rowCount);
      item->setNeedResize();
      rowCount += item->numRowsRegister();

      if(item->isErronous()) {
        if(!m_firstErronous)
          m_firstErronous = item;
        m_lastErronous = item;
      }
    }

    updateAlternate();

    // create item index
    setupItemIndex(rowCount);

    bool needUpdateHeaders = (numRows() != rowCount) | forceUpdateRowHeight;

    // setup QTable.  Make sure to suppress screen updates for now
    bool updatesEnabled = isUpdatesEnabled();
    setUpdatesEnabled(false);
    setNumRows(rowCount);

    // if we need to update the headers, we do it now for all rows
    // again we make sure to suppress screen updates
    if(needUpdateHeaders) {
      // int height = rowHeightHint();

      verticalHeader()->setUpdatesEnabled(false);

      for(int i = 0; i < rowCount; ++i) {
        RegisterItem* item = itemAtRow(i);
        if(item->isVisible()) {
          showRow(i);
        } else {
          hideRow(i);
        }
        verticalHeader()->resizeSection(i, item->rowHeightHint());
      }
      verticalHeader()->setUpdatesEnabled(true);
    }

    // add or remove scrollbars as required
    updateScrollBars();

    setUpdatesEnabled(updatesEnabled);

    // force resizeing of the columns if necessary
    if(m_needInitialColumnResize) {
      QTimer::singleShot(0, this, SLOT(resize()));
      m_needInitialColumnResize = false;
    } else {
      updateContents();

      // if the number of rows changed, we might need to resize the register
      // to make sure we reflect the current visibility of the scrollbars.
      if(needUpdateHeaders)
        QTimer::singleShot(0, this, SLOT(resize()));
    }
  }
//FIXME: Port to KDE4  
  //::timetrace("Done updateing register");
}

int Register::rowHeightHint(void) const
{
  if(!m_rowHeightHint) {
    qDebug("Register::rowHeightHint(): m_rowHeightHint is zero!!");
  }
  return m_rowHeightHint;
}

void Register::paintCell(QPainter* painter, int row, int col, const QRect& r, bool selected, const QColorGroup& cg)
{
  // determine the item that we need to paint in the row and call it's paintRegisterCell() method
  if((row < 0) || (row > m_itemIndex.size())) {
    qDebug("Register::paintCell: row %d out of bounds %d", row, (int)m_itemIndex.size());
    return;
  }

  // qDebug("paintCell(%d,%d)", row, col);
  RegisterItem* const item = m_itemIndex[row];
  item->paintRegisterCell(painter, row - item->startRow(), col, r, selected, cg);
}

void Register::focusInEvent(QFocusEvent* ev)
{
  Q3Table::focusInEvent(ev);
  if(m_focusItem) {
    m_focusItem->setFocus(true, false);
    repaintItems(m_focusItem);
  }
}

void Register::focusOutEvent(QFocusEvent* ev)
{
  if(m_focusItem) {
    m_focusItem->setFocus(false, false);
    repaintItems(m_focusItem);
  }
  Q3Table::focusOutEvent(ev);
}

void Register::resize(void)
{
  resize(DetailColumn);
}

void Register::resize(int col)
{
  bool enabled = isUpdatesEnabled();
  setUpdatesEnabled(false);

  // resize the register
  int w = visibleWidth();

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
  if(m_account.value("kmm-ledger-column-width").isEmpty()) {
#endif

    // check which space we need
    if(columnWidth(NumberColumn))
      adjustColumn(NumberColumn);
    if(columnWidth(AccountColumn))
      adjustColumn(AccountColumn);
    if(columnWidth(PaymentColumn))
      adjustColumn(PaymentColumn);
    if(columnWidth(DepositColumn))
      adjustColumn(DepositColumn);
    if(columnWidth(BalanceColumn))
      adjustColumn(BalanceColumn);
    if(columnWidth(PriceColumn))
      adjustColumn(PriceColumn);
    if(columnWidth(ValueColumn))
      adjustColumn(ValueColumn);

    // make amount columns all the same size
    // only extend the entry columns to make sure they fit
    // the widget
    int dwidth = 0;
    int ewidth = 0;
    if(ewidth < columnWidth(PaymentColumn))
      ewidth = columnWidth(PaymentColumn);
    if(ewidth < columnWidth(DepositColumn))
      ewidth = columnWidth(DepositColumn);
    if(dwidth < columnWidth(BalanceColumn))
      dwidth = columnWidth(BalanceColumn);
    if(ewidth < columnWidth(PriceColumn))
      ewidth = columnWidth(PriceColumn);
    if(dwidth < columnWidth(ValueColumn))
      dwidth = columnWidth(ValueColumn);

    int swidth = columnWidth(SecurityColumn);
    if(swidth > 0) {
      adjustColumn(SecurityColumn);
      swidth = columnWidth(SecurityColumn);
    }

#ifndef KMM_DESIGNER
    // Resize the date and money fields to either
    // a) the size required by the input widget if no transaction form is shown
    // b) the adjusted value for the input widget if the transaction form is visible
    if(!KMyMoneyGlobalSettings::transactionForm()) {
      kMyMoneyDateInput* dateField = new kMyMoneyDateInput;
      kMyMoneyEdit* valField = new kMyMoneyEdit;

      dateField->setFont(KMyMoneyGlobalSettings::listCellFont());
      setColumnWidth(DateColumn, dateField->minimumSizeHint().width());
      valField->setMinimumWidth(ewidth);
      ewidth = valField->minimumSizeHint().width();

      if(swidth > 0) {
        swidth = columnWidth(SecurityColumn) + 40;
      }
      delete valField;
      delete dateField;
    } else {
      adjustColumn(DateColumn);
    }
#endif

    if(columnWidth(PaymentColumn))
      setColumnWidth(PaymentColumn, ewidth);
    if(columnWidth(DepositColumn))
      setColumnWidth(DepositColumn, ewidth);
    if(columnWidth(BalanceColumn))
      setColumnWidth(BalanceColumn, dwidth);
    if(columnWidth(PriceColumn))
      setColumnWidth(PriceColumn, ewidth);
    if(columnWidth(ValueColumn))
      setColumnWidth(ValueColumn, dwidth);

    if(columnWidth(ReconcileFlagColumn))
      setColumnWidth(ReconcileFlagColumn, 20);

    if(swidth > 0)
      setColumnWidth(SecurityColumn, swidth);
#if 0
  // see comment above
  } else {
    QStringList colSizes = QStringList::split(",", m_account.value("kmm-ledger-column-width"), true);
    for(int i; i < colSizes.count(); ++i) {
      int colWidth = colSizes[i].toInt();
      if(colWidth == 0)
        continue;
      setColumnWidth(i, w * colWidth / 100);
    }
  }
#endif

  for(int i = 0; i < numCols(); ++i) {
    if(i == col)
      continue;

    w -= columnWidth(i);
  }
  setColumnWidth(col, w);

  setUpdatesEnabled(enabled);
  updateContents();
}


void Register::adjustColumn(int col)
{
#ifdef KMM_DESIGNER
  Q_UNUSED(col)
#else
  QString msg = "%1 adjusting column %2";
//FIXME: Port to KDE4
  //::timetrace((msg.arg("Start").arg(col)).data());
  Q3Header *topHeader = horizontalHeader();
  QFontMetrics cellFontMetrics(KMyMoneyGlobalSettings::listCellFont());

  int w = topHeader->fontMetrics().width( topHeader->label( col ) ) + 10;
  if ( topHeader->iconSet( col ) )
    w += topHeader->iconSet( col )->pixmap().width();
  w = qMax( w, 20 );

  int maxWidth = 0;
  switch(col) {
    case NumberColumn:
      maxWidth = cellFontMetrics.width("0123456789");
      break;
    default:
      break;
  }

  // check for date column
  if(col == DateColumn) {
    QString txt = KGlobal::locale()->formatDate(QDate(6999,12,29), KLocale::ShortDate);
    int nw = cellFontMetrics.width(txt+"  ");
    w = qMax( w, nw );
  } else {

    // scan through the transactions
    for(int i = 0; i < m_items.size(); ++i) {
      RegisterItem* const item = m_items[i];
      if(!item)
        continue;
      Transaction* t = dynamic_cast<Transaction*>(item);
      if(t) {
        int nw = t->registerColWidth(col, cellFontMetrics);
        w = qMax( w, nw );
        if(maxWidth) {
          if(w > maxWidth) {
            w = maxWidth;
            break;
          }
        }
      }
    }
  }

  setColumnWidth( col, w );
#endif
}

void Register::repaintItems(RegisterItem* first, RegisterItem* last)
{
  if(first == 0 && last == 0) {
    first = firstItem();
    last = lastItem();
  }

  if(first == 0)
    return;

  if(last == 0)
    last = first;

  // qDebug("repaintItems from row %d to row %d", first->startRow(), last->startRow()+last->numRowsRegister()-1);

  // the following code is based on code I found in
  // QTable::cellGeometry() and QTable::updateCell()  (ipwizard)
  QRect cg(0,
           rowPos(first->startRow()),
           visibleWidth(),
           rowPos(last->startRow()+last->numRowsRegister()-1) - rowPos(first->startRow()) + rowHeight(last->startRow()+last->numRowsRegister()-1));

  QRect r(contentsToViewport(QPoint (cg.x() - 2, cg.y() - 2 )), QSize(cg.width() + 4, cg.height() + 4 ));

  QRect tmp = m_lastRepaintRect | r;
  if(abs(tmp.height()) > 3000) {
    // make sure that the previously triggered repaint has been done before we
    // trigger the next. Not having this used to cause some trouble when changing
    // the focus within a 2000 item ledger from the last to the first item.
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInput, 10);
  }
  m_lastRepaintRect = r;
//FIXME: Port to KDE4
  QApplication::postEvent( viewport(), new QPaintEvent( r ) );

}

void Register::clearSelection(void)
{
  unselectItems();
}

void Register::doSelectItems(int from, int to, bool selected)
{
  int start, end;
  // make sure start is smaller than end
  if(from <= to) {
    start = from;
    end = to;
  } else {
    start = to;
    end = from;
  }
  // make sure we stay in bounds
  if(start < 0)
    start = 0;
  if((end <= -1) || (end > (m_items.size()-1)))
    end = m_items.size()-1;

  RegisterItem* firstItem;
  RegisterItem* lastItem;
  firstItem = lastItem = 0;
  for(int i = start; i <= end; ++i) {
    RegisterItem* const item = m_items[i];
    if(item) {
      if(selected != item->isSelected()) {
        if(!firstItem)
          firstItem = item;
        item->setSelected(selected);
        lastItem = item;
      }
    }
  }

  // anything changed?
  if(firstItem || lastItem)
    repaintItems(firstItem, lastItem);
}

RegisterItem* Register::itemAtRow(int row) const
{
  if(row >= 0 && row < m_itemIndex.size()) {
    return m_itemIndex[row];
  }
  return 0;
}

int Register::rowToIndex(int row) const
{
  for(int i = 0; i < m_items.size(); ++i) {
    RegisterItem* const item = m_items[i];
    if(!item)
      continue;
    if(row >= item->startRow() && row < (item->startRow() + item->numRowsRegister()))
      return i;
  }
  return -1;
}

void Register::selectedTransactions(SelectedTransactions& list) const
{
  if(m_focusItem && m_focusItem->isSelected() && m_focusItem->isVisible()) {
    Transaction* t = dynamic_cast<Transaction*>(m_focusItem);
    if(t) {
      QString id;
      if(t->isScheduled())
        id = t->transaction().id();
      SelectedTransaction s(t->transaction(), t->split(), id);
      list << s;
    }
  }

  for(int i = 0; i < m_items.size(); ++i) {
    RegisterItem* const item = m_items[i];
    // make sure, we don't include the focus item twice
    if(item == m_focusItem)
      continue;
    if(item && item->isSelected() && item->isVisible()) {
      Transaction* t = dynamic_cast<Transaction*>(item);
      if(t) {
        QString id;
        if(t->isScheduled())
          id = t->transaction().id();
        SelectedTransaction s(t->transaction(), t->split(), id);
        list << s;
      }
    }
  }
}

QList<RegisterItem*> Register::selectedItems(void) const
{
  QList<RegisterItem*> list;

  RegisterItem* item = m_firstItem;
  while(item) {
    if(item && item->isSelected() && item->isVisible()) {
      list << item;
    }
    item = item->nextItem();
  }
  return list;
}

int Register::selectedItemsCount(void) const
{
  int cnt = 0;
  RegisterItem* item = m_firstItem;
  while(item) {
    if(item->isSelected() && item->isVisible())
      ++cnt;
    item = item->nextItem();
  }
  return cnt;
}

void Register::contentsMouseReleaseEvent( QMouseEvent *e )
{
  if(m_ignoreNextButtonRelease) {
    m_ignoreNextButtonRelease = false;
    return;
  }

  m_buttonState = e->state();
  Q3Table::contentsMouseReleaseEvent(e);
}

void Register::selectItem(int row, int col, int button, const QPoint& /* mousePos */)
{
  if(row >= 0 && row < m_itemIndex.size()) {
    RegisterItem* item = m_itemIndex[row];

    // don't support selecting when the item has an editor
    // or the item itself is not selectable
    if(item->hasEditorOpen() || !item->isSelectable())
      return;

    QString id = item->id();
    selectItem(item);
    // selectItem() might have changed the pointers, so we
    // need to reconstruct it here
    item = itemById(id);
    Transaction* t = dynamic_cast<Transaction*>(item);
    if(t) {
      if(!id.isEmpty()) {
        switch(button & Qt::MouseButtonMask) {
          case Qt::RightButton:
            emit openContextMenu();
            break;

          case Qt::LeftButton:
            if(t && col == ReconcileFlagColumn && selectedItemsCount() == 1 && !t->isScheduled())
              emit reconcileStateColumnClicked(t);
            break;

          default:
            break;
        }
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
  if(focusItem && focusItem->canHaveFocus()) {
    if(m_focusItem) {
      m_focusItem->setFocus(false);
      // issue a repaint here only if we move the focus
      if(m_focusItem != focusItem)
        repaintItems(m_focusItem);
    }
    Transaction* item = dynamic_cast<Transaction*>(focusItem);
    if(m_focusItem != focusItem && item) {
      emit focusChanged(item);
    }

    m_focusItem = focusItem;
    m_focusItem->setFocus(true);
    if(m_listsDirty)
      updateRegister(KMyMoneyGlobalSettings::ledgerLens() | !KMyMoneyGlobalSettings::transactionForm());
    ensureItemVisible(m_focusItem);
    repaintItems(m_focusItem);
    return true;
  } else
    return false;
}

bool Register::setFocusToTop(void)
{
  RegisterItem* rgItem=m_firstItem;
  while (rgItem) {
    if (setFocusItem(rgItem))
      return true;
    rgItem=rgItem->nextItem();
  }
  return false;
}

void Register::selectItem(RegisterItem* item, bool dontChangeSelections)
{
  if(!item)
    return;

  // kDebug(2) << "Register::selectItem(" << item << "): type is " << typeid(*item).name();

  Qt::ButtonState buttonState = m_buttonState;
  m_buttonState = Qt::NoButton;

  if(m_selectionMode == NoSelection)
    return;

  if(item->isSelectable()) {
    QString id = item->id();
    QList<RegisterItem*> itemList = selectedItems();
    bool okToSelect = true;
    int cnt = itemList.count();
    bool sameEntryType = true;
    if(cnt > 0) {
      if(typeid(*itemList.begin()) != typeid(item))
        sameEntryType = false;
    }

    if(buttonState & Qt::LeftButton) {
      if(!(buttonState & (Qt::ShiftModifier | Qt::ControlModifier))) {
        if((cnt != 1) || ((cnt == 1) && !item->isSelected())) {
          emit aboutToSelectItem(item, okToSelect);
          if(okToSelect) {
            // pointer 'item' might have changed. reconstruct it.
            item = itemById(id);
            unselectItems();
            item->setSelected(true);
            setFocusItem(item);
          }
        }
        if(okToSelect)
          m_selectAnchor = item;
      }

      if(m_selectionMode == Multi) {
        switch(buttonState & (Qt::ShiftModifier | Qt::ControlModifier)) {
          case Qt::ControlModifier:
            okToSelect = sameEntryType;
            if(typeid(*item) == typeid(StdTransactionScheduled))
              okToSelect = false;
            // toggle selection state of current item
            emit aboutToSelectItem(item, okToSelect);
            if(okToSelect) {
              // pointer 'item' might have changed. reconstruct it.
              item = itemById(id);
              item->setSelected(!item->isSelected());
              setFocusItem(item);
            }
            break;

          case Qt::ShiftModifier:
            okToSelect = sameEntryType;
            if(typeid(*item) == typeid(StdTransactionScheduled))
              okToSelect = false;
            emit aboutToSelectItem(item, okToSelect);
            if(okToSelect) {
              // pointer 'item' might have changed. reconstruct it.
              item = itemById(id);
              unselectItems();
              selectItems(rowToIndex(m_selectAnchor->startRow()), rowToIndex(item->startRow()));
              setFocusItem(item);
            }
            break;
        }
      }
    } else if(buttonState & Qt::RightButton) {
      // if the right button is pressed then only change the
      // selection if none of the Shift/Ctrl button is pressed and
      // one of the following conditions is true:
      //
      // a) single transaction is selected
      // b) multiple transactions are selected and the one to be selected is not
      if(!(buttonState & (Qt::ShiftModifier | Qt::ControlModifier))) {
        if((cnt > 0) && (!item->isSelected())) {
          okToSelect = sameEntryType;
          emit aboutToSelectItem(item, okToSelect);
          if(okToSelect) {
            // pointer 'item' might have changed. reconstruct it.
            item = itemById(id);
            unselectItems();
            item->setSelected(true);
            setFocusItem(item);
          }
        }
        if(okToSelect)
          m_selectAnchor = item;
      }
    } else {
      // we get here when called by application logic
      emit aboutToSelectItem(item, okToSelect);
      if(okToSelect) {
        // pointer 'item' might have changed. reconstruct it.
        item = itemById(id);
        if(!dontChangeSelections)
          unselectItems();
        item->setSelected(true);
        setFocusItem(item);
        m_selectAnchor = item;
      }
    }
    if(okToSelect) {
      SelectedTransactions list(this);
      emit selectionChanged(list);
    }
  }
}

void Register::ensureItemVisible(RegisterItem* item)
{
  if(!item)
    return;

  m_ensureVisibleItem = item;
  QTimer::singleShot(0, this, SLOT(slotEnsureItemVisible()));
}

void Register::slotDoubleClicked(int row, int, int, const QPoint&)
{
  if(row >= 0 && row < m_itemIndex.size()) {
    RegisterItem* p = m_itemIndex[row];
    if(p->isSelectable()) {
      m_ignoreNextButtonRelease = true;
      // double click to start editing only works if the focus
      // item is among the selected ones
      if(!focusItem()) {
        setFocusItem(p);
        if(m_selectionMode != NoSelection)
          p->setSelected(true);
      }

      if(m_focusItem->isSelected()) {
        // don't emit the signal right away but wait until
        // we come back to the Qt main loop
        QTimer::singleShot(0, this, SIGNAL(editTransaction()));
      }
    }
  }
}

void Register::slotEnsureItemVisible(void)
{
  // if clear() has been called since the timer was
  // started, we just ignore the call
  if(!m_ensureVisibleItem)
    return;

  // make sure to catch latest changes
  bool enabled = isUpdatesEnabled();
  setUpdatesEnabled(false);
  updateRegister();
  setUpdatesEnabled(enabled);

  RegisterItem* item = m_ensureVisibleItem;
  RegisterItem* prev = item->prevItem();
  while(prev && !prev->isVisible())
    prev = prev->prevItem();
  RegisterItem* next = item->nextItem();
  while(next && !next->isVisible())
    next = next->nextItem();

  int rowPrev, rowNext;
  rowPrev = item->startRow();
  rowNext = item->startRow() + item->numRowsRegister() - 1;

  if(prev)
    rowPrev = prev->startRow();
  if(next)
    rowNext = next->startRow() + next->numRowsRegister() - 1;

  if(rowPrev < 0)
    rowPrev = 0;
  if(rowNext >= numRows())
    rowNext = numRows()-1;

  int wt = contentsY();           // window top
  int wh = visibleHeight();       // window height
  int lt = rowPos(rowPrev);       // top of line above lens
  int lb = rowPos(rowNext)+rowHeight(rowNext);       // bottom of line below lens

  // only update widget, if the transaction is not fully visible
  if(lt < wt || lb >= (wt + wh)) {
    if(rowPrev >= 0) {
      ensureCellVisible(rowPrev, 0);
    }

    ensureCellVisible(item->startRow(), 0);

    if(rowNext < numRows()) {
      ensureCellVisible(rowNext, 0);
    }
  }
}

TransactionSortField KMyMoneyRegister::textToSortOrder(const QString& text)
{
  for(int idx = 1; idx < static_cast<int>(MaxSortFields); ++idx) {
    if(text == /*i18n*/(sortOrderText[idx])) {
      return static_cast<TransactionSortField>(idx);
    }
  }
  return UnknownSort;
}

const QString KMyMoneyRegister::sortOrderToText(TransactionSortField idx)
{
  if(idx < PostDateSort || idx >= MaxSortFields)
    idx = UnknownSort;
  return /*i18n*/(sortOrderText[idx]);
}

QString Register::text(int /*row*/, int /*col*/) const
{
  return QString("a");
}

QWidget* Register::cellWidget(int row, int col) const
{
  // separated here in two if()s, because this method is called for each
  // event from QTable::eventFilter and in the most cases it is -1, -1
  if(row < 0 || col < 0)
    return 0;

  if(row > numRows() - 1 || col > numCols() - 1) {
    if(numRows() && numCols())
      qWarning("Register::cellWidget(%d,%d) out of bounds (%d,%d)", row, col, numRows(), numCols());
    return 0;
  }

  if(!m_cellWidgets.count())
    return 0;

  QWidget* w = 0;
  QPair<int, int> idx = qMakePair(row, col);
  QMap<QPair<int, int>, QWidget*>::const_iterator it_w;

  it_w = m_cellWidgets.constFind(idx);
  if(it_w != m_cellWidgets.constEnd())
    w = *it_w;
  return w;
}

void Register::insertWidget(int row, int col, QWidget* w)
{
  if(row < 0 || col < 0 || row > numRows() - 1 || col > numCols() - 1) {
    qWarning("Register::insertWidget(%d,%d) out of bounds", row, col);
    return;
  }

  QPair<int, int> idx = qMakePair(row, col);
  m_cellWidgets[idx] = w;
}

void Register::clearCellWidget(int row, int col)
{
  if(row < 0 || col < 0 || row > numRows() - 1 || col > numCols() - 1) {
    qWarning("Register::clearCellWidget(%d,%d) out of bounds", row, col);
    return;
  }

  QPair<int, int> idx = qMakePair(row, col);
  QMap<QPair<int, int>, QWidget*>::iterator it_w;

  it_w = m_cellWidgets.find(idx);
  if(it_w != m_cellWidgets.end()) {
    (*it_w)->deleteLater();
    m_cellWidgets.erase(it_w);
  }
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
  for(it = editWidgets.begin(); it != editWidgets.end(); ) {
    if((*it)->parentWidget() == this) {
      editWidgets.erase(it);
      it = editWidgets.begin();
    } else
      ++it;
  }

  // now delete the widgets
  KMyMoneyRegister::Transaction* t = dynamic_cast<KMyMoneyRegister::Transaction*>(focusItem());
  for(int row = t->startRow(); row < t->startRow() + t->numRowsRegister(true); ++row) {
    for(int col = 0; col < numCols(); ++col) {
      if(cellWidget(row, col))
        clearCellWidget(row, col);
    }
    // make sure to reduce the possibly size to what it was before editing started
    setRowHeight(row, t->rowHeightHint());
  }
}

void Register::slotToggleErronousTransactions(void)
{
  // toggle switch
  m_markErronousTransactions ^= 1;

  // check if anything needs to be redrawn
  KMyMoneyRegister::RegisterItem* p = m_firstErronous;
  while(p && p->prevItem() != m_lastErronous) {
    if(p->isErronous())
      repaintItems(p);
    p = p->nextItem();
  }

  // restart timer
  QTimer::singleShot(500, this, SLOT(slotToggleErronousTransactions()));
}

RegisterItem* Register::itemById(const QString& id) const
{
  if(id.isEmpty())
    return m_lastItem;

  for(Q3ValueVector<RegisterItem*>::size_type i = 0; i < m_items.size(); ++i) {
    RegisterItem* item = m_items[i];
    if(!item)
      continue;
    if(item->id() == id)
      return item;
  }
  return 0;
}

void Register::handleItemChange(RegisterItem* old, bool shift, bool control)
{
  if(m_selectionMode == Multi) {
    if(shift) {
      selectRange(m_selectAnchor ? m_selectAnchor : old,
                  m_focusItem, false, true, (m_selectAnchor && !control) ? true : false);
    } else if(!control) {
      selectItem(m_focusItem, false);
    }
  }
}

void Register::selectRange(RegisterItem* from, RegisterItem* to, bool invert, bool includeFirst, bool clearSel)
{
  if(!from || !to)
    return;
  if(from == to && !includeFirst)
    return;
  bool swap = false;
  if(to == from->prevItem())
    swap = true;

  RegisterItem* item;
  if(!swap && from != to && from != to->prevItem()) {
    bool found = false;
    for(item = from; item; item = item->nextItem()) {
      if(item == to) {
        found = true;
        break;
      }
    }
    if(!found)
      swap = true;
  }

  if(swap) {
    item = from;
    from = to;
    to = item;
    if(!includeFirst)
      to = to->prevItem();

  } else if(!includeFirst) {
    from = from->nextItem();
  }

  bool changed = false;
  if(clearSel) {
    for(item = firstItem(); item; item = item->nextItem()) {
      if(item->isSelected() && item->isVisible()) {
        item->setSelected(false);
        changed = true;
      }
    }
  }

  for(item = from; item; item = item->nextItem()) {
    if(item->isSelectable()) {
      if(!invert) {
        if(!item->isSelected() && item->isVisible()) {
          item->setSelected(true);
          changed = true;
        }
      } else {
        bool sel = !item->isSelected();
        if((item->isSelected() != sel) && (sel || !sel)) {
          if(item->isVisible()) {
            item->setSelected(sel);
            changed = true;
          }
        }
      }
    }
    if(item == to)
      break;
  }
}

void Register::scrollPage(int key, Qt::ButtonState state)
{
  RegisterItem* oldFocusItem = m_focusItem;

  // make sure we have a focus item
  if(!m_focusItem)
    setFocusItem(m_firstItem);
  if(!m_focusItem && m_firstItem)
    setFocusItem(m_firstItem->nextItem());
  if(!m_focusItem)
    return;

  RegisterItem* item = m_focusItem;
  int height = 0;

  switch(key) {
    case Qt::Key_PageUp:
      while(height < visibleHeight() && item->prevItem()) {
        do {
          item = item->prevItem();
          if(item->isVisible())
            height += item->rowHeightHint();
        } while((!item->isSelectable() || !item->isVisible()) && item->prevItem());
      }
      break;
    case Qt::Key_PageDown:
      while(height < visibleHeight() && item->nextItem()) {
        do {
          if(item->isVisible())
            height += item->rowHeightHint();
          item = item->nextItem();
        } while((!item->isSelectable() || !item->isVisible()) && item->nextItem());
      }
      break;

    case Qt::Key_Up:
      if(item->prevItem()) {
        do {
          item = item->prevItem();
        } while((!item->isSelectable() || !item->isVisible()) && item->prevItem());
      }
      break;

    case Qt::Key_Down:
      if(item->nextItem()) {
        do {
          item = item->nextItem();
        } while((!item->isSelectable() || !item->isVisible()) && item->nextItem());
      }
      break;

    case Qt::Key_Home:
      item = m_firstItem;
      while((!item->isSelectable() || !item->isVisible()) && item->nextItem())
        item = item->nextItem();
      break;

    case Qt::Key_End:
      item = m_lastItem;
      while((!item->isSelectable() || !item->isVisible()) && item->prevItem())
        item = item->prevItem();
      break;
  }

  // make sure to avoid selecting a possible empty transaction at the end
  Transaction* t = dynamic_cast<Transaction*>(item);
  if(t && t->transaction().id().isEmpty()) {
    if(t->prevItem()) {
      item = t->prevItem();
    }
  }

  if(!(state & Qt::ShiftModifier) || !m_selectAnchor)
    m_selectAnchor = item;

  setFocusItem(item);

  if(item->isSelectable()) {
    handleItemChange(oldFocusItem, state & Qt::ShiftModifier, state & Qt::ControlModifier);
  }

  if(m_focusItem && !m_focusItem->isSelected() && m_selectionMode == Single)
    selectItem(item);

}

void Register::keyPressEvent(QKeyEvent* ev)
{
  switch(ev->key()) {
    case Qt::Key_Space:
      if(m_selectionMode != NoSelection) {
        // get the state out of the event ...
        m_buttonState = ev->state();
        // ... and pretend that we have pressed the left mouse button ;)
        m_buttonState = static_cast<Qt::ButtonState>(m_buttonState | Qt::LeftButton);
        selectItem(m_focusItem);
      }
      break;

    case Qt::Key_PageUp:
    case Qt::Key_PageDown:
    case Qt::Key_Home:
    case Qt::Key_End:
    case Qt::Key_Down:
    case Qt::Key_Up:
      scrollPage(ev->key(), ev->state());
      break;

    default:
      Q3Table::keyPressEvent(ev);
      break;
  }
}

Transaction* Register::transactionFactory(Register *parent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId)
{
  Transaction* t = 0;
  MyMoneySplit s = split;

  if(parent->account() == MyMoneyAccount()) {
    t = new KMyMoneyRegister::StdTransaction(parent, transaction, s, uniqueId);
    return t;
  }

  switch(parent->account().accountType()) {
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
      if(s.accountId().isEmpty())
        s.setAccountId(parent->account().id());
      if(s.isMatched())
        t = new KMyMoneyRegister::StdTransactionMatched(parent, transaction, s, uniqueId);
      else if(transaction.isImported())
        t = new KMyMoneyRegister::StdTransactionDownloaded(parent, transaction, s, uniqueId);
      else
        t = new KMyMoneyRegister::StdTransaction(parent, transaction, s, uniqueId);
      break;

    case MyMoneyAccount::Investment:
      if(s.isMatched())
        t = new KMyMoneyRegister::InvestTransaction/* Matched */(parent, transaction, s, uniqueId);
      else if(transaction.isImported())
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

void Register::addGroupMarkers(void)
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

  switch(primarySortKey()) {
    case KMyMoneyRegister::PostDateSort:
    case KMyMoneyRegister::EntryDateSort:
      today = QDate::currentDate();
      thisMonth.setYMD(today.year(), today.month(), 1);
      lastMonth = thisMonth.addMonths(-1);
      yesterday = today.addDays(-1);
      // a = QDate::dayOfWeek()      todays weekday (1 = Monday, 7 = Sunday)
      // b = KLocale::weekStartDay() first day of week (1 = Monday, 7 = Sunday)
      weekStartOfs = today.dayOfWeek() - KGlobal::locale()->weekStartDay();
      if(weekStartOfs < 0) {
        weekStartOfs = 7 + weekStartOfs;
      }
      thisWeek = today.addDays(-weekStartOfs);
      lastWeek = thisWeek.addDays(-7);
      thisYear.setYMD(today.year(), 1, 1);
      if(KMyMoneyGlobalSettings::startDate().date() != QDate(1900,1,1))
        new KMyMoneyRegister::FancyDateGroupMarker(this, KMyMoneyGlobalSettings::startDate().date(), i18n("Prior transactions possibly filtered"));

      if(KMyMoneyGlobalSettings::showFancyMarker()) {
        if(m_account.lastReconciliationDate().isValid())
          new KMyMoneyRegister::StatementGroupMarker(this, KMyMoneyRegister::Deposit, m_account.lastReconciliationDate(), i18n("Last reconciliation"));

        if(!m_account.value("lastImportedTransactionDate").isEmpty()
        && !m_account.value("lastStatementBalance").isEmpty()) {
          MyMoneyMoney balance(m_account.value("lastStatementBalance"));
          if(m_account.accountGroup() == MyMoneyAccount::Liability)
            balance = -balance;
          QString txt = i18n("Online Statement Balance: %1",balance.formatMoney(m_account.fraction()));
          new KMyMoneyRegister::StatementGroupMarker(this, KMyMoneyRegister::Deposit, QDate::fromString(m_account.value("lastImportedTransactionDate"), Qt::ISODate), txt);
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
      if(KMyMoneyGlobalSettings::showFiscalMarker()) {
        QDate currentFiscalYear(QDate::currentDate().year(), KMyMoneyGlobalSettings::firstFiscalMonth(), KMyMoneyGlobalSettings::firstFiscalDay());

        if(QDate::currentDate() < currentFiscalYear)
          currentFiscalYear = currentFiscalYear.addYears(-1);
        QDate previousFiscalYear = currentFiscalYear.addYears(-1);
        new KMyMoneyRegister::FiscalYearGroupMarker(this, currentFiscalYear, i18n("Current fiscal year"));
        new KMyMoneyRegister::FiscalYearGroupMarker(this, currentFiscalYear.addYears(-1), i18n("Previous fiscal year"));
        new KMyMoneyRegister::FiscalYearGroupMarker(this, currentFiscalYear.addYears(1), i18n("Next fiscal year"));
      }
      break;

    case KMyMoneyRegister::TypeSort:
      if(KMyMoneyGlobalSettings::showFancyMarker()) {
        new KMyMoneyRegister::TypeGroupMarker(this, KMyMoneyRegister::Deposit, m_account.accountType());
        new KMyMoneyRegister::TypeGroupMarker(this, KMyMoneyRegister::Payment, m_account.accountType());
      }
      break;

    case KMyMoneyRegister::ReconcileStateSort:
      if(KMyMoneyGlobalSettings::showFancyMarker()) {
        new KMyMoneyRegister::ReconcileGroupMarker(this, MyMoneySplit::NotReconciled);
        new KMyMoneyRegister::ReconcileGroupMarker(this, MyMoneySplit::Cleared);
        new KMyMoneyRegister::ReconcileGroupMarker(this, MyMoneySplit::Reconciled);
        new KMyMoneyRegister::ReconcileGroupMarker(this, MyMoneySplit::Frozen);
      }
      break;

    case KMyMoneyRegister::PayeeSort:
      if(KMyMoneyGlobalSettings::showFancyMarker()) {
        while(p) {
          t = dynamic_cast<KMyMoneyRegister::Transaction*>(p);
          if(t) {
            list[t->sortPayee()] = 1;
          }
          p = p->nextItem();
        }
        for(it = list.constBegin(); it != list.constEnd(); ++it) {
          name = it.key();
          if(name.isEmpty()) {
            name = i18nc("Unknown payee", "Unknown");
          }
          new KMyMoneyRegister::PayeeGroupMarker(this, name);
        }
      }
      break;

    case KMyMoneyRegister::CategorySort:
      if(KMyMoneyGlobalSettings::showFancyMarker()) {
        while(p) {
          t = dynamic_cast<KMyMoneyRegister::Transaction*>(p);
          if(t) {
            list[t->sortCategory()] = 1;
          }
          p = p->nextItem();
        }
        for(it = list.constBegin(); it != list.constEnd(); ++it) {
          name = it.key();
          if(name.isEmpty()) {
            name = i18nc("Unknown category", "Unknown");
          }
          new KMyMoneyRegister::CategoryGroupMarker(this, name);
        }
      }
      break;

    case KMyMoneyRegister::SecuritySort:
      if(KMyMoneyGlobalSettings::showFancyMarker()) {
        while(p) {
          t = dynamic_cast<KMyMoneyRegister::InvestTransaction*>(p);
          if(t) {
            list[t->sortSecurity()] = 1;
          }
          p = p->nextItem();
        }
        for(it = list.constBegin(); it != list.constEnd(); ++it) {
          name = it.key();
          if(name.isEmpty()) {
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

void Register::removeUnwantedGroupMarkers(void)
{
  // remove all trailing group markers except statement markers
  KMyMoneyRegister::RegisterItem* q;
  KMyMoneyRegister::RegisterItem* p = lastItem();
  while(p) {
    q = p;
    if(dynamic_cast<KMyMoneyRegister::Transaction*>(p)
    || dynamic_cast<KMyMoneyRegister::StatementGroupMarker*>(p))
      break;

    p = p->prevItem();
    delete q;
  }

  // remove all adjacent group markers
  bool lastWasGroupMarker = false;
  p = lastItem();
  while(p) {
    q = p;
    KMyMoneyRegister::GroupMarker* m = dynamic_cast<KMyMoneyRegister::GroupMarker*>(p);
    p = p->prevItem();
    if(m) {
      m->markVisible(true);
      // make adjacent group marker invisible except those that show statement information
      if(lastWasGroupMarker && (dynamic_cast<KMyMoneyRegister::StatementGroupMarker*>(m) == 0)) {
        m->markVisible(false);
      }
      lastWasGroupMarker = true;
    } else if(q->isVisible())
      lastWasGroupMarker = false;
  }
}


#include "register.moc"

// vim:cin:si:ai:et:ts=2:sw=2:
