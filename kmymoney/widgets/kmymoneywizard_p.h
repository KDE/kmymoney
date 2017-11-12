/***************************************************************************
                             kmymoneywizard_p.h
                             -------------------
    copyright            : (C) 2006 by Thomas Baumagrt
    email                : ipwizard@users.sourceforge.net
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

#ifndef KMYMONEYWIZARD_P_H
#define KMYMONEYWIZARD_P_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QObject>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

/**
  * @author Thomas Baumgart (C) 2006
  *
  * This class represents a helper object required
  * to be able to use Qt's signal/slot mechanism within
  * the KMyMoneyWizardPage object which cannot be
  * derived from QObject directly.
  */
class KMyMoneyWizardPagePrivate : public QObject
{
  Q_OBJECT
public:
  /**
    * Constructor
    */
  explicit KMyMoneyWizardPagePrivate(QObject* parent);

  void emitCompleteStateChanged(void);

signals:
  void completeStateChanged(void);
};

#endif
