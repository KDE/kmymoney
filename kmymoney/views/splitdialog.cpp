/*
    SPDX-FileCopyrightText: 2015-2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "splitdialog.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDebug>
#include <QHeaderView>
#include <QPointer>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KColorScheme>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

// ----------------------------------------------------------------------------
// Project Includes

#include "icons.h"
#include "mymoneysecurity.h"
#include "splitadjustdialog.h"
#include "splitmodel.h"
#include "ui_splitdialog.h"

using namespace Icons;

class SplitDialog::Private
{
    Q_DISABLE_COPY_MOVE(Private)

public:
    Private(SplitDialog* p)
        : parent(p)
        , ui(new Ui_SplitDialog)
        , transactionEditor(nullptr)
        , splitModel(nullptr)
        , fraction(100)
        , readOnly(false)
    {
    }

    ~Private()
    {
        delete ui;
    }

    void deleteSplits(QModelIndexList indexList);
    void blockEditorStart(bool blocked);
    void blockImmediateEditor();
    void selectRow(int row);

    SplitDialog* parent;
    Ui_SplitDialog* ui;

    /**
     * The parent transaction editor which opened the split editor
     */
    QWidget* transactionEditor;

    SplitModel* splitModel;

    /**
     * The fraction of the account for which this split editor was opened
     */
    int fraction;

    MyMoneyMoney transactionTotal;
    MyMoneyMoney splitsTotal;
    MyMoneyMoney inversionFactor;
    QString transactionPayeeId;
    QString commoditySymbol;
    bool readOnly;
};

static const int SumRow = 0;
static const int DiffRow = 1;
static const int AmountRow = 2;
static const int HeaderCol = 0;
static const int ValueCol = 1;
static const int SummaryRows = 3;
static const int SummaryCols = 2;

void SplitDialog::Private::deleteSplits(QModelIndexList indexList)
{
    if (indexList.isEmpty()) {
        return;
    }

    // remove from the end so that the row information stays
    // consistent and is not changed due to index changes
    QMap<int, int> sortedList;
    for (const auto& index : indexList) {
        sortedList[index.row()] = index.row();
    }

    blockEditorStart(true);
    const auto model = ui->splitView->model();
    QMap<int, int>::const_iterator it = sortedList.constEnd();
    do {
        --it;
        const auto idx = model->index(*it, 0);
        const auto id = idx.data(eMyMoney::Model::IdRole).toString();
        if (!(id.isEmpty() || id.endsWith('-'))) {
            model->removeRow(*it);
        }
    } while (it != sortedList.constBegin());
    blockEditorStart(false);
}

void SplitDialog::Private::blockEditorStart(bool blocked)
{
    ui->splitView->blockEditorStart(blocked);
}

void SplitDialog::Private::blockImmediateEditor()
{
    if (ui->splitView->model()->rowCount() <= 1) {
        ui->splitView->skipStartEditing();
    }
}

void SplitDialog::Private::selectRow(int row)
{
    if (row >= ui->splitView->model()->rowCount())
        row = ui->splitView->model()->rowCount() - 1;
    if (row >= 0) {
        blockEditorStart(true);
        ui->splitView->selectRow(row);
        blockEditorStart(false);
    }
}

