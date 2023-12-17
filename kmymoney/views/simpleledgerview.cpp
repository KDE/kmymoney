/*
    SPDX-FileCopyrightText: 2015-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "simpleledgerview.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QAction>
#include <QDesktopServices>
#include <QHeaderView>
#include <QKeyEvent>
#include <QLineEdit>
#include <QTabBar>
#include <QTimer>
#include <QToolButton>
#include <QTreeView>
#include <QUrl>
#include <QUuid>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>
#include <KPageWidgetItem>

// ----------------------------------------------------------------------------
// Project Includes

#include "accountsmodel.h"
#include "icons.h"
#include "institutionsmodel.h"
#include "kmymoneyaccountcombo.h"
#include "kmymoneysettings.h"
#include "kmymoneyviewbase_p.h"
#include "ledgerviewpage.h"
#include "menuenums.h"
#include "mymoneyaccount.h"
#include "mymoneyenums.h"
#include "mymoneyfile.h"
#include "mymoneyinstitution.h"
#include "reconciliationledgerviewpage.h"
#include "selectedobjects.h"
#include <ui_simpleledgerview.h>

using namespace Icons;

class SimpleLedgerViewPrivate : public KMyMoneyViewBasePrivate
{
    Q_DECLARE_PUBLIC(SimpleLedgerView)

public:
    explicit SimpleLedgerViewPrivate(SimpleLedgerView* qq)
        : KMyMoneyViewBasePrivate(qq)
        , ui(new Ui_SimpleLedgerView)
        , accountsModel(nullptr)
        , newTabWidget(nullptr)
        , webSiteButton(nullptr)
        , accountCombo(nullptr)
        , lastIdx(-1)
        , inModelUpdate(false)
        , m_needInit(true)
    {}

    ~SimpleLedgerViewPrivate()
    {
        delete ui;
    }

    void removeCloseButton(int idx, const QString& tooltip = QString())
    {
        Q_Q(SimpleLedgerView);
        // remove close button from new page
        QTabBar* bar = ui->ledgerTab->findChild<QTabBar*>();
        if (bar) {
            QTabBar::ButtonPosition closeSide =
                (QTabBar::ButtonPosition)q->style()->styleHint(QStyle::SH_TabBar_CloseButtonPosition, 0, ui->ledgerTab->widget(idx));
            QWidget* w = bar->tabButton(idx, closeSide);
            bar->setTabButton(idx, closeSide, 0);
            bar->setTabToolTip(idx, tooltip);
            w->deleteLater();
        }
    }

    void init()
    {
        Q_Q(SimpleLedgerView);
        m_needInit = false;
        ui->setupUi(q);
        ui->ledgerTab->setTabIcon(0, Icons::get(Icon::ListAdd));
        ui->ledgerTab->setTabText(0, QString());
        newTabWidget = ui->ledgerTab->widget(0);

        accountsModel = new AccountNamesFilterProxyModel(q);
        q->slotSettingsChanged();

        // remove close button from new page
        QTabBar* bar = ui->ledgerTab->findChild<QTabBar*>();
        if(bar) {
            q->connect(bar, &QTabBar::tabMoved, q, &SimpleLedgerView::checkTabOrder);
        }
        removeCloseButton(0);

        webSiteButton = new QToolButton;
        ui->ledgerTab->setCornerWidget(webSiteButton);
        q->connect(webSiteButton, &QToolButton::pressed, q,
        [=] {
            QDesktopServices::openUrl(webSiteUrl);
        });

        q->connect(ui->ledgerTab, &QTabWidget::currentChanged, q, &SimpleLedgerView::tabSelected);
        q->connect(ui->ledgerTab, &QTabWidget::tabBarClicked, q, &SimpleLedgerView::tabClicked);
        q->connect(ui->ledgerTab, &QTabWidget::tabCloseRequested, q, &SimpleLedgerView::closeLedger);

        // we reload the icon if the institution data changed
        q->connect(MyMoneyFile::instance()->institutionsModel(), &InstitutionsModel::dataChanged, q, &SimpleLedgerView::setupCornerWidget);

        accountsModel->addAccountGroup(QVector<eMyMoney::Account::Type> {eMyMoney::Account::Type::Asset, eMyMoney::Account::Type::Liability, eMyMoney::Account::Type::Equity});

        accountsModel->setHideEquityAccounts(!KMyMoneySettings::expertMode());
        accountsModel->setHideZeroBalancedEquityAccounts(KMyMoneySettings::hideZeroBalanceEquities());
        auto const model = MyMoneyFile::instance()->accountsModel();
        accountsModel->setSourceModel(model);
        accountsModel->sort(AccountsModel::Column::AccountName);

        accountCombo = new KMyMoneyAccountCombo(accountsModel, ui->ledgerTab);
        accountCombo->setEditable(true);
        accountCombo->setSplitActionVisible(false);
        accountCombo->hide();
        q->connect(accountCombo, &KMyMoneyAccountCombo::accountSelected, q, &SimpleLedgerView::openLedger);

        accountCombo->installEventFilter(q);
        accountCombo->popup()->installEventFilter(q);
        updateTitlePage();

        q->tabSelected(0);
        openLedgersAfterFileOpen();
    }

    void openLedgersAfterFileOpen()
    {
        if (m_needInit)
            return;

        Q_Q(SimpleLedgerView);
        // get storage id without the enclosing braces
        const auto storageId = MyMoneyFile::instance()->storageId().toString(QUuid::WithoutBraces);
        KSharedConfigPtr config = KSharedConfig::openConfig();
        KConfigGroup grp = config->group("OpenLedgers");

        // in case we have a previous setting, we open them
        const auto openLedgers = grp.readEntry(storageId, QStringList());
        if (!openLedgers.isEmpty()) {
            for (const auto& id: qAsConst(openLedgers)) {
                auto thisId = id;
                openLedger(thisId.remove(QLatin1String("*")), id.endsWith(QLatin1String("*")));
            }

            // in case we have not opened any ledger, we proceed with the favorites
            if (ui->ledgerTab->count() > 1) {
                q->tabSelected(ui->ledgerTab->currentIndex());
                return;
            }
        }


        AccountsModel* model = MyMoneyFile::instance()->accountsModel();

        const auto subtrees = QVector<QModelIndex> ({ model->favoriteIndex(), model->assetIndex(), model->liabilityIndex() });

        bool stopAfterFirstAccount = false;
        Q_FOREACH(const auto startIdx, subtrees) {
            // retrieve all items in the current subtree
            auto indexes = model->match(model->index(0, 0, startIdx), Qt::DisplayRole, QString("*"), -1, Qt::MatchWildcard);

            // indexes now has a list of favorite accounts
            Q_FOREACH (const auto idx, indexes) {
                openLedger(idx.data(eMyMoney::Model::Roles::IdRole).toString(), false);
                if (stopAfterFirstAccount) {
                    break;
                }
            }

            // if at least one account was found and opened
            // we stop processing
            if (!indexes.isEmpty()) {
                break;
            }
            stopAfterFirstAccount = true;
        }
        ui->ledgerTab->setCurrentIndex(0);
    }

    void openLedger(QString accountId, bool makeCurrentLedger)
    {
        Q_Q(SimpleLedgerView);
        if(inModelUpdate || accountId.isEmpty())
            return;

        accountCombo->hide();

        const auto idx = tabIdByAccountId(accountId);
        if (idx != -1) {
            ui->ledgerTab->setCurrentIndex(idx);
            return;
        }

        LedgerViewPage* view = nullptr;
        MyMoneyAccount acc = MyMoneyFile::instance()->accountsModel()->itemById(accountId);

        // need a new tab, we insert it before the rightmost one
        if(!acc.id().isEmpty()) {

            QString configGroupName;
            switch(acc.accountType()) {
            case eMyMoney::Account::Type::Investment:
                configGroupName = QStringLiteral("InvestmentLedger");
                break;
            default:
                configGroupName = QStringLiteral("StandardLedger");
                break;
            }
            // create new ledger view page
            view = new LedgerViewPage(q, configGroupName);

            view->setAccount(acc);
            view->setShowEntryForNewTransaction(!acc.isClosed());
            view->showTransactionForm(KMyMoneySettings::transactionForm());

            // insert new ledger view page in tab view
            int newIdx = ui->ledgerTab->insertTab(ui->ledgerTab->count()-1, view, acc.name());
            if (makeCurrentLedger) {
                view->prepareToShow();
                // selecting the last tab (the one with the +) and then the new one
                // makes sure that all signals about the new selection are emitted
                ui->ledgerTab->setCurrentIndex(ui->ledgerTab->count()-1);
                ui->ledgerTab->setCurrentIndex(newIdx);
            }

            q->connect(view, &LedgerViewPage::requestSelectionChanged, q, &SimpleLedgerView::requestSelectionChange);
            q->connect(view, &LedgerViewPage::requestCustomContextMenu, q, &SimpleLedgerView::requestCustomContextMenu);

            q->connect(q, &SimpleLedgerView::settingsChanged, view, &LedgerViewPage::slotSettingsChanged);
            q->connect(view, &LedgerViewPage::sectionResized, q, &SimpleLedgerView::sectionResized);
            q->connect(view, &LedgerViewPage::sectionMoved, q, &SimpleLedgerView::sectionMoved);

            q->connect(q, &SimpleLedgerView::resizeSection, view, &LedgerViewPage::resizeSection);
            q->connect(q, &SimpleLedgerView::moveSection, view, &LedgerViewPage::moveSection);

            q->connect(view, &LedgerViewPage::requestView, q, &SimpleLedgerView::requestView);
        }
    }

    void openReconciliationLedger(QString accountId)
    {
        Q_Q(SimpleLedgerView);
        if (inModelUpdate || accountId.isEmpty())
            return;

        accountCombo->hide();

        // in case a stock account is selected, we switch to the parent which
        // is the investment account
        MyMoneyAccount acc = MyMoneyFile::instance()->accountsModel()->itemById(accountId);
        if (acc.isInvest()) {
            acc = MyMoneyFile::instance()->accountsModel()->itemById(acc.parentAccountId());
            accountId = acc.id();
        }

        // check the position of the new tab
        auto newPos = ui->ledgerTab->count() - 1;
        LedgerViewPage* view = 0;
        // check if ledger is already opened
        for (int idx = 0; idx < ui->ledgerTab->count() - 1; ++idx) {
            view = qobject_cast<LedgerViewPage*>(ui->ledgerTab->widget(idx));
            if (view) {
                if (accountId == view->accountId()) {
                    newPos = idx;
                    break;
                }
            }
            view = nullptr; // not found
        }

        // need a new tab, we insert it before the rightmost one
        if (!acc.id().isEmpty()) {
            QString configGroupName;
            switch (acc.accountType()) {
            case eMyMoney::Account::Type::Investment:
                configGroupName = QStringLiteral("InvestmentLedger");
                break;
            default:
                configGroupName = QStringLiteral("StandardLedger");
                break;
            }

            // create new reconciliation ledger view page
            auto reconciliationView = new ReconciliationLedgerViewPage(q, configGroupName);

            reconciliationView->setAccount(acc);
            reconciliationView->setShowEntryForNewTransaction();
            reconciledAccount = acc.id();

            // keep the current view so that we can get it back after reconciliation
            if (view) {
                reconciliationView->pushView(view);
                view->hide();
                ui->ledgerTab->removeTab(newPos);
                reconciliationView->setSplitterSizes(view->splitterSizes());
            }
            reconciliationView->showTransactionForm(KMyMoneySettings::transactionForm());

            // insert new ledger reconciliationView page in tab view
            int newIdx = ui->ledgerTab->insertTab(newPos, reconciliationView, acc.name());

            // we don't allow closing the tab without using the action buttons
            removeCloseButton(
                newIdx,
                i18nc("@info:tooltip",
                      "The close button for this account has been removed because you are reconciling it. Finish, cancel or postpone the reconciliation "
                      "to get it back."));

            q->connect(reconciliationView, &LedgerViewPage::requestSelectionChanged, q, &SimpleLedgerView::slotRequestSelectionChange);
            q->connect(reconciliationView, &LedgerViewPage::requestCustomContextMenu, q, &SimpleLedgerView::requestCustomContextMenu);

            q->connect(q, &SimpleLedgerView::settingsChanged, reconciliationView, &LedgerViewPage::slotSettingsChanged);
            q->connect(reconciliationView, &LedgerViewPage::sectionResized, q, &SimpleLedgerView::sectionResized);
            q->connect(reconciliationView, &LedgerViewPage::sectionMoved, q, &SimpleLedgerView::sectionMoved);
            q->connect(q, &SimpleLedgerView::resizeSection, reconciliationView, &LedgerViewPage::resizeSection);
            q->connect(q, &SimpleLedgerView::moveSection, reconciliationView, &LedgerViewPage::moveSection);

            // selecting the last tab (the one with the +) and then the new one
            // makes sure that all signal about the new selection are emitted
            reconciliationView->prepareToShow();
            ui->ledgerTab->setCurrentIndex(ui->ledgerTab->count() - 1);
            ui->ledgerTab->setCurrentIndex(newIdx);
        }
    }

    void closeLedgers()
    {
        Q_Q(SimpleLedgerView);
        if (m_needInit)
            return;

        // get storage id without the enclosing braces
        const auto storageId = MyMoneyFile::instance()->storageId().toString(QUuid::WithoutBraces);
        KSharedConfigPtr config = KSharedConfig::openConfig();
        KConfigGroup grp = config->group("OpenLedgers");
        grp.deleteEntry(storageId);

        // collect account ids of open ledgers
        QStringList openLedgers;
        LedgerViewPage* view = 0;
        for(int idx = 0; idx < ui->ledgerTab->count()-1; ++idx) {
            view = qobject_cast<LedgerViewPage*>(ui->ledgerTab->widget(idx));
            if(view) {
                auto id = view->accountId();
                if (idx == ui->ledgerTab->currentIndex()) {
                    id.append(QLatin1String("*"));
                }
                openLedgers.append(id);
            }
        }
        // save the ones we have found
        if (!openLedgers.isEmpty()) {
            grp.writeEntry(storageId, openLedgers);
        }

        auto tabCount = ui->ledgerTab->count();
        // check that we have a least one tab that can be closed
        if(tabCount > 1) {
            // we keep the tab with the selector open at all times
            // which is located in the right most position
            --tabCount;
            do {
                --tabCount;
                q->closeLedger(tabCount);
            } while(tabCount > 0);
        }
    }

    void propagateActionToViews(eMenu::Action action, const SelectedObjects& selections)
    {
        LedgerViewPage* view = 0;
        for (int idx = 0; idx < ui->ledgerTab->count() - 1; ++idx) {
            view = qobject_cast<LedgerViewPage*>(ui->ledgerTab->widget(idx));
            if (view) {
                view->executeAction(action, selections);
            }
        }
    }

    /**
     * This method returns the tab index for the given @a accountId
     * or -1 if there is no tab for it open or @a accountId is empty
     *
     * @note updates accountId to point to the investment account in case
     * @a accountId references a stock account
     */
    int tabIdByAccountId(QString& accountId)
    {
        if (ui && ui->ledgerTab && !accountId.isEmpty()) {
            // in case a stock account is selected, we switch to the parent which
            // is the investment account
            MyMoneyAccount acc = MyMoneyFile::instance()->accountsModel()->itemById(accountId);
            if (acc.isInvest()) {
                acc = MyMoneyFile::instance()->accountsModel()->itemById(acc.parentAccountId());
                accountId = acc.id();
            }

            // check if ledger is already opened
            for (int idx = 0; idx < ui->ledgerTab->count() - 1; ++idx) {
                const auto view = qobject_cast<LedgerViewPage*>(ui->ledgerTab->widget(idx));
                if (view) {
                    if (accountId == view->accountId()) {
                        return idx;
                    }
                }
            }
        }
        return -1;
    }

    void updateTitlePage()
    {
        QString txt = QLatin1String("<html><head/><body><p>");
        if (MyMoneyFile::instance()->accountsModel()->itemList().isEmpty()) {
            txt.append(i18nc("@label displayed when no ledger is open",
                             "This page shows the accounts ledger. Currently, no accounts exist so this text is shown instead."));
        } else {
            txt.append(i18nc("@label displayed when no ledger is open",
                             "This page shows the accounts ledger. Currently, no accounts are selected so this text is shown instead."));
        }
        txt.append(QLatin1String("</p></body></html>"));
        ui->l1->setText(txt);
    }

    Ui_SimpleLedgerView*          ui;
    AccountNamesFilterProxyModel* accountsModel;
    QWidget*                      newTabWidget;
    QToolButton*                  webSiteButton;
    KMyMoneyAccountCombo*         accountCombo;
    QUrl                          webSiteUrl;
    QString reconciledAccount;
    int                           lastIdx;
    bool                          inModelUpdate;
    bool                          m_needInit;
};


