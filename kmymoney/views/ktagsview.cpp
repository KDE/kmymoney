/***************************************************************************
                          ktagsview.h
                          -------------
    begin                : Sat Oct 13 2012
    copyright            : (C) 2012 by Alessandro Russo <axela74@yahoo.it>
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

// ----------------------------------------------------------------------------
// QT Includes

#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QTabWidget>
#include <QCheckBox>
#include <QSplitter>
#include <QMap>
#include <QList>
#include <QTimer>
#include <QIcon>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KMessageBox>
#include <KGuiItem>
#include <KHelpClient>
#include <KSharedConfig>
#include <KActionCollection>
#include <KListWidgetSearchLine>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "mymoneyprice.h"
#include "mymoneytransactionfilter.h"
#include "kmymoneyglobalsettings.h"
#include "kmymoney.h"
#include "models.h"
#include "accountsmodel.h"
#include "mymoneysecurity.h"
#include "icons.h"

using namespace Icons;

/* -------------------------------------------------------------------------*/
/*                         KTransactionPtrVector                            */
/* -------------------------------------------------------------------------*/

// *** KTagListItem Implementation ***

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

// *** KTagsView Implementation ***

KTagsView::KTagsView(QWidget *parent) :
    QWidget(parent),
    m_needReload(false),
    m_needLoad(true),
    m_inSelection(false),
    m_allowEditing(true),
    m_tagFilterType(0)
{
}

KTagsView::~KTagsView()
{
  if (!m_needLoad) {
    // remember the splitter settings for startup
    KConfigGroup grp = KSharedConfig::openConfig()->group("Last Use Settings");
    grp.writeEntry("KTagsViewSplitterSize", m_splitter->saveState());
    grp.sync();
  }
}

void KTagsView::setDefaultFocus()
{
  QTimer::singleShot(0, m_searchWidget, SLOT(setFocus()));
}

