/*
    SPDX-FileCopyrightText: 2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ksearchtransactiondlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QAction>
#include <QKeyEvent>
#include <QPointer>
#include <QPushButton>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KGuiItem>
#include <KLocalizedString>
#include <KStandardGuiItem>

// ----------------------------------------------------------------------------
// Project Includes

#include "journalmodel.h"
#include "ktransactionfilter.h"
#include "ledgerjournalidfilter.h"
#include "menuenums.h"
#include "mymoneyfile.h"
#include "mymoneytransactionfilter.h"
#include "selectedobjects.h"
#include "specialdatesmodel.h"
#include "ui_ksearchtransactiondlg.h"

class KSearchTransactionDlgPrivate
{
    Q_DISABLE_COPY(KSearchTransactionDlgPrivate)
    Q_DECLARE_PUBLIC(KSearchTransactionDlg)

public:
    KSearchTransactionDlgPrivate(KSearchTransactionDlg* qq)
        : q_ptr(qq)
        , filterModel(new LedgerJournalIdFilter(qq, QVector<QAbstractItemModel*>{/*MyMoneyFile::instance()->specialDatesModel()*/}))
    {
    }

    void init()
    {
        Q_Q(KSearchTransactionDlg);

        ui.setupUi(q);
        filterTab = new KTransactionFilter(q);
        ui.m_tabWidget->insertTab(0, filterTab, i18nc("Criteria tab", "Criteria"));

        // disable access to result page until we have a result
        ui.m_tabWidget->setTabEnabled(ui.m_tabWidget->indexOf(ui.m_resultPage), false);

        // only allow searches when a selection has been made
        ui.buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
        ui.buttonBox->button(QDialogButtonBox::Apply)->setDefault(true);
        ui.buttonBox->button(QDialogButtonBox::Apply)->setAutoDefault(true);
        KGuiItem::assign(ui.buttonBox->button(QDialogButtonBox::Apply), KStandardGuiItem::find());
        ui.buttonBox->button(QDialogButtonBox::Apply)->setToolTip(i18nc("@info:tooltip for find transaction apply button", "Search transactions"));
        q->connect(filterTab, &KTransactionFilter::selectionNotEmpty, ui.buttonBox->button(QDialogButtonBox::Apply), &QWidget::setEnabled);
    }

    void search()
    {
        // perform the search only if the button is enabled
        if (!ui.buttonBox->button(QDialogButtonBox::Apply)->isEnabled())
            return;

        filterModel->setSourceModel(MyMoneyFile::instance()->journalModel());
        ui.m_ledgerView->setModel(filterModel);

        // setup the filter from the dialog widgets
        filterModel->setFilterRole(eMyMoney::Model::Roles::IdRole);
        const auto filter = filterTab->setupFilter();
        const auto list = MyMoneyFile::instance()->journalEntryIds(filter);
        filterModel->setFilterFixedStrings(list);

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
        ui.m_ledgerView->setColumnsHidden(columns);
        columns = {
            JournalModel::Column::Date,
            JournalModel::Column::Account,
            JournalModel::Column::Detail,
            JournalModel::Column::Reconciliation,
            JournalModel::Column::Payment,
            JournalModel::Column::Deposit,
        };
        ui.m_ledgerView->setColumnsShown(columns);

        ui.m_tabWidget->setTabEnabled(ui.m_tabWidget->indexOf(ui.m_resultPage), true);
        ui.m_tabWidget->setCurrentWidget(ui.m_resultPage);
        ui.m_ledgerView->setFocus();

        ui.m_foundText->setText(i18np("Found %1 matching splits", "Found %1 matching splits", filterModel->rowCount()));
    }

    void selectTransaction(const QModelIndex& idx)
    {
        Q_Q(KSearchTransactionDlg);

        const auto accountId = idx.data(eMyMoney::Model::JournalSplitAccountIdRole).toString();
        SelectedObjects selections;
        selections.setSelection(SelectedObjects::Account, accountId);
        selections.setSelection(SelectedObjects::JournalEntry, idx.data(eMyMoney::Model::IdRole).toString());

        // close myself and open the ledger of the account
        q->hide();

        q->requestSelectionChange(selections);
        pActions[eMenu::Action::GoToAccount]->setData(accountId);
        pActions[eMenu::Action::GoToAccount]->trigger();
    }

    KSearchTransactionDlg* q_ptr;
    LedgerJournalIdFilter* filterModel;
    Ui_KSearchTransactionDlg ui;
    QPointer<KTransactionFilter> filterTab;
};

KSearchTransactionDlg::KSearchTransactionDlg(QWidget* parent)
    : QDialog(parent)
    , d_ptr(new KSearchTransactionDlgPrivate(this))
{
    Q_D(KSearchTransactionDlg);
    d->init();

    connect(d->ui.buttonBox->button(QDialogButtonBox::Apply), &QAbstractButton::clicked, this, [&]() {
        Q_D(KSearchTransactionDlg);
        d->search();
    });

    connect(d->ui.buttonBox->button(QDialogButtonBox::Reset), &QAbstractButton::clicked, this, [&]() {
        Q_D(KSearchTransactionDlg);
        d->filterTab->slotReset();
    });

    connect(d->ui.buttonBox->button(QDialogButtonBox::Help), &QAbstractButton::clicked, this, [&]() {
        Q_D(KSearchTransactionDlg);
        if (d->ui.m_tabWidget->currentIndex() == 0) {
            d->filterTab->slotShowHelp();
        }
    });

    connect(d->ui.buttonBox->button(QDialogButtonBox::Close), &QAbstractButton::clicked, this, &QObject::deleteLater);

    d->filterTab->setFocus();

    // we don't allow editing here but double click selects the current
    // selected transaction in the ledger view
    d->ui.m_ledgerView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(d->ui.m_ledgerView, &QAbstractItemView::doubleClicked, this, [&](const QModelIndex& idx) {
        Q_D(KSearchTransactionDlg);
        d->selectTransaction(idx);
    });

    d->ui.m_ledgerView->installEventFilter(this);
}

KSearchTransactionDlg::~KSearchTransactionDlg()
{
}

bool KSearchTransactionDlg::eventFilter(QObject* watched, QEvent* event)
{
    Q_D(KSearchTransactionDlg);
    if (watched == d->ui.m_ledgerView) {
        if (event->type() == QEvent::KeyPress) {
            const auto kev = static_cast<QKeyEvent*>(event);
            if ((kev->key() == Qt::Key_Enter) || (kev->key() == Qt::Key_Return)) {
                d->selectTransaction(d->ui.m_ledgerView->currentIndex());
            }
        }
    }
    return QDialog::eventFilter(watched, event);
}
