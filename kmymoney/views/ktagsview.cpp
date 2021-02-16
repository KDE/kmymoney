/*
    SPDX-FileCopyrightText: 2012 Alessandro Russo <axela74@yahoo.it>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ktagsview.h"
#include "ktagsview_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QTimer>
#include <QMenu>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>
#include <KHelpClient>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyexception.h"
#include "mymoneymoney.h"
#include "mymoneyprice.h"
#include "kmymoneysettings.h"
#include "ktagreassigndlg.h"
#include "kmymoneyutils.h"
#include "kmymoneymvccombo.h"
#include "mymoneysecurity.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "mymoneyschedule.h"
#include "transaction.h"
#include "menuenums.h"

using namespace Icons;

/* -------------------------------------------------------------------------*/
/*                         KTransactionPtrVector                            */
/* -------------------------------------------------------------------------*/

// *** KTagsView Implementation ***
KTagsView::KTagsView(QWidget *parent) :
    KMyMoneyViewBase(*new KTagsViewPrivate(this), parent)
{
  typedef void(KTagsView::*KTagsViewFunc)();
  const QHash<eMenu::Action, KTagsViewFunc> actionConnections {
    {eMenu::Action::NewTag,    &KTagsView::slotNewTag},
    {eMenu::Action::RenameTag, &KTagsView::slotRenameTag},
    {eMenu::Action::DeleteTag, &KTagsView::slotDeleteTag}
  };

  for (auto a = actionConnections.cbegin(); a != actionConnections.cend(); ++a)
    connect(pActions[a.key()], &QAction::triggered, this, a.value());
}

KTagsView::~KTagsView()
{
}

void KTagsView::executeCustomAction(eView::Action action)
{
  Q_D(KTagsView);
  switch(action) {
    case eView::Action::Refresh:
      refresh();
      break;

    case eView::Action::SetDefaultFocus:
      QTimer::singleShot(0, d->m_searchWidget, SLOT(setFocus()));
      break;

    default:
      break;
  }
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

// This variant is only called when a single tag is selected and renamed.
void KTagsView::slotRenameSingleTag(QListWidgetItem* ta)
{
  Q_D(KTagsView);
  //if there is no current item selected, exit
  if (d->m_allowEditing == false || !d->ui->m_tagsList->currentItem() || ta != d->ui->m_tagsList->currentItem())
    return;

  //qDebug() << "[KTagsView::slotRenameTag]";
  // create a copy of the new name without appended whitespaces
  auto new_name = ta->text();
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
      KMessageBox::detailedSorry(this, i18n("Unable to modify tag"), QString::fromLatin1(e.what()));
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
  slotSelectTags(tagsList);

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
    qDebug("exception during display of tag: %s", e.what());
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
  d->ui->m_register->setSortOrder(KMyMoneySettings::sortSearchView());

  // clear the register
  d->ui->m_register->clear();

  if (d->m_tag.id().isEmpty() || !d->ui->m_tabWidget->isEnabled()) {
    d->ui->m_balanceLabel->setText(i18n("Balance: %1", balance.formatMoney(file->baseCurrency().smallestAccountFraction())));
    return;
  }

  // setup the list and the pointer vector
  MyMoneyTransactionFilter filter;
  filter.setConsiderCategorySplits();
  filter.addTag(d->m_tag.id());
  filter.setDateFilter(KMyMoneySettings::startDate().date(), QDate());

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
      KMessageBox::detailedSorry(this, i18n("Unable to modify tag"), QString::fromLatin1(e.what()));
    }
  }
}

void KTagsView::showEvent(QShowEvent* event)
{
  if (MyMoneyFile::instance()->storageAttached()) {
    Q_D(KTagsView);
    if (d->m_needLoad)
      d->init();

    emit customActionRequested(View::Tags, eView::Action::AboutToShow);

    if (d->m_needsRefresh)
      refresh();

    QList<MyMoneyTag> list;
    selectedTags(list);
    slotSelectTags(list);
  }

  // don't forget base class implementation
  QWidget::showEvent(event);
}

