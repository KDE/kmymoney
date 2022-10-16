/*
    SPDX-FileCopyrightText: 2012 Alessandro Russo <axela74@yahoo.it>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ktagsview.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QComboBox>
#include <QMenu>
#include <QTimer>
#include <QVector>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KConfigGroup>
#include <KHelpClient>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPageWidgetItem>
#include <KSharedConfig>

// ----------------------------------------------------------------------------
// Project Includes

#include "icons.h"
#include "itemrenameproxymodel.h"
#include "journalmodel.h"
#include "kmymoneymvccombo.h"
#include "kmymoneysettings.h"
#include "kmymoneyutils.h"
#include "kmymoneyviewbase_p.h"
#include "ktagreassigndlg.h"
#include "ledgertagfilter.h"
#include "ledgerviewsettings.h"
#include "mymoneyexception.h"
#include "mymoneymoney.h"
#include "mymoneyprice.h"
#include "mymoneyreport.h"
#include "mymoneyschedule.h"
#include "mymoneysecurity.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "specialdatesfilter.h"
#include "specialdatesmodel.h"
#include "tagsmodel.h"

#include "ui_ktagsview.h"

using namespace Icons;

namespace Ui {
class KTagsView;
}

class KTagsViewPrivate : public KMyMoneyViewBasePrivate
{
    Q_DECLARE_PUBLIC(KTagsView)

public:
    explicit KTagsViewPrivate(KTagsView* qq)
        : KMyMoneyViewBasePrivate(qq)
        , ui(new Ui::KTagsView)
        , m_transactionFilter(nullptr)
        , m_renameProxyModel(nullptr)
        , m_updateAction(nullptr)
        , m_needLoad(true)
    {
    }

    ~KTagsViewPrivate() override
    {
        if (!m_needLoad) {
            // remember the splitter settings for startup
            KConfigGroup grp = KSharedConfig::openConfig()->group("Last Use Settings");
            grp.writeEntry("KTagsViewSplitterSize", ui->m_splitter->saveState());
            grp.sync();
        }
        delete ui;
    }

    void init()
    {
        Q_Q(KTagsView);
        m_needLoad = false;
        ui->setupUi(q);

        m_updateAction = new QAction(Icons::get(Icon::DialogOK), i18nc("@action:button Update button in tags view", "Update"), q);
        q->connect(m_updateAction, &QAction::triggered, q, &KTagsView::slotUpdateTag);
        m_updateAction->setEnabled(false);

        ui->m_register->setSingleLineDetailRole(eMyMoney::Model::TransactionCounterAccountRole);
        ui->m_tagsList->setContextMenuPolicy(Qt::CustomContextMenu);

        ui->m_filterBox->addItem(i18nc("@item Show all tags", "All"), ItemRenameProxyModel::eAllItem);
        ui->m_filterBox->addItem(i18nc("@item Show only used tags", "Used"), ItemRenameProxyModel::eReferencedItems);
        ui->m_filterBox->addItem(i18nc("@item Show only unused tags", "Unused"), ItemRenameProxyModel::eUnReferencedItems);
        ui->m_filterBox->addItem(i18nc("@item Show only opened tags", "Opened"), ItemRenameProxyModel::eOpenedItems);
        ui->m_filterBox->addItem(i18nc("@item Show only closed tags", "Closed"), ItemRenameProxyModel::eClosedItems);
        ui->m_filterBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);

        ui->m_newButton->setDefaultAction(pActions[eMenu::Action::NewTag]);
        ui->m_renameButton->setDefaultAction(pActions[eMenu::Action::RenameTag]);
        ui->m_deleteButton->setDefaultAction(pActions[eMenu::Action::DeleteTag]);
        ui->m_updateButton->setDefaultAction(m_updateAction);

        // setup the model stack
        auto file = MyMoneyFile::instance();
        m_transactionFilter = new LedgerTagFilter(ui->m_register, QVector<QAbstractItemModel*>{file->specialDatesModel()});
        m_transactionFilter->setHideReconciledTransactions(LedgerViewSettings::instance()->hideReconciledTransactions());
        m_transactionFilter->setHideTransactionsBefore(LedgerViewSettings::instance()->hideTransactionsBefore());

        auto specialDatesFilter = new SpecialDatesFilter(q);
        specialDatesFilter->setSourceModel(m_transactionFilter);
        ui->m_register->setModel(specialDatesFilter);

        // keep track of changing balances
        q->connect(file->journalModel(), &JournalModel::balanceChanged, m_transactionFilter, &LedgerTagFilter::recalculateBalancesOnIdle);

        ui->m_balanceLabel->hide();

        m_renameProxyModel = new ItemRenameProxyModel(q);
        ui->m_tagsList->setModel(m_renameProxyModel);

        m_renameProxyModel->setReferenceFilter(ItemRenameProxyModel::eAllItem);
        m_renameProxyModel->setFilterKeyColumn(TagsModel::Column::Name);
        m_renameProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
        m_renameProxyModel->setRenameColumn(TagsModel::Column::Name);
        m_renameProxyModel->setSortRole(eMyMoney::Model::TagNameRole);
        m_renameProxyModel->setSortLocaleAware(true);
        m_renameProxyModel->sort(0);
        m_renameProxyModel->setDynamicSortFilter(true);

        m_renameProxyModel->setSourceModel(MyMoneyFile::instance()->tagsModel());

        ui->m_tagsList->setSelectionMode(QAbstractItemView::ExtendedSelection);
        q->connect(ui->m_tagsList->selectionModel(), &QItemSelectionModel::selectionChanged, q, &KTagsView::slotTagSelectionChanged);
        q->connect(ui->m_register->selectionModel(), &QItemSelectionModel::selectionChanged, q, &KTagsView::slotTransactionSelectionChanged);

        q->connect(m_renameProxyModel, &ItemRenameProxyModel::renameItem, q, &KTagsView::slotRenameSingleTag);
        q->connect(m_renameProxyModel, &ItemRenameProxyModel::dataChanged, q, &KTagsView::slotModelDataChanged);

        q->connect(ui->m_colorbutton, &KColorButton::changed, q, &KTagsView::slotTagDataChanged);
        q->connect(ui->m_closed, &QCheckBox::stateChanged, q, &KTagsView::slotTagDataChanged);
        q->connect(ui->m_notes, &QTextEdit::textChanged, q, &KTagsView::slotTagDataChanged);

        q->connect(ui->m_helpButton, &QAbstractButton::clicked, q, &KTagsView::slotHelp);

        // use the size settings of the last run (if any)
        auto grp = KSharedConfig::openConfig()->group("Last Use Settings");
        ui->m_splitter->restoreState(grp.readEntry("KTagsViewSplitterSize", QByteArray()));
        ui->m_splitter->setChildrenCollapsible(false);

        QVector<int> columns;
        columns = {
            JournalModel::Column::Number,
            JournalModel::Column::Security,
            JournalModel::Column::CostCenter,
            JournalModel::Column::Quantity,
            JournalModel::Column::Price,
            JournalModel::Column::Amount,
            JournalModel::Column::Value,
            JournalModel::Column::Balance,
        };
        ui->m_register->setColumnsHidden(columns);
        columns = {
            JournalModel::Column::Date,
            JournalModel::Column::Account,
            JournalModel::Column::Detail,
            JournalModel::Column::Reconciliation,
            JournalModel::Column::Payment,
            JournalModel::Column::Deposit,
        };
        ui->m_register->setColumnsShown(columns);

        // setup the searchline widget
        q->connect(ui->m_searchWidget, &QLineEdit::textChanged, m_renameProxyModel, &QSortFilterProxyModel::setFilterFixedString);
        ui->m_searchWidget->setClearButtonEnabled(true);
        ui->m_searchWidget->setPlaceholderText(i18nc("Placeholder text", "Search"));

        // At start we haven't any tag selected
        ui->m_tabWidget->setEnabled(false); // disable tab widget

        m_tag = MyMoneyTag(); // make sure we don't access an undefined tag
        clearItemData();

        m_focusWidget = ui->m_searchWidget;
    }

    void clearItemData()
    {
        ui->m_colorbutton->setColor(QColor());
        ui->m_closed->setChecked(false);
        ui->m_notes->setText(QString());
        showTransactions();
    }

    void showTransactions()
    {
        MyMoneyMoney balance;
        auto file = MyMoneyFile::instance();
        MyMoneySecurity base = file->baseCurrency();

        const auto tagIds = m_selections.selection(SelectedObjects::Tag);

        if (tagIds.isEmpty()) {
            ui->m_balanceLabel->setText(i18n("Balance: %1", balance.formatMoney(file->baseCurrency().smallestAccountFraction())));
            return;
        }

        m_transactionFilter->setTagIdList(tagIds);

        MyMoneyMoney deposit, payment;
        bool balanceAccurate = true;
        QSet<QString> accountIds;

        const auto viewModel = ui->m_register->model();
        const auto rows = viewModel->rowCount();

        for (int row = 0; row < rows; ++row) {
            const auto idx = viewModel->index(row, 0);
            const auto baseIdx = file->journalModel()->mapToBaseSource(idx);
            if (baseIdx.model() == file->journalModel()) {
                const auto shares = baseIdx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>();
                const auto splitAccountId = baseIdx.data(eMyMoney::Model::SplitAccountIdRole).toString();
                accountIds.insert(splitAccountId);
                MyMoneyAccount acc = file->account(splitAccountId);

                // take care of foreign currencies
                MyMoneyMoney val = shares.abs();
                if (acc.currencyId() != base.id()) {
                    const auto price = file->price(acc.currencyId(), base.id());
                    // in case the price is valid, we use it. Otherwise, we keep
                    // a flag that tells us that the balance is somewhat inaccurate
                    if (price.isValid()) {
                        val *= price.rate(base.id());
                    } else {
                        balanceAccurate = false;
                    }
                }
                if (shares.isNegative()) {
                    payment += val;
                } else {
                    deposit += val;
                }
            }
        }

        balance = deposit - payment;
        ui->m_balanceLabel->setText(
            i18n("Balance: %1%2", balanceAccurate ? QString() : QStringLiteral("~ "), balance.formatMoney(file->baseCurrency().smallestAccountFraction())));
        // only make balance visible if all transactions cover a single account
        ui->m_balanceLabel->setVisible(accountIds.count() < 2);
    }

    void ensureTagVisible(const QString& id)
    {
        const auto baseIdx = MyMoneyFile::instance()->tagsModel()->indexById(id);
        if (baseIdx.isValid()) {
            const auto idx = MyMoneyFile::baseModel()->mapFromBaseSource(m_renameProxyModel, baseIdx);
            ui->m_tagsList->setCurrentIndex(idx);
            ui->m_tagsList->scrollTo(idx);
        }
    }

    void loadDetails()
    {
        ui->m_colorbutton->setEnabled(true);
        ui->m_colorbutton->setColor(m_tag.tagColor());
        ui->m_closed->setEnabled(true);
        ui->m_closed->setChecked(m_tag.isClosed());
        ui->m_notes->setEnabled(true);
        ui->m_notes->setText(m_tag.notes());
    }

    void finalizePendingChanges()
    {
        Q_Q(KTagsView);
        // check if the content of a currently selected tag was modified
        // and ask to store the data
        if (m_havePendingChanges) {
            if (KMessageBox::questionTwoActions(q,
                                                QString("<qt>%1</qt>").arg(i18n("Do you want to save the changes for <b>%1</b>?", m_newName)),
                                                i18n("Save changes"),
                                                KMMYesNo::yes(),
                                                KMMYesNo::no())
                == KMessageBox::PrimaryAction) {
                q->slotUpdateTag();
            }
        }
    }

    /**
     * Check if a list contains a tag with a given id
     *
     * @param list const reference to value list
     * @param id const reference to id
     *
     * @retval true object has been found
     * @retval false object is not in list
     */
    bool tagInList(const QList<MyMoneyTag>& list, const QString& id) const
    {
        bool rc = false;
        QList<MyMoneyTag>::const_iterator it_p = list.begin();
        while (it_p != list.end()) {
            if ((*it_p).id() == id) {
                rc = true;
                break;
            }
            ++it_p;
        }
        return rc;
    }

    Ui::KTagsView* ui;
    LedgerTagFilter* m_transactionFilter;
    ItemRenameProxyModel* m_renameProxyModel;
    QAction* m_updateAction;

    MyMoneyTag m_tag;
    QString m_newName;

    /**
     * This member holds the load state of page
     */
    bool m_needLoad;
};

