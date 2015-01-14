/*
 * This file is part of KMyMoney, A Personal Finance Manager for KDE
 * Copyright (C) 2014 Christian Dávid <christian-david@web.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TASKCONVERTERGERMANTOSEPA_H
#define TASKCONVERTERGERMANTOSEPA_H

#include <onlinetasks/interfaces/converter/onlinetaskconverter.h>

class taskConverterGermanToSepa : public onlineTaskConverter
{
public:
  virtual onlineTask* convert(const onlineTask& source, onlineTaskConverter::convertType& convertResult, QString& userInformation) const;
  virtual QString convertedTask() const;
  virtual QStringList convertibleTasks() const;
};

#endif // TASKCONVERTERGERMANTOSEPA_H
