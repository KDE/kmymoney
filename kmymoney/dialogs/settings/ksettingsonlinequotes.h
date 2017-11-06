/***************************************************************************
                          ksettingsonlinequotes.h
                             -------------------
    begin                : Thu Dec 30 2004
    copyright            : (C) 2004 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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

protected slots:
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
