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

// ----------------------------------------------------------------------------
// QT Includes

#include <qapplication.h>
//Added by qt3to4:
#include <Q3Frame>
#include <QKeyEvent>
#include <QEvent>

// ----------------------------------------------------------------------------
// KDE Includes

#include <k3listview.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneycompletion.h"
#include <kmymoney/kmymoneyselector.h>
#include <kmymoney/kmymoneylistviewitem.h>
#include "./kmymoneycombo.h"

const int kMyMoneyCompletion::MAX_ITEMS = 16;

kMyMoneyCompletion::kMyMoneyCompletion(QWidget *parent, const char *name ) :
  Q3VBox(parent, name, Qt::WType_Popup)
{
  m_selector = new KMyMoneySelector(this);
  m_selector->listView()->setFocusProxy(this);

  m_parent = parent;
  setFocusProxy((parent) ? parent : (QWidget*) NoFocus);
  setFrameStyle(Q3Frame::PopupPanel | Q3Frame::Raised);
  connectSignals(m_selector, m_selector->listView());
}

void kMyMoneyCompletion::connectSignals(QWidget* widget, K3ListView* lv)
{
  m_widget = widget;
  m_lv = lv;
  connect(lv, SIGNAL(executed(Q3ListViewItem*,const QPoint&,int)), this, SLOT(slotItemSelected(Q3ListViewItem*,const QPoint&,int)));
}

kMyMoneyCompletion::~kMyMoneyCompletion()
{
}

void kMyMoneyCompletion::adjustSize(void)
{
  Q3ListViewItemIterator it(m_lv, Q3ListViewItemIterator::Visible);
  int count = 0;
  while(it.current()) {
    ++count;
    ++it;
  }
  adjustSize(count);
}

void kMyMoneyCompletion::adjustSize(const int count)
{
  int w = m_widget->sizeHint().width();
  if(m_parent && w < m_parent->width())
    w = m_parent->width();

  QFontMetrics fm(font());
  if(w < fm.maxWidth()*15)
    w = fm.maxWidth()*15;

  int h = 0;
  Q3ListViewItemIterator it(m_lv, Q3ListViewItemIterator::Visible);
  Q3ListViewItem* item = it.current();
  if(item)
    h = item->height() * (count > MAX_ITEMS ? MAX_ITEMS : count);

  // the offset of 4 in the next statement avoids the
  // display of a scroll bar if count < MAX_ITEMS.
  resize(w, h+4);

  if(m_parent) {
    // the code of this basic block is taken from K3CompletionBox::show()
    // and modified to our local needs

    // this is probably better, once kde switches to requiring qt3.1
    // QRect screenSize = QApplication::desktop()->availableGeometry(d->m_parent);
    // for now use this since it's qt3.0.x-safe
    QRect screenSize = QApplication::desktop()->screenGeometry(QApplication::desktop()->screenNumber(m_parent));

    QPoint orig = m_parent->mapToGlobal( QPoint(0, m_parent->height()) );
    int x = orig.x();
    int y = orig.y();

    if ( x + width() > screenSize.right() )
        x = screenSize.right() - width();

    // check for the maximum height here to avoid flipping
    // of the completion box from top to bottom of the
    // edit widget. The offset (y) is certainly based
    // on the actual height.
    if(item) {
      if ((y + item->height()*MAX_ITEMS) > screenSize.bottom() )
        y = y - height() - m_parent->height();
    }

    move( x, y);
  }
}

void kMyMoneyCompletion::show(bool presetSelected)
{
  if(!m_id.isEmpty() && presetSelected)
    m_selector->setSelected(m_id);

  adjustSize();

  if(m_parent) {
    m_parent->installEventFilter(this);
    // make sure to install the filter for the combobox lineedit as well
    // We have do this here because QObject::installEventFilter() is not
    // declared virtual and we have no chance to override it in KMyMoneyCombo
    KMyMoneyCombo* c = dynamic_cast<KMyMoneyCombo*>(m_parent);
    if(c && c->lineEdit()) {
      c->lineEdit()->installEventFilter(this);
    }
  }

  Q3VBox::show();
}

