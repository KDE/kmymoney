/*
 * Copyright 2019       Thomas Baumgart <tbaumgart@kde.org>
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

#ifndef PASSWORDTOGGLE_H
#define PASSWORDTOGGLE_H

#include "kmm_base_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QObject>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class QLineEdit;
class QAction;

class KMM_BASE_WIDGETS_EXPORT PasswordToggle : public QObject
{
  Q_OBJECT
public:
  explicit PasswordToggle(QLineEdit* parent);

protected Q_SLOTS:
  void toggleEchoModeAction(const QString& text);
  void toggleEchoMode();
private:
  QLineEdit*    m_lineEdit;
  QAction*      m_toggleAction;
};


#endif // PASSWORDTOGGLE_H