SplitDialog::SplitDialog(const MyMoneySecurity& commodity,
                         const MyMoneyMoney& amount,
                         int fraction,
                         const MyMoneyMoney& inversionFactor,
                         QWidget* parent,
                         Qt::WindowFlags f)
    : QDialog(parent, f)
    , d(new Private(this))
{
    d->transactionEditor = parent;
    d->fraction = fraction;
    d->transactionTotal = amount;
    d->inversionFactor = inversionFactor;
    d->commoditySymbol = commodity.tradingSymbol();
    d->ui->setupUi(this);

    d->ui->splitView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    d->ui->splitView->setSelectionBehavior(QAbstractItemView::SelectRows);
    d->ui->splitView->setCommodity(commodity);
    d->ui->splitView->setTotalTransactionValue(amount);

    d->ui->okButton->setIcon(Icons::get(Icon::DialogOK));
    d->ui->cancelButton->setIcon(Icons::get(Icon::DialogCancel));

    // setup some connections
    connect(d->ui->splitView, &SplitView::aboutToStartEdit, this, &SplitDialog::disableButtons);
    connect(d->ui->splitView, &SplitView::aboutToFinishEdit, this, &SplitDialog::enableButtons);
    connect(d->ui->splitView, &SplitView::deleteSelectedSplits, this, &SplitDialog::deleteSelectedSplits);

    connect(d->ui->deleteAllButton, &QAbstractButton::pressed, this, &SplitDialog::deleteAllSplits);
    connect(d->ui->deleteButton, &QAbstractButton::pressed, this, &SplitDialog::deleteSelectedSplits);
    connect(d->ui->deleteZeroButton, &QAbstractButton::pressed, this, &SplitDialog::deleteZeroSplits);
    connect(d->ui->adjustUnassigned, &QAbstractButton::pressed, this, &SplitDialog::adjustUnassigned);
    connect(d->ui->mergeButton, &QAbstractButton::pressed, this, &SplitDialog::mergeSplits);
    connect(d->ui->newSplitButton, &QAbstractButton::pressed, this, &SplitDialog::newSplit);

    ensurePolished();

    QSize size(width(), height());
    KConfigGroup grp = KSharedConfig::openConfig()->group("SplitTransactionEditor");
    size = grp.readEntry("Geometry", size);
    size.setHeight(size.height() - 1);
    resize(size.expandedTo(minimumSizeHint()));

    // m_unassigned_over = KColorScheme(QPalette::Normal).foreground(KColorScheme::PositiveText);
    // m_unassigned_under = KColorScheme(QPalette::Normal).foreground(KColorScheme::NegativeText);
    m_unassigned_error = KColorScheme(QPalette::Normal).foreground(KColorScheme::NegativeText);
    m_unassigned_normal = KColorScheme(QPalette::Normal).foreground(KColorScheme::NormalText);

    const int rowHeight = d->ui->summaryView->verticalHeader()->fontMetrics().lineSpacing() + 2;
    d->ui->summaryView->verticalHeader()->setMinimumSectionSize(20);
    d->ui->summaryView->verticalHeader()->setDefaultSectionSize(rowHeight);
    d->ui->summaryView->setMinimumHeight((d->ui->summaryView->model()->rowCount() * rowHeight) + 4);

    // finish polishing the widgets
    QMetaObject::invokeMethod(this, "adjustSummary", Qt::QueuedConnection);
}

SplitDialog::~SplitDialog()
{
    auto grp = KSharedConfig::openConfig()->group("SplitTransactionEditor");
    grp.writeEntry("Geometry", size());
}

int SplitDialog::exec()
{
    if (!d->ui->splitView->model()) {
        qWarning() << "SplitDialog::exec() executed without a model. Use setModel() before calling exec().";
        return QDialog::Rejected;
    }
    return QDialog::exec();
}

