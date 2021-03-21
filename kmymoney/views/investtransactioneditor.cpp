/*
    SPDX-FileCopyrightText: 2019-2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
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
        , activitiesModel(new QStringListModel(parent))
        , securitiesModel(new QSortFilterProxyModel(parent))
        , accountsListModel(new KDescendantsProxyModel(parent))
        , currentActivity(nullptr)
        , feeSplitModel(new SplitModel(parent, &undoStack))
        , interestSplitModel(new SplitModel(parent, &undoStack))
        , accepted(false)
        , bypassPriceEditor(false)
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

    void setSecurity(const MyMoneySecurity& sec);

    bool valueChanged(const SplitModel* model, AmountEdit* widget);

    void updateWidgetState();

    MyMoneyMoney getPrice(const SplitModel* model, const AmountEdit* widget);

    void editSplits(SplitModel* splitModel, AmountEdit* editWidget, const MyMoneyMoney& factor);
    void removeUnusedSplits(MyMoneyTransaction& t, SplitModel* splitModel);
    void addSplits(MyMoneyTransaction& t, SplitModel* splitModel);
    void setupParentInvestmentAccount(const QString& accountId);
    QModelIndex adjustToSecuritySplitIdx(const QModelIndex& idx);

    InvestTransactionEditor*      q;
    Ui_InvestTransactionEditor*   ui;

    // models for UI elements
    AccountNamesFilterProxyModel* accountsModel;
    AccountNamesFilterProxyModel* feesModel;
    AccountNamesFilterProxyModel* interestModel;
    QStringListModel*             activitiesModel;
    QSortFilterProxyModel*        securitiesModel;
    KDescendantsProxyModel*       accountsListModel;

    QUndoStack                    undoStack;
    Invest::Activity*             currentActivity;

    QSet<AmountEditCurrencyHelper*> amountEditCurrencyHelpers;

    // the selected security and the account holding it
    MyMoneySecurity               security;
    // and its trading currency
    MyMoneySecurity               currency;
    // and account holding the security
    MyMoneyAccount                stockAccount;

    MyMoneyAccount                assetAccount;

    // the containing investment account (parent of stockAccount)
    MyMoneyAccount                parentAccount;

    // the transaction
    MyMoneyTransaction            transaction;

    // the various splits
    MyMoneySplit                  stockSplit;
    MyMoneySplit                  assetSplit;
    SplitModel*                   feeSplitModel;
    SplitModel*                   interestSplitModel;

    // exchange rate information for assetSplit
    MyMoneyPrice                  assetPrice;

    bool                          accepted;
    bool                          bypassPriceEditor;
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
    } catch (MyMoneyException& e) {
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

bool InvestTransactionEditor::Private::categoryChanged(SplitModel* model, const QString& accountId, AmountEdit* widget, const MyMoneyMoney& factor)
{
    bool rc = true;
    if (!accountId.isEmpty() && model->rowCount() <= 1) {
        try {
            MyMoneyAccount category = MyMoneyFile::instance()->account(accountId);

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

void InvestTransactionEditor::Private::setSecurity(const MyMoneySecurity& sec)
{
    if (sec.tradingCurrency() != security.tradingCurrency()) {
        for (const auto helper : qAsConst(amountEditCurrencyHelpers)) {
            helper->setCommodity(sec.tradingCurrency());
        }
        transaction.setCommodity(sec.tradingCurrency());
        currency = MyMoneyFile::instance()->currency(sec.tradingCurrency());

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

        if (assetPrice.from() != currency.id()) {
            /// @todo collect exchange rate from user for asset account
        }

        bool needWarning = !assetSplit.value().isZero();
        if (currentActivity) {
            needWarning |= ((currentActivity->feesRequired() != Invest::Activity::Unused) && haveValue(feeSplitModel));
            needWarning |= ((currentActivity->interestRequired() != Invest::Activity::Unused) && haveValue(interestSplitModel));
        }

        if (needWarning) {
            ui->infoMessage->setText(i18nc("@info:usagetip", "The transaction commodity has been changed which will possibly make all price information invalid. Please check them."));
            if (!ui->infoMessage->isShowAnimationRunning()) {
                ui->infoMessage->animatedShow();
                emit q->editorLayoutChanged();
            }
        }
    }

    security = sec;

    // update the precision to that used by the new security
    ui->sharesAmountEdit->setPrecision(MyMoneyMoney::denomToPrec(security.smallestAccountFraction()));
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
            /// @todo I think security in the following three occurrences needs to be replaced with currency
            if (sec.id() != security.id()) {
                if (result == MyMoneyMoney::ONE) {
                    result = MyMoneyFile::instance()->price(sec.id(), security.id(), QDate()).rate(sec.id());
                }

                QPointer<KCurrencyCalculator> calc =
                    new KCurrencyCalculator(sec,
                                            security,
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

    QPointer<SplitDialog> splitDialog = new SplitDialog(parentAccount, security, MyMoneyMoney::autoCalc, transactionFactor, q);
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
    d->ui->securityAccountCombo->setModel(d->securitiesModel);
    d->ui->securityAccountCombo->lineEdit()->setReadOnly(true);

    d->accountsModel->addAccountGroup(QVector<eMyMoney::Account::Type> { eMyMoney::Account::Type::Asset, eMyMoney::Account::Type::Liability } );
    d->accountsModel->setHideEquityAccounts(false);
    d->accountsModel->setSourceModel(model);
    d->accountsModel->sort(AccountsModel::Column::AccountName);
    d->ui->assetAccountCombo->setModel(d->accountsModel);
    d->ui->assetAccountCombo->setSplitActionVisible(false);

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
    connect(d->ui->sharesAmountEdit, &AmountEdit::textChanged, this, &InvestTransactionEditor::sharesChanged);

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
    frameCollection->addFrame(new WidgetHintFrame(d->ui->assetAccountCombo));
    frameCollection->addFrame(new WidgetHintFrame(d->ui->sharesAmountEdit));
    frameCollection->addFrame(new WidgetHintFrame(d->ui->priceAmountEdit));
    frameCollection->addFrame(new WidgetHintFrame(d->ui->feesCombo));
    frameCollection->addFrame(new WidgetHintFrame(d->ui->feesAmountEdit));
    frameCollection->addFrame(new WidgetHintFrame(d->ui->interestCombo));
    frameCollection->addFrame(new WidgetHintFrame(d->ui->interestAmountEdit));
    frameCollection->addWidget(d->ui->enterButton);


    connect(d->ui->assetAccountCombo, &KMyMoneyAccountCombo::accountSelected, this, &InvestTransactionEditor::assetAccountChanged);
    connect(d->ui->dateEdit, &KMyMoneyDateEdit::dateChanged, this, &InvestTransactionEditor::postdateChanged);
    connect(d->ui->activityCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &InvestTransactionEditor::activityChanged);
    connect(d->ui->securityAccountCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &InvestTransactionEditor::securityAccountChanged);

    connect(d->ui->feesCombo, &KMyMoneyAccountCombo::accountSelected, this, &InvestTransactionEditor::feeCategoryChanged);
    connect(d->ui->feesCombo, &KMyMoneyAccountCombo::splitDialogRequest, this, &InvestTransactionEditor::editFeeSplits, Qt::QueuedConnection);

    connect(d->ui->interestCombo, &KMyMoneyAccountCombo::accountSelected, this, &InvestTransactionEditor::interestCategoryChanged);
    connect(d->ui->interestCombo, &KMyMoneyAccountCombo::splitDialogRequest, this, &InvestTransactionEditor::editInterestSplits, Qt::QueuedConnection);

    /// @todo convert to new signal/slot syntax
    connect(d->ui->cancelButton, &QToolButton::clicked, this, [&]() {
        emit done();
    } );
    connect(d->ui->enterButton, &QToolButton::clicked, this, [&]() {
        d->accepted = true;
        emit done();
    } );

    // handle some events in certain conditions different from default
    d->ui->activityCombo->installEventFilter(this);
    d->ui->statusCombo->installEventFilter(this);

    d->ui->totalAmountEdit->setCalculatorButtonVisible(false);

    d->setupParentInvestmentAccount(accountId);

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

void InvestTransactionEditor::updateTotalAmount()
{
    d->updateWidgetState();
    if (d->currentActivity) {
        const auto totalAmount = d->currentActivity->totalAmount(d->stockSplit, d->feeSplitModel, d->interestSplitModel);
        d->ui->totalAmountEdit->setValue(totalAmount.abs());
        d->assetSplit.setValue(-totalAmount);
        d->assetSplit.setShares(d->assetSplit.value() / d->assetPrice.rate(d->assetAccount.currencyId()));
    }
}


void InvestTransactionEditor::loadTransaction(const QModelIndex& index)
{
    d->ui->activityCombo->setCurrentIndex(-1);
    d->ui->securityAccountCombo->setCurrentIndex(-1);
    const auto file = MyMoneyFile::instance();
    auto idx = d->adjustToSecuritySplitIdx(MyMoneyFile::baseModel()->mapToBaseSource(index));
    if (!idx.isValid() || idx.data(eMyMoney::Model::IdRole).toString().isEmpty()) {
        d->transaction = MyMoneyTransaction();
        d->transaction.setCommodity(d->parentAccount.currencyId());
        d->currency = MyMoneyFile::instance()->baseCurrency();
        d->security = MyMoneySecurity();
        d->security.setTradingCurrency(d->currency.id());
        d->stockSplit = MyMoneySplit();
        d->assetSplit = MyMoneySplit();
        d->assetAccount = MyMoneyAccount();
        d->ui->activityCombo->setCurrentIndex(0);
        d->ui->securityAccountCombo->setCurrentIndex(0);
        const auto lastUsedPostDate = KMyMoneySettings::lastUsedPostDate();
        if (lastUsedPostDate.isValid()) {
            d->ui->dateEdit->setDate(lastUsedPostDate.date());
        } else {
            d->ui->dateEdit->setDate(QDate::currentDate());
        }
        // select the associated brokerage account if it exists
        const auto brokerageAccount = file->accountsModel()->itemByName(d->parentAccount.brokerageName());
        if (!brokerageAccount.id().isEmpty()) {
            d->ui->assetAccountCombo->setSelected(brokerageAccount.id());
        }
    } else {
        // keep a copy of the transaction and split
        d->transaction = file->journalModel()->itemByIndex(idx).transaction();
        d->stockSplit = file->journalModel()->itemByIndex(idx).split();

        QModelIndex assetAccountSplitIdx;
        eMyMoney::Split::InvestmentTransactionType transactionType;

        KMyMoneyUtils::dissectInvestmentTransaction(idx, assetAccountSplitIdx, d->feeSplitModel, d->interestSplitModel, d->security, d->currency, transactionType);
        d->assetSplit = file->journalModel()->itemByIndex(assetAccountSplitIdx).split();
        if (!d->assetSplit.id().isEmpty())
            d->assetAccount = file->account(d->assetSplit.accountId());

        // extract conversion rate information for asset split before changing
        // the activity because that will need it (in updateTotalAmount() )
        if (!(d->assetSplit.shares().isZero() || d->assetSplit.value().isZero())) {
            const auto rate = d->assetSplit.value() / d->assetSplit.shares();
            d->assetPrice = MyMoneyPrice(d->currency.id(), d->assetAccount.currencyId(), d->transaction.postDate(), rate, QLatin1String("KMyMoney"));
        }

        // load the widgets. setting activityCombo also initializes
        // d->currentActivity to have the right object
        d->ui->activityCombo->setCurrentIndex(static_cast<int>(transactionType));
        d->ui->dateEdit->setDate(d->transaction.postDate());

        d->ui->memoEdit->setPlainText(d->stockSplit.memo());

        d->ui->assetAccountCombo->setSelected(d->assetSplit.accountId());

        d->ui->sharesAmountEdit->setPrecision(MyMoneyMoney::denomToPrec(d->security.smallestAccountFraction()));
        d->ui->sharesAmountEdit->setValue(d->stockSplit.shares() * d->currentActivity->sharesFactor());

        const auto indexes = d->securitiesModel->match(d->securitiesModel->index(0,0), eMyMoney::Model::IdRole, d->stockSplit.accountId(), 1, Qt::MatchFixedString);
        if (!indexes.isEmpty()) {
            d->ui->securityAccountCombo->setCurrentIndex(indexes.first().row());
            d->stockAccount = file->account(d->stockSplit.accountId());
        }

        d->ui->feesAmountEdit->setValue(d->feeSplitModel->valueSum() * d->currentActivity->feesFactor());
        d->ui->interestAmountEdit->setValue(d->interestSplitModel->valueSum() * d->currentActivity->interestFactor());

        d->currentActivity->loadPriceWidget(d->stockSplit);
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

void InvestTransactionEditor::securityAccountChanged(int index)
{
    const auto idx = d->ui->securityAccountCombo->model()->index(index, 0);
    if (idx.isValid()) {
        const auto accountId = idx.data(eMyMoney::Model::IdRole).toString();
        const auto securityId = idx.data(eMyMoney::Model::AccountCurrencyIdRole).toString();
        try {
            const auto file = MyMoneyFile::instance();
            const auto sec = file->security(securityId);

            d->stockAccount = file->account(accountId);
            d->stockSplit.setAccountId(accountId);
            d->setSecurity(sec);

            updateTotalAmount();

        } catch(MyMoneyException& e) {
            qDebug() << "Problem to find securityId" << accountId << "or" << securityId << "in InvestTransactionEditor::securityAccountChanged";
        }
    }
}


void InvestTransactionEditor::activityChanged(int index)
{
    const auto type = static_cast<eMyMoney::Split::InvestmentTransactionType>(index);
    if (!d->currentActivity || type != d->currentActivity->type()) {
        auto oldType = eMyMoney::Split::InvestmentTransactionType::UnknownTransactionType;
        if (d->currentActivity) {
            oldType = d->currentActivity->type();
        }
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

        if (type != eMyMoney::Split::InvestmentTransactionType::SplitShares &&
                oldType == eMyMoney::Split::InvestmentTransactionType::SplitShares) {
            // switch to split
            d->stockSplit.setValue(MyMoneyMoney());
            d->stockSplit.setPrice(MyMoneyMoney());
            d->ui->sharesAmountEdit->setPrecision(-1);
        } else if (type == eMyMoney::Split::InvestmentTransactionType::SplitShares &&
                   oldType != eMyMoney::Split::InvestmentTransactionType::SplitShares) {
            // switch away from split
            d->stockSplit.setPrice(d->ui->priceAmountEdit->value());
            d->stockSplit.setValue(d->stockSplit.shares() * d->stockSplit.price());
            d->ui->sharesAmountEdit->setPrecision(MyMoneyMoney::denomToPrec(d->security.smallestAccountFraction()));
        }
        updateTotalAmount();
        d->updateWidgetState();
        emit editorLayoutChanged();
    }
}

void InvestTransactionEditor::sharesChanged()
{
    if (d->currentActivity) {
        if (d->currentActivity->type() != eMyMoney::Split::InvestmentTransactionType::SplitShares) {
            updateTotalAmount();
        }
    }
}

void InvestTransactionEditor::assetAccountChanged(const QString& accountId)
{
    const auto account = MyMoneyFile::instance()->account(accountId);
    if (account.currencyId() != d->assetAccount.currencyId()) {
        /// @todo update price rate information
    }
    d->postdateChanged(d->ui->dateEdit->date());
    d->updateWidgetState();
}

void InvestTransactionEditor::feeCategoryChanged(const QString& accountId)
{
    d->categoryChanged(d->feeSplitModel, accountId, d->ui->feesAmountEdit, MyMoneyMoney::ONE);
    d->updateWidgetState();
    updateTotalAmount();
}

void InvestTransactionEditor::interestCategoryChanged(const QString& accountId)
{
    d->categoryChanged(d->interestSplitModel, accountId, d->ui->interestAmountEdit, MyMoneyMoney::MINUS_ONE);
    d->updateWidgetState();
    updateTotalAmount();
}

void InvestTransactionEditor::postdateChanged(const QDate& date)
{
    d->postdateChanged(date);
}

void InvestTransactionEditor::feesValueChanged()
{
    d->valueChanged(d->feeSplitModel, d->ui->feesAmountEdit);
    d->updateWidgetState();
    updateTotalAmount();
}

void InvestTransactionEditor::interestValueChanged()
{
    d->valueChanged(d->interestSplitModel, d->ui->interestAmountEdit);
    d->updateWidgetState();
    updateTotalAmount();
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
    return d->assetSplit.value();
}

void InvestTransactionEditor::saveTransaction()
{
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

    // we start with the previous values, clear id to make sure
    // we can add them later on
    d->stockSplit.clearId();

    t.setCommodity(d->currency.id());

    t.removeSplits();

    t.setPostDate(d->ui->dateEdit->date());
    d->stockSplit.setMemo(d->ui->memoEdit->toPlainText());

    d->currentActivity->adjustStockSplit(d->stockSplit);

    QList<MyMoneySplit> resultSplits;  // concatenates splits for easy processing

    // now update and add what we have in the model(s)
    if (d->currentActivity->assetAccountRequired() != Invest::Activity::Unused) {
        resultSplits.append(d->assetSplit);
    }
    if (d->currentActivity->feesRequired() != Invest::Activity::Unused) {
        addSplitsFromModel(resultSplits, d->feeSplitModel);
    }
    if (d->currentActivity->interestRequired() != Invest::Activity::Unused) {
        addSplitsFromModel(resultSplits, d->interestSplitModel);
    }

    AlkValue::RoundingMethod roundingMethod = AlkValue::RoundRound;
    if (d->security.roundingMethod() != AlkValue::RoundNever)
        roundingMethod = d->security.roundingMethod();

    int currencyFraction = d->currency.smallestAccountFraction();
    int securityFraction = d->security.smallestAccountFraction();

    // assuming that all non-stock splits are monetary
    foreach (auto split, resultSplits) {
        split.clearId();
        split.setShares(MyMoneyMoney(split.shares().convertDenominator(currencyFraction, roundingMethod)));
        split.setValue(MyMoneyMoney(split.value().convertDenominator(currencyFraction, roundingMethod)));
        t.addSplit(split);
    }

    // Don't do any rounding on a split factor
    if (d->currentActivity->type() != eMyMoney::Split::InvestmentTransactionType::SplitShares) {
        // only the shares variable of a stock split isn't evaluated in currency
        d->stockSplit.setShares(MyMoneyMoney(d->stockSplit.shares().convertDenominator(securityFraction, roundingMethod)));
        d->stockSplit.setValue(MyMoneyMoney(d->stockSplit.value().convertDenominator(currencyFraction, roundingMethod)));
    }
    t.addSplit(d->stockSplit);

    MyMoneyFileTransaction ft;
    try {
        const auto file = MyMoneyFile::instance();
        if (t.id().isEmpty()) {
            file->addTransaction(t);
        } else {
            file->modifyTransaction(t);
        }
        ft.commit();

    } catch (const MyMoneyException& e) {
        qDebug() << Q_FUNC_INFO << "something went wrong" << e.what();
    }
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

void InvestTransactionEditor::keyPressEvent(QKeyEvent* e)
{
    if (!e->modifiers() || ((e->modifiers() & Qt::KeypadModifier) && (e->key() == Qt::Key_Enter))) {
        switch (e->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return: {
            if (focusWidget() == d->ui->cancelButton) {
                d->ui->cancelButton->click();
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
            d->ui->cancelButton->click();
            break;

        default:
            e->ignore();
            return;
        }
    } else {
        e->ignore();
    }
}


// kate: indent-mode cstyle; indent-width 4; replace-tabs on; remove-trailing-space on;remove-trailing-space-save on;
