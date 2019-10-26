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

#include "passwordtoggle.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QLineEdit>
#include <QAction>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

PasswordToggle::PasswordToggle(QLineEdit* parent)
  : QObject(parent)
  , m_lineEdit(parent)
{
  m_toggleAction = m_lineEdit->addAction(QIcon::fromTheme(QStringLiteral("visibility")), QLineEdit::TrailingPosition);
  m_toggleAction->setVisible(false);
  m_toggleAction->setToolTip(i18n("Change the visibility of the password"));
  connect(m_lineEdit, &QLineEdit::textChanged, this, &PasswordToggle::toggleEchoModeAction);
  connect(m_toggleAction, &QAction::triggered, this, &PasswordToggle::toggleEchoMode);
}

void PasswordToggle::toggleEchoModeAction(const QString& text)
{
  m_toggleAction->setVisible(!text.isEmpty());
}

void PasswordToggle::toggleEchoMode()
{
  if (m_lineEdit->echoMode() == QLineEdit::Password) {
    m_lineEdit->setEchoMode(QLineEdit::Normal);
    m_toggleAction->setIcon(QIcon::fromTheme(QStringLiteral("hint")));
  } else if (m_lineEdit->echoMode() == QLineEdit::Normal) {
    m_lineEdit->setEchoMode(QLineEdit::Password);
    m_toggleAction->setIcon(QIcon::fromTheme(QStringLiteral("visibility")));
  }
}

