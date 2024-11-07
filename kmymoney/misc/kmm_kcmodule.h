/*
    SPDX-FileCopyrightText: 2024 Ralf Habacker <ralf.habacker@freenet.de>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KMM_KCMODULE_H
#define KMM_KCMODULE_H

#include <KCModule>

#include <QWidget>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
class KMMKCModule : public KCModule
{
public:
    KMMKCModule(QObject* parent, const QVariantList& args = QVariantList())
        : KCModule(parent)
    {
        Q_UNUSED(args);
    }
};
#else
class KMMKCModule : public KCModule
{
public:
    KMMKCModule(QObject* parent, const QVariantList& args = QVariantList())
        : KCModule(dynamic_cast<QWidget*>(parent), args)
    {
    }

    QWidget* widget()
    {
        return this;
    }
};
#endif

#endif // KMM_KCMODULE_H
