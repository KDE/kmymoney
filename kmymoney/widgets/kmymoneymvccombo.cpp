/***************************************************************************
                          kmymoneymvccombo.cpp  -  description
                             -------------------
    begin                : Sat Jan 09 2010
    copyright            : (C) 2010 by Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Cristian Onet <cristian.onet@gmail.com>
                           Alvaro Soliverez <asoliverez@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kmymoneymvccombo.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QStandardItemModel>
#include <QStandardItem>
#include <QAbstractItemView>
#include <QSortFilterProxyModel>
#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QToolButton>

// ----------------------------------------------------------------------------
// KDE Includes
#include <kapplication.h>
#include <klocale.h>
#include <kdebug.h>
#include <klineedit.h>
#include <KIconLoader>
#include <kmessagebox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneysettings.h"

class KMyMoneyMVCCombo::Private
{
public:
  Private() :
      m_canCreateObjects(false),
      m_inFocusOutEvent(false),
      m_completer(0),
      m_filterProxyModel(0) {}

  /**
    * Flag to control object creation. Use
    * KMyMoneyMVCCombo::setSuppressObjectCreation()
    * to modify it's setting. Defaults to @a false.
    */
  bool                  m_canCreateObjects;

  /**
    * Flag to check whether a focusOutEvent processing is underway or not
    */
  bool                  m_inFocusOutEvent;

  QCompleter            *m_completer;

  /**
    * Filter model used to make the completion.
    */
  QSortFilterProxyModel *m_filterProxyModel;
};


KMyMoneyMVCCombo::KMyMoneyMVCCombo(QWidget* parent) :
    KComboBox(parent),
    d(new Private)
{
  view()->setAlternatingRowColors(true);
  connect(this, SIGNAL(activated(int)), SLOT(activated(int)));
}

KMyMoneyMVCCombo::KMyMoneyMVCCombo(bool editable, QWidget* parent) :
    KComboBox(editable, parent),
    d(new Private)
{
  d->m_completer = new QCompleter(this);
  d->m_completer->setCaseSensitivity(Qt::CaseInsensitive);
  d->m_completer->setModel(model());
  setCompleter(d->m_completer);

  // setSubstringSearch(!KMyMoneySettings::stringMatchFromStart());

  view()->setAlternatingRowColors(true);
  setInsertPolicy(QComboBox::NoInsert); // don't insert new objects due to object creation
  connect(this, SIGNAL(activated(int)), SLOT(activated(int)));
}

KMyMoneyMVCCombo::~KMyMoneyMVCCombo()
{
  delete d;
}

void KMyMoneyMVCCombo::setSubstringSearch(bool enabled)
{
  if (enabled) {
    // if substring search should be turned on and
    // is already on, we can quit right away
    if (d->m_completer->model() == d->m_filterProxyModel)
      return;

    // make sure we have a proxy model
    if (!d->m_filterProxyModel) {
      d->m_filterProxyModel = new QSortFilterProxyModel(this);
      d->m_filterProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
      d->m_filterProxyModel->setSourceModel(model());
    }
    connect(this, SIGNAL(editTextChanged(QString)), this, SLOT(editTextChanged(QString)));
    d->m_completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    d->m_completer->setModel(d->m_filterProxyModel);

  } else {
    // if substring search should be turned off, we check
    // for it being turned on. In case the current completer
    // model is different from m_filterProxyModel it is not
    // active and we can quit right away
    if (d->m_completer->model() != d->m_filterProxyModel)
      return;

    disconnect(this, SIGNAL(editTextChanged(QString)), this, SLOT(editTextChanged(QString)));
    d->m_completer->setCompletionMode(QCompleter::PopupCompletion);
    d->m_completer->setModel(model());
  }
}

void KMyMoneyMVCCombo::setSubstringSearchForChildren(QWidget*const widget, bool enabled)
{
  Q_CHECK_PTR(widget);
  QList<KMyMoneyMVCCombo *> comboList;
  comboList = widget->findChildren<KMyMoneyMVCCombo *>();
  foreach (KMyMoneyMVCCombo *combo, comboList) {
    combo->setSubstringSearch(enabled);
  }
}

