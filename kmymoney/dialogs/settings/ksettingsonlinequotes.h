/*
 * Copyright 2005-2010  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
