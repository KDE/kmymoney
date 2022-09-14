/*
    SPDX-FileCopyrightText: 2021      Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "popuppositioner.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QApplication>
#include <QRect>
#include <QScreen>
#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes


PopupPositioner::PopupPositioner(QWidget* baseWidget, QWidget* popupWidget, Anchor anchor)
{
    const auto screenRect = baseWidget->screen()->availableGeometry();

    auto p = baseWidget->mapToGlobal(QPoint());
    // align the y coordinate
    switch(anchor) {
    case BottemLeft:
    case BottomRight:
        if (p.y() + baseWidget->height() + popupWidget->height() > screenRect.bottomLeft().y()) {
            p.setY(p.y() - popupWidget->height());
        } else {
            p.setY(p.y() + baseWidget->height());
        }
        break;
    case TopLeft:
    case TopRight:
        if (p.y() - popupWidget->height() < screenRect.topLeft().y()) {
            p.setY(p.y() + baseWidget->height());
        } else {
            p.setY(p.y() - popupWidget->height());
        }
        break;
    }

    // align the x coordinate
    switch(anchor) {
    case BottemLeft:
    case TopLeft:
        // if left aligning causes the widget to leave the screen area
        // then align it to the right edge of the base widget instead
        if (p.x() + popupWidget->width() > screenRect.topRight().x()) {
            p.setX(p.x() + baseWidget->width() - popupWidget->width());
        }
        break;

    case BottomRight:
    case TopRight:
        // align to the right
        p.setX(p.x() + baseWidget->width() - popupWidget->width());

        // if right aligning causes the widget to leave the screen area
        // then align it to the left edge of the base widget
        if (p.x() < screenRect.topLeft().x()) {
            p.setX(baseWidget->x());
        }
        break;
    }

    // if the popup widget leaves the screen to the left
    // then align it to the left side of the screen
    if (p.x() < screenRect.topLeft().x()) {
        p.setX(screenRect.topLeft().x());
    }
    // if the popup widget leaves the screen to the right
    // then align its right edge to the right side of the screen
    if (p.x() + popupWidget->width() > screenRect.topRight().x()) {
        p.setX(screenRect.topRight().x() - popupWidget->width());
    }

    // move the popup widget to its location
    popupWidget->move(p);
}