void KMyMoneyMVCCombo::setModel(QAbstractItemModel *model)
{
  if (!model)
    return;

  KComboBox::setModel(model);
  d->m_filterProxyModel->setSourceModel(model);
  if (d->m_completer->model() != d->m_filterProxyModel) {
    d->m_completer->setModel(model);
  }
}

void KMyMoneyMVCCombo::setClickMessage(const QString& hint) const
{
  KLineEdit* le = qobject_cast<KLineEdit*>(lineEdit());
  if (le) {
    le->setClickMessage(hint);
  }
}

const QString& KMyMoneyMVCCombo::selectedItem() const
{
  QVariant data = itemData(currentIndex());
  if (data.isValid())
    m_id = data.toString();
  else
    m_id.clear();
  return m_id;
}

void KMyMoneyMVCCombo::setSelectedItem(const QString& id)
{
  m_id = id;
  setCurrentIndex(findData(QVariant(m_id)));
}

void KMyMoneyMVCCombo::activated(int index)
{
  QVariant data = itemData(index);
  if (data.isValid()) {
    m_id = data.toString();
    emit itemSelected(m_id);
  }
}

/**
  * Use the completion prefix of the completer for filtering instead of the
  * edit text since when navigating between the completions the edit text
  * changes to the current completion making all other completions disappear.
  */
void KMyMoneyMVCCombo::editTextChanged(const QString &text)
{
  if (text.isEmpty()) {
    setCurrentIndex(0);
  } else if (d->m_filterProxyModel && completer()) {
    d->m_filterProxyModel->setFilterFixedString(completer()->completionPrefix());
  }
}

void KMyMoneyMVCCombo::connectNotify(const char* signal)
{
  if (signal && QLatin1String(signal) != QLatin1String(QMetaObject::normalizedSignature(SIGNAL(createItem(QString,QString&))))) {
    d->m_canCreateObjects = true;
  }
}

void KMyMoneyMVCCombo::disconnectNotify(const char* signal)
{
  if (signal && QLatin1String(signal) != QLatin1String(QMetaObject::normalizedSignature(SIGNAL(createItem(QString,QString&))))) {
    d->m_canCreateObjects = false;
  }
}

void KMyMoneyMVCCombo::focusOutEvent(QFocusEvent* e)
{
  // when showing m_completion we'll receive a focus out event even if the focus
  // will still remain at this widget since this widget is the completion's focus proxy
  // so ignore the focus out event caused by showin a widget of type Qt::Popup
  if (e->reason() == Qt::PopupFocusReason)
    return;

  if (d->m_inFocusOutEvent) {
    KComboBox::focusOutEvent(e);
    return;
  }

  //check if the focus went to a widget in TransactionFrom or in the Register
  if (e->reason() == Qt::MouseFocusReason) {
    QObject *w = this->parent();
    QObject *q = qApp->focusWidget()->parent();
    // KMyMoneyTagCombo is inside KTagContainer, KMyMoneyPayeeCombo isn't it
    if (w->inherits("KTagContainer"))
      w = w->parent();
    while (q && q->objectName() != "qt_scrollarea_viewport")
      q = q->parent();
    if (q != w && qApp->focusWidget()->objectName() != "register") {
      KComboBox::focusOutEvent(e);
      return;
    }
  }

  d->m_inFocusOutEvent = true;
  if (isEditable() && !currentText().isEmpty() && e->reason() != Qt::ActiveWindowFocusReason) {
    if (d->m_canCreateObjects) {
      // in case we tab out, we make sure that if the current completion
      // contains the current text that we set the current text to
      // the full completion text but only if the completion box is visible.
      // BUG 254984 is resolved with the visbility check
      if (e->reason() != Qt::MouseFocusReason) {
        if (d->m_completer->popup() && d->m_completer->popup()->isVisible()
            && d->m_completer->currentCompletion().contains(currentText(), Qt::CaseInsensitive)) {
          lineEdit()->setText(d->m_completer->currentCompletion());
        }
      }

      //check if the current text is contained in the internal list, if not ask the user if want to create a new item.
      checkCurrentText();

      // else if we cannot create objects, and the current text is not
      // in the list, then we clear the text and the selection.
    } else if (!contains(currentText())) {
      clearEditText();
    }
    //this is to cover the case when you highlight an item but don't activate it with Enter
    if (currentText() != itemText(currentIndex())) {
      setCurrentIndex(findText(currentText(), Qt::MatchExactly));
      emit activated(currentIndex());
    }
  }

  KComboBox::focusOutEvent(e);

  // force update of hint and id if there is no text in the widget
  if (isEditable() && currentText().isEmpty()) {
    QString id = m_id;
    m_id.clear();
    if (!id.isEmpty())
      emit itemSelected(m_id);
    update();
  }

  d->m_inFocusOutEvent = false;
  // This is used only be KMyMoneyTagCombo at this time
  emit lostFocus();
}

