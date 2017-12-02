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
                          (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kmymoneyaccountcombo.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>
#include <QKeyEvent>
#include <QTreeView>
#include <QLineEdit>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "modelenums.h"

class KMyMoneyAccountCombo::Private
{
public:
  Private(KMyMoneyAccountCombo* q)
    : m_q(q)
    , m_popupView(0)
    , m_inMakeCompletion(false)
  {
    m_q->setInsertPolicy(QComboBox::NoInsert);
    m_q->setMinimumWidth(m_q->fontMetrics().width(QLatin1Char('W')) * 15);
    m_q->setMaxVisibleItems(15);
  }

  KMyMoneyAccountCombo*           m_q;
  QTreeView*                      m_popupView;
  QString                         m_lastSelectedAccount;
  bool                            m_inMakeCompletion;

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
  if(m_popupView) {
    bool isBlocked = m_popupView->blockSignals(true);
    m_popupView->setCurrentIndex(QModelIndex());
    for (auto i = 0; i < m_q->model()->rowCount(QModelIndex()); ++i) {
      QModelIndex childIndex = m_q->model()->index(i, 0);
      if (m_q->model()->hasChildren(childIndex)) {
        // search the first leaf
        do {
          childIndex = m_q->model()->index(0, 0, childIndex);
        } while(m_q->model()->hasChildren(childIndex));

        // make it the current selection if it's selectable
        if(m_q->model()->flags(childIndex) & Qt::ItemIsSelectable) {
          m_popupView->setCurrentIndex(childIndex);
        }
        break;
      }
    }
    m_popupView->blockSignals(isBlocked);
  }
}




KMyMoneyAccountCombo::KMyMoneyAccountCombo(QSortFilterProxyModel *model, QWidget *parent)
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
}

