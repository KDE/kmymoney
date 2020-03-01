/*
 * Copyright 2004-2020  Thomas Baumgart <tbaumgart@kde.org>
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

#include "kmymoneyaccountcombo.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>
#include <QEvent>
#include <QKeyEvent>
#include <QTreeView>
#include <QLineEdit>
#include <QAction>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "mymoneyenums.h"
#include "accountsmodel.h"
#include "icons.h"

class KMyMoneyAccountCombo::Private
{
public:
  Private(KMyMoneyAccountCombo* q)
    : m_q(q)
    , m_popupView(nullptr)
    , m_splitAction(nullptr)
    , m_inMakeCompletion(false)
  {
    m_q->setInsertPolicy(QComboBox::NoInsert);
    m_q->setMinimumWidth(m_q->fontMetrics().width(QLatin1Char('W')) * 15);
    m_q->setMaxVisibleItems(15);
  }

  KMyMoneyAccountCombo*           m_q;
  QTreeView*                      m_popupView;
  QAction*                        m_splitAction;
  QString                         m_lastSelectedAccount;
  bool                            m_inMakeCompletion;

  void selectFirstMatchingItem()
  {
    if(m_popupView) {
      QSignalBlocker blocker(m_popupView);
      m_popupView->setCurrentIndex(QModelIndex());
      const auto rows = m_q->model()->rowCount();
      for (auto i = 0; i < rows; ++i) {
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
    }
  }

  void showSplitAction(bool show)
  {
    if (show && !m_splitAction) {
      m_splitAction = m_q->lineEdit()->addAction(Icons::get(Icons::Icon::Split), QLineEdit::TrailingPosition);
      // this for some reason does not work and I had to
      // add logic to the eventFilter below to catch this
      // key-sequence. Left it here, since it does not hurt either.
      m_splitAction->setShortcut(QKeySequence(Qt::Key_Control, Qt::Key_Space));
      m_q->connect(m_splitAction, &QAction::triggered, m_q, &KMyMoneyAccountCombo::splitDialogRequest);

    } else if(!show && m_splitAction) {
      m_splitAction->deleteLater();
      m_splitAction = nullptr;
    }
  }
};





KMyMoneyAccountCombo::KMyMoneyAccountCombo(QSortFilterProxyModel *model, QWidget *parent)
  : KComboBox(parent)
  , d(new Private(this))
{
  init();
  setModel(model);
}

KMyMoneyAccountCombo::KMyMoneyAccountCombo(QWidget *parent)
  : KComboBox(parent)
  , d(new Private(this))
{
  init();
}

void KMyMoneyAccountCombo::init()
{
  setObjectName("ComboBox");
  setMaxVisibleItems(15);
  setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
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
    lineEdit()->setClearButtonEnabled(true);
    connect(lineEdit(), &QLineEdit::textEdited, this, &KMyMoneyAccountCombo::makeCompletion, Qt::UniqueConnection);
    installEventFilter(this);
    d->showSplitAction(true);
  }
}

void KMyMoneyAccountCombo::setSplitActionVisible(bool show)
{
  if (lineEdit()) {
    d->showSplitAction(show);
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
  auto accountId = view()->currentIndex().data(eMyMoney::Model::Roles::IdRole).toString();
  if (!accountId.isEmpty() && !(lineEdit() && lineEdit()->text().isEmpty())) {
    selectItem(view()->currentIndex());
  }
}

bool KMyMoneyAccountCombo::eventFilter(QObject* o, QEvent* e)
{
  if(isEditable()) {
    if (o == d->m_popupView) {
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
          case Qt::Key_Escape:
            hidePopup();
            return true;

          case Qt::Key_Enter:
          case Qt::Key_Return:
            activated();
            hidePopup();
            break;
          default:
            break;
        }

      } else if(e->type() == QEvent::FocusOut) {
        // if we tab out and have a selection in the popup view
        // than we use that entry completely
        activated();
        hidePopup();
      }
    } else if(o == this) {
      if(e->type() == QEvent::KeyPress) {
        const auto kev = static_cast<QKeyEvent*>(e);
        if (kev->modifiers() & Qt::ControlModifier && kev->key() == Qt::Key_Space) {
          emit splitDialogRequest();
          return true;
        }
      }
    }
  }
  return KComboBox::eventFilter(o, e);
}

void KMyMoneyAccountCombo::setSelected(const QString& id)
{
  if (id.isEmpty()) {
    d->m_lastSelectedAccount.clear();
    emit accountSelected(id);
    return;
  }

  if (id == d->m_lastSelectedAccount) {
    // nothing to do
    return;
  }

  // make sure, we have all items available for search
  if(isEditable()) {
    lineEdit()->clear();
  }

  // reset the filter of the model
  auto* filterModel = qobject_cast<QSortFilterProxyModel*>(model());
  filterModel->setFilterFixedString(QString());

  // find which item has this id and set it as the current item
  // and we always skip over the favorite section
  int startRow = model()->index(0, 0).data(eMyMoney::Model::Roles::IdRole).toString() == MyMoneyAccount::stdAccName(eMyMoney::Account::Standard::Favorite) ? 1 : 0;
  // Note: Without Qt::MatchWrap we might not get results for credit card
  const auto list = model()->match(model()->index(startRow, 0), eMyMoney::Model::Roles::IdRole,
                                   QVariant(id),
                                   1,
                                   Qt::MatchFlags(Qt::MatchExactly | Qt::MatchWrap | Qt::MatchRecursive));

  if (!list.isEmpty()) {
    // make sure the popup is closed from here on
    hidePopup();
    d->m_lastSelectedAccount = id;
    const auto idx = list.front();

    QSignalBlocker blocker(this);
    setRootModelIndex(idx.parent());
    setCurrentIndex(idx.row());
    setRootModelIndex(QModelIndex());
    blocker.unblock();

    if(isEditable()) {
      lineEdit()->setText(idx.data(eMyMoney::Model::AccountFullNameRole).toString());
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
  // CAUTION! Assumption is being made that AccountName column number is always 0
  if (AccountsModel::Column::AccountName != 0) {
    qFatal("AccountsModel::Column::AccountName must be 0 in modelenums.h");
  }

  // since we create a new popup view, we get rid of an existing one
  delete d->m_popupView;

  // call base class implementation
  KComboBox::setModel(model);

  // setup filtering criteria
  model->setFilterKeyColumn(AccountsModel::Column::AccountName);
  model->setFilterRole(eMyMoney::Model::Roles::AccountFullHierarchyNameRole);

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
  connect(d->m_popupView, &QTreeView::activated, this, &KMyMoneyAccountCombo::selectItem);

  // for some unknown reason, the first selection with the mouse (not with the keyboard)
  // after the qlineedit had been cleared using the clear button does not trigger the
  // activated() signal of d->m_popupView. This is a workaround to catch this scenario
  // and still get valid settings.
  connect(this, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&]() {
    const auto idx = d->m_popupView->currentIndex();
    if (d->m_lastSelectedAccount.isEmpty() && idx.isValid()) {
      selectItem(idx);
    }
  });

  if(isEditable()) {
    connect(lineEdit(), &QLineEdit::textEdited, this, &KMyMoneyAccountCombo::makeCompletion, Qt::UniqueConnection);
  } else {
    connect(this, static_cast<void (KComboBox::*)(int)>(&KMyMoneyAccountCombo::KComboBox::activated), this, &KMyMoneyAccountCombo::activated);
  }
}

void KMyMoneyAccountCombo::selectItem(const QModelIndex& index)
{
  if (d->m_inMakeCompletion)
    return;

  if (index.model() != model()) {
    qDebug() << "KMyMoneyAccountCombo::selectItem called with wrong model";
  }
  if(index.isValid() && (index.model()->flags(index) & Qt::ItemIsSelectable)) {
    // delay the call until the next time in the event loop
    QMetaObject::invokeMethod(this, "setSelected", Qt::QueuedConnection, Q_ARG(QString, index.data(eMyMoney::Model::Roles::IdRole).toString()));
  }
}

void KMyMoneyAccountCombo::makeCompletion(const QString& txt)
{
  if(!d->m_inMakeCompletion) {
    d->m_inMakeCompletion = true;
    if (txt.isEmpty()) {
      d->m_lastSelectedAccount.clear();
      d->m_popupView->selectionModel()->clearSelection();
      d->m_popupView->setCurrentIndex(QModelIndex());
      setRootModelIndex(QModelIndex());
      setCurrentIndex(-1);
    } else {
      AccountNamesFilterProxyModel* filterModel = qobject_cast<AccountNamesFilterProxyModel*>(model());

      if(filterModel) {
        const auto completionStr = QStringLiteral(".*");
        // for some reason it helps to avoid internal errors if we
        // clear the filter before setting it to a new value
        filterModel->setFilterFixedString(QString());
        if (txt.contains(MyMoneyFile::AccountSeparator) == 0) {
          const auto filterString = QString::fromLatin1("%1%2%3").arg(completionStr).arg(QRegExp::escape(txt)).arg(completionStr);
          filterModel->setFilterRegExp(QRegExp(filterString, Qt::CaseInsensitive));
        } else {
          QStringList parts = txt.split(MyMoneyFile::AccountSeparator /*, QString::SkipEmptyParts */);
          QString pattern;
          QStringList::iterator it;
          for (it = parts.begin(); it != parts.end(); ++it) {
            if (pattern.length() > 1)
              pattern += MyMoneyFile::AccountSeparator;
            pattern += QRegExp::escape(QString(*it).trimmed()) + completionStr;
          }
          filterModel->setFilterRegExp(QRegExp(pattern, Qt::CaseInsensitive));
          // if we don't have a match, we try it again, but this time
          // we add a wildcard for the top level
          if (filterModel->visibleItems() == 0) {
            // for some reason it helps to avoid internal errors if we
            // clear the filter before setting it to a new value
            filterModel->setFilterFixedString(QString());
            pattern = pattern.prepend(completionStr + MyMoneyFile::AccountSeparator);
            filterModel->setFilterRegExp(QRegExp(pattern, Qt::CaseInsensitive));
          }
        }

        // if nothing is shown, we might as well close the popup
        switch(filterModel->visibleItems()) {
          case 0:
            hidePopup();
            break;
          default:
            expandAll();
            showPopup();
            d->selectFirstMatchingItem();
            break;
        }

        // keep current text in edit widget no matter what
        QSignalBlocker blocker(lineEdit());
        lineEdit()->setText(txt);
      }
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

