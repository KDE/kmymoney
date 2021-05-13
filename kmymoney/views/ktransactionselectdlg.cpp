/*
    SPDX-FileCopyrightText: 2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ktransactionselectdlg.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_ktransactionselectdlg.h"

#include "journalmodel.h"
#include "ledgerjournalidfilter.h"
#include "mymoneyfile.h"

class KTransactionSelectDlgPrivate
{
    Q_DISABLE_COPY(KTransactionSelectDlgPrivate)
    Q_DECLARE_PUBLIC(KTransactionSelectDlg)
public:
    explicit KTransactionSelectDlgPrivate(KTransactionSelectDlg* qq)
        : q_ptr(qq)
        , ui(new Ui::KTransactionSelectDlg)
        , filterModel(new LedgerJournalIdFilter(qq, QVector<QAbstractItemModel*>{}))
    {
    }

    ~KTransactionSelectDlgPrivate()
    {
        delete ui;
    }

    KTransactionSelectDlg* q_ptr;
    Ui::KTransactionSelectDlg* ui;
    LedgerJournalIdFilter* filterModel;
};

KTransactionSelectDlg::KTransactionSelectDlg(QWidget* parent)
    : QDialog(parent)
    , d_ptr(new KTransactionSelectDlgPrivate(this))
{
    Q_D(KTransactionSelectDlg);
    d->ui->setupUi(this);

    d->filterModel->setSourceModel(MyMoneyFile::instance()->journalModel());
    d->ui->m_ledgerView->setModel(d->filterModel);

    QVector<int> columns;
    columns = {
        JournalModel::Column::Number,
        JournalModel::Column::Account,
        JournalModel::Column::Security,
        JournalModel::Column::CostCenter,
        JournalModel::Column::Quantity,
        JournalModel::Column::Price,
        JournalModel::Column::Amount,
        JournalModel::Column::Value,
        JournalModel::Column::Balance,
    };
    d->ui->m_ledgerView->setColumnsHidden(columns);
    columns = {
        JournalModel::Column::Date,
        JournalModel::Column::Detail,
        JournalModel::Column::Reconciliation,
        JournalModel::Column::Payment,
        JournalModel::Column::Deposit,
    };
    d->ui->m_ledgerView->setColumnsShown(columns);
    d->filterModel->setSortRole(eMyMoney::Model::IdRole);
    d->filterModel->setSortLocaleAware(true);
    d->filterModel->sort(0);
    d->filterModel->setDynamicSortFilter(true);
}

KTransactionSelectDlg::~KTransactionSelectDlg()
{
    Q_D(KTransactionSelectDlg);
    delete d;
}

void KTransactionSelectDlg::addTransaction(const QString& journalEntryId)
{
    Q_D(KTransactionSelectDlg);
    d->filterModel->appendFilterFixedString(journalEntryId);
}

KTransactionMergeDlg::KTransactionMergeDlg(QWidget* parent)
    : KTransactionSelectDlg(parent)
{
    // setup descriptive texts
    setWindowTitle(i18n("Merge Transactions"));
    d_ptr->ui->label->setText(
        i18n("Are you sure you wish to merge these transactions?  The one you "
             "selected first is the top one, and its values will be used in "
             "the merged transaction.  Cancel and select the other transaction "
             "first, if you want its values to be used."));

    // no selection possible
    // d_ptr->ui->m_register->setSelectionMode(QTableWidget::NoSelection);

    // override default and enable ok button right away
    // d_ptr->ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void KTransactionMergeDlg::addTransaction(const QString& journalEntryId)
{
    Q_D(KTransactionSelectDlg);

    KTransactionSelectDlg::addTransaction(journalEntryId);
    const auto idx = d->filterModel->index(0, 0);
    d->ui->m_ledgerView->setSelectedJournalEntries(QStringList(idx.data(eMyMoney::Model::IdRole).toString()));
}
