/*
    SPDX-FileCopyrightText: 2013-2015 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "onlinejobfolder.h"

onlineJobFolder::onlineJobFolder() :
    m_folder(folderOutbox)
{
}

onlineJobFolder::onlineJobFolder(const onlineJobFolders& folder) :
    m_folder(folder)
{
}

onlineJobFolder::onlineJobFolder(const onlineJobFolder &other)
    : m_folder(other.m_folder)
{
}

onlineJobFolder onlineJobFolder::outbox()
{
  return onlineJobFolder(folderOutbox);
}

onlineJobFolder onlineJobFolder::drafts()
{
  return onlineJobFolder(folderDrafts);
}

onlineJobFolder onlineJobFolder::templates()
{
  return onlineJobFolder(folderTemplates);
}

onlineJobFolder onlineJobFolder::historic()
{
  return onlineJobFolder(folderHistoric);
}
