/*
    SPDX-FileCopyrightText: 2024 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMMEMPTYVIEW_H
#define KMMEMPTYVIEW_H

#include "kmm_base_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QListView>
#include <QPainter>
#include <QTableView>
#include <QTextBrowser>
#include <QTreeView>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

/**
 * This template used for QAbstractItemViews that print a predefined message
 * on the viewport in case there is no data in the underlying model.
 * This could either be, because there is no data in the model or it
 * has been filtered by a search filter.
 *
 * For QTreeViews using the setSkipRootLevelEntries() method allows
 * skip counting the root level entries so that if you have 5 groups
 * in a tree view but none of them has children the message will still be
 * displayed.
 *
 * Using the setText() method the application can overwrite the
 * predefined message "No data found".
 */
template<typename T>
class KMMEmptyView : public T
{
    class Private
    {
    public:
        Private()
            : m_text(i18nc("Placeholder for when there is no data in a view", "No data found"))
            , m_skipRootLevelEntries(false)
        {
        }

        bool isModelEmpty(QAbstractItemModel* model) const
        {
            if (model != nullptr) {
                if (m_skipRootLevelEntries) {
                    const auto rows = model->rowCount();
                    for (int row = 0; row < rows; ++row) {
                        const auto idx = model->index(row, 0);
                        if (model->rowCount(idx) > 0) {
                            return false;
                        }
                    }
                    return true;
                }
                return model->rowCount() == 0;
            }
            return true;
        }

        QString m_text;
        QFont m_font;
        bool m_skipRootLevelEntries;
    };

public:
    explicit KMMEmptyView(QWidget* parent = nullptr)
        : T(parent)
        , d(new Private)
    {
        d->m_font = T::font();
    }

    virtual ~KMMEmptyView()
    {
        delete d;
    }

    /**
     * Use @a text as the message to be shown when the
     * view does not show any data
     *
     * @param text Text to be displayed
     */
    void setEmptyText(const QString& text)
    {
        d->m_text = text;
        // Invalidate the area to trigger a repaint
        T::viewport()->update();
    }

    /**
     * Use @a font as the font to show the message when the
     * view does not show any data
     *
     * @param text Text to be displayed
     */
    void setEmptyFont(const QFont& font)
    {
        d->m_font = font;
        // Invalidate the area to trigger a repaint
        T::viewport()->update();
    }

    /**
     * If @a skipRootLevelEntries is true, even if root
     * level entries are present the message will be
     * displayed when none of them has children.
     *
     * @param skipRootLevelEntries
     *
     * @note Enabled only if T is QTreeView or derived from QTreeView
     *
     */
    template<typename U = T>
    typename std::enable_if<std::is_base_of<QTreeView, U>::value, void>::type setSkipRootLevelEntries(bool skipRootLevelEntries)
    {
        d->m_skipRootLevelEntries = skipRootLevelEntries;
        // Invalidate the area to trigger a repaint
        T::viewport()->update();
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        T::paintEvent(event); // Call the base class paint event

        bool empty = false;
        if (std::is_base_of<QAbstractItemView, T>::value) {
            empty = d->isModelEmpty(qobject_cast<QAbstractItemView*>(this)->model());
        } else if (std::is_base_of<QTextBrowser, T>::value) {
            empty = qobject_cast<QTextBrowser*>(this)->toPlainText().isEmpty();
        }

        // Check if no data is in the view
        if (empty) {
            QPainter painter(T::viewport());
            painter.save();
            painter.setPen(Qt::gray);
            painter.setFont(d->m_font);
            painter.drawText(T::rect(), Qt::AlignCenter, d->m_text);
            painter.restore();
        }
    }

private:
    Private* const d;
};

typedef KMMEmptyView<QTreeView> KMMEmptyTreeView;
typedef KMMEmptyView<QTableView> KMMEmptyTableView;
typedef KMMEmptyView<QListView> KMMEmptyListView;
typedef KMMEmptyView<QTextBrowser> KMMEmptyTextBrowser;

#endif // KMMEMPTYVIEW_H
