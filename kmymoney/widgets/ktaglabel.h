/***************************************************************************
                          ktaglabel.h  -  description
                             -------------------
    begin                : Mon Jan 09 2010
    copyright            : (C) 2010 by Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Cristian Onet <cristian.onet@gmail.com>
                           Alvaro Soliverez <asoliverez@gmail.com>
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

#ifndef KTAGLABEL_H
#define KTAGLABEL_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QFrame>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmm_widgets_export.h"

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

signals:
  void clicked(bool);

private:
  QString m_tagId;
};

#endif
