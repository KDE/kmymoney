/*
    SPDX-FileCopyrightText: 2019-2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "investtransactioneditor.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QAbstractItemView>
#include <QCompleter>
#include <QDebug>
#include <QGlobalStatic>
#include <QSortFilterProxyModel>
#include <QStringList>
#include <QStringListModel>
#include <QTreeView>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KDescendantsProxyModel>

// ----------------------------------------------------------------------------
// Project Includes

#include "accountcreator.h"
#include "accountsmodel.h"
#include "dialogenums.h"
#include "icons.h"
#include "investactivities.h"
#include "journalmodel.h"
#include "kcurrencycalculator.h"
#include "kmymoneyaccountcombo.h"
#include "kmymoneysettings.h"
#include "kmymoneyutils.h"
#include "mymoneyaccount.h"
#include "mymoneyenums.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneyprice.h"
#include "mymoneysecurity.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "securitiesmodel.h"
#include "splitdialog.h"
#include "splitmodel.h"
#include "statusmodel.h"
#include "ui_investtransactioneditor.h"
#include "widgethintframe.h"

using namespace Icons;

class InvestTransactionEditor::Private
{
public:
    Private(InvestTransactionEditor* parent)
        : q(parent)
        , ui(new Ui_InvestTransactionEditor)
        , tabOrderUi(nullptr)
        , accountsModel(new AccountNamesFilterProxyModel(parent))
        , feesModel(new AccountNamesFilterProxyModel(parent))
        , interestModel(new AccountNamesFilterProxyModel(parent))
        , activitiesModel(new QStringListModel(parent))
        , securitiesModel(new QSortFilterProxyModel(parent))
        , securityFilterModel(new AccountsProxyModel(parent))
        , accountsListModel(new KDescendantsProxyModel(parent))
        , currentActivity(nullptr)
        , feeSplitModel(new SplitModel(parent, &undoStack))
        , interestSplitModel(new SplitModel(parent, &undoStack))
        , accepted(false)
        , bypassUserPriceUpdate(false)
    {
        accountsModel->setObjectName("InvestTransactionEditor::accountsModel");
        feeSplitModel->setObjectName("FeesSplitModel");
        interestSplitModel->setObjectName("InterestSplitModel");

        // keep in sync with eMyMoney::Split::InvestmentTransactionType
        QStringList activityItems{
            i18nc("@item:inlistbox transaction type", "Buy shares"),
            i18nc("@item:inlistbox transaction type", "Sell shares"),
            i18nc("@item:inlistbox transaction type", "Dividend"),
            i18nc("@item:inlistbox transaction type", "Reinvest dividend"),
            i18nc("@item:inlistbox transaction type", "Yield"),
            i18nc("@item:inlistbox transaction type", "Add shares"),
            i18nc("@item:inlistbox transaction type", "Remove shares"),
            i18nc("@item:inlistbox transaction type", "Split shares"),
            i18nc("@item:inlistbox transaction type", "Interest Income"),
        };

        activitiesModel->setStringList(activityItems);
    }

    ~Private()
    {
        delete ui;
    }

    void dumpSplitModel(const QString& header, const QAbstractItemModel* model)
    {
        const auto rows = model->rowCount();
        qDebug() << header;
        for (int row = 0; row < rows; ++row) {
            const auto idx = model->index(row, 0);
            qDebug() << row << idx.data(eMyMoney::Model::IdRole).toString() << idx.data(eMyMoney::Model::SplitAccountIdRole).toString()
                     << idx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>().formatMoney(100) << idx.data(eMyMoney::Model::SplitValueRole).value<MyMoneyMoney>().formatMoney(100);
        }
    }
    void createStatusEntry(eMyMoney::Split::State status);
    bool isDatePostOpeningDate(const QDate& date, const QString& accountId);

    bool postdateChanged(const QDate& date);
    bool categoryChanged(SplitModel* model, const QString& accountId, AmountEdit* widget, const MyMoneyMoney& factor);

    void createFeeCategory();
    void createInterestCategory();
    void createAssetAccount();

    void setSecurity(const MyMoneySecurity& sec);

    bool amountChanged(SplitModel* model, AmountEdit* widget, const MyMoneyMoney& transactionFactor);

    void scheduleUpdateTotalAmount();
    void updateWidgetState();
    void setupTabOrder();

    void editSplits(SplitModel* sourceSplitModel, AmountEdit* amountEdit, const MyMoneyMoney& transactionFactor);
    void removeUnusedSplits(MyMoneyTransaction& t, SplitModel* splitModel);
    void addSplits(MyMoneyTransaction& t, SplitModel* splitModel);
    void setupParentInvestmentAccount(const QString& accountId);
    QModelIndex adjustToSecuritySplitIdx(const QModelIndex& idx);

    void loadFeeAndInterestAmountEdits();
    void adjustSharesCommodity(AmountEdit* amountEdit, const QString& accountId);
    void setupAssetAccount(const QString& accountId);

    InvestTransactionEditor* q;
    Ui_InvestTransactionEditor* ui;
    Ui_InvestTransactionEditor* tabOrderUi;

    // models for UI elements
    AccountNamesFilterProxyModel* accountsModel;
    AccountNamesFilterProxyModel* feesModel;
    AccountNamesFilterProxyModel* interestModel;
    QStringListModel* activitiesModel;
    QSortFilterProxyModel* securitiesModel;
    AccountsProxyModel* securityFilterModel;
    KDescendantsProxyModel* accountsListModel;
    KMyMoneyAccountComboSplitHelper* feeSplitHelper;
    KMyMoneyAccountComboSplitHelper* interestSplitHelper;

    QUndoStack undoStack;
    Invest::Activity* currentActivity;

    // the selected security and the account holding it
    MyMoneySecurity security;
    // and its trading currency
    MyMoneySecurity transactionCurrency;

    // the asset or brokerage account
    MyMoneyAccount assetAccount;
    // and its currency
    MyMoneySecurity assetSecurity;

    // the containing investment account (parent of stockAccount)
    MyMoneyAccount parentAccount;

    // the transaction
    MyMoneyTransaction transaction;

    // the various splits
    MyMoneySplit stockSplit;
    MyMoneySplit assetSplit;
    SplitModel* feeSplitModel;
    SplitModel* interestSplitModel;

    // exchange rate information for assetSplit
    MyMoneyPrice assetPrice;

    bool accepted;

    /**
     * Flag to bypass the user dialog to modify exchange rate information.
     * This is used during the loading of a transaction, when data is
     * changed due to the load operation but no user interaction is
     * wanted.
     */
    bool bypassUserPriceUpdate;
};

