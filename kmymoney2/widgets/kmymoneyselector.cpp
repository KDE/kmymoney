/***************************************************************************
                             kmymoneyselector.cpp
                             -------------------
    begin                : Thu Jun 29 2006
    copyright            : (C) 2006 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
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

#include <QLayout>
#include <q3header.h>
#include <QTimer>
#include <QStyle>
#include <QRegExp>
//Added by qt3to4:
#include <Q3HBoxLayout>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoneyselector.h>
#include <kmymoneylistviewitem.h>
#include <kmymoneychecklistitem.h>

#include "kmymoneyglobalsettings.h"
#include "config-kmymoney.h"

KMyMoneySelector::KMyMoneySelector(QWidget *parent, const char *name, Qt::WFlags flags) :
  QWidget(parent, name, flags)
{
  m_selMode = Q3ListView::Single;

  m_listView = new K3ListView(this);
  // don't show horizontal scroll bar
  m_listView->setHScrollBarMode(Q3ScrollView::AlwaysOff);

  m_listView->setSorting(-1);

  if(parent) {
    setFocusProxy(parent->focusProxy());
    m_listView->setFocusProxy(parent->focusProxy());
  }

  m_listView->setAllColumnsShowFocus(true);

  m_layout = new Q3HBoxLayout( this, 0, 6);

  m_listView->addColumn( "Hidden" );
  m_listView->header()->hide();
  m_listView->header()->setStretchEnabled(true, -1);
  m_listView->header()->adjustHeaderSize();

  m_layout->addWidget( m_listView );

  // force init
  m_selMode = Q3ListView::Multi;
  setSelectionMode(Q3ListView::Single);

  connect(m_listView, SIGNAL(rightButtonPressed(Q3ListViewItem* , const QPoint&, int)), this, SLOT(slotListRightMouse(Q3ListViewItem*, const QPoint&, int)));
}

KMyMoneySelector::~KMyMoneySelector()
{
}

void KMyMoneySelector::clear(void)
{
  m_listView->clear();
  m_visibleItem = 0;
}

void KMyMoneySelector::setSelectionMode(const Q3ListView::SelectionMode mode)
{
  if(m_selMode != mode) {
    m_selMode = mode;
    clear();

    // make sure, it's either Multi or Single
    if(mode != Q3ListView::Multi) {
      m_selMode = Q3ListView::Single;
      connect(m_listView, SIGNAL(selectionChanged(void)), this, SIGNAL(stateChanged(void)));
      connect(m_listView, SIGNAL(executed(Q3ListViewItem*)), this, SLOT(slotItemSelected(Q3ListViewItem*)));
    } else {
      disconnect(m_listView, SIGNAL(selectionChanged(void)), this, SIGNAL(stateChanged(void)));
      disconnect(m_listView, SIGNAL(executed(Q3ListViewItem*)), this, SLOT(slotItemSelected(Q3ListViewItem*)));
    }
  }
  QWidget::update();
}

void KMyMoneySelector::slotItemSelected(Q3ListViewItem *item)
{
  if(m_selMode == Q3ListView::Single) {
    KMyMoneyListViewItem* l_item = dynamic_cast<KMyMoneyListViewItem*>(item);
    if(l_item && l_item->isSelectable()) {
      emit itemSelected(l_item->id());
    }
  }
}

Q3ListViewItem* KMyMoneySelector::newItem(const QString& name, Q3ListViewItem* after, const QString& key, const QString& id, Q3CheckListItem::Type type)
{
  Q3ListViewItem* item;
  if(after)
    item = new KMyMoneyCheckListItem(m_listView, after, name, key, id, type);
  else
    item = new KMyMoneyCheckListItem(m_listView, name, key, id, type);

  item->setSelectable(!id.isEmpty());
  item->setOpen(true);
  return item;
}

Q3ListViewItem* KMyMoneySelector::newItem(const QString& name, const QString& key, const QString& id, Q3CheckListItem::Type type)
{
  return newItem(name, 0, key, id, type);
}

Q3ListViewItem* KMyMoneySelector::newTopItem(const QString& name, const QString& key, const QString& id)
{
  Q3ListViewItem* p;

  if(m_selMode == Q3ListView::Multi) {
    KMyMoneyCheckListItem* q = new KMyMoneyCheckListItem(m_listView, name, key, id);
    connect(q, SIGNAL(stateChanged(bool)), this, SIGNAL(stateChanged(void)));
    p = static_cast<Q3ListViewItem*> (q);

  } else {
    KMyMoneyListViewItem* q = new KMyMoneyListViewItem(m_listView, name, key, id);
    p = static_cast<Q3ListViewItem*> (q);
  }

  return p;
}

Q3ListViewItem* KMyMoneySelector::newItem(Q3ListViewItem* parent, const QString& name, const QString& key, const QString& id)
{
  Q3ListViewItem* p;

  if(m_selMode == Q3ListView::Multi) {
    KMyMoneyCheckListItem* q = new KMyMoneyCheckListItem(parent, name, key, id);
    connect(q, SIGNAL(stateChanged(bool)), this, SIGNAL(stateChanged(void)));
    p = static_cast<Q3ListViewItem*> (q);

  } else {
    KMyMoneyListViewItem* q = new KMyMoneyListViewItem(parent, name, key, id);
    p = static_cast<Q3ListViewItem*> (q);
  }

  return p;
}

void KMyMoneySelector::protectItem(const QString& itemId, const bool protect)
{
  Q3ListViewItemIterator it(m_listView, Q3ListViewItemIterator::Selectable);
  Q3ListViewItem* it_v;
  KMyMoneyListViewItem* it_l;
  KMyMoneyCheckListItem* it_c;

  // scan items
  while((it_v = it.current()) != 0) {
    it_l = dynamic_cast<KMyMoneyListViewItem*>(it_v);
    if(it_l) {
      if(it_l->id() == itemId) {
        it_l->setSelectable(!protect);
        break;
      }
    } else {
      it_c = dynamic_cast<KMyMoneyCheckListItem*>(it_v);
      if(it_c) {
        if(it_c->id() == itemId) {
          it_c->setSelectable(!protect);
          break;
        }
      }
    }
    ++it;
  }
}

Q3ListViewItem* KMyMoneySelector::item(const QString& id) const
{
  Q3ListViewItemIterator it(m_listView, Q3ListViewItemIterator::Selectable);
  Q3ListViewItem* it_v;
  KMyMoneyListViewItem* it_l;
  KMyMoneyCheckListItem* it_c;

  while((it_v = it.current()) != 0) {
    it_l = dynamic_cast<KMyMoneyListViewItem*>(it_v);
    if(it_l) {
      if(it_l->id() == id)
        break;
    } else {
      it_c = dynamic_cast<KMyMoneyCheckListItem*>(it_v);
      if(it_c->id() == id)
        break;
    }
    ++it;
  }
  return it_v;
}

int KMyMoneySelector::optimizedWidth(void) const
{
  Q3ListViewItemIterator it(m_listView, Q3ListViewItemIterator::Selectable);
  Q3ListViewItem* it_v;
  KMyMoneyListViewItem* it_l;
  KMyMoneyCheckListItem* it_c;

  // scan items
  int w = 0;
#ifndef KMM_DESIGNER
  QFontMetrics fm( KMyMoneyGlobalSettings::listCellFont());
#else
  QFontMetrics fm( font() );
#endif
  while((it_v = it.current()) != 0) {
    it_l = dynamic_cast<KMyMoneyListViewItem*>(it_v);
    int nw = 0;
    if(it_l) {
      nw = it_l->width(fm, m_listView, 0);
    } else {
      it_c = dynamic_cast<KMyMoneyCheckListItem*>(it_v);
      if(it_c) {
        nw = it_c->width(fm, m_listView, 0);
      }
    }
    if(nw > w)
      w = nw;
    ++it;
  }
  return w;
}

void KMyMoneySelector::setOptimizedWidth(void)
{
  int w = optimizedWidth();

  m_listView->setMinimumWidth(w+30);
  m_listView->setMaximumWidth(w+30);
  m_listView->setColumnWidth(0, w+28);
}

bool KMyMoneySelector::allItemsSelected(void) const
{
  Q3ListViewItem* it_v;

  if(m_selMode == Q3ListView::Single)
    return false;

  for(it_v = m_listView->firstChild(); it_v != 0; it_v = it_v->nextSibling()) {
    if(it_v->rtti() == 1) {
      Q3CheckListItem* it_c = dynamic_cast<Q3CheckListItem*>(it_v);
      if(it_c->type() == Q3CheckListItem::CheckBox) {
        if(!(it_c->isOn() && allItemsSelected(it_v)))
          return false;
      } else {
        if(!allItemsSelected(it_v))
          return false;
      }
    }
  }
  return true;
}

bool KMyMoneySelector::allItemsSelected(const Q3ListViewItem *item) const
{
  Q3ListViewItem* it_v;

  for(it_v = item->firstChild(); it_v != 0; it_v = it_v->nextSibling()) {
    if(it_v->rtti() == 1) {
      Q3CheckListItem* it_c = static_cast<Q3CheckListItem*>(it_v);
      if(!(it_c->isOn() && allItemsSelected(it_v)))
        return false;
    }
  }
  return true;
}

void KMyMoneySelector::removeItem(const QString& id)
{
  Q3ListViewItem* it_v;
  Q3ListViewItemIterator it;

  it = Q3ListViewItemIterator(m_listView);
  while((it_v = it.current()) != 0) {
    if(it_v->rtti() == 1) {
      KMyMoneyCheckListItem* it_c = dynamic_cast<KMyMoneyCheckListItem*>(it_v);
      if(it_c->type() == Q3CheckListItem::CheckBox) {
        if(id == it_c->id()) {
          if(it_c->firstChild()) {
            it_c->setSelectable(false);
          } else {
            delete it_c;
          }
        }
      }
    } else if(it_v->rtti() == 0) {
      KMyMoneyListViewItem* it_c = dynamic_cast<KMyMoneyListViewItem*>(it_v);
      if(id == it_c->id()) {
        if(it_c->firstChild()) {
          it_c->setSelectable(false);
        } else {
          delete it_c;
        }
      }
    }
    it++;
  }

  // get rid of top items that just lost the last children (e.g. Favorites)
  it = Q3ListViewItemIterator(m_listView, Q3ListViewItemIterator::NotSelectable);
  while((it_v = it.current()) != 0) {
    if(it_v->rtti() == 1) {
      KMyMoneyCheckListItem* it_c = dynamic_cast<KMyMoneyCheckListItem*>(it_v);
      if(it_c->childCount() == 0)
        delete it_c;
    }
    it++;
  }

  return;
}


void KMyMoneySelector::selectAllItems(const bool state)
{
  Q3ListViewItem* it_v;

  for(it_v = m_listView->firstChild(); it_v != 0; it_v = it_v->nextSibling()) {
    if(it_v->rtti() == 1) {
      Q3CheckListItem* it_c = dynamic_cast<Q3CheckListItem*>(it_v);
      if(it_c->type() == Q3CheckListItem::CheckBox) {
        it_c->setOn(state);
      }
      selectAllSubItems(it_v, state);
    }
  }
  emit stateChanged();
}

void KMyMoneySelector::selectItems(const QStringList& itemList, const bool state)
{
  Q3ListViewItem* it_v;

  for(it_v = m_listView->firstChild(); it_v != 0; it_v = it_v->nextSibling()) {
    if(it_v->rtti() == 1) {
      KMyMoneyCheckListItem* it_c = dynamic_cast<KMyMoneyCheckListItem*>(it_v);
      if(it_c->type() == Q3CheckListItem::CheckBox && itemList.contains(it_c->id())) {
        it_c->setOn(state);
      }
      selectSubItems(it_v, itemList, state);
    }
  }
  emit stateChanged();
}

void KMyMoneySelector::selectSubItems(Q3ListViewItem* item, const QStringList& itemList, const bool state)
{
  Q3ListViewItem* it_v;

  for(it_v = item->firstChild(); it_v != 0; it_v = it_v->nextSibling()) {
    if(it_v->rtti() == 1) {
      KMyMoneyCheckListItem* it_c = dynamic_cast<KMyMoneyCheckListItem*>(it_v);
      if(it_c->type() == Q3CheckListItem::CheckBox && itemList.contains(it_c->id())) {
        it_c->setOn(state);
      }
      selectSubItems(it_v, itemList, state);
    }
  }
}

void KMyMoneySelector::selectAllSubItems(Q3ListViewItem* item, const bool state)
{
  Q3ListViewItem* it_v;

  for(it_v = item->firstChild(); it_v != 0; it_v = it_v->nextSibling()) {
    if(it_v->rtti() == 1) {
      Q3CheckListItem* it_c = dynamic_cast<Q3CheckListItem*>(it_v);
      if(it_c->type() == Q3CheckListItem::CheckBox) {
        it_c->setOn(state);
      }
      selectAllSubItems(it_v, state);
    }
  }
}

void KMyMoneySelector::selectedItems(QStringList& list) const
{
  Q3ListViewItem*  it_v;

  list.clear();
  if(m_selMode == Q3ListView::Single) {
    KMyMoneyListViewItem* it_c = dynamic_cast<KMyMoneyListViewItem*>(m_listView->selectedItem());
    if(it_c != 0)
      list << it_c->id();

  } else {
    for(it_v = m_listView->firstChild(); it_v != 0; it_v = it_v->nextSibling()) {
      if(it_v->rtti() == 1) {
        KMyMoneyCheckListItem* it_c = dynamic_cast<KMyMoneyCheckListItem*>(it_v);
        if(it_c->type() == Q3CheckListItem::CheckBox) {
          if(it_c->isOn())
            list << (*it_c).id();
        }
        selectedItems(list, it_v);
      }
    }
  }
}

void KMyMoneySelector::selectedItems(QStringList& list, Q3ListViewItem* item) const
{
  Q3ListViewItem* it_v;

  for(it_v = item->firstChild(); it_v != 0; it_v = it_v->nextSibling()) {
    if(it_v->rtti() == 1) {
      KMyMoneyCheckListItem* it_c = dynamic_cast<KMyMoneyCheckListItem*>(it_v);
      if(it_c->type() == Q3CheckListItem::CheckBox) {
        if(it_c->isOn())
          list << (*it_c).id();
        selectedItems(list, it_v);
      }
    }
  }
}

void KMyMoneySelector::itemList(QStringList& list) const
{
  Q3ListViewItemIterator it;
  Q3ListViewItem* it_v;

  it = Q3ListViewItemIterator(m_listView, Q3ListViewItemIterator::Selectable);
  while((it_v = it.current()) != 0) {
    {
      if(it_v->rtti() == 1) {
        KMyMoneyCheckListItem* it_c = dynamic_cast<KMyMoneyCheckListItem*>(it_v);
        if(it_c->type() == Q3CheckListItem::CheckBox) {
          list << it_c->id();
        }
      } else if(it_v->rtti() == 0) {
        KMyMoneyListViewItem* it_c = dynamic_cast<KMyMoneyListViewItem*>(it_v);
        list << it_c->id();
      }
    }
    it++;
  }
}

void KMyMoneySelector::setSelected(const QString& id, const bool state)
{
  Q3ListViewItemIterator it;
  Q3ListViewItem* it_v;
  Q3ListViewItem* it_visible = 0;

  it = Q3ListViewItemIterator(m_listView, Q3ListViewItemIterator::Selectable);
  while((it_v = it.current()) != 0) {
    if(it_v->rtti() == 1) {
      KMyMoneyCheckListItem* it_c = dynamic_cast<KMyMoneyCheckListItem*>(it_v);
      Q_CHECK_PTR(it_c);
      if(it_c->type() == Q3CheckListItem::CheckBox) {
        if(it_c->id() == id) {
          it_c->setOn(state);
          m_listView->setSelected(it_v, true);
          if(!it_visible)
            it_visible = it_v;
        }
      }
    } else if(it_v->rtti() == 0) {
      KMyMoneyListViewItem* it_c = dynamic_cast<KMyMoneyListViewItem*>(it_v);
      Q_CHECK_PTR(it_c);
      if(it_c->id() == id) {
        m_listView->setSelected(it_v, true);
        if(!it_visible)
          it_visible = it_v;
        ensureItemVisible(it_v);
        return;
      }
    }
    it++;
  }

  // make sure the first one found is visible
  if(it_visible)
    ensureItemVisible(it_visible);
}

void KMyMoneySelector::ensureItemVisible(const Q3ListViewItem *it_v)
{
  // for some reason, I could only use the ensureItemVisible() method
  // of QListView successfully, after the widget was drawn on the screen.
  // If called before it had no effect (if the item was not visible).
  //
  // The solution was to store the item we wanted to see in a local var
  // and call QListView::ensureItemVisible() about 10ms later in
  // the slot slotShowSelected.  (ipwizard, 12/29/2003)
  m_visibleItem = it_v;

  QTimer::singleShot(100, this, SLOT(slotShowSelected()));
}

void KMyMoneySelector::slotShowSelected(void)
{
  if(m_listView && m_visibleItem)
    m_listView->ensureItemVisible(m_visibleItem);
}

int KMyMoneySelector::slotMakeCompletion(const QString& _txt)
{
  QString txt(QRegExp::escape(_txt));
  if(KMyMoneyGlobalSettings::stringMatchFromStart() && this->isA("KMyMoneySelector") )
    txt.prepend('^');
  return slotMakeCompletion(QRegExp(txt, false));
}

bool KMyMoneySelector::match(const QRegExp& exp, Q3ListViewItem* item) const
{
  return exp.search(item->text(0)) != -1;
}

int KMyMoneySelector::slotMakeCompletion(const QRegExp& exp)
{
  Q3ListViewItemIterator it(m_listView, Q3ListViewItemIterator::Selectable);

  Q3ListViewItem* it_v;

  // The logic used here seems to be awkward. The problem is, that
  // QListViewItem::setVisible works recursively on all it's children
  // and grand-children.
  //
  // The way out of this is as follows: Make all items visible.
  // Then go through the list again and perform the checks.
  // If an item does not have any children (last leaf in the tree view)
  // perform the check. Then check recursively on the parent of this
  // leaf that it has no visible children. If that is the case, make the
  // parent invisible and continue this check with it's parent.
  while((it_v = it.current()) != 0) {
    it_v->setVisible(true);
    ++it;
  }

  Q3ListViewItem* firstMatch = 0;

  if(!exp.pattern().isEmpty()) {
    it = Q3ListViewItemIterator(m_listView, Q3ListViewItemIterator::Selectable);
    while((it_v = it.current()) != 0) {
      if(it_v->firstChild() == 0) {
        if(!match(exp, it_v)) {
          // this is a node which does not contain the
          // text and does not have children. So we can
          // safely hide it. Then we check, if the parent
          // has more children which are still visible. If
          // none are found, the parent node is hidden also. We
          // continue until the top of the tree or until we
          // find a node that still has visible children.
          bool hide = true;
          while(hide) {
            it_v->setVisible(false);
            it_v = it_v->parent();
            if(it_v && it_v->isSelectable()) {
              hide = !match(exp, it_v);
              Q3ListViewItem* child = it_v->firstChild();
              for(; child && hide; child = child->nextSibling()) {
                if(child->isVisible())
                  hide = false;
              }
            } else
              hide = false;
          }
        } else if(!firstMatch) {
          firstMatch = it_v;
        }
        ++it;

      } else if(match(exp, it_v)) {
        if(!firstMatch) {
          firstMatch = it_v;
        }
        // a node with children contains the text. We want
        // to display all child nodes in this case, so we need
        // to advance the iterator to the next sibling of the
        // current node. This could well be the sibling of a
        // parent or grandparent node.
        Q3ListViewItem* curr = it_v;
        Q3ListViewItem* item;
        while((item = curr->nextSibling()) == 0) {
          curr = curr->parent();
          if(curr == 0)
            break;
          if(match(exp, curr))
            firstMatch = curr;
        }
        do {
          ++it;
        } while(it.current() && it.current() != item);

      } else {
        // It's a node with children that does not match. We don't
        // change it's status here.
        ++it;
      }
    }
  }

  // make the first match the one that is selected
  // if we have no match, make sure none is selected
  if(m_selMode == Q3ListView::Single) {
    if(firstMatch) {
      m_listView->setSelected(firstMatch, true);
      ensureItemVisible(firstMatch);
    } else
      m_listView->selectAll(false);
  }

  // Get the number of visible nodes for the return code
  int cnt = 0;

  it = Q3ListViewItemIterator(m_listView, Q3ListViewItemIterator::Selectable | Q3ListViewItemIterator::Visible);
  while((it_v = it.current()) != 0) {
    cnt++;
    it++;
  }
  return cnt;
}

bool KMyMoneySelector::contains(const QString& txt) const
{
  Q3ListViewItemIterator it(m_listView, Q3ListViewItemIterator::Selectable);
  Q3ListViewItem* it_v;
  while((it_v = it.current()) != 0) {
    if(it_v->rtti() == 1) {
      KMyMoneyCheckListItem* it_c = dynamic_cast<KMyMoneyCheckListItem*>(it_v);
      if(it_c->text() == txt) {
        return true;
      }
    } else if(it_v->rtti() == 0) {
      KMyMoneyListViewItem* it_c = dynamic_cast<KMyMoneyListViewItem*>(it_v);
      if(it_c->text(0) == txt) {
        return true;
      }
    }
    it++;
  }
  return false;
}

void KMyMoneySelector::slotListRightMouse(Q3ListViewItem* it_v, const QPoint& pos, int /* col */)
{
  if(it_v && (it_v->rtti() == 1)) {
    KMyMoneyCheckListItem* it_c = static_cast<KMyMoneyCheckListItem*>(it_v);
    if(it_c->type() == Q3CheckListItem::CheckBox) {
      // the following is copied from QCheckListItem::activate() et al
      int boxsize = 0;
#warning "port to kde4"
#if 0
      int boxsize = m_listView->style().pixelMetric(QStyle::PM_CheckListButtonSize, m_listView);
#endif
      int align = m_listView->columnAlignment( 0 );
      int marg = m_listView->itemMargin();
      int y = 0;

      if ( align & Qt::AlignVCenter )
        y = ( ( height() - boxsize ) / 2 ) + marg;
      else
        y = (m_listView->fontMetrics().height() + 2 + marg - boxsize) / 2;

      QRect r( 0, y, boxsize-3, boxsize-3 );
      // columns might have been swapped
      r.moveBy( m_listView->header()->sectionPos( 0 ), 0 );

      QPoint topLeft = m_listView->itemRect(it_v).topLeft(); //### inefficient?
      QPoint p = m_listView->mapFromGlobal( pos ) - topLeft;

      int xdepth = m_listView->treeStepSize() * (it_v->depth() + (m_listView->rootIsDecorated() ? 1 : 0))
                   + m_listView->itemMargin();
      xdepth += m_listView->header()->sectionPos( m_listView->header()->mapToSection( 0 ) );
      p.rx() -= xdepth;
      // copy ends around here

      if ( r.contains( p ) ) {
        // we get down here, if we have a right click onto the checkbox
        selectAllSubItems(it_c, it_c->isOn());
      }
    }
  }
}

QStringList KMyMoneySelector::selectedItems(void) const
{
  QStringList list;
  selectedItems(list);
  return list;
}

QStringList KMyMoneySelector::itemList(void) const
{
  QStringList list;
  itemList(list);
  return list;
}

#include "kmymoneyselector.moc"
