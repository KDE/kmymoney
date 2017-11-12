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

#include <config-kmymoney.h>

#include "kaccounttemplateselector.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDir>
#include <QTimer>
#include <QList>
#include <QTreeWidget>
#include <QStandardPaths>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KTextEdit>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kaccounttemplateselector.h"

#include <mymoneytemplate.h>

class KAccountTemplateSelectorPrivate
{
  Q_DISABLE_COPY(KAccountTemplateSelectorPrivate)

public:
  KAccountTemplateSelectorPrivate() :
    ui(new Ui::KAccountTemplateSelector),
    id(0)
  {
  }

  ~KAccountTemplateSelectorPrivate()
  {
    delete ui;
  }

#ifndef KMM_DESIGNER
QTreeWidgetItem* hierarchyItem(const QString& parent, const QString& name)
{
  if (!m_templateHierarchy.contains(parent)
      || m_templateHierarchy[parent] == 0) {
    QRegExp exp("(.*):(.*)");
    if (exp.indexIn(parent) != -1)
      m_templateHierarchy[parent] = hierarchyItem(exp.cap(1), exp.cap(2));
  }
  QTreeWidgetItem *item = new QTreeWidgetItem(m_templateHierarchy[parent]);
  item->setText(0, name);
  return item;
}

void loadHierarchy()
{
  m_templateHierarchy.clear();
  QTreeWidgetItemIterator it(ui->m_groupList, QTreeWidgetItemIterator::Selected);
  QTreeWidgetItem* it_v;
  while ((it_v = *it) != 0) {
    m_templates[it_v->data(0, Qt::UserRole).toInt()].hierarchy(m_templateHierarchy);
    ++it;
  }

  // I need to think about this some more. The code works and shows
  // the current account hierarchy. It might be useful, to show
  // existing accounts dimmed and the new ones in bold or so.
#if 0

  // add the hierarchy from the MyMoneyFile object
  QList<MyMoneyAccount> aList;
  QList<MyMoneyAccount>::const_iterator it_a;
  auto file = MyMoneyFile::instance();
  file->accountList(aList);
  if (aList.count() > 0) {
    m_templateHierarchy[file->accountToCategory(file->asset().id(), true)] = 0;
    m_templateHierarchy[file->accountToCategory(file->liability().id(), true)] = 0;
    m_templateHierarchy[file->accountToCategory(file->income().id(), true)] = 0;
    m_templateHierarchy[file->accountToCategory(file->expense().id(), true)] = 0;
    m_templateHierarchy[file->accountToCategory(file->equity().id(), true)] = 0;
  }

  for (it_a = aList.begin(); it_a != aList.end(); ++it_a) {
    m_templateHierarchy[file->accountToCategory((*it_a).id(), true)] = 0;
  }
#endif

  ui->m_accountList->clear();

  QRegExp exp("(.*):(.*)");
  for (QMap<QString, QTreeWidgetItem*>::iterator it_m = m_templateHierarchy.begin(); it_m != m_templateHierarchy.end(); ++it_m) {
    if (exp.indexIn(it_m.key()) == -1) {
      (*it_m) = new QTreeWidgetItem(ui->m_accountList);
      (*it_m)->setText(0, it_m.key());
    } else {
      (*it_m) = hierarchyItem(exp.cap(1), exp.cap(2));
    }
    (*it_m)->setExpanded(true);
  }

  ui->m_description->clear();
  if (ui->m_groupList->currentItem()) {
    ui->m_description->setText(m_templates[ui->m_groupList->currentItem()->data(0, Qt::UserRole).toInt()].longDescription());
  }
}

QList<MyMoneyTemplate> selectedTemplates() const
{
  QList<MyMoneyTemplate> list;
  QTreeWidgetItemIterator it(ui->m_groupList, QTreeWidgetItemIterator::Selected);
  QTreeWidgetItem* it_v;
  while ((it_v = *it) != 0) {
    list << m_templates[it_v->data(0, Qt::UserRole).toInt()];
    ++it;
  }
  return list;
}
#endif



public:
  Ui::KAccountTemplateSelector    *ui;
  QMap<QString, QTreeWidgetItem*>  m_templateHierarchy;
#ifndef KMM_DESIGNER
  QMap<int, MyMoneyTemplate>       m_templates;
  // a map of country name or country name (language name) -> localeId (lang_country) so be careful how you use it
  QMap<QString, QString>           countries;
  QString                          currentLocaleId;
  QMap<QString, QString>::iterator it_m;
  QStringList                      dirlist;
  int                              id;
#endif
};

KAccountTemplateSelector::KAccountTemplateSelector(QWidget* parent) :
  QWidget(parent),
  d_ptr(new KAccountTemplateSelectorPrivate)
{
  Q_D(KAccountTemplateSelector);
  d->ui->setupUi(this);
  d->ui->m_accountList->header()->hide();
  d->ui->m_groupList->setSelectionMode(QAbstractItemView::ExtendedSelection);
  connect(d->ui->m_groupList, &QTreeWidget::itemSelectionChanged, this, &KAccountTemplateSelector::slotLoadHierarchy);

  // kick off loading of account template data
  QTimer::singleShot(0, this, SLOT(slotLoadTemplateList()));
}

