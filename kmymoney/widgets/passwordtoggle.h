/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PASSWORDTOGGLE_H
#define PASSWORDTOGGLE_H

#include "kmm_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QObject>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class QLineEdit;
class QAction;

class KMM_WIDGETS_EXPORT PasswordToggle : public QObject
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
