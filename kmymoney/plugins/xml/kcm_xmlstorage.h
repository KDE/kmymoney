/*
 * SPDX-FileCopyrightText: 2005-2010 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KCM_XMLSTORAGE_H
#define KCM_XMLSTORAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KCModule>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_xmlstoragesettings.h"

class PluginSettingsWidget : public QWidget, public Ui::XMLStorageSettings
{
  Q_OBJECT
  Q_DISABLE_COPY(PluginSettingsWidget)

public:
  explicit PluginSettingsWidget(QWidget* parent = nullptr);

public Q_SLOTS:
  void showEvent(QShowEvent *event) override;

protected Q_SLOTS:
  void slotStatusChanged(bool state);
  void slotIdChanged();
  void slotIdChanged(int idx);
  void slotKeyListChanged();

private:
  int               m_checkCount;
  bool              m_needCheckList;
  bool              m_listOk;
};

class KCMXMLStorage : public KCModule
{
public:
  explicit KCMXMLStorage(QWidget *parent, const QVariantList &args);
  ~KCMXMLStorage();
};

#endif

