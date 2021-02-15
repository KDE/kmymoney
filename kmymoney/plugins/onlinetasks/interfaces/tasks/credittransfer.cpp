/*
  This file is part of KMyMoney, A Personal Finance Manager by KDE
  SPDX-FileCopyrightText: 2013 Christian Dávid <christian-david@web.de>

 SPDX-License-Identifier: GPL-2.0-or-laterrg/licenses/>.
*/

#include "credittransfer.h"

creditTransfer::~creditTransfer()
{
}

QString creditTransfer::jobTypeName() const
{
  return "Credit Transfer";
}
