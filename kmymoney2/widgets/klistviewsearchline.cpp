/* This file is part of the KDE libraries
   Copyright (c) 2003 Scott Wheeler <wheeler@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "klistviewsearchline.h"

#include <k3listview.h>
#include <kiconloader.h>
#include <ktoolbar.h>

#include <kdebug.h>
#include <klocale.h>

#include <qapplication.h>
#include <qtimer.h>
#include <q3popupmenu.h>
#include <qlabel.h>
#include <q3header.h>
//Added by qt3to4:
#include <Q3ValueList>

#define KLISTVIEWSEARCHLINE_ALLVISIBLECOLUMNS_ID 2004

class K3ListViewSearchLine::KListViewSearchLinePrivate
{
public:
    KListViewSearchLinePrivate() :
        listView(0),
        caseSensitive(false),
        activeSearch(false),
        keepParentsVisible(true),
        queuedSearches(0) {}

    K3ListView *listView;
    bool caseSensitive;
    bool activeSearch;
    bool keepParentsVisible;
    QString search;
    int queuedSearches;
    Q3ValueList<int> searchColumns;
};

////////////////////////////////////////////////////////////////////////////////
// public methods
////////////////////////////////////////////////////////////////////////////////

K3ListViewSearchLine::K3ListViewSearchLine(QWidget *parent, K3ListView *listView, const char *name) :
    KLineEdit(parent, name)
{
    d = new KListViewSearchLinePrivate;

    d->listView = listView;

    connect(this, SIGNAL(textChanged(const QString &)),
            this, SLOT(queueSearch(const QString &)));

    if(listView) {
        connect(listView, SIGNAL(destroyed()),
                this, SLOT(listViewDeleted()));

#if KDE_IS_VERSION(3,3,0)
        connect(listView, SIGNAL(itemAdded(Q3ListViewItem *)),
                this, SLOT(itemAdded(Q3ListViewItem *)));
#endif
    }
    else
        setEnabled(false);
}

K3ListViewSearchLine::K3ListViewSearchLine(QWidget *parent, const char *name) :
    KLineEdit(parent, name)
{
    d = new KListViewSearchLinePrivate;

    d->listView = 0;

    connect(this, SIGNAL(textChanged(const QString &)),
            this, SLOT(queueSearch(const QString &)));

    setEnabled(false);
}

K3ListViewSearchLine::~K3ListViewSearchLine()
{
    delete d;
}

bool K3ListViewSearchLine::caseSensitive() const
{
    return d->caseSensitive;
}

Q3ValueList<int> K3ListViewSearchLine::searchColumns() const
{
    return d->searchColumns;
}

bool K3ListViewSearchLine::keepParentsVisible() const
{
    return d->keepParentsVisible;
}

K3ListView *K3ListViewSearchLine::listView() const
{
    return d->listView;
}

////////////////////////////////////////////////////////////////////////////////
// public slots
////////////////////////////////////////////////////////////////////////////////

void K3ListViewSearchLine::updateSearch(const QString &s)
{
    if(!d->listView)
        return;

    d->search = s.isNull() ? text() : s;

    // If there's a selected item that is visible, make sure that it's visible
    // when the search changes too (assuming that it still matches).

    Q3ListViewItem *currentItem = 0;

    switch(d->listView->selectionMode())
    {
    case K3ListView::NoSelection:
        break;
    case K3ListView::Single:
        currentItem = d->listView->selectedItem();
        break;
    default:
    {
        int flags = Q3ListViewItemIterator::Selected | Q3ListViewItemIterator::Visible;
        for(Q3ListViewItemIterator it(d->listView, flags);
            it.current() && !currentItem;
            ++it)
        {
            if(d->listView->itemRect(it.current()).isValid())
                currentItem = it.current();
        }
    }
    }

    if(d->keepParentsVisible)
        checkItemParentsVisible(d->listView->firstChild());
    else
        checkItemParentsNotVisible();

    if(currentItem)
        d->listView->ensureItemVisible(currentItem);
}

void K3ListViewSearchLine::setCaseSensitive(bool cs)
{
    d->caseSensitive = cs;
}

void K3ListViewSearchLine::setKeepParentsVisible(bool v)
{
    d->keepParentsVisible = v;
}

void K3ListViewSearchLine::setSearchColumns(const Q3ValueList<int> &columns)
{
    d->searchColumns = columns;
}

void K3ListViewSearchLine::setListView(K3ListView *lv)
{
    if(d->listView) {
        disconnect(d->listView, SIGNAL(destroyed()),
                   this, SLOT(listViewDeleted()));

#if KDE_IS_VERSION(3,3,0)
        disconnect(d->listView, SIGNAL(itemAdded(Q3ListViewItem *)),
                   this, SLOT(itemAdded(Q3ListViewItem *)));
#endif
    }

    d->listView = lv;

    if(lv) {
        connect(d->listView, SIGNAL(destroyed()),
                this, SLOT(listViewDeleted()));

#if KDE_IS_VERSION(3,3,0)
        connect(d->listView, SIGNAL(itemAdded(Q3ListViewItem *)),
                this, SLOT(itemAdded(Q3ListViewItem *)));
#endif
    }

    setEnabled(bool(lv));
}

////////////////////////////////////////////////////////////////////////////////
// protected members
////////////////////////////////////////////////////////////////////////////////

bool K3ListViewSearchLine::itemMatches(const Q3ListViewItem *item, const QString &s) const
{
    if(s.isEmpty())
        return true;

    // If the search column list is populated, search just the columns
    // specifified.  If it is empty default to searching all of the columns.

    if(!d->searchColumns.isEmpty()) {
        Q3ValueList<int>::ConstIterator it = d->searchColumns.begin();
        for(; it != d->searchColumns.end(); ++it) {
            if(*it < item->listView()->columns() &&
               item->text(*it).find(s, 0, d->caseSensitive) >= 0)
                return true;
        }
    }
    else {
        for(int i = 0; i < item->listView()->columns(); i++) {
            if(item->listView()->columnWidth(i) > 0 &&
               item->text(i).find(s, 0, d->caseSensitive) >= 0)
            {
                return true;
            }
        }
    }

    return false;
}

Q3PopupMenu *K3ListViewSearchLine::createPopupMenu()
{
    Q3PopupMenu *popup = KLineEdit::createPopupMenu();

    if (d->listView->columns()>1) {
        Q3PopupMenu *subMenu = new Q3PopupMenu(popup);
        connect(subMenu, SIGNAL(activated(int)), this, SLOT(searchColumnsMenuActivated(int)));

        popup->insertSeparator();
        popup->insertItem(i18n("Search Columns"), subMenu);

        subMenu->insertItem(i18n("All Visible Columns"), KLISTVIEWSEARCHLINE_ALLVISIBLECOLUMNS_ID);
        subMenu->insertSeparator();

        bool allColumnsAreSearchColumns = true;
  // TODO Make the entry order match the actual column order
  Q3Header* const header = d->listView->header();
  int visibleColumns = 0;
  for(int i = 0; i < d->listView->columns(); i++) {
      if(d->listView->columnWidth(i)>0) {
          QString columnText = d->listView->columnText(i);
          if(columnText.isEmpty()) {
        int visiblePosition=1;
        for(int j = 0; j < header->mapToIndex(i); j++)
            if(d->listView->columnWidth(header->mapToSection(j))>0)
                visiblePosition++;
        columnText = i18n("Column number %1","Column No. %1").arg(visiblePosition);
          }
                subMenu->insertItem(columnText, visibleColumns);
          if(d->searchColumns.isEmpty() || d->searchColumns.find(i) != d->searchColumns.end())
        subMenu->setItemChecked(visibleColumns, true);
                else
                    allColumnsAreSearchColumns = false;
          visibleColumns++;
      }
        }
        subMenu->setItemChecked(KLISTVIEWSEARCHLINE_ALLVISIBLECOLUMNS_ID, allColumnsAreSearchColumns);

        // searchColumnsMenuActivated() relies on one possible "all" representation
        if(allColumnsAreSearchColumns && !d->searchColumns.isEmpty())
            d->searchColumns.clear();
    }

    return popup;
}

////////////////////////////////////////////////////////////////////////////////
// protected slots
////////////////////////////////////////////////////////////////////////////////

void K3ListViewSearchLine::queueSearch(const QString &search)
{
    d->queuedSearches++;
    d->search = search;
    QTimer::singleShot(200, this, SLOT(activateSearch()));
}

void K3ListViewSearchLine::activateSearch()
{
    --(d->queuedSearches);

    if(d->queuedSearches == 0)
        updateSearch(d->search);
}

////////////////////////////////////////////////////////////////////////////////
// private slots
////////////////////////////////////////////////////////////////////////////////

void K3ListViewSearchLine::itemAdded(Q3ListViewItem *item) const
{
    item->setVisible(itemMatches(item, text()));
}

void K3ListViewSearchLine::listViewDeleted()
{
    d->listView = 0;
    setEnabled(false);
}

void K3ListViewSearchLine::searchColumnsMenuActivated(int id)
{
    if(id == KLISTVIEWSEARCHLINE_ALLVISIBLECOLUMNS_ID) {
        if(d->searchColumns.isEmpty())
            d->searchColumns.append(0);
        else
            d->searchColumns.clear();
    }
    else {
        if(d->searchColumns.find(id) != d->searchColumns.end())
            d->searchColumns.remove(id);
        else {
            if(d->searchColumns.isEmpty()) {
                for(int i = 0; i < d->listView->columns(); i++) {
                    if(i != id)
                        d->searchColumns.append(i);
                }
            }
            else
                d->searchColumns.append(id);
        }
    }
    updateSearch();
}

////////////////////////////////////////////////////////////////////////////////
// private methods
////////////////////////////////////////////////////////////////////////////////

void K3ListViewSearchLine::checkItemParentsNotVisible()
{
    Q3ListViewItemIterator it(d->listView);
    for(; it.current(); ++it)
    {
        Q3ListViewItem *item = it.current();
        if(itemMatches(item, d->search))
            item->setVisible(true);
        else
            item->setVisible(false);
    }
}

#include <kdebug.h>

/** Check whether \p item, its siblings and their descendents should be shown. Show or hide the items as necessary.
 *
 *  \p item  The list view item to start showing / hiding items at. Typically, this is the first child of another item, or the
 *              the first child of the list view.
 *  \p highestHiddenParent  The highest (closest to root) ancestor of \p item which is hidden. If 0, all parents of
 *                           \p item must be visible.
 *  \return \c true if an item which should be visible is found, \c false if all items found should be hidden. If this function
 *             returns true and \p highestHiddenParent was not 0, highestHiddenParent will have been shown.
 */
