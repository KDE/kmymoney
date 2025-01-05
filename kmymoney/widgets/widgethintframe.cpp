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

typedef enum {
    EditWidgetToolTip,
    HintFrameToolTip,
    ToolTipIndexCount,
} ToolTipIndex;

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
    WidgetHintFrame* frame = reinterpret_cast<WidgetHintFrame*>(o);
    if (frame) {
        d->frameList.removeAll(frame);
    }
}

void WidgetHintFrameCollection::updateWidgets()
{
    bool enabled = d->chainedCollectionState;
    for (const auto& frame : qAsConst(d->frameList)) {
        if (frame->isVisible()) {
            enabled &= !frame->isErroneous();
            if (!enabled) {
                break;
            }
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
        , ownToolTipChange(false)
        , style(Error)
        , offset(2)
    {
        toolTip.resize(ToolTipIndexCount);
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

    void selectToolTip(ToolTipIndex tipIndex)
    {
        if (editWidget && (tipIndex >= 0) && (tipIndex < ToolTipIndexCount)) {
            // inform eventFilter that we change the tooltip
            ownToolTipChange = true;
            editWidget->setToolTip(toolTip.at(tipIndex));
            ownToolTipChange = false;
        }
    }

    WidgetHintFrame* q;
    QWidget* editWidget;
    bool status;
    bool ownToolTipChange;
    FrameStyle style;
    int offset;
    QVector<QString> toolTip;
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

WidgetHintFrame* WidgetHintFrame::frameForWidget(QWidget* editWidget)
{
    if (editWidget && editWidget->parentWidget()) {
        const QList<WidgetHintFrame*> allErrorFrames = editWidget->parentWidget()->findChildren<WidgetHintFrame*>();
        for (const auto& f : qAsConst(allErrorFrames)) {
            if (f->editWidget() == editWidget) {
                return f;
            }
        }
    }
    return nullptr;
}

void WidgetHintFrame::show(QWidget* editWidget, const QString& tooltip)
{
    WidgetHintFrame* f = frameForWidget(editWidget);
    if (f) {
        if (!tooltip.isNull()) {
            f->setToolTip(tooltip);
        }
        if (editWidget->isVisible()) {
            f->QWidget::show();
            f->d->selectToolTip(HintFrameToolTip);
        }
        f->d->status = true;
        Q_EMIT f->changed();
    }
}

void WidgetHintFrame::hide(QWidget* editWidget, const QString& tooltip)
{
    WidgetHintFrame* f = frameForWidget(editWidget);
    if (f) {
        f->d->status = false;
        f->QWidget::hide();
        f->d->selectToolTip(EditWidgetToolTip);
        Q_EMIT f->changed();
    }
    if (!tooltip.isNull()) {
        editWidget->setToolTip(tooltip);
    }
}

QWidget* WidgetHintFrame::editWidget() const
{
    return d->editWidget;
}

void WidgetHintFrame::detachFromWidget()
{
    if (d->editWidget) {
        d->selectToolTip(EditWidgetToolTip);
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
        d->toolTip[EditWidgetToolTip] = d->editWidget->toolTip();
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
        QMoveEvent* mev = nullptr;
        QResizeEvent* sev = nullptr;
        const auto increment = d->offset * 2;

        switch (e->type()) {
        case QEvent::ToolTipChange:
            if (!d->ownToolTipChange) {
                // if initiated by application, keep a copy of the new text
                d->toolTip[EditWidgetToolTip] = d->editWidget->toolTip();
                // and overwrite with frame message if frame is visible
                if (d->editWidget->isEnabled() && d->status) {
                    QWidget::show();
                    d->selectToolTip(HintFrameToolTip);
                }
            }
            break;

        case QEvent::EnabledChange:
            // in case editWidget is enabled, the frame is shown when needed
            if (d->editWidget->isEnabled() && d->status) {
                QWidget::show();
                d->selectToolTip(HintFrameToolTip);
            } else {
                d->selectToolTip(EditWidgetToolTip);
                QWidget::hide();
            }
            break;

        case QEvent::Hide:
            // in case the editWidget is hidden, the frame needs to be hidden too
            QWidget::hide();
            break;

        case QEvent::Show:
            // in case editWidget is shown, the frame is also shown when needed
            if (d->status) {
                d->selectToolTip(HintFrameToolTip);
                QWidget::show();
            } else {
                d->selectToolTip(EditWidgetToolTip);
            }
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

void WidgetHintFrame::setToolTip(const QString& tooltip)
{
    d->toolTip[HintFrameToolTip] = tooltip;
}
