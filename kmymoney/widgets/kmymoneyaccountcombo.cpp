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
#include <QHeaderView>

// ----------------------------------------------------------------------------
// KDE Includes

#include "klocalizedstring.h"

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyfile.h>
#include "kmymoneyaccountcompletion.h"

AccountNamesFilterProxyModel::AccountNamesFilterProxyModel(QObject *parent)
    : AccountsFilterProxyModel(parent)
{
}

/**
  * Top items are not selectable because they are not real accounts but are only used for grouping.
  */
Qt::ItemFlags AccountNamesFilterProxyModel::flags(const QModelIndex &index) const
{
  if (!index.parent().isValid())
    return AccountsFilterProxyModel::flags(index) & ~Qt::ItemIsSelectable;
  return AccountsFilterProxyModel::flags(index);
}

/**
  * Filter all but the first column.
  */
bool AccountNamesFilterProxyModel::filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const
{
  Q_UNUSED(source_parent)
  if (source_column == 0)
    return true;
  return false;
}

class KMyMoneyAccountCombo::Private
{
public:
  Private() : m_popupView(0) {}
  QTreeView *m_popupView;
  QString    m_lastSelectedAccount;
};

KMyMoneyAccountCombo::KMyMoneyAccountCombo(AccountNamesFilterProxyModel *model, QWidget *parent/* = 0*/)
    : KComboBox(parent), d(new Private)
{
  setModel(model);
  setMinimumWidth(fontMetrics().width(QLatin1Char('W')) * 15);
}

KMyMoneyAccountCombo::KMyMoneyAccountCombo(QWidget *parent)
    : KComboBox(parent), d(new Private)
{
  setMinimumWidth(fontMetrics().width(QLatin1Char('W')) * 15);
}

KMyMoneyAccountCombo::~KMyMoneyAccountCombo()
{
  delete d;
}

void KMyMoneyAccountCombo::wheelEvent(QWheelEvent *ev)
{
  Q_UNUSED(ev)
  // don't change anything with the help of the wheel, yet (due to the tree model)
}

void KMyMoneyAccountCombo::expandAll()
{
  if (d->m_popupView)
    d->m_popupView->expandAll();
}

void KMyMoneyAccountCombo::activated()
{
  // emit the account selected signal, just like the old widget
  QVariant data = view()->currentIndex().data(AccountsModel::AccountIdRole);
  if (data.isValid()) {
    d->m_lastSelectedAccount = data.toString();
    emit accountSelected(data.toString());
  }
}

void KMyMoneyAccountCombo::setSelected(const QString& id)
{
  // find which item has this id and set is as the current item
  QModelIndexList list = model()->match(model()->index(0, 0), AccountsModel::AccountIdRole, QVariant(id), 1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive | Qt::MatchRecursive));
  if (list.count() > 0) {
    d->m_lastSelectedAccount = id;
    QModelIndex index = list.front();
    // set the current index, for this we must set the parent item as the root item
    QModelIndex oldRootModelIndex = rootModelIndex();
    setRootModelIndex(index.parent());
    setCurrentIndex(index.row());
    // restore the old root item
    setRootModelIndex(oldRootModelIndex);
    emit accountSelected(id);
  }
}

const QString& KMyMoneyAccountCombo::getSelected() const
{
  return d->m_lastSelectedAccount;
}

void KMyMoneyAccountCombo::setModel(AccountNamesFilterProxyModel *model)
{
  KComboBox::setModel(model);
  delete d->m_popupView;
  d->m_popupView = new QTreeView(this);
  setView(d->m_popupView);

  d->m_popupView->setHeaderHidden(true);
  d->m_popupView->setRootIsDecorated(false);
  d->m_popupView->setAlternatingRowColors(true);
  d->m_popupView->setAnimated(true);

  d->m_popupView->expandAll();

  connect(this, SIGNAL(activated(int)), SLOT(activated()));
}
