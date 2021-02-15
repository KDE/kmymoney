/*
    SPDX-FileCopyrightText: 2012 Alessandro Russo <axela74@yahoo.it>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
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

#include <KHelpClient>
#include <KPageWidgetItem>

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
  Q_D(KTagsView);
  switch(action) {
    case eView::Action::SetDefaultFocus:
      QMetaObject::invokeMethod(d->ui->m_searchWidget, "setFocus");
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

void KTagsView::aboutToShow()
{
  Q_D(KTagsView);
  d->loadDetails();

  // don't forget base class logic
  KMyMoneyViewBase::aboutToShow();
}

void KTagsView::aboutToHide()
{
  Q_D(KTagsView);

  d->finalizePendingChanges();

  // don't forget base class logic
  KMyMoneyViewBase::aboutToHide();
}

void KTagsView::slotTagSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  Q_UNUSED(selected)
  Q_UNUSED(deselected)

  Q_D(KTagsView);
  d->finalizePendingChanges();

  // loop over all tags and count the number of tags, also
  // obtain last selected tag

  d->m_selections.clearSelections();
  for (const auto& idx : selected.indexes()) {
    d->m_selections.addSelection(SelectedObjects::Tag, idx.data(eMyMoney::Model::IdRole).toString());
  }

  if (d->m_selections.selection(SelectedObjects::Tag).isEmpty()) {
    d->m_tag = MyMoneyTag();
  } else {
    d->m_tag = MyMoneyFile::instance()->tagsModel()->itemById(d->m_selections.selection(SelectedObjects::Tag).at(0));
  }

  emit requestSelectionChange(d->m_selections);

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
  d->m_havePendingChanges = false;

  if (d->ui->m_tabWidget->isEnabled()) {
        d->m_havePendingChanges |= ((d->m_tag.tagColor().isValid() != d->ui->m_colorbutton->color().isValid())
           || (d->ui->m_colorbutton->color().isValid() && d->m_tag.tagColor() != d->ui->m_colorbutton->color()));
        d->m_havePendingChanges |= (d->ui->m_closed->isChecked() != d->m_tag.isClosed());
        d->m_havePendingChanges |= ((d->m_tag.notes().isEmpty() != d->ui->m_notes->toPlainText().isEmpty())
           || (!d->ui->m_notes->toPlainText().isEmpty() && d->m_tag.notes() != d->ui->m_notes->toPlainText()));
  }
  d->m_updateAction->setEnabled(d->m_havePendingChanges);
}

void KTagsView::slotUpdateTag()
{
  Q_D(KTagsView);
  if (d->m_havePendingChanges) {
    MyMoneyFileTransaction ft;
    try {
      d->m_tag.setName(d->m_newName);
      d->m_tag.setTagColor(d->ui->m_colorbutton->color());
      d->m_tag.setClosed(d->ui->m_closed->isChecked());
      d->m_tag.setNotes(d->ui->m_notes->toPlainText());

      MyMoneyFile::instance()->modifyTag(d->m_tag);
      ft.commit();

      d->m_updateAction->setEnabled(false);
      d->m_havePendingChanges = false;

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

    connect(d->ui->m_tagsList, &QWidget::customContextMenuRequested, this, [&](QPoint pos) {
      Q_D(KTagsView);
      emit requestCustomContextMenu(eMenu::Menu::Tag, d->ui->m_tagsList->mapToGlobal(pos));
    });

    connect(d->ui->m_register, &QWidget::customContextMenuRequested, this, [&](QPoint pos) {
      Q_D(KTagsView);
      emit requestCustomContextMenu(eMenu::Menu::Transaction, d->ui->m_register->mapToGlobal(pos));
    });

  }
  // don't forget base class implementation
  QWidget::showEvent(event);
}

void KTagsView::updateActions(const SelectedObjects& selections)
{
  Q_D(KTagsView);

  // needs complete initialization
  if (d->m_needLoad) {
    return;
  }

  pActions[eMenu::Action::DeleteTag]->setEnabled(false);
  pActions[eMenu::Action::RenameTag]->setEnabled(false);

  switch(selections.selection(SelectedObjects::Tag).count()) {
    case 0:
      d->ui->m_tabWidget->setEnabled(false); // disable tab widget
      d->ui->m_balanceLabel->hide();
      d->clearItemData();
      break;
    case 1:
      d->ui->m_tabWidget->setEnabled(true); // disable tab widget
      d->ui->m_balanceLabel->show();
      pActions[eMenu::Action::DeleteTag]->setEnabled(true);
      pActions[eMenu::Action::RenameTag]->setEnabled(true);
      break;
    default:
      d->ui->m_tabWidget->setEnabled(false); // disable tab widget
      d->ui->m_balanceLabel->hide();
      pActions[eMenu::Action::DeleteTag]->setEnabled(true);
      d->clearItemData();
      break;
  }
}

void KTagsView::slotSelectTagAndTransaction(const QString& tagId, const QString& accountId, const QString& transactionId)
{
  Q_D(KTagsView);
  if (!isVisible())
    return;

  /// @todo port to new model code
  const auto model = MyMoneyFile::instance()->tagsModel();
  const auto baseIdx = model->indexById(tagId);
  auto idx = model->mapFromBaseSource(d->m_renameProxyModel, baseIdx);
  if (!idx.isValid()) {
    // item not found, maybe it is not visible due to filter and search
    // clear out any filter so it may become visible
    d->ui->m_searchWidget->clear();
    d->ui->m_filterBox->setCurrentIndex(0);
    // and try again
    idx = model->mapFromBaseSource(d->m_renameProxyModel, baseIdx);
  }
  if (idx.isValid()) {
    d->ui->m_tagsList->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::ClearAndSelect);
  }

#if 0
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
  QList<MyMoneyTag> selectedTags;
  const auto file = MyMoneyFile::instance();
  const auto model = file->tagsModel();
  QModelIndex baseIdx;

  for (const auto& idx : d->ui->m_tagsList->selectionModel()->selectedIndexes()) {
    baseIdx = model->mapToBaseSource(idx);
    const auto tag = model->itemByIndex(baseIdx);
    if (!tag.id().isEmpty()) {
      selectedTags.append(tag);
    }
  }
  if (selectedTags.isEmpty())
    return; // shouldn't happen

  // first create list with all non-selected tags
  QList<MyMoneyTag> remainingTags = file->tagList();
  QList<MyMoneyTag>::iterator it_ta;
  for (it_ta = remainingTags.begin(); it_ta != remainingTags.end();) {
    if (selectedTags.contains(*it_ta)) {
      it_ta = remainingTags.erase(it_ta);
    } else {
      ++it_ta;
    }
  }

  // get confirmation from user
  QString prompt;
  if (selectedTags.size() == 1)
    prompt = i18n("<p>Do you really want to remove the tag <b>%1</b>?</p>", selectedTags.front().name());
  else
    prompt = i18n("Do you really want to remove all selected tags?");

  if (KMessageBox::questionYesNo(this, prompt, i18n("Remove Tag")) == KMessageBox::No)
    return;

  MyMoneyFileTransaction ft;
  try {
    // create a transaction filter that contains all tags selected for removal
    MyMoneyTransactionFilter f = MyMoneyTransactionFilter();
    for (const auto& tag : selectedTags) {
      f.addTag(tag.id());
    }
    // request a list of all transactions that still use the tags in question
    auto translist = file->transactionList(f);
//     qDebug() << "[KTagsView::slotDeleteTag]  " << translist.count() << " transaction still assigned to tags";

    // now get a list of all schedules that make use of one of the tags
    QList<MyMoneySchedule> used_schedules;
    for (const auto& schedule : file->scheduleList()) {
      // loop over all splits in the transaction of the schedule
      for (const auto& split : qAsConst(schedule.transaction().splits())) {
        for (auto i = 0; i < split.tagIdList().size(); ++i) {
          // is the tag in the split to be deleted?
          if (d->tagInList(selectedTags, split.tagIdList()[i])) {
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
      auto tag_id = dlg->show(d->m_selections.selection(SelectedObjects::Tag));
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
              if (d->tagInList(selectedTags, tagIdList[i])) {
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
              if (d->tagInList(selectedTags, tagIdList[i])) {
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
    for (const auto& tag : selectedTags) {
      file->removeTag(tag);
    }

    ft.commit();

  } catch (const MyMoneyException &e) {
    KMessageBox::detailedSorry(this, i18n("Unable to remove tag(s)"), QString::fromLatin1(e.what()));
  }
}