SimpleLedgerView::SimpleLedgerView(QWidget *parent) :
    KMyMoneyViewBase(*new SimpleLedgerViewPrivate(this), parent)
{
    Q_D(SimpleLedgerView);
    d->m_sharedToolbarActions.insert(eMenu::Action::FileNew, pActions[eMenu::Action::NewTransaction]);

    connect(MyMoneyFile::instance()->accountsModel(), &QAbstractItemModel::dataChanged, this, [&](const QModelIndex& topLeft, const QModelIndex& bottomRight) {
        Q_D(SimpleLedgerView);
        Q_ASSERT(topLeft.parent() == bottomRight.parent());
        for (int row = topLeft.row(); row <= bottomRight.row(); ++row) {
            const auto idx = MyMoneyFile::instance()->accountsModel()->index(row, 0, topLeft.parent());
            const auto modifiedAccountId = idx.data(eMyMoney::Model::IdRole).toString();
            auto tabAccountId = modifiedAccountId;
            const auto tab = d->tabIdByAccountId(tabAccountId);
            // d->tabIdByAccountId() may return the tab for the parent in case
            // a stock account was changed. In this case it also updates
            // tabAccountId to contain the id of the parent account. We only
            // update the name if the parent was changed directly.
            if (tab != -1 && (tabAccountId == modifiedAccountId)) {
                d->ui->ledgerTab->setTabText(tab, idx.data(eMyMoney::Model::AccountNameRole).toString());
            }
        }
    });
}

