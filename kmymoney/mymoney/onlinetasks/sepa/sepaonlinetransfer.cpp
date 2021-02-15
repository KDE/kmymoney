/*
 * SPDX-FileCopyrightText: 2013-2016 Christian Dávid <christian-david@web.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "sepaonlinetransfer.h"

sepaOnlineTransfer::sepaOnlineTransfer() :
  onlineTask(), creditTransfer()
{

}

sepaOnlineTransfer::sepaOnlineTransfer(const sepaOnlineTransfer &other) :
  onlineTask(other), creditTransfer(other)
{
}
