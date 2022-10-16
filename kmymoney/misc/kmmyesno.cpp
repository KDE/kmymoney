/*
    SPDX-FileCopyrightText: 2022 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kmmyesno.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

KGuiItem KMMYesNo::yes()
{
    return KGuiItem(i18nc("@action:button", "Yes"), QStringLiteral("dialog-ok"));
}

KGuiItem KMMYesNo::no()
{
    return KGuiItem(i18nc("@action:button", "No"), QStringLiteral("dialog-cancel"));
}
