/*
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz lukasz.wojnilowicz @gmail.com
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "kcm_qif.h"
#include <config-kmymoney-version.h>

// KDE includes

#include "kmymoneypluginclass.h"
#include <KAboutData>
#include <KPluginFactory>

#include "mymoneyqifprofileeditor.h"

KCMqif::KCMqif(QWidget *parent, const QVariantList& args) : KCModule(parent, args)
{
    auto editor = new MyMoneyQifProfileEditor(true, this);
    auto layout = new QVBoxLayout;
    setLayout(layout);
    layout->addWidget(editor);
    setButtons(NoAdditionalButton);
    load();
}

K_PLUGIN_CLASS(KCMqif)

#include "kcm_qif.moc"
