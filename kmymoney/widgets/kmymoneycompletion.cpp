/***************************************************************************
                          kmymoneycompletion.cpp  -  description
                             -------------------
    begin                : Mon Apr 26 2004
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

#include "kmymoneycompletion.h"

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

const int kMyMoneyCompletion::MAX_ITEMS = 16;

kMyMoneyCompletion::kMyMoneyCompletion(QWidget *parent) :
    QWidget(parent)
{
  setWindowFlags(Qt::ToolTip);
  // make it look like the Qt completer
  QVBoxLayout *completionLayout = new QVBoxLayout(this);
  completionLayout->setContentsMargins(0, 0, 0, 0);
  completionLayout->setSpacing(0);

  m_parent = parent;
  m_selector = new KMyMoneySelector(this);
  m_selector->listView()->setFocusProxy(parent);
  completionLayout->addWidget(m_selector);

  // to handle the keyboard events received by this widget in the same way as
  // the keyboard events received by the other widgets
  installEventFilter(this);

  connectSignals(m_selector, m_selector->listView());
}

void kMyMoneyCompletion::connectSignals(QWidget* widget, QTreeWidget* lv)
{
  m_widget = widget;
  m_lv = lv;
  connect(lv, SIGNAL(itemActivated(QTreeWidgetItem*,int)), this, SLOT(slotItemSelected(QTreeWidgetItem*,int)));
  connect(lv, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(slotItemSelected(QTreeWidgetItem*,int)));
}

kMyMoneyCompletion::~kMyMoneyCompletion()
{
}

void kMyMoneyCompletion::adjustSize()
{
  QTreeWidgetItemIterator it(m_lv, QTreeWidgetItemIterator::NotHidden);
  int count = 0;
  while (*it) {
    ++count;
    ++it;
  }
  adjustSize(count);
}

void kMyMoneyCompletion::adjustSize(const int count)
{
  int w = m_widget->sizeHint().width();
  if (m_parent && w < m_parent->width())
    w = m_parent->width();

  const int minimumWidth = fontMetrics().width(QLatin1Char('W')) * 15;
  w = qMax(w, minimumWidth);

  int h = 0;
  QTreeWidgetItemIterator it(m_lv, QTreeWidgetItemIterator::NotHidden);
  QTreeWidgetItem* item = *it;
  if (item)
    // the +1 in the next statement avoids the display of a scroll bar if count < MAX_ITEMS.
    h = item->treeWidget()->visualItemRect(item).height() * (count > MAX_ITEMS - 1 ? MAX_ITEMS : count + 1);

  resize(w, h);

  if (m_parent) {
    // the code of this basic block is taken from KCompletionBox::show()
    // and modified to our local needs

    QRect screenSize = QApplication::desktop()->availableGeometry(parentWidget());

    QPoint orig = m_parent->mapToGlobal(QPoint(0, m_parent->height()));
    int x = orig.x();
    int y = orig.y();

    if (x + width() > screenSize.right())
      x = screenSize.right() - width();

    // check for the maximum height here to avoid flipping
    // of the completion box from top to bottom of the
    // edit widget. The offset (y) is certainly based
    // on the actual height.
    if (item) {
      if ((y + item->treeWidget()->visualItemRect(item).height() * MAX_ITEMS) > screenSize.bottom())
        y = y - height() - m_parent->height();
    }

    move(x, y);
  }
}

void kMyMoneyCompletion::showEvent(QShowEvent* e)
{
  show(true);
  QWidget::showEvent(e);
}

void kMyMoneyCompletion::show(bool presetSelected)
{
  if (!m_id.isEmpty() && presetSelected)
    m_selector->setSelected(m_id);

  adjustSize();

  if (m_parent) {
    m_parent->installEventFilter(this);
    // make sure to install the filter for the combobox lineedit as well
    // We have do this here because QObject::installEventFilter() is not
    // declared virtual and we have no chance to override it in KMyMoneyCombo
    KMyMoneyCombo* c = dynamic_cast<KMyMoneyCombo*>(m_parent);
    if (c && c->lineEdit()) {
      c->lineEdit()->installEventFilter(this);
    }
  }
  QWidget::show();

  // make sure that the parent is the input context's focus widget instead of the selector's list
  //if (qApp->inputContext()->focusWidget() == m_selector->listView())
    //qApp->inputContext()->setFocusWidget(m_parent);
}

void kMyMoneyCompletion::hide()
{
  if (m_parent) {
    m_parent->removeEventFilter(this);
    // make sure to uninstall the filter for the combobox lineedit as well
    // We have do this here because QObject::installEventFilter() is not
    // declared virtual and we have no chance to override it in KMyMoneyCombo
    KMyMoneyCombo* c = dynamic_cast<KMyMoneyCombo*>(m_parent);
    if (c && c->lineEdit()) {
      c->lineEdit()->removeEventFilter(this);
    }
  }
  QWidget::hide();
}

bool kMyMoneyCompletion::eventFilter(QObject* o, QEvent* e)
{
  KMyMoneyCombo *c = dynamic_cast<KMyMoneyCombo*>(m_parent);
  if (o == m_parent || (c && o == c->lineEdit()) || o == this) {
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
            slotItemSelected(m_lv->currentItem(), 0);
            break;

          case Qt::Key_Down:
          case Qt::Key_PageDown:
            item = m_lv->currentItem();
            while (item) {
              item = m_lv->itemBelow(item);
              if (item && selector()->match(m_lastCompletion, item))
                break;
            }
            if (item) {
              m_lv->setCurrentItem(item);
              m_lv->scrollToItem(item);
            }
            ev->accept();
            return true;

          case Qt::Key_Up:
          case Qt::Key_PageUp:
            item = m_lv->currentItem();
            while (item) {
              item = m_lv->itemAbove(item);
              if (item && selector()->match(m_lastCompletion, item))
                break;
            }
            if (item) {
              m_lv->setCurrentItem(item);
              // make sure, we always see a possible (non-selectable) group item
              if (m_lv->itemAbove(item))
                item = m_lv->itemAbove(item);
              m_lv->scrollToItem(item);
            }
            ev->accept();
            return true;

          case Qt::Key_Escape:
            hide();
            ev->accept();
            return true;

          case Qt::Key_Enter:
          case Qt::Key_Return:
            slotItemSelected(m_lv->currentItem(), 0);
            ev->accept();
            return true;

          case Qt::Key_Home:
          case Qt::Key_End:
            if (ev->modifiers() & Qt::ControlModifier) {
              item = m_lv->currentItem();
              if (ev->key() == Qt::Key_Home) {
                while (item && m_lv->itemAbove(item)) {
                  item = m_lv->itemAbove(item);
                }
                while (item && !selector()->match(m_lastCompletion, item)) {
                  item = m_lv->itemBelow(item);
                }
              } else {
                while (item && m_lv->itemBelow(item)) {
                  item = m_lv->itemBelow(item);
                }
                while (item && !selector()->match(m_lastCompletion, item)) {
                  item = m_lv->itemAbove(item);
                }
              }
              if (item) {
                m_lv->setCurrentItem(item);
                // make sure, we always see a possible (non-selectable) group item
                if (m_lv->itemAbove(item))
                  item = m_lv->itemAbove(item);
                m_lv->scrollToItem(item);
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

void kMyMoneyCompletion::slotMakeCompletion(const QString& txt)
{
  int cnt = selector()->slotMakeCompletion(txt.trimmed());

  if (m_parent && m_parent->isVisible() && !isVisible() && cnt)
    show(false);
  else {
    if (cnt != 0) {
      adjustSize();
    } else {
      hide();
    }
  }
}

void kMyMoneyCompletion::slotItemSelected(QTreeWidgetItem *item, int)
{
  if (item && item->flags().testFlag(Qt::ItemIsSelectable)) {
    QString id = item->data(0, KMyMoneySelector::IdRole).toString();
    // hide the widget, so we can debug the slots that are connect
    // to the signal we emit very soon
    hide();
    m_id = id;
    emit itemSelected(id);
  }
}

void kMyMoneyCompletion::setSelected(const QString& id)
{
  m_id = id;
  m_selector->setSelected(id, true);
}