SimpleLedgerView::~SimpleLedgerView()
{
}

bool SimpleLedgerView::eventFilter(QObject* o, QEvent* e)
{
    Q_D(SimpleLedgerView);

    if (e->type() == QEvent::KeyPress) {
        if (o == d->accountCombo) {
            const auto kev = static_cast<QKeyEvent*>(e);
            if (kev->key() == Qt::Key_Escape) {
                d->accountCombo->hide();
                return true;
            }
        }
    }
    return false;
}

void SimpleLedgerView::openLedger(QString accountId)
{
    Q_D(SimpleLedgerView);
    d->openLedger(accountId, true);
}

void SimpleLedgerView::tabClicked(int idx)
{
    Q_D(SimpleLedgerView);
    if (idx == (d->ui->ledgerTab->count()-1)) {
        const auto rect = d->ui->ledgerTab->tabBar()->tabRect(idx);
        if (!d->accountCombo->isVisible()) {
            d->accountCombo->lineEdit()->clear();
            d->accountCombo->lineEdit()->setFocus();
            // Using the QMetaObject::invokeMethod calls showPopup too early
            // so we delay it a bit (not recongnizable for the user)
            QTimer::singleShot(50, d->accountCombo, SLOT(showPopup()));
        }
        // make the combo box visible
        d->accountCombo->raise();
        d->accountCombo->show();
        // and adjust the size to the largest account
        // shown in the popup treeview
        const auto popupView = d->accountCombo->popup();
        popupView->resizeColumnToContents(0);
        d->accountCombo->resize(popupView->header()->sectionSize(0), d->accountCombo->height());
        // now place the combobox either left or right aligned to the tab button
        if (rect.left() + popupView->header()->sectionSize(0) < width()) {
            d->accountCombo->move(rect.left(), rect.bottom());
        } else {
            d->accountCombo->move(rect.left()+rect.width()- popupView->header()->sectionSize(0), rect.bottom());
        }
    } else {
        // make sure the view's model is initialized before
        // the view is shown
        const auto view = qobject_cast<LedgerViewPage*>(d->ui->ledgerTab->widget(idx));
        if (view) {
            view->prepareToShow();
        }
        d->accountCombo->hide();
    }
}