bool K3ListViewSearchLine::checkItemParentsVisible(Q3ListViewItem *item, Q3ListViewItem *highestHiddenParent)
{
    bool visible = false;
    Q3ListViewItem * first = item;
    for(; item; item = item->nextSibling())
    {
        //What we pass to our children as highestHiddenParent:
        Q3ListViewItem * hhp = highestHiddenParent ? highestHiddenParent : item->isVisible() ? 0L : item;
        bool childMatch = false;
        if(item->firstChild() && checkItemParentsVisible(item->firstChild(), hhp))
            childMatch = true;
        // Should this item be shown? It should if any children should be, or if it matches.
        if(childMatch || itemMatches(item, d->search))
        {
            visible = true;
            if (highestHiddenParent)
            {
                highestHiddenParent->setVisible(true);
                // Calling setVisible on our ancestor will unhide all its descendents. Hide the ones
                // before us that should not be shown.
                for(Q3ListViewItem *hide = first; hide != item; hide = hide->nextSibling())
                    hide->setVisible(false);
                highestHiddenParent = 0;
                // If we matched, than none of our children matched, yet the setVisible() call on our
                // ancestor unhid them, undo the damage:
                if(!childMatch)
                    for(Q3ListViewItem *hide = item->firstChild(); hide; hide = hide->nextSibling())
                        hide->setVisible(false);
            }
            else
                item->setVisible(true);
        }
        else
            item->setVisible(false);
    }
    return visible;
}

