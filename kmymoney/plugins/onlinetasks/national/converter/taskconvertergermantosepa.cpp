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

#include "taskconvertergermantosepa.h"

#include "tasks/germanonlinetransfer.h"
#include "../../sepa/tasks/sepaonlinetransfer.h"

onlineTask* taskConverterGermanToSepa::convert(const onlineTask& source, onlineTaskConverter::convertType& convertResult, QString& userInformation) const
{
  userInformation = QString();
  convertResult = convertionLoseless;
  
  Q_ASSERT( source.taskName() == germanOnlineTransfer::name() );
  
  sepaOnlineTransfer* convert = new sepaOnlineTransfer;
  Q_CHECK_PTR(convert);
  
  const germanOnlineTransfer& origTask = static_cast<const germanOnlineTransfer&>(source);
  convert->setOriginAccount( origTask.responsibleAccount() );
  convert->setValue( origTask.value() );
  convert->setPurpose( origTask.purpose() );
  if ( !origTask.purpose().isEmpty() ) {
    QSharedPointer<const sepaOnlineTransfer::settings> settings = convert->getSettings();
    if ( !settings->checkPurposeCharset(convert->purpose()) ) {
      userInformation = i18n("Due to the convert the purpose contains characters which are not available in SEPA credit-transfers.");
    }
  }
  convert->setEndToEndReference( QString() );
  return convert;
}

QString taskConverterGermanToSepa::convertedTask() const
{
  return sepaOnlineTransfer::name();
}

QStringList taskConverterGermanToSepa::convertibleTasks() const
{
  return QStringList( germanOnlineTransfer::name() );
}