class KMyMoneyAccountComboSplitHelperPrivate
{
  Q_DISABLE_COPY(KMyMoneyAccountComboSplitHelperPrivate)
  Q_DECLARE_PUBLIC(KMyMoneyAccountComboSplitHelper)

public:
  KMyMoneyAccountComboSplitHelperPrivate(KMyMoneyAccountComboSplitHelper* qq)
  : q_ptr(qq)
  , m_accountCombo(nullptr)
  , m_splitModel(nullptr)
  , m_norecursive(false)
  {
  }

  KMyMoneyAccountComboSplitHelper*  q_ptr;
  QComboBox*                        m_accountCombo;
  QAbstractItemModel*               m_splitModel;
  bool                              m_norecursive;
};


KMyMoneyAccountComboSplitHelper::KMyMoneyAccountComboSplitHelper(QComboBox* accountCombo, QAbstractItemModel* model)
  : QObject(accountCombo)
  , d_ptr(new KMyMoneyAccountComboSplitHelperPrivate(this))
{
  Q_D(KMyMoneyAccountComboSplitHelper);
  d->m_accountCombo = accountCombo;
  d->m_splitModel = model;

  connect(model, &QAbstractItemModel::dataChanged, this, &KMyMoneyAccountComboSplitHelper::splitCountChanged /*, Qt::QueuedConnection */);
  connect(model, &QAbstractItemModel::rowsRemoved, this, &KMyMoneyAccountComboSplitHelper::splitCountChanged, Qt::QueuedConnection);
  connect(model, &QAbstractItemModel::modelReset, this, &KMyMoneyAccountComboSplitHelper::splitCountChanged, Qt::QueuedConnection);
  connect(model, &QAbstractItemModel::destroyed, this, &KMyMoneyAccountComboSplitHelper::modelDestroyed);

  accountCombo->installEventFilter(this);
  if (accountCombo->lineEdit()) {
    accountCombo->lineEdit()->installEventFilter(this);
  }
  splitCountChanged();
}