void KTagsView::init()
{
  m_needLoad = false;
  setupUi(this);

  m_filterProxyModel = new AccountNamesFilterProxyModel(this);
  m_filterProxyModel->addAccountGroup(QVector<eMyMoney::Account> {eMyMoney::Account::Asset, eMyMoney::Account::Liability, eMyMoney::Account::Income, eMyMoney::Account::Expense});
  auto const model = Models::instance()->accountsModel();
  m_filterProxyModel->setSourceModel(model);
  m_filterProxyModel->setSourceColumns(model->getColumns());
  m_filterProxyModel->sort((int)eAccountsModel::Column::Account);

  // create the searchline widget
  // and insert it into the existing layout
  m_searchWidget = new KListWidgetSearchLine(this, m_tagsList);
  m_searchWidget->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
  m_tagsList->setContextMenuPolicy(Qt::CustomContextMenu);
  m_listTopHLayout->insertWidget(0, m_searchWidget);

  //load the filter type
  m_filterBox->addItem(i18nc("@item Show all tags", "All"));
  m_filterBox->addItem(i18nc("@item Show only used tags", "Used"));
  m_filterBox->addItem(i18nc("@item Show only unused tags", "Unused"));
  m_filterBox->addItem(i18nc("@item Show only opened tags", "Opened"));
  m_filterBox->addItem(i18nc("@item Show only closed tags", "Closed"));
  m_filterBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);

  KGuiItem newButtonItem(QString(),
                         QIcon::fromTheme(g_Icons[Icon::ListAddTag]),
                         i18n("Creates a new tag"),
                         i18n("Use this to create a new tag."));
  KGuiItem::assign(m_newButton, newButtonItem);
  m_newButton->setToolTip(newButtonItem.toolTip());

  KGuiItem renameButtonItem(QString(),
                            QIcon::fromTheme(g_Icons[Icon::EditRename]),
                            i18n("Rename the current selected tag"),
                            i18n("Use this to start renaming the selected tag."));
  KGuiItem::assign(m_renameButton, renameButtonItem);
  m_renameButton->setToolTip(renameButtonItem.toolTip());

  KGuiItem deleteButtonItem(QString(),
                            QIcon::fromTheme(g_Icons[Icon::ListRemoveTag]),
                            i18n("Delete the current selected tag"),
                            i18n("Use this to delete the selected tag."));
  KGuiItem::assign(m_deleteButton, deleteButtonItem);
  m_deleteButton->setToolTip(deleteButtonItem.toolTip());

  KGuiItem updateButtonItem(i18nc("Update tag", "Update"),
                            QIcon::fromTheme(g_Icons[Icon::DialogOK]),
                            i18n("Accepts the entered data and stores it"),
                            i18n("Use this to accept the modified data."));
  KGuiItem::assign(m_updateButton, updateButtonItem);

  m_updateButton->setEnabled(false);

  QList<KMyMoneyRegister::Column> cols;
  cols << KMyMoneyRegister::DateColumn;
  cols << KMyMoneyRegister::AccountColumn;
  cols << KMyMoneyRegister::DetailColumn;
  cols << KMyMoneyRegister::ReconcileFlagColumn;
  cols << KMyMoneyRegister::PaymentColumn;
  cols << KMyMoneyRegister::DepositColumn;
  m_register->setupRegister(MyMoneyAccount(), cols);
  m_register->setSelectionMode(QTableWidget::SingleSelection);
  m_register->setDetailsColumnType(KMyMoneyRegister::AccountFirst);
  m_balanceLabel->hide();

  connect(m_tagsList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(slotSelectTag(QListWidgetItem*,QListWidgetItem*)));
  connect(m_tagsList, SIGNAL(itemSelectionChanged()), this, SLOT(slotSelectTag()));
  connect(m_tagsList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(slotStartRename(QListWidgetItem*)));
  connect(m_tagsList, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(slotRenameTag(QListWidgetItem*)));
  connect(m_tagsList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotOpenContextMenu(QPoint)));

  connect(m_renameButton, SIGNAL(clicked()), this, SLOT(slotRenameButtonCliked()));
  connect(m_deleteButton, SIGNAL(clicked()), kmymoney->actionCollection()->action(kmymoney->s_Actions[Action::TagDelete]), SLOT(trigger()));
  connect(m_newButton, SIGNAL(clicked()), this, SLOT(slotTagNew()));

  connect(m_colorbutton, SIGNAL(changed(QColor)), this, SLOT(slotTagDataChanged()));
  connect(m_closed, SIGNAL(stateChanged(int)), this, SLOT(slotTagDataChanged()));
  connect(m_notes, SIGNAL(textChanged()), this, SLOT(slotTagDataChanged()));

  connect(m_updateButton, SIGNAL(clicked()), this, SLOT(slotUpdateTag()));
  connect(m_helpButton, SIGNAL(clicked()), this, SLOT(slotHelp()));

  connect(m_register, SIGNAL(editTransaction()), this, SLOT(slotSelectTransaction()));

  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadTags()));

  connect(m_filterBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotChangeFilter(int)));

  // use the size settings of the last run (if any)
  KConfigGroup grp = KSharedConfig::openConfig()->group("Last Use Settings");
  m_splitter->restoreState(grp.readEntry("KTagsViewSplitterSize", QByteArray()));
  m_splitter->setChildrenCollapsible(false);

  // At start we haven't any tag selected
  m_tabWidget->setEnabled(false); // disable tab widget
  m_deleteButton->setEnabled(false); // disable delete and rename button
  m_renameButton->setEnabled(false);
  m_tag = MyMoneyTag(); // make sure we don't access an undefined tag
  clearItemData();
}

void KTagsView::slotStartRename(QListWidgetItem* item)
{
  m_allowEditing = true;
  m_tagsList->editItem(item);
}

void KTagsView::slotRenameButtonCliked()
{
  if (m_tagsList->currentItem() && m_tagsList->selectedItems().count() == 1) {
    slotStartRename(m_tagsList->currentItem());
  }
}