void InvestTransactionEditor::Private::removeUnusedSplits(MyMoneyTransaction& t, SplitModel* splitModel)
{
    for (const auto& sp : qAsConst(t.splits())) {
        if (sp.id() == stockSplit.id()) {
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
        const auto idx = splitModel->index(row, 0);
        MyMoneySplit s;
        const auto splitId = idx.data(eMyMoney::Model::IdRole).toString();
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
    } catch (MyMoneyException&) {
        qDebug() << "Ooops: invalid account id" << accountId << "in" << Q_FUNC_INFO;
    }
    return rc;
}

bool InvestTransactionEditor::Private::postdateChanged(const QDate& date)
{
    bool rc = true;
    WidgetHintFrame::hide(ui->dateEdit, i18n("The posting date of the transaction."));

    QStringList accountIds;

    auto collectAccounts = [&](const SplitModel* model) {
        const auto rows = model->rowCount();
        for (int row = 0; row < rows; ++row) {
            const auto index = model->index(row, 0);
            accountIds << index.data(eMyMoney::Model::SplitAccountIdRole).toString();
        }
    };

    // collect all account ids
    accountIds << parentAccount.id();
    if (currentActivity->feesRequired() != Invest::Activity::Unused) {
        collectAccounts(feeSplitModel);
    }
    if (currentActivity->interestRequired() != Invest::Activity::Unused) {
        collectAccounts(interestSplitModel);
    }

    for (const auto& accountId : qAsConst(accountIds)) {
        if (!isDatePostOpeningDate(date, accountId)) {
            const auto account = MyMoneyFile::instance()->account(accountId);
            WidgetHintFrame::show(ui->dateEdit, i18n("The posting date is prior to the opening date of account <b>%1</b>.", account.name()));
            rc = false;
            break;
        }
    }
    return rc;
}

bool InvestTransactionEditor::Private::categoryChanged(SplitModel* model, const QString& accountId, AmountEdit* amountEdit, const MyMoneyMoney& factor)
{
    bool rc = true;
    if (accountId.isEmpty()) {
        // in case the user cleared the category, we need
        // to make sure that there are no leftovers in the
        // split model so we clear it. This can only happen
        // for single split categories, so at most we
        // have to clear one item
        if (model->rowCount() == 1) {
            const auto idx = model->index(0, 0);
            const auto s = model->itemByIndex(idx);
            model->removeItem(s);
        }
        return true;
    }

    if (model->rowCount() <= 1) {
        try {
            MyMoneyAccount category = MyMoneyFile::instance()->account(accountId);

            // make sure we have a split in the model
            if (model->rowCount() == 0) {
                // add an empty split
                MyMoneySplit s;
                model->addItem(s);
            }

            const auto index = model->index(0, 0);
            model->setData(index, accountId, eMyMoney::Model::SplitAccountIdRole);

            // extract the categories currency
            const auto accountIdx = MyMoneyFile::instance()->accountsModel()->indexById(accountId);
            const auto currencyId = accountIdx.data(eMyMoney::Model::AccountCurrencyIdRole).toString();
            const auto currency = MyMoneyFile::instance()->currenciesModel()->itemById(currencyId);

            // in case the commodity changes, we need to update the shares part
            if (currency.id() != amountEdit->sharesCommodity().id()) {
                // switch to value display so that we show the transaction commodity
                // for single currency data entry this does not have an effect
                amountEdit->setDisplayState(MultiCurrencyEdit::DisplayValue);
                amountEdit->setSharesCommodity(currency);
                auto sharesAmount = amountEdit->value();
                if (!sharesAmount.isZero()) {
                    amountEdit->setShares(sharesAmount);
                    KCurrencyCalculator::updateConversion(amountEdit, ui->dateEdit->date());
                }
            }

            model->setData(index, QVariant::fromValue<MyMoneyMoney>(factor * amountEdit->value()), eMyMoney::Model::SplitValueRole);
            model->setData(index, QVariant::fromValue<MyMoneyMoney>(factor * amountEdit->shares()), eMyMoney::Model::SplitSharesRole);

        } catch (MyMoneyException&) {
            qDebug() << "Ooops: invalid account id" << accountId << "in" << Q_FUNC_INFO;
        }
    }

    return rc;
}

void InvestTransactionEditor::Private::setSecurity(const MyMoneySecurity& sec)
{
    if (sec.tradingCurrency() != security.tradingCurrency()) {
        transactionCurrency = MyMoneyFile::instance()->currency(sec.tradingCurrency());
        ui->totalAmountEdit->setValueCommodity(transactionCurrency);
        transaction.setCommodity(sec.tradingCurrency());
        feeSplitModel->setTransactionCommodity(sec.tradingCurrency());
        interestSplitModel->setTransactionCommodity(sec.tradingCurrency());
        loadFeeAndInterestAmountEdits();

        auto haveValue = [&](const SplitModel* model) {
            const auto rows = model->rowCount();
            for (int row = 0; row < rows; ++row) {
                const auto idx = model->index(row, 0);
                if (!idx.data(eMyMoney::Model::SplitValueRole).value<MyMoneyMoney>().isZero()) {
                    return true;
                }
            }
            return false;
        };

        bool needWarning = !assetSplit.value().isZero();
        if (currentActivity) {
            needWarning |= ((currentActivity->feesRequired() != Invest::Activity::Unused) && haveValue(feeSplitModel));
            needWarning |= ((currentActivity->interestRequired() != Invest::Activity::Unused) && haveValue(interestSplitModel));
        }

        if (needWarning) {
            ui->infoMessage->setText(i18nc("@info:usagetip", "The transaction commodity has been changed which will possibly make all price information invalid. Please check them."));
            if (!ui->infoMessage->isShowAnimationRunning()) {
                ui->infoMessage->animatedShow();
                Q_EMIT q->editorLayoutChanged();
            }
        }
    }

    security = sec;

    // update the precision to that used by the new security
    ui->sharesAmountEdit->setPrecision(MyMoneyMoney::denomToPrec(security.smallestAccountFraction()));
}

bool InvestTransactionEditor::Private::amountChanged(SplitModel* model, AmountEdit* amountEdit, const MyMoneyMoney& transactionFactor)
{
    bool rc = true;
    if (!amountEdit->text().isEmpty() && (model->rowCount() <= 1)) {
        rc = false;
        try {
            MyMoneyMoney shares;
            if (model->rowCount() == 1) {
                const auto index = model->index(0, 0);

                // check if there is a change in the values other than simply reverting the sign
                // and get an updated price in that case
                if ((index.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>() != -amountEdit->shares())
                    || (index.data(eMyMoney::Model::SplitValueRole).value<MyMoneyMoney>() != -amountEdit->value())) {
                    KCurrencyCalculator::updateConversion(amountEdit, ui->dateEdit->date());
                }

                model->setData(index, QVariant::fromValue<MyMoneyMoney>((amountEdit->value() * transactionFactor)), eMyMoney::Model::SplitValueRole);
                model->setData(index, QVariant::fromValue<MyMoneyMoney>((amountEdit->shares() * transactionFactor)), eMyMoney::Model::SplitSharesRole);
            }
            rc = true;

        } catch (MyMoneyException&) {
            qDebug() << "Ooops: something went wrong in" << Q_FUNC_INFO;
        }
    } else {
        /// @todo ask what to do: if the rest of the splits is the same amount we could simply reverse the sign
        /// of all splits, otherwise we could ask if the user wants to start the split editor or anything else.
    }
    return rc;
}

void InvestTransactionEditor::Private::editSplits(SplitModel* sourceSplitModel, AmountEdit* amountEdit, const MyMoneyMoney& transactionFactor)
{
    SplitModel splitModel(q, nullptr, *sourceSplitModel);

    // create an empty split at the end
    // used to create new splits
    splitModel.appendEmptySplit();

    QPointer<SplitDialog> splitDialog = new SplitDialog(transactionCurrency, MyMoneyMoney::autoCalc, parentAccount.fraction(), transactionFactor, q);
    splitDialog->setModel(&splitModel);

    int rc = splitDialog->exec();

    if (splitDialog && (rc == QDialog::Accepted)) {
        // remove that empty split again before we update the splits
        splitModel.removeEmptySplit();

        // copy the splits model contents
        *sourceSplitModel = splitModel;

        // update the transaction amount
        amountEdit->setSharesCommodity(amountEdit->valueCommodity());
        auto amountShares = splitDialog->transactionAmount() * transactionFactor;
        amountEdit->setValue(amountShares);

        // the price might have been changed, so we have to update our copy
        // but only if there is one counter split
        if (sourceSplitModel->rowCount() == 1) {
            const auto idx = sourceSplitModel->index(0, 0);
            amountShares = idx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>();

            adjustSharesCommodity(amountEdit, idx.data(eMyMoney::Model::SplitAccountIdRole).toString());

            // make sure to show the value in the widget
            // according to the currency presented
        }
        amountEdit->setShares(amountShares * transactionFactor);

        updateWidgetState();
    }

    if (splitDialog) {
        splitDialog->deleteLater();
    }
}

void InvestTransactionEditor::Private::setupParentInvestmentAccount(const QString& accountId)
{
    auto const file = MyMoneyFile::instance();
    auto const model = file->accountsModel();

    // extract account information from model
    const auto index = model->indexById(accountId);
    parentAccount = model->itemByIndex(index);

    // show child accounts in the combo box
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
    WidgetHintFrame::hide(ui->assetAccountCombo, i18nc("@info:tooltip", "Asset or brokerage account"));
    WidgetHintFrame::hide(ui->priceAmountEdit, i18nc("@info:tooltip", "Price information for this transaction"));
    WidgetHintFrame::hide(ui->securityAccountCombo, i18nc("@info:tooltip", "Security for this transaction"));

    // all the other logic needs a valid activity
    if (currentActivity == nullptr) {
        return;
    }

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

    QString accountId;
    switch(currentActivity->assetAccountRequired()) {
    case Invest::Activity::Unused:
        break;
    case Invest::Activity::Optional:
    case Invest::Activity::Mandatory:
        accountId = ui->assetAccountCombo->getSelected();
        if (MyMoneyFile::instance()->isStandardAccount(accountId)) {
            accountId.clear();
        }
        if (accountId.isEmpty()) {
            WidgetHintFrame::show(ui->assetAccountCombo, i18nc("@info:tooltip", "Select account to balance the transaction"));
        }
        break;
    }

    if (!currentActivity->haveFees(currentActivity->feesRequired())) {
        if (ui->feesCombo->currentText().isEmpty()) {
            WidgetHintFrame::show(ui->feesCombo, i18nc("@info:tooltip", "Enter category for fees"));
        }
        if (ui->feesAmountEdit->value().isZero()) {
            WidgetHintFrame::show(ui->feesAmountEdit, i18nc("@info:tooltip", "Enter amount of fees"));
        }
    }

    if (!currentActivity->haveInterest(currentActivity->interestRequired())) {
        if (ui->interestCombo->currentText().isEmpty()) {
            WidgetHintFrame::show(ui->interestCombo, i18nc("@info:tooltip", "Enter category for interest"));
        }
        if (ui->interestAmountEdit->value().isZero()) {
            WidgetHintFrame::show(ui->interestAmountEdit, i18nc("@info:tooltip", "Enter amount of interest"));
        }
    }

    if (ui->securityAccountCombo->currentIndex() == -1) {
        WidgetHintFrame::show(ui->securityAccountCombo, i18nc("@info:tooltip", "Select the security for this transaction"));
    }
}

void InvestTransactionEditor::Private::loadFeeAndInterestAmountEdits()
{
    auto loadAmountEdit = [&](SplitModel* model, AmountEdit* amountEdit) {
        amountEdit->setReadOnly(false);
        amountEdit->setCommodity(transactionCurrency);
        switch (model->rowCount()) {
        case 0:
            amountEdit->clear();
            break;
        case 1: {
            const auto idx = model->index(0, 0);
            amountEdit->setValue(idx.data(eMyMoney::Model::SplitValueRole).value<MyMoneyMoney>().abs());
            amountEdit->setShares(idx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>().abs());
            adjustSharesCommodity(amountEdit, idx.data(eMyMoney::Model::SplitAccountIdRole).toString());
        } break;
        default:
            amountEdit->setValue(model->valueSum().abs());
            amountEdit->setShares(model->valueSum().abs());
            amountEdit->setReadOnly(true);
            break;
        }
    };

    // possibly update the currency on the fees and interest
    loadAmountEdit(feeSplitModel, ui->feesAmountEdit);
    loadAmountEdit(interestSplitModel, ui->interestAmountEdit);
}

void InvestTransactionEditor::Private::adjustSharesCommodity(AmountEdit* amountEdit, const QString& accountId)
{
    // adjust the commodity for the shares
    const auto accountIdx = MyMoneyFile::instance()->accountsModel()->indexById(accountId);
    const auto currencyId = accountIdx.data(eMyMoney::Model::AccountCurrencyIdRole).toString();
    const auto currency = MyMoneyFile::instance()->currenciesModel()->itemById(currencyId);
    amountEdit->setSharesCommodity(currency);
}

void InvestTransactionEditor::Private::setupAssetAccount(const QString& accountId)
{
    const auto file = MyMoneyFile::instance();
    assetAccount = file->account(accountId);
    assetSecurity = file->currency(assetAccount.currencyId());
    ui->totalAmountEdit->setSharesCommodity(assetSecurity);
}

void InvestTransactionEditor::Private::scheduleUpdateTotalAmount()
{
    QMetaObject::invokeMethod(q, "updateTotalAmount", Qt::QueuedConnection);
}

void InvestTransactionEditor::Private::setupTabOrder()
{
    const auto defaultTabOrder = QStringList{
        QLatin1String("activityCombo"),
        QLatin1String("dateEdit"),
        QLatin1String("securityAccountCombo"),
        QLatin1String("sharesAmountEdit"),
        QLatin1String("assetAccountCombo"),
        QLatin1String("priceAmountEdit"),
        QLatin1String("feesCombo"),
        QLatin1String("feesAmountEdit"),
        QLatin1String("interestCombo"),
        QLatin1String("interestAmountEdit"),
        QLatin1String("memoEdit"),
        QLatin1String("statusCombo"),
        QLatin1String("enterButton"),
        QLatin1String("cancelButton"),
    };
    q->setProperty("kmm_defaulttaborder", defaultTabOrder);
    q->setProperty("kmm_currenttaborder", q->tabOrder(QLatin1String("investTransactionEditor"), defaultTabOrder));

    q->setupTabOrder(q->property("kmm_currenttaborder").toStringList());
}

void InvestTransactionEditor::Private::createFeeCategory()
{
    auto creator = new AccountCreator(q);
    creator->setComboBox(ui->feesCombo);
    creator->addButton(ui->cancelButton);
    creator->addButton(ui->enterButton);
    creator->setAccountType(eMyMoney::Account::Type::Expense);
    creator->createAccount();
}

void InvestTransactionEditor::Private::createInterestCategory()
{
    auto creator = new AccountCreator(q);
    creator->setComboBox(ui->interestCombo);
    creator->addButton(ui->cancelButton);
    creator->addButton(ui->enterButton);
    creator->setAccountType(eMyMoney::Account::Type::Income);
    creator->createAccount();
}

void InvestTransactionEditor::Private::createAssetAccount()
{
    auto creator = new AccountCreator(q);
    creator->setComboBox(ui->assetAccountCombo);
    creator->addButton(ui->cancelButton);
    creator->addButton(ui->enterButton);
    creator->setAccountType(eMyMoney::Account::Type::Asset);
    creator->createAccount();
}

InvestTransactionEditor::InvestTransactionEditor(QWidget* parent, const QString& accId)
    : TransactionEditorBase(parent, accId)
    , d(new Private(this))
{
    d->ui->setupUi(this);

    // initially, the info message is hidden
    d->ui->infoMessage->hide();

    d->ui->activityCombo->setModel(d->activitiesModel);

    auto const accountsModel = MyMoneyFile::instance()->accountsModel();

    d->securityFilterModel->addAccountGroup({eMyMoney::Account::Type::Asset});
    d->securityFilterModel->setSourceModel(accountsModel);
    d->securityFilterModel->setHideEquityAccounts(false);
    d->securityFilterModel->setHideClosedAccounts(!KMyMoneySettings::showAllAccounts());

    d->accountsListModel->setSourceModel(d->securityFilterModel);

    d->securitiesModel->setSourceModel(d->accountsListModel);
    d->securitiesModel->setFilterRole(eMyMoney::Model::AccountParentIdRole);
    d->securitiesModel->setFilterKeyColumn(0);
    d->securitiesModel->setSortRole(Qt::DisplayRole);
    d->securitiesModel->setSortLocaleAware(true);
    d->securitiesModel->sort(AccountsModel::Column::AccountName);

    d->ui->securityAccountCombo->setModel(d->securitiesModel);
    d->ui->securityAccountCombo->lineEdit()->setClearButtonEnabled(true);

    d->accountsModel->addAccountGroup(QVector<eMyMoney::Account::Type> { eMyMoney::Account::Type::Asset, eMyMoney::Account::Type::Liability } );
    d->accountsModel->setHideEquityAccounts(false);
    d->accountsModel->setSourceModel(accountsModel);
    d->accountsModel->sort(AccountsModel::Column::AccountName);
    d->ui->assetAccountCombo->setModel(d->accountsModel);
    d->ui->assetAccountCombo->setSplitActionVisible(false);

    d->feesModel->addAccountGroup(QVector<eMyMoney::Account::Type> { eMyMoney::Account::Type::Expense });
    d->feesModel->setSourceModel(accountsModel);
    d->feesModel->sort(AccountsModel::Column::AccountName);
    d->ui->feesCombo->setModel(d->feesModel);
    d->feeSplitHelper = new KMyMoneyAccountComboSplitHelper(d->ui->feesCombo, d->feeSplitModel);
    connect(d->feeSplitHelper, &KMyMoneyAccountComboSplitHelper::accountComboDisabled, d->ui->feesAmountEdit, &AmountEdit::setReadOnly);

    d->interestModel->addAccountGroup(QVector<eMyMoney::Account::Type> { eMyMoney::Account::Type::Income });
    d->interestModel->setSourceModel(accountsModel);
    d->interestModel->sort(AccountsModel::Column::AccountName);
    d->ui->interestCombo->setModel(d->interestModel);
    d->interestSplitHelper = new KMyMoneyAccountComboSplitHelper(d->ui->interestCombo, d->interestSplitModel);
    connect(d->interestSplitHelper, &KMyMoneyAccountComboSplitHelper::accountComboDisabled, d->ui->interestAmountEdit, &AmountEdit::setReadOnly);

    d->ui->enterButton->setIcon(Icons::get(Icon::DialogOK));
    d->ui->cancelButton->setIcon(Icons::get(Icon::DialogCancel));

    d->ui->statusCombo->setModel(MyMoneyFile::instance()->statusModel());

    d->ui->sharesAmountEdit->setAllowEmpty(true);
    d->ui->sharesAmountEdit->setCalculatorButtonVisible(true);

    connect(d->ui->sharesAmountEdit, &AmountEdit::amountChanged, this, [&]() {
        if (d->currentActivity) {
            d->stockSplit.setShares(d->ui->sharesAmountEdit->value() * d->currentActivity->sharesFactor());
            d->stockSplit.setValue(d->currentActivity->valueAllShares().convert(d->transactionCurrency.smallestAccountFraction(), d->security.roundingMethod())
                                   * d->currentActivity->sharesFactor());
            if (d->currentActivity->type() != eMyMoney::Split::InvestmentTransactionType::SplitShares) {
                d->scheduleUpdateTotalAmount();
            }
            d->updateWidgetState();
        }
    });

    d->ui->priceAmountEdit->setAllowEmpty(true);
    d->ui->priceAmountEdit->setCalculatorButtonVisible(true);
    connect(d->ui->priceAmountEdit, &AmountEdit::amountChanged, this, [&]() {
        if (d->currentActivity) {
            d->stockSplit.setValue(d->currentActivity->valueAllShares().convert(d->transactionCurrency.smallestAccountFraction(), d->security.roundingMethod())
                                   * d->currentActivity->sharesFactor());
            if (d->currentActivity->type() != eMyMoney::Split::InvestmentTransactionType::SplitShares) {
                d->scheduleUpdateTotalAmount();
            }
            d->updateWidgetState();
        }
    });

    d->ui->feesAmountEdit->setAllowEmpty(true);
    d->ui->feesAmountEdit->setCalculatorButtonVisible(true);
    connect(d->ui->feesAmountEdit, &AmountEdit::amountChanged, this, [&]() {
        d->amountChanged(d->feeSplitModel, d->ui->feesAmountEdit, MyMoneyMoney::ONE);
        d->updateWidgetState();
        if (!d->ui->feesCombo->getSelected().isEmpty()) {
            d->scheduleUpdateTotalAmount();
        }
    });

    d->ui->interestAmountEdit->setAllowEmpty(true);
    d->ui->interestAmountEdit->setCalculatorButtonVisible(true);
    connect(d->ui->interestAmountEdit, &AmountEdit::amountChanged, this, [&]() {
        d->amountChanged(d->interestSplitModel, d->ui->interestAmountEdit, MyMoneyMoney::MINUS_ONE);
        d->updateWidgetState();
        if (!d->ui->interestCombo->getSelected().isEmpty()) {
            d->scheduleUpdateTotalAmount();
        }
    });

    WidgetHintFrameCollection* frameCollection = new WidgetHintFrameCollection(this);
    frameCollection->addFrame(new WidgetHintFrame(d->ui->dateEdit));
    frameCollection->addFrame(new WidgetHintFrame(d->ui->securityAccountCombo));
    frameCollection->addFrame(new WidgetHintFrame(d->ui->assetAccountCombo));
    frameCollection->addFrame(new WidgetHintFrame(d->ui->sharesAmountEdit));
    frameCollection->addFrame(new WidgetHintFrame(d->ui->priceAmountEdit));
    frameCollection->addFrame(new WidgetHintFrame(d->ui->feesCombo));
    frameCollection->addFrame(new WidgetHintFrame(d->ui->feesAmountEdit));
    frameCollection->addFrame(new WidgetHintFrame(d->ui->interestCombo));
    frameCollection->addFrame(new WidgetHintFrame(d->ui->interestAmountEdit));
    frameCollection->addWidget(d->ui->enterButton);

    connect(d->ui->assetAccountCombo, &KMyMoneyAccountCombo::accountSelected, this, [&](const QString& accountId) {
        d->setupAssetAccount(accountId);
        d->assetSplit.setAccountId(accountId);

        // check the opening dates of this account and
        // update the widgets accordingly
        d->postdateChanged(d->ui->dateEdit->date());
        d->updateWidgetState();
    });

    connect(d->ui->dateEdit, &KMyMoneyDateEdit::dateChanged, this, [&](const QDate& date) {
        d->postdateChanged(date);
    });

    connect(d->ui->activityCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &InvestTransactionEditor::activityChanged);
    connect(d->ui->securityAccountCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](int index) {
        const auto idx = d->ui->securityAccountCombo->model()->index(index, 0);
        if (idx.isValid()) {
            const auto accountId = idx.data(eMyMoney::Model::IdRole).toString();
            const auto securityId = idx.data(eMyMoney::Model::AccountCurrencyIdRole).toString();
            try {
                const auto file = MyMoneyFile::instance();
                const auto sec = file->security(securityId);

                d->stockSplit.setAccountId(accountId);
                d->setSecurity(sec);

                d->scheduleUpdateTotalAmount();

            } catch (MyMoneyException&) {
                qDebug() << "Problem to find securityId" << accountId << "or" << securityId << "in InvestTransactionEditor::securityAccountChanged";
            }
        }
    });

    connect(d->ui->securityAccountCombo, &QComboBox::currentTextChanged, this, [&](const QString& text) {
        if (text.isEmpty() && d->ui->securityAccountCombo->currentIndex() != -1) {
            d->ui->securityAccountCombo->setCurrentIndex(-1);
            updateWidgets();
        }
    });

    connect(d->ui->feesCombo, &KMyMoneyAccountCombo::accountSelected, this, [&](const QString& accountId) {
        d->categoryChanged(d->feeSplitModel, accountId, d->ui->feesAmountEdit, MyMoneyMoney::ONE);
        d->updateWidgetState();
        if (!d->feeSplitModel->valueSum().isZero()) {
            d->scheduleUpdateTotalAmount();
        }
    });

    connect(
        d->ui->feesCombo,
        &KMyMoneyAccountCombo::splitDialogRequest,
        this,
        [&]() {
            d->editSplits(d->feeSplitModel, d->ui->feesAmountEdit, MyMoneyMoney::ONE);
        },
        Qt::QueuedConnection);

    connect(d->ui->interestCombo, &KMyMoneyAccountCombo::accountSelected, this, [&](const QString& accountId) {
        d->categoryChanged(d->interestSplitModel, accountId, d->ui->interestAmountEdit, MyMoneyMoney::MINUS_ONE);
        d->updateWidgetState();
        if (!d->interestSplitModel->valueSum().isZero()) {
            d->scheduleUpdateTotalAmount();
        }
    });

    connect(
        d->ui->interestCombo,
        &KMyMoneyAccountCombo::splitDialogRequest,
        this,
        [&]() {
            d->editSplits(d->interestSplitModel, d->ui->interestAmountEdit, MyMoneyMoney::MINUS_ONE);
        },
        Qt::QueuedConnection);

    connect(d->ui->cancelButton, &QToolButton::clicked, this, [&]() {
        Q_EMIT done();
    });
    connect(d->ui->enterButton, &QToolButton::clicked, this, [&]() {
        d->accepted = true;
        Q_EMIT done();
    });

    // handle some events in certain conditions different from default
    d->ui->activityCombo->installEventFilter(this);
    d->ui->statusCombo->installEventFilter(this);
    d->ui->feesCombo->installEventFilter(this);
    d->ui->interestCombo->installEventFilter(this);
    d->ui->assetAccountCombo->installEventFilter(this);

    d->ui->totalAmountEdit->setCalculatorButtonVisible(false);

    d->setupParentInvestmentAccount(accId);

    setCancelButton(d->ui->cancelButton);
    setEnterButton(d->ui->enterButton);

    d->setupTabOrder();
}