////////////////////////////////////////////////////////////////////////////////
// K3ListViewSearchLineWidget
////////////////////////////////////////////////////////////////////////////////

class K3ListViewSearchLineWidget::KListViewSearchLineWidgetPrivate
{
public:
    KListViewSearchLineWidgetPrivate() : listView(0), searchLine(0), clearButton(0) {}
    K3ListView *listView;
    K3ListViewSearchLine *searchLine;
    QToolButton *clearButton;
};

K3ListViewSearchLineWidget::K3ListViewSearchLineWidget(K3ListView *listView,
                                                     QWidget *parent,
                                                     const char *name) :
    Q3HBox(parent, name)
{
    d = new KListViewSearchLineWidgetPrivate;
    d->listView = listView;

    setSpacing(5);

    QTimer::singleShot(0, this, SLOT(createWidgets()));
}

K3ListViewSearchLineWidget::~K3ListViewSearchLineWidget()
{
    delete d;
}

K3ListViewSearchLine *K3ListViewSearchLineWidget::createSearchLine(K3ListView *listView)
{
    if(!d->searchLine)
        d->searchLine = new K3ListViewSearchLine(this, listView);
    return d->searchLine;
}

void K3ListViewSearchLineWidget::createWidgets()
{
    positionInToolBar();

    if(!d->clearButton) {
        d->clearButton = new QToolButton(this);
        QIcon icon = SmallIconSet(QApplication::isRightToLeft() ? "clear_left" : "locationbar_erase");
        d->clearButton->setIconSet(icon);
    }

    d->clearButton->show();

    QLabel *label = new QLabel(i18n("S&earch:"), this, "kde toolbar widget");

    d->searchLine = createSearchLine(d->listView);
    d->searchLine->show();

    label->setBuddy(d->searchLine);
    label->show();

    connect(d->clearButton, SIGNAL(clicked()), d->searchLine, SLOT(clear()));
}

K3ListViewSearchLine *K3ListViewSearchLineWidget::searchLine() const
{
    return d->searchLine;
}

void K3ListViewSearchLineWidget::positionInToolBar()
{
    KToolBar *toolBar = dynamic_cast<KToolBar *>(parent());

    if(toolBar) {

        // Here we have The Big Ugly.  Figure out how many widgets are in the
        // and do a hack-ish iteration over them to find this widget so that we
        // can insert the clear button before it.

        int widgetCount = toolBar->count();

        for(int index = 0; index < widgetCount; index++) {
            int id = toolBar->idAt(index);
            if(toolBar->getWidget(id) == this) {
                toolBar->setItemAutoSized(id);
                if(!d->clearButton) {
                    QString icon = QApplication::isRightToLeft() ? "clear_left" : "locationbar_erase";
                    d->clearButton = new KToolBarButton(icon, 2005, toolBar);
                }
                toolBar->insertWidget(2005, d->clearButton->width(), d->clearButton, index);
                break;
            }
        }
    }

    if(d->searchLine)
        d->searchLine->show();
}

#include "klistviewsearchline.moc"
