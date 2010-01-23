/***************************************************************************
                         kmymoneyaccountbutton  -  description
                            -------------------
   begin                : Mon May 31 2004
   copyright            : (C) 2000-2004 by Michael Edwardes
   email                : mte@users.sourceforge.net
                          Javier Campos Morales <javi_c@users.sourceforge.net>
                          Felix Rodriguez <frodriguez@users.sourceforge.net>
                          John C <thetacoturtle@users.sourceforge.net>
                          Thomas Baumgart <ipwizard@users.sourceforge.net>
                          Kevin Tambascio <ktambascio@users.sourceforge.net>
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

#include "kmymoneyaccountcombo.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPainter>
#include <QStyle>
#include <QApplication>
#include <QMouseEvent>
#include <QList>
#include <QKeyEvent>
#include <QTreeView>


// ----------------------------------------------------------------------------
// KDE Includes

#include "klocalizedstring.h"

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyfile.h>
#include "kmymoneyaccountcompletion.h"
#include "models/accountsmodel.h"

class KMyMoneyMVCAccountCombo::Private
{
public:
  Private() : m_popupView(0) {}
  QTreeView *m_popupView;
};

KMyMoneyMVCAccountCombo::KMyMoneyMVCAccountCombo(QWidget* parent/* = 0*/)
    : KComboBox(parent), d(new Private)
{
}

KMyMoneyMVCAccountCombo::~KMyMoneyMVCAccountCombo()
{
}

void KMyMoneyMVCAccountCombo::wheelEvent(QWheelEvent *ev)
{
  Q_UNUSED(ev)
  // don't change anything with the help of the wheel, yet (due to the tree model)
}

void KMyMoneyMVCAccountCombo::modelWasSet()
{
  // OK, now that the model was set we are ready to set-up the completer view
  delete d->m_popupView;
  d->m_popupView = new QTreeView(this);
  setView(d->m_popupView);

  d->m_popupView->setHeaderHidden(true);
  d->m_popupView->setRootIsDecorated(false);
  d->m_popupView->setAlternatingRowColors(true);
  d->m_popupView->setAnimated(true);

  d->m_popupView->expandAll();

  connect(this, SIGNAL(activated(int)), SLOT(activated()));

  // set the widget's size, just like the old widget
  QFontMetrics fm(font());
  setMinimumWidth(fm.maxWidth()*15);
}

void KMyMoneyMVCAccountCombo::expandAll()
{
  if (d->m_popupView)
    d->m_popupView->expandAll();
}

void KMyMoneyMVCAccountCombo::activated()
{
  // emit the account selected signal, just like the old widget
  QVariant data = model()->data(view()->currentIndex(), AccountIdRole);
  if (data.isValid()) {
    emit accountSelected(data.toString());
  }
}

void KMyMoneyMVCAccountCombo::setSelected(const QString& id)
{
  // find which item has this id and set is as the current item
  QModelIndexList list = model()->match(model()->index(0, 0), AccountIdRole, QVariant(id), 1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive | Qt::MatchRecursive));
  if (list.count() > 0) {
    QModelIndex index = list.front();
    // set the current index, for this we must set the parent item as the root item
    QModelIndex oldRootModelIndex = rootModelIndex();
    setRootModelIndex(index.parent());
    setCurrentIndex(index.row());
    // restore the old root item
    setRootModelIndex(oldRootModelIndex);
  }
}

KMyMoneyAccountCombo::KMyMoneyAccountCombo(QWidget* parent) :
    KComboBox(parent),
    m_completion(0),
    m_mlbDown(false)
{
  m_completion = new kMyMoneyAccountCompletion(this);

  connect(this, SIGNAL(clicked()), this, SLOT(slotButtonPressed()));
  connect(m_completion, SIGNAL(itemSelected(const QString&)), this, SLOT(slotSelected(const QString&)));

  // make sure that we can display a minimum of characters
  QFontMetrics fm(font());
  setMinimumWidth(fm.maxWidth()*15);
  setMaximumHeight(height());

  // we only use this one item and replace the text as we have our own dropdown box
  insertItem(0, QString());
}

KMyMoneyAccountCombo::~KMyMoneyAccountCombo()
{
}