// *** KTagsView Implementation ***
KTagsView::KTagsView(QWidget *parent) :
    KMyMoneyViewBase(*new KTagsViewPrivate(this), parent)
{
    typedef void(KTagsView::*KTagsViewFunc)();
    const QHash<eMenu::Action, KTagsViewFunc> actionConnections {
        {eMenu::Action::NewTag,    &KTagsView::slotNewTag},
        {eMenu::Action::RenameTag, &KTagsView::slotRenameTag},
        {eMenu::Action::DeleteTag, &KTagsView::slotDeleteTag},
    };

    for (auto a = actionConnections.cbegin(); a != actionConnections.cend(); ++a)
        connect(pActions[a.key()], &QAction::triggered, this, a.value());
}

KTagsView::~KTagsView()
{
}

void KTagsView::slotRenameSingleTag(const QModelIndex& idx, const QVariant& value)
{
    Q_D(KTagsView);
    //if there is no current item selected, exit
    if (!idx.isValid())
        return;

    //qDebug() << "[KTagsView::slotRenameTag]";
    // create a copy of the new name without appended whitespaces
    const auto new_name = value.toString();
    // reload
    d->m_tag = MyMoneyFile::instance()->tagsModel()->itemById(idx.data(eMyMoney::Model::IdRole).toString());
    ;
    if (d->m_tag.name() != new_name) {
        MyMoneyFileTransaction ft;
        try {
            // check if we already have a tag with the new name
            const auto tag = MyMoneyFile::instance()->tagByName(new_name);
            // if the name already exists, ask the user whether he's sure to keep the name
            if (!tag.id().isEmpty()) {
                if (KMessageBox::questionTwoActions(this,
                                                    i18n("A tag with the name '%1' already exists. It is not advisable to have "
                                                         "multiple tags with the same identification name. Are you sure you would like "
                                                         "to rename the tag?",
                                                         new_name),
                                                    i18nc("@title:window", "Duplicate tag name"),
                                                    KMMYesNo::yes(),
                                                    KMMYesNo::no())
                    != KMessageBox::PrimaryAction) {
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
            KMessageBox::detailedError(this, i18n("Unable to modify tag"), QString::fromLatin1(e.what()));
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
    Q_UNUSED(deselected)

    Q_D(KTagsView);
    d->finalizePendingChanges();

    // loop over all tags and count the number of tags, also
    // obtain last selected tag

    for (const auto& idx : deselected.indexes()) {
        d->m_selections.removeSelection(SelectedObjects::Tag, idx.data(eMyMoney::Model::IdRole).toString());
    }
    for (const auto& idx : selected.indexes()) {
        d->m_selections.addSelection(SelectedObjects::Tag, idx.data(eMyMoney::Model::IdRole).toString());
    }

    if (d->m_selections.selection(SelectedObjects::Tag).isEmpty()) {
        d->m_tag = MyMoneyTag();
    } else {
        d->m_tag = MyMoneyFile::instance()->tagsModel()->itemById(d->m_selections.selection(SelectedObjects::Tag).at(0));
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

    Q_EMIT requestSelectionChange(d->m_selections);
}

void KTagsView::slotTransactionSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_UNUSED(deselected)

    Q_D(KTagsView);

    d->m_selections.clearSelections(SelectedObjects::JournalEntry);
    if (!selected.indexes().isEmpty()) {
        d->m_selections.addSelection(SelectedObjects::JournalEntry, selected.indexes().first().data(eMyMoney::Model::IdRole).toString());
    }

    Q_EMIT requestSelectionChange(d->m_selections);
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
            KMessageBox::detailedError(this, i18n("Unable to modify tag"), QString::fromLatin1(e.what()));
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
            Q_EMIT requestCustomContextMenu(eMenu::Menu::Tag, d->ui->m_tagsList->mapToGlobal(pos));
        });

        connect(d->ui->m_register, &QWidget::customContextMenuRequested, this, [&](QPoint pos) {
            Q_D(KTagsView);
            Q_EMIT requestCustomContextMenu(eMenu::Menu::Transaction, d->ui->m_register->mapToGlobal(pos));
        });

        connect(LedgerViewSettings::instance(), &LedgerViewSettings::settingsChanged, this, [&]() {
            d->m_transactionFilter->setHideReconciledTransactions(LedgerViewSettings::instance()->hideReconciledTransactions());
            d->m_transactionFilter->setHideTransactionsBefore(LedgerViewSettings::instance()->hideTransactionsBefore());
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

void KTagsView::slotSelectTag(const QString& tagId)
{
    Q_D(KTagsView);
    if (!isVisible())
        return;

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
    slotSelectTag(id);
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

    MyMoneyFileTransaction ft;
    try {
        // create a transaction filter that contains all tags selected for removal
        MyMoneyTransactionFilter f = MyMoneyTransactionFilter();
        for (const auto& tag : selectedTags) {
            f.addTag(tag.id());
        }
        // request a list of all transactions that still use the tags in question
        QList<MyMoneyTransaction> translist;
        file->transactionList(translist, f);
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

        QList<MyMoneyReport> used_reports;
        for (const auto& report : file->reportList()) {
            QStringList tagList(report.tags());
            for (const auto& tag : tagList) {
                if (d->tagInList(selectedTags, tag)) {
                    used_reports.push_back(report);
                    break;
                }
            }
        }

//     qDebug() << "[KTagsView::slotDeleteTag]  " << used_schedules.count() << " schedules use one of the selected tags";

        MyMoneyTag newTag;
        // if at least one tag is still referenced, we need to reassign its transactions first
        if (!translist.isEmpty() || !used_schedules.isEmpty() || !used_reports.isEmpty()) {
            // show transaction reassignment dialog
            QPointer<KTagReassignDlg> dlg = new KTagReassignDlg(this);
            KMyMoneyMVCCombo::setSubstringSearchForChildren(dlg, !KMyMoneySettings::stringMatchFromStart());
            dlg->setupFilter(d->m_selections.selection(SelectedObjects::Tag));
            if ((dlg->exec() == QDialog::Rejected) || !dlg) {
                delete dlg;
                return; // the user aborted the dialog, so let's abort as well
            }
            auto newTagId = dlg->reassignTo();
            delete dlg; // and kill the dialog

            if (!newTagId.isEmpty()) {
                newTag = file->tag(newTagId);
            }

            // check if we have a report that explicitly uses one of our tags
            // and remove/replace it
            try {
                // now loop over all report and reassign tag
                for (auto report : used_reports) {
                    QStringList tagIdList(report.tags());
                    for (const auto& tagId : tagIdList) {
                        if (d->tagInList(selectedTags, tagId)) {
                            report.removeReference(tagId);
                            if (!newTagId.isEmpty()) {
                                report.addTag(newTagId);
                            }
                        }
                    }
                    file->modifyReport(report); // modify the transaction in the MyMoney object
                }

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
                                if (!newTagId.isEmpty()) {
                                    if (tagIdList.indexOf(newTagId) == -1) {
                                        tagIdList.append(newTagId);
                                    }
                                }
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
                                if (!newTagId.isEmpty()) {
                                    if (tagIdList.indexOf(newTagId) == -1) {
                                        tagIdList.append(newTagId);
                                    }
                                }
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
                KMessageBox::detailedError(this, i18n("Unable to reassign tag of transaction/split"), QString::fromLatin1(e.what()));
            }
        } // if !translist.isEmpty()

        // now loop over all selected tags and remove them
        for (const auto& tag : selectedTags) {
            file->removeTag(tag);
        }

        ft.commit();

    } catch (const MyMoneyException &e) {
        KMessageBox::detailedError(this, i18n("Unable to remove tag(s)"), QString::fromLatin1(e.what()));
    }
}
