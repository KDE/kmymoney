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
    inline onlineJobFolder( const onlineJobFolder &other )
        : m_folder( other.m_folder )
    {}

    static onlineJobFolder outbox() { return onlineJobFolder(folderOutbox); }
    static onlineJobFolder drafts() { return onlineJobFolder(folderDrafts); }
    static onlineJobFolder templates() { return onlineJobFolder(folderTemplates); }
    static onlineJobFolder historic() { return onlineJobFolder(folderHistoric); }

private:
    enum onlineJobFolders {
        folderOutbox,
        folderDrafts,
        folderTemplates,
        folderHistoric
    };

    onlineJobFolder();
    onlineJobFolder( const onlineJobFolders& folder )
        :m_folder( folder )
    {}

    onlineJobFolders m_folder;
};

#endif // ONLINEJOBFOLDER_H
