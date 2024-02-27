/*
    SPDX-FileCopyrightText: 2024 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kmmemptytreeview.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPainter>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

class KMMEmptyTreeView::Private
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
    bool m_skipRootLevelEntries;
};

KMMEmptyTreeView::KMMEmptyTreeView(QWidget* parent)
    : QTreeView(parent)
    , d(new Private)
{
}

KMMEmptyTreeView::~KMMEmptyTreeView()
{
    delete d;
}

void KMMEmptyTreeView::paintEvent(QPaintEvent* event)
{
    QTreeView::paintEvent(event); // Call the base class paint event

    // Check if the model is empty
    if (d->isModelEmpty(model())) {
        QPainter painter(viewport());
        painter.save();
        painter.setPen(Qt::gray);
        painter.drawText(rect(), Qt::AlignCenter, d->m_text);
        painter.restore();
    }
}

void KMMEmptyTreeView::setSkipRootLevelEntries(bool skipRootLevelEntries)
{
    d->m_skipRootLevelEntries = skipRootLevelEntries;
    // Invalidate the area to trigger a repaint
    viewport()->update();
}

void KMMEmptyTreeView::setText(const QString& text)
{
    d->m_text = text;
    // Invalidate the area to trigger a repaint
    viewport()->update();
}
