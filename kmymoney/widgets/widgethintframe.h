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
     * Set the @a style of the frame. For the stlye @c Focus
     * the @c offset will be set to 0 for all other styles
     * it will be set to 2.
     *
     * @sa setOffset()
     */
    void setStyle(FrameStyle style);

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
     * (the default) the @a editWidget's tooltip will not be changed.
     */
    static void hide(QWidget* editWidget, const QString& tooltip = QString());

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
    explicit WidgetHintFrameCollection(QObject* parent = 0);
    ~WidgetHintFrameCollection();

    void addFrame(WidgetHintFrame* frame);
    void addWidget(QWidget* w);
    void removeWidget(QWidget* w);

    /**
     * Connect the @a chainedCollection so that its result affects this
     * collection. Only one collection can be chained.
     *
     * @returns true if the connection was setup correctly.
     */
    bool chainFrameCollection(WidgetHintFrameCollection* chainedCollection);

protected:
    void connectNotify(const QMetaMethod& signal) override;

protected Q_SLOTS:
    virtual void unchainFrameCollection();
    virtual void frameDestroyed(QObject* o);
    virtual void changeChainedCollectionState(bool valid);
    virtual void updateWidgets();

Q_SIGNALS:
    void inputIsValid(bool valid);

private:
    class Private;
    Private* const d;
};

#endif // WIDGETHINTFRAME_H