KAccountTemplateSelector::~KAccountTemplateSelector()
{
  Q_D(KAccountTemplateSelector);
  delete d;
}

void KAccountTemplateSelector::slotLoadTemplateList()
{
#ifndef KMM_DESIGNER
  Q_D(KAccountTemplateSelector);
  QStringList dirs;
  // get list of template subdirs and scan them for the list of subdirs
  d->dirlist = QStandardPaths::locateAll(QStandardPaths::DataLocation, "templates", QStandardPaths::LocateDirectory);
  QStringList::iterator it;
  for (it = d->dirlist.begin(); it != d->dirlist.end(); ++it) {
    QDir dir(*it);
    dirs = dir.entryList(QStringList("*"), QDir::Dirs);
    QStringList::iterator it_d;
    for (it_d = dirs.begin(); it_d != dirs.end(); ++it_d) {
      // we don't care about . and ..
      if ((*it_d) == ".." || (*it_d) == "." || (*it_d) == "C")
        continue;
      QLocale templateLocale(*it_d);
      if (templateLocale.language() != QLocale::C) {
        QString country = QLocale().countryToString(templateLocale.country());
        QString lang = QLocale().languageToString(templateLocale.language());
        if (d->countries.contains(country)) {
          if (d->countries[country] != *it_d) {
            QString otherName = d->countries[country];
            QLocale otherTemplateLocale(otherName);
            QString otherCountry = QLocale().countryToString(otherTemplateLocale.country());
            QString otherLang = QLocale().languageToString(otherTemplateLocale.language());
            d->countries.remove(country);
            d->countries[QString("%1 (%2)").arg(otherCountry, otherLang)] = otherName;
            d->countries[QString("%1 (%2)").arg(country, lang)] = *it_d;
            // retain the item corresponding to the current locale
            if (QLocale().country() == templateLocale.country()) {
              d->currentLocaleId = *it_d;
            }
          }
        } else {
          d->countries[country] = *it_d;
            // retain the item corresponding to the current locale
          if (QLocale().country() == templateLocale.country()) {
              d->currentLocaleId = *it_d;
          }
        }
      } else {
        qDebug("'%s/%s' not scanned", qPrintable(*it), qPrintable(*it_d));
      }
    }
  }

  // now that we know, what we can get at max, we scan everything
  // and parse the templates into memory
  d->ui->m_groupList->clear();
  d->m_templates.clear();
  d->it_m = d->countries.begin();
  d->id = 1;
  if (d->it_m != d->countries.end())
    QTimer::singleShot(0, this, SLOT(slotLoadCountry()));
  else {
    d->loadHierarchy();
  }
#endif
}

void KAccountTemplateSelector::slotLoadCountry()
{
#ifndef KMM_DESIGNER
  Q_D(KAccountTemplateSelector);
  QTreeWidgetItem *parent = new QTreeWidgetItem(d->ui->m_groupList);
  parent->setText(0, d->it_m.key());
  parent->setFlags(parent->flags() & ~Qt::ItemIsSelectable);
  for (QStringList::iterator it = d->dirlist.begin(); it != d->dirlist.end(); ++it) {
    QDir dir(QString("%1/%2").arg(*it).arg(*(d->it_m)));
    if (dir.exists()) {
      QStringList files = dir.entryList(QStringList("*"), QDir::Files);
      for (QStringList::iterator it_f = files.begin(); it_f != files.end(); ++it_f) {
        MyMoneyTemplate templ(QUrl::fromUserInput(QString("%1/%2").arg(dir.canonicalPath(), *it_f)));
        d->m_templates[d->id] = templ;
        QTreeWidgetItem *item = new QTreeWidgetItem(parent);
        item->setText(0, templ.title());
        item->setText(1, templ.shortDescription());
        item->setData(0, Qt::UserRole, QString("%1").arg(d->id));
        ++d->id;
      }
    }
  }
  // make visible the templates of the current locale
  if (d->it_m.value() == d->currentLocaleId) {
    d->ui->m_groupList->setCurrentItem(parent);
    d->ui->m_groupList->expandItem(parent);
    d->ui->m_groupList->scrollToItem(parent, QTreeView::PositionAtTop);
  }
  ++d->it_m;
  if (d->it_m != d->countries.end())
    QTimer::singleShot(0, this, SLOT(slotLoadCountry()));
  else {
    d->loadHierarchy();
  }
#endif

}

void KAccountTemplateSelector::slotLoadHierarchy()
{
#ifndef KMM_DESIGNER
  Q_D(KAccountTemplateSelector);
  d->loadHierarchy();
#endif
}

QList<MyMoneyTemplate> KAccountTemplateSelector::selectedTemplates() const
{
#ifndef KMM_DESIGNER
  Q_D(const KAccountTemplateSelector);
  return d->selectedTemplates();
#else
  return QList<MyMoneyTemplate>();
#endif
}
