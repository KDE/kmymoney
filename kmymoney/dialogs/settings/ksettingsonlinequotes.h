/*
    SPDX-FileCopyrightText: 2005-2010 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KSETTINGSONLINEQUOTES_H
#define KSETTINGSONLINEQUOTES_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class QListWidgetItem;

class KSettingsOnlineQuotesPrivate;
class KSettingsOnlineQuotes : public QWidget
{
  Q_OBJECT
  Q_DISABLE_COPY(KSettingsOnlineQuotes)

public:
  explicit KSettingsOnlineQuotes(QWidget* parent = nullptr);
  ~KSettingsOnlineQuotes();

  void writeConfig() {}
  void readConfig() {}
  void resetConfig();

protected Q_SLOTS:
  void slotDumpCSVProfile();
  void slotUpdateEntry();
  void slotLoadWidgets();
  void slotEntryChanged();
  void slotEntryChanged(int idx);
  void slotEntryChanged(const QString& str);
  void slotEntryChanged(bool b);
  void slotNewEntry();
  void slotDeleteEntry();
  void slotEntryRenamed(QListWidgetItem* item);
  void slotStartRename(QListWidgetItem* item);

protected:
  void loadList(const bool updateResetList = false);

private:
  KSettingsOnlineQuotesPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KSettingsOnlineQuotes)
};

#endif
