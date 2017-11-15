/***************************************************************************
                          ktagsview.cpp
                          -------------
    begin                : Sat Oct 13 2012
    copyright            : (C) 2012 by Alessandro Russo <axela74@yahoo.it>
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

#include "ktagsview.h"
#include "ktagsview_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QTimer>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>
#include <KHelpClient>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyprice.h"
#include "kmymoneyglobalsettings.h"
#include "mymoneysecurity.h"

using namespace Icons;

/* -------------------------------------------------------------------------*/
/*                         KTransactionPtrVector                            */
/* -------------------------------------------------------------------------*/

// *** KTagListItem Implementation ***

/**
  * This class represents an item in the tags list view.
  */
class KTagListItem : public QListWidgetItem
{
public:
  /**
    * Constructor to be used to construct a tag entry object.
    *
    * @param parent pointer to the QListWidget object this entry should be
    *               added to.
    * @param tag    const reference to MyMoneyTag for which
    *               the QListWidget entry is constructed
    */
  explicit KTagListItem(QListWidget *parent, const MyMoneyTag& tag);
  ~KTagListItem();

  MyMoneyTag tag() const;

private:
  MyMoneyTag  m_tag;
};

KTagListItem::KTagListItem(QListWidget *parent, const MyMoneyTag& tag) :
    QListWidgetItem(parent, QListWidgetItem::UserType),
    m_tag(tag)
{
  setText(tag.name());
  // allow in column rename
  setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

KTagListItem::~KTagListItem()
{
}

MyMoneyTag KTagListItem::tag() const
{
  return m_tag;
}

// *** KTagsView Implementation ***
KTagsView::KTagsView(QWidget *parent) :
    KMyMoneyViewBase(*new KTagsViewPrivate(this), parent)
{
  Q_D(KTagsView);
  d->m_needLoad = true;
  d->m_inSelection = false;
  d->m_allowEditing = true;
  d->m_tagFilterType = 0;
}

KTagsView::~KTagsView()
{
  Q_D(KTagsView);
  if (!d->m_needLoad) {
    // remember the splitter settings for startup
    KConfigGroup grp = KSharedConfig::openConfig()->group("Last Use Settings");
    grp.writeEntry("KTagsViewSplitterSize", d->ui->m_splitter->saveState());
    grp.sync();
  }
}

void KTagsView::setDefaultFocus()
{
  Q_D(KTagsView);
  QTimer::singleShot(0, d->m_searchWidget, SLOT(setFocus()));
}

void KTagsView::refresh()
{

  Q_D(KTagsView);
  if (isVisible()) {
    if (d->m_inSelection)
      QTimer::singleShot(0, this, SLOT(refresh()));
    else
      loadTags();
    d->m_needsRefresh = false;
  } else {
    d->m_needsRefresh = true;
  }
}

void KTagsView::slotStartRename(QListWidgetItem* item)
{
  Q_D(KTagsView);
  d->m_allowEditing = true;
  d->ui->m_tagsList->editItem(item);
}

void KTagsView::slotRenameButtonCliked()
{
  Q_D(KTagsView);
  if (d->ui->m_tagsList->currentItem() && d->ui->m_tagsList->selectedItems().count() == 1) {
    slotStartRename(d->ui->m_tagsList->currentItem());
  }
}

// This variant is only called when a single tag is selected and renamed.
void KTagsView::slotRenameTag(QListWidgetItem* ta)
{
  Q_D(KTagsView);
  //if there is no current item selected, exit
  if (d->m_allowEditing == false || !d->ui->m_tagsList->currentItem() || ta != d->ui->m_tagsList->currentItem())
    return;

  //qDebug() << "[KTagsView::slotRenameTag]";
  // create a copy of the new name without appended whitespaces
  QString new_name = ta->text();
  if (d->m_tag.name() != new_name) {
    MyMoneyFileTransaction ft;
    try {
      // check if we already have a tag with the new name
      try {
        // this function call will throw an exception, if the tag
        // hasn't been found.
        MyMoneyFile::instance()->tagByName(new_name);
        // the name already exists, ask the user whether he's sure to keep the name
        if (KMessageBox::questionYesNo(this,
                                       i18n("A tag with the name '%1' already exists. It is not advisable to have "
                                            "multiple tags with the same identification name. Are you sure you would like "
                                            "to rename the tag?", new_name)) != KMessageBox::Yes) {
          ta->setText(d->m_tag.name());
          return;
        }
      } catch (const MyMoneyException &) {
        // all ok, the name is unique
      }

      d->m_tag.setName(new_name);
      d->m_newName = new_name;
      MyMoneyFile::instance()->modifyTag(d->m_tag);

      // the above call to modifyTag will reload the view so
      // all references and pointers to the view have to be
      // re-established.

      // make sure, that the record is visible even if it moved
      // out of sight due to the rename operation
      ensureTagVisible(d->m_tag.id());

      ft.commit();

    } catch (const MyMoneyException &e) {
      KMessageBox::detailedSorry(0, i18n("Unable to modify tag"),
                                 i18n("%1 thrown in %2:%3", e.what(), e.file(), e.line()));
    }
  } else {
    ta->setText(new_name);
  }
}

void KTagsView::ensureTagVisible(const QString& id)
{
  Q_D(KTagsView);
  for (int i = 0; i < d->ui->m_tagsList->count(); ++i) {
    KTagListItem* ta = dynamic_cast<KTagListItem*>(d->ui->m_tagsList->item(0));
    if (ta && ta->tag().id() == id) {
      d->ui->m_tagsList->scrollToItem(ta, QAbstractItemView::PositionAtCenter);

      d->ui->m_tagsList->setCurrentItem(ta);      // active item and deselect all others
      d->ui->m_tagsList->setCurrentRow(i, QItemSelectionModel::ClearAndSelect);   // and select it
      break;
    }
  }
}

void KTagsView::selectedTags(QList<MyMoneyTag>& tagsList) const
{
  Q_D(const KTagsView);
  QList<QListWidgetItem *> selectedItems = d->ui->m_tagsList->selectedItems();
  QList<QListWidgetItem *>::ConstIterator itemsIt = selectedItems.constBegin();
  while (itemsIt != selectedItems.constEnd()) {
    KTagListItem* item = dynamic_cast<KTagListItem*>(*itemsIt);
    if (item)
      tagsList << item->tag();
    ++itemsIt;
  }
}

void KTagsView::slotSelectTag(QListWidgetItem* cur, QListWidgetItem* prev)
{
  Q_D(KTagsView);
  Q_UNUSED(cur);
  Q_UNUSED(prev);

  d->m_allowEditing = false;
}

void KTagsView::slotSelectTag()
{
  Q_D(KTagsView);
  // check if the content of a currently selected tag was modified
  // and ask to store the data
  if (d->ui->m_updateButton->isEnabled()) {
    if (KMessageBox::questionYesNo(this, QString("<qt>%1</qt>").arg(
                                     i18n("Do you want to save the changes for <b>%1</b>?", d->m_newName)),
                                   i18n("Save changes")) == KMessageBox::Yes) {
      d->m_inSelection = true;
      slotUpdateTag();
      d->m_inSelection = false;
    }
  }
  // loop over all tags and count the number of tags, also
  // obtain last selected tag
  QList<MyMoneyTag> tagsList;
  selectedTags(tagsList);

  emit selectObjects(tagsList);

  if (tagsList.isEmpty()) {
    d->ui->m_tabWidget->setEnabled(false); // disable tab widget
    d->ui->m_balanceLabel->hide();
    d->ui->m_deleteButton->setEnabled(false); //disable delete and rename button
    d->ui->m_renameButton->setEnabled(false);
    clearItemData();
    d->m_tag = MyMoneyTag();
    return; // make sure we don't access an undefined tag
  }

  d->ui->m_deleteButton->setEnabled(true); //re-enable delete button

  // if we have multiple tags selected, clear and disable the tag information
  if (tagsList.count() > 1) {
    d->ui->m_tabWidget->setEnabled(false); // disable tab widget
    d->ui->m_renameButton->setEnabled(false); // disable also the rename button
    d->ui->m_balanceLabel->hide();
    clearItemData();
  } else d->ui->m_renameButton->setEnabled(true);

  // otherwise we have just one selected, enable tag information widget and renameButton
  d->ui->m_tabWidget->setEnabled(true);
  d->ui->m_balanceLabel->show();

  // as of now we are updating only the last selected tag, and until
  // selection mode of the QListView has been changed to Extended, this
  // will also be the only selection and behave exactly as before - Andreas
  try {
    d->m_tag = tagsList[0];

    d->m_newName = d->m_tag.name();
    d->ui->m_colorbutton->setEnabled(true);
    d->ui->m_colorbutton->setColor(d->m_tag.tagColor());
    d->ui->m_closed->setEnabled(true);
    d->ui->m_closed->setChecked(d->m_tag.isClosed());
    d->ui->m_notes->setEnabled(true);
    d->ui->m_notes->setText(d->m_tag.notes());
    slotTagDataChanged();

    showTransactions();

  } catch (const MyMoneyException &e) {
    qDebug("exception during display of tag: %s at %s:%ld", qPrintable(e.what()), qPrintable(e.file()), e.line());
    d->ui->m_register->clear();
    d->m_tag = MyMoneyTag();
  }
  d->m_allowEditing = true;
}

void KTagsView::clearItemData()
{
  Q_D(KTagsView);
  d->ui->m_colorbutton->setColor(QColor());
  d->ui->m_closed->setChecked(false);
  d->ui->m_notes->setText(QString());
  showTransactions();
}

void KTagsView::showTransactions()
{
  Q_D(KTagsView);
  MyMoneyMoney balance;
  auto file = MyMoneyFile::instance();
  MyMoneySecurity base = file->baseCurrency();

  // setup sort order
  d->ui->m_register->setSortOrder(KMyMoneyGlobalSettings::sortSearchView());

  // clear the register
  d->ui->m_register->clear();

  if (d->m_tag.id().isEmpty() || !d->ui->m_tabWidget->isEnabled()) {
    d->ui->m_balanceLabel->setText(i18n("Balance: %1", balance.formatMoney(file->baseCurrency().smallestAccountFraction())));
    return;
  }

  // setup the list and the pointer vector
  MyMoneyTransactionFilter filter;
  filter.addTag(d->m_tag.id());
  filter.setDateFilter(KMyMoneyGlobalSettings::startDate().date(), QDate());

  // retrieve the list from the engine
  file->transactionList(d->m_transactionList, filter);

  // create the elements for the register
  QList<QPair<MyMoneyTransaction, MyMoneySplit> >::const_iterator it;
  QMap<QString, int> uniqueMap;
  MyMoneyMoney deposit, payment;

  int splitCount = 0;
  bool balanceAccurate = true;
  for (it = d->m_transactionList.constBegin(); it != d->m_transactionList.constEnd(); ++it) {
    const MyMoneySplit& split = (*it).second;
    MyMoneyAccount acc = file->account(split.accountId());
    ++splitCount;
    uniqueMap[(*it).first.id()]++;

    KMyMoneyRegister::Register::transactionFactory(d->ui->m_register, (*it).first, (*it).second, uniqueMap[(*it).first.id()]);

    // take care of foreign currencies
    MyMoneyMoney val = split.shares().abs();
    if (acc.currencyId() != base.id()) {
      const MyMoneyPrice &price = file->price(acc.currencyId(), base.id());
      // in case the price is valid, we use it. Otherwise, we keep
      // a flag that tells us that the balance is somewhat inaccurate
      if (price.isValid()) {
        val *= price.rate(base.id());
      } else {
        balanceAccurate = false;
      }
    }

    if (split.shares().isNegative()) {
      payment += val;
    } else {
      deposit += val;
    }
  }
  balance = deposit - payment;

  // add the group markers
  d->ui->m_register->addGroupMarkers();

  // sort the transactions according to the sort setting
  d->ui->m_register->sortItems();

  // remove trailing and adjacent markers
  d->ui->m_register->removeUnwantedGroupMarkers();

  d->ui->m_register->updateRegister(true);

  // we might end up here with updates disabled on the register so
  // make sure that we enable updates here
  d->ui->m_register->setUpdatesEnabled(true);
  d->ui->m_balanceLabel->setText(i18n("Balance: %1%2",
                               balanceAccurate ? "" : "~",
                               balance.formatMoney(file->baseCurrency().smallestAccountFraction())));
}

void KTagsView::slotTagDataChanged()
{
  Q_D(KTagsView);
  auto rc = false;

  if (d->ui->m_tabWidget->isEnabled()) {
    rc |= ((d->m_tag.tagColor().isValid() != d->ui->m_colorbutton->color().isValid())
           || (d->ui->m_colorbutton->color().isValid() && d->m_tag.tagColor() != d->ui->m_colorbutton->color()));
    rc |= (d->ui->m_closed->isChecked() != d->m_tag.isClosed());
    rc |= ((d->m_tag.notes().isEmpty() != d->ui->m_notes->toPlainText().isEmpty())
           || (!d->ui->m_notes->toPlainText().isEmpty() && d->m_tag.notes() != d->ui->m_notes->toPlainText()));
  }
  d->ui->m_updateButton->setEnabled(rc);
}

void KTagsView::slotUpdateTag()
{
  Q_D(KTagsView);
  if (d->ui->m_updateButton->isEnabled()) {
    MyMoneyFileTransaction ft;
    d->ui->m_updateButton->setEnabled(false);
    try {
      d->m_tag.setName(d->m_newName);
      d->m_tag.setTagColor(d->ui->m_colorbutton->color());
      d->m_tag.setClosed(d->ui->m_closed->isChecked());
      d->m_tag.setNotes(d->ui->m_notes->toPlainText());

      MyMoneyFile::instance()->modifyTag(d->m_tag);
      ft.commit();

    } catch (const MyMoneyException &e) {
      KMessageBox::detailedSorry(0, i18n("Unable to modify tag"),
                                 i18n("%1 thrown in %2:%3", e.what(), e.file(), e.line()));
    }
  }
}

void KTagsView::showEvent(QShowEvent* event)
{
  Q_D(KTagsView);
  if (d->m_needLoad)
    d->init();

  emit aboutToShow(View::Tags);

  if (d->m_needsRefresh)
    refresh();

  // don't forget base class implementation
  QWidget::showEvent(event);

  QList<MyMoneyTag> list;
  selectedTags(list);
  emit selectObjects(list);
}

void KTagsView::loadTags()
{
  Q_D(KTagsView);
  if (d->m_inSelection)
    return;

  QMap<QString, bool> isSelected;
  QString id;
  MyMoneyFile* file = MyMoneyFile::instance();

  // remember which items are selected in the list
  QList<QListWidgetItem *> selectedItems = d->ui->m_tagsList->selectedItems();
  QList<QListWidgetItem *>::const_iterator tagsIt = selectedItems.constBegin();

  while (tagsIt != selectedItems.constEnd()) {
    KTagListItem* item = dynamic_cast<KTagListItem*>(*tagsIt);
    if (item)
      isSelected[item->tag().id()] = true;
    ++tagsIt;
  }

  // keep current selected item
  KTagListItem *currentItem = static_cast<KTagListItem *>(d->ui->m_tagsList->currentItem());
  if (currentItem)
    id = currentItem->tag().id();

  d->m_allowEditing = false;
  // clear the list
  d->m_searchWidget->clear();
  d->m_searchWidget->updateSearch();
  d->ui->m_tagsList->clear();
  d->ui->m_register->clear();
  currentItem = 0;

  QList<MyMoneyTag>list = file->tagList();
  QList<MyMoneyTag>::ConstIterator it;

  for (it = list.constBegin(); it != list.constEnd(); ++it) {
    if (d->m_tagFilterType == (int)eView::Tag::All ||
        (d->m_tagFilterType == (int)eView::Tag::Referenced && file->isReferenced(*it)) ||
        (d->m_tagFilterType == (int)eView::Tag::Unused && !file->isReferenced(*it)) ||
        (d->m_tagFilterType == (int)eView::Tag::Opened && !(*it).isClosed()) ||
        (d->m_tagFilterType == (int)eView::Tag::Closed && (*it).isClosed())) {
      KTagListItem* item = new KTagListItem(d->ui->m_tagsList, *it);
      if (item->tag().id() == id)
        currentItem = item;
      if (isSelected[item->tag().id()])
        item->setSelected(true);
    }
  }
  d->ui->m_tagsList->sortItems();

  if (currentItem) {
    d->ui->m_tagsList->setCurrentItem(currentItem);
    d->ui->m_tagsList->scrollToItem(currentItem);
  }

  slotSelectTag(0, 0);
  d->m_allowEditing = true;
}

void KTagsView::slotSelectTransaction()
{
  Q_D(KTagsView);
  QList<KMyMoneyRegister::RegisterItem*> list = d->ui->m_register->selectedItems();
  if (!list.isEmpty()) {
    KMyMoneyRegister::Transaction* t = dynamic_cast<KMyMoneyRegister::Transaction*>(list[0]);
    if (t)
      emit transactionSelected(t->split().accountId(), t->transaction().id());
  }
}

void KTagsView::slotSelectTagAndTransaction(const QString& tagId, const QString& accountId, const QString& transactionId)
{
  if (!isVisible())
    return;

  Q_D(KTagsView);
  try {
    // clear filter
    d->m_searchWidget->clear();
    d->m_searchWidget->updateSearch();

    // deselect all other selected items
    QList<QListWidgetItem *> selectedItems = d->ui->m_tagsList->selectedItems();
    QList<QListWidgetItem *>::const_iterator tagsIt = selectedItems.constBegin();
    while (tagsIt != selectedItems.constEnd()) {
      KTagListItem* item = dynamic_cast<KTagListItem*>(*tagsIt);
      if (item)
        item->setSelected(false);
      ++tagsIt;
    }

    // find the tag in the list
    QListWidgetItem* it;
    for (int i = 0; i < d->ui->m_tagsList->count(); ++i) {
      it = d->ui->m_tagsList->item(i);
      KTagListItem* item = dynamic_cast<KTagListItem *>(it);
      if (item && item->tag().id() == tagId) {
        d->ui->m_tagsList->scrollToItem(it, QAbstractItemView::PositionAtCenter);

        d->ui->m_tagsList->setCurrentItem(it);     // active item and deselect all others
        d->ui->m_tagsList->setCurrentRow(i, QItemSelectionModel::ClearAndSelect); // and select it

        //make sure the tag selection is updated and transactions are updated accordingly
        slotSelectTag();

        KMyMoneyRegister::RegisterItem *item = 0;
        for (int i = 0; i < d->ui->m_register->rowCount(); ++i) {
          item = d->ui->m_register->itemAtRow(i);
          KMyMoneyRegister::Transaction* t = dynamic_cast<KMyMoneyRegister::Transaction*>(item);
          if (t) {
            if (t->transaction().id() == transactionId && t->transaction().accountReferenced(accountId)) {
              d->ui->m_register->selectItem(item);
              d->ui->m_register->ensureItemVisible(item);
              break;
            }
          }
        }
        // quit out of for() loop
        break;
      }
    }
  } catch (const MyMoneyException &e) {
    qWarning("Unexpected exception in KTagsView::slotSelectTagAndTransaction %s", qPrintable(e.what()));
  }
}

void KTagsView::slotSelectTagAndTransaction(const QString& tagId)
{
  slotSelectTagAndTransaction(tagId, QString(), QString());
}

void KTagsView::slotOpenContextMenu(const QPoint& /*ta*/)
{
  Q_D(KTagsView);
  auto item = dynamic_cast<KTagListItem*>(d->ui->m_tagsList->currentItem());
  if (item) {
    slotSelectTag();
    emit openContextMenu(item->tag());
  }
}

void KTagsView::slotHelp()
{
  KHelpClient::invokeHelp("details.tags.attributes");
  //FIXME-ALEX update help file
}

void KTagsView::slotChangeFilter(int index)
{
  Q_D(KTagsView);
  //update the filter type then reload the tags list
  d->m_tagFilterType = index;
  loadTags();
}
