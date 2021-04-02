/*
    SPDX-FileCopyrightText: 2009-2010 Cristian One ț <onet.cristian@gmail.com>
    SPDX-FileCopyrightText: 2009-2010 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-FileCopyrightText: 2011-2017 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTAGLABEL_H
#define KTAGLABEL_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QFrame>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

/**
  * This class implements a tag label. It create a QFrame and inside it a QToolButton
  * with a 'X' Icon and a QLabel with the name of the Tag
  *
  * @author Alessandro Russo
  */
class KTagLabel : public QFrame
{
    Q_OBJECT
    Q_DISABLE_COPY(KTagLabel)

public:
    explicit KTagLabel(const QString& id, const QString& name, QWidget* parent = nullptr);

Q_SIGNALS:
    void clicked(bool);

private:
    QString m_tagId;
};

#endif