void SimpleLedgerView::tabSelected(int idx)
{
    Q_D(SimpleLedgerView);
    // make sure that the ledger does not change
    // when the user accesses the account selection combo box
    if(idx != (d->ui->ledgerTab->count()-1)) {
        d->lastIdx = idx;

    } else {
        d->ui->ledgerTab->setCurrentIndex(d->lastIdx);
    }

    const auto view = qobject_cast<LedgerViewPage*>(d->ui->ledgerTab->widget(idx));
    if (view) {
        d->m_selections = view->selections();
    }
    slotRequestSelectionChange(d->m_selections);

    setupCornerWidget();
}

void SimpleLedgerView::closeLedger(int idx)
{
    Q_D(SimpleLedgerView);
    // don't react on the close request for the new ledger function
    // and non-existing tabs
    if ((idx >= 0) && (idx != (d->ui->ledgerTab->count() - 1))) {
        // if the currently selected ledger is closed, we
        // remove any selection
        d->m_selections.clearSelections();
        Q_EMIT requestSelectionChange(d->m_selections);

        auto tab = d->ui->ledgerTab->widget(idx);
        d->ui->ledgerTab->removeTab(idx);

        auto view = qobject_cast<LedgerViewPage*>(tab);
        if (view) {
            view = view->popView();
            if (view) {
                view->prepareToShow();
                d->ui->ledgerTab->insertTab(idx, view, view->accountName());
                d->ui->ledgerTab->setCurrentIndex(d->ui->ledgerTab->count() - 1);
                d->ui->ledgerTab->setCurrentIndex(idx);
                d->reconciledAccount.clear();
            }
        }
        delete tab;

        // make sure we always show an account
        if (d->ui->ledgerTab->currentIndex() == (d->ui->ledgerTab->count()-1)) {
            if (d->ui->ledgerTab->count() > 1) {
                d->ui->ledgerTab->setCurrentIndex((d->ui->ledgerTab->count()-2));
            }
        } else {
            tabSelected(d->ui->ledgerTab->currentIndex());
        }
    }
}