InvestTransactionEditor::~InvestTransactionEditor()
{
}

bool InvestTransactionEditor::accepted() const
{
    return d->accepted;
}

void InvestTransactionEditor::updateTotalAmount()
{
    d->updateWidgetState();
    if (d->currentActivity) {
        const auto totalAmount = d->currentActivity->totalAmount(d->stockSplit, d->feeSplitModel, d->interestSplitModel);
        const auto oldValue = d->ui->totalAmountEdit->value();
        const auto oldShares = d->ui->totalAmountEdit->shares();

        d->ui->totalAmountEdit->setValue(totalAmount.abs());
        d->ui->totalAmountEdit->setValueCommodity(d->transactionCurrency);
        d->assetSplit.setValue(-totalAmount);
        d->assetSplit.setShares(d->assetSplit.value() / d->assetPrice.rate(d->assetAccount.currencyId()));
        d->ui->totalAmountEdit->setShares(d->assetSplit.shares().abs());
        // only ask the user for an exchange rate if the value differs from zero
        // and the values have changed (reverting in sign does not count as a change)
        if (!totalAmount.isZero() && !d->assetSplit.shares().isZero() && !d->bypassUserPriceUpdate) {
            if ((oldValue.abs() != d->ui->totalAmountEdit->value().abs()) || (oldShares.abs() != d->ui->totalAmountEdit->shares().abs())) {
                // force display to the transaction commodity so that the values are correct
                d->ui->totalAmountEdit->setDisplayState(AmountEdit::DisplayValue);
                KCurrencyCalculator::updateConversion(d->ui->totalAmountEdit, d->ui->dateEdit->date());
                const auto rate = d->ui->totalAmountEdit->value() / d->ui->totalAmountEdit->shares();
                d->assetPrice =
                    MyMoneyPrice(d->transactionCurrency.id(), d->assetAccount.currencyId(), d->transaction.postDate(), rate, QLatin1String("KMyMoney"));
                d->assetSplit.setShares(d->ui->totalAmountEdit->shares());
                // since the total amount is kept as a positive number, we may
                // need to adjust the sign of the shares. The value nevertheless
                // has the correct sign. So if the sign does not match, we
                // simply revert the sign of the shares.
                if (d->assetSplit.shares().isNegative() != d->assetSplit.value().isNegative()) {
                    d->assetSplit.setShares(-d->assetSplit.shares());
                }
            }
        }
    }
}


