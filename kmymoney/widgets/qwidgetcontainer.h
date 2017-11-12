/***************************************************************************
                             qwidgetcontainer.h
                             ----------
    begin                : Fri Mar 10 2006
    copyright            : (C) 2006 by Thomas Baumgart
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

#ifndef QWIDGETCONTAINER_H
#define QWIDGETCONTAINER_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QMap>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class QString;
class QWidget;

namespace KMyMoneyRegister
{
  struct QWidgetContainer : public QMap<QString, QWidget*>
  {
    Q_DISABLE_COPY(QWidgetContainer)
    QWidgetContainer();
    QWidget* haveWidget(const QString& name) const;
    void removeOrphans();
  };
} // namespace

#endif