void SimpleLedgerView::checkTabOrder(int from, int to)
{
    Q_D(SimpleLedgerView);
    if(d->inModelUpdate)
        return;

    QTabBar* bar = d->ui->ledgerTab->findChild<QTabBar*>();
    if(bar) {
        const int rightMostIdx = d->ui->ledgerTab->count()-1;

        if(from == rightMostIdx) {
            // someone tries to move the new account tab away from the rightmost position
            d->inModelUpdate = true;
            bar->moveTab(to, from);
            d->inModelUpdate = false;
        }
    }
}

void SimpleLedgerView::showEvent(QShowEvent* event)
{
    Q_D(SimpleLedgerView);
    if (d->m_needInit) {
        d->init();

        // close ledgers of accounts about to be removed
        connect(MyMoneyFile::instance()->accountsModel(), &AccountsModel::rowsAboutToBeRemoved, this, [&](const QModelIndex& parent, int first, int last) {
            Q_D(SimpleLedgerView);
            // we only need to react on real account removals not if the favorite
            // option is removed and the row is removed from the favorite subtree
            if (parent != MyMoneyFile::instance()->accountsModel()->favoriteIndex()) {
                for (int row = first; row <= last; ++row) {
                    const auto modelIdx = parent.model()->index(row, 0, parent);
                    auto accountId = modelIdx.data(eMyMoney::Model::IdRole).toString();
                    const auto idx = d->tabIdByAccountId(accountId);
                    if (idx != -1) {
                        closeLedger(idx);
                    }
                }
            }
        });
    }
    // don't forget base class implementation
    QWidget::showEvent(event);
}

