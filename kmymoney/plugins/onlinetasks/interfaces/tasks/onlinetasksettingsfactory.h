/*
    SPDX-FileCopyrightText: 2013 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ONLINETASKSETTINGSFACTORY_H
#define ONLINETASKSETTINGSFACTORY_H

#include "onlinetasks/interfaces/tasks/ionlinetasksettings.h"

class onlineTaskSettingsFactory
{
protected:
  virtual IonlineTaskSettings::ptr createSettings() const = 0;
};

Q_DECLARE_INTERFACE(onlineTaskSettingsFactory, "org.kmymoney.onlinetask.settingsFactory")

#endif // ONLINETASKSETTINGSFACTORY_H
