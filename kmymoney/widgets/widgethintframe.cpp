/*
    SPDX-FileCopyrightText: 2015-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "widgethintframe.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QApplication>
#include <QEvent>
#include <QMetaMethod>
#include <QMoveEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QProxyStyle>
#include <QStyleOption>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

typedef enum {
    EditWidgetToolTip,
    HintFrameToolTip,
    ToolTipIndexCount,
} ToolTipIndex;

class WidgetHintFrameStyle : public QProxyStyle
{
    Q_OBJECT

public:
    WidgetHintFrameStyle();
    ~WidgetHintFrameStyle() = default;

    void drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const override;

    static WidgetHintFrameStyle* instance();
};

WidgetHintFrameStyle* WidgetHintFrameStyle::instance()
{
    static WidgetHintFrameStyle* style = nullptr;
    if (style == nullptr) {
        style = new WidgetHintFrameStyle;
        style->setParent(qApp);
    }
    return style;
}

WidgetHintFrameStyle::WidgetHintFrameStyle()
    : QProxyStyle(QApplication::style())
{
}

void WidgetHintFrameStyle::drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    static constexpr int FrameRadius = 5;

    if (element == CE_ShapedFrame) {
        WidgetHintFrame* frame = const_cast<WidgetHintFrame*>(qobject_cast<const WidgetHintFrame*>(widget));
        if (frame) {
            QPen pen;
            auto frameColor = QColor("red");
            pen.setStyle(Qt::SolidLine);

            switch (frame->hintFrameStyle()) {
            case WidgetHintFrame::Error:
                break;
            case WidgetHintFrame::Warning:
                pen.setStyle(Qt::DashLine);
                break;
            case WidgetHintFrame::Info:
                pen.setStyle(Qt::DashLine);
                frameColor = QColor("blue");
                break;

            case WidgetHintFrame::Focus:
                QProxyStyle::drawControl(CE_FocusFrame, option, painter, widget);
                return;
            }
            pen.setColor(frameColor);
            pen.setWidth(4);

            QPainterPath focusFramePath;
            focusFramePath.setFillRule(Qt::OddEvenFill);
            focusFramePath.addRoundedRect(option->rect, FrameRadius, FrameRadius);

            if (pen.style() != Qt::SolidLine) {
                QVector<qreal> pattern = {3.0, 3.0};
                QPainterPathStroker str;
                str.setCapStyle(Qt::FlatCap);
                str.setJoinStyle(Qt::RoundJoin);
                str.setMiterLimit(0);
                str.setDashPattern(pattern);
                str.setWidth(6.0);

                focusFramePath = str.createStroke(focusFramePath);
            }

            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            painter->setPen(pen);
            painter->fillPath(focusFramePath, frameColor);
            painter->restore();

        } else {
            QProxyStyle::drawControl(element, option, painter, widget);
        }

    } else {
        QProxyStyle::drawControl(element, option, painter, widget);
    }
}

class WidgetHintFrameCollection::Private
{
public:
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
        connect(frame, &WidgetHintFrame::changed, this, &WidgetHintFrameCollection::updateWidgets, Qt::QueuedConnection);
        d->frameList.append(frame);
    }
}

void WidgetHintFrameCollection::inheritFrameCollection(WidgetHintFrameCollection* chainedCollection)
{
    while (!chainedCollection->d->frameList.isEmpty()) {
        auto frame = chainedCollection->d->frameList.takeFirst();
        // remove it from the source
        disconnect(frame, &QObject::destroyed, chainedCollection, &WidgetHintFrameCollection::frameDestroyed);
        disconnect(frame, &WidgetHintFrame::changed, chainedCollection, &WidgetHintFrameCollection::updateWidgets);
        // and add it here
        addFrame(frame);
    }
}

void WidgetHintFrameCollection::addWidget(QWidget* w)
{
    connect(this, &WidgetHintFrameCollection::inputIsValid, w, &QWidget::setEnabled, Qt::UniqueConnection);
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
    bool enabled = true;
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

bool WidgetHintFrameCollection::isFrameVisible(QWidget* w) const
{
    for (const auto& frame : qAsConst(d->frameList)) {
        if (w == frame->editWidget()) {
            return frame->isFrameVisible();
        }
    }
    return false;
}

WidgetHintFrame* WidgetHintFrameCollection::frameForWidget(QWidget* w) const
{
    const auto it = std::find_if(d->frameList.cbegin(), d->frameList.cend(), [&](WidgetHintFrame* frame) {
        return frame->editWidget() == w;
    });
    return it != d->frameList.cend() ? *it : nullptr;
}

class WidgetHintFrame::Private
{
public:
    Private(WidgetHintFrame* qq)
        : q(qq)
        , m_editWidget(nullptr)
        , m_status(false)
        , m_ownToolTipChange(false)
        , m_frameStyle(Error)
        , m_offset(2)
    {
        m_toolTip.resize(ToolTipIndexCount);
    }

    void updateStyle()
    {
        q->setLineWidth(2);
        switch (m_frameStyle) {
        case Error:
            break;
        case Warning:
        case Info:
            // lineStyle = QLatin1String("dashed");
            break;
        case Focus:
            // color = QStringLiteral("#%1").arg(q->palette().color(QPalette::Active, QPalette::Highlight).rgb() & 0xFFFFFF, 6, 16, QLatin1Char('0'));
            q->setLineWidth(1);
            break;
        }
    }

    void selectToolTip(ToolTipIndex tipIndex)
    {
        if (m_editWidget && (tipIndex >= 0) && (tipIndex < ToolTipIndexCount)) {
            // inform eventFilter that we change the tooltip
            m_ownToolTipChange = true;
            m_editWidget->setToolTip(m_toolTip.at(tipIndex));
            m_ownToolTipChange = false;
        }
    }

    WidgetHintFrame* q;
    QWidget* m_editWidget;
    bool m_status;
    bool m_ownToolTipChange;
    FrameStyle m_frameStyle;
    int m_offset;
    QVector<QString> m_toolTip;
};

WidgetHintFrame::WidgetHintFrame(QWidget* editWidget, FrameStyle style, Qt::WindowFlags f)
    : QFrame(editWidget->parentWidget(), f)
    , d(new Private(this))
{
    setHintFrameStyle(style);
    QWidget::setStyle(WidgetHintFrameStyle::instance());
    attachToWidget(editWidget);
}

WidgetHintFrame::~WidgetHintFrame()
{
    delete d;
}

bool WidgetHintFrame::isFrameVisible() const
{
    return d->m_status;
}

bool WidgetHintFrame::isErroneous() const
{
    return (d->m_frameStyle == Error) && (d->m_status == true);
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
        f->d->m_status = true;
        Q_EMIT f->changed();
    }
}

void WidgetHintFrame::hide(QWidget* editWidget, const QString& tooltip)
{
    WidgetHintFrame* f = frameForWidget(editWidget);
    if (f) {
        f->d->m_status = false;
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
    return d->m_editWidget;
}

void WidgetHintFrame::detachFromWidget()
{
    if (d->m_editWidget) {
        d->selectToolTip(EditWidgetToolTip);
        d->m_editWidget->removeEventFilter(this);
        d->m_editWidget = nullptr;
    }
}

void WidgetHintFrame::attachToWidget(QWidget* w)
{
    // detach first
    detachFromWidget();
    if (w) {
        d->m_editWidget = w;
        d->m_toolTip[EditWidgetToolTip] = d->m_editWidget->toolTip();
        // make sure we receive changes in position and size
        w->installEventFilter(this);
        // place frame around widget
        move(w->pos() - QPoint(d->m_offset, d->m_offset));
        const auto increment = d->m_offset * 2;
        resize(w->width() + increment, w->height() + increment);
        // make sure widget is on top of frame
        w->raise();
        // and hide frame for now
        QWidget::hide();
    }
}

bool WidgetHintFrame::eventFilter(QObject* o, QEvent* e)
{
    if (o == d->m_editWidget) {
        QMoveEvent* mev = nullptr;
        QResizeEvent* sev = nullptr;
        const auto increment = d->m_offset * 2;

        switch (e->type()) {
        case QEvent::ToolTipChange:
            if (!d->m_ownToolTipChange) {
                // if initiated by application, keep a copy of the new text
                d->m_toolTip[EditWidgetToolTip] = d->m_editWidget->toolTip();
                // and overwrite with frame message if frame is visible
                if (d->m_editWidget->isEnabled() && d->m_status) {
                    QWidget::show();
                    d->selectToolTip(HintFrameToolTip);
                }
            }
            break;

        case QEvent::EnabledChange:
            // in case editWidget is enabled, the frame is shown when needed
            if (d->m_editWidget->isEnabled() && d->m_status) {
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
            if (d->m_status) {
                d->selectToolTip(HintFrameToolTip);
                QWidget::show();
            } else {
                d->selectToolTip(EditWidgetToolTip);
            }
            break;

        case QEvent::Move:
            mev = static_cast<QMoveEvent*>(e);
            move(mev->pos() - QPoint(d->m_offset, d->m_offset));
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
    d->m_offset = offset;
    // update frame position around widget
    if (d->m_editWidget) {
        move(d->m_editWidget->pos() - QPoint(d->m_offset, d->m_offset));
        const auto increment = d->m_offset * 2;
        resize(d->m_editWidget->width() + increment, d->m_editWidget->height() + increment);
    }
}

void WidgetHintFrame::setHintFrameStyle(WidgetHintFrame::FrameStyle frameStyle)
{
    setFrameStyle(QFrame::Box);
    d->m_frameStyle = frameStyle;
    switch (frameStyle) {
    default:
        setOffset(2);
        break;
    case Focus:
        setOffset(0);
        break;
    }
    d->updateStyle();
}

WidgetHintFrame::FrameStyle WidgetHintFrame::hintFrameStyle() const
{
    return d->m_frameStyle;
}

void WidgetHintFrame::setToolTip(const QString& tooltip)
{
    d->m_toolTip[HintFrameToolTip] = tooltip;
}

#include "widgethintframe.moc"
