/*
    SPDX-FileCopyrightText: 2014 Cristian Oneț <onet.cristian@gmail.com>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

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

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const final override {
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
        m_pub(pub),
        m_parent(parent) {
    }

    void syncExpanded(const QModelIndex& parentIndex = QModelIndex()) {
        const int rows = m_parent->model()->rowCount(parentIndex);
        for (auto i = 0; i < rows; ++i) {
            const QModelIndex &index = m_parent->model()->index(i, 0, parentIndex);
            if (m_parent->isExpanded(index)) {
                m_pub->expand(index);
                syncExpanded(index);
            }
        }
    }

    void syncModels() {
        if (m_pub->model() != m_parent->model()) {
            // set the model
            m_pub->setModel(m_parent->model());

            // hide all but the first column
            for (int col = 1; col < m_pub->model()->columnCount(); ++col)
                m_pub->setColumnHidden(col, true);

            // set the selection model
            m_pub->setSelectionModel(m_parent->selectionModel());

            // when the model has changed we need to sync the expanded state of the views
            syncExpanded();
        }
    }

    void syncProperties() {
        //pub->setAllColumnsShowFocus(parent->allColumnsShowFocus());
        m_pub->setAlternatingRowColors(m_parent->alternatingRowColors());
        m_pub->setIconSize(m_parent->iconSize());
        m_pub->setSortingEnabled(m_parent->isSortingEnabled());
        m_pub->setUniformRowHeights(m_pub->uniformRowHeights());
    }

    void syncGeometry() {
        // update the geometry of the fixed column view to match that of the source model's geometry
        m_pub->setGeometry(m_parent->frameWidth(), m_parent->frameWidth(), m_parent->columnWidth(0),
                           m_parent->viewport()->height() + (m_parent->header()->isVisible() ? m_parent->header()->height() : 0));
    }

    FixedColumnTreeView *m_pub;
    QTreeView *m_parent;
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
    setItemDelegate(new FixedColumnDelegate(this, d->m_parent));

    // stack the source view under the fixed column view
    d->m_parent->viewport()->stackUnder(this);

    // the resize mode of the fixed view needs to be fixed to allow a user resize only from the parent tree
    header()->sectionResizeMode(QHeaderView::Fixed);

    // connect the scroll bars to keep the two views in sync
    connect(verticalScrollBar(), &QAbstractSlider::valueChanged, d->m_parent->verticalScrollBar(), &QAbstractSlider::setValue);
    connect(d->m_parent->verticalScrollBar(), &QAbstractSlider::valueChanged, verticalScrollBar(), &QAbstractSlider::setValue);

    // keep the expanded/collapsed states synchronized between the two views
    connect(d->m_parent, &QTreeView::expanded, this, &FixedColumnTreeView::onExpanded);
    connect(this, &QTreeView::expanded, this, &FixedColumnTreeView::onExpanded);
    connect(d->m_parent, &QTreeView::collapsed, this, &FixedColumnTreeView::onCollapsed);
    connect(this, &QTreeView::collapsed, this, &FixedColumnTreeView::onCollapsed);

    // keep the sort indicators synchronized between the two views
    connect(d->m_parent->header(), &QHeaderView::sortIndicatorChanged, this, &FixedColumnTreeView::updateSortIndicator);
    connect(header(), &QHeaderView::sortIndicatorChanged, this, &FixedColumnTreeView::updateSortIndicator);

    // forward all signals
    connect(this, &QAbstractItemView::activated, d->m_parent, &QAbstractItemView::activated);
    connect(this, &QAbstractItemView::clicked, d->m_parent, &QAbstractItemView::clicked);
    connect(this, &QAbstractItemView::doubleClicked, d->m_parent, &QAbstractItemView::doubleClicked);
    connect(this, &QAbstractItemView::entered, d->m_parent, &QAbstractItemView::entered);
    connect(this, &QAbstractItemView::pressed, d->m_parent, &QAbstractItemView::pressed);
    connect(this, &QAbstractItemView::viewportEntered, d->m_parent, &QAbstractItemView::viewportEntered);

    // handle the resize of the first column in the source view
    connect(d->m_parent->header(), &QHeaderView::sectionResized, this, &FixedColumnTreeView::updateSectionWidth);

    // forward right click to the source list
    setContextMenuPolicy(d->m_parent->contextMenuPolicy());
    if (contextMenuPolicy() == Qt::CustomContextMenu) {
        connect(this, &QWidget::customContextMenuRequested, d->m_parent, &QWidget::customContextMenuRequested);
    }

    // enable hover indicator synchronization between the two views
    d->m_parent->viewport()->installEventFilter(this);
    d->m_parent->viewport()->setMouseTracking(true);
    viewport()->setMouseTracking(true);

    d->syncProperties();

    if (d->m_parent->isVisible()) {
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
    if (sender() == this && !d->m_parent->isExpanded(index)) {
        d->m_parent->expand(index);
    }

    if (sender() == d->m_parent && !isExpanded(index)) {
        expand(index);
    }
}

void FixedColumnTreeView::onCollapsed(const QModelIndex& index)
{
    if (sender() == this && d->m_parent->isExpanded(index)) {
        d->m_parent->collapse(index);
    }

    if (sender() == d->m_parent && isExpanded(index)) {
        collapse(index);
    }
}

bool FixedColumnTreeView::viewportEvent(QEvent *event)
{
    if (underMouse()) {
        // forward mouse move and hover leave events to the source list
        if (event->type() == QEvent::MouseMove || event->type() == QEvent::HoverLeave) {
            QApplication::sendEvent(d->m_parent->viewport(), event);
        }
    }
    return QTreeView::viewportEvent(event);
}

bool FixedColumnTreeView::eventFilter(QObject *object, QEvent *event)
{
    if (object == d->m_parent->viewport()) {
        switch (event->type()) {
        case QEvent::MouseMove:
            if (!underMouse() && d->m_parent->underMouse()) {
                QMouseEvent *me = static_cast<QMouseEvent*>(event);
                // translate the position of the event but don't send buttons or modifiers because we only need the movement for the hover
                QMouseEvent translatedMouseEvent(me->type(), QPoint(width() - 2, me->pos().y()), Qt::NoButton, Qt::MouseButtons(), Qt::KeyboardModifiers());
                QApplication::sendEvent(viewport(), &translatedMouseEvent);
            }
            break;
        case QEvent::HoverLeave:
            if (!underMouse() && d->m_parent->underMouse()) {
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
        const int maxFirstColumnWidth = d->m_parent->width() * 0.8;
        if (newSize > maxFirstColumnWidth) {
            // limit the size of the first column so that it will not become larger than the view's width
            d->m_parent->setColumnWidth(logicalIndex, maxFirstColumnWidth);
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
    if (sender() == header() && d->m_parent->header()->sortIndicatorSection() != logicalIndex) {
        d->m_parent->header()->setSortIndicator(logicalIndex, order);
    }

    if (sender() == d->m_parent->header() && header()->sortIndicatorSection() != logicalIndex) {
        header()->setSortIndicator(logicalIndex, order);
    }
}