void InvestTransactionEditor::loadTransaction(const QModelIndex& index)
{
    d->bypassUserPriceUpdate = true;
    d->ui->activityCombo->setCurrentIndex(-1);
    d->ui->securityAccountCombo->setCurrentIndex(-1);
    const auto file = MyMoneyFile::instance();
    auto idx = d->adjustToSecuritySplitIdx(MyMoneyFile::baseModel()->mapToBaseSource(index));
    if (!idx.isValid() || idx.data(eMyMoney::Model::IdRole).toString().isEmpty()) {
        d->transaction = MyMoneyTransaction();
        d->transaction.setCommodity(d->parentAccount.currencyId());
        d->transactionCurrency = MyMoneyFile::instance()->baseCurrency();
        d->ui->totalAmountEdit->setCommodity(d->transactionCurrency);
        d->security = MyMoneySecurity();
        d->security.setTradingCurrency(d->transactionCurrency.id());
        d->stockSplit = MyMoneySplit();
        d->assetSplit = MyMoneySplit();
        d->assetAccount = MyMoneyAccount();
        d->assetSecurity = MyMoneySecurity();
        d->ui->activityCombo->setCurrentIndex(0);
        d->ui->securityAccountCombo->setCurrentIndex(-1);
        const auto lastUsedPostDate = KMyMoneySettings::lastUsedPostDate();
        if (lastUsedPostDate.isValid()) {
            d->ui->dateEdit->setDate(lastUsedPostDate.date());
        } else {
            d->ui->dateEdit->setDate(QDate::currentDate());
        }
        // select the associated brokerage account if it exists
        const auto brokerageAccount = file->accountsModel()->itemByName(d->parentAccount.brokerageName());
        d->ui->assetAccountCombo->setSelected(brokerageAccount.id());
        d->loadFeeAndInterestAmountEdits();

    } else {
        // keep a copy of the transaction and split
        d->transaction = file->journalModel()->itemByIndex(idx).transaction();
        d->stockSplit = file->journalModel()->itemByIndex(idx).split();

        // during loading the editor the stocksplit object maybe changed which
        // don't want here. Therefore, we keep a local copy and reload it
        // once needed
        const auto stockSplitCopy(d->stockSplit);

        QModelIndex assetAccountSplitIdx;
        eMyMoney::Split::InvestmentTransactionType transactionType;

        // KMyMoneyUtils::dissectInvestmentTransaction fills the split models which
        // causes to update the widgets when they are not yet setup. So we simply
        // prevent sending out signals for them
        QSignalBlocker feesModelBlocker(d->feeSplitModel);
        QSignalBlocker interestModelBlocker(d->interestSplitModel);

        KMyMoneyUtils::dissectInvestmentTransaction(idx,
                                                    assetAccountSplitIdx,
                                                    d->feeSplitModel,
                                                    d->interestSplitModel,
                                                    d->security,
                                                    d->transactionCurrency,
                                                    transactionType);
        d->assetSplit = file->journalModel()->itemByIndex(assetAccountSplitIdx).split();
        if (!d->assetSplit.id().isEmpty()) {
            d->setupAssetAccount(d->assetSplit.accountId());
        }

        // extract conversion rate information for asset split before changing
        // the activity because that will need it (in updateTotalAmount() )
        if (!(d->assetSplit.shares().isZero() || d->assetSplit.value().isZero())) {
            const auto rate = d->assetSplit.value() / d->assetSplit.shares();
            d->assetPrice = MyMoneyPrice(d->transactionCurrency.id(), d->assetAccount.currencyId(), d->transaction.postDate(), rate, QLatin1String("KMyMoney"));
        }

        // load the widgets. setting activityCombo also initializes
        // d->currentActivity to have the right object
        d->ui->activityCombo->setCurrentIndex(static_cast<int>(transactionType));

        // changing the transactionType may have modified the stocksplit which is
        // not necessary here. To cope with that, we simply reload it from the backup
        d->stockSplit = stockSplitCopy;

        d->ui->dateEdit->setDate(d->transaction.postDate());

        d->ui->memoEdit->setPlainText(d->stockSplit.memo());

        d->ui->assetAccountCombo->setSelected(d->assetSplit.accountId());

        d->ui->statusCombo->setCurrentIndex(static_cast<int>(d->stockSplit.reconcileFlag()));

        // Avoid updating other widgets (connected through signal/slot) during loading
        const auto indexes = d->securitiesModel->match(d->securitiesModel->index(0,0), eMyMoney::Model::IdRole, d->stockSplit.accountId(), 1, Qt::MatchFixedString);
        if (!indexes.isEmpty()) {
            d->ui->securityAccountCombo->setCurrentIndex(indexes.first().row());
        }

        // changing the security in the last step may have modified the stocksplit
        // which is not wanted here. To cope with that, we simply reload it from the model
        d->stockSplit = stockSplitCopy;

        // also, setting the security may have changed the precision so we
        // reload it here
        QSignalBlocker blockShares(d->ui->sharesAmountEdit);
        if (transactionType == eMyMoney::Split::InvestmentTransactionType::SplitShares)
            d->ui->sharesAmountEdit->setPrecision(-1);
        else
            d->ui->sharesAmountEdit->setPrecision(MyMoneyMoney::denomToPrec(d->security.smallestAccountFraction()));
        d->ui->sharesAmountEdit->setValue(d->stockSplit.shares() * d->currentActivity->sharesFactor());

        d->loadFeeAndInterestAmountEdits();

        d->feeSplitHelper->updateWidget();
        d->interestSplitHelper->updateWidget();

        // Avoid updating other widgets (connected through signal/slot) during loading
        QSignalBlocker blockPrice(d->ui->priceAmountEdit);
        d->currentActivity->loadPriceWidget(d->stockSplit);

        updateTotalAmount();
    }

    d->bypassUserPriceUpdate = false;

    // delay update until next run of event loop so that all necessary widgets are visible
    QMetaObject::invokeMethod(this, "updateWidgets", Qt::QueuedConnection);

    // set focus to first tab field once we return to event loop
    const auto tabOrder = property("kmm_currenttaborder").toStringList();
    if (!tabOrder.isEmpty()) {
        const auto focusWidget = findChild<QWidget*>(tabOrder.first());
        if (focusWidget) {
            QMetaObject::invokeMethod(focusWidget, "setFocus", Qt::QueuedConnection);
        }
    }
}

