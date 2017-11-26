/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2013 Christian Dávid <christian-david@web.de>
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

#ifndef ONLINEJOBFOLDER_H
#define ONLINEJOBFOLDER_H

/**
 * @brief Folder to organize @ref onlineJob "onlineJobs"
 *
 * This class is mainly for forward compatibility. At the monent there are only four default
 * folders outbox(), drafts(), templates(), historic(). These static methods are also the only
 * way to create a folder.
 *
 * If job organizing becomes more complicated this class can be extended.
 *
 */
class onlineJobFolder
{
public:
  inline onlineJobFolder(const onlineJobFolder &other) // krazy:exclude=explicit
      : m_folder(other.m_folder) {}

  static onlineJobFolder outbox() {
    return onlineJobFolder(folderOutbox);
  }
  static onlineJobFolder drafts() {
    return onlineJobFolder(folderDrafts);
  }
  static onlineJobFolder templates() {
    return onlineJobFolder(folderTemplates);
  }
  static onlineJobFolder historic() {
    return onlineJobFolder(folderHistoric);
  }

private:
  enum onlineJobFolders {
    folderOutbox,
    folderDrafts,
    folderTemplates,
    folderHistoric
  };

  onlineJobFolder();
  explicit onlineJobFolder(const onlineJobFolders& folder);

  onlineJobFolders m_folder;
};

#endif // ONLINEJOBFOLDER_H