void KMyMoneyAccountCombo::slotButtonPressed(void)
{
  m_completion->setVisible(true);
}

void KMyMoneyAccountCombo::slotSelected(const QString& id)
{
  try {
    MyMoneyAccount acc = MyMoneyFile::instance()->account(id);
    setText(acc.name());
    emit accountSelected(id);
  } catch (MyMoneyException *e) {
    delete e;
  }
}

void KMyMoneyAccountCombo::setSelected(const QString& id)
{
  if (!id.isEmpty()) {
    try {
      MyMoneyAccount acc = MyMoneyFile::instance()->account(id);
      setSelected(acc);
    } catch (MyMoneyException *e) {
      qDebug("Account '%s' not found in %s(%d)", qPrintable(id), __FILE__, __LINE__);
      delete e;
    }
  } else {
    setText(QString());
    m_completion->setSelected(id);
  }
}

void KMyMoneyAccountCombo::setSelected(const MyMoneyAccount& acc)
{
  m_completion->setSelected(acc.id());
  setText(acc.name());
}

void KMyMoneyAccountCombo::setText(const QString& txt)
{
  setItemText(currentIndex(), txt);
}

int KMyMoneyAccountCombo::loadList(const QString& baseName, const QList<QString>& accountIdList, const bool clear)
{
  AccountSet set;

  return set.load(m_completion->selector(), baseName, accountIdList, clear);
}

int KMyMoneyAccountCombo::loadList(KMyMoneyUtils::categoryTypeE typeMask)
{
  AccountSet set;
  QList<int> typeList;

  if (typeMask & KMyMoneyUtils::asset) {
    set.addAccountGroup(MyMoneyAccount::Asset);
  }
  if (typeMask & KMyMoneyUtils::liability) {
    set.addAccountGroup(MyMoneyAccount::Liability);
  }
  if (typeMask & KMyMoneyUtils::income) {
    set.addAccountGroup(MyMoneyAccount::Income);
  }
  if (typeMask & KMyMoneyUtils::expense) {
    set.addAccountGroup(MyMoneyAccount::Expense);
  }

  return set.load(m_completion->selector());
}

int KMyMoneyAccountCombo::loadList(MyMoneyAccount::accountTypeE type)
{
  AccountSet set;

  set.addAccountType(type);

  return set.load(m_completion->selector());
}

void KMyMoneyAccountCombo::keyPressEvent(QKeyEvent* k)
{
  switch (k->key()) {
  case Qt::Key_Tab:
    break;

  case Qt::Key_Space:
    emit clicked();
    break;

  default:
    break;
  }
  return;
}

void KMyMoneyAccountCombo::mousePressEvent(QMouseEvent *e)
{
  if (e->button() != Qt::LeftButton) {
    e->ignore();
    return;
  }
  bool hit = rect().contains(e->pos());
  if (hit) {                                  // mouse press on button
    m_mlbDown = true;                         // left mouse button down
    emit pressed();
  }
}

void KMyMoneyAccountCombo::mouseReleaseEvent(QMouseEvent *e)
{
  if (e->button() != Qt::LeftButton) {
    e->ignore();
    return;
  }
  if (!m_mlbDown)
    return;
  m_mlbDown = false;                            // left mouse button up
  emit released();
  if (rect().contains(e->pos())) {                  // mouse release on button
    emit clicked();
  }
}

int KMyMoneyAccountCombo::count(void) const
{
  return m_completion->selector()->accountList().count();
}

QStringList KMyMoneyAccountCombo::accountList(const QList<MyMoneyAccount::accountTypeE>& list) const
{
  return m_completion->selector()->accountList(list);
}

int KMyMoneyAccountCombo::loadList(const QList<int>& list)
{
  // FIXME make the caller construct the AccountSet directly
  AccountSet set;
  QList<int>::const_iterator it;
  for (it = list.begin(); it != list.end(); ++it) {
    set.addAccountType(static_cast<MyMoneyAccount::accountTypeE>(*it));
  }
  return set.load(m_completion->selector());
}

QStringList KMyMoneyAccountCombo::selectedAccounts(void) const
{
  QStringList list;
  if (m_completion)
    m_completion->selector()->selectedItems(list);
  return list;
}

#include "kmymoneyaccountcombo.moc"