void InvestTransactionEditor::updateWidgets()
{
    d->updateWidgetState();
}

void InvestTransactionEditor::activityChanged(int index)
{
    const auto type = static_cast<eMyMoney::Split::InvestmentTransactionType>(index);
    if (!d->currentActivity || type != d->currentActivity->type()) {
        auto oldType = eMyMoney::Split::InvestmentTransactionType::UnknownTransactionType;
        if (d->currentActivity) {
            oldType = d->currentActivity->type();
        }
        const auto previousActivity = d->currentActivity;
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

        // update the stocksplit when switching between e.g. buy and sell
        if (previousActivity && previousActivity->sharesFactor() != d->currentActivity->sharesFactor()) {
            d->stockSplit.setShares(-d->stockSplit.shares());
            d->stockSplit.setValue(-d->stockSplit.value());
        }
        delete previousActivity;

        if (type == eMyMoney::Split::InvestmentTransactionType::SplitShares && oldType != eMyMoney::Split::InvestmentTransactionType::SplitShares) {
            // switch to split
            d->stockSplit.setValue(MyMoneyMoney());
            d->stockSplit.setPrice(MyMoneyMoney());
            d->ui->sharesAmountEdit->setPrecision(-1);
        } else if (type != eMyMoney::Split::InvestmentTransactionType::SplitShares && oldType == eMyMoney::Split::InvestmentTransactionType::SplitShares) {
            // switch away from split
            d->stockSplit.setPrice(d->ui->priceAmountEdit->value());
            d->stockSplit.setValue(d->stockSplit.shares() * d->stockSplit.price());
            d->ui->sharesAmountEdit->setPrecision(MyMoneyMoney::denomToPrec(d->security.smallestAccountFraction()));
        }

        if (type == eMyMoney::Split::InvestmentTransactionType::Dividend && oldType != eMyMoney::Split::InvestmentTransactionType::Dividend) {
            // switch to dividend
            d->stockSplit.setShares(MyMoneyMoney()); // dividend payments don't affect the number of shares
            d->stockSplit.setValue(MyMoneyMoney());
            d->stockSplit.setPrice(MyMoneyMoney());
        } else if (type != eMyMoney::Split::InvestmentTransactionType::Dividend && oldType == eMyMoney::Split::InvestmentTransactionType::Dividend) {
            // switch away from dividend
            d->stockSplit.setShares(d->ui->sharesAmountEdit->shares());
            d->stockSplit.setPrice(d->ui->priceAmountEdit->value());
            d->stockSplit.setValue(d->stockSplit.shares() * d->stockSplit.price());
        }

        updateTotalAmount();
        d->updateWidgetState();
        Q_EMIT editorLayoutChanged();
    }
}

