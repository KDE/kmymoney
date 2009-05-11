/***************************************************************************
                          kmymoneygpgconfig.h  -  description
                             -------------------
    begin                : Mon Jan 3 2005
    copyright            : (C) 2005 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KMYMONEYGPGCONFIG_H
#define KMYMONEYGPGCONFIG_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "../widgets/kmymoneygpgconfigdecl.h"

/**
  * @author Thomas Baumgart
  */

/**
  * This class provides the necessary user interface to
  * setup the parameters required for data encryption
  */
class kMyMoneyGPGConfig : public kMyMoneyGPGConfigDecl
{
  Q_OBJECT
public:
  kMyMoneyGPGConfig(QWidget* parent, const char *name);
  virtual ~kMyMoneyGPGConfig() {}

  void writeConfig(void);
  void readConfig(void);
  void resetConfig(void);

protected slots:
  void slotIdChanged(const QString& txt);
  void slotStatusChanged(bool);

private:
  QString    m_resetUserId;
  bool       m_resetUseEncryption;
  bool       m_resetRecover;
  int        m_checkCount;
};

#endif