void SimpleLedgerView::setupCornerWidget()
{
    Q_D(SimpleLedgerView);

    // check if we already have the button, quit if not
    if (!d->webSiteButton)
        return;

    d->webSiteButton->hide();
    auto view = qobject_cast<LedgerViewPage*>(d->ui->ledgerTab->currentWidget());
    if (view) {
        const auto accountsModel = MyMoneyFile::instance()->accountsModel();
        auto index = accountsModel->indexById(view->accountId());
        if(index.isValid()) {
            // get icon name and url via account and institution object
            const auto acc = accountsModel->itemByIndex(index);
            if (!acc.institutionId().isEmpty()) {
                const auto institutionsModel = MyMoneyFile::instance()->institutionsModel();
                index = institutionsModel->indexById(acc.institutionId());
                const auto institution = institutionsModel->itemByIndex(index);
                const auto url = institution.value(QStringLiteral("url"));
                const auto iconName = institution.value(QStringLiteral("icon"));
                if (!url.isEmpty() && !iconName.isEmpty()) {
                    const auto favIcon = Icons::loadIconFromApplicationCache(iconName);
                    if (!favIcon.isNull()) {
                        d->webSiteButton->show();
                        d->webSiteButton->setIcon(favIcon);
                        d->webSiteButton->setText(url);
                        d->webSiteButton->setToolTip(i18n("Open website of <b>%1</b> in your browser.", institution.name()));

                        // we remove all parts of the URL that we provide ourselves first
                        auto webUrl(url);
                        auto pos = webUrl.indexOf(QLatin1String("://"));
                        if (pos != -1) {
                            webUrl = webUrl.mid(pos + 3);
                        }
                        // make sure it has at least one /
                        pos = webUrl.indexOf(QLatin1Char('/'));
                        if (pos != -1) {
                            webUrl.push_back(QLatin1Char('/'));
                        }
                        d->webSiteUrl.setUrl(QString::fromLatin1("https://%1").arg(url));
                    }
                }
            }
        }
    }
}

