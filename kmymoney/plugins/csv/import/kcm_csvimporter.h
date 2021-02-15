/*
 * SPDX-FileCopyrightText: 2016-2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KCM_CSVIMPORTER_H
#define KCM_CSVIMPORTER_H

#include <KCModule>
#include <QWidget>
#include "ui_pluginsettingsdecl.h"

class PluginSettingsWidget : public QWidget, public Ui::PluginSettingsDecl
{
  Q_OBJECT
public:
  explicit PluginSettingsWidget(QWidget* parent = 0);
};

class KCMCSVImporter : public KCModule
{
public:
  explicit KCMCSVImporter(QWidget* parent, const QVariantList& args);
  ~KCMCSVImporter();
};

#endif // KCM_CSVIMPORT_H

