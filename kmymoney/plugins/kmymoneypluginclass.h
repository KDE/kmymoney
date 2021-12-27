// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

#include <KPluginFactory>

// If we don't have this macro defined, we have to do define it ourselves

#ifndef K_PLUGIN_CLASS
#ifdef KPLUGINFACTORY_PLUGIN_CLASS_INTERNAL_NAME
#define K_PLUGIN_CLASS(classname) K_PLUGIN_FACTORY(KPLUGINFACTORY_PLUGIN_CLASS_INTERNAL_NAME, registerPlugin<classname>();)
#else
#define K_PLUGIN_CLASS(classname) K_PLUGIN_FACTORY(classname##Factory, registerPlugin<classname>();)
#endif

#endif
