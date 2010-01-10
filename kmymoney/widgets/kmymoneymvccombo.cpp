/***************************************************************************
                          kmymoneycombo.cpp  -  description
                             -------------------
    begin                : Sat Jan 09 2010
    copyright            : (C) 2010 by Thomas Baumgart
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

#include "kmymoneymvccombo.h"

// ----------------------------------------------------------------------------
// QT Includes

/*#include <QRect>
#include <QStyle>
#include <QPainter>
#include <QApplication>
#include <QKeyEvent>
#include <QList>
#include <QFocusEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QSortFilterProxyModel>
#include <QCompleter>*/

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kdebug.h>
/*
#include <k3listview.h>

#include <kconfig.h>*/

// ----------------------------------------------------------------------------
// Project Includes

/*#include <kconfiggroup.h>
#include "kmymoneyselector.h"
#include <kmymoneycompletion.h>
#include <kmymoneylineedit.h>
#include <mymoneysplit.h>
#include <registeritem.h>
#include <mymoneyscheduled.h>
#include "kmymoneyutils.h"*/


KMyMoneyMVCCombo::KMyMoneyMVCCombo(QWidget* parent) :
  KComboBox(parent),
  m_canCreateObjects(false),
  m_inFocusOutEvent(false)
{
  QCompleter *completer = new QCompleter(this);
  setCompleter(completer);
}

KMyMoneyMVCCombo::KMyMoneyMVCCombo(bool editable, QWidget* parent) :
  KComboBox(editable, parent),
  m_canCreateObjects(false),
  m_inFocusOutEvent(false)
{
  QCompleter *completer = new QCompleter(this);
  completer->setCaseSensitivity(Qt::CaseInsensitive);
  completer->setCompletionMode(QCompleter::PopupCompletion);
  completer->setModel(model());
  setCompleter(completer);
  setInsertPolicy(QComboBox::NoInsert); // don't instert new objects due to object creation
  connect(this, SIGNAL(activated(int)), SLOT(activated(int)));
}

void KMyMoneyMVCCombo::setHint(const QString& hint) const
{
  Q_UNUSED(hint);
}

const QString& KMyMoneyMVCCombo::selectedItem(void) const
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

void KMyMoneyMVCCombo::connectNotify(const char* signal)
{
  if(signal && QLatin1String(signal) != QLatin1String(QMetaObject::normalizedSignature(SIGNAL(createItem(const QString&,QString&))))) {
    m_canCreateObjects = true;
  }
}

void KMyMoneyMVCCombo::disconnectNotify(const char* signal)
{
  if(signal && QLatin1String(signal) != QLatin1String(QMetaObject::normalizedSignature(SIGNAL(createItem(const QString&,QString&))))) {
    m_canCreateObjects = false;
  }
}

void KMyMoneyMVCCombo::focusOutEvent(QFocusEvent* e)
{
  if(m_inFocusOutEvent) {
    KComboBox::focusOutEvent(e);
    return;
  }

  m_inFocusOutEvent = true;
  if(isEditable() && !currentText().isEmpty()) {
    if(m_canCreateObjects) {
      if(!contains(currentText())) {
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
        //m_completion->hide();
      }

    // else if we cannot create objects, and the current text is not
    // in the list, then we clear the text and the selection.
    } else if(!contains(currentText())) {
      clearEditText();
    }
  }

  KComboBox::focusOutEvent(e);

  // force update of hint and id if there is no text in the widget
  if(isEditable() && currentText().isEmpty()) {
    QString id = m_id;
    m_id.clear();
    if(!id.isEmpty())
      emit itemSelected(m_id);
    update();
  }

  m_inFocusOutEvent = false;
}

void KMyMoneyMVCCombo::setCurrentTextById(const QString& id)
{
    setCurrentText();
    if(!id.isEmpty()) {
      int index = findData(QVariant(id), Qt::UserRole, Qt::MatchExactly);
      if(index > -1 ) {
        setCompletedText(itemText(index));
        setEditText(itemText(index));
      }
    }
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
  for(it = list.constBegin(); it != list.constEnd(); ++it) {
    addItem((*it).name(),QVariant((*it).id()));
  }

  //sort the model, which will sort the list in the combo
  model()->sort(Qt::DisplayRole, Qt::AscendingOrder);

  //set the text to empty and the index to the first item on the list
  setCurrentIndex(0);
  clearEditText();
}

KMyMoneyReconcileCombo::KMyMoneyReconcileCombo(QWidget* w) :
  KMyMoneyMVCCombo(false, w)
{
  // add the items in reverse order of appearance (see KMyMoneySelector::newItem() for details)
  addItem(i18n("Reconciled"), QVariant("R"));
  addItem(i18nc("Reconciliation state 'Cleared'", "Cleared"), QVariant("C"));
  addItem(i18n("Not reconciled"), QVariant(" "));
  addItem(" ", QVariant("U"));

  connect(this, SIGNAL(itemSelected(const QString&)), this, SLOT(slotSetState(const QString&)));
}

void KMyMoneyReconcileCombo::slotSetState(const QString& state)
{
  setSelectedItem(state);
}

void KMyMoneyReconcileCombo::removeDontCare(void)
{
  //Remove unknown state
  removeItem(3);
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
      kDebug(2) << "Unknown reconcile state '" << state << "' in KMyMoneyReconcileCombo::setState()\n";
      break;
  }
  setSelectedItem(id);
}

MyMoneySplit::reconcileFlagE KMyMoneyReconcileCombo::state(void) const
{
  MyMoneySplit::reconcileFlagE state = MyMoneySplit::NotReconciled;

  QVariant data = itemData(currentIndex());
  QString dataVal;
  if (data.isValid())
    dataVal = data.toString();
  else
    return state;

  if(!dataVal.isEmpty()) {
    if(dataVal == "C")
      state = MyMoneySplit::Cleared;
    if(dataVal == "R")
      state = MyMoneySplit::Reconciled;
    if(dataVal == "F")
      state = MyMoneySplit::Frozen;
    if(dataVal == "U")
      state = MyMoneySplit::Unknown;
  }
  return state;
}

#include "kmymoneymvccombo.moc"
