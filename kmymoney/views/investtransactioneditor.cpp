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
#include <QKeyEvent>
#include <QSortFilterProxyModel>
#include <QStringList>
#include <QStringListModel>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KDescendantsProxyModel>

// ----------------------------------------------------------------------------
// Project Includes

#include "icons.h"
#include "investactivities.h"
#include "journalmodel.h"
#include "kcurrencyconverter.h"
#include "kmymoneysettings.h"
#include "kmymoneyutils.h"
#include "mymoneyaccount.h"
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
#include "taborder.h"
#include "ui_investtransactioneditor.h"
#include "widgethintframe.h"

using namespace Icons;

class InvestTransactionEditor::Private
{
    Q_DISABLE_COPY_MOVE(Private)

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
        , feeSplitHelper(nullptr)
        , interestSplitHelper(nullptr)
        , currentActivity(nullptr)
        , feeSplitModel(new SplitModel(parent, &undoStack))
        , interestSplitModel(new SplitModel(parent, &undoStack))
        , loadedFromModel(false)
        , bypassUserPriceUpdate(false)
        , m_tabOrder(QLatin1String("investTransactionEditor"),
                     QStringList{
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
                     })
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
    bool checkForValidTransaction(bool doUserInteraction = true);

    void setSecurity(const MyMoneySecurity& sec);

    bool amountChanged(SplitModel* model, AmountEdit* widget, const MyMoneyMoney& transactionFactor);
    bool isDividendOrYield(eMyMoney::Split::InvestmentTransactionType type) const;

    void scheduleUpdateTotalAmount();
    void updateWidgetState();
    void protectWidgetsForClosedAccount();

    void editSplits(SplitModel* sourceSplitModel, AmountEdit* amountEdit, const MyMoneyMoney& transactionFactor);
    void removeUnusedSplits(MyMoneyTransaction& t, SplitModel* splitModel);
    void addSplits(MyMoneyTransaction& t, SplitModel* splitModel);
    void setupParentInvestmentAccount(const QString& accountId);
    QModelIndex adjustToSecuritySplitIdx(const QModelIndex& idx);

    void loadFeeAndInterestAmountEdits();
    void adjustSharesCommodity(AmountEdit* amountEdit, const QString& accountId);
    void setupAssetAccount(const QString& accountId);
    void storePrice() const;
    void updateTotalAmount();

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

    bool loadedFromModel;

    /**
     * Flag to bypass the user dialog to modify exchange rate information.
     * This is used during the loading of a transaction, when data is
     * changed due to the load operation but no user interaction is
     * wanted.
     */
    bool bypassUserPriceUpdate;

    TabOrder m_tabOrder;
};