void SplitDialog::accept()
{
    adjustSummary();
    bool accept = true;
    if (d->transactionTotal.isAutoCalc()) {
        d->transactionTotal = d->splitsTotal;

    } else if (d->transactionTotal != d->splitsTotal) {
        QPointer<SplitAdjustDialog> dlg = new SplitAdjustDialog(this);
        dlg->setValues(d->ui->summaryView->item(AmountRow, ValueCol)->data(Qt::DisplayRole).toString(),
                       d->ui->summaryView->item(SumRow, ValueCol)->data(Qt::DisplayRole).toString(),
                       d->ui->summaryView->item(DiffRow, ValueCol)->data(Qt::DisplayRole).toString(),
                       d->ui->splitView->model()->rowCount());
        accept = false;
        if (dlg->exec() == QDialog::Accepted && dlg) {
            switch (dlg->selectedOption()) {
            case SplitAdjustDialog::SplitAdjustContinue:
                break;
            case SplitAdjustDialog::SplitAdjustChange:
                d->transactionTotal = d->splitsTotal;
                accept = true;
                break;
            case SplitAdjustDialog::SplitAdjustDistribute:
                qWarning() << "SplitDialog::accept needs to implement the case SplitAdjustDialog::SplitAdjustDistribute";
                accept = true;
                break;
            case SplitAdjustDialog::SplitAdjustLeaveAsIs:
                accept = true;
                break;
            }
        }
        delete dlg;
        updateButtonState();
    }
    if (accept)
        QDialog::accept();
}

void SplitDialog::enableButtons()
{
    d->ui->buttonContainer->setEnabled(true);
}

void SplitDialog::disableButtons()
{
    d->ui->buttonContainer->setEnabled(false);
}

void SplitDialog::setModel(SplitModel* model)
{
    d->splitModel = model;
    d->ui->splitView->setModel(model);

    if (model->rowCount() > 0) {
        QModelIndex index = model->index(0, 0);
        d->ui->splitView->setCurrentIndex(index);
    }

    adjustSummary();

    // force an update of the summary if data changes in the model
    connect(model, &QAbstractItemModel::dataChanged, this, &SplitDialog::adjustSummary);
    connect(d->ui->splitView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SplitDialog::selectionChanged);
}

void SplitDialog::adjustSummary()
{
    // Apply color scheme to the summary panel
    for (int row = 0; row < SummaryRows; row++) {
        for (int col = 0; col < SummaryCols; col++) {
            if (row == DiffRow && col == ValueCol)
                continue;
            d->ui->summaryView->item(row, col)->setForeground(m_unassigned_normal);
        }
    }

    // Only show the currency symbol when multiple currencies are involved
    QString currencySymbol = d->commoditySymbol;
    if (!d->splitModel->hasMultiCurrencySplits()) {
        currencySymbol.clear();
    }

    d->splitsTotal = 0;
    const auto model = d->ui->splitView->model();

    bool haveAutoCalcSplits(false);
    for (int row = 0; row < model->rowCount(); ++row) {
        const auto index = model->index(row, 0);
        if (index.isValid() && !index.data(eMyMoney::Model::SplitIsNewRole).toBool()) {
            haveAutoCalcSplits |= index.data(eMyMoney::Model::SplitIsAutoCalcRole).toBool();
            d->splitsTotal += index.data(eMyMoney::Model::SplitValueRole).value<MyMoneyMoney>();
        }
    }

    const int denom = MyMoneyMoney::denomToPrec(d->fraction);
    QString formattedValue = (d->splitsTotal * d->inversionFactor).formatMoney(currencySymbol, denom);
    if (!haveAutoCalcSplits) {
        d->ui->summaryView->item(SumRow, HeaderCol)->setData(Qt::DisplayRole, i18nc("Split editor summary", "Sum of splits"));
        d->ui->summaryView->item(SumRow, ValueCol)->setData(Qt::DisplayRole, formattedValue);
    } else {
        d->ui->summaryView->item(SumRow, HeaderCol)->setData(Qt::DisplayRole, QString());
        d->ui->summaryView->item(SumRow, ValueCol)->setData(Qt::DisplayRole, QString());
    }

    if (d->transactionEditor) {
        if (d->transactionTotal.isAutoCalc()) {
            formattedValue = (d->splitsTotal * d->inversionFactor).formatMoney(currencySymbol, denom);
        } else {
            formattedValue = (d->transactionTotal * d->inversionFactor).formatMoney(currencySymbol, denom);
        }
        d->ui->summaryView->item(AmountRow, ValueCol)->setData(Qt::DisplayRole, formattedValue);

        if (!d->transactionTotal.isAutoCalc() && !haveAutoCalcSplits) {
            auto diff = d->transactionTotal.abs() - d->splitsTotal.abs();
            if (diff.isNegative()) {
                d->ui->summaryView->item(DiffRow, HeaderCol)->setData(Qt::DisplayRole, i18nc("Split editor summary", "Overassigned"));
                d->ui->summaryView->item(DiffRow, ValueCol)->setForeground(m_unassigned_error);
            } else {
                d->ui->summaryView->item(DiffRow, HeaderCol)->setData(Qt::DisplayRole, i18nc("Split editor summary", "Unassigned"));
                if (diff.isZero()) {
                    d->ui->summaryView->item(DiffRow, ValueCol)->setForeground(m_unassigned_normal);
                } else {
                    d->ui->summaryView->item(DiffRow, ValueCol)->setForeground(m_unassigned_error);
                }
            }
            formattedValue = (d->transactionTotal - d->splitsTotal).abs().formatMoney(currencySymbol, denom);
            d->ui->summaryView->item(DiffRow, ValueCol)->setData(Qt::DisplayRole, formattedValue);
        } else {
            d->ui->summaryView->item(DiffRow, HeaderCol)->setData(Qt::DisplayRole, QString());
            d->ui->summaryView->item(DiffRow, ValueCol)->setData(Qt::DisplayRole, QString());
        }
    } else {
        d->ui->summaryView->item(SumRow, ValueCol)->setData(Qt::DisplayRole, QString());
        d->ui->summaryView->item(AmountRow, ValueCol)->setData(Qt::DisplayRole, QString());
    }

    adjustSummaryWidth();
    updateButtonState();
}

