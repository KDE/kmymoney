/*
    SPDX-FileCopyrightText: 2013-2015 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
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
  onlineJobFolder(const onlineJobFolder &other); // krazy:exclude=explicit

  static onlineJobFolder outbox();
  static onlineJobFolder drafts();
  static onlineJobFolder templates();
  static onlineJobFolder historic();

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
