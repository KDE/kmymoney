/*
    SPDX-FileCopyrightText: 2004 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYTEMPLATE_H
#define MYMONEYTEMPLATE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDomDocument>
#include <QDomNode>
#include <QUrl>
#include <QMap>

class QTreeWidgetItem;
class QSaveFile;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

/**
  * @author Thomas Baumgart
  */

/**
  * This class represents an account template handler. It is capable
  * to read an XML formatted account template file and import it into
  * the current engine. Also, it can save the current account structure
  * of the engine to an XML formatted template file.
  */
class MyMoneyAccount;
class MyMoneyTemplate
{
public:
  MyMoneyTemplate();
  explicit MyMoneyTemplate(const QUrl &url);
  ~MyMoneyTemplate();

  bool loadTemplate(const QUrl &url);
  bool saveTemplate(const QUrl &url);
  bool importTemplate(void(*callback)(int, int, const QString&));
  bool exportTemplate(void(*callback)(int, int, const QString&));

  const QString& title() const;
  const QString& shortDescription() const;
  const QString& longDescription() const;

  void setTitle(const QString &s);
  void setShortDescription(const QString &s);
  void setLongDescription(const QString &s);

  void hierarchy(QMap<QString, QTreeWidgetItem*>& list);

protected:
  bool loadDescription();
  bool createAccounts(MyMoneyAccount& parent, QDomNode account);
  bool setFlags(MyMoneyAccount& acc, QDomNode flags);
  bool saveToLocalFile(QSaveFile* qfile);
  bool addAccountStructure(QDomElement& parent, const MyMoneyAccount& acc);
  bool hierarchy(QMap<QString, QTreeWidgetItem*>& list, const QString& parent, QDomNode account);

  /**
    * This method is used to update the progress information. It
    * checks if an appropriate function is known and calls it.
    *
    * For a parameter description see KMyMoneyView::progressCallback().
    */
  void signalProgress(int current, int total, const QString& = "");

private:
  QDomDocument    m_doc;
  QDomNode        m_accounts;
  QString         m_title;
  QString         m_shortDesc;
  QString         m_longDesc;
  QUrl            m_source;
  void (*m_progressCallback)(int, int, const QString&);
  int             m_accountsRead;
  QMap<QString,QString> m_vatAccountMap;
};

#endif
