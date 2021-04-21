/*
    SPDX-FileCopyrightText: 2021      Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef POPUPPOSITIONER_H
#define POPUPPOSITIONER_H

#include "kmm_base_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

class QWidget;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

/**
  * This class represents a helper to position a @a popupWidget
  * relative to a @a baseWidget while making sure that the
  * popupWidget does not cross screen boundaries on a virtual
  * desktop.
  *
  * The @
  *
  * @author Thomas Baumgart
  */
class KMM_BASE_WIDGETS_EXPORT PopupPositioner
{
public:
    typedef enum { TopLeft, TopRight, BottemLeft, BottomRight } Anchor;

    explicit PopupPositioner(QWidget* baseWidget, QWidget* popupWidget, Anchor anchor);
};

#endif