MyMoneyMoney InvestTransactionEditor::totalAmount() const
{
    return d->assetSplit.value();
}

QStringList InvestTransactionEditor::saveTransaction(const QStringList& selectedJournalEntries)
{
    MyMoneyTransaction t;

    auto selection(selectedJournalEntries);
    connect(MyMoneyFile::instance()->journalModel(), &JournalModel::idChanged, this, [&](const QString& currentId, const QString& previousId) {
        selection.replaceInStrings(previousId, currentId);
    });

    AlkValue::RoundingMethod roundingMethod = AlkValue::RoundRound;
    if (d->security.roundingMethod() != AlkValue::RoundNever)
        roundingMethod = d->security.roundingMethod();

    int currencyFraction = d->transactionCurrency.smallestAccountFraction();
    int securityFraction = d->security.smallestAccountFraction();

    auto roundSplitValues = [&](MyMoneySplit& split, int sharesFraction) {
        split.setShares(MyMoneyMoney(split.shares().convertDenominator(sharesFraction, roundingMethod)));
        split.setValue(MyMoneyMoney(split.value().convertDenominator(currencyFraction, roundingMethod)));
    };

    if (!d->transaction.id().isEmpty()) {
        t = d->transaction;
    } else {
        // we keep the date when adding a new transaction
        // for the next new one
        KMyMoneySettings::setLastUsedPostDate(d->ui->dateEdit->date().startOfDay());
    }

    d->removeUnusedSplits(t, d->feeSplitModel);
    d->removeUnusedSplits(t, d->interestSplitModel);

    // we start with the previous values, clear id to make sure
    // we can add them later on
    d->stockSplit.clearId();
    d->assetSplit.clearId();

    t.setCommodity(d->transactionCurrency.id());

    t.removeSplits();

    t.setPostDate(d->ui->dateEdit->date());
    d->stockSplit.setMemo(d->ui->memoEdit->toPlainText());
    d->stockSplit.setAction(d->currentActivity->actionString());

    d->currentActivity->adjustStockSplit(d->stockSplit);

    QList<MyMoneySplit> resultSplits;  // concatenates splits for easy processing

    // now update and add what we have in the models
    if (d->currentActivity->assetAccountRequired() != Invest::Activity::Unused) {
        d->assetSplit.clearId();
        roundSplitValues(d->assetSplit, currencyFraction);
        t.addSplit(d->assetSplit);
    }

    // Don't do any rounding on a split factor
    if (d->currentActivity->type() != eMyMoney::Split::InvestmentTransactionType::SplitShares) {
        roundSplitValues(d->stockSplit, securityFraction);
        // if there are no shares, we don't have a price either
        if (!d->stockSplit.shares().isZero()) {
            if (d->currentActivity->priceMode() == eDialogs::PriceMode::PricePerTransaction) {
                d->stockSplit.setPrice(d->stockSplit.value() / d->stockSplit.shares());
            } else {
                d->stockSplit.setPrice(d->ui->priceAmountEdit->value());
            }
        }
    }

    if (d->stockSplit.reconcileFlag() != eMyMoney::Split::State::Reconciled && !d->stockSplit.reconcileDate().isValid()
        && d->ui->statusCombo->currentIndex() == (int)eMyMoney::Split::State::Reconciled) {
        d->stockSplit.setReconcileDate(QDate::currentDate());
    }
    d->stockSplit.setReconcileFlag(static_cast<eMyMoney::Split::State>(d->ui->statusCombo->currentIndex()));

    t.addSplit(d->stockSplit);

    if (d->currentActivity->feesRequired() != Invest::Activity::Unused) {
        resultSplits.append(d->feeSplitModel->splitList());
    }
    if (d->currentActivity->interestRequired() != Invest::Activity::Unused) {
        resultSplits.append(d->interestSplitModel->splitList());
    }

    // assuming that all non-stock splits are monetary
    for (auto& split : resultSplits) {
        split.clearId();
        roundSplitValues(split, currencyFraction);
        t.addSplit(split);
    }

    MyMoneyFileTransaction ft;
    try {
        const auto file = MyMoneyFile::instance();
        if (t.id().isEmpty()) {
            file->addTransaction(t);
        } else {
            t.setImported(false);
            file->modifyTransaction(t);
        }
        ft.commit();

    } catch (const MyMoneyException& e) {
        qDebug() << Q_FUNC_INFO << "something went wrong" << e.what();
        selection = selectedJournalEntries;
    }
    return selection;
}