void KMyMoneyMVCCombo::checkCurrentText()
{
  if (!contains(currentText())) {
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
  }
}

void KMyMoneyMVCCombo::setCurrentTextById(const QString& id)
{
  clearEditText();
  if (!id.isEmpty()) {
    int index = findData(QVariant(id), Qt::UserRole, Qt::MatchExactly);
    if (index > -1) {
      setCompletedText(itemText(index));
      setEditText(itemText(index));
      setCurrentIndex(index);
    }
  }
}

void KMyMoneyMVCCombo::protectItem(int id, bool protect)
{
  QStandardItemModel* standardModel = qobject_cast<QStandardItemModel*> (model());
  QStandardItem* standardItem = standardModel->item(id);
  standardItem->setSelectable(!protect);
}

KMyMoneyPayeeCombo::KMyMoneyPayeeCombo(QWidget* parent) :
    KMyMoneyMVCCombo(true, parent)
{
}

void KMyMoneyPayeeCombo::loadPayees(const QList<MyMoneyPayee>& list)
{
  clear();

  //add a blank item, since the field is optional
  addItem(QString(), QVariant(QString()));

  //add all payees
  QList<MyMoneyPayee>::const_iterator it;
  for (it = list.constBegin(); it != list.constEnd(); ++it) {
    addItem((*it).name(), QVariant((*it).id()));
  }

  //sort the model, which will sort the list in the combo
  model()->sort(Qt::DisplayRole, Qt::AscendingOrder);

  //set the text to empty and the index to the first item on the list
  setCurrentIndex(0);
  clearEditText();
}

KMyMoneyTagCombo::KMyMoneyTagCombo(QWidget* parent) :
    KMyMoneyMVCCombo(true, parent)
{
}

void KMyMoneyTagCombo::loadTags(const QList<MyMoneyTag>& list)
{
  clear();

  //add a blank item, since the field is optional
  addItem(QString(), QVariant(QString()));

  //add all not closed tags
  QList<MyMoneyTag>::const_iterator it;
  for (it = list.constBegin(); it != list.constEnd(); ++it) {
    if (!(*it).isClosed())
      addItem((*it).name(), QVariant((*it).id()));
    else {
      m_closedIdList.append((*it).id());
      m_closedTagNameList.append((*it).name());
    }
  }

  //sort the model, which will sort the list in the combo
  model()->sort(Qt::DisplayRole, Qt::AscendingOrder);

  //set the text to empty and the index to the first item on the list
  setCurrentIndex(0);
  clearEditText();
}

void KMyMoneyTagCombo::setUsedTagList(QList<QString>& usedIdList, QList<QString>& usedTagNameList)
{
  m_usedIdList = usedIdList;
  m_usedTagNameList = usedTagNameList;
  for (int i = 0; i < m_usedIdList.size(); ++i) {
    int index = findData(QVariant(m_usedIdList.at(i)), Qt::UserRole, Qt::MatchExactly);
    if (index != -1) removeItem(index);
  }
}

