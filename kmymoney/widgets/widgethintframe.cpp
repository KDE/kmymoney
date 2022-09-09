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
    QWidget* editWidget = nullptr;
    bool status = false;
    FrameStyle style;
};

WidgetHintFrame::WidgetHintFrame(QWidget* editWidget, FrameStyle style, Qt::WindowFlags f)
    : QFrame(editWidget->parentWidget(), f)
    , d(new Private)
{
    d->style = style;
    switch (style) {
    case Error:
        setStyleSheet("QFrame { background-color: none; padding: 1px; border: 2px solid red; border-radius: 4px; }");
        break;
    case Warning:
    case Info:
        setStyleSheet("QFrame { background-color: none; padding: 1px; border: 2px dashed red; border-radius: 4px; }");
        break;
    }
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
        move(w->pos() - QPoint(2, 2));
        resize(w->width() + 4, w->height() + 4);
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
            move(mev->pos() - QPoint(2, 2));
            break;

        case QEvent::Resize:
            sev = static_cast<QResizeEvent*>(e);
            resize(sev->size().width() + 4, sev->size().height() + 4);
            break;
        default:
            break;
        }
    }
    return QObject::eventFilter(o, e);
}
