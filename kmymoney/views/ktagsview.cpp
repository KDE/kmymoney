/*
 * Copyright 2012       Alessandro Russo <axela74@yahoo.it>
 * Copyright 2017       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 * Copyright 2020       Thomas Baumgart <tbaumgart@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ktagsview.h"
#include "ktagsview_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QTimer>
#include <QMenu>
#include <QComboBox>

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
  switch(action) {
    case eView::Action::SetDefaultFocus:
      {
        Q_D(KTagsView);
        QMetaObject::invokeMethod(d->ui->m_searchWidget, "setFocus");
      }
      break;

    default:
      break;
  }
}

void KTagsView::slotRenameSingleTag(const QModelIndex& idx, const QVariant& value)
{
  Q_D(KTagsView);
  //if there is no current item selected, exit
  if (d->m_allowEditing == false || !idx.isValid())
    return;

  //qDebug() << "[KTagsView::slotRenameTag]";
  // create a copy of the new name without appended whitespaces
  const auto new_name = value.toString();
  if (d->m_tag.name() != new_name) {
    MyMoneyFileTransaction ft;
    try {
      // check if we already have a tag with the new name
      const auto tag = MyMoneyFile::instance()->tagByName(new_name);
      // if the name already exists, ask the user whether he's sure to keep the name
      if (!tag.id().isEmpty()) {
        if (KMessageBox::questionYesNo(this,
                                        i18n("A tag with the name '%1' already exists. It is not advisable to have "
                                            "multiple tags with the same identification name. Are you sure you would like "
                                            "to rename the tag?", new_name)) != KMessageBox::Yes) {
          return;
        }
      }

      d->m_tag.setName(new_name);
      d->m_newName = new_name;
      MyMoneyFile::instance()->modifyTag(d->m_tag);

      // the above call to modifyTag will reload the view so
      // all references and pointers to the view have to be
      // re-established.

      // make sure, that the record is visible even if it moved
      // out of sight due to the rename operation
      d->ensureTagVisible(d->m_tag.id());

      ft.commit();

    } catch (const MyMoneyException &e) {
      KMessageBox::detailedSorry(this, i18n("Unable to modify tag"), QString::fromLatin1(e.what()));
    }
  }
}

QList<MyMoneyTag> KTagsView::selectedTags() const
{
  Q_D(const KTagsView);
  QList<MyMoneyTag> tags;
  auto baseModel = MyMoneyFile::instance()->tagsModel();
  const QModelIndexList selection = d->ui->m_tagsList->selectionModel()->selectedIndexes();
  for (const auto& idx : selection) {
    const auto baseIdx = baseModel->mapToBaseSource(idx);
    const auto tag = baseModel->itemByIndex(baseIdx);
    if (!tag.id().isEmpty()) {
      tags.append(tag);
    }
  }
  return tags;
}

void KTagsView::slotTagSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  Q_UNUSED(selected)
  Q_UNUSED(deselected)

  Q_D(KTagsView);
  // check if the content of a currently selected tag was modified
  // and ask to store the data
  if (d->ui->m_updateButton->isEnabled()) {
    if (KMessageBox::questionYesNo(this, QString("<qt>%1</qt>").arg(
                                     i18n("Do you want to save the changes for <b>%1</b>?", d->m_newName)),
                                   i18n("Save changes")) == KMessageBox::Yes) {
      slotUpdateTag();
    }
  }

  // loop over all tags and count the number of tags, also
  // obtain last selected tag
  const auto selectedItems = selectedTags();
  updateTagActions(selectedItems);

  const auto selectedItemCount = selectedItems.count();

  if (selectedItemCount == 0) {
    d->ui->m_tabWidget->setEnabled(false); // disable tab widget
    d->ui->m_balanceLabel->hide();
    d->ui->m_deleteButton->setEnabled(false); //disable delete and rename button
    d->ui->m_renameButton->setEnabled(false);
    d->clearItemData();
    d->m_tag = MyMoneyTag();
    return; // make sure we don't access an undefined tag
  }

  d->ui->m_deleteButton->setEnabled(true); //re-enable delete button

  // if we have multiple tags selected, clear and disable the tag information
  if (selectedItemCount > 1) {
    d->ui->m_tabWidget->setEnabled(false); // disable tab widget
    d->ui->m_renameButton->setEnabled(false); // disable also the rename button
    d->ui->m_balanceLabel->hide();
    d->clearItemData();
  } else {
    d->ui->m_renameButton->setEnabled(true);
  }

  // otherwise we have just one selected, enable tag information widget and renameButton
  d->ui->m_tabWidget->setEnabled(true);
  d->ui->m_balanceLabel->show();

  // as of now we are updating only the last selected tag, and until
  // selection mode of the QListView has been changed to Extended, this
  // will also be the only selection and behave exactly as before - Andreas
  d->m_tag = MyMoneyTag();
  if (!selectedItems.isEmpty()) {
    d->m_tag = selectedItems.at(0);
  }

  try {
    d->m_newName = d->m_tag.name();
    d->loadDetails();
    slotTagDataChanged();

    d->showTransactions();

  } catch (const MyMoneyException &e) {
    qDebug("exception during display of tag: %s", e.what());
    d->m_tag = MyMoneyTag();
  }
  d->m_allowEditing = true;
}

void KTagsView::slotModelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
  Q_D(KTagsView);
  QModelIndex idx;
  if (topLeft.model() == d->m_renameProxyModel) {
    const auto baseModel = MyMoneyFile::instance()->tagsModel();
    for (int row = topLeft.row(); row <= bottomRight.row(); ++row) {
      idx = topLeft.model()->index(row, 0, topLeft.parent());
      if (d->m_tag.id() == idx.data(eMyMoney::Model::IdRole).toString()) {
        d->m_tag = baseModel->itemById(d->m_tag.id());
        d->loadDetails();
      }
    }
  }
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
  Q_D(KTagsView);
  if (d->m_needLoad) {
    d->init();
    connect(d->ui->m_filterBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](int idx) {
      Q_D(KTagsView);
      d->m_renameProxyModel->setReferenceFilter(d->ui->m_filterBox->itemData(idx));
    } );

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

void KTagsView::slotSelectTagAndTransaction(const QString& tagId, const QString& accountId, const QString& transactionId)
{
  if (!isVisible())
    return;

  /// @todo port to new model code
#if 0
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
#endif
}

void KTagsView::slotSelectTagAndTransaction(const QString& tagId)
{
  slotSelectTagAndTransaction(tagId, QString(), QString());
}

void KTagsView::slotShowTagsMenu(const QPoint& /*ta*/)
{
  Q_D(KTagsView);
  if (!d->ui->m_tagsList->selectionModel()->selectedIndexes().isEmpty()) {
    pMenus[eMenu::Menu::Tag]->exec(QCursor::pos());
  }
}