void KMyMoneyTagCombo::checkCurrentText()
{
  if (!contains(currentText())) {
    if (m_closedTagNameList.contains(currentText())) {
      // Tell the user what's happened
      QString msg = QString("<qt>") + i18n("Closed tags can't be used.") + QString("</qt>");
      KMessageBox::information(this, msg, i18n("Closed tag"), "Closed tag");
      setCurrentText();
      return;
    } else if (m_usedTagNameList.contains(currentText())) {
      // Tell the user what's happened
      QString msg = QString("<qt>") + i18n("The tag is already present.") + QString("</qt>");
      KMessageBox::information(this, msg, i18n("Duplicate tag"), "Duplicate tag");
      setCurrentText();
      return;
    }
    QString id;
    // annouce that we go into a possible dialog to create an object
    // This can be used by upstream widgets to disable filters etc.
    emit objectCreation(true);

    emit createItem(currentText(), id);

    // Announce that we return from object creation
    emit objectCreation(false);

    // update the field to a possibly created object
    //m_id = id;
    setCurrentTextById(id);
  }
}

KTagLabel::KTagLabel(const QString& id, const QString& name, QWidget* parent) :
    QFrame(parent)
{
  QToolButton *t = new QToolButton(this);
  t->setIcon(QIcon(SmallIcon("dialog-close")));
  t->setAutoRaise(true);
  QLabel *l = new QLabel(name, this);
  m_tagId = id;
  QHBoxLayout *layout = new QHBoxLayout;
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  this->setLayout(layout);
  layout->addWidget(t);
  layout->addWidget(l);
  connect(t, SIGNAL(clicked(bool)), this, SIGNAL(clicked(bool)));
  //this->setFrameStyle(QFrame::Panel | QFrame::Plain);
}

KTagContainer::KTagContainer(QWidget* parent) :
    QWidget(parent)
{
  m_tagCombo = new KMyMoneyTagCombo;
  QHBoxLayout *layout = new QHBoxLayout;
  layout->setContentsMargins(0, 0, 5, 0);
  layout->setSpacing(0);
  layout->addWidget(m_tagCombo, 100);
  this->setLayout(layout);
  this->setFocusProxy(m_tagCombo);
  connect(m_tagCombo, SIGNAL(lostFocus()), this, SLOT(slotAddTagWidget()));
}

void KTagContainer::loadTags(const QList<MyMoneyTag>& list)
{
  m_list = list;
  m_tagCombo->loadTags(list);
}

const QList<QString> KTagContainer::selectedTags()
{
  return m_tagIdList;
}

void KTagContainer::addTagWidget(const QString& id)
{
  if (id.isNull() || m_tagIdList.contains(id))
    return;
  const QString tagName = m_tagCombo->itemText(m_tagCombo->findData(QVariant(id), Qt::UserRole, Qt::MatchExactly));
  KTagLabel *t = new KTagLabel(id, tagName, this);
  connect(t, SIGNAL(clicked(bool)), this, SLOT(slotRemoveTagWidget()));
  m_tagLabelList.append(t);
  m_tagNameList.append(tagName);
  m_tagIdList.append(id);
  this->layout()->addWidget(t);
  m_tagCombo->loadTags(m_list);
  m_tagCombo->setUsedTagList(m_tagIdList, m_tagNameList);
  m_tagCombo->setCurrentIndex(0);
  m_tagCombo->setFocus();
}

void KTagContainer::RemoveAllTagWidgets()
{
  m_tagIdList.clear();
  m_tagNameList.clear();
  while (!m_tagLabelList.isEmpty())
    delete m_tagLabelList.takeLast();
  m_tagCombo->loadTags(m_list);
  m_tagCombo->setUsedTagList(m_tagIdList, m_tagNameList);
  m_tagCombo->setCurrentIndex(0);
}

void KTagContainer::slotAddTagWidget()
{
  addTagWidget(m_tagCombo->selectedItem());
}

void KTagContainer::slotRemoveTagWidget()
{
  this->tagCombo()->setFocus();
  KTagLabel *t = (KTagLabel *)sender();
  int index = m_tagLabelList.indexOf(t);
  m_tagLabelList.removeAt(index);
  m_tagIdList.removeAt(index);
  m_tagNameList.removeAt(index);
  delete t;
  m_tagCombo->loadTags(m_list);
  m_tagCombo->setUsedTagList(m_tagIdList, m_tagNameList);
  m_tagCombo->setCurrentIndex(0);
}

