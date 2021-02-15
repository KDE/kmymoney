/*
    SPDX-FileCopyrightText: 2004-2011 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kmymoneycompletion.h"
#include "kmymoneycompletion_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QApplication>
#include <QKeyEvent>
#include <QEvent>
#include <QDesktopWidget>
#include <QLineEdit>
#include <QVBoxLayout>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoneyselector.h>
#include "kmymoneycombo.h"
#include "widgetenums.h"

KMyMoneyCompletion::KMyMoneyCompletion(QWidget *parent) :
    QWidget(parent),
    d_ptr(new KMyMoneyCompletionPrivate)
{
  Q_D(KMyMoneyCompletion);
  setWindowFlags(Qt::ToolTip);
  // make it look like the Qt completer
  QVBoxLayout *completionLayout = new QVBoxLayout(this);
  completionLayout->setContentsMargins(0, 0, 0, 0);
  completionLayout->setSpacing(0);

  d->m_parent = parent;
  d->m_selector = new KMyMoneySelector(this);
  d->m_selector->listView()->setFocusProxy(parent);
  completionLayout->addWidget(d->m_selector);

  // to handle the keyboard events received by this widget in the same way as
  // the keyboard events received by the other widgets
  installEventFilter(this);

  connectSignals(d->m_selector, d->m_selector->listView());
}

void KMyMoneyCompletion::connectSignals(QWidget* widget, QTreeWidget* lv)
{
  Q_D(KMyMoneyCompletion);
  d->m_widget = widget;
  d->m_lv = lv;
  connect(lv, &QTreeWidget::itemActivated, this, &KMyMoneyCompletion::slotItemSelected);
  connect(lv, &QTreeWidget::itemClicked, this, &KMyMoneyCompletion::slotItemSelected);
}

KMyMoneyCompletion::KMyMoneyCompletion(KMyMoneyCompletionPrivate &dd, QWidget* parent) :
  QWidget(parent),
  d_ptr(&dd)
{
}

KMyMoneyCompletion::~KMyMoneyCompletion()
{
  Q_D(KMyMoneyCompletion);
  delete d;
}

void KMyMoneyCompletion::adjustSize()
{
  Q_D(KMyMoneyCompletion);
  QTreeWidgetItemIterator it(d->m_lv, QTreeWidgetItemIterator::NotHidden);
  int count = 0;
  while (*it) {
    ++count;
    ++it;
  }
  adjustSize(count);
}

void KMyMoneyCompletion::adjustSize(const int count)
{
  Q_D(KMyMoneyCompletion);
  int w = d->m_widget->sizeHint().width();
  if (d->m_parent && w < d->m_parent->width())
    w = d->m_parent->width();

  const int minimumWidth = fontMetrics().width(QLatin1Char('W')) * 15;
  w = qMax(w, minimumWidth);

  int h = 0;
  QTreeWidgetItemIterator it(d->m_lv, QTreeWidgetItemIterator::NotHidden);
  QTreeWidgetItem* item = *it;
  if (item)
    // the +1 in the next statement avoids the display of a scroll bar if count < MAX_ITEMS.
    h = item->treeWidget()->visualItemRect(item).height() * (count > KMyMoneyCompletionPrivate::MAX_ITEMS - 1 ? KMyMoneyCompletionPrivate::MAX_ITEMS : count + 1);

  resize(w, h);

  if (d->m_parent) {
    // the code of this basic block is taken from KCompletionBox::show()
    // and modified to our local needs

    QRect screenSize = QApplication::desktop()->availableGeometry(parentWidget());

    QPoint orig = d->m_parent->mapToGlobal(QPoint(0, d->m_parent->height()));
    int x = orig.x();
    int y = orig.y();

    if (x + width() > screenSize.right())
      x = screenSize.right() - width();

    // check for the maximum height here to avoid flipping
    // of the completion box from top to bottom of the
    // edit widget. The offset (y) is certainly based
    // on the actual height.
    if (item) {
      if ((y + item->treeWidget()->visualItemRect(item).height() * KMyMoneyCompletionPrivate::MAX_ITEMS) > screenSize.bottom())
        y = y - height() - d->m_parent->height();
    }

    move(x, y);
  }
}

void KMyMoneyCompletion::showEvent(QShowEvent* e)
{
  show(true);
  QWidget::showEvent(e);
}

void KMyMoneyCompletion::show(bool presetSelected)
{
  Q_D(KMyMoneyCompletion);
  if (!d->m_id.isEmpty() && presetSelected)
    d->m_selector->setSelected(d->m_id);

  adjustSize();

  if (d->m_parent) {
    d->m_parent->installEventFilter(this);
    // make sure to install the filter for the combobox lineedit as well
    // We have do this here because QObject::installEventFilter() is not
    // declared virtual and we have no chance to override it in KMyMoneyCombo
    KMyMoneyCombo* c = dynamic_cast<KMyMoneyCombo*>(d->m_parent);
    if (c && c->lineEdit()) {
      c->lineEdit()->installEventFilter(this);
    }
  }
  QWidget::show();

  // make sure that the parent is the input context's focus widget instead of the selector's list
  //if (qApp->inputContext()->focusWidget() == m_selector->listView())
    //qApp->inputContext()->setFocusWidget(m_parent);
}

void KMyMoneyCompletion::hide()
{
  Q_D(KMyMoneyCompletion);
  if (d->m_parent) {
    d->m_parent->removeEventFilter(this);
    // make sure to uninstall the filter for the combobox lineedit as well
    // We have do this here because QObject::installEventFilter() is not
    // declared virtual and we have no chance to override it in KMyMoneyCombo
    KMyMoneyCombo* c = dynamic_cast<KMyMoneyCombo*>(d->m_parent);
    if (c && c->lineEdit()) {
      c->lineEdit()->removeEventFilter(this);
    }
  }
  QWidget::hide();
}

bool KMyMoneyCompletion::eventFilter(QObject* o, QEvent* e)
{
  Q_D(KMyMoneyCompletion);
  KMyMoneyCombo *c = dynamic_cast<KMyMoneyCombo*>(d->m_parent);
  if (o == d->m_parent || (c && o == c->lineEdit()) || o == this) {
    if (isVisible()) {
#ifdef Q_OS_WIN32                   //krazy:exclude=cpp 
      // hide the completer only if the focus was not lost because of windows activation or the activated window is not an application window
      if (e->type() == QEvent::FocusOut && (static_cast<QFocusEvent*>(e)->reason() != Qt::ActiveWindowFocusReason || QApplication::activeWindow() == 0)) {
#else
      if (e->type() == QEvent::FocusOut) {
#endif
        hide();
      }
      if (e->type() == QEvent::KeyPress) {
        QTreeWidgetItem* item = 0;
        QKeyEvent* ev = static_cast<QKeyEvent*>(e);
        switch (ev->key()) {
          case Qt::Key_Tab:
          case Qt::Key_Backtab:
            slotItemSelected(d->m_lv->currentItem(), 0);
            break;

          case Qt::Key_Down:
          case Qt::Key_PageDown:
            item = d->m_lv->currentItem();
            while (item) {
              item = d->m_lv->itemBelow(item);
              if (item && selector()->match(d->m_lastCompletion, item))
                break;
            }
            if (item) {
              d->m_lv->setCurrentItem(item);
              d->m_lv->scrollToItem(item);
            }
            ev->accept();
            return true;

          case Qt::Key_Up:
          case Qt::Key_PageUp:
            item = d->m_lv->currentItem();
            while (item) {
              item = d->m_lv->itemAbove(item);
              if (item && selector()->match(d->m_lastCompletion, item))
                break;
            }
            if (item) {
              d->m_lv->setCurrentItem(item);
              // make sure, we always see a possible (non-selectable) group item
              if (d->m_lv->itemAbove(item))
                item = d->m_lv->itemAbove(item);
              d->m_lv->scrollToItem(item);
            }
            ev->accept();
            return true;

          case Qt::Key_Escape:
            hide();
            ev->accept();
            return true;

          case Qt::Key_Enter:
          case Qt::Key_Return:
            slotItemSelected(d->m_lv->currentItem(), 0);
            ev->accept();
            return true;

          case Qt::Key_Home:
          case Qt::Key_End:
            if (ev->modifiers() & Qt::ControlModifier) {
              item = d->m_lv->currentItem();
              if (ev->key() == Qt::Key_Home) {
                while (item && d->m_lv->itemAbove(item)) {
                  item = d->m_lv->itemAbove(item);
                }
                while (item && !selector()->match(d->m_lastCompletion, item)) {
                  item = d->m_lv->itemBelow(item);
                }
              } else {
                while (item && d->m_lv->itemBelow(item)) {
                  item = d->m_lv->itemBelow(item);
                }
                while (item && !selector()->match(d->m_lastCompletion, item)) {
                  item = d->m_lv->itemAbove(item);
                }
              }
              if (item) {
                d->m_lv->setCurrentItem(item);
                // make sure, we always see a possible (non-selectable) group item
                if (d->m_lv->itemAbove(item))
                  item = d->m_lv->itemAbove(item);
                d->m_lv->scrollToItem(item);
              }
              ev->accept();
              return true;
            }
            break;

          default:
            break;
        }
      }
    }
  }
  return QWidget::eventFilter(o, e);
}

void KMyMoneyCompletion::slotMakeCompletion(const QString& txt)
{
  Q_D(KMyMoneyCompletion);
  auto cnt = selector()->slotMakeCompletion(txt.trimmed());

  if (d->m_parent && d->m_parent->isVisible() && !isVisible() && cnt)
    show(false);
  else {
    if (cnt != 0) {
      adjustSize();
    } else {
      hide();
    }
  }
}

void KMyMoneyCompletion::slotItemSelected(QTreeWidgetItem *item, int)
{
  Q_D(KMyMoneyCompletion);
  if (item && item->flags().testFlag(Qt::ItemIsSelectable)) {
    QString id = item->data(0, (int)eWidgets::Selector::Role::Id).toString();
    // hide the widget, so we can debug the slots that are connect
    // to the signal we emit very soon
    hide();
    d->m_id = id;
    emit itemSelected(id);
  }
}

void KMyMoneyCompletion::setSelected(const QString& id)
{
  Q_D(KMyMoneyCompletion);
  d->m_id = id;
  d->m_selector->setSelected(id, true);
}

KMyMoneySelector* KMyMoneyCompletion::selector() const
{
  Q_D(const KMyMoneyCompletion);
  return d->m_selector;
}