void kMyMoneyCompletion::hide(void)
{
  if(m_parent) {
    m_parent->removeEventFilter(this);
    // make sure to uninstall the filter for the combobox lineedit as well
    // We have do this here because QObject::installEventFilter() is not
    // declared virtual and we have no chance to override it in KMyMoneyCombo
    KMyMoneyCombo* c = dynamic_cast<KMyMoneyCombo*>(m_parent);
    if(c && c->lineEdit()) {
      c->lineEdit()->removeEventFilter(this);
    }
  }
  Q3VBox::hide();
}

bool kMyMoneyCompletion::eventFilter(QObject* o, QEvent* e)
{
  int type = e->type();

  KMyMoneyCombo *c = dynamic_cast<KMyMoneyCombo*>(m_parent);
  Q3ListViewItem* item;
  if(o == m_parent || (c && o == c->lineEdit())) {
    if(isVisible()) {
      if(type == QEvent::KeyPress) {
        QKeyEvent* ev = static_cast<QKeyEvent*> (e);
        QKeyEvent evt(QEvent::KeyPress,
                      Qt::Key_Down, 0, ev->state(), QString::null,
                      ev->isAutoRepeat(), ev->count());
        QKeyEvent evbt(QEvent::KeyPress,
                      Qt::Key_Up, 0, ev->state(), QString::null,
                      ev->isAutoRepeat(), ev->count());

        switch(ev->key()) {
          case Qt::Key_Tab:
          case Qt::Key_Backtab:
            slotItemSelected(m_lv->currentItem(), QPoint(0,0), 0);
            break;

          case Qt::Key_Down:
          case Qt::Key_PageDown:
            item = m_lv->currentItem();
            while(item) {
              item = item->itemBelow();
              if(item && selector()->match(m_lastCompletion, item))
                break;
            }
            if(item) {
              m_lv->setCurrentItem(item);
              selector()->ensureItemVisible(item);
            }
            ev->accept();
            return true;

          case Qt::Key_Up:
          case Qt::Key_PageUp:
            item = m_lv->currentItem();
            while(item) {
              item = item->itemAbove();
              if(item && selector()->match(m_lastCompletion, item))
                break;
            }
            if(item) {
              m_lv->setCurrentItem(item);
              // make sure, we always see a possible (non-selectable) group item
              if(item->itemAbove())
                item = item->itemAbove();
              selector()->ensureItemVisible(item);
            }
            ev->accept();
            return true;

          case Qt::Key_Escape:
            hide();
            ev->accept();
            return true;

          case Qt::Key_Enter:
          case Qt::Key_Return:
            slotItemSelected(m_lv->currentItem(), QPoint(0,0), 0);
            ev->accept();
            return true;

          case Qt::Key_Home:
          case Qt::Key_End:
            if(ev->state() & Qt::ControlModifier) {
              item = m_lv->currentItem();
              if(ev->key() == Qt::Key_Home) {
                while(item && item->itemAbove()) {
                  item = item->itemAbove();
                }
                while(item && !selector()->match(m_lastCompletion, item)) {
                  item = item->itemBelow();
                }
              } else {
                while(item && item->itemBelow()) {
                  item = item->itemBelow();
                }
                while(item && !selector()->match(m_lastCompletion, item)) {
                  item = item->itemAbove();
                }
              }
              if(item) {
                m_lv->setCurrentItem(item);
                // make sure, we always see a possible (non-selectable) group item
                if(item->itemAbove())
                  item = item->itemAbove();
                selector()->ensureItemVisible(item);
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
  return Q3VBox::eventFilter(o, e);
}

void kMyMoneyCompletion::slotMakeCompletion(const QString& txt)
{
  int cnt = selector()->slotMakeCompletion(txt);

  if(m_parent && m_parent->isVisible() && !isVisible() && cnt)
    show(false);
  else {
    if(cnt != 0) {
      adjustSize();
    } else {
      hide();
    }
  }
}

void kMyMoneyCompletion::slotItemSelected(Q3ListViewItem *item, const QPoint&, int)
{
  KMyMoneyListViewItem* it_v = static_cast<KMyMoneyListViewItem*>(item);
  if(it_v && it_v->isSelectable()) {
    QString id = it_v->id();
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

#include "kmymoneycompletion.moc"