void SimpleLedgerView::aboutToShow()
{
    Q_D(SimpleLedgerView);

    d->m_selections.clearSelections();

    // we don't do anything special here if
    // we are not fully initialized
    if (!d->m_needInit) {
        // in case we have at least one account open
        if (d->ui->ledgerTab->count() > 1) {
            // use its current selection
            const auto view = qobject_cast<LedgerViewPage*>(d->ui->ledgerTab->currentWidget());
            if (view) {
                d->m_selections = view->selections();
            }
        }
    }

    // don't forget base class logic
    KMyMoneyViewBase::aboutToShow();
}

void SimpleLedgerView::slotSettingsChanged()
{
    Q_D(SimpleLedgerView);
    if (d->accountsModel) {
        d->accountsModel->setHideClosedAccounts(!KMyMoneySettings::showAllAccounts());
        d->accountsModel->setHideEquityAccounts(!KMyMoneySettings::expertMode());
        d->accountsModel->setHideZeroBalancedEquityAccounts(KMyMoneySettings::hideZeroBalanceEquities());
        d->accountsModel->setHideFavoriteAccounts(false);
    }
    Q_EMIT settingsChanged();
}

void SimpleLedgerView::slotRequestSelectionChange(const SelectedObjects& selections) const
{
    Q_D(const SimpleLedgerView);
    auto newSelections(selections);

    if (!d->reconciledAccount.isEmpty()) {
        newSelections.setSelection(SelectedObjects::ReconciliationAccount, d->reconciledAccount);
    }
    Q_EMIT requestSelectionChange(newSelections);
}

void SimpleLedgerView::updateActions(const SelectedObjects& selections)
{
    Q_D(SimpleLedgerView);
    if (d->isActiveView()) {
        const auto reconciledAccount = selections.firstSelection(SelectedObjects::ReconciliationAccount);
        if (!reconciledAccount.isEmpty()) {
            if (reconciledAccount == selections.firstSelection(SelectedObjects::Account)) {
                pActions[eMenu::Action::PostponeReconciliation]->setEnabled(true);
                pActions[eMenu::Action::FinishReconciliation]->setEnabled(true);
            }
            pActions[eMenu::Action::CancelReconciliation]->setEnabled(true);
        }
    }
    if (!d->m_needInit) {
        d->updateTitlePage();
    }
}

