/*
    SPDX-FileCopyrightText: 2012 Alessandro Russo <axela74@yahoo.it>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2020 Thomas Baugart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTAGSVIEW_P_H
#define KTAGSVIEW_P_H

#include "ktagsview.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>
#include <QAction>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KSharedConfig>
#include <KConfigGroup>
#include <KMessageBox>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_ktagsview.h"

#include "kmymoneyviewbase_p.h"
#include "mymoneyaccount.h"
#include "mymoneyfile.h"
#include "mymoneytag.h"
#include "mymoneytransactionfilter.h"
#include "icons.h"
#include "viewenums.h"
#include "widgetenums.h"
#include "itemrenameproxymodel.h"
#include "tagsmodel.h"
#include "journalmodel.h"
#include "ledgertagfilter.h"
#include "specialdatesmodel.h"
#include "specialdatesfilter.h"
#include "menuenums.h"

using namespace Icons;
namespace Ui {
class KTagsView;
}

class KTagsViewPrivate : public KMyMoneyViewBasePrivate
{
    Q_DECLARE_PUBLIC(KTagsView)

public:

    explicit KTagsViewPrivate(KTagsView *qq)
        : KMyMoneyViewBasePrivate(qq)
        , ui(new Ui::KTagsView)
        , m_transactionFilter(nullptr)
        , m_renameProxyModel(nullptr)
        , m_updateAction(nullptr)
        , m_needLoad(true)
        , m_allowEditing(true)
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

        m_updateAction = new QAction(Icons::get(Icon::DialogOK), i18nc("@action:button Update button in tags vew", "Update"), q);
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
        m_transactionFilter = new LedgerTagFilter(ui->m_register, QVector<QAbstractItemModel*> { file->specialDatesModel() });
        auto specialDatesFilter = new SpecialDatesFilter(file->specialDatesModel(), q);
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

        q->connect(ui->m_tagsList->selectionModel(), &QItemSelectionModel::selectionChanged, q, &KTagsView::slotTagSelectionChanged);

        q->connect(m_renameProxyModel, &ItemRenameProxyModel::renameItem, q, &KTagsView::slotRenameSingleTag);
        q->connect(m_renameProxyModel, &ItemRenameProxyModel::dataChanged, q, &KTagsView::slotModelDataChanged);

        q->connect(ui->m_colorbutton, &KColorButton::changed,   q, &KTagsView::slotTagDataChanged);
        q->connect(ui->m_closed,      &QCheckBox::stateChanged, q, &KTagsView::slotTagDataChanged);
        q->connect(ui->m_notes,       &QTextEdit::textChanged,  q, &KTagsView::slotTagDataChanged);

        q->connect(ui->m_helpButton, &QAbstractButton::clicked,   q, &KTagsView::slotHelp);

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

        if (tagIds.isEmpty() || !ui->m_tabWidget->isEnabled()) {
            ui->m_balanceLabel->setText(i18n("Balance: %1", balance.formatMoney(file->baseCurrency().smallestAccountFraction())));
            return;
        }

        m_transactionFilter->setTagIdList(tagIds);

        MyMoneyMoney deposit, payment;
        bool balanceAccurate = true;
        QSet<QString> accountIds;
        /// @todo port to new model code
#if 0
        Q_Q(KTagsView);
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
        QSet<QString> accountIds;
        for (it = d->m_transactionList.constBegin(); it != d->m_transactionList.constEnd(); ++it) {
            const MyMoneySplit& split = (*it).second;
            accountIds.insert(split.accountId());
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


        // add the group markers
        ui->m_register->addGroupMarkers();

        // sort the transactions according to the sort setting
        ui->m_register->sortItems();

        // remove trailing and adjacent markers
        ui->m_register->removeUnwantedGroupMarkers();

        ui->m_register->updateRegister(true);

        // we might end up here with updates disabled on the register so
        // make sure that we enable updates here
        ui->m_register->setUpdatesEnabled(true);
#endif
        balance = deposit - payment;
        ui->m_balanceLabel->setText(i18n("Balance: %1%2",
                                         balanceAccurate ? "" : "~",
                                         balance.formatMoney(file->baseCurrency().smallestAccountFraction())));
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
            if (KMessageBox::questionYesNo(q, QString("<qt>%1</qt>").arg(
                                               i18n("Do you want to save the changes for <b>%1</b>?", m_newName)),
                                           i18n("Save changes")) == KMessageBox::Yes) {
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

    Ui::KTagsView*                ui;
    LedgerTagFilter*              m_transactionFilter;
    ItemRenameProxyModel*         m_renameProxyModel;
    QAction*                      m_updateAction;

    MyMoneyTag                    m_tag;
    QString                       m_newName;

    /**
      * This member holds the load state of page
      */
    bool                          m_needLoad;

    /**
      * This signals whether a tag can be edited
      **/
    bool                          m_allowEditing;
};


#endif
