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
    void updateWidgetState();
    bool checkForValidTransaction(bool doUserInteraction = true);
    bool isDatePostOpeningDate(const QDate& date, const QString& accountId);

    bool postdateChanged(const QDate& date);
    bool categoryChanged(const QString& accountId);
    bool securityChanged(const QString& securityId);

    bool valueChanged(CreditDebitHelper* valueHelper);
    MyMoneyMoney getPrice();

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

void InvestTransactionEditor::Private::updateWidgetState()
{
    // setup the category/account combo box. If we have a split transaction, we disable the
    // combo box altogether. Changes can only be made via the split dialog editor
    bool blocked = false;
    QModelIndex index;

#if 0
    // update the category combo box
    ui->accountCombo->setEnabled(true);
    index = splitModel.index(0, 0);
    switch (splitModel.rowCount()) {
    case 0:
        ui->accountCombo->setSelected(QString());
        break;
    case 1:
        ui->accountCombo->setSelected(splitModel.data(index, eMyMoney::Model::SplitAccountIdRole).toString());
        break;
    default:
        blocked = ui->accountCombo->lineEdit()->blockSignals(true);
        ui->accountCombo->lineEdit()->setText(i18n("Split transaction"));
        ui->accountCombo->setDisabled(true);
        ui->accountCombo->lineEdit()->blockSignals(blocked);
        break;
    }
    ui->accountCombo->hidePopup();

    // update the costcenter combo box
    if (ui->costCenterCombo->isEnabled()) {
        // extract the cost center
        index = MyMoneyFile::instance()->costCenterModel()->indexById(splitModel.data(index, eMyMoney::Model::SplitCostCenterIdRole).toString());
        if (index.isValid())
            ui->costCenterCombo->setCurrentIndex(costCenterModel->mapFromSource(index).row());
    }
#endif
}

bool InvestTransactionEditor::Private::checkForValidTransaction(bool doUserInteraction)
{
    QStringList infos;
    bool rc = true;
    if (!postdateChanged(ui->dateEdit->date())) {
        infos << ui->dateEdit->toolTip();
        rc = false;
    }

    if (doUserInteraction) {
        /// @todo add dialog here that shows the @a infos about the problem
    }
    return rc;
}

