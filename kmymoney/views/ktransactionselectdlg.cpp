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

#include "icons.h"
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
        , sortOrder(Qt::AscendingOrder)
    {
    }

    ~KTransactionSelectDlgPrivate()
    {
        delete ui;
    }

    KTransactionSelectDlg* q_ptr;
    Ui::KTransactionSelectDlg* ui;
    LedgerJournalIdFilter* filterModel;
    Qt::SortOrder sortOrder;
};

KTransactionSelectDlg::KTransactionSelectDlg(QWidget* parent)
    : QDialog(parent)
    , d_ptr(new KTransactionSelectDlgPrivate(this))
{
    Q_D(KTransactionSelectDlg);
    d->ui->setupUi(this);
    d->ui->switchButton->hide();

    d->filterModel->setSourceModel(MyMoneyFile::instance()->journalModel());
    d->ui->m_ledgerView->setModel(d->filterModel);
    // don't show sort indicator and don't allow sorting via header
    d->ui->m_ledgerView->horizontalHeader()->setSortIndicatorShown(false);
    d->ui->m_ledgerView->setSortingEnabled(false);

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
    d->filterModel->sort(JournalModel::Column::Date, Qt::AscendingOrder);
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
    Q_D(KTransactionSelectDlg);
    // setup descriptive texts
    setWindowTitle(i18n("Merge Transactions"));
    d_ptr->ui->label->setText(i18nc("@info:label Description of merge transaction dialog",
                                    "The two selected transactions to merge are shown. The one at "
                                    "the top and its values will be used in the resulting "
                                    "merged transaction.  If you want to use the other one "
                                    "press the exchange button on the right."));

    d->ui->switchButton->setIcon(Icons::get(Icons::Icon::ItemExchange));
    d->ui->switchButton->setToolTip(i18nc("@info:tooltip Exchange transactions about to merge", "Press to exchange the two transactions."));
    d->ui->switchButton->show();

    connect(d->ui->switchButton, &QAbstractButton::clicked, this, [&]() {
        Q_D(KTransactionSelectDlg);
        // swap order
        d->sortOrder = (d->sortOrder == Qt::DescendingOrder) ? Qt::AscendingOrder : Qt::DescendingOrder;
        d->filterModel->setSortRole(eMyMoney::Model::IdRole);
        d->filterModel->sort(0, d->sortOrder);

        // and reselect the first entry
        const auto idx = d->filterModel->index(0, 0);
        d->ui->m_ledgerView->setSelectedJournalEntries(QStringList(idx.data(eMyMoney::Model::IdRole).toString()));
    });
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

QString KTransactionMergeDlg::remainingTransactionId() const
{
    Q_D(const KTransactionSelectDlg);

    if (d->filterModel->rowCount() == 2) {
        const auto idx = d->filterModel->index(0, 0);
        return idx.data(eMyMoney::Model::IdRole).toString();
    }
    return {};
}

QString KTransactionMergeDlg::mergedTransactionId() const
{
    Q_D(const KTransactionSelectDlg);

    if (d->filterModel->rowCount() == 2) {
        const auto idx = d->filterModel->index(1, 0);
        return idx.data(eMyMoney::Model::IdRole).toString();
    }
    return {};
}
