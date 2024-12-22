/*
    SPDX-FileCopyrightText: 2025 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TABORDER_H
#define TABORDER_H

#include "kmm_base_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QStringList>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class QWidget;
class WidgetHintFrameCollection;
class TabOrderPrivate;

class KMM_BASE_WIDGETS_EXPORT TabOrder
{
public:
    explicit TabOrder(const QString& name, const QStringList& defaultTabOrder);

    /**
     * Setup the widget for which this object shall be used
     */
    void setWidget(QWidget* topLevelWidget);

    /**
     * Update the tabOrder
     */
    void setTabOrder(const QStringList& tabOrder);

    /**
     * Returns a pointer to the first widget in the topLevelWidget assigned
     * using setWidget according to the current taborder if available.
     * In case @a frameCollection is not @c nullptr the first widget with a
     * visible information will be selected.
     * In case the taborder property is not set,
     * topLevelWidget->focusWidget() will be returned.
     */
    QWidget* initialFocusWidget(WidgetHintFrameCollection* frameCollection) const;

    /**
     * Returns the next (@a next is true) or previous (@a next is false) widget
     * in the tab order starting from @a focusWidget. In case @a focusWidget is
     * @c nullptr it will be replaced with topLevelWidget->focusWidget().
     *
     * @sa setWidget
     */
    QWidget* tabFocusHelper(bool next, QWidget* focusWidget = nullptr);

private:
    TabOrderPrivate* d;
};

#endif // TABORDER_H
