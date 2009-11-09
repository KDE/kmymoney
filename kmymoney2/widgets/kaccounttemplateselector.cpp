/***************************************************************************
                          kaccounttemplateselector.cpp  -  description
                             -------------------
    begin                : Tue Feb 5 2008
    copyright            : (C) 2008 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// ----------------------------------------------------------------------------
// QT Includes

#include <QDir>
#include <q3header.h>
#include <QTimer>
#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <k3listview.h>
#include <ktextedit.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "config-kmymoney.h"
#include <mymoneytemplate.h>

#include "kaccounttemplateselector.h"

class KTemplateListItem : public K3ListViewItem
{
  public:
    KTemplateListItem(Q3ListViewItem* parent, const QString& text);
    void setAvailable(void);
  public:
    bool m_isAvailable;
};

KTemplateListItem::KTemplateListItem(Q3ListViewItem* parent, const QString& text) :
  K3ListViewItem(parent, text),
  m_isAvailable(false)
{
}

void KTemplateListItem::setAvailable(void)
{
  m_isAvailable = true;
}

class KAccountTemplateSelector::Private
{
  public:
    Private(KAccountTemplateSelector* p) { m_parent = p; }
#ifndef KMM_DESIGNER
    QList<MyMoneyTemplate> selectedTemplates(void) const;
    Q3ListViewItem* hierarchyItem(const QString& parent, const QString& name);
    void loadHierarchy(void);
#endif

  public:
    KAccountTemplateSelector*         m_parent;
    QMap<QString, Q3ListViewItem*>     m_templateHierarchy;
#ifndef KMM_DESIGNER
    QMap<QString, MyMoneyTemplate>    m_templates;
    QMap<QString, QString> countries;
    QMap<QString, QString>::iterator it_m;
    QStringList                      dirlist;
    int                              id;
#endif
};


#ifndef KMM_DESIGNER
Q3ListViewItem* KAccountTemplateSelector::Private::hierarchyItem(const QString& parent, const QString& name)
{
  if(!m_templateHierarchy.contains(parent)
  || m_templateHierarchy[parent] == 0) {
    QRegExp exp("(.*):(.*)");
    if(exp.indexIn(parent) != -1)
      m_templateHierarchy[parent] = hierarchyItem(exp.cap(1), exp.cap(2));
  }
  return new KTemplateListItem(m_templateHierarchy[parent], name);
}

void KAccountTemplateSelector::Private::loadHierarchy(void)
{
  m_templateHierarchy.clear();
  Q3ListViewItemIterator it(m_parent->m_groupList, Q3ListViewItemIterator::Selected);
  Q3ListViewItem* it_v;
  while((it_v = it.current()) != 0) {
    m_templates[it_v->text(2)].hierarchy(m_templateHierarchy);
    ++it;
  }

  // I need to think about this some more. The code works and shows
  // the current account hierarchy. It might be useful, to show
  // existing accounts dimmed and the new ones in bold or so.
#if 0

  // add the hierarchy from the MyMoneyFile object
  QList<MyMoneyAccount> aList;
  QList<MyMoneyAccount>::const_iterator it_a;
  MyMoneyFile* file = MyMoneyFile::instance();
  file->accountList(aList);
  if(aList.count() > 0) {
    m_templateHierarchy[file->accountToCategory(file->asset().id(), true)] = 0;
    m_templateHierarchy[file->accountToCategory(file->liability().id(), true)] = 0;
    m_templateHierarchy[file->accountToCategory(file->income().id(), true)] = 0;
    m_templateHierarchy[file->accountToCategory(file->expense().id(), true)] = 0;
    m_templateHierarchy[file->accountToCategory(file->equity().id(), true)] = 0;
  }

  for(it_a = aList.begin(); it_a != aList.end(); ++it_a) {
    m_templateHierarchy[file->accountToCategory((*it_a).id(), true)] = 0;
  }
#endif

  m_parent->m_accountList->clear();
  QMap<QString, Q3ListViewItem*>::iterator it_m;

  QRegExp exp("(.*):(.*)");
  for(it_m = m_templateHierarchy.begin(); it_m != m_templateHierarchy.end(); ++it_m) {
    if(exp.indexIn(it_m.key()) == -1) {
      (*it_m) = new K3ListViewItem(m_parent->m_accountList, it_m.key());
    } else {
      (*it_m) = hierarchyItem(exp.cap(1), exp.cap(2));
    }
    (*it_m)->setOpen(true);
  }

  m_parent->m_description->clear();
  if(m_parent->m_groupList->currentItem()) {
    m_parent->m_description->setText(m_templates[m_parent->m_groupList->currentItem()->text(2)].longDescription());
  }
}

QList<MyMoneyTemplate> KAccountTemplateSelector::Private::selectedTemplates(void) const
{
  QList<MyMoneyTemplate> list;
  Q3ListViewItemIterator it(m_parent->m_groupList, Q3ListViewItemIterator::Selected);
  Q3ListViewItem* it_v;
  while((it_v = it.current()) != 0) {
    list << m_templates[it_v->text(2)];
    ++it;
  }
  return list;
}
#endif


KAccountTemplateSelector::KAccountTemplateSelector(QWidget* parent) :
  KAccountTemplateSelectorDecl(parent),
  d(new Private(this))
{
  m_accountList->header()->hide();
  connect(m_groupList, SIGNAL(selectionChanged()), this, SLOT(slotLoadHierarchy()));

  // kick off loading of account template data
  QTimer::singleShot(0, this, SLOT(slotLoadTemplateList()));
}

KAccountTemplateSelector::~KAccountTemplateSelector()
{
  delete d;
}

void KAccountTemplateSelector::slotLoadTemplateList(void)
{
#ifndef KMM_DESIGNER
  QStringList dirs;
  // get list of template subdirs and scan them for the list of subdirs
  d->dirlist = KGlobal::dirs()->findDirs("appdata", "templates");
  QStringList::iterator it;
  for(it = d->dirlist.begin(); it != d->dirlist.end(); ++it) {
    QDir dir(*it);
    // qDebug("Reading dir '%s' with %d entries", (*it).data(), dir.count());
    dirs = dir.entryList("*", QDir::Dirs);
    QStringList::iterator it_d;
    for(it_d= dirs.begin(); it_d != dirs.end(); ++it_d) {
      // we don't care about . and ..
      if((*it_d) == ".." || (*it_d) == "." || (*it_d) == "C")
        continue;
      QRegExp exp("(..)_(..)");
      if(exp.indexIn(*it_d) != -1) {

        QString country = KGlobal::locale()->countryCodeToName(exp.cap(2));
        if(country.isEmpty())
          country = exp.cap(2);
        QString lang = KGlobal::locale()->languageCodeToName(exp.cap(1));
        if(d->countries.contains(country)) {
          if(d->countries[country] != *it_d) {
            QString oName = d->countries[country];
            exp.indexIn(oName);
            QString oCountry = KGlobal::locale()->countryCodeToName(exp.cap(2));
            QString oLang = KGlobal::locale()->languageCodeToName(exp.cap(1));
            d->countries.remove(country);
            d->countries[QString("%1 (%2)").arg(oCountry).arg(oLang)] = oName;
            d->countries[QString("%1 (%2)").arg(country).arg(lang)] = *it_d;
          }
        } else {
          d->countries[country] = *it_d;
        }
      } else if((*it_d).length() == 2) {
        QString country = KGlobal::locale()->countryCodeToName((*it_d).toUpper());
        d->countries[country] = *it_d;
      } else {
        qDebug("'%s/%s' not scanned", qPrintable(*it), qPrintable(*it_d));
      }
    }
  }

  // now that we know, what we can get at max, we scan everything
  // and parse the templates into memory
  m_groupList->clear();
  d->m_templates.clear();
  d->it_m = d->countries.begin();
  d->id = 1;
  QTimer::singleShot(0, this, SLOT(slotLoadCountry()));
#endif
}

void KAccountTemplateSelector::slotLoadCountry(void)
{
#ifndef KMM_DESIGNER

  K3ListViewItem* parent = new K3ListViewItem(m_groupList, d->it_m.key());
  parent->setSelectable(false);
  QStringList::iterator it;
  for(it = d->dirlist.begin(); it != d->dirlist.end(); ++it) {
    QStringList::iterator it_f;
    QDir dir(QString("%1%2").arg(*it).arg(*(d->it_m)));
    if(dir.exists()) {
      QStringList files = dir.entryList("*", QDir::Files);
      for(it_f = files.begin(); it_f != files.end(); ++it_f) {
        MyMoneyTemplate templ(QString("%1/%2").arg(dir.canonicalPath()).arg(*it_f));
        d->m_templates[QString("%1").arg(d->id)] = templ;
        new K3ListViewItem(parent, templ.title(), templ.shortDescription(), QString("%1").arg(d->id));
        ++d->id;
      }
    }
  }

  ++d->it_m;
  if(d->it_m != d->countries.end())
    QTimer::singleShot(0, this, SLOT(slotLoadCountry()));
  else {
    d->loadHierarchy();
  }
#endif

}

void KAccountTemplateSelector::slotLoadHierarchy(void)
{
#ifndef KMM_DESIGNER
  d->loadHierarchy();
#endif
}

QList<MyMoneyTemplate> KAccountTemplateSelector::selectedTemplates(void) const
{
#ifndef KMM_DESIGNER
  return d->selectedTemplates();
#else
  return QList<MyMoneyTemplate>();
#endif
}

#include "kaccounttemplateselector.moc"