void KMyMoneyAccountCombo::setEditable(bool isEditable)
{
  KComboBox::setEditable(isEditable);
  // don't do the standard behavior
  if(lineEdit()) {
    lineEdit()->setObjectName("AccountComboLineEdit");
    connect(lineEdit(), &QLineEdit::textEdited, this, &KMyMoneyAccountCombo::makeCompletion);
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
  QVariant data = view()->currentIndex().data((int)eAccountsModel::Role::ID);
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
    } else if(e->type() == QEvent::KeyRelease) {
      QKeyEvent* kev = static_cast<QKeyEvent*>(e);
      switch(kev->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
          activated();
          hidePopup();
          break;
      }

    } else if(e->type() == QEvent::FocusOut) {
      // if we tab out and have a selection in the popup view
      // than we use that entry completely
      activated();
      hidePopup();
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
  // find which item has this id and set it as the current item
  QModelIndexList list = model()->match(model()->index(0, 0), (int)eAccountsModel::Role::ID,
                                        QVariant(id),
                                        1,
                                        Qt::MatchFlags(Qt::MatchExactly | Qt::MatchWrap | Qt::MatchRecursive)); // CAUTION: Without Qt::MatchWrap no results for credit card, so nothing happens in ledger view

  if (!list.isEmpty()) {
    // make sure the popup is closed from here on
    hidePopup();
    d->m_lastSelectedAccount = id;
    QModelIndex index = list.front();

    if(isEditable()) {
      lineEdit()->setText(d->fullAccountName(model(), index));
    } else {
      // ensure that combobox is properly set when KMyMoneyAccountCombo::setSelected is called programmatically
      blockSignals(true);
      setRootModelIndex(index.parent());
      setCurrentIndex(index.row());
      setRootModelIndex(QModelIndex());
      blockSignals(false);
    }
    emit accountSelected(id);
  }
}

const QString& KMyMoneyAccountCombo::getSelected() const
{
  return d->m_lastSelectedAccount;
}

void KMyMoneyAccountCombo::setModel(QSortFilterProxyModel *model)
{
  // CAUTION! Assumption is being made that Account column number is always 0
  if ((int)eAccountsModel::Column::Account != 0) {
    qFatal("eAccountsModel::Column::Account must be 0 in modelenums.h");
  }

  // since we create a new popup view, we get rid of an existing one
  delete d->m_popupView;

  // call base class implementation
  KComboBox::setModel(model);

  // setup filtering criteria
  model->setFilterKeyColumn((int)eAccountsModel::Column::Account);
  model->setFilterRole((int)eAccountsModel::Role::FullName);

  // create popup view, attach model and allow to select a single item
  d->m_popupView = new QTreeView(this);
  d->m_popupView->setModel(model);
  d->m_popupView->setSelectionMode(QAbstractItemView::SingleSelection);
  setView(d->m_popupView);

  // setup view parameters
  d->m_popupView->setHeaderHidden(true);
  d->m_popupView->setRootIsDecorated(true);
  d->m_popupView->setAlternatingRowColors(true);
  d->m_popupView->setAnimated(true);

  d->m_popupView->expandAll();

  // setup signal connections
  connect(d->m_popupView, &QAbstractItemView::activated, this, &KMyMoneyAccountCombo::selectItem);

  if(isEditable()) {
    connect(lineEdit(), &QLineEdit::textEdited, this, &KMyMoneyAccountCombo::makeCompletion);
  } else {
    connect(this, static_cast<void (KComboBox::*)(int)>(&KMyMoneyAccountCombo::KComboBox::activated), this, &KMyMoneyAccountCombo::activated);
  }
}

void KMyMoneyAccountCombo::selectItem(const QModelIndex& index)
{
  if(index.isValid() && (model()->flags(index) & Qt::ItemIsSelectable)) {
    setSelected(model()->data(index, (int)eAccountsModel::Role::ID).toString());
  }
}

void KMyMoneyAccountCombo::makeCompletion(const QString& txt)
{
  if(!d->m_inMakeCompletion) {
    d->m_inMakeCompletion = true;
    AccountNamesFilterProxyModel* filterModel = qobject_cast<AccountNamesFilterProxyModel*>(model());

    if(filterModel) {
      const auto completionStr = QStringLiteral(".*");
      if (txt.contains(MyMoneyFile::AccountSeperator) == 0) {
        // for some reason it helps to avoid internal errors if we
        // clear the filter before setting it to a new value
        filterModel->setFilterFixedString(QString());
        const auto filterString = QString::fromLatin1("%1%2%3").arg(completionStr).arg(QRegExp::escape(txt)).arg(completionStr);
        filterModel->setFilterRegExp(QRegExp(filterString, Qt::CaseInsensitive));
      } else {
        QStringList parts = txt.split(MyMoneyFile::AccountSeperator /*, QString::SkipEmptyParts */);
        QString pattern;
        QStringList::iterator it;
        for (it = parts.begin(); it != parts.end(); ++it) {
          if (pattern.length() > 1)
            pattern += MyMoneyFile::AccountSeperator;
          pattern += QRegExp::escape(QString(*it).trimmed()) + completionStr;
        }
        // for some reason it helps to avoid internal errors if we
        // clear the filter before setting it to a new value
        filterModel->setFilterFixedString(QString());
        filterModel->setFilterRegExp(QRegExp(pattern, Qt::CaseInsensitive));
        // if we don't have a match, we try it again, but this time
        // we add a wildcard for the top level
        if (filterModel->visibleItems() == 0) {
          // for some reason it helps to avoid internal errors if we
          // clear the filter before setting it to a new value
          pattern = pattern.prepend(completionStr + MyMoneyFile::AccountSeperator);
          filterModel->setFilterFixedString(QString());
          filterModel->setFilterRegExp(QRegExp(pattern, Qt::CaseInsensitive));
        }
      }

      // if nothing is shown, we might as well close the popup
      switch(filterModel->visibleItems()) {
        case 0:
          hidePopup();
          break;
        default:
          setMaxVisibleItems(15);
          expandAll();
          showPopup();
          break;
      }
      d->selectFirstMatchingItem();

      // keep current text in edit widget no matter what
      bool blocked = lineEdit()->signalsBlocked();
      lineEdit()->blockSignals(true);
      lineEdit()->setText(txt);
      lineEdit()->blockSignals(blocked);
    }
    d->m_inMakeCompletion = false;
  }
}

void KMyMoneyAccountCombo::showPopup()
{
  if(d->m_popupView) {
    d->m_popupView->show();
    d->m_popupView->installEventFilter(this);
  }
  KComboBox::showPopup();
}

void KMyMoneyAccountCombo::hidePopup()
{
  if(d->m_popupView) {
    d->m_popupView->hide();
    d->m_popupView->removeEventFilter(this);
  }
  KComboBox::hidePopup();
}

// kate: space-indent on; indent-width 2; remove-trailing-space on; remove-trailing-space-save on;
