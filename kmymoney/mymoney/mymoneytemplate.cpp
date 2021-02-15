/*
    SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneytemplate.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QFile>
#include <QUrl>
#include <QMap>
#include <QDomDocument>
#include <QDomNode>
#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#if 0
#include "kmymoneyutils.h"
#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneyexception.h"
#include "mymoneyenums.h"
#endif
#include "mymoneyobject_p.h"

bool MyMoneyTemplateLoadResult::isOK() const
{
  return (errorLine == -1) && (errorColumn == -1);
}

void MyMoneyTemplateLoadResult::setErrorMsg(const QString& msg)
{
  errorMsg = msg;
  // make sure isOK() returns false
  if (isOK()) {
    errorLine = 0;
    errorColumn = 0;
  }
}

class MyMoneyTemplatePrivate : public MyMoneyObjectPrivate
{
public:
  MyMoneyTemplatePrivate() {}

  QDomDocument          m_doc;
  QDomNode              m_accounts;
  QString               m_title;
  QString               m_shortDesc;
  QString               m_longDesc;
  QString               m_locale;
  QUrl                  m_source;
  QMap<QString,QString> m_vatAccountMap;

  MyMoneyTemplateLoadResult loadDescription()
  {
    MyMoneyTemplateLoadResult result;
    int validMask = 0x00;
    const int validAccount = 0x01;
    const int validTitle = 0x02;
    const int validShort = 0x04;
    const int validLong = 0x08;
    const int invalid = 0x10;
    const int validHeader = 0x0F;

    QDomElement rootElement = m_doc.documentElement();
    if (!rootElement.isNull()
      && rootElement.tagName() == QLatin1String("kmymoney-account-template")) {
      QDomNode child = rootElement.firstChild();
      while (!child.isNull() && child.isElement()) {
        QDomElement childElement = child.toElement();
        // qDebug("MyMoneyTemplate::import: Processing child node %s", childElement.tagName().data());
        if (childElement.tagName() == QLatin1String("accounts")) {
          m_accounts = childElement.firstChild();
          validMask |= validAccount;
        } else if (childElement.tagName() == QLatin1String("title")) {
          m_title = childElement.text();
          validMask |= validTitle;
        } else if (childElement.tagName() == QLatin1String("shortdesc")) {
          m_shortDesc = childElement.text();
          validMask |= validShort;
        } else if (childElement.tagName() == QLatin1String("longdesc")) {
          m_longDesc = childElement.text();
          validMask |= validLong;
        } else {
          result.setErrorMsg(i18n("<p>Invalid tag <b>%1</b> in template file <b>%2</b></p>", childElement.tagName(), m_source.toDisplayString()));
          validMask |= invalid;
        }
        child = child.nextSibling();
      }
      if (validMask != validHeader) {
        if (!(validMask & validAccount)) {
          result.setErrorMsg(i18n("<p>Missing tag <b>%1</b> in template file <b>%2</b></p>", QLatin1String("accounts"), m_source.toDisplayString()));
        } else if (!(validMask & validTitle)) {
          result.setErrorMsg(i18n("<p>Missing tag <b>%1</b> in template file <b>%2</b></p>", QLatin1String("title"), m_source.toDisplayString()));
        } else if (!(validMask & validShort)) {
          result.setErrorMsg(i18n("<p>Missing tag <b>%1</b> in template file <b>%2</b></p>", QLatin1String("shortdesc"), m_source.toDisplayString()));
        } else if (!(validMask & validLong)) {
          result.setErrorMsg(i18n("<p>Missing tag <b>%1</b> in template file <b>%2</b></p>", QLatin1String("longdesc"), m_source.toDisplayString()));
        }
      }
    } else {
      result.setErrorMsg(i18n("<p>Invalid root element in template file <b>%1</b></p>", m_source.toDisplayString()));
    }
    return result;
  }
};


MyMoneyTemplate::MyMoneyTemplate()
  : MyMoneyObject(*(new MyMoneyTemplatePrivate))
{
}

MyMoneyTemplate::MyMoneyTemplate(const QString& id, const MyMoneyTemplate& other)
  : MyMoneyObject(*new MyMoneyTemplatePrivate(*other.d_func()), id)
{
}

MyMoneyTemplate::MyMoneyTemplate(const MyMoneyTemplate& other)
  : MyMoneyObject(*new MyMoneyTemplatePrivate(*other.d_func()), other.id())
{
}

MyMoneyTemplate::~MyMoneyTemplate()
{
}

MyMoneyTemplateLoadResult MyMoneyTemplate::setAccountTree(QFile* file)
{
  Q_D(MyMoneyTemplate);
  MyMoneyTemplateLoadResult result;
  d->m_doc.setContent(file, &result.errorMsg, &result.errorLine, &result.errorColumn);
  if (result.isOK()) {
    result = d->loadDescription();
  }
  return result;
}

const QDomNode& MyMoneyTemplate::accountTree() const
{
  Q_D(const MyMoneyTemplate);
  return d->m_accounts;
}

const QString& MyMoneyTemplate::title() const
{
  Q_D(const MyMoneyTemplate);
  return d->m_title;
}

const QString& MyMoneyTemplate::shortDescription() const
{
  Q_D(const MyMoneyTemplate);
  return d->m_shortDesc;
}

const QString& MyMoneyTemplate::longDescription() const
{
  Q_D(const MyMoneyTemplate);
  return d->m_longDesc;
}

const QString& MyMoneyTemplate::locale() const
{
  Q_D(const MyMoneyTemplate);
  return d->m_locale;
}

const QUrl& MyMoneyTemplate::source() const
{
  Q_D(const MyMoneyTemplate);
  return d->m_source;
}

void MyMoneyTemplate::setTitle(const QString &s)
{
  Q_D(MyMoneyTemplate);
  d->m_title = s;
}

void MyMoneyTemplate::setShortDescription(const QString &s)
{
  Q_D(MyMoneyTemplate);
  d->m_shortDesc = s;
}

void MyMoneyTemplate::setLongDescription(const QString &s)
{
  Q_D(MyMoneyTemplate);
  d->m_longDesc = s;
}

void MyMoneyTemplate::setLocale(const QString& s)
{
  Q_D(MyMoneyTemplate);
  d->m_locale = s;
}

void MyMoneyTemplate::setSource(const QUrl& s)
{
  Q_D(MyMoneyTemplate);
  d->m_source = s;
}

bool MyMoneyTemplate::hasReferenceTo(const QString& id) const
{
  Q_UNUSED(id)
  return false;
}

QSet<QString> MyMoneyTemplate::referencedObjects() const
{
  return {};
}
