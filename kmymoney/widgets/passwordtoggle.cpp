/*
 * SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "passwordtoggle.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QLineEdit>
#include <QAction>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>


// ----------------------------------------------------------------------------
// Project Includes

#include "icons/icons.h"

using namespace Icons;

PasswordToggle::PasswordToggle(QLineEdit* parent)
  : QObject(parent)
  , m_lineEdit(parent)
{
  m_toggleAction = m_lineEdit->addAction(Icons::get(Icon::Visibility), QLineEdit::TrailingPosition);
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
    m_toggleAction->setIcon(Icons::get(Icon::NoVisibility));
  } else if (m_lineEdit->echoMode() == QLineEdit::Normal) {
    m_lineEdit->setEchoMode(QLineEdit::Password);
    m_toggleAction->setIcon(Icons::get(Icon::Visibility));
  }
}