void KTagsView::slotHelp()
{
  KHelpClient::invokeHelp("details.tags.attributes");
  //FIXME-ALEX update help file
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
  if (d->ui->m_tagsList->currentIndex().isValid() && d->ui->m_tagsList->selectionModel()->selectedIndexes().count() == 1) {
    d->ui->m_tagsList->edit(d->ui->m_tagsList->currentIndex());
  }
}

void KTagsView::slotDeleteTag()
{
  Q_D(KTagsView);
  d->m_selectedTags = selectedTags();
  if (d->m_selectedTags.isEmpty())
    return; // shouldn't happen

  const auto file = MyMoneyFile::instance();

  // first create list with all non-selected tags
  QList<MyMoneyTag> remainingTags = file->tagList();
  QList<QString> selectedTagIds;
  QList<MyMoneyTag>::iterator it_ta;
  for (it_ta = remainingTags.begin(); it_ta != remainingTags.end();) {
    if (d->m_selectedTags.contains(*it_ta)) {
      selectedTagIds.append((*it_ta).id());
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
      QPointer<KTagReassignDlg> dlg = new KTagReassignDlg(this);
      KMyMoneyMVCCombo::setSubstringSearchForChildren(dlg, !KMyMoneySettings::stringMatchFromStart());
      auto tag_id = dlg->show(selectedTagIds);
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
          for (auto split : transaction.splits()) {
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

  } catch (const MyMoneyException &e) {
    KMessageBox::detailedSorry(this, i18n("Unable to remove tag(s)"), QString::fromLatin1(e.what()));
  }
}