// This variant is only called when a single tag is selected and renamed.
void KTagsView::slotRenameTag(QListWidgetItem* ta)
{
  //if there is no current item selected, exit
  if (m_allowEditing == false || !m_tagsList->currentItem() || ta != m_tagsList->currentItem())
    return;

  //qDebug() << "[KTagsView::slotRenameTag]";
  // create a copy of the new name without appended whitespaces
  QString new_name = ta->text();
  if (m_tag.name() != new_name) {
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
          ta->setText(m_tag.name());
          return;
        }
      } catch (const MyMoneyException &) {
        // all ok, the name is unique
      }

      m_tag.setName(new_name);
      m_newName = new_name;
      MyMoneyFile::instance()->modifyTag(m_tag);

      // the above call to modifyTag will reload the view so
      // all references and pointers to the view have to be
      // re-established.

      // make sure, that the record is visible even if it moved
      // out of sight due to the rename operation
      ensureTagVisible(m_tag.id());

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
  for (int i = 0; i < m_tagsList->count(); ++i) {
    KTagListItem* ta = dynamic_cast<KTagListItem*>(m_tagsList->item(0));
    if (ta && ta->tag().id() == id) {
      m_tagsList->scrollToItem(ta, QAbstractItemView::PositionAtCenter);

      m_tagsList->setCurrentItem(ta);      // active item and deselect all others
      m_tagsList->setCurrentRow(i, QItemSelectionModel::ClearAndSelect);   // and select it
      break;
    }
  }
}

void KTagsView::selectedTags(QList<MyMoneyTag>& tagsList) const
{
  QList<QListWidgetItem *> selectedItems = m_tagsList->selectedItems();
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
  Q_UNUSED(cur);
  Q_UNUSED(prev);

  m_allowEditing = false;
}

void KTagsView::slotSelectTag()
{
  // check if the content of a currently selected tag was modified
  // and ask to store the data
  if (m_updateButton->isEnabled()) {
    if (KMessageBox::questionYesNo(this, QString("<qt>%1</qt>").arg(
                                     i18n("Do you want to save the changes for <b>%1</b>?", m_newName)),
                                   i18n("Save changes")) == KMessageBox::Yes) {
      m_inSelection = true;
      slotUpdateTag();
      m_inSelection = false;
    }
  }
  // loop over all tags and count the number of tags, also
  // obtain last selected tag
  QList<MyMoneyTag> tagsList;
  selectedTags(tagsList);

  emit selectObjects(tagsList);

  if (tagsList.isEmpty()) {
    m_tabWidget->setEnabled(false); // disable tab widget
    m_balanceLabel->hide();
    m_deleteButton->setEnabled(false); //disable delete and rename button
    m_renameButton->setEnabled(false);
    clearItemData();
    m_tag = MyMoneyTag();
    return; // make sure we don't access an undefined tag
  }

  m_deleteButton->setEnabled(true); //re-enable delete button

  // if we have multiple tags selected, clear and disable the tag information
  if (tagsList.count() > 1) {
    m_tabWidget->setEnabled(false); // disable tab widget
    m_renameButton->setEnabled(false); // disable also the rename button
    m_balanceLabel->hide();
    clearItemData();
  } else m_renameButton->setEnabled(true);

  // otherwise we have just one selected, enable tag information widget and renameButton
  m_tabWidget->setEnabled(true);
  m_balanceLabel->show();

  // as of now we are updating only the last selected tag, and until
  // selection mode of the QListView has been changed to Extended, this
  // will also be the only selection and behave exactly as before - Andreas
  try {
    m_tag = tagsList[0];

    m_newName = m_tag.name();
    m_colorbutton->setEnabled(true);
    m_colorbutton->setColor(m_tag.tagColor());
    m_closed->setEnabled(true);
    m_closed->setChecked(m_tag.isClosed());
    m_notes->setEnabled(true);
    m_notes->setText(m_tag.notes());
    slotTagDataChanged();

    showTransactions();

  } catch (const MyMoneyException &e) {
    qDebug("exception during display of tag: %s at %s:%ld", qPrintable(e.what()), qPrintable(e.file()), e.line());
    m_register->clear();
    m_tag = MyMoneyTag();
  }
  m_allowEditing = true;
}

void KTagsView::clearItemData()
{
  m_colorbutton->setColor(QColor());
  m_closed->setChecked(false);
  m_notes->setText(QString());
  showTransactions();
}