KMyMoneyReconcileCombo::KMyMoneyReconcileCombo(QWidget* w) :
    KMyMoneyMVCCombo(false, w)
{
  // add the items in reverse order of appearance (see KMyMoneySelector::newItem() for details)
  addItem(i18n("Reconciled"), QVariant("R"));
  addItem(i18nc("Reconciliation state 'Cleared'", "Cleared"), QVariant("C"));
  addItem(i18n("Not reconciled"), QVariant(" "));
  addItem(" ", QVariant("U"));

  connect(this, SIGNAL(itemSelected(QString)), this, SLOT(slotSetState(QString)));
}

void KMyMoneyReconcileCombo::slotSetState(const QString& state)
{
  setSelectedItem(state);
}

void KMyMoneyReconcileCombo::removeDontCare()
{
  //Remove unknown state
  removeItem(3);
}

void KMyMoneyReconcileCombo::setState(MyMoneySplit::reconcileFlagE state)
{
  QString id;

  switch (state) {
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
      kDebug(2) << "Unknown reconcile state '" << state << "' in KMyMoneyReconcileCombo::setState()\n";
      break;
  }
  setSelectedItem(id);
}

MyMoneySplit::reconcileFlagE KMyMoneyReconcileCombo::state() const
{
  MyMoneySplit::reconcileFlagE state = MyMoneySplit::NotReconciled;

  QVariant data = itemData(currentIndex());
  QString dataVal;
  if (data.isValid())
    dataVal = data.toString();
  else
    return state;

  if (!dataVal.isEmpty()) {
    if (dataVal == "C")
      state = MyMoneySplit::Cleared;
    if (dataVal == "R")
      state = MyMoneySplit::Reconciled;
    if (dataVal == "F")
      state = MyMoneySplit::Frozen;
    if (dataVal == "U")
      state = MyMoneySplit::Unknown;
  }
  return state;
}

KMyMoneyCashFlowCombo::KMyMoneyCashFlowCombo(QWidget* w, MyMoneyAccount::accountTypeE accountType) :
    KMyMoneyMVCCombo(false, w),
    m_dir(KMyMoneyRegister::Unknown)
{
  addItem(" ", QVariant(KMyMoneyRegister::Unknown));
  if (accountType == MyMoneyAccount::Income || accountType == MyMoneyAccount::Expense) {
    // this is used for income/expense accounts to just show the reverse sense
    addItem(i18nc("Activity for income categories", "Received"), QVariant(KMyMoneyRegister::Payment));
    addItem(i18nc("Activity for expense categories", "Paid"), QVariant(KMyMoneyRegister::Deposit));
  } else {
    addItem(i18n("Pay to"), QVariant(KMyMoneyRegister::Payment));
    addItem(i18n("From"), QVariant(KMyMoneyRegister::Deposit));
  }

  connect(this, SIGNAL(itemSelected(QString)), this, SLOT(slotSetDirection(QString)));
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
  for (int i = KMyMoneyRegister::Deposit; i <= KMyMoneyRegister::Unknown; ++i) {
    num.setNum(i);
    if (num == id) {
      m_dir = static_cast<KMyMoneyRegister::CashFlowDirection>(i);
      break;
    }
  }
  emit directionSelected(m_dir);
  update();
}

void KMyMoneyCashFlowCombo::removeDontCare()
{
  removeItem(findData(QVariant(KMyMoneyRegister::Unknown), Qt::UserRole, Qt::MatchExactly));
}


KMyMoneyActivityCombo::KMyMoneyActivityCombo(QWidget* w) :
    KMyMoneyMVCCombo(false, w),
    m_activity(MyMoneySplit::UnknownTransactionType)
{
  addItem(i18nc("Finance", "Buy shares"), QVariant(MyMoneySplit::BuyShares));
  addItem(i18nc("Finance", "Sell shares"), QVariant(MyMoneySplit::SellShares));
  addItem(i18n("Dividend"), QVariant(MyMoneySplit::Dividend));
  addItem(i18n("Reinvest dividend"), QVariant(MyMoneySplit::ReinvestDividend));
  addItem(i18n("Yield"), QVariant(MyMoneySplit::Yield));
  addItem(i18nc("Finance", "Add shares"), QVariant(MyMoneySplit::AddShares));
  addItem(i18nc("Finance", "Remove shares"), QVariant(MyMoneySplit::RemoveShares));
  addItem(i18nc("Finance", "Split shares"), QVariant(MyMoneySplit::SplitShares));
  addItem(i18n("Interest Income"), QVariant(MyMoneySplit::InterestIncome));

  connect(this, SIGNAL(itemSelected(QString)), this, SLOT(slotSetActivity(QString)));
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
  for (int i = MyMoneySplit::BuyShares; i <= MyMoneySplit::InterestIncome; ++i) {
    num.setNum(i);
    if (num == id) {
      m_activity = static_cast<MyMoneySplit::investTransactionTypeE>(i);
      break;
    }
  }
  emit activitySelected(m_activity);
  update();
}