void KTagsView::updateTagActions(const QList<MyMoneyTag>& tags)
{
  pActions[eMenu::Action::NewTag]->setEnabled(true);
  const auto tagsCount = tags.count();
  auto b = tagsCount == 1 ? true : false;
  pActions[eMenu::Action::RenameTag]->setEnabled(b);
  b = tagsCount >= 1 ? true : false;
  pActions[eMenu::Action::DeleteTag]->setEnabled(b);
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
      emit selectByVariant(QVariantList {QVariant(t->split().accountId()), QVariant(t->transaction().id())}, eView::Intent::ShowTransaction);
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

        KMyMoneyRegister::RegisterItem *registerItem = 0;
        for (i = 0; i < d->ui->m_register->rowCount(); ++i) {
          registerItem = d->ui->m_register->itemAtRow(i);
          KMyMoneyRegister::Transaction* t = dynamic_cast<KMyMoneyRegister::Transaction*>(registerItem);
          if (t) {
            if (t->transaction().id() == transactionId && t->transaction().accountReferenced(accountId)) {
              d->ui->m_register->selectItem(registerItem);
              d->ui->m_register->ensureItemVisible(registerItem);
              break;
            }
          }
        }
        // quit out of outer for() loop
        break;
      }
    }
  } catch (const MyMoneyException &e) {
    qWarning("Unexpected exception in KTagsView::slotSelectTagAndTransaction %s", e.what());
  }
}

void KTagsView::slotSelectTagAndTransaction(const QString& tagId)
{
  slotSelectTagAndTransaction(tagId, QString(), QString());
}

