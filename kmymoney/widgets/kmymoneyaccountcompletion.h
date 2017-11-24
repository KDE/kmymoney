/***************************************************************************
                          kmymoneyaccountcompletion.h  -  description
                             -------------------
    begin                : Mon Apr 26 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KMYMONEYACCOUNTCOMPLETION_H
#define KMYMONEYACCOUNTCOMPLETION_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneycompletion.h"

namespace eMyMoney { namespace Account { enum class Type; } }

class KMyMoneyAccountSelector;

/**
  * @author Thomas Baumgart
  */
class KMyMoneyAccountCompletion : public KMyMoneyCompletion
{
  Q_OBJECT
  Q_DISABLE_COPY(KMyMoneyAccountCompletion)

public:

  explicit KMyMoneyAccountCompletion(QWidget* parent = nullptr);
  ~KMyMoneyAccountCompletion() override;

  QStringList accountList(const QList<eMyMoney::Account::Type>& list) const;
  QStringList accountList() const;

  /**
    * reimplemented from KMyMoneyCompletion
    */
  KMyMoneyAccountSelector* selector() const;

public slots:
  void slotMakeCompletion(const QString& txt);
};

#endif