KMyMoneyGeneralCombo::KMyMoneyGeneralCombo(QWidget* w) :
    KComboBox(w)
{
  connect(this, SIGNAL(highlighted(int)), this, SLOT(slotChangeItem(int)));
}

KMyMoneyGeneralCombo::~KMyMoneyGeneralCombo()
{
}

void KMyMoneyGeneralCombo::setCurrentItem(int id)
{
  setCurrentIndex(findData(QVariant(id), Qt::UserRole, Qt::MatchExactly));
}

int KMyMoneyGeneralCombo::currentItem() const
{
  return itemData(currentIndex()).toInt();
}

void KMyMoneyGeneralCombo::clear()
{
  KComboBox::clear();
}

void KMyMoneyGeneralCombo::insertItem(const QString& txt, int id, int idx)
{
  KComboBox::insertItem(idx, txt, QVariant(id));
}

void KMyMoneyGeneralCombo::removeItem(int id)
{
  KComboBox::removeItem(findData(QVariant(id), Qt::UserRole, Qt::MatchExactly));
}

void KMyMoneyGeneralCombo::slotChangeItem(int idx)
{
  emit itemSelected(itemData(idx).toInt());
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
  insertItem(i18n("Next quarter"), MyMoneyTransactionFilter::nextQuarter);
  insertItem(i18n("Next 6 months"), MyMoneyTransactionFilter::next6Months);
  insertItem(i18n("Next 12 months"), MyMoneyTransactionFilter::next12Months);
  insertItem(i18n("Next 18 months"), MyMoneyTransactionFilter::next18Months);
  insertItem(i18n("Last 3 months to next 3 months"), MyMoneyTransactionFilter::last3ToNext3Months);
  insertItem(i18n("User defined"), MyMoneyTransactionFilter::userDefined);
}

void KMyMoneyPeriodCombo::setCurrentItem(MyMoneyTransactionFilter::dateOptionE id)
{
  if (id >= MyMoneyTransactionFilter::dateOptionCount)
    id = MyMoneyTransactionFilter::userDefined;

  KMyMoneyGeneralCombo::setCurrentItem(id);
}

MyMoneyTransactionFilter::dateOptionE KMyMoneyPeriodCombo::currentItem() const
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

MyMoneySchedule::occurrenceE KMyMoneyOccurrenceCombo::currentItem() const
{
  return static_cast<MyMoneySchedule::occurrenceE>(KMyMoneyGeneralCombo::currentItem());
}

KMyMoneyOccurrencePeriodCombo::KMyMoneyOccurrencePeriodCombo(QWidget* parent) :
    KMyMoneyOccurrenceCombo(parent)
{
  addItem(i18nc("Schedule occurrence period", MyMoneySchedule::occurrencePeriodToString(MyMoneySchedule::OCCUR_ONCE).toLatin1()), MyMoneySchedule::OCCUR_ONCE);
  addItem(i18nc("Schedule occurrence period", MyMoneySchedule::occurrencePeriodToString(MyMoneySchedule::OCCUR_DAILY).toLatin1()), MyMoneySchedule::OCCUR_DAILY);
  addItem(i18nc("Schedule occurrence period", MyMoneySchedule::occurrencePeriodToString(MyMoneySchedule::OCCUR_WEEKLY).toLatin1()), MyMoneySchedule::OCCUR_WEEKLY);
  addItem(i18nc("Schedule occurrence period", MyMoneySchedule::occurrencePeriodToString(MyMoneySchedule::OCCUR_EVERYHALFMONTH).toLatin1()), MyMoneySchedule::OCCUR_EVERYHALFMONTH);
  addItem(i18nc("Schedule occurrence period", MyMoneySchedule::occurrencePeriodToString(MyMoneySchedule::OCCUR_MONTHLY).toLatin1()), MyMoneySchedule::OCCUR_MONTHLY);
  addItem(i18nc("Schedule occurrence period", MyMoneySchedule::occurrencePeriodToString(MyMoneySchedule::OCCUR_YEARLY).toLatin1()), MyMoneySchedule::OCCUR_YEARLY);
}

