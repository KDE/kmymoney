/*
    SPDX-FileCopyrightText: 2015-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WIDGETHINTFRAME_H
#define WIDGETHINTFRAME_H

#include "kmm_base_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QFrame>
class QWidget;
class QMetaMethod;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class KMM_BASE_WIDGETS_EXPORT WidgetHintFrame : public QFrame
{
    Q_OBJECT

public:
    enum FrameStyle {
        Error = 0,
        Warning,
        Info,
        Focus,
    };
    Q_ENUM(FrameStyle)

    explicit WidgetHintFrame(QWidget* editWidget, FrameStyle style = Error, Qt::WindowFlags f = {});
    ~WidgetHintFrame();

    /**
     * Attach the WidgetHintFrame to the widget @a w.
     */
    void attachToWidget(QWidget* w);

    /**
     * Remove the frame from the widget it is attached to.
     * If no widget is attached, nothing will be done.
     */
    void detachFromWidget();

    /**
     * Set the @a style of the frame. For the style @c Focus
     * the @c offset will be set to 0 for all other styles
     * it will be set to 2.
     *
     * @sa setOffset(), hintFrameStyle()
     */
    void setHintFrameStyle(FrameStyle style);

    /**
     * Returns the selected hint style of the frame.
     *
     * @sa setHintFrameStyle()
     */
    FrameStyle hintFrameStyle() const;

    /**
     * Set the offset to be kept between the attached
     * widget and the frame in pixels. The setting will
     * be overridden by setStyle().
     *
     * @sa setStyle()
     */
    void setOffset(int offset);

    /**
     * Returns @c true when @c style is @c Error and
     * the frame is visible. Returns @c false otherwise.
     */
    bool isErroneous() const;

    /**
     * Returns a pointer to the widget surrounded by
     * the frame or @c nullptr if none is attached.
     *
     * @sa attachToWidget(), detachFromWidget()
     */
    QWidget* editWidget() const;

    /**
     * Set the @a tooltip that should be shown on editWidget when frame is visible.
     * A possible tooltip on the editWidget will be kept and re-assigned once
     * the frame is removed or hidden.
     *
     * @sa hide()
     */
    void setToolTip(const QString& tooltip);

    /**
     * Shows the info frame around @a editWidget and in case @a tooltip
     * is not null (@sa QString::isNull()) the respective message will
     * be loaded into the @a editWidget's tooltip. In case @a tooltip is null
     * (the default) the @a editWidget's tooltip will not be changed.
     */
    static void show(QWidget* editWidget, const QString& tooltip = QString());

    /**
     * Hides the info frame around @a editWidget and in case @a tooltip
     * is not null (@sa QString::isNull()) the respective message will
     * be loaded into the @a editWidget's tooltip. In case @a tooltip is null
     * (the default) the tooltip set using setToolTip() will be shown.
     *
     * @sa setToolTip()
     */
    static void hide(QWidget* editWidget, const QString& tooltip = QString());

    /**
     * Returns the pointer to the WidgetHintFrame around the
     * @a editWidget or @c nullptr if none is present.
     */
    static WidgetHintFrame* frameForWidget(QWidget* editWidget);

    /**
     * Provides information if a frame is currently shown or not
     *
     * @returns true if the frame is visible
     */
    bool isFrameVisible() const;

protected:
    bool eventFilter(QObject* o, QEvent* e) final override;

Q_SIGNALS:
    void changed();

private:
    class Private;
    Private* const d;
};

class KMM_BASE_WIDGETS_EXPORT WidgetHintFrameCollection : public QObject
{
    Q_OBJECT
public:
    explicit WidgetHintFrameCollection(QObject* parent = nullptr);
    ~WidgetHintFrameCollection();

    void addFrame(WidgetHintFrame* frame);
    void addWidget(QWidget* w);
    void removeWidget(QWidget* w);

    /**
     * Inherits the widgets from @a chainedCollection and removes them from
     * their original source
     */
    void inheritFrameCollection(WidgetHintFrameCollection* chainedCollection);

    /**
     * Provides information if a frame around the edit widget
     * @a w is currently shown or not.
     *
     * @returns true if the frame is visible
     */
    bool isFrameVisible(QWidget* w) const;

    /**
     * Returns the frame for widget @a w or @c nullptr
     * if no frame is found.
     */
    WidgetHintFrame* frameForWidget(QWidget* w) const;

protected:
    void connectNotify(const QMetaMethod& signal) override;

protected Q_SLOTS:
    virtual void frameDestroyed(QObject* o);
    virtual void updateWidgets();

Q_SIGNALS:
    void inputIsValid(bool valid);

private:
    class Private;
    Private* const d;
};

#endif // WIDGETHINTFRAME_H