void SimpleLedgerView::executeAction(eMenu::Action action, const SelectedObjects& selections)
{
    Q_D(SimpleLedgerView);
    auto accountId = selections.firstSelection(SelectedObjects::Account);
    switch (action) {
    case eMenu::Action::GoToAccount:
    case eMenu::Action::NewTransaction:
    case eMenu::Action::OpenAccount:
    case eMenu::Action::EditTransaction:
    case eMenu::Action::EditSplits:
    case eMenu::Action::SelectAllTransactions:
    case eMenu::Action::EditTabOrder:
    case eMenu::Action::ShowTransaction:
        if (d->isActiveView() && !accountId.isEmpty()) {
            d->openLedger(accountId, true);
            auto view = qobject_cast<LedgerViewPage*>(d->ui->ledgerTab->currentWidget());
            if (view) {
                view->executeAction(action, selections);
            }
        }
        break;
    case eMenu::Action::StartReconciliation:
        if (d->isActiveView() && !accountId.isEmpty()) {
            const auto reconciledAccountId = selections.firstSelection(SelectedObjects::ReconciliationAccount);
            if (reconciledAccountId.isEmpty())
                d->openReconciliationLedger(accountId);
            else
                d->openLedger(accountId, true);
            auto view = qobject_cast<LedgerViewPage*>(d->ui->ledgerTab->currentWidget());
            if (view) {
                view->executeAction(action, selections);
            }
        }
        break;
    case eMenu::Action::FinishReconciliation:
    case eMenu::Action::PostponeReconciliation:
    case eMenu::Action::CancelReconciliation:
        if (d->isActiveView() && !accountId.isEmpty()) {
            const auto reconciledAccountId = selections.firstSelection(SelectedObjects::ReconciliationAccount);
            d->openLedger(reconciledAccountId, true);
            auto view = qobject_cast<LedgerViewPage*>(d->ui->ledgerTab->currentWidget());
            if (view) {
                if (view->executeAction(action, selections)) {
                    closeLedger(d->ui->ledgerTab->currentIndex());
                }
            }
        }
        break;

    case eMenu::Action::MatchTransaction:
        d->propagateActionToViews(action, selections);
        break;

    case eMenu::Action::FileNew:
        d->openLedgersAfterFileOpen();
        break;

    case eMenu::Action::FileClose:
        d->closeLedgers();
        break;

    case eMenu::Action::CloseAccount:
        // close the ledger in case it is shown
        closeLedger(d->tabIdByAccountId(accountId));
        break;

    default:
        break;
    }
}

void SimpleLedgerView::sectionResized(QWidget* view, const QString& configGroupName, int section, int oldSize, int newSize) const
{
    Q_EMIT resizeSection(view, configGroupName, section, oldSize, newSize);
}

void SimpleLedgerView::sectionMoved(QWidget* view, int section, int oldIndex, int newIndex) const
{
    Q_EMIT moveSection(view, section, oldIndex, newIndex);
}

bool SimpleLedgerView::hasClosableView() const
{
    Q_D(const SimpleLedgerView);
    return d->ui->ledgerTab->count() > 1;
}

void SimpleLedgerView::closeCurrentView()
{
    Q_D(SimpleLedgerView);
    const auto idx = d->ui->ledgerTab->currentIndex();

    // in case we're in reconciliation, we cannot simply close the
    // view but must use the finish/postpone reconciliation actions
    auto tab = d->ui->ledgerTab->widget(idx);
    auto view = qobject_cast<LedgerViewPage*>(tab);
    if (view) {
        if (view->hasPushedView()) {
            KMessageBox::information(
                this,
                i18n("You cannot close this view because you are reconciling the account. Finish, cancel or postpone the reconciliation first."),
                i18nc("@title:info In reconciliation", "View cannot be closed"));
            return;
        }
    }

    // ok to close the view
    closeLedger(idx);
}