bool InvestTransactionEditor::eventFilter(QObject* o, QEvent* e)
{
    auto cb = qobject_cast<QComboBox*>(o);
    if (cb) {
        // filter out wheel events for combo boxes if the popup view is not visible
        if ((e->type() == QEvent::Wheel) && !cb->view()->isVisible()) {
            return true;
        }
        if (e->type() == QEvent::FocusOut) {
            if (o == d->ui->feesCombo) {
                if (!d->ui->feesCombo->popup()->isVisible() && !cb->currentText().isEmpty() && !d->ui->feesCombo->lineEdit()->isReadOnly()) {
                    const auto accountId = d->ui->feesCombo->getSelected();
                    const auto accountIdx = MyMoneyFile::instance()->accountsModel()->indexById(accountId);
                    if (!accountIdx.isValid() || accountIdx.data(eMyMoney::Model::AccountFullNameRole).toString().compare(cb->currentText())) {
                        d->createFeeCategory();
                    }
                }
            } else if (o == d->ui->interestCombo) {
                if (!d->ui->interestCombo->popup()->isVisible() && !cb->currentText().isEmpty() && !d->ui->interestCombo->lineEdit()->isReadOnly()) {
                    const auto accountId = d->ui->interestCombo->getSelected();
                    const auto accountIdx = MyMoneyFile::instance()->accountsModel()->indexById(accountId);
                    if (!accountIdx.isValid() || accountIdx.data(eMyMoney::Model::AccountFullNameRole).toString().compare(cb->currentText())) {
                        d->createInterestCategory();
                    }
                }
            } else if (o == d->ui->assetAccountCombo) {
                if (!d->ui->assetAccountCombo->popup()->isVisible() && !cb->currentText().isEmpty() && !d->ui->assetAccountCombo->lineEdit()->isReadOnly()) {
                    const auto accountId = d->ui->assetAccountCombo->getSelected();
                    const auto accountIdx = MyMoneyFile::instance()->accountsModel()->indexById(accountId);
                    if (!accountIdx.isValid() || accountIdx.data(eMyMoney::Model::AccountFullNameRole).toString().compare(cb->currentText())) {
                        d->createAssetAccount();
                    }
                }
            }
        }
    }
    return QWidget::eventFilter(o, e);
}

void InvestTransactionEditor::setupUi(QWidget* parent)
{
    if (d->tabOrderUi == nullptr) {
        d->tabOrderUi = new Ui::InvestTransactionEditor;
    }
    d->tabOrderUi->setupUi(parent);
    d->tabOrderUi->infoMessage->setHidden(true);
}

void InvestTransactionEditor::storeTabOrder(const QStringList& tabOrder)
{
    TransactionEditorBase::storeTabOrder(QLatin1String("investTransactionEditor"), tabOrder);
}

void InvestTransactionEditor::slotSettingsChanged()
{
    d->securityFilterModel->setHideClosedAccounts(!KMyMoneySettings::showAllAccounts());
}