KMyMoneyFrequencyCombo::KMyMoneyFrequencyCombo(QWidget* parent) :
    KMyMoneyOccurrenceCombo(parent)
{
  addItem(i18nc("Frequency of schedule", MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_ONCE).toLatin1()), MyMoneySchedule::OCCUR_ONCE);
  addItem(i18nc("Frequency of schedule", MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_DAILY).toLatin1()), MyMoneySchedule::OCCUR_DAILY);
  addItem(i18nc("Frequency of schedule", MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_WEEKLY).toLatin1()), MyMoneySchedule::OCCUR_WEEKLY);
  addItem(i18nc("Frequency of schedule", MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYOTHERWEEK).toLatin1()), MyMoneySchedule::OCCUR_EVERYOTHERWEEK);
  addItem(i18nc("Frequency of schedule", MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYHALFMONTH).toLatin1()), MyMoneySchedule::OCCUR_EVERYHALFMONTH);
  addItem(i18nc("Frequency of schedule", MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYTHREEWEEKS).toLatin1()), MyMoneySchedule::OCCUR_EVERYTHREEWEEKS);
  addItem(i18nc("Frequency of schedule", MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS).toLatin1()), MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS);
  addItem(i18nc("Frequency of schedule", MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYFOURWEEKS).toLatin1()), MyMoneySchedule::OCCUR_EVERYFOURWEEKS);
  addItem(i18nc("Frequency of schedule", MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_MONTHLY).toLatin1()), MyMoneySchedule::OCCUR_MONTHLY);
  addItem(i18nc("Frequency of schedule", MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS).toLatin1()), MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS);
  addItem(i18nc("Frequency of schedule", MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYOTHERMONTH).toLatin1()), MyMoneySchedule::OCCUR_EVERYOTHERMONTH);
  addItem(i18nc("Frequency of schedule", MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYTHREEMONTHS).toLatin1()), MyMoneySchedule::OCCUR_EVERYTHREEMONTHS);
  addItem(i18nc("Frequency of schedule", MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYFOURMONTHS).toLatin1()), MyMoneySchedule::OCCUR_EVERYFOURMONTHS);
  addItem(i18nc("Frequency of schedule", MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_TWICEYEARLY).toLatin1()), MyMoneySchedule::OCCUR_TWICEYEARLY);
  addItem(i18nc("Frequency of schedule", MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_YEARLY).toLatin1()), MyMoneySchedule::OCCUR_YEARLY);
  addItem(i18nc("Frequency of schedule", MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYOTHERYEAR).toLatin1()), MyMoneySchedule::OCCUR_EVERYOTHERYEAR);

  connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(slotCurrentDataChanged()));
}

int KMyMoneyFrequencyCombo::daysBetweenEvents() const
{
  return MyMoneySchedule::daysBetweenEvents(currentItem());
}

int KMyMoneyFrequencyCombo::eventsPerYear() const
{
  return MyMoneySchedule::eventsPerYear(currentItem());
}

QVariant KMyMoneyFrequencyCombo::currentData() const
{
  return itemData(currentIndex(), Qt::UserRole);
}

void KMyMoneyFrequencyCombo::setCurrentData(QVariant data)
{
  setItemData(currentIndex(), data, Qt::UserRole);
}

void KMyMoneyFrequencyCombo::slotCurrentDataChanged()
{
  emit currentDataChanged(currentData());
}

#include "moc_kmymoneymvccombo.cpp"
