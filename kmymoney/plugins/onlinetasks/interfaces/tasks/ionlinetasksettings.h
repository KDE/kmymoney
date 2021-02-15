/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * SPDX-FileCopyrightText: 2014 Christian Dávid <christian-david@web.de>
 *
 *SPDX-License-Identifier: GPL-2.0-or-laterrg/licenses/>.
 */

#ifndef IONLINETASKSETTINGS_H
#define IONLINETASKSETTINGS_H

#include <QSharedPointer>

/**
 * @brief Account/plugin dependent settings for an onlineTask
 *
 * Many onlineTasks settings vary due to multiple reasons. E.g.
 * a credit transfer could have a maximum amount it can transfer at
 * once. But this amount could depend on the account and the user's
 * contract with the bank.
 *
 * Therefor onlineTasks can offer their own set of configurations. There
 * is no predefined behavior, only subclass onlineTask::settings.
 * Of course onlinePlugins and widgets which support that task
 * need to know how to handle that specific settings.
 *
 * Using @ref onlineJobAdministration::taskSettings() KMyMoney will
 * request the correct onlinePlugin to create the settings and return
 * them as shared pointer. Please note that KMyMoney will try to reuse
 * that pointer if possible, so do not edit it.
 */

class IonlineTaskSettings
{
public:
  typedef QSharedPointer<IonlineTaskSettings> ptr;

  /**
   * Ensure this class to be polymorph
   * Make gcc happy and prevent a warning
   */
  virtual ~IonlineTaskSettings() {}
};

Q_DECLARE_INTERFACE(IonlineTaskSettings, "org.kmymoney.onlinetask.settings")

#endif // IONLINETASKSETTINGS_H