bool InvestTransactionEditor::Private::isDatePostOpeningDate(const QDate& date, const QString& accountId)
{
    bool rc = true;

    try {
        MyMoneyAccount account = MyMoneyFile::instance()->account(accountId);
        const bool isIncomeExpense = account.isIncomeExpense();

        // we don't check for categories
        if (!isIncomeExpense) {
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


bool InvestTransactionEditor::Private::categoryChanged(const QString& accountId)
{
    bool rc = true;
    if (!accountId.isEmpty() && feeSplitModel->rowCount() <= 1) {
        try {
            MyMoneyAccount category = MyMoneyFile::instance()->account(accountId);
            const bool isIncomeExpense = category.isIncomeExpense();

            bool needValueSet = false;
            // make sure we have a split in the model
            if (feeSplitModel->rowCount() == 0) {
                // add an empty split
                MyMoneySplit s;
                feeSplitModel->addItem(s);
                needValueSet = true;
            }

            const QModelIndex index = feeSplitModel->index(0, 0);
            if (!needValueSet) {
                // update the values only if the category changes. This prevents
                // the call of the currency calculator if not needed.
                needValueSet = (index.data(eMyMoney::Model::SplitAccountIdRole).toString().compare(accountId) != 0);
            }
            feeSplitModel->setData(index, accountId, eMyMoney::Model::SplitAccountIdRole);

            rc &= postdateChanged(ui->dateEdit->date());

#if 0
            if (amountHelper->haveValue() && needValueSet) {
                feeSplitModel.setData(index, QVariant::fromValue<MyMoneyMoney>(-amountHelper->value()), eMyMoney::Model::SplitValueRole);
                feeSplitModel.setData(index, QVariant::fromValue<MyMoneyMoney>(-amountHelper->value() / getPrice()), eMyMoney::Model::SplitSharesRole);
            }
#endif

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

MyMoneyMoney InvestTransactionEditor::Private::getPrice()
{
    MyMoneyMoney result(price);
    if (!bypassPriceEditor && feeSplitModel->rowCount() > 0) {
        const QModelIndex idx = feeSplitModel->index(0, 0);
        const auto categoryId = idx.data(eMyMoney::Model::SplitAccountIdRole).toString();
        const auto category = MyMoneyFile::instance()->accountsModel()->itemById(categoryId);
        if (!category.id().isEmpty()) {
            const auto security = MyMoneyFile::instance()->security(category.currencyId());
            if (security.id() != transaction.commodity()) {
                const auto commodity = MyMoneyFile::instance()->security(transaction.commodity());
#if 0
                QPointer<KCurrencyCalculator> calc =
                    new KCurrencyCalculator(commodity,
                                            security,
                                            amountHelper->value(),
                                            amountHelper->value() / result,
                                            ui->dateEdit->date(),
                                            security.smallestAccountFraction(),
                                            q);

                if (calc->exec() == QDialog::Accepted && calc) {
                    result = calc->price();
                }
                delete calc;
#endif

            } else {
                result = MyMoneyMoney::ONE;
            }
            // keep for next round
            price = result;
        }
    }
    return result;
}


bool InvestTransactionEditor::Private::valueChanged(CreditDebitHelper* valueHelper)
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
                feeSplitModel.setData(index, QVariant::fromValue<MyMoneyMoney>(-amountHelper->value() / getPrice()), eMyMoney::Model::SplitSharesRole);
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
    const auto id = index.data(eMyMoney::Model::IdRole).toString();

    // find the first split of this transaction in the journal
    QModelIndex idx;
    int startRow;
    for (startRow = index.row()-1; startRow >= 0; --startRow) {
        idx = index.model()->index(startRow, 0);
        const auto cid = idx.data(eMyMoney::Model::IdRole).toString();
        if (cid != id)
            break;
    }
    startRow++;

    const auto rows = index.data(eMyMoney::Model::TransactionSplitCountRole).toInt();
    for(int row = startRow; row < startRow + rows; ++row) {
        idx = index.model()->index(row, 0);
        const auto accountId = idx.data(eMyMoney::Model::SplitAccountIdRole).toString();
        const auto account = MyMoneyFile::instance()->accountsModel()->itemById(accountId);
        if (account.isInvest()) {
            return idx;
        }
    }
    return {};
}

InvestTransactionEditor::InvestTransactionEditor(QWidget* parent, const QString& accountId)
    : TransactionEditorBase(parent, accountId)
    , d(new Private(this))
{
    d->ui->setupUi(this);

    d->ui->activityCombo->setModel(d->activitiesModel);

    auto const model = MyMoneyFile::instance()->accountsModel();
    d->accountsListModel->setSourceModel(model);
    d->securitiesModel->setSourceModel(d->accountsListModel);
    d->securitiesModel->setFilterRole(eMyMoney::Model::AccountParentIdRole);
    d->securitiesModel->setFilterKeyColumn(0);
    d->ui->securityCombo->setModel(d->securitiesModel);

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
    frameCollection->addWidget(d->ui->enterButton);

    // connect(d->ui->accountCombo, &KMyMoneyAccountCombo::accountSelected, this, &InvestTransactionEditor::categoryChanged);
    connect(d->ui->dateEdit, &KMyMoneyDateEdit::dateChanged, this, &InvestTransactionEditor::postdateChanged);
    connect(d->ui->activityCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &InvestTransactionEditor::setupActivity);
    connect(d->ui->securityCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &InvestTransactionEditor::securityChanged);

    connect(d->ui->feesCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, QOverload<int>::of(&InvestTransactionEditor::feesChanged));
    connect(d->ui->feesCombo, QOverload<const QString&>::of(&QComboBox::currentIndexChanged), this, QOverload<const QString&>::of(&InvestTransactionEditor::feesChanged));
    connect(d->ui->feesCombo, &KMyMoneyAccountCombo::splitDialogRequest, this, &InvestTransactionEditor::editFeeSplits, Qt::QueuedConnection);

    connect(d->ui->interestCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, QOverload<int>::of(&InvestTransactionEditor::interestChanged));
    connect(d->ui->interestCombo, QOverload<const QString&>::of(&QComboBox::currentIndexChanged), this, QOverload<const QString&>::of(&InvestTransactionEditor::interestChanged));
    connect(d->ui->interestCombo, &KMyMoneyAccountCombo::splitDialogRequest, this, &InvestTransactionEditor::editInterestSplits, Qt::QueuedConnection);

    /// @todo convert to new signal/slot syntax
    connect(d->ui->cancelButton, SIGNAL(clicked(bool)), this, SLOT(reject()));
    connect(d->ui->enterButton, SIGNAL(clicked(bool)), this, SLOT(acceptEdit()));

    // handle some events in certain conditions different from default
    d->ui->activityCombo->installEventFilter(this);
    d->ui->statusCombo->installEventFilter(this);

    d->ui->totalAmountEdit->setCalculatorButtonVisible(false);

    d->setupAccount(accountId);

    // setup tooltip

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

        // delay update until next run of event loop so that all necessary widgets are visible
        QMetaObject::invokeMethod(this, "updateTotalAmount", Qt::QueuedConnection, Q_ARG(QString, QString()));
    }






#if 0
    auto idx = MyMoneyFile::baseModel()->mapToBaseSource(index);
    if (idx.data(eMyMoney::Model::IdRole).toString().isEmpty()) {
        d->transaction = MyMoneyTransaction();
        d->transaction.setCommodity(d->m_account.currencyId());
        d->split = MyMoneySplit();
        d->split.setAccountId(d->m_account.id());
        if (lastUsedPostDate()->isValid()) {
            d->ui->dateEdit->setDate(*lastUsedPostDate());
        } else {
            d->ui->dateEdit->setDate(QDate::currentDate());
        }
        bool blocked = d->ui->accountCombo->lineEdit()->blockSignals(true);
        d->ui->accountCombo->clearEditText();
        d->ui->accountCombo->lineEdit()->blockSignals(blocked);

    } else {
        // find which item has this id and set is as the current item
        const auto selectedSplitRow = idx.row();

        // keep a copy of the transaction and split
        d->transaction = MyMoneyFile::instance()->journalModel()->itemByIndex(idx).transaction();
        d->split = MyMoneyFile::instance()->journalModel()->itemByIndex(idx).split();
        const auto list = idx.model()->match(idx.model()->index(0, 0), eMyMoney::Model::JournalTransactionIdRole,
                                             idx.data(eMyMoney::Model::JournalTransactionIdRole),
                                             -1,                         // all splits
                                             Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive | Qt::MatchRecursive));

        for (const auto& splitIdx : list) {
            if (selectedSplitRow == splitIdx.row()) {
                d->ui->dateEdit->setDate(splitIdx.data(eMyMoney::Model::TransactionPostDateRole).toDate());

                bool blocked = d->ui->accountCombo->blockSignals(true);
                switch (splitIdx.data(eMyMoney::Model::TransactionSplitCountRole).toInt()) {
                case 1:
                    d->ui->accountCombo->clearEditText();
                    d->ui->accountCombo->setSelected(QString());
                    break;
                case 2:
                    d->ui->accountCombo->setSelected(splitIdx.data(eMyMoney::Model::TransactionCounterAccountIdRole).toString());
                    break;
                default:
                    d->ui->accountCombo->setEditText(splitIdx.data(eMyMoney::Model::TransactionCounterAccountRole).toString());
                    break;
                }
                d->ui->accountCombo->blockSignals(blocked);

                d->ui->memoEdit->clear();
                d->ui->memoEdit->insertPlainText(splitIdx.data(eMyMoney::Model::SplitMemoRole).toString());
                d->ui->memoEdit->moveCursor(QTextCursor::Start);
                d->ui->memoEdit->ensureCursorVisible();

                d->ui->amountEditCredit->setText(splitIdx.data(eMyMoney::Model::JournalSplitPaymentRole).toString());
                d->ui->amountEditDebit->setText(splitIdx.data(eMyMoney::Model::JournalSplitDepositRole).toString());

                d->ui->statusCombo->setCurrentIndex(splitIdx.data(eMyMoney::Model::SplitReconcileFlagRole).toInt());
            } else {
                d->splitModel.appendSplit(MyMoneyFile::instance()->journalModel()->itemByIndex(splitIdx).split());
                if (splitIdx.data(eMyMoney::Model::TransactionSplitCountRole) == 2) {
                    const auto shares = splitIdx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>();
                    const auto value = splitIdx.data(eMyMoney::Model::SplitValueRole).value<MyMoneyMoney>();
                    if (!shares.isZero()) {
                        d->price = value / shares;
                    }
                }
            }
        }
        d->bypassPriceEditor = true;
        d->updateWidgetState();
        d->bypassPriceEditor = false;
    }
#endif
    // set focus to date edit once we return to event loop
    QMetaObject::invokeMethod(d->ui->dateEdit, "setFocus", Qt::QueuedConnection);
}

void InvestTransactionEditor::securityChanged(int index)
{
    const auto idx = d->ui->securityCombo->model()->index(index, 0);
    if (idx.isValid()) {
        d->securityChanged(idx.data(eMyMoney::Model::AccountCurrencyIdRole).toString());
    }
}


void InvestTransactionEditor::setupActivity(int index)
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
    }
}

void InvestTransactionEditor::feesChanged(int index)
{
    qDebug() << "Fees changed" << index;
}

void InvestTransactionEditor::feesChanged(const QString& txt)
{
    qDebug() << "Fees changed" << txt;
}

void InvestTransactionEditor::interestChanged(int index)
{
    qDebug() << "Interest changed" << index;
}

void InvestTransactionEditor::interestChanged(const QString& txt)
{
    qDebug() << "Interest changed" << txt;
}

void InvestTransactionEditor::categoryChanged(const QString& accountId)
{
    d->categoryChanged(accountId);
}

void InvestTransactionEditor::postdateChanged(const QDate& date)
{
    d->postdateChanged(date);
}

void InvestTransactionEditor::valueChanged()
{
#if 0
    d->valueChanged(d->amountHelper);
#endif
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

}

bool InvestTransactionEditor::eventFilter(QObject* o, QEvent* e)
{
    auto cb = qobject_cast<QComboBox*>(o);
    if (o) {
        // filter out wheel events for combo boxes if the popup view is not visible
        if ((e->type() == QEvent::Wheel) && !cb->view()->isVisible()) {
            return true;
        }
    }
    return QFrame::eventFilter(o, e);
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs on; remove-trailing-space on;remove-trailing-space-save on;
