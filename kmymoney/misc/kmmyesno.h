/*
    SPDX-FileCopyrightText: 2022 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMMYESNO_H
#define KMMYESNO_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KGuiItem>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmm_yesno_export.h"

class QWidget;
class QPushButton;

namespace KMMYesNo {
/** @todo Workaround for KF5 > 100 where yes and no are deprecated
 *        we need to replace them over time, but this also affects
 *        the text in the dialogs which refer to Yes and No and it
 *        may also affect the documentation. For now, we leave it
 *        as is and provide our own version.
 */
KMM_YESNO_EXPORT KGuiItem yes();
KMM_YESNO_EXPORT KGuiItem no();
} // namespace KMMYesNo

#endif // KMMYESNO_H
