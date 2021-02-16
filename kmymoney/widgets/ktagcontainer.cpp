/*
    SPDX-FileCopyrightText: 2009-2016 Cristian One ț <onet.cristian@gmail.com>
    SPDX-FileCopyrightText: 2009-2010 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-FileCopyrightText: 2010-2016 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ktagcontainer.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QHBoxLayout>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ktaglabel.h"
#include "mymoneytag.h"
#include "kmymoneytagcombo.h"

class KTagContainerPrivate
{
  Q_DISABLE_COPY(KTagContainerPrivate)

public:
  KTagContainerPrivate() :
    m_tagCombo(nullptr)
  {
  }

  KMyMoneyTagCombo *m_tagCombo;
  QList<KTagLabel*> m_tagLabelList;
  QList<QString> m_tagIdList;
  QList<QString> m_tagNameList;

  // A local cache of the list of all Tags, it's updated when loadTags is called
  QList<MyMoneyTag> m_list;
};

KTagContainer::KTagContainer(QWidget* parent) :
  QWidget(parent),
  d_ptr(new KTagContainerPrivate)
{
  Q_D(KTagContainer);
  d->m_tagCombo = new KMyMoneyTagCombo;
  QHBoxLayout *layout = new QHBoxLayout;
  layout->setContentsMargins(0, 0, 5, 0);
  layout->setSpacing(0);
  layout->addWidget(d->m_tagCombo, 100);
  this->setLayout(layout);
  this->setFocusProxy(d->m_tagCombo);
  connect(d->m_tagCombo, &KMyMoneyMVCCombo::lostFocus, this, &KTagContainer::slotAddTagWidget);
}

KTagContainer::~KTagContainer()
{
  Q_D(KTagContainer);
  // make sure we are not called as part of the destruction
  disconnect(d->m_tagCombo, &KMyMoneyMVCCombo::lostFocus, this, &KTagContainer::slotAddTagWidget);
  delete d;
}

void KTagContainer::loadTags(const QList<MyMoneyTag>& list)
{
  Q_D(KTagContainer);
  d->m_list = list;
  d->m_tagCombo->loadTags(list);
}

KMyMoneyTagCombo* KTagContainer::tagCombo()
{
  Q_D(KTagContainer);
  return d->m_tagCombo;
}

const QList<QString> KTagContainer::selectedTags()
{
  Q_D(KTagContainer);
  // Adding possible selected tag which is not yet in the list
  slotAddTagWidget();
  return d->m_tagIdList;
}

void KTagContainer::addTagWidget(const QString& id)
{
  Q_D(KTagContainer);
  if (id.isNull() || d->m_tagIdList.contains(id))
    return;
  const QString tagName = d->m_tagCombo->itemText(d->m_tagCombo->findData(QVariant(id), Qt::UserRole, Qt::MatchExactly));
  KTagLabel *t = new KTagLabel(id, tagName, this);
  connect(t, &KTagLabel::clicked, this, &KTagContainer::slotRemoveTagWidget);
  d->m_tagLabelList.append(t);
  d->m_tagNameList.append(tagName);
  d->m_tagIdList.append(id);
  this->layout()->addWidget(t);
  d->m_tagCombo->loadTags(d->m_list);
  d->m_tagCombo->setUsedTagList(d->m_tagIdList, d->m_tagNameList);
  d->m_tagCombo->setCurrentIndex(0);
  d->m_tagCombo->setFocus();
}

void KTagContainer::RemoveAllTagWidgets()
{
  Q_D(KTagContainer);
  d->m_tagIdList.clear();
  d->m_tagNameList.clear();
  while (!d->m_tagLabelList.isEmpty())
    delete d->m_tagLabelList.takeLast();
  d->m_tagCombo->loadTags(d->m_list);
  d->m_tagCombo->setUsedTagList(d->m_tagIdList, d->m_tagNameList);
  d->m_tagCombo->setCurrentIndex(0);
}

void KTagContainer::slotAddTagWidget()
{
  Q_D(KTagContainer);
  addTagWidget(d->m_tagCombo->selectedItem());
}

void KTagContainer::slotRemoveTagWidget()
{
  Q_D(KTagContainer);
  this->tagCombo()->setFocus();
  KTagLabel *t = (KTagLabel *)sender();
  int index = d->m_tagLabelList.indexOf(t);
  d->m_tagLabelList.removeAt(index);
  d->m_tagIdList.removeAt(index);
  d->m_tagNameList.removeAt(index);
  delete t;
  d->m_tagCombo->loadTags(d->m_list);
  d->m_tagCombo->setUsedTagList(d->m_tagIdList, d->m_tagNameList);
  d->m_tagCombo->setCurrentIndex(0);
}
