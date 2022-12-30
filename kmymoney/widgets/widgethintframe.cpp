/*
    SPDX-FileCopyrightText: 2015-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "widgethintframe.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QEvent>
#include <QMetaMethod>
#include <QMoveEvent>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class WidgetHintFrameCollection::Private
{
public:
    bool haveChainedCollection = false;
    bool chainedCollectionState = true;
    QList<WidgetHintFrame*> frameList;
};

WidgetHintFrameCollection::WidgetHintFrameCollection(QObject* parent)
    : QObject(parent)
    , d(new Private)
{
}

WidgetHintFrameCollection::~WidgetHintFrameCollection()
{
    delete d;
}

bool WidgetHintFrameCollection::chainFrameCollection(WidgetHintFrameCollection* chainedCollection)
{
    if ((chainedCollection == nullptr) || (d->haveChainedCollection)) {
        return false;
    }
    d->haveChainedCollection = true;
    connect(chainedCollection, &WidgetHintFrameCollection::inputIsValid, this, &WidgetHintFrameCollection::changeChainedCollectionState);
    connect(chainedCollection, &WidgetHintFrameCollection::destroyed, this, &WidgetHintFrameCollection::unchainFrameCollection);
    return true;
}

void WidgetHintFrameCollection::unchainFrameCollection()
{
    if (d->haveChainedCollection) {
        d->haveChainedCollection = false;
        d->chainedCollectionState = true;
    }
}

void WidgetHintFrameCollection::connectNotify(const QMetaMethod& signal)
{
    // Whenever a new object connects to our inputIsValid signal
    // we emit the current status right away.
    if (signal == QMetaMethod::fromSignal(&WidgetHintFrameCollection::inputIsValid)) {
        updateWidgets();
    }
}

void WidgetHintFrameCollection::addFrame(WidgetHintFrame* frame)
{
    if (!d->frameList.contains(frame)) {
        connect(frame, &QObject::destroyed, this, &WidgetHintFrameCollection::frameDestroyed);
        connect(frame, &WidgetHintFrame::changed, this, [=] {
            QMetaObject::invokeMethod(this, "updateWidgets", Qt::QueuedConnection);
        });
        d->frameList.append(frame);
    }
}

void WidgetHintFrameCollection::addWidget(QWidget* w)
{
    connect(this, &WidgetHintFrameCollection::inputIsValid, w, &QWidget::setEnabled, Qt::UniqueConnection);
}

void WidgetHintFrameCollection::changeChainedCollectionState(bool valid)
{
    d->chainedCollectionState = valid;
    updateWidgets();
}

void WidgetHintFrameCollection::removeWidget(QWidget* w)
{
    disconnect(this, &WidgetHintFrameCollection::inputIsValid, w, &QWidget::setEnabled);
    w->setEnabled(true);
}

void WidgetHintFrameCollection::frameDestroyed(QObject* o)
{
    WidgetHintFrame* frame = qobject_cast<WidgetHintFrame*>(o);
    if (frame) {
        d->frameList.removeAll(frame);
    }
}

void WidgetHintFrameCollection::updateWidgets()
{
    bool enabled = d->chainedCollectionState;
    for (const auto& frame : d->frameList) {
        enabled &= !frame->isErroneous();
        if (!enabled) {
            break;
        }
    }

    Q_EMIT inputIsValid(enabled);
}

class WidgetHintFrame::Private
{
public:
    Private(WidgetHintFrame* qq)
        : q(qq)
        , editWidget(nullptr)
        , status(false)
        , style(Error)
        , offset(2)
    {
    }

    void updateStyle()
    {
        QString color("red");
        QString width("2");
        QString lineStyle("solid");

        switch (style) {
        case Error:
            break;
        case Warning:
        case Info:
            lineStyle = QLatin1String("dashed");
            break;
        case Focus:
            color = QStringLiteral("#%1").arg(q->palette().color(QPalette::Active, QPalette::Highlight).rgb() & 0xFFFFFF, 6, 16, QLatin1Char('0'));
            width = QLatin1String("1");
            break;
        }
        q->setStyleSheet(
            QStringLiteral("QFrame { background-color: none; padding: 1px; border: %1px %2 %3; border-radius: 4px; }").arg(width, lineStyle, color));
    }

    WidgetHintFrame* q;
    QWidget* editWidget;
    bool status;
    FrameStyle style;
    int offset;
};

WidgetHintFrame::WidgetHintFrame(QWidget* editWidget, FrameStyle style, Qt::WindowFlags f)
    : QFrame(editWidget->parentWidget(), f)
    , d(new Private(this))
{
    setStyle(style);
    attachToWidget(editWidget);
}

WidgetHintFrame::~WidgetHintFrame()
{
    delete d;
}

bool WidgetHintFrame::isErroneous() const
{
    return (d->style == Error) && (d->status == true);
}

static WidgetHintFrame* frame(QWidget* editWidget)
{
    if (editWidget && editWidget->parentWidget()) {
        QList<WidgetHintFrame*> allErrorFrames = editWidget->parentWidget()->findChildren<WidgetHintFrame*>();
        QList<WidgetHintFrame*>::const_iterator it;
        for (const auto& f : allErrorFrames) {
            if (f->editWidget() == editWidget) {
                return f;
            }
        }
    }
    return nullptr;
}

void WidgetHintFrame::show(QWidget* editWidget, const QString& tooltip)
{
    WidgetHintFrame* f = frame(editWidget);
    if (f) {
        f->QWidget::show();
        f->d->status = true;
        Q_EMIT f->changed();
    }
    if (!tooltip.isNull())
        editWidget->setToolTip(tooltip);
}

void WidgetHintFrame::hide(QWidget* editWidget, const QString& tooltip)
{
    WidgetHintFrame* f = frame(editWidget);
    if (f) {
        f->QWidget::hide();
        f->d->status = false;
        Q_EMIT f->changed();
    }
    if (!tooltip.isNull())
        editWidget->setToolTip(tooltip);
}

QWidget* WidgetHintFrame::editWidget() const
{
    return d->editWidget;
}

void WidgetHintFrame::detachFromWidget()
{
    if (d->editWidget) {
        d->editWidget->removeEventFilter(this);
        d->editWidget = nullptr;
    }
}

void WidgetHintFrame::attachToWidget(QWidget* w)
{
    // detach first
    detachFromWidget();
    if (w) {
        d->editWidget = w;
        // make sure we receive changes in position and size
        w->installEventFilter(this);
        // place frame around widget
        move(w->pos() - QPoint(d->offset, d->offset));
        const auto increment = d->offset * 2;
        resize(w->width() + increment, w->height() + increment);
        // make sure widget is on top of frame
        w->raise();
        // and hide frame for now
        QWidget::hide();
    }
}

bool WidgetHintFrame::eventFilter(QObject* o, QEvent* e)
{
    if (o == d->editWidget) {
        QMoveEvent* mev = 0;
        QResizeEvent* sev = 0;
        const auto increment = d->offset * 2;
        switch (e->type()) {
        case QEvent::EnabledChange:
        case QEvent::Hide:
        case QEvent::Show:
            /**
             * @todo think about what to do when widget is enabled/disabled
             * hidden or shown
             */
            break;

        case QEvent::Move:
            mev = static_cast<QMoveEvent*>(e);
            move(mev->pos() - QPoint(d->offset, d->offset));
            break;

        case QEvent::Resize:
            sev = static_cast<QResizeEvent*>(e);
            resize(sev->size().width() + increment, sev->size().height() + increment);
            break;
        default:
            break;
        }
    }
    return QObject::eventFilter(o, e);
}

void WidgetHintFrame::setOffset(int offset)
{
    d->offset = offset;
    // update frame position around widget
    if (d->editWidget) {
        move(d->editWidget->pos() - QPoint(d->offset, d->offset));
        const auto increment = d->offset * 2;
        resize(d->editWidget->width() + increment, d->editWidget->height() + increment);
    }
}

void WidgetHintFrame::setStyle(WidgetHintFrame::FrameStyle style)
{
    d->style = style;
    switch (style) {
    default:
        setOffset(2);
        break;
    case Focus:
        setOffset(0);
        break;
    }
    d->updateStyle();
}
