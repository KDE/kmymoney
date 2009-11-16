/***************************************************************************
                          kmymoneycombo.cpp  -  description
                             -------------------
    begin                : Mon Mar 12 2007
    copyright            : (C) 2007 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kmymoneycombo.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QRect>
#include <QStyle>
#include <QPainter>
#include <QApplication>
//Added by qt3to4:
#include <QKeyEvent>
#include <QList>
#include <QFocusEvent>
#include <QMouseEvent>
#include <QPaintEvent>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <k3listview.h>
#include <kdebug.h>
#include <kconfig.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kconfiggroup.h>
#include "kmymoneyselector.h"
#include <kmymoneycompletion.h>
#include <kmymoneylineedit.h>
#include <mymoneysplit.h>
#include <registeritem.h>
#include <mymoneyscheduled.h>
#include "kmymoneyutils.h"

KMyMoneyCombo::KMyMoneyCombo(QWidget *w) :
  KComboBox(w),
  m_completion(0),
  m_edit(0),
  m_canCreateObjects(false),
  m_inFocusOutEvent(false)
{
}

KMyMoneyCombo::KMyMoneyCombo(bool rw, QWidget *w) :
  KComboBox(rw, w),
  m_completion(0),
  m_edit(0),
  m_canCreateObjects(false),
  m_inFocusOutEvent(false)
{
  if(rw) {
    m_edit = new kMyMoneyLineEdit(this, "combo edit");
    setLineEdit(m_edit);
  }
}

void KMyMoneyCombo::setCurrentTextById(const QString& id)
{
    setCurrentText();
    if(!id.isEmpty()) {
      Q3ListViewItem* item = selector()->item(id);
      if(item) {
        setCompletedText(item->text(0));
        setEditText(item->text(0));
      }
    }
}

void KMyMoneyCombo::slotItemSelected(const QString& id)
{
  if(isEditable()) {
    bool blocked = signalsBlocked();
    blockSignals(true);
    setCurrentTextById(id);
    blockSignals(blocked);
  }

  m_completion->hide();

  if(m_id != id) {
    m_id = id;
    emit itemSelected(id);
  }
}

void KMyMoneyCombo::setEditable(bool y)
{
  if(y == isEditable())
    return;

  KComboBox::setEditable(y);

  // make sure we use our own line edit style
  if(y) {
    m_edit = new kMyMoneyLineEdit(this, "combo edit");
    setLineEdit(m_edit);
    m_edit->setPalette(palette());
  } else {
    m_edit = 0;
  }
}

void KMyMoneyCombo::setHint(const QString& hint) const
{
  if(m_edit)
    m_edit->setHint(hint);
}

void KMyMoneyCombo::paintEvent(QPaintEvent* ev)
{
  KComboBox::paintEvent(ev);
  // if we don't have an edit field, we need to paint the text onto the button
  if(!m_edit) {
    if(m_completion) {
      QStringList list;
      selector()->selectedItems(list);
      if(!list.isEmpty()) {
        QString str = selector()->item(list[0])->text(0);
        // we only paint, if the text is longer than 1 char. Assumption
        // is that length 1 is the blank case so no need to do painting
        if(str.length() > 1) {
          QPainter p( this );
          p.setPen(palette().text().color());
          QStyleOptionComboBox opt;
          initStyleOption(&opt);
          QRect re = style()->subControlRect(QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxEditField, this);
          p.setClipRect( re );
          p.save();
          p.setFont(font());
          QFontMetrics fm(font());
          int x = re.x(), y = re.y() + fm.ascent();
          p.drawText( x, y, str );
          p.restore();
        }
      }
    }
  }
}

void KMyMoneyCombo::mousePressEvent(QMouseEvent *e)
{
  // mostly copied from QCombo::mousePressEvent() and adjusted for our needs
  if(e->button() != Qt::LeftButton)
    return;

  if(((!isEditable() || isInArrowArea(e->globalPos())) && selector()->itemList().count()) && !m_completion->isVisible()) {
    m_completion->setVisible(true);
  }

  if(m_timer.isActive()) {
    m_timer.stop();
    m_completion->slotMakeCompletion("");
  } else {
    KConfig config( "kcminputrc" );
    KConfigGroup grp = config.group("KDE");
    m_timer.setSingleShot(true);
    m_timer.start(grp.readEntry("DoubleClickInterval", 400));
  }
}

bool KMyMoneyCombo::isInArrowArea(const QPoint& pos) const
{
  QStyleOptionComboBox opt;
  initStyleOption(&opt);
  QRect arrowRect = style()->subControlRect(QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxArrow, this);

  // Correction for motif style, where arrow is smaller
  // and thus has a rect that doesn't fit the button.
  arrowRect.setHeight( qMax(  height() - (2 * arrowRect.y()), arrowRect.height() ) );

  // if the button is not isEditable, it covers the whole widget
  if(!isEditable())
    arrowRect = rect();

  return arrowRect.contains(mapFromGlobal(pos));
}

void KMyMoneyCombo::keyPressEvent(QKeyEvent* e)
{
  if((e->key() == Qt::Key_F4 && e->modifiers() == 0 ) ||
     (e->key() == Qt::Key_Down && (e->modifiers() & Qt::AltModifier)) ||
     (!isEditable() && e->key() == Qt::Key_Space)) {
    // if we have at least one item in the list, we open the dropdown
    if(selector()->listView()->firstChild())
      m_completion->setVisible(true);
    e->ignore();
    return;
  }
  KComboBox::keyPressEvent(e);
}

void KMyMoneyCombo::connectNotify(const char* signal)
{
  if(signal && QLatin1String(signal) != QLatin1String(QMetaObject::normalizedSignature(SIGNAL(createItem(const QString&,QString&))))) {
    m_canCreateObjects = true;
  }
}

void KMyMoneyCombo::disconnectNotify(const char* signal)
{
  if(signal && QLatin1String(signal) != QLatin1String(QMetaObject::normalizedSignature(SIGNAL(createItem(const QString&,QString&))))) {
    m_canCreateObjects = false;
  }
}

void KMyMoneyCombo::focusOutEvent(QFocusEvent* e)
{
  // when showing m_completion we'll receive a focus out event even if the focus
  // will still remain at this widget since this widget is the completion's focus proxy
  // so ignore the focus out event caused by showin a widget of type Qt::Popup
  if(e->reason() == Qt::PopupFocusReason)
    return;

  if(m_inFocusOutEvent) {
    KComboBox::focusOutEvent(e);
    return;
  }

  m_inFocusOutEvent = true;
  if(isEditable() && !currentText().isEmpty()) {
    if(m_canCreateObjects) {
      if(!m_completion->selector()->contains(currentText())) {
        QString id;
        // annouce that we go into a possible dialog to create an object
        // This can be used by upstream widgets to disable filters etc.
        emit objectCreation(true);

        emit createItem(currentText(), id);

        // Announce that we return from object creation
        emit objectCreation(false);

        // update the field to a possibly created object
        m_id = id;
        setCurrentTextById(id);

        // make sure the completion does not show through
        m_completion->hide();
      }

    // else if we cannot create objects, and the current text is not
    // in the list, then we clear the text and the selection.
    } else if(!m_completion->selector()->contains(currentText())) {
      setCurrentText(QString());
    }
  }

  KComboBox::focusOutEvent(e);

  // force update of hint and id if there is no text in the widget
  if(isEditable() && currentText().isEmpty()) {
    QString id = m_id;
    m_id = QString();
    if(!id.isEmpty())
      emit itemSelected(m_id);
    update();
  }
  m_inFocusOutEvent = false;
}

KMyMoneySelector* KMyMoneyCombo::selector(void) const
{
  return m_completion->selector();
}

kMyMoneyCompletion* KMyMoneyCombo::completion(void) const
{
  return m_completion;
}

void KMyMoneyCombo::selectedItem(QString& id) const
{
  id = m_id;
}

void KMyMoneyCombo::selectedItems(QStringList& list) const
{
  if(lineEdit() && lineEdit()->text().length() == 0) {
    list.clear();
  } else {
    m_completion->selector()->selectedItems(list);
  }
}

void KMyMoneyCombo::setSelectedItem(const QString& id)
{
  m_completion->selector()->setSelected(id, true);
  blockSignals(true);
  slotItemSelected(id);
  blockSignals(false);
  update();
}

QSize KMyMoneyCombo::sizeHint() const
{
  return KComboBox::sizeHint();

  // I wanted to use the code below to adjust the size of the combo box
  // according to the largest item in the selector list. Apparently that
  // does not work too well in the enter and edit schedule dialog for
  // the category combo box. So we just use the standard implementation for now.
#if 0
  constPolish();
  int i, w;
  QFontMetrics fm = fontMetrics();

  int maxW = count() ? 18 : 7 * fm.width(QChar('x')) + 18;
  int maxH = qMax( fm.lineSpacing(), 14 ) + 2;

  w = selector()->optimizedWidth();
  if ( w > maxW )
    maxW = w;

  QSize sizeHint = (style().sizeFromContents(QStyle::CT_ComboBox, this,
                 QSize(maxW, maxH)).
      expandedTo(QApplication::globalStrut()));

  return sizeHint;
#endif
}



KMyMoneyReconcileCombo::KMyMoneyReconcileCombo(QWidget* w) :
  KMyMoneyCombo(false, w)
{
  m_completion = new kMyMoneyCompletion(this);
  // connect(m_completion, SIGNAL(itemSelected(const QString&)), this, SIGNAL(itemSelected(const QString&)));

  // add the items in reverse order of appearance (see KMyMoneySelector::newItem() for details)
  // selector()->newTopItem(i18n("Frozen"), QString(), "F");
  selector()->newTopItem(i18n("Reconciled"), QString(), "R");
  selector()->newTopItem(i18nc("Reconciliation state 'Cleared'", "Cleared"), QString(), "C");
  selector()->newTopItem(i18n("Not reconciled"), QString(), " ");
  selector()->newTopItem(" ", QString(), "U");

  connect(m_completion, SIGNAL(itemSelected(const QString&)), this, SLOT(slotItemSelected(const QString&)));
  connect(this, SIGNAL(itemSelected(const QString&)), this, SLOT(slotSetState(const QString&)));
}

void KMyMoneyReconcileCombo::slotSetState(const QString& state)
{
  setSelectedItem(state);
}

void KMyMoneyReconcileCombo::removeDontCare(void)
{
  selector()->removeItem("U");
}

void KMyMoneyReconcileCombo::setState(MyMoneySplit::reconcileFlagE state)
{
  QString id;
  switch(state) {
    case MyMoneySplit::NotReconciled:
      id = ' ';
      break;
    case MyMoneySplit::Cleared:
      id = 'C';
      break;
    case MyMoneySplit::Reconciled:
      id = 'R';
      break;
    case MyMoneySplit::Frozen:
      id = 'F';
      break;
    case MyMoneySplit::Unknown:
      id = 'U';
      break;
    default:
      kDebug(2) << "Unknown reconcile state '" << state << "' in KMyMoneyComboReconcile::setState()\n";
      break;
  }
  setSelectedItem(id);
}

MyMoneySplit::reconcileFlagE KMyMoneyReconcileCombo::state(void) const
{
  MyMoneySplit::reconcileFlagE state = MyMoneySplit::NotReconciled;

  QStringList list;
  selector()->selectedItems(list);
  if(!list.isEmpty()) {
    if(list[0] == "C")
      state = MyMoneySplit::Cleared;
    if(list[0] == "R")
      state = MyMoneySplit::Reconciled;
    if(list[0] == "F")
      state = MyMoneySplit::Frozen;
    if(list[0] == "U")
      state = MyMoneySplit::Unknown;
  }
  return state;
}


KMyMoneyComboAction::KMyMoneyComboAction(QWidget* w) :
  KMyMoneyCombo(false, w)
{
  m_completion = new kMyMoneyCompletion(this);
  QString num;
  // add the items in reverse order of appearance (see KMyMoneySelector::newItem() for details)
  selector()->newTopItem(i18n("ATM"), QString(), num.setNum(KMyMoneyRegister::ActionAtm));
  selector()->newTopItem(i18n("Withdrawal"), QString(), num.setNum(KMyMoneyRegister::ActionWithdrawal));
  selector()->newTopItem(i18n("Transfer"), QString(), num.setNum(KMyMoneyRegister::ActionTransfer));
  selector()->newTopItem(i18n("Deposit"), QString(), num.setNum(KMyMoneyRegister::ActionDeposit));
  selector()->newTopItem(i18n("Cheque"), QString(), num.setNum(KMyMoneyRegister::ActionCheck));
  connect(m_completion, SIGNAL(itemSelected(const QString&)), this, SLOT(slotItemSelected(const QString&)));
  connect(this, SIGNAL(itemSelected(const QString&)), this, SLOT(slotSetAction(const QString&)));
}

void KMyMoneyComboAction::protectItem(int id, bool protect)
{
  QString num;
  selector()->protectItem(num.setNum(id), protect);
}

void KMyMoneyComboAction::slotSetAction(const QString& act)
{
  setSelectedItem(act);
  update();
  emit actionSelected(action());
}

void KMyMoneyComboAction::setAction(int action)
{
  if(action < 0 || action > 5) {
    kDebug(2) << "KMyMoneyComboAction::slotSetAction(" << action << ") invalid. Replaced with 2\n";
    action = 2;
  }
  QString act;
  act.setNum(action);
  setSelectedItem(act);
}

int KMyMoneyComboAction::action(void) const
{
  QStringList list;
  selector()->selectedItems(list);
  if(!list.isEmpty()) {
    return list[0].toInt();
  }
  kDebug(2) << "KMyMoneyComboAction::action(void): unknown selection\n";
  return 0;
}

KMyMoneyCashFlowCombo::KMyMoneyCashFlowCombo(QWidget* w, MyMoneyAccount::accountTypeE accountType) :
  KMyMoneyCombo(false, w)
{
  m_completion = new kMyMoneyCompletion(this);
  QString num;
  // add the items in reverse order of appearance (see KMyMoneySelector::newItem() for details)
  if(accountType == MyMoneyAccount::Income || accountType == MyMoneyAccount::Expense) {
    // this is used for income/expense accounts to just show the reverse sense
    selector()->newTopItem(i18nc("Activity for expense categories", "Paid"), QString(), num.setNum(KMyMoneyRegister::Deposit));
    selector()->newTopItem(i18nc("Activity for income categories", "Received"), QString(), num.setNum(KMyMoneyRegister::Payment));
  } else {
    selector()->newTopItem(i18n("From"), QString(), num.setNum(KMyMoneyRegister::Deposit));
    selector()->newTopItem(i18n("Pay to"), QString(), num.setNum(KMyMoneyRegister::Payment));
  }
  selector()->newTopItem(" ", QString(), num.setNum(KMyMoneyRegister::Unknown));
  connect(m_completion, SIGNAL(itemSelected(const QString&)), this, SLOT(slotItemSelected(const QString&)));
  connect(this, SIGNAL(itemSelected(const QString&)), this, SLOT(slotSetDirection(const QString&)));
}

void KMyMoneyCashFlowCombo::setDirection(KMyMoneyRegister::CashFlowDirection dir)
{
  m_dir = dir;
  QString num;
  setSelectedItem(num.setNum(dir));
}

void KMyMoneyCashFlowCombo::slotSetDirection(const QString& id)
{
  QString num;
  for(int i = KMyMoneyRegister::Deposit; i <= KMyMoneyRegister::Unknown; ++i) {
    num.setNum(i);
    if(num == id) {
      m_dir = static_cast<KMyMoneyRegister::CashFlowDirection>(i);
      break;
    }
  }
  emit directionSelected(m_dir);
  update();
}

void KMyMoneyCashFlowCombo::removeDontCare(void)
{
  QString num;
  selector()->removeItem(num.setNum(KMyMoneyRegister::Unknown));
}


KMyMoneyActivityCombo::KMyMoneyActivityCombo(QWidget* w) :
  KMyMoneyCombo(false, w),
  m_activity(MyMoneySplit::UnknownTransactionType)
{
  m_completion = new kMyMoneyCompletion(this);
  QString num;
  // add the items in reverse order of appearance (see KMyMoneySelector::newItem() for details)
  selector()->newTopItem(i18n("Split shares"), QString(), num.setNum(MyMoneySplit::SplitShares));
  selector()->newTopItem(i18n("Remove shares"), QString(), num.setNum(MyMoneySplit::RemoveShares));
  selector()->newTopItem(i18n("Add shares"), QString(), num.setNum(MyMoneySplit::AddShares));
  selector()->newTopItem(i18n("Yield"), QString(), num.setNum(MyMoneySplit::Yield));
  selector()->newTopItem(i18n("Reinvest dividend"), QString(), num.setNum(MyMoneySplit::ReinvestDividend));
  selector()->newTopItem(i18n("Dividend"), QString(), num.setNum(MyMoneySplit::Dividend));
  selector()->newTopItem(i18n("Sell shares"), QString(), num.setNum(MyMoneySplit::SellShares));
  selector()->newTopItem(i18n("Buy shares"), QString(), num.setNum(MyMoneySplit::BuyShares));

  connect(m_completion, SIGNAL(itemSelected(const QString&)), this, SLOT(slotItemSelected(const QString&)));
  connect(this, SIGNAL(itemSelected(const QString&)), this, SLOT(slotSetActivity(const QString&)));
}

void KMyMoneyActivityCombo::setActivity(MyMoneySplit::investTransactionTypeE activity)
{
  m_activity = activity;
  QString num;
  setSelectedItem(num.setNum(activity));
}

void KMyMoneyActivityCombo::slotSetActivity(const QString& id)
{
  QString num;
  for(int i = MyMoneySplit::BuyShares; i <= MyMoneySplit::SplitShares; ++i) {
    num.setNum(i);
    if(num == id) {
      m_activity = static_cast<MyMoneySplit::investTransactionTypeE>(i);
      break;
    }
  }
  emit activitySelected(m_activity);
  update();
}

KMyMoneyPayeeCombo::KMyMoneyPayeeCombo(QWidget* parent) :
  KMyMoneyCombo(true, parent)
{
  m_completion = new kMyMoneyCompletion(this);

  // set to ascending sort
  selector()->listView()->setSorting(0);

  connect(m_completion, SIGNAL(itemSelected(const QString&)), this, SLOT(slotItemSelected(const QString&)));
  connect(this, SIGNAL(textChanged(const QString&)), m_completion, SLOT(slotMakeCompletion(const QString&)));
}

void KMyMoneyPayeeCombo::loadPayees(const QList<MyMoneyPayee>& list)
{
  selector()->listView()->clear();
  QList<MyMoneyPayee>::const_iterator it;
  for(it = list.begin(); it != list.end(); ++it) {
    selector()->newTopItem((*it).name(), QString(), (*it).id());
  }
}


class KMyMoneyGeneralCombo::Private {
public:
  QString m_null;
  QMap<QString, int> m_strings;
  void insertItem(const QString& s, int id) { m_strings[s] = id; }

  int itemId(const QString& s) const {
    QMap<QString, int>::const_iterator it;
    it = m_strings.constFind(s);
    if(it != m_strings.constEnd())
      return *it;
    return -1;
  }

  const QString& itemText(int id) {
    QMap<QString, int>::const_iterator it;
    for(it = m_strings.constBegin(); it != m_strings.constEnd(); ++it) {
      if(*it == id) {
        return it.key();
      }
    }
    return m_null;
  }
};

KMyMoneyGeneralCombo::KMyMoneyGeneralCombo(QWidget* w) :
  KComboBox(w),
  d(new Private)
{
  connect(this, SIGNAL(highlighted(int)), this, SLOT(slotChangeItem(int)));
}

KMyMoneyGeneralCombo::~KMyMoneyGeneralCombo()
{
  delete d;
}

void KMyMoneyGeneralCombo::setItem(int id)
{
  setCurrentItem(id);
}

int KMyMoneyGeneralCombo::item(void) const
{
  return currentItem();
}

void KMyMoneyGeneralCombo::setCurrentItem(int id)
{
  const QString& txt = d->itemText(id);
  for(int idx = 0; idx < count(); ++idx) {
    if(txt == itemText(idx)) {
      KComboBox::setCurrentIndex(idx);
      break;
    }
  }
}

int KMyMoneyGeneralCombo::currentItem(void) const
{
  return d->itemId(currentText());
}

void KMyMoneyGeneralCombo::clear(void)
{
  d->m_strings.clear();
  KComboBox::clear();
}

void KMyMoneyGeneralCombo::insertItem(const QString& txt, int id, int idx)
{
  d->insertItem(txt, id);
  KComboBox::insertItem(idx, txt);
}

void KMyMoneyGeneralCombo::removeItem(int id)
{
  const QString& txt = d->itemText(id);
  for(int idx = 0; idx < count(); ++idx) {
    if(txt == itemText(idx)) {
      KComboBox::removeItem(idx);
      break;
    }
  }
}

void KMyMoneyGeneralCombo::slotChangeItem(int idx)
{
  emit itemSelected(d->itemId(itemText(idx)));
}

KMyMoneyPeriodCombo::KMyMoneyPeriodCombo(QWidget* parent) :
  KMyMoneyGeneralCombo(parent)
{
  insertItem(i18n("All dates"), MyMoneyTransactionFilter::allDates);
  insertItem(i18n("As of today"), MyMoneyTransactionFilter::asOfToday);
  insertItem(i18n("Today"), MyMoneyTransactionFilter::today);
  insertItem(i18n("Current month"), MyMoneyTransactionFilter::currentMonth);
  insertItem(i18n("Current quarter"), MyMoneyTransactionFilter::currentQuarter);
  insertItem(i18n("Current year"), MyMoneyTransactionFilter::currentYear);
  insertItem(i18n("Current fiscal year"), MyMoneyTransactionFilter::currentFiscalYear);
  insertItem(i18n("Month to date"), MyMoneyTransactionFilter::monthToDate);
  insertItem(i18n("Year to date"), MyMoneyTransactionFilter::yearToDate);
  insertItem(i18n("Year to month"), MyMoneyTransactionFilter::yearToMonth);
  insertItem(i18n("Last month"), MyMoneyTransactionFilter::lastMonth);
  insertItem(i18n("Last year"), MyMoneyTransactionFilter::lastYear);
  insertItem(i18n("Last fiscal year"), MyMoneyTransactionFilter::lastFiscalYear);
  insertItem(i18n("Last 7 days"), MyMoneyTransactionFilter::last7Days);
  insertItem(i18n("Last 30 days"), MyMoneyTransactionFilter::last30Days);
  insertItem(i18n("Last 3 months"), MyMoneyTransactionFilter::last3Months);
  insertItem(i18n("Last quarter"), MyMoneyTransactionFilter::lastQuarter);
  insertItem(i18n("Last 6 months"), MyMoneyTransactionFilter::last6Months);
  insertItem(i18n("Last 11 months"), MyMoneyTransactionFilter::last11Months);
  insertItem(i18n("Last 12 months"), MyMoneyTransactionFilter::last12Months);
  insertItem(i18n("Next 7 days"), MyMoneyTransactionFilter::next7Days);
  insertItem(i18n("Next 30 days"), MyMoneyTransactionFilter::next30Days);
  insertItem(i18n("Next 3 months"), MyMoneyTransactionFilter::next3Months);
  insertItem(i18n("Next quarter"), MyMoneyTransactionFilter::lastQuarter);
  insertItem(i18n("Next 6 months"), MyMoneyTransactionFilter::next6Months);
  insertItem(i18n("Next 12 months"), MyMoneyTransactionFilter::next12Months);
  insertItem(i18n("Last 3 months to next 3 months"), MyMoneyTransactionFilter::last3ToNext3Months);
  insertItem(i18n("User defined"), MyMoneyTransactionFilter::userDefined);
}

void KMyMoneyPeriodCombo::setCurrentItem(MyMoneyTransactionFilter::dateOptionE id)
{
  if(id >= MyMoneyTransactionFilter::dateOptionCount)
    id = MyMoneyTransactionFilter::userDefined;

  KMyMoneyGeneralCombo::setCurrentItem(id);
}

MyMoneyTransactionFilter::dateOptionE KMyMoneyPeriodCombo::currentItem(void) const
{
  return static_cast<MyMoneyTransactionFilter::dateOptionE>(KMyMoneyGeneralCombo::currentItem());
}

QDate KMyMoneyPeriodCombo::start(MyMoneyTransactionFilter::dateOptionE id)
{
  QDate start, end;
  MyMoneyTransactionFilter::translateDateRange(id, start, end);
  return start;
}

QDate KMyMoneyPeriodCombo::end(MyMoneyTransactionFilter::dateOptionE id)
{
  QDate start, end;
  MyMoneyTransactionFilter::translateDateRange(id, start, end);
  return end;
}

#if 0
void KMyMoneyPeriodCombo::dates(QDate& start, QDate& end, MyMoneyTransactionFilter::dateOptionE id)
{
}
#endif

KMyMoneyOccurrenceCombo::KMyMoneyOccurrenceCombo(QWidget* parent) :
  KMyMoneyGeneralCombo(parent)
{
}

MyMoneySchedule::occurrenceE KMyMoneyOccurrenceCombo::currentItem(void) const
{
  return static_cast<MyMoneySchedule::occurrenceE>(KMyMoneyGeneralCombo::currentItem());
}

KMyMoneyOccurrencePeriodCombo::KMyMoneyOccurrencePeriodCombo(QWidget* parent) :
  KMyMoneyOccurrenceCombo(parent)
{
  insertItem(i18n(MyMoneySchedule::occurrencePeriodToString(MyMoneySchedule::OCCUR_ONCE).toLatin1()), MyMoneySchedule::OCCUR_ONCE);
  insertItem(i18n(MyMoneySchedule::occurrencePeriodToString(MyMoneySchedule::OCCUR_DAILY).toLatin1()), MyMoneySchedule::OCCUR_DAILY);
  insertItem(i18n(MyMoneySchedule::occurrencePeriodToString(MyMoneySchedule::OCCUR_WEEKLY).toLatin1()), MyMoneySchedule::OCCUR_WEEKLY);
  insertItem(i18n(MyMoneySchedule::occurrencePeriodToString(MyMoneySchedule::OCCUR_EVERYHALFMONTH).toLatin1()), MyMoneySchedule::OCCUR_EVERYHALFMONTH);
  insertItem(i18n(MyMoneySchedule::occurrencePeriodToString(MyMoneySchedule::OCCUR_MONTHLY).toLatin1()), MyMoneySchedule::OCCUR_MONTHLY);
  insertItem(i18n(MyMoneySchedule::occurrencePeriodToString(MyMoneySchedule::OCCUR_YEARLY).toLatin1()), MyMoneySchedule::OCCUR_YEARLY);
}

KMyMoneyFrequencyCombo::KMyMoneyFrequencyCombo(QWidget* parent) :
  KMyMoneyOccurrenceCombo(parent)
{
  insertItem(i18n(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_ONCE).toLatin1()), MyMoneySchedule::OCCUR_ONCE);
  insertItem(i18n(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_DAILY).toLatin1()), MyMoneySchedule::OCCUR_DAILY);
  insertItem(i18n(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_WEEKLY).toLatin1()), MyMoneySchedule::OCCUR_WEEKLY);
  insertItem(i18n(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYOTHERWEEK).toLatin1()), MyMoneySchedule::OCCUR_EVERYOTHERWEEK);
  insertItem(i18n(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYHALFMONTH).toLatin1()), MyMoneySchedule::OCCUR_EVERYHALFMONTH);
  insertItem(i18n(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYTHREEWEEKS).toLatin1()), MyMoneySchedule::OCCUR_EVERYTHREEWEEKS);
  insertItem(i18n(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS).toLatin1()), MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS);
  insertItem(i18n(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYFOURWEEKS).toLatin1()), MyMoneySchedule::OCCUR_EVERYFOURWEEKS);
  insertItem(i18n(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_MONTHLY).toLatin1()), MyMoneySchedule::OCCUR_MONTHLY);
  insertItem(i18n(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS).toLatin1()), MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS);
  insertItem(i18n(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYOTHERMONTH).toLatin1()), MyMoneySchedule::OCCUR_EVERYOTHERMONTH);
  insertItem(i18n(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYTHREEMONTHS).toLatin1()), MyMoneySchedule::OCCUR_EVERYTHREEMONTHS);
  insertItem(i18n(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYFOURMONTHS).toLatin1()), MyMoneySchedule::OCCUR_EVERYFOURMONTHS);
  insertItem(i18n(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_TWICEYEARLY).toLatin1()), MyMoneySchedule::OCCUR_TWICEYEARLY);
  insertItem(i18n(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_YEARLY).toLatin1()), MyMoneySchedule::OCCUR_YEARLY);
  insertItem(i18n(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYOTHERYEAR).toLatin1()), MyMoneySchedule::OCCUR_EVERYOTHERYEAR);
}

int KMyMoneyFrequencyCombo::daysBetweenEvents(void) const
{
  return MyMoneySchedule::daysBetweenEvents(currentItem());
}

int KMyMoneyFrequencyCombo::eventsPerYear(void) const
{
  return MyMoneySchedule::eventsPerYear(currentItem());
}
#include "kmymoneycombo.moc"