void InvestTransactionEditor::Private::removeUnusedSplits(MyMoneyTransaction& t, SplitModel* splitModel)
{
    // iterate over a copy of the splits as
    // the one in the transaction gets modified here
    const auto splits = t.splits();
    for (const auto& sp : splits) {
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
    WidgetHintFrame::hide(ui->dateEdit);

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
                amountEdit->setShares(sharesAmount);
                if (!sharesAmount.isZero()) {
                    q->updateConversionRate(amountEdit);
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

bool InvestTransactionEditor::Private::checkForValidTransaction(bool doUserInteraction)
{
    QStringList infos;
    bool rc = true;
    if (!postdateChanged(ui->dateEdit->date())) {
        infos << ui->dateEdit->toolTip();
        rc = false;
    }

    if (q->needCreateCategory(ui->feesCombo) || q->needCreateCategory(ui->interestCombo) || q->needCreateCategory(ui->assetAccountCombo)) {
        rc = false;
    }

    if (doUserInteraction) {
        /// @todo add dialog here that shows the @a infos about the problem
    }
    return rc;
}

void InvestTransactionEditor::Private::setSecurity(const MyMoneySecurity& sec)
{
    if (sec.tradingCurrency() != security.tradingCurrency()) {
        transactionCurrency = MyMoneyFile::instance()->currency(sec.tradingCurrency());
        ui->totalAmountEdit->setValueCommodity(transactionCurrency);
        ui->priceAmountEdit->setCommodity(transactionCurrency);
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
            currentActivity->updateLabelText();
            updateTotalAmount();
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
    ui->sharesAmountEdit->setPrecision(MyMoneyMoney::denomToPrec(security.smallestAccountFraction()), true);
    ui->priceAmountEdit->setPrecision(security.pricePrecision(), true);
}

bool InvestTransactionEditor::Private::amountChanged(SplitModel* model, AmountEdit* amountEdit, const MyMoneyMoney& transactionFactor)
{
    bool rc = true;
    if (!amountEdit->text().isEmpty() && (model->rowCount() <= 1)) {
        try {
            MyMoneyMoney shares;
            if (model->rowCount() == 1) {
                const auto index = model->index(0, 0);

                // check if there is a change in the values other than simply reverting the sign
                // and get an updated price in that case
                if (!bypassUserPriceUpdate) {
                    if ((index.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>() != -amountEdit->shares())
                        || (index.data(eMyMoney::Model::SplitValueRole).value<MyMoneyMoney>() != -amountEdit->value())) {
                        q->updateConversionRate(amountEdit);
                    }
                }

                model->setData(index, QVariant::fromValue<MyMoneyMoney>((amountEdit->value() * transactionFactor)), eMyMoney::Model::SplitValueRole);
                model->setData(index, QVariant::fromValue<MyMoneyMoney>((amountEdit->shares() * transactionFactor)), eMyMoney::Model::SplitSharesRole);
            }

        } catch (MyMoneyException&) {
            rc = false;
            qDebug() << "Ooops: something went wrong in" << Q_FUNC_INFO;
        }
    } else {
        /// @todo ask what to do: if the rest of the splits is the same amount we could simply reverse the sign
        /// of all splits, otherwise we could ask if the user wants to start the split editor or anything else.
    }
    return rc;
}

bool InvestTransactionEditor::Private::isDividendOrYield(eMyMoney::Split::InvestmentTransactionType type) const
{
    return (type == eMyMoney::Split::InvestmentTransactionType::Dividend) || (type == eMyMoney::Split::InvestmentTransactionType::Yield);
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

        // skip price update by user while returning back
        // from the split dialog
        bypassUserPriceUpdate = true;

        // in case the new model has only one split, we need to update
        // the amount widget with the values in the splitModel so that
        // they are available during further processing (copying calls
        // InvestTransactionEditor::Private::categoryChanged through
        // signals which needs it)
        if (splitModel.rowCount() == 1) {
            const auto idx = splitModel.index(0, 0);
            amountEdit->setValue(idx.data(eMyMoney::Model::SplitValueRole).value<MyMoneyMoney>());
            amountEdit->setShares(idx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>());
            // make sure that the commodity of the shares is changed to the current selected account
            const auto accountId = idx.data(eMyMoney::Model::SplitAccountIdRole).toString();
            const auto accountIdx = MyMoneyFile::instance()->accountsModel()->indexById(accountId);
            const auto currencyId = accountIdx.data(eMyMoney::Model::AccountCurrencyIdRole).toString();
            const auto currency = MyMoneyFile::instance()->currenciesModel()->itemById(currencyId);
            // switch to value display so that we show the transaction commodity
            // for single currency data entry this does not have an effect
            amountEdit->setDisplayState(MultiCurrencyEdit::DisplayValue);
            amountEdit->setSharesCommodity(currency);
        }

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

        // reactivate the price update for the use
        bypassUserPriceUpdate = false;

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

void InvestTransactionEditor::Private::protectWidgetsForClosedAccount()
{
    const auto securityAccont = MyMoneyFile::instance()->accountsModel()->itemById(stockSplit.accountId());
    const bool closed = securityAccont.isClosed();
    ui->sharesAmountEdit->setReadOnly(closed);
    ui->securityAccountCombo->setDisabled(closed);
    ui->activityCombo->setDisabled(closed);
}

void InvestTransactionEditor::Private::updateWidgetState()
{
    WidgetHintFrame::hide(ui->feesCombo);
    WidgetHintFrame::hide(ui->feesAmountEdit);
    WidgetHintFrame::hide(ui->interestCombo);
    WidgetHintFrame::hide(ui->interestAmountEdit);
    WidgetHintFrame::hide(ui->assetAccountCombo);
    WidgetHintFrame::hide(ui->priceAmountEdit);
    WidgetHintFrame::hide(ui->totalAmountEdit);

    if (ui->securityAccountCombo->isEnabled()) {
        WidgetHintFrame::hide(ui->securityAccountCombo);
        ui->activityCombo->setToolTip(i18nc("@info:tooltip", "Select the activity for this transaction."));
    } else {
        WidgetHintFrame::hide(ui->securityAccountCombo,
                              i18nc("@info:tooltip", "The security for this transaction cannot be modified because the security account is closed."));
        // ui->activityCombo->setToolTip(i18nc("@info:tooltip", "This activity cannot be modified because the security account is closed."));
    }

    // all the other logic needs a valid activity
    if (currentActivity == nullptr) {
        return;
    }

    const auto widget = ui->sharesAmountEdit;
    switch(currentActivity->type()) {
    default:
        if (ui->securityAccountCombo->isEnabled()) {
            WidgetHintFrame::hide(widget, i18nc("@info:tooltip", "Number of shares"));
        } else {
            WidgetHintFrame::hide(widget, i18nc("@info:tooltip", "The number of shares cannot be modified because the security account is closed."));
        }
        if (widget->isVisible()) {
            if (widget->value().isZero() || widget->text().isEmpty()) {
                WidgetHintFrame::show(widget, i18nc("@info:tooltip", "Enter number of shares for this transaction"));
            }
        }
        break;
    case eMyMoney::Split::InvestmentTransactionType::SplitShares:
        if (ui->securityAccountCombo->isEnabled()) {
            WidgetHintFrame::hide(widget, i18nc("@info:tooltip", "Split ratio"));
        } else {
            WidgetHintFrame::hide(widget, i18nc("@info:tooltip", "The split ratio cannot be modified because the security account is closed."));
        }
        if (widget->isVisible()) {
            if (widget->value().isZero() || widget->text().isEmpty()) {
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
        if (ui->priceAmountEdit->value().isZero() || ui->priceAmountEdit->text().isEmpty()) {
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

    if (currentActivity->type() == eMyMoney::Split::InvestmentTransactionType::ReinvestDividend && !ui->totalAmountEdit->value().isZero()) {
        if (ui->assetAccountCombo->getSelected().isEmpty()) {
            WidgetHintFrame::show(ui->totalAmountEdit, i18nc("@info:tooltip", "Transaction is not balanced. Either correct values or assign an account."));
        }
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

void InvestTransactionEditor::Private::storePrice() const
{
    if ((stockSplit.action() == QLatin1String("Buy")) || (stockSplit.action() == QLatin1String("Reinvestset"))) {
        const auto id = stockSplit.accountId();
        const auto file = MyMoneyFile::instance();
        const auto acc = file->account(id);
        MyMoneySecurity sec = file->security(acc.currencyId());
        MyMoneyPrice price(acc.currencyId(), sec.tradingCurrency(), ui->dateEdit->date(), stockSplit.price(), "Transaction");
        file->addPrice(price);
    }
}

void InvestTransactionEditor::Private::updateTotalAmount()
{
    stockSplit.setValue(currentActivity->valueAllShares().convert(transactionCurrency.smallestAccountFraction(), security.roundingMethod())
                        * currentActivity->sharesFactor());
    if (currentActivity->type() != eMyMoney::Split::InvestmentTransactionType::SplitShares) {
        scheduleUpdateTotalAmount();
    }
    updateWidgetState();
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
    d->securityFilterModel->setShowSecuritySymbols(true);

    d->accountsListModel->setSourceModel(d->securityFilterModel);

    d->securitiesModel->setSourceModel(d->accountsListModel);
    d->securitiesModel->setFilterRole(eMyMoney::Model::AccountParentIdRole);
    d->securitiesModel->setFilterKeyColumn(0);
    d->securitiesModel->setSortRole(Qt::DisplayRole);
    d->securitiesModel->setSortLocaleAware(true);
    d->securitiesModel->sort(AccountsModel::Column::AccountName);

    d->ui->securityAccountCombo->setModel(d->securitiesModel);
    d->ui->securityAccountCombo->lineEdit()->setClearButtonEnabled(true);
    d->ui->securityAccountCombo->completer()->setCompletionMode(QCompleter::PopupCompletion);
    d->ui->securityAccountCombo->completer()->setCaseSensitivity(Qt::CaseInsensitive);
    d->ui->securityAccountCombo->completer()->setFilterMode(Qt::MatchContains);

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
            d->updateTotalAmount();
        }
    });

    d->ui->priceAmountEdit->setAllowEmpty(true);
    d->ui->priceAmountEdit->setCalculatorButtonVisible(true);
    d->ui->priceAmountEdit->setPrecisionOverridesFraction(true);

    connect(d->ui->priceAmountEdit, &AmountEdit::amountChanged, this, [&]() {
        if (d->currentActivity) {
            d->updateTotalAmount();
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
    frameCollection->addFrame(new WidgetHintFrame(d->ui->totalAmountEdit, WidgetHintFrame::Warning));
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
        if (d->currentActivity) {
            d->currentActivity->updateLabelText();
            d->updateTotalAmount();
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
            d->scheduleUpdateTotalAmount();
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
            d->scheduleUpdateTotalAmount();
        },
        Qt::QueuedConnection);

    connect(d->ui->cancelButton, &QToolButton::clicked, this, [&]() {
        Q_EMIT done();
    });
    connect(d->ui->enterButton, &QToolButton::clicked, this, &InvestTransactionEditor::acceptEdit);

    connect(accountsModel, &QAbstractItemModel::dataChanged, this, [&]() {
        d->protectWidgetsForClosedAccount();
        d->updateWidgetState();
    });

    // handle some events in certain conditions different from default
    d->ui->activityCombo->installEventFilter(this);
    d->ui->statusCombo->installEventFilter(this);
    d->ui->feesCombo->installEventFilter(this);
    d->ui->interestCombo->installEventFilter(this);
    d->ui->assetAccountCombo->installEventFilter(this);
    d->ui->securityAccountCombo->installEventFilter(this);
    d->ui->memoEdit->installEventFilter(this);

    // make sure that an empty widget causes the hint frame to show
    d->ui->sharesAmountEdit->installEventFilter(this);
    d->ui->priceAmountEdit->installEventFilter(this);
    d->ui->feesAmountEdit->installEventFilter(this);
    d->ui->interestAmountEdit->installEventFilter(this);

    d->ui->totalAmountEdit->setCalculatorButtonVisible(false);

    // in case the total is in a different currency, we do allow
    // to modify the amount for the brokerage account.
    d->ui->totalAmountEdit->setAllowModifyShares(true);

    d->setupParentInvestmentAccount(accId);

    setCancelButton(d->ui->cancelButton);
    setEnterButton(d->ui->enterButton);

    connect(d->ui->totalAmountEdit, &AmountEdit::amountChanged, this, [&]() {
        if (!d->ui->totalAmountEdit->shares().isZero()) {
            const auto rate = d->ui->totalAmountEdit->value() / d->ui->totalAmountEdit->shares();
            d->assetPrice = MyMoneyPrice(d->transactionCurrency.id(), d->assetAccount.currencyId(), d->transaction.postDate(), rate, QLatin1String("KMyMoney"));
            updateTotalAmount();
        }
    });

    d->m_tabOrder.setWidget(this);

    slotSettingsChanged();
}

InvestTransactionEditor::~InvestTransactionEditor()
{
}

void InvestTransactionEditor::updateTotalAmount()
{
    // update widget state according to current scenario
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
                updateConversionRate(d->ui->totalAmountEdit);

                // since the total amount is kept as a positive number, we may
                // need to adjust the sign of the shares. The value nevertheless
                // has the correct sign. So if the sign does not match, we
                // simply revert the sign of the shares.
                if (d->assetSplit.shares().isNegative() != d->assetSplit.value().isNegative()) {
                    d->assetSplit.setShares(-d->assetSplit.shares());
                }
            }
        }
        // update widget state again, because the conditions may have changed
        d->updateWidgetState();
    }
}


void InvestTransactionEditor::loadTransaction(const QModelIndex& index)
{
    // we may also get here during saving the transaction as
    // a callback from the model, but we can safely ignore it
    // same when we get called from the delegate's setEditorData()
    // method
    if (accepted() || !index.isValid() || d->loadedFromModel)
        return;

    d->loadedFromModel = true;

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
        d->ui->priceAmountEdit->setCommodity(MyMoneySecurity());
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

        // make sure to set the precision before the value gets loaded
        d->ui->priceAmountEdit->setPrecision(d->security.pricePrecision(), true);
        d->currentActivity->loadPriceWidget(d->stockSplit);

        // check if security and amount of shares needs to be
        // protected because the security account is closed
        d->protectWidgetsForClosedAccount();

        updateTotalAmount();

        d->currentActivity->consistencyCheck();
    }

    d->bypassUserPriceUpdate = false;

    // delay update until next run of event loop so that all necessary widgets are visible
    QMetaObject::invokeMethod(this, "updateWidgets", Qt::QueuedConnection);

    setInitialFocus();
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
            d->currentActivity = new Invest::Div(this);
            break;
        case eMyMoney::Split::InvestmentTransactionType::Yield:
            d->currentActivity = new Invest::Yield(this);
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
            d->stockSplit.setValue(d->stockSplit.shares() * d->stockSplit.possiblyCalculatedPrice());
            d->ui->sharesAmountEdit->setPrecision(MyMoneyMoney::denomToPrec(d->security.smallestAccountFraction()));
        }

        if (d->isDividendOrYield(type) && !d->isDividendOrYield(oldType)) {
            // switch to dividend/yield
            d->stockSplit.setShares(MyMoneyMoney()); // dividend payments don't affect the number of shares
            d->stockSplit.setValue(MyMoneyMoney());
            d->stockSplit.setPrice(MyMoneyMoney());
        } else if (!d->isDividendOrYield(type) && d->isDividendOrYield(oldType)) {
            // switch away from dividend/yield
            d->stockSplit.setShares(d->ui->sharesAmountEdit->shares());
            d->stockSplit.setPrice(d->ui->priceAmountEdit->value());
            d->stockSplit.setValue(d->stockSplit.shares() * d->stockSplit.possiblyCalculatedPrice());
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

    updateTotalAmount();

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
    d->assetSplit.setMemo(d->ui->memoEdit->toPlainText());
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
            if (d->currentActivity->priceMode() == eMyMoney::Invest::PriceMode::PricePerTransaction) {
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
            d->storePrice();
            selection = journalEntrySelection(t.id(), d->stockSplit.accountId());
        } else {
            t.setImported(false);
            file->modifyTransaction(t);
            d->storePrice();
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
    auto ae = qobject_cast<AmountEdit*>(o);
    if (cb) {
        if (e->type() == QEvent::KeyPress) {
            // the activity combo needs special handling, because it does
            // not process the return/enter key directly but ignores it
            if (o == d->ui->activityCombo) {
                const auto key = static_cast<QKeyEvent*>(e)->key();
                if (key == Qt::Key_Return || key == Qt::Key_Enter) {
                    processReturnKey();
                }
            }
        }
        if (e->type() == QEvent::FocusOut) {
            if (o == d->ui->feesCombo) {
                if (needCreateCategory(d->ui->feesCombo)) {
                    createCategory(d->ui->feesCombo, eMyMoney::Account::Type::Expense);
                }
            } else if (o == d->ui->interestCombo) {
                if (needCreateCategory(d->ui->interestCombo)) {
                    createCategory(d->ui->interestCombo, eMyMoney::Account::Type::Income);
                }
            } else if (o == d->ui->assetAccountCombo) {
                if (needCreateCategory(d->ui->assetAccountCombo)) {
                    createCategory(d->ui->assetAccountCombo, eMyMoney::Account::Type::Asset);
                }
            } else if (o == d->ui->securityAccountCombo) {
                const auto accountId = d->ui->securityAccountCombo->completer()->currentIndex().data(eMyMoney::Model::IdRole).toString();
                const auto indexes = d->securitiesModel->match(d->securitiesModel->index(0, 0), eMyMoney::Model::IdRole, accountId);
                if (indexes.count() == 1) {
                    d->ui->securityAccountCombo->setCurrentIndex(indexes.at(0).row());
                }
            }
            updateWidgets();
        }
    }

    if (ae && (e->type() == QEvent::FocusOut)) {
        // in case an AmountEdit is cleared, the value must also be zero
        if (!ae->shares().isZero() && ae->text().isEmpty()) {
            ae->clear();
        }
        updateWidgets();
    }

    return TransactionEditorBase::eventFilter(o, e);
}

QWidget* InvestTransactionEditor::setupUi(QWidget* parent)
{
    if (d->tabOrderUi == nullptr) {
        d->tabOrderUi = new Ui::InvestTransactionEditor;
    }
    d->tabOrderUi->setupUi(parent);
    d->tabOrderUi->infoMessage->setHidden(true);
    return this;
}

void InvestTransactionEditor::storeTabOrder(const QStringList& tabOrder)
{
    d->m_tabOrder.setTabOrder(tabOrder);
}

void InvestTransactionEditor::slotSettingsChanged()
{
    TransactionEditorBase::slotSettingsChanged();

    d->securityFilterModel->setHideClosedAccounts(!KMyMoneySettings::showAllAccounts());
}

bool InvestTransactionEditor::isTransactionDataValid() const
{
    return d->checkForValidTransaction(false);
}

QDate InvestTransactionEditor::postDate() const
{
    return d->ui->dateEdit->date();
}

TabOrder* InvestTransactionEditor::tabOrder() const
{
    return &d->m_tabOrder;
}