void KTagsView::showTransactions()
{
  MyMoneyMoney balance;
  MyMoneyFile *file = MyMoneyFile::instance();
  MyMoneySecurity base = file->baseCurrency();

  // setup sort order
  m_register->setSortOrder(KMyMoneyGlobalSettings::sortSearchView());

  // clear the register
  m_register->clear();

  if (m_tag.id().isEmpty() || !m_tabWidget->isEnabled()) {
    m_balanceLabel->setText(i18n("Balance: %1", balance.formatMoney(file->baseCurrency().smallestAccountFraction())));
    return;
  }

  // setup the list and the pointer vector
  MyMoneyTransactionFilter filter;
  filter.addTag(m_tag.id());
  filter.setDateFilter(KMyMoneyGlobalSettings::startDate().date(), QDate());

  // retrieve the list from the engine
  file->transactionList(m_transactionList, filter);

  // create the elements for the register
  QList<QPair<MyMoneyTransaction, MyMoneySplit> >::const_iterator it;
  QMap<QString, int> uniqueMap;
  MyMoneyMoney deposit, payment;

  int splitCount = 0;
  bool balanceAccurate = true;
  for (it = m_transactionList.constBegin(); it != m_transactionList.constEnd(); ++it) {
    const MyMoneySplit& split = (*it).second;
    MyMoneyAccount acc = file->account(split.accountId());
    ++splitCount;
    uniqueMap[(*it).first.id()]++;

    KMyMoneyRegister::Register::transactionFactory(m_register, (*it).first, (*it).second, uniqueMap[(*it).first.id()]);

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
  m_register->addGroupMarkers();

  // sort the transactions according to the sort setting
  m_register->sortItems();

  // remove trailing and adjacent markers
  m_register->removeUnwantedGroupMarkers();

  m_register->updateRegister(true);

  // we might end up here with updates disabled on the register so
  // make sure that we enable updates here
  m_register->setUpdatesEnabled(true);
  m_balanceLabel->setText(i18n("Balance: %1%2",
                               balanceAccurate ? "" : "~",
                               balance.formatMoney(file->baseCurrency().smallestAccountFraction())));
}

void KTagsView::slotTagDataChanged()
{
  bool rc = false;

  if (m_tabWidget->isEnabled()) {
    rc |= ((m_tag.tagColor().isValid() != m_colorbutton->color().isValid())
           || (m_colorbutton->color().isValid() && m_tag.tagColor() != m_colorbutton->color()));
    rc |= (m_closed->isChecked() != m_tag.isClosed());
    rc |= ((m_tag.notes().isEmpty() != m_notes->toPlainText().isEmpty())
           || (!m_notes->toPlainText().isEmpty() && m_tag.notes() != m_notes->toPlainText()));
  }
  m_updateButton->setEnabled(rc);
}

void KTagsView::slotUpdateTag()
{
  if (m_updateButton->isEnabled()) {
    MyMoneyFileTransaction ft;
    m_updateButton->setEnabled(false);
    try {
      m_tag.setName(m_newName);
      m_tag.setTagColor(m_colorbutton->color());
      m_tag.setClosed(m_closed->isChecked());
      m_tag.setNotes(m_notes->toPlainText());

      MyMoneyFile::instance()->modifyTag(m_tag);
      ft.commit();

    } catch (const MyMoneyException &e) {
      KMessageBox::detailedSorry(0, i18n("Unable to modify tag"),
                                 i18n("%1 thrown in %2:%3", e.what(), e.file(), e.line()));
    }
  }
}

void KTagsView::showEvent(QShowEvent* event)
{
  if (m_needLoad)
    init();

  emit aboutToShow();

  if (m_needReload) {
    loadTags();
    m_needReload = false;
  }

  // don't forget base class implementation
  QWidget::showEvent(event);

  QList<MyMoneyTag> list;
  selectedTags(list);
  emit selectObjects(list);
}

void KTagsView::slotLoadTags()
{
  if (isVisible()) {
    if (m_inSelection)
      QTimer::singleShot(0, this, SLOT(slotLoadTags()));
    else
      loadTags();
  } else {
    m_needReload = true;
  }
}

void KTagsView::loadTags()
{
  if (m_inSelection)
    return;

  QMap<QString, bool> isSelected;
  QString id;
  MyMoneyFile* file = MyMoneyFile::instance();

  // remember which items are selected in the list
  QList<QListWidgetItem *> selectedItems = m_tagsList->selectedItems();
  QList<QListWidgetItem *>::const_iterator tagsIt = selectedItems.constBegin();

  while (tagsIt != selectedItems.constEnd()) {
    KTagListItem* item = dynamic_cast<KTagListItem*>(*tagsIt);
    if (item)
      isSelected[item->tag().id()] = true;
    ++tagsIt;
  }

  // keep current selected item
  KTagListItem *currentItem = static_cast<KTagListItem *>(m_tagsList->currentItem());
  if (currentItem)
    id = currentItem->tag().id();

  m_allowEditing = false;
  // clear the list
  m_searchWidget->clear();
  m_searchWidget->updateSearch();
  m_tagsList->clear();
  m_register->clear();
  currentItem = 0;

  QList<MyMoneyTag>list = file->tagList();
  QList<MyMoneyTag>::ConstIterator it;

  for (it = list.constBegin(); it != list.constEnd(); ++it) {
    if (m_tagFilterType == eAllTags ||
        (m_tagFilterType == eReferencedTags && file->isReferenced(*it)) ||
        (m_tagFilterType == eUnusedTags && !file->isReferenced(*it)) ||
        (m_tagFilterType == eOpenedTags && !(*it).isClosed()) ||
        (m_tagFilterType == eClosedTags && (*it).isClosed())) {
      KTagListItem* item = new KTagListItem(m_tagsList, *it);
      if (item->tag().id() == id)
        currentItem = item;
      if (isSelected[item->tag().id()])
        item->setSelected(true);
    }
  }
  m_tagsList->sortItems();

  if (currentItem) {
    m_tagsList->setCurrentItem(currentItem);
    m_tagsList->scrollToItem(currentItem);
  }

  m_filterProxyModel->invalidate();

  slotSelectTag(0, 0);
  m_allowEditing = true;
}

void KTagsView::slotSelectTransaction()
{
  QList<KMyMoneyRegister::RegisterItem*> list = m_register->selectedItems();
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

  try {
    // clear filter
    m_searchWidget->clear();
    m_searchWidget->updateSearch();

    // deselect all other selected items
    QList<QListWidgetItem *> selectedItems = m_tagsList->selectedItems();
    QList<QListWidgetItem *>::const_iterator tagsIt = selectedItems.constBegin();
    while (tagsIt != selectedItems.constEnd()) {
      KTagListItem* item = dynamic_cast<KTagListItem*>(*tagsIt);
      if (item)
        item->setSelected(false);
      ++tagsIt;
    }

    // find the tag in the list
    QListWidgetItem* it;
    for (int i = 0; i < m_tagsList->count(); ++i) {
      it = m_tagsList->item(i);
      KTagListItem* item = dynamic_cast<KTagListItem *>(it);
      if (item && item->tag().id() == tagId) {
        m_tagsList->scrollToItem(it, QAbstractItemView::PositionAtCenter);

        m_tagsList->setCurrentItem(it);     // active item and deselect all others
        m_tagsList->setCurrentRow(i, QItemSelectionModel::ClearAndSelect); // and select it

        //make sure the tag selection is updated and transactions are updated accordingly
        slotSelectTag();

        KMyMoneyRegister::RegisterItem *item = 0;
        for (int i = 0; i < m_register->rowCount(); ++i) {
          item = m_register->itemAtRow(i);
          KMyMoneyRegister::Transaction* t = dynamic_cast<KMyMoneyRegister::Transaction*>(item);
          if (t) {
            if (t->transaction().id() == transactionId && t->transaction().accountReferenced(accountId)) {
              m_register->selectItem(item);
              m_register->ensureItemVisible(item);
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

void KTagsView::slotOpenContextMenu(const QPoint& /*ta*/)
{
  KTagListItem* item = dynamic_cast<KTagListItem*>(m_tagsList->currentItem());
  if (item) {
    slotSelectTag();
    emit openContextMenu(item->tag());
  }
}

void KTagsView::slotTagNew()
{
  kmymoney->actionCollection()->action(kmymoney->s_Actions[Action::TagNew])->trigger();
}

void KTagsView::slotHelp()
{
  KHelpClient::invokeHelp("details.tags.attributes");
  //FIXME-ALEX update help file
}

void KTagsView::slotChangeFilter(int index)
{
  //update the filter type then reload the tags list
  m_tagFilterType = index;
  loadTags();
}