void SplitDialog::resizeEvent(QResizeEvent* ev)
{
    QDialog::resizeEvent(ev);
    adjustSummaryWidth();
}

void SplitDialog::adjustSummaryWidth()
{
    d->ui->summaryView->resizeColumnToContents(1);
    d->ui->summaryView->horizontalHeader()->resizeSection(0, d->ui->summaryView->width() - d->ui->summaryView->horizontalHeader()->sectionSize(1) - 10);
}

void SplitDialog::newSplit()
{
    // creating a new split is easy, because we simply
    // need to select the last entry in the view. If we
    // are on this row already with the editor closed things
    // are a bit more complicated.
    QModelIndex index = d->ui->splitView->currentIndex();
    if (index.isValid()) {
        int row = index.row();
        if (row != d->ui->splitView->model()->rowCount() - 1) {
            d->ui->splitView->selectRow(d->ui->splitView->model()->rowCount() - 1);
        } else {
            d->ui->splitView->edit(index);
        }
    } else {
        d->ui->splitView->selectRow(d->ui->splitView->model()->rowCount() - 1);
    }
}

MyMoneyMoney SplitDialog::transactionAmount() const
{
    return d->transactionTotal;
}

void SplitDialog::selectionChanged()
{
    updateButtonState();
}

void SplitDialog::updateButtonState()
{
    d->ui->deleteButton->setEnabled(false);
    d->ui->deleteAllButton->setEnabled(false);
    d->ui->mergeButton->setEnabled(false);
    d->ui->deleteZeroButton->setEnabled(false);
    d->ui->adjustUnassigned->setEnabled(false);

    if (!d->readOnly) {
        if (d->ui->splitView->selectionModel()->selectedRows().count() > 0) {
            d->ui->deleteButton->setEnabled(true);
        }

        if (d->ui->splitView->model()->rowCount() > 2) {
            d->ui->deleteAllButton->setEnabled(true);
        }

        if (d->ui->splitView->selectionModel()->selectedRows().count() == 1
            && !d->ui->splitView->selectionModel()->selectedIndexes().at(0).data(eMyMoney::Model::IdRole).toString().isEmpty()) {
            if (!d->transactionTotal.isAutoCalc()) {
                d->ui->adjustUnassigned->setDisabled((d->transactionTotal.abs() - d->splitsTotal.abs()).isZero());
            }
        }

        QAbstractItemModel* model = d->ui->splitView->model();
        QSet<QString> accountIDs;
        const auto rows = model->rowCount();
        for (int row = 0; row < rows; ++row) {
            const auto idx = model->index(row, 0);
            // don't check the empty line at the end
            if (idx.data(eMyMoney::Model::IdRole).toString().isEmpty())
                continue;

            const auto accountID = idx.data(eMyMoney::Model::SplitAccountIdRole).toString();
            const auto amount = idx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>();
            if (accountIDs.contains(accountID)) {
                d->ui->mergeButton->setEnabled(true);
            }
            if (amount.isZero()) {
                d->ui->deleteZeroButton->setEnabled(true);
            }
        }
    }
}

