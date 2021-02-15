/***************************************************************************
                          ksettingsreports.h
                             -------------------
    copyright            : (C) 2010 by Bernd Gonsior
    email                : bernd.gonsior@googlemail.com
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef KSETTINGSREPORTS_H
#define KSETTINGSREPORTS_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class KSettingsReportsPrivate;
class KSettingsReports : public QWidget
{
  Q_OBJECT
  Q_DISABLE_COPY(KSettingsReports)

public:
  explicit KSettingsReports(QWidget* parent = nullptr);
  ~KSettingsReports();

protected Q_SLOTS:
  void slotCssUrlSelected(const QUrl&);
  void slotEditingFinished();

private:
  KSettingsReportsPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KSettingsReports)
};
#endif
