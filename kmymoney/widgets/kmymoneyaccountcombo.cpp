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
#include <QLineEdit>
#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class KMyMoneyAccountCombo::Private
{
public:
  Private(KMyMoneyAccountCombo* q)
    : m_q(q)
    , m_popupView(0)
  {
    m_q->setInsertPolicy(QComboBox::NoInsert);
    m_q->setMinimumWidth(m_q->fontMetrics().width(QLatin1Char('W')) * 15);
    m_q->setMaxVisibleItems(15);
  }

  KMyMoneyAccountCombo*           m_q;
  QTreeView*                      m_popupView;
  QString                         m_lastSelectedAccount;

  QString fullAccountName(const QAbstractItemModel* model, const QModelIndex& index, bool includeMainCategory = false) const;
  void selectFirstMatchingItem();
};

QString KMyMoneyAccountCombo::Private::fullAccountName(const QAbstractItemModel* model, const QModelIndex& _index, bool includeMainCategory) const
{
  QString rc;
  if(_index.isValid()) {
    QModelIndex index = _index;
    QString sep;
    do {
      rc = QString("%1%2%3").arg(model->data(index).toString()).arg(sep).arg(rc);
      sep = QLatin1String(":");
      index = index.parent();
    } while(index.isValid());

    if(!includeMainCategory) {
      QRegExp mainCategory(QString("[^%1]+%2(.*)").arg(sep).arg(sep));
      if(mainCategory.exactMatch(rc)) {
        rc = mainCategory.cap(1);
      }
    }
  }
  return rc;
}

void KMyMoneyAccountCombo::Private::selectFirstMatchingItem()
{
  for (int i = 0; i < m_q->model()->rowCount(QModelIndex()); ++i) {
    QModelIndex childIndex = m_q->model()->index(i, 0);
    if (m_q->model()->hasChildren(childIndex)) {
      // search the first leaf
      do {
        childIndex = m_q->model()->index(0, 0, childIndex);
      } while(m_q->model()->hasChildren(childIndex));

      // make it the current selection
      bool isBlocked = m_popupView->blockSignals(true);
      m_popupView->setCurrentIndex(childIndex);
      m_popupView->blockSignals(isBlocked);
      break;
    }
  }
}







KMyMoneyAccountCombo::KMyMoneyAccountCombo(QAbstractItemModel *model, QWidget *parent/* = 0*/)
  : KComboBox(parent)
  , d(new Private(this))
{
  setModel(model);
}

KMyMoneyAccountCombo::KMyMoneyAccountCombo(QWidget *parent)
  : KComboBox(parent)
  , d(new Private(this))
{
}

KMyMoneyAccountCombo::~KMyMoneyAccountCombo()
{
  delete d;
}

void KMyMoneyAccountCombo::setEditable(bool isEditable)
{
  KComboBox::setEditable(isEditable);
  // don't do the standard behavior
  if(lineEdit()) {
    lineEdit()->setObjectName("AccountComboLineEdit");
    connect(lineEdit(), SIGNAL(textEdited(QString)), this, SLOT(makeCompletion(QString)));
  }
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

void KMyMoneyAccountCombo::collapseAll()
{
  if (d->m_popupView)
    d->m_popupView->collapseAll();
}

void KMyMoneyAccountCombo::activated()
{
  QVariant data = view()->currentIndex().data(AccountsModel::AccountIdRole);
  if (data.isValid()) {
    setSelected(data.toString());
  }
}

bool KMyMoneyAccountCombo::eventFilter(QObject* o, QEvent* e)
{
  if(isEditable() && o == d->m_popupView) {
    // propagate all relevant key press events to the lineEdit widget
    if(e->type() == QEvent::KeyPress) {
      QKeyEvent* kev = static_cast<QKeyEvent*>(e);
      bool forLineEdit = (kev->text().length() > 0);
      switch(kev->key()) {
        case Qt::Key_Escape:
        case Qt::Key_Up:
        case Qt::Key_Down:
          forLineEdit = false;
          break;
        default:
          break;
      }
      if(forLineEdit) {
        return lineEdit()->event(e);
      }
    }
  }
  return KComboBox::eventFilter(o, e);
}

void KMyMoneyAccountCombo::setSelected(const QString& id)
{
  // make sure, we have all items available for search
  if(isEditable()) {
    lineEdit()->clear();
  }
  // find which item has this id and set is as the current item
  QModelIndexList list = model()->match(model()->index(0, 0), AccountsModel::AccountIdRole,
                                        QVariant(id),
                                        1,
                                        Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive | Qt::MatchRecursive));
  if (list.count() > 0) {
    d->m_lastSelectedAccount = id;
    QModelIndex index = list.front();
    QString accountName = d->fullAccountName(model(), index);

    // set the current index, for this we must set the parent item as the root item
    QModelIndex oldRootModelIndex = rootModelIndex();
    setRootModelIndex(index.parent());
    setCurrentIndex(index.row());
    if(isEditable()) {
      d->m_popupView->collapseAll();
      d->m_popupView->expand(index);
    }
    // restore the old root item
    setRootModelIndex(oldRootModelIndex);
    if(isEditable()) {
      lineEdit()->setText(accountName);
    }
    emit accountSelected(id);
  }
}