void SplitDialog::deleteSelectedSplits()
{
    if (!d->ui->splitView->selectionModel()->selectedRows().isEmpty()) {
        const auto row = d->ui->splitView->selectionModel()->selectedRows().first().row();
        d->deleteSplits(d->ui->splitView->selectionModel()->selectedRows());
        adjustSummary();
        d->selectRow(row);
    }
}

void SplitDialog::deleteAllSplits()
{
    QAbstractItemModel* model = d->ui->splitView->model();
    QModelIndexList list = model->match(model->index(0, 0), eMyMoney::Model::IdRole, QLatin1String(".+"), -1, Qt::MatchRegularExpression);
    const auto row = d->ui->splitView->selectionModel()->selectedRows().first().row();
    d->deleteSplits(list);
    adjustSummary();
    d->selectRow(row);
}

void SplitDialog::adjustUnassigned()
{
    QModelIndex index = d->ui->splitView->currentIndex();
    if (index.isValid()) {
        // extract current values ...
        auto shares = index.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>();
        auto value = index.data(eMyMoney::Model::SplitValueRole).value<MyMoneyMoney>();
        const auto price = value / shares;
        const auto diff = d->transactionTotal - d->splitsTotal;
        // ... and adjust shares and value ...
        value += diff;
        shares = value / price;
        // ... and update the model
        auto model = d->ui->splitView->model();
        model->setData(index, QVariant::fromValue<MyMoneyMoney>(shares), eMyMoney::Model::SplitSharesRole);
        model->setData(index, QVariant::fromValue<MyMoneyMoney>(value), eMyMoney::Model::SplitValueRole);

        adjustSummary();
    }
}

void SplitDialog::deleteZeroSplits()
{
    QAbstractItemModel* model = d->ui->splitView->model();
    QModelIndexList list = model->match(model->index(0, 0), eMyMoney::Model::IdRole, QLatin1String(".+"), -1, Qt::MatchRegularExpression);

    for (int row = 0; row < list.count();) {
        const auto idx = list.at(row);
        if (!idx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>().isZero()) {
            list.removeAt(row);
        } else {
            ++row;
        }
    }
    const auto row = d->ui->splitView->selectionModel()->selectedRows().first().row();
    d->deleteSplits(list);
    adjustSummary();
    d->selectRow(row);
}

void SplitDialog::mergeSplits()
{
    auto row = d->ui->splitView->selectionModel()->selectedRows().first().row();
    qDebug() << "Merge splits not yet implemented.";
    adjustSummary();
    d->selectRow(row);
}

void SplitDialog::setTransactionPayeeId(const QString& id)
{
    d->ui->splitView->setTransactionPayeeId(id);
}

void SplitDialog::setReadOnly(bool readOnly)
{
    d->readOnly = readOnly;
    d->ui->okButton->setDisabled(readOnly);
    d->ui->newSplitButton->setDisabled(readOnly);
    d->ui->splitView->setReadOnlyMode(readOnly);
    updateButtonState();
}
