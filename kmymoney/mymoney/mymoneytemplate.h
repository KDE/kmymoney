/*
    SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYTEMPLATE_H
#define MYMONEYTEMPLATE_H

#include "kmm_mymoney_export.h"

// ----------------------------------------------------------------------------
// QT Includes

class QDomNode;
class QFile;
class QUrl;
#include <QString>
#include <QMetaType>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyobject.h"


/**
  * @author Thomas Baumgart
  */

struct KMM_MYMONEY_EXPORT MyMoneyTemplateLoadResult
{
    QString   errorMsg;
    int       errorLine {-1};
    int       errorColumn {-1};

    void setErrorMsg(const QString& msg);
    bool isOK() const;
};

/**
  * This class represents an account template handler. It is capable
  * to read an XML formatted account template file and import it into
  * the current engine. Also, it can save the current account structure
  * of the engine to an XML formatted template file.
  */
class MyMoneyTemplatePrivate;
class KMM_MYMONEY_EXPORT MyMoneyTemplate : public MyMoneyObject
{
    Q_DECLARE_PRIVATE_D(MyMoneyObject::d_ptr, MyMoneyTemplate)

public:
    MyMoneyTemplate();
    explicit MyMoneyTemplate(const QString& id, const MyMoneyTemplate& other);
    MyMoneyTemplate(const MyMoneyTemplate & other);
    MyMoneyTemplate(MyMoneyTemplate && other);
    MyMoneyTemplate & operator=(MyMoneyTemplate other);
    friend void swap(MyMoneyTemplate& first, MyMoneyTemplate& second);

    ~MyMoneyTemplate() override;

    const QString& title() const;
    const QString& shortDescription() const;
    const QString& longDescription() const;
    const QString& locale() const;
    const QUrl& source() const;

    void setTitle(const QString &s);
    void setShortDescription(const QString &s);
    void setLongDescription(const QString &s);
    void setLocale(const QString& s);
    void setSource(const QUrl& s);

    MyMoneyTemplateLoadResult setAccountTree(QFile *file);
    const QDomNode& accountTree() const;

protected:
#if 0
    bool loadDescription();
    bool createAccounts(MyMoneyAccount& parent, QDomNode account);
    bool setFlags(MyMoneyAccount& acc, QDomNode flags);
    bool saveToLocalFile(QSaveFile* qfile);
    bool addAccountStructure(QDomElement& parent, const MyMoneyAccount& acc);
    bool hierarchy(QMap<QString, QTreeWidgetItem*>& list, const QString& parent, QDomNode account);
#endif

private:
};

inline void swap(MyMoneyTemplate& first, MyMoneyTemplate& second) // krazy:exclude=inline
{
    using std::swap;
    swap(first.d_ptr, second.d_ptr);
}

inline MyMoneyTemplate::MyMoneyTemplate(MyMoneyTemplate && other) : MyMoneyTemplate() // krazy:exclude=inline
{
    swap(*this, other);
}

inline MyMoneyTemplate & MyMoneyTemplate::operator=(MyMoneyTemplate other) // krazy:exclude=inline
{
    swap(*this, other);
    return *this;
}

/**
 * Make it possible to hold @ref MyMoneyTemplate objects inside @ref QVariant objects.
 */
Q_DECLARE_METATYPE(MyMoneyTemplate)


#endif
