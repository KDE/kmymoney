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

#include "taskconvertersepatogerman.h"

#include "tasks/germanonlinetransfer.h"
#include "../sepa/tasks/sepaonlinetransfer.h"

onlineTask* taskConverterSepaToGerman::convert(const onlineTask& source, onlineTaskConverter::convertType& convertResult, QString& userInformation) const
{
  Q_ASSERT( source.taskName() == sepaOnlineTransfer::name() );

  convertResult = convertionLoseless;
  userInformation = QString();
  
  germanOnlineTransfer* convert = new germanOnlineTransfer;
  Q_CHECK_PTR(convert);
  
  const sepaOnlineTransfer& sepaTask = static_cast<const sepaOnlineTransfer&>(source);
  convert->setOriginAccount( sepaTask.responsibleAccount() );
  convert->setValue( sepaTask.value() );
  
  // Purpose: add end-to-end reference if if is given
  QString purpose = sepaTask.purpose();

  if ( !sepaTask.endToEndReference().isEmpty() ) {
    userInformation = i18n("The SEPA credit-transfer had an end-to-end reference which is not supported in national transfers. It was added to the purpose instead.");
    purpose.append( QChar('\n') + sepaTask.endToEndReference() );
    convertResult = convertionLossyMinor;
  }
  convert->setPurpose( purpose );
  return convert;
}

QString taskConverterSepaToGerman::convertedTask() const
{
  return germanOnlineTransfer::name();
}

QStringList taskConverterSepaToGerman::convertibleTasks() const
{
  return QStringList(sepaOnlineTransfer::name());
}