void KTagsView::slotShowTagsMenu(const QPoint& /*ta*/)
{
  Q_D(KTagsView);
  auto item = dynamic_cast<KTagListItem*>(d->ui->m_tagsList->currentItem());
  if (item) {
    slotSelectTag();
    pMenus[eMenu::Menu::Tag]->exec(QCursor::pos());
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

void KTagsView::slotSelectTags(const QList<MyMoneyTag>& list)
{
  Q_D(KTagsView);
  d->m_selectedTags = list;
  updateTagActions(list);
}

void KTagsView::slotNewTag()
{
  QString id;
  KMyMoneyUtils::newTag(i18n("New Tag"), id);
  slotSelectTagAndTransaction(id);
}

void KTagsView::slotRenameTag()
{
  Q_D(KTagsView);
  if (d->ui->m_tagsList->currentItem() && d->ui->m_tagsList->selectedItems().count() == 1) {
    slotStartRename(d->ui->m_tagsList->currentItem());
  }
}

void KTagsView::slotDeleteTag()
{
  Q_D(KTagsView);
  if (d->m_selectedTags.isEmpty())
    return; // shouldn't happen

  const auto file = MyMoneyFile::instance();

  // first create list with all non-selected tags
  QList<MyMoneyTag> remainingTags = file->tagList();
  QList<MyMoneyTag>::iterator it_ta;
  for (it_ta = remainingTags.begin(); it_ta != remainingTags.end();) {
    if (d->m_selectedTags.contains(*it_ta)) {
      it_ta = remainingTags.erase(it_ta);
    } else {
      ++it_ta;
    }
  }

  // get confirmation from user
  QString prompt;
  if (d->m_selectedTags.size() == 1)
    prompt = i18n("<p>Do you really want to remove the tag <b>%1</b>?</p>", d->m_selectedTags.front().name());
  else
    prompt = i18n("Do you really want to remove all selected tags?");

  if (KMessageBox::questionYesNo(this, prompt, i18n("Remove Tag")) == KMessageBox::No)
    return;

  MyMoneyFileTransaction ft;
  try {
    // create a transaction filter that contains all tags selected for removal
    MyMoneyTransactionFilter f = MyMoneyTransactionFilter();
    for (QList<MyMoneyTag>::const_iterator it = d->m_selectedTags.constBegin();
         it != d->m_selectedTags.constEnd(); ++it) {
      f.addTag((*it).id());
    }
    // request a list of all transactions that still use the tags in question
    auto translist = file->transactionList(f);
//     qDebug() << "[KTagsView::slotDeleteTag]  " << translist.count() << " transaction still assigned to tags";

    // now get a list of all schedules that make use of one of the tags
    QList<MyMoneySchedule> used_schedules;
    foreach (const auto schedule, file->scheduleList()) {
      // loop over all splits in the transaction of the schedule
      foreach (const auto split, schedule.transaction().splits()) {
        for (auto i = 0; i < split.tagIdList().size(); ++i) {
          // is the tag in the split to be deleted?
          if (d->tagInList(d->m_selectedTags, split.tagIdList()[i])) {
            used_schedules.push_back(schedule); // remember this schedule
            break;
          }
        }
      }
    }
//     qDebug() << "[KTagsView::slotDeleteTag]  " << used_schedules.count() << " schedules use one of the selected tags";

    MyMoneyTag newTag;
    // if at least one tag is still referenced, we need to reassign its transactions first
    if (!translist.isEmpty() || !used_schedules.isEmpty()) {
      // show error message if no tags remain
      //FIXME-ALEX Tags are optional so we can delete all of them and simply delete every tagId from every transaction
      if (remainingTags.isEmpty()) {
        KMessageBox::sorry(this, i18n("At least one transaction/scheduled transaction is still referenced by a tag. "
                                      "Currently you have all tags selected. However, at least one tag must remain so "
                                      "that the transaction/scheduled transaction can be reassigned."));
        return;
      }

      // show transaction reassignment dialog
      auto dlg = new KTagReassignDlg(this);
      KMyMoneyMVCCombo::setSubstringSearchForChildren(dlg, !KMyMoneySettings::stringMatchFromStart());
      auto tag_id = dlg->show(remainingTags);
      delete dlg; // and kill the dialog
      if (tag_id.isEmpty())  //FIXME-ALEX Let the user choose to not reassign a to-be deleted tag to another one.
        return; // the user aborted the dialog, so let's abort as well

      newTag = file->tag(tag_id);

      // TODO : check if we have a report that explicitly uses one of our tags
      //        and issue an appropriate warning
      try {
        // now loop over all transactions and reassign tag
        for (auto& transaction : translist) {
          // create a copy of the splits list in the transaction
          // loop over all splits
          for (auto& split : transaction.splits()) {
            QList<QString> tagIdList = split.tagIdList();
            for (int i = 0; i < tagIdList.size(); ++i) {
              // if the split is assigned to one of the selected tags, we need to modify it
              if (d->tagInList(d->m_selectedTags, tagIdList[i])) {
                tagIdList.removeAt(i);
                if (tagIdList.indexOf(tag_id) == -1)
                  tagIdList.append(tag_id);
                i = -1; // restart from the first element
              }
            }
            split.setTagIdList(tagIdList); // first modify tag list in current split
            // then modify the split in our local copy of the transaction list
            transaction.modifySplit(split); // this does not modify the list object 'splits'!
          } // for - Splits
          file->modifyTransaction(transaction);  // modify the transaction in the MyMoney object
        } // for - Transactions

        // now loop over all schedules and reassign tags
        for (auto& schedule : used_schedules) {
          // create copy of transaction in current schedule
          auto trans = schedule.transaction();
          // create copy of lists of splits
          for (auto& split : trans.splits()) {
            QList<QString> tagIdList = split.tagIdList();
            for (auto i = 0; i < tagIdList.size(); ++i) {
              if (d->tagInList(d->m_selectedTags, tagIdList[i])) {
                tagIdList.removeAt(i);
                if (tagIdList.indexOf(tag_id) == -1)
                  tagIdList.append(tag_id);
                i = -1; // restart from the first element
              }
            }
            split.setTagIdList(tagIdList);
            trans.modifySplit(split); // does not modify the list object 'splits'!
          } // for - Splits
          // store transaction in current schedule
          schedule.setTransaction(trans);
          file->modifySchedule(schedule);  // modify the schedule in the MyMoney engine
        } // for - Schedules

      } catch (const MyMoneyException &e) {
        KMessageBox::detailedSorry(this, i18n("Unable to reassign tag of transaction/split"), QString::fromLatin1(e.what()));
      }
    } // if !translist.isEmpty()

    // now loop over all selected tags and remove them
    for (QList<MyMoneyTag>::iterator it = d->m_selectedTags.begin();
         it != d->m_selectedTags.end(); ++it) {
      file->removeTag(*it);
    }

    ft.commit();

    // If we just deleted the tags, they sure don't exist anymore
    slotSelectTags(QList<MyMoneyTag>());

  } catch (const MyMoneyException &e) {
    KMessageBox::detailedSorry(this, i18n("Unable to remove tag(s)"), QString::fromLatin1(e.what()));
  }
}
