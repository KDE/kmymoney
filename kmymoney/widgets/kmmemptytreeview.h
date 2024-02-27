/*
    SPDX-FileCopyrightText: 2024 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMMEMPTYTREEVIEW_H
#define KMMEMPTYTREEVIEW_H

#include "kmm_base_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QTreeView>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

/**
 * This class provides a QTreeView that prints a predefined message
 * on the viewport in case there is no data in the underlying model.
 * This could either be, because there is no data in the model or it
 * has been filtered by a search filter.
 *
 * Using the setSkipRootLevelEntries() method allows skip counting
 * the root level entries so that if you have 5 groups in a tree
 * view but none of them has children the message will still be
 * displayed.
 *
 * Using the setText() method the application can overwrite the
 * predefined message "No data found".
 */
class KMM_BASE_WIDGETS_EXPORT KMMEmptyTreeView : public QTreeView
{
    Q_OBJECT

public:
    explicit KMMEmptyTreeView(QWidget* parent = nullptr);
    ~KMMEmptyTreeView();

    /**
     * Use @a text as the message to be shown when the
     * underlying model does not have items
     *
     * @param text Text to be displayed
     */
    void setText(const QString& text);

    /**
     * If @a skipRootLevelEntries is true, even if root
     * level entries are present the message will be
     * displayed when none of them has children.
     *
     * @param skipRootLevelEntries
     */
    void setSkipRootLevelEntries(bool skipRootLevelEntries);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    class Private;
    Private* const d;
};

#endif // KMMEMPTYTREEVIEW_