KMyMoneyAccountComboSplitHelper::~KMyMoneyAccountComboSplitHelper()
{
}

bool KMyMoneyAccountComboSplitHelper::eventFilter(QObject* watched, QEvent* event)
{
  Q_D(KMyMoneyAccountComboSplitHelper);
  if (d->m_splitModel && (d->m_splitModel->rowCount() > 1)) {
    const auto type = event->type();
    if (watched == d->m_accountCombo) {
      if (type == QEvent::FocusIn) {
        // select the complete text (which is readonly)
        // to signal focus in the lineedit widget to the user
        const auto lineEdit = d->m_accountCombo->lineEdit();
        if (lineEdit) {
          lineEdit->selectAll();
        }
      }
    }
    if ((type == QEvent::MouseButtonPress)
      || (type == QEvent::MouseButtonRelease)
      || (type == QEvent::MouseButtonDblClick)) {
      // suppress opening the combo box
      // or selecting text in the lineedit
      return true;
    }
  }
  return QObject::eventFilter(watched, event);
}

void KMyMoneyAccountComboSplitHelper::modelDestroyed()
{
  Q_D(KMyMoneyAccountComboSplitHelper);
  d->m_splitModel = nullptr;
}

void KMyMoneyAccountComboSplitHelper::splitCountChanged()
{
  Q_D(KMyMoneyAccountComboSplitHelper);
  // sanity check
  if (!d->m_accountCombo || !d->m_splitModel || d->m_norecursive)
    return;

  d->m_norecursive = true;

  QModelIndexList indexes;
  d->m_accountCombo->lineEdit()->setReadOnly(false);
  switch (d->m_splitModel->rowCount()) {
    case 0:
      d->m_accountCombo->setCurrentIndex(-1);
      d->m_accountCombo->setCurrentText(QString());
      break;
    case 1:
      indexes = d->m_accountCombo->model()->match(d->m_accountCombo->model()->index(0,0),
                                                  eMyMoney::Model::IdRole,
                                                  d->m_splitModel->index(0, 0).data(eMyMoney::Model::SplitAccountIdRole).toString(),
                                                  1,
                                                  Qt::MatchFlags(Qt::MatchExactly | Qt::MatchWrap | Qt::MatchRecursive));
      if (indexes.isEmpty()) {
        d->m_accountCombo->setCurrentIndex(-1);
        d->m_accountCombo->setCurrentText(QString());
      } else {
        const auto idx = indexes.first();
        QSignalBlocker comboBoxBlocker(d->m_accountCombo);
        d->m_accountCombo->setRootModelIndex(idx.parent());
        d->m_accountCombo->setCurrentIndex(idx.row());
        d->m_accountCombo->setRootModelIndex(QModelIndex());
      }
      break;
    default:
      {
        QSignalBlocker lineEditBlocker(d->m_accountCombo->lineEdit());
        d->m_accountCombo->lineEdit()->setText(i18n("Split transaction"));
        d->m_accountCombo->lineEdit()->setReadOnly(true);
      }
      break;
  }
  d->m_accountCombo->hidePopup();
  emit accountComboEnabled(d->m_accountCombo->isEnabled());

  d->m_norecursive = false;
}

// kate: space-indent on; indent-width 2; remove-trailing-space on; remove-trailing-space-save on;