const QString& KMyMoneyAccountCombo::getSelected() const
{
  return d->m_lastSelectedAccount;
}

void KMyMoneyAccountCombo::setModel(QAbstractItemModel *model)
{
  delete d->m_popupView;

  KComboBox::setModel(model);

  AccountNamesFilterProxyModel* filterModel = qobject_cast<AccountNamesFilterProxyModel*>(model);
  if(filterModel) {
    filterModel->setFilterKeyColumn(AccountsModel::Account);
    filterModel->setFilterRole(AccountsModel::FullNameRole);
  }

  d->m_popupView = new QTreeView(this);
  d->m_popupView->setSelectionMode(QAbstractItemView::SingleSelection);
  setView(d->m_popupView);

  d->m_popupView->installEventFilter(this);

  d->m_popupView->setHeaderHidden(true);
  d->m_popupView->setRootIsDecorated(false);
  d->m_popupView->setAlternatingRowColors(true);
  d->m_popupView->setAnimated(true);

  d->m_popupView->expandAll();

  connect(this, SIGNAL(activated(int)), SLOT(activated()));
  connect(d->m_popupView, SIGNAL(activated(QModelIndex)), this, SLOT(selectItem(QModelIndex)));
  connect(d->m_popupView, SIGNAL(pressed(QModelIndex)), this, SLOT(selectItem(QModelIndex)));

  if(isEditable()) {
    connect(lineEdit(), SIGNAL(textEdited(QString)), this, SLOT(makeCompletion(QString)));
  }
}

void KMyMoneyAccountCombo::selectItem(const QModelIndex& index)
{
  if(index.isValid()) {
    setSelected(model()->data(index, AccountsModel::AccountIdRole).toString());
  }
}

void KMyMoneyAccountCombo::makeCompletion(const QString& txt)
{
  AccountNamesFilterProxyModel* filterModel = qobject_cast<AccountNamesFilterProxyModel*>(model());

  if(filterModel) {
    int cnt = 0;
    qDebug() << "Account text" << txt;
    if (txt.contains(MyMoneyFile::AccountSeperator) == 0) {
      // for some reason it helps to avoid internal errors if we
      // clear the filter before setting it to a new value
      filterModel->setFilterFixedString("");
      filterModel->setFilterRegExp(QRegExp(QString("%1%2%3").arg(".*").arg(QRegExp::escape(txt)).arg(".*"), Qt::CaseInsensitive));
    } else {
      QStringList parts = txt.split(MyMoneyFile::AccountSeperator /*, QString::SkipEmptyParts */);
      QString pattern;
      QStringList::iterator it;
      for (it = parts.begin(); it != parts.end(); ++it) {
        if (pattern.length() > 1)
          pattern += MyMoneyFile::AccountSeperator;
        pattern += QRegExp::escape(QString(*it).trimmed()) + ".*";
      }
      filterModel->setFilterFixedString("");
      filterModel->setFilterRegExp(QRegExp(pattern, Qt::CaseInsensitive));
      // if we don't have a match, we try it again, but this time
      // we add a wildcard for the top level
      if (filterModel->visibleItems() == 0) {
        pattern = pattern.prepend(QString(".*") + MyMoneyFile::AccountSeperator);
        filterModel->setFilterFixedString("");
        filterModel->setFilterRegExp(QRegExp(pattern, Qt::CaseInsensitive));
      }
    }
    const int visibleAccounts = filterModel->visibleItems();
    switch(visibleAccounts) {
      case 0:
        hidePopup();
        break;
      default:
        setMaxVisibleItems(15);
        expandAll();
        showPopup();
        d->selectFirstMatchingItem();
        break;
    }
  }
}
