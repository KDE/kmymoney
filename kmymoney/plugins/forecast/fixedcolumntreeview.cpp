/*
 * Copyright 2014  Cristian Oneț <onet.cristian@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "fixedcolumntreeview.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QEvent>

#include <QScrollBar>
#include <QStyledItemDelegate>
#include <QHeaderView>
#include <QApplication>
#include <QMouseEvent>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class FixedColumnDelegate : public QStyledItemDelegate
{
  QTreeView *m_sourceView;

public:
  explicit FixedColumnDelegate(FixedColumnTreeView *fixedColumView, QTreeView *sourceView) :
      QStyledItemDelegate(fixedColumView),
      m_sourceView(sourceView) {
  }

  virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QStyleOptionViewItem optV4 = option;
    initStyleOption(&optV4, index);
    // the fixed column's position has always this value
    optV4.viewItemPosition = QStyleOptionViewItem::Beginning;
    if (m_sourceView->hasFocus()) {
      // draw the current row as active if the source list has focus
      QModelIndex currentIndex = m_sourceView->currentIndex();
      if (currentIndex.isValid() && currentIndex.row() == index.row() && currentIndex.parent() == index.parent()) {
        optV4.state |= QStyle::State_Active;
      }
    }
    QStyledItemDelegate::paint(painter, optV4, index);
  }
};

struct FixedColumnTreeView::Private {
  Private(FixedColumnTreeView *pub, QTreeView *parent) :
      pub(pub),
      parent(parent) {
  }

  void syncExpanded(const QModelIndex& parentIndex = QModelIndex()) {
    const int rows = parent->model()->rowCount(parentIndex);
    for (auto i = 0; i < rows; ++i) {
      const QModelIndex &index = parent->model()->index(i, 0, parentIndex);
      if (parent->isExpanded(index)) {
        pub->expand(index);
        syncExpanded(index);
      }
    }
  }

  void syncModels() {
    if (pub->model() != parent->model()) {
      // set the model
      pub->setModel(parent->model());

      // hide all but the first column
      for (int col = 1; col < pub->model()->columnCount(); ++col)
        pub->setColumnHidden(col, true);

      // set the selection model
      pub->setSelectionModel(parent->selectionModel());

      // when the model has changed we need to sync the expanded state of the views
      syncExpanded();
    }
  }

  void syncProperties() {
    //pub->setAllColumnsShowFocus(parent->allColumnsShowFocus());
    pub->setAlternatingRowColors(parent->alternatingRowColors());
    pub->setIconSize(parent->iconSize());
    pub->setSortingEnabled(parent->isSortingEnabled());
    pub->setUniformRowHeights(pub->uniformRowHeights());
  }

  void syncGeometry() {
    // update the geometry of the fixed column view to match that of the source model's geometry
    pub->setGeometry(parent->frameWidth(), parent->frameWidth(), parent->columnWidth(0),
                     parent->viewport()->height() + (parent->header()->isVisible() ? parent->header()->height() : 0));
  }

  FixedColumnTreeView *pub;
  QTreeView *parent;
};

FixedColumnTreeView::FixedColumnTreeView(QTreeView *parent)
    : QTreeView(parent)
    , d(new Private(this, parent))
{
  // no borders and scrollbars for the fixed column view
  setStyleSheet("QTreeView { border: none; }");
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  // the focus proxy is forwarded to the source list
  setFocusProxy(parent);

  // perform the special selection and hover drawing even when the fixed column view has no focus
  setItemDelegate(new FixedColumnDelegate(this, d->parent));

  // stack the source view under the fixed column view
  d->parent->viewport()->stackUnder(this);

  // the resize mode of the fixed view needs to be fixed to allow a user resize only from the parent tree
  header()->sectionResizeMode(QHeaderView::Fixed);

  // connect the scroll bars to keep the two views in sync
  connect(verticalScrollBar(), &QAbstractSlider::valueChanged, d->parent->verticalScrollBar(), &QAbstractSlider::setValue);
  connect(d->parent->verticalScrollBar(), &QAbstractSlider::valueChanged, verticalScrollBar(), &QAbstractSlider::setValue);

  // keep the expanded/collapsed states synchronized between the two views
  connect(d->parent, &QTreeView::expanded, this, &FixedColumnTreeView::onExpanded);
  connect(this, &QTreeView::expanded, this, &FixedColumnTreeView::onExpanded);
  connect(d->parent, &QTreeView::collapsed, this, &FixedColumnTreeView::onCollapsed);
  connect(this, &QTreeView::collapsed, this, &FixedColumnTreeView::onCollapsed);

  // keep the sort indicators synchronized between the two views
  connect(d->parent->header(), &QHeaderView::sortIndicatorChanged, this, &FixedColumnTreeView::updateSortIndicator);
  connect(header(), &QHeaderView::sortIndicatorChanged, this, &FixedColumnTreeView::updateSortIndicator);

  // forward all signals
  connect(this, &QAbstractItemView::activated, d->parent, &QAbstractItemView::activated);
  connect(this, &QAbstractItemView::clicked, d->parent, &QAbstractItemView::clicked);
  connect(this, &QAbstractItemView::doubleClicked, d->parent, &QAbstractItemView::doubleClicked);
  connect(this, &QAbstractItemView::entered, d->parent, &QAbstractItemView::entered);
  connect(this, &QAbstractItemView::pressed, d->parent, &QAbstractItemView::pressed);
  connect(this, &QAbstractItemView::viewportEntered, d->parent, &QAbstractItemView::viewportEntered);

  // handle the resize of the first column in the source view
  connect(d->parent->header(), &QHeaderView::sectionResized, this, &FixedColumnTreeView::updateSectionWidth);

  // forward right click to the source list
  setContextMenuPolicy(d->parent->contextMenuPolicy());
  if (contextMenuPolicy() == Qt::CustomContextMenu) {
    connect(this, &QWidget::customContextMenuRequested, d->parent, &QWidget::customContextMenuRequested);
  }

  // enable hover indicator synchronization between the two views
  d->parent->viewport()->installEventFilter(this);
  d->parent->viewport()->setMouseTracking(true);
  viewport()->setMouseTracking(true);

  d->syncProperties();

  if (d->parent->isVisible()) {
    // the source view is already visible so show the frozen column also
    d->syncModels();
    show();
    d->syncGeometry();
  }
}

FixedColumnTreeView::~FixedColumnTreeView()
{
  delete d;
}

void FixedColumnTreeView::sourceModelUpdated()
{
  d->syncModels();
  d->syncGeometry();
}

void FixedColumnTreeView::onExpanded(const QModelIndex& index)
{
  if (sender() == this && !d->parent->isExpanded(index)) {
    d->parent->expand(index);
  }

  if (sender() == d->parent && !isExpanded(index)) {
    expand(index);
  }
}

void FixedColumnTreeView::onCollapsed(const QModelIndex& index)
{
  if (sender() == this && d->parent->isExpanded(index)) {
    d->parent->collapse(index);
  }

  if (sender() == d->parent && isExpanded(index)) {
    collapse(index);
  }
}

bool FixedColumnTreeView::viewportEvent(QEvent *event)
{
  if (underMouse()) {
    // forward mouse move and hover leave events to the source list
    if (event->type() == QEvent::MouseMove || event->type() == QEvent::HoverLeave) {
      QApplication::sendEvent(d->parent->viewport(), event);
    }
  }
  return QTreeView::viewportEvent(event);
}

bool FixedColumnTreeView::eventFilter(QObject *object, QEvent *event)
{
  if (object == d->parent->viewport()) {
    switch (event->type()) {
      case QEvent::MouseMove:
        if (!underMouse() && d->parent->underMouse()) {
          QMouseEvent *me = static_cast<QMouseEvent*>(event);
          // translate the position of the event but don't send buttons or modifiers because we only need the movement for the hover
          QMouseEvent translatedMouseEvent(me->type(), QPoint(width() - 2, me->pos().y()), Qt::NoButton, Qt::MouseButtons(), Qt::KeyboardModifiers());
          QApplication::sendEvent(viewport(), &translatedMouseEvent);
        }
        break;
      case QEvent::HoverLeave:
        if (!underMouse() && d->parent->underMouse()) {
          QApplication::sendEvent(viewport(), event);
        }
        break;
      case QEvent::Show:
        d->syncModels();
        show();
        // intentional fall through
      case QEvent::Resize:
        d->syncGeometry();
        break;
      default:
        break;
    }
  }
  return QTreeView::eventFilter(object, event);
}

void FixedColumnTreeView::updateSectionWidth(int logicalIndex, int, int newSize)
{
  if (logicalIndex == 0) {
    const int maxFirstColumnWidth = d->parent->width() * 0.8;
    if (newSize > maxFirstColumnWidth) {
      // limit the size of the first column so that it will not become larger than the view's width
      d->parent->setColumnWidth(logicalIndex, maxFirstColumnWidth);
    } else {
      // update the size of the fixed column
      setColumnWidth(0, newSize);
      // update the geometry
      d->syncGeometry();
    }
  }
}

void FixedColumnTreeView::updateSortIndicator(int logicalIndex, Qt::SortOrder order)
{
  if (sender() == header() && d->parent->header()->sortIndicatorSection() != logicalIndex) {
    d->parent->header()->setSortIndicator(logicalIndex, order);
  }

  if (sender() == d->parent->header() && header()->sortIndicatorSection() != logicalIndex) {
    header()->setSortIndicator(logicalIndex, order);
  }
}
