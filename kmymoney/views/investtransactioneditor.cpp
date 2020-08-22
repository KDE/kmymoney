/*
 * Copyright 2019-2020  Thomas Baumgart <tbaumgart@kde.org>
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


#include "investtransactioneditor.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QCompleter>
#include <QStringList>
#include <QDebug>
#include <QGlobalStatic>
#include <QAbstractItemView>
#include <QStringListModel>
#include <QSortFilterProxyModel>


// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KConcatenateRowsProxyModel>
#include <KDescendantsProxyModel>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_investtransactioneditor.h"
#include "creditdebithelper.h"
#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneyexception.h"
#include "kmymoneyutils.h"
#include "kmymoneyaccountcombo.h"
#include "accountsmodel.h"
#include "journalmodel.h"
#include "statusmodel.h"
#include "splitmodel.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "splitdialog.h"
#include "widgethintframe.h"
#include "icons/icons.h"
#include "modelenums.h"
#include "mymoneyenums.h"
#include "mymoneysecurity.h"
#include "kcurrencycalculator.h"
#include "investactivities.h"
#include "kmymoneysettings.h"
#include "mymoneyprice.h"
#include "amounteditcurrencyhelper.h"

using namespace Icons;

class InvestTransactionEditor::Private
{
public:
    Private(InvestTransactionEditor* parent)
        : q(parent)
        , ui(new Ui_InvestTransactionEditor)
        , accountsModel(new AccountNamesFilterProxyModel(parent))
        , feesModel(new AccountNamesFilterProxyModel(parent))
        , interestModel(new AccountNamesFilterProxyModel(parent))
        , accepted(false)
        , bypassPriceEditor(false)
        , feeSplitModel(new SplitModel(parent, &undoStack))
        , interestSplitModel(new SplitModel(parent, &undoStack))
        , activitiesModel(new QStringListModel(parent))
        , securitiesModel(new QSortFilterProxyModel(parent))
        , accountsListModel(new KDescendantsProxyModel(parent))
        , currentActivity(nullptr)
        , price(MyMoneyMoney::ONE)
    {
        accountsModel->setObjectName("InvestTransactionEditor::accountsModel");
        feeSplitModel->setObjectName("FeesSplitModel");
        interestSplitModel->setObjectName("InterestSplitModel");

        QStringList activityItems;
        activityItems << i18n("Buy shares")
                      << i18n("Sell shares")
                      << i18n("Dividend")
                      << i18n("Reinvest dividend")
                      << i18n("Yield")
                      << i18n("Add shares")
                      << i18n("Remove shares")
                      << i18n("Split shares")
                      << i18n("Interest Income");

       activitiesModel->setStringList(activityItems);
    }

    ~Private()
    {
        delete ui;
    }

    void dumpModel(const QAbstractItemModel* model)
    {
        const auto rows = model->rowCount();
        for (int row = 0; row < rows; ++row) {
            const auto idx = model->index(row, 0);
            qDebug() << row << idx.data(eMyMoney::Model::IdRole).toString() << idx.data(eMyMoney::Model::AccountFullNameRole).toString();
        }
    }
    void createStatusEntry(eMyMoney::Split::State status);
    bool checkForValidTransaction(bool doUserInteraction = true);
    bool isDatePostOpeningDate(const QDate& date, const QString& accountId);

    bool postdateChanged(const QDate& date);
    bool securityChanged(const QString& securityId);
    bool categoryChanged(SplitModel* model, const QString& accountId, AmountEdit* widget, const MyMoneyMoney& factor);

    bool valueChanged(const SplitModel* model, AmountEdit* widget);

    void updateWidgetState();

    MyMoneyMoney getPrice(const SplitModel* model, const AmountEdit* widget);

    void editSplits(SplitModel* splitModel, AmountEdit* editWidget, const MyMoneyMoney& factor);
    void removeUnusedSplits(MyMoneyTransaction& t, SplitModel* splitModel);
    void addSplits(MyMoneyTransaction& t, SplitModel* splitModel);
    void setupAccount(const QString& accountId);
    QModelIndex adjustToSecuritySplitIdx(const QModelIndex& idx);

    MyMoneyMoney splitValue(SplitModel* model) const;

    InvestTransactionEditor*      q;
    Ui_InvestTransactionEditor*   ui;
    AccountNamesFilterProxyModel* accountsModel;
    AccountNamesFilterProxyModel* feesModel;
    AccountNamesFilterProxyModel* interestModel;
    bool                          accepted;
    bool                          bypassPriceEditor;
    eMyMoney::Split::InvestmentTransactionType m_transactionType;
    QUndoStack                    undoStack;
    SplitModel*                   feeSplitModel;
    SplitModel*                   interestSplitModel;
    QStringListModel*             activitiesModel;
    QSortFilterProxyModel*        securitiesModel;
    KDescendantsProxyModel*       accountsListModel;
    Invest::Activity*             currentActivity;
    MyMoneySecurity               security;
    MyMoneySecurity               currency;
    MyMoneyAccount                m_account;
    MyMoneyTransaction            transaction;
    MyMoneySplit                  split;
    MyMoneyMoney                  price;
    QSet<AmountEditCurrencyHelper*> amountEditCurrencyHelpers;
};

void InvestTransactionEditor::Private::removeUnusedSplits(MyMoneyTransaction& t, SplitModel* splitModel)
{
    for (const auto& sp : t.splits()) {
        if (sp.id() == split.id()) {
            continue;
        }
        const auto rows = splitModel->rowCount();
        int row;
        for (row = 0; row < rows; ++row) {
            const QModelIndex index = splitModel->index(row, 0);
            if (index.data(eMyMoney::Model::IdRole).toString() == sp.id()) {
                break;
            }
        }

        // if the split is not in the model, we get rid of it
        if (splitModel->rowCount() == row) {
            t.removeSplit(sp);
        }
    }
}

void InvestTransactionEditor::Private::addSplits(MyMoneyTransaction& t, SplitModel* splitModel)
{
    for (int row = 0; row < splitModel->rowCount(); ++row) {
        const QModelIndex idx = splitModel->index(row, 0);
        MyMoneySplit s;
        const QString splitId = idx.data(eMyMoney::Model::IdRole).toString();
        // Extract the split from the transaction if
        // it already exists. Otherwise it remains
        // an empty split and will be added later.
        try {
            s = t.splitById(splitId);
        } catch(const MyMoneyException&) {
        }
        s.setMemo(idx.data(eMyMoney::Model::SplitMemoRole).toString());
        s.setAccountId(idx.data(eMyMoney::Model::SplitAccountIdRole).toString());
        s.setShares(idx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>());
        s.setValue(idx.data(eMyMoney::Model::SplitValueRole).value<MyMoneyMoney>());

        if (s.id().isEmpty() || splitModel->isNewSplitId(s.id())) {
            s.clearId();
            t.addSplit(s);
        } else {
            t.modifySplit(s);
        }
    }
}

bool InvestTransactionEditor::Private::checkForValidTransaction(bool doUserInteraction)
{
    QStringList infos;
    bool rc = true;
    if (!postdateChanged(ui->dateEdit->date())) {
        infos << ui->dateEdit->toolTip();
        rc = false;
    } else {
        QString info;
        if (!currentActivity->isComplete(info)) {
            infos << info;
            rc = false;
        }
    }


    if (doUserInteraction && (rc == false)) {
    }
    return rc;
}

bool InvestTransactionEditor::Private::isDatePostOpeningDate(const QDate& date, const QString& accountId)
{
    bool rc = true;

    try {
        MyMoneyAccount account = MyMoneyFile::instance()->account(accountId);

        // we don't check for categories
        if (!account.isIncomeExpense()) {
            if (date < account.openingDate())
                rc = false;
        }
    } catch (MyMoneyException& e) {
        qDebug() << "Ooops: invalid account id" << accountId << "in" << Q_FUNC_INFO;
    }
    return rc;
}

bool InvestTransactionEditor::Private::postdateChanged(const QDate& date)
{
    bool rc = true;
    WidgetHintFrame::hide(ui->dateEdit, i18n("The posting date of the transaction."));

    // collect all account ids
    QStringList accountIds;
    accountIds << m_account.id();
    const auto rows = feeSplitModel->rowCount();
    for (int row = 0; row < rows; ++row) {
        const auto index = feeSplitModel->index(row, 0);
        accountIds << index.data(eMyMoney::Model::SplitAccountIdRole).toString();
    }

    for (const auto& accountId : accountIds) {
        if (!isDatePostOpeningDate(date, accountId)) {
            MyMoneyAccount account = MyMoneyFile::instance()->account(accountId);
            WidgetHintFrame::show(ui->dateEdit, i18n("The posting date is prior to the opening date of account <b>%1</b>.", account.name()));
            rc = false;
            break;
        }
    }
    return rc;
}

bool InvestTransactionEditor::Private::categoryChanged(SplitModel* model, const QString& accountId, AmountEdit* widget, const MyMoneyMoney& factor)
{
    bool rc = true;
    if (!accountId.isEmpty() && model->rowCount() <= 1) {
        try {
            MyMoneyAccount category = MyMoneyFile::instance()->account(accountId);
            const bool isIncomeExpense = category.isIncomeExpense();

            bool needValueSet = false;
            // make sure we have a split in the model
            if (model->rowCount() == 0) {
                // add an empty split
                MyMoneySplit s;
                model->addItem(s);
                needValueSet = true;
            }

            const QModelIndex index = model->index(0, 0);
            if (!needValueSet) {
                // update the values only if the category changes. This prevents
                // the call of the currency calculator if not needed.
                needValueSet = (index.data(eMyMoney::Model::SplitAccountIdRole).toString().compare(accountId) != 0);
            }
            model->setData(index, accountId, eMyMoney::Model::SplitAccountIdRole);

            if (!widget->value().isZero() && needValueSet) {
                model->setData(index, QVariant::fromValue<MyMoneyMoney>(factor * widget->value() * getPrice(model, widget)), eMyMoney::Model::SplitValueRole);
                model->setData(index, QVariant::fromValue<MyMoneyMoney>(factor * widget->value()), eMyMoney::Model::SplitSharesRole);
            }

        } catch (MyMoneyException& e) {
            qDebug() << "Ooops: invalid account id" << accountId << "in" << Q_FUNC_INFO;
        }
    }
    return rc;
}

bool InvestTransactionEditor::Private::securityChanged(const QString& securityId)
{
    auto sec = MyMoneyFile::instance()->security(securityId);
    if (!sec.id().isEmpty()) {
        security = sec;
        ui->sharesAmountEdit->setPrecision(MyMoneyMoney::denomToPrec(security.smallestAccountFraction()));
        return true;
    }
    return false;
}

MyMoneyMoney InvestTransactionEditor::Private::getPrice(const SplitModel* model, const AmountEdit* widget)
{
    auto result(MyMoneyMoney::ONE);
    const QModelIndex splitIdx = model->index(0, 0);
    if (splitIdx.isValid()) {
        const auto shares = splitIdx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>();
        const auto value = splitIdx.data(eMyMoney::Model::SplitValueRole).value<MyMoneyMoney>();
        if (!shares.isZero()) {
            result = value / shares;
        }
    }

    if (!bypassPriceEditor && splitIdx.isValid()) {
        const auto categoryId = splitIdx.data(eMyMoney::Model::SplitAccountIdRole).toString();
        const auto category = MyMoneyFile::instance()->accountsModel()->itemById(categoryId);
        if (!category.id().isEmpty()) {
            const auto sec = MyMoneyFile::instance()->security(category.currencyId());
            if (sec.id() != transaction.commodity()) {
                const auto commodity = MyMoneyFile::instance()->security(transaction.commodity());
                if (result == MyMoneyMoney::ONE) {
                    result = MyMoneyFile::instance()->price(sec.id(), commodity.id(), QDate()).rate(sec.id());
                }

                QPointer<KCurrencyCalculator> calc =
                    new KCurrencyCalculator(sec,
                                            commodity,
                                            widget->value(),
                                            widget->value() / result,
                                            ui->dateEdit->date(),
                                            sec.smallestAccountFraction(),
                                            q);

                if (calc->exec() == QDialog::Accepted && calc) {
                    result = calc->price();
                }
                delete calc;

            } else {
                result = MyMoneyMoney::ONE;
            }
            // keep for next round
            price = result;
        }
    }
    return result;
}


bool InvestTransactionEditor::Private::valueChanged(const SplitModel* model, AmountEdit* widget)
{
    bool rc = true;
#if 0
    if (valueHelper->haveValue() && (feeSplitModel.rowCount() <= 1) && (amountHelper->value() != split.value())) {
        rc = false;
        try {
            MyMoneyMoney shares;
            if (feeSplitModel.rowCount() == 1) {
                const QModelIndex index = feeSplitModel.index(0, 0);
                feeSplitModel.setData(index, QVariant::fromValue<MyMoneyMoney>(-amountHelper->value()), eMyMoney::Model::SplitValueRole);
                feeSplitModel.setData(index, QVariant::fromValue<MyMoneyMoney>(-amountHelper->value() / getPrice(model)), eMyMoney::Model::SplitSharesRole);
            }
            rc = true;

        } catch (MyMoneyException& e) {
            qDebug() << "Ooops: something went wrong in" << Q_FUNC_INFO;
        }
    } else {
        /// @todo ask what to do: if the rest of the splits is the same amount we could simply reverse the sign
        /// of all splits, otherwise we could ask if the user wants to start the split editor or anything else.
    }
#endif
    return rc;
}

void InvestTransactionEditor::Private::editSplits(SplitModel* sourceSplitModel, AmountEdit* editWidget, const MyMoneyMoney& transactionFactor)
{
    SplitModel splitModel(q, nullptr, *sourceSplitModel);

    // create an empty split at the end
    // used to create new splits
    splitModel.appendEmptySplit();

    auto commodityId = transaction.commodity();
    if (commodityId.isEmpty())
        commodityId = m_account.currencyId();
    const auto commodity = MyMoneyFile::instance()->security(commodityId);

    QPointer<SplitDialog> splitDialog = new SplitDialog(m_account, commodity, MyMoneyMoney::autoCalc, transactionFactor, q);
    splitDialog->setModel(&splitModel);

    int rc = splitDialog->exec();

    if (splitDialog && (rc == QDialog::Accepted)) {
        // remove that empty split again before we update the splits
        splitModel.removeEmptySplit();

        // copy the splits model contents
        *sourceSplitModel = splitModel;

        // update the transaction amount
        editWidget->setValue(splitDialog->transactionAmount() * transactionFactor);

        // the price might have been changed, so we have to update our copy
        // but only if there is one counter split
        if (sourceSplitModel->rowCount() == 1) {
            const auto splitIdx = sourceSplitModel->index(0, 0);
            const auto shares = splitIdx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>();
            const auto value = splitIdx.data(eMyMoney::Model::SplitValueRole).value<MyMoneyMoney>();
            if (!shares.isZero()) {
                price = value / shares;
            }
            // make sure to show the value in the widget
            // according to the currency presented
            editWidget->setValue(shares * transactionFactor);
        }

        // bypass the currency calculator here, we have all info already
        bypassPriceEditor = true;
        updateWidgetState();
        bypassPriceEditor = false;
    }

    if (splitDialog) {
        splitDialog->deleteLater();
    }
}

MyMoneyMoney InvestTransactionEditor::Private::splitValue(SplitModel* model) const
{
    const auto rows = model->rowCount();
    MyMoneyMoney sum;
    for (int row = 0; row < rows; ++row) {
        const auto idx = model->index(row, 0);
        sum += idx.data(eMyMoney::Model::SplitValueRole).value<MyMoneyMoney>();
    }
    return sum;
}

void InvestTransactionEditor::Private::setupAccount(const QString& accountId)
{
    auto const file = MyMoneyFile::instance();
    auto const model = file->accountsModel();

    // extract account information from model
    const auto index = model->indexById(accountId);
    m_account = model->itemByIndex(index);

    securitiesModel->setFilterFixedString(accountId);
}

QModelIndex InvestTransactionEditor::Private::adjustToSecuritySplitIdx(const QModelIndex& index)
{
    if (!index.isValid()) {
        return {};
    }
    const auto first = MyMoneyFile::instance()->journalModel()->adjustToFirstSplitIdx(index);
    const auto id = first.data(eMyMoney::Model::IdRole).toString();

    const auto rows = first.data(eMyMoney::Model::TransactionSplitCountRole).toInt();
    const auto endRow = first.row() + rows;
    for(int row = first.row(); row < endRow; ++row) {
        const auto idx = index.model()->index(row, 0);
        const auto accountId = idx.data(eMyMoney::Model::SplitAccountIdRole).toString();
        const auto account = MyMoneyFile::instance()->accountsModel()->itemById(accountId);
        if (account.isInvest()) {
            return idx;
        }
    }
    return {};
}

void InvestTransactionEditor::Private::updateWidgetState()
{
    WidgetHintFrame::hide(ui->feesCombo, i18nc("@info:tooltip", "Category for fees"));
    WidgetHintFrame::hide(ui->feesAmountEdit, i18nc("@info:tooltip", "Amount of fees"));
    WidgetHintFrame::hide(ui->interestCombo, i18nc("@info:tooltip", "Category for interest"));
    WidgetHintFrame::hide(ui->interestAmountEdit, i18nc("@info:tooltip", "Amount of interest"));
    WidgetHintFrame::hide(ui->accountCombo, i18nc("@info:tooltip", "Asset or brokerage account"));
    WidgetHintFrame::hide(ui->priceAmountEdit, i18nc("@info:tooltip", "Price information for this transaction"));

    const auto widget = ui->sharesAmountEdit;
    switch(currentActivity->type()) {
        default:
            WidgetHintFrame::hide(widget, i18nc("@info:tooltip", "Number of shares"));
            if (widget->isVisible()) {
                if (widget->value().isZero()) {
                    WidgetHintFrame::show(widget, i18nc("@info:tooltip", "Enter number of shares for this transaction"));
                }
            }
            break;
        case eMyMoney::Split::InvestmentTransactionType::SplitShares:
            WidgetHintFrame::hide(widget, i18nc("@info:tooltip", "Split ratio"));
            if (widget->isVisible()) {
                if (widget->value().isZero()) {
                    WidgetHintFrame::show(widget, i18nc("@info:tooltip", "Enter the split ratio for this transaction"));
                }
            }
            break;
    }

    switch(currentActivity->priceRequired()) {
        case Invest::Activity::Unused:
            break;
        case Invest::Activity::Optional:
        case Invest::Activity::Mandatory:
            if (ui->priceAmountEdit->value().isZero()) {
                WidgetHintFrame::show(ui->priceAmountEdit, i18nc("@info:tooltip", "Enter price information for this transaction"));
            }
            break;
    }


    switch(currentActivity->assetAccountRequired()) {
        case Invest::Activity::Unused:
            break;
        case Invest::Activity::Optional:
        case Invest::Activity::Mandatory:
            if (ui->accountCombo->currentText().isEmpty()) {
                WidgetHintFrame::show(ui->accountCombo, i18nc("@info:tooltip", "Select account to balance the transaction"));
            }
            break;
    }

    if (!currentActivity->haveFees(currentActivity->feesRequired())) {
        if (ui->feesCombo->currentText().isEmpty()) {
            WidgetHintFrame::show(ui->feesCombo, i18nc("@info:tooltip", "Enter category for fees"));
        }
        if (ui->feesAmountEdit->value().isZero()) {
            WidgetHintFrame::show(ui->feesAmountEdit, i18nc("@info:tooltip", "Enter amout of fees"));
        }
    }

    if (!currentActivity->haveInterest(currentActivity->interestRequired())) {
        if (ui->interestCombo->currentText().isEmpty()) {
            WidgetHintFrame::show(ui->interestCombo, i18nc("@info:tooltip", "Enter category for interest"));
        }
        if (ui->interestAmountEdit->value().isZero()) {
            WidgetHintFrame::show(ui->interestAmountEdit, i18nc("@info:tooltip", "Enter amout of interest"));
        }
    }
}

InvestTransactionEditor::InvestTransactionEditor(QWidget* parent, const QString& accountId)
    : TransactionEditorBase(parent, accountId)
    , d(new Private(this))
{
    d->ui->setupUi(this);

    // initially, the info message is hidden
    d->ui->infoMessage->hide();

    d->ui->activityCombo->setModel(d->activitiesModel);

    auto const model = MyMoneyFile::instance()->accountsModel();
    d->accountsListModel->setSourceModel(model);
    d->securitiesModel->setSourceModel(d->accountsListModel);
    d->securitiesModel->setFilterRole(eMyMoney::Model::AccountParentIdRole);
    d->securitiesModel->setFilterKeyColumn(0);
    d->ui->securityCombo->setModel(d->securitiesModel);
    d->ui->securityCombo->lineEdit()->setReadOnly(true);

    d->accountsModel->addAccountGroup(QVector<eMyMoney::Account::Type> { eMyMoney::Account::Type::Asset, eMyMoney::Account::Type::Liability } );
    d->accountsModel->setHideEquityAccounts(false);
    d->accountsModel->setSourceModel(model);
    d->accountsModel->sort(AccountsModel::Column::AccountName);
    d->ui->accountCombo->setModel(d->accountsModel);
    d->ui->accountCombo->setSplitActionVisible(false);

    d->feesModel->addAccountGroup(QVector<eMyMoney::Account::Type> { eMyMoney::Account::Type::Expense });
    d->feesModel->setSourceModel(model);
    d->feesModel->sort(AccountsModel::Column::AccountName);
    d->ui->feesCombo->setModel(d->feesModel);
    auto helper = new KMyMoneyAccountComboSplitHelper(d->ui->feesCombo, d->feeSplitModel);
    connect(helper, &KMyMoneyAccountComboSplitHelper::accountComboDisabled, d->ui->feesAmountEdit, &AmountEdit::setReadOnly);

    d->interestModel->addAccountGroup(QVector<eMyMoney::Account::Type> { eMyMoney::Account::Type::Income });
    d->interestModel->setSourceModel(model);
    d->interestModel->sort(AccountsModel::Column::AccountName);
    d->ui->interestCombo->setModel(d->interestModel);
    helper = new KMyMoneyAccountComboSplitHelper(d->ui->interestCombo, d->interestSplitModel);
    connect(helper, &KMyMoneyAccountComboSplitHelper::accountComboDisabled, d->ui->interestAmountEdit, &AmountEdit::setReadOnly);

    d->ui->enterButton->setIcon(Icons::get(Icon::DialogOK));
    d->ui->cancelButton->setIcon(Icons::get(Icon::DialogCancel));

    d->ui->statusCombo->setModel(MyMoneyFile::instance()->statusModel());

    d->ui->dateEdit->setDisplayFormat(QLocale().dateFormat(QLocale::ShortFormat));

    d->ui->sharesAmountEdit->setAllowEmpty(true);
    d->ui->sharesAmountEdit->setCalculatorButtonVisible(true);
    connect(d->ui->sharesAmountEdit, &AmountEdit::textChanged, this, &InvestTransactionEditor::updateTotalAmount);


    d->ui->priceAmountEdit->setAllowEmpty(true);
    d->ui->priceAmountEdit->setCalculatorButtonVisible(true);
    connect(d->ui->priceAmountEdit, &AmountEdit::textChanged, this, &InvestTransactionEditor::updateTotalAmount);

    d->ui->feesAmountEdit->setAllowEmpty(true);
    d->ui->feesAmountEdit->setCalculatorButtonVisible(true);
    connect(d->ui->feesAmountEdit, &AmountEdit::textChanged, this, &InvestTransactionEditor::updateTotalAmount);

    d->ui->interestAmountEdit->setAllowEmpty(true);
    d->ui->interestAmountEdit->setCalculatorButtonVisible(true);
    connect(d->ui->interestAmountEdit, &AmountEdit::textChanged, this, &InvestTransactionEditor::updateTotalAmount);

    WidgetHintFrameCollection* frameCollection = new WidgetHintFrameCollection(this);
    frameCollection->addFrame(new WidgetHintFrame(d->ui->dateEdit));
    frameCollection->addFrame(new WidgetHintFrame(d->ui->accountCombo));
    frameCollection->addFrame(new WidgetHintFrame(d->ui->sharesAmountEdit));
    frameCollection->addFrame(new WidgetHintFrame(d->ui->priceAmountEdit));
    frameCollection->addFrame(new WidgetHintFrame(d->ui->feesCombo));
    frameCollection->addFrame(new WidgetHintFrame(d->ui->feesAmountEdit));
    frameCollection->addFrame(new WidgetHintFrame(d->ui->interestCombo));
    frameCollection->addFrame(new WidgetHintFrame(d->ui->interestAmountEdit));
    frameCollection->addWidget(d->ui->enterButton);


    connect(d->ui->accountCombo, &KMyMoneyAccountCombo::accountSelected, this, &InvestTransactionEditor::accountChanged);
    connect(d->ui->dateEdit, &KMyMoneyDateEdit::dateChanged, this, &InvestTransactionEditor::postdateChanged);
    connect(d->ui->activityCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &InvestTransactionEditor::activityChanged);
    connect(d->ui->securityCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &InvestTransactionEditor::securityChanged);

    connect(d->ui->feesCombo, &KMyMoneyAccountCombo::accountSelected, this, &InvestTransactionEditor::feeCategoryChanged);
    connect(d->ui->feesCombo, &KMyMoneyAccountCombo::splitDialogRequest, this, &InvestTransactionEditor::editFeeSplits, Qt::QueuedConnection);

    connect(d->ui->interestCombo, &KMyMoneyAccountCombo::accountSelected, this, &InvestTransactionEditor::interestCategoryChanged);
    connect(d->ui->interestCombo, &KMyMoneyAccountCombo::splitDialogRequest, this, &InvestTransactionEditor::editInterestSplits, Qt::QueuedConnection);

    /// @todo convert to new signal/slot syntax
    connect(d->ui->cancelButton, SIGNAL(clicked(bool)), this, SLOT(reject()));
    connect(d->ui->enterButton, SIGNAL(clicked(bool)), this, SLOT(acceptEdit()));

    // handle some events in certain conditions different from default
    d->ui->activityCombo->installEventFilter(this);
    d->ui->statusCombo->installEventFilter(this);

    d->ui->totalAmountEdit->setCalculatorButtonVisible(false);

    d->setupAccount(accountId);

    d->amountEditCurrencyHelpers.insert(new AmountEditCurrencyHelper(d->ui->feesCombo, d->ui->feesAmountEdit, d->transaction.commodity()));
    d->amountEditCurrencyHelpers.insert(new AmountEditCurrencyHelper(d->ui->interestCombo, d->ui->interestAmountEdit, d->transaction.commodity()));

    // setWindowFlags(Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
}

InvestTransactionEditor::~InvestTransactionEditor()
{
}

bool InvestTransactionEditor::accepted() const
{
    return d->accepted;
}

void InvestTransactionEditor::acceptEdit()
{
    if (d->checkForValidTransaction()) {
        d->accepted = true;
        emit done();
    }
}

void InvestTransactionEditor::reject()
{
    emit done();
}

void InvestTransactionEditor::keyPressEvent(QKeyEvent* e)
{
    if (!e->modifiers() || (e->modifiers() & Qt::KeypadModifier && e->key() == Qt::Key_Enter)) {
        switch (e->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return: {
            if (focusWidget() == d->ui->cancelButton) {
                reject();
            } else {
                if (d->ui->enterButton->isEnabled()) {
                    // move focus to enter button which
                    // triggers update of widgets
                    d->ui->enterButton->setFocus();
                    d->ui->enterButton->click();
                }
                return;
            }
        }
        break;

        case Qt::Key_Escape:
            reject();
            break;

        default:
            e->ignore();
            return;
        }
    } else {
        e->ignore();
    }
}

void InvestTransactionEditor::updateTotalAmount(const QString& txt)
{
    Q_UNUSED(txt)
    d->updateWidgetState();
    if (d->currentActivity) {
        d->ui->totalAmountEdit->setValue(d->currentActivity->totalAmount());
    }
}


void InvestTransactionEditor::loadTransaction(const QModelIndex& index)
{
    d->ui->activityCombo->setCurrentIndex(-1);
    d->ui->securityCombo->setCurrentIndex(-1);
    auto idx = d->adjustToSecuritySplitIdx(MyMoneyFile::baseModel()->mapToBaseSource(index));
    if (!idx.isValid() || idx.data(eMyMoney::Model::IdRole).toString().isEmpty()) {
        d->transaction = MyMoneyTransaction();
        d->transaction.setCommodity(d->m_account.currencyId());
        d->split = MyMoneySplit();
        d->ui->activityCombo->setCurrentIndex(0);
        d->ui->securityCombo->setCurrentIndex(0);
        const auto lastUsedPostDate = KMyMoneySettings::lastUsedPostDate();
        if (lastUsedPostDate.isValid()) {
            d->ui->dateEdit->setDate(lastUsedPostDate.date());
        } else {
            d->ui->dateEdit->setDate(QDate::currentDate());
        }
        // select the associated brokerage account if it exists
        const auto& brokerageAccount = MyMoneyFile::instance()->accountsModel()->itemByName(d->m_account.brokerageName());
        if (!brokerageAccount.id().isEmpty()) {
            d->ui->accountCombo->setSelected(brokerageAccount.id());
        }
    } else {
        // keep a copy of the transaction and split
        d->transaction = MyMoneyFile::instance()->journalModel()->itemByIndex(idx).transaction();
        d->split = MyMoneyFile::instance()->journalModel()->itemByIndex(idx).split();

        QModelIndex assetAccountSplitIdx;
        KMyMoneyUtils::dissectInvestmentTransaction(idx, assetAccountSplitIdx, d->feeSplitModel, d->interestSplitModel, d->security, d->currency, d->m_transactionType);

        // load the widgets. setting activityCombo also initializes
        // d->currentActivity to have the right object
        d->ui->activityCombo->setCurrentIndex(static_cast<int>(d->m_transactionType));
        d->ui->dateEdit->setDate(d->transaction.postDate());

        d->ui->memoEdit->setPlainText(d->split.memo());

        d->currentActivity->memoText() = d->split.memo();
        d->currentActivity->memoChanged() = false;

        d->ui->accountCombo->setSelected(assetAccountSplitIdx.data(eMyMoney::Model::SplitAccountIdRole).toString());

        d->ui->sharesAmountEdit->setPrecision(MyMoneyMoney::denomToPrec(d->security.smallestAccountFraction()));
        d->ui->sharesAmountEdit->setValue(d->split.shares() * d->currentActivity->sharesFactor());

        const auto indexes = d->securitiesModel->match(d->securitiesModel->index(0,0), eMyMoney::Model::IdRole, d->split.accountId(), 1, Qt::MatchFixedString);
        if (!indexes.isEmpty()) {
            d->ui->securityCombo->setCurrentIndex(indexes.first().row());
        }

        d->ui->feesAmountEdit->setValue(d->splitValue(d->feeSplitModel) * d->currentActivity->feesFactor());
        d->ui->interestAmountEdit->setValue(d->splitValue(d->interestSplitModel) * d->currentActivity->interestFactor());

        d->currentActivity->loadPriceWidget(d->split);
    }

    for (const auto helper : qAsConst(d->amountEditCurrencyHelpers)) {
        helper->setCommodity(d->transaction.commodity());
    }

    // delay update until next run of event loop so that all necessary widgets are visible
    QMetaObject::invokeMethod(this, "updateWidgets", Qt::QueuedConnection);

    // set focus to date edit once we return to event loop
    QMetaObject::invokeMethod(d->ui->dateEdit, "setFocus", Qt::QueuedConnection);
}

void InvestTransactionEditor::updateWidgets()
{
    d->updateWidgetState();
}

void InvestTransactionEditor::securityChanged(int index)
{
    const auto idx = d->ui->securityCombo->model()->index(index, 0);
    if (idx.isValid()) {
        d->securityChanged(idx.data(eMyMoney::Model::AccountCurrencyIdRole).toString());
    }
}


void InvestTransactionEditor::activityChanged(int index)
{
    const auto type = static_cast<eMyMoney::Split::InvestmentTransactionType>(index);
    if (!d->currentActivity || type != d->currentActivity->type()) {
        delete d->currentActivity;
        switch(type) {
            default:
            case eMyMoney::Split::InvestmentTransactionType::BuyShares:
                d->currentActivity = new Invest::Buy(this);
                break;
            case eMyMoney::Split::InvestmentTransactionType::SellShares:
                d->currentActivity = new Invest::Sell(this);
                break;
            case eMyMoney::Split::InvestmentTransactionType::Dividend:
            case eMyMoney::Split::InvestmentTransactionType::Yield:
                d->currentActivity = new Invest::Div(this);
                break;
            case eMyMoney::Split::InvestmentTransactionType::ReinvestDividend:
                d->currentActivity = new Invest::Reinvest(this);
                break;
            case eMyMoney::Split::InvestmentTransactionType::AddShares:
                d->currentActivity = new Invest::Add(this);
                break;
            case eMyMoney::Split::InvestmentTransactionType::RemoveShares:
                d->currentActivity = new Invest::Remove(this);
                break;
            case eMyMoney::Split::InvestmentTransactionType::SplitShares:
                d->currentActivity = new Invest::Split(this);
                break;
            case eMyMoney::Split::InvestmentTransactionType::InterestIncome:
                d->currentActivity = new Invest::IntInc(this);
                break;
        }
        d->currentActivity->showWidgets();
        d->ui->totalAmountEdit->setValue(d->currentActivity->totalAmount());
        emit editorLayoutChanged();
        d->updateWidgetState();
    }
}

void InvestTransactionEditor::accountChanged(const QString& accountId)
{
    d->updateWidgetState();
}

void InvestTransactionEditor::feeCategoryChanged(const QString& accountId)
{
    d->categoryChanged(d->feeSplitModel, accountId, d->ui->feesAmountEdit, MyMoneyMoney::ONE);
    d->updateWidgetState();
}

void InvestTransactionEditor::interestCategoryChanged(const QString& accountId)
{
    d->categoryChanged(d->interestSplitModel, accountId, d->ui->interestAmountEdit, MyMoneyMoney::MINUS_ONE);
    d->updateWidgetState();
}

void InvestTransactionEditor::postdateChanged(const QDate& date)
{
    d->postdateChanged(date);
}

void InvestTransactionEditor::feesValueChanged()
{
    d->valueChanged(d->feeSplitModel, d->ui->feesAmountEdit);
    d->updateWidgetState();
}

void InvestTransactionEditor::interestValueChanged()
{
    d->valueChanged(d->interestSplitModel, d->ui->interestAmountEdit);
    d->updateWidgetState();
}

void InvestTransactionEditor::editFeeSplits()
{
    d->editSplits(d->feeSplitModel, d->ui->feesAmountEdit, MyMoneyMoney::ONE);
#if 0
    QWidget* next = d->ui->tagComboBox;
    next->setFocus();
#endif
}

void InvestTransactionEditor::editInterestSplits()
{
    d->editSplits(d->interestSplitModel, d->ui->interestAmountEdit, MyMoneyMoney::MINUS_ONE);

#if 0
    QWidget* next = d->ui->tagComboBox;
    next->setFocus();
#endif
}

MyMoneyMoney InvestTransactionEditor::transactionAmount() const
{
#if 0
    return d->amountHelper->value();
#endif
    return {};
}

MyMoneyMoney InvestTransactionEditor::totalAmount() const
{
    MyMoneyMoney sum;
    sum += d->splitValue(d->feeSplitModel);
    sum -= d->splitValue(d->interestSplitModel);

    return {};
}

void InvestTransactionEditor::saveTransaction()
{
#if 0
    MyMoneyTransaction t;

    if (!d->transaction.id().isEmpty()) {
        t = d->transaction;
    } else {
        // we keep the date when adding a new transaction
        // for the next new one
        KMyMoneySettings::setLastUsedPostDate(QDateTime(d->ui->dateEdit->date()));
    }

    d->removeUnusedSplits(t, d->feeSplitModel);
    d->removeUnusedSplits(t, d->interestSplitModel);

    MyMoneyFileTransaction ft;
    try {
        // new we update the split we are opened for
        MyMoneySplit sp(d->split);
        sp.setMemo(d->ui->memoEdit->toPlainText());

        if (sp.reconcileFlag() != eMyMoney::Split::State::Reconciled
                && !sp.reconcileDate().isValid()
                && d->ui->statusCombo->currentIndex() == (int)eMyMoney::Split::State::Reconciled) {
            sp.setReconcileDate(QDate::currentDate());
        }
        sp.setReconcileFlag(static_cast<eMyMoney::Split::State>(d->ui->statusCombo->currentIndex()));

        if (sp.id().isEmpty()) {
            t.addSplit(sp);
        } else {
            t.modifySplit(sp);
        }
        t.setPostDate(d->ui->dateEdit->date());

        // now update and add what we have in the model
        d->addSplits(t, d->feeSplitModel);
        const SplitModel* model = d->feeSplitModel;
        for (int row = 0; row < model->rowCount(); ++row) {
            const QModelIndex idx = model->index(row, 0);
            MyMoneySplit s;
            const QString splitId = idx.data(eMyMoney::Model::IdRole).toString();
            // Extract the split from the transaction if
            // it already exists. Otherwise it remains
            // an empty split and will be added later.
            try {
                s = t.splitById(splitId);
            } catch(const MyMoneyException&) {
            }
            s.setMemo(idx.data(eMyMoney::Model::SplitMemoRole).toString());
            s.setAccountId(idx.data(eMyMoney::Model::SplitAccountIdRole).toString());
            s.setShares(idx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>());
            s.setValue(idx.data(eMyMoney::Model::SplitValueRole).value<MyMoneyMoney>());

            if (s.id().isEmpty()) {
                t.addSplit(s);
            } else {
                t.modifySplit(s);
            }
        }

        if (t.id().isEmpty()) {
            MyMoneyFile::instance()->addTransaction(t);
        } else {
            MyMoneyFile::instance()->modifyTransaction(t);
        }
        ft.commit();

    } catch (const MyMoneyException& e) {
        qDebug() << Q_FUNC_INFO << "something went wrong" << e.what();
    }
#endif
}

bool InvestTransactionEditor::eventFilter(QObject* o, QEvent* e)
{
    auto cb = qobject_cast<QComboBox*>(o);
    if (cb) {
        // filter out wheel events for combo boxes if the popup view is not visible
        if ((e->type() == QEvent::Wheel) && !cb->view()->isVisible()) {
            return true;
        }
    }
    return QFrame::eventFilter(o, e);
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs on; remove-trailing-space on;remove-trailing-space-save on;
