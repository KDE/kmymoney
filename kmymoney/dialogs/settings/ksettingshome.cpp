/***************************************************************************
                             ksettingshome.cpp
                             --------------------
    copyright            : (C) 2005 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ksettingshome.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QStringList>
#include <QPushButton>
#include <QIcon>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_ksettingshome.h"

#include "kmymoneysettings.h"
#include "kmymoney/kmymoneyutils.h"
#include "icons/icons.h"

using namespace Icons;

class KSettingsHomePrivate
{
  Q_DISABLE_COPY(KSettingsHomePrivate)

public:
  KSettingsHomePrivate() :
    ui(new Ui::KSettingsHome),
    m_noNeedToUpdateList(false)
  {
  }

  ~KSettingsHomePrivate()
  {
    delete ui;
  }

  Ui::KSettingsHome *ui;
  bool m_noNeedToUpdateList;
};

KSettingsHome::KSettingsHome(QWidget* parent) :
  QWidget(parent),
  d_ptr(new KSettingsHomePrivate)
{
  Q_D(KSettingsHome);
  d->ui->setupUi(this);
  d->ui->m_homePageList->setSortingEnabled(false);

  d->ui->m_upButton->setIcon(Icons::get(Icon::ArrowUp));
  d->ui->m_downButton->setIcon(Icons::get(Icon::ArrowDown));

  d->ui->m_upButton->setEnabled(false);
  d->ui->m_downButton->setEnabled(false);

  // connect this, so that the list gets loaded once the edit field is filled
  connect(d->ui->kcfg_ItemList, &QLineEdit::textChanged, this, &KSettingsHome::slotLoadItems);

  connect(d->ui->m_homePageList, &QListWidget::itemSelectionChanged,
          this, &KSettingsHome::slotSelectHomePageItem);
  connect(d->ui->m_homePageList, &QAbstractItemView::clicked, this, &KSettingsHome::slotUpdateItemList);

  connect(d->ui->m_upButton, &QAbstractButton::clicked, this, &KSettingsHome::slotMoveUp);
  connect(d->ui->m_downButton, &QAbstractButton::clicked, this, &KSettingsHome::slotMoveDown);

  // Don't show it to the user, we only need it to load and save the settings
  d->ui->kcfg_ItemList->hide();
}

KSettingsHome::~KSettingsHome()
{
  Q_D(KSettingsHome);
  delete d;
}

void KSettingsHome::slotLoadItems()
{
  Q_D(KSettingsHome);
  if (d->m_noNeedToUpdateList)
    return;

  QStringList list = KMyMoneySettings::listOfItems();
  QStringList::ConstIterator it;
  d->ui->m_homePageList->clear();
  QListWidgetItem *sel = 0;

  for (it = list.constBegin(); it != list.constEnd(); ++it) {
    int idx = (*it).toInt();
    // skip over unknown item entries
    if (idx == 0)
      continue;
    bool enabled = idx > 0;
    if (!enabled) idx = -idx;
    QListWidgetItem* item = new QListWidgetItem(d->ui->m_homePageList);
    item->setText(KMyMoneyUtils::homePageItemToString(idx));
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);

    // qDebug("Adding %s", item->text(0).toLatin1());
    if (enabled) {
      item->setCheckState(Qt::Checked);
    } else {
      item->setCheckState(Qt::Unchecked);
    }

    if (sel == 0)
      sel = item;
  }

  if (sel) {
    d->ui->m_homePageList->setCurrentItem(sel);
    slotSelectHomePageItem();
  }
}

void KSettingsHome::slotUpdateItemList()
{
  Q_D(KSettingsHome);
  QString list;
  QListWidgetItem *it;

  for (it = d->ui->m_homePageList->item(0); it;) {
    int item = KMyMoneyUtils::stringToHomePageItem(it->text());
    if (it->checkState() == Qt::Unchecked)
      item = -item;
    list += QString::number(item);
    if (d->ui->m_homePageList->count() > (d->ui->m_homePageList->row(it) + 1)) {
      it = d->ui->m_homePageList->item(d->ui->m_homePageList->row(it) + 1);
      if (it) {
        list += ',';
      }
    } else {
      break;
    }
  }

  // don't update the list
  d->m_noNeedToUpdateList = true;
  d->ui->kcfg_ItemList->setText(list);
  d->m_noNeedToUpdateList = false;
}

void KSettingsHome::slotSelectHomePageItem()
{
  Q_D(KSettingsHome);
  auto item = d->ui->m_homePageList->currentItem();
  d->ui->m_upButton->setEnabled(d->ui->m_homePageList->item(0) != item);
  d->ui->m_downButton->setEnabled(d->ui->m_homePageList->count() > (d->ui->m_homePageList->row(item) + 1));
}

void KSettingsHome::slotMoveUp()
{
  Q_D(KSettingsHome);
  auto item = d->ui->m_homePageList->currentItem();
  auto prev = d->ui->m_homePageList->item(d->ui->m_homePageList->row(item) - 1);
  int prevRow = d->ui->m_homePageList->row(prev);
  if (prev) {
    d->ui->m_homePageList->takeItem(d->ui->m_homePageList->row(item));
    d->ui->m_homePageList->insertItem(prevRow, item);
    d->ui->m_homePageList->setCurrentRow(d->ui->m_homePageList->row(item));
    slotSelectHomePageItem();
    slotUpdateItemList();
  }
}

void KSettingsHome::slotMoveDown()
{
  Q_D(KSettingsHome);
  auto item = d->ui->m_homePageList->currentItem();
  auto next = d->ui->m_homePageList->item(d->ui->m_homePageList->row(item) + 1);
  int nextRow = d->ui->m_homePageList->row(next);
  if (next) {
    d->ui->m_homePageList->takeItem(d->ui->m_homePageList->row(item));
    d->ui->m_homePageList->insertItem(nextRow, item);
    d->ui->m_homePageList->setCurrentRow(d->ui->m_homePageList->row(item));
    slotSelectHomePageItem();
    slotUpdateItemList();
  }
}
