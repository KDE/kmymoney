/*
    SPDX-FileCopyrightText: 2000, 2003 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2003, 2023 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kendingbalancedlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QAbstractButton>
#include <QBitArray>
#include <QDate>
#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KHelpClient>
#include <KMessageBox>
#include <KStandardGuiItem>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_checkingstatementinfowizardpage.h"
#include "ui_interestchargecheckingswizardpage.h"
#include "ui_kendingbalancedlg.h"

#include "kcurrencycalculator.h"
#include "kmymoneycategory.h"
#include "kmymoneysettings.h"
#include "kmymoneyutils.h"
#include "knewaccountdlg.h"
#include "mymoneyaccount.h"
#include "mymoneyenums.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneyinstitution.h"
#include "mymoneymoney.h"
#include "mymoneypayee.h"
#include "mymoneysecurity.h"
#include "mymoneysplit.h"
#include "mymoneytracer.h"
#include "mymoneytransaction.h"
#include "mymoneytransactionfilter.h"
#include "mymoneyutils.h"

class KEndingBalanceDlgPrivate
{
    Q_DISABLE_COPY(KEndingBalanceDlgPrivate)

public:
    KEndingBalanceDlgPrivate(KEndingBalanceDlg* qq, int numPages)
        : q(qq)
        , ui(new Ui::KEndingBalanceDlg)
        , m_currencyCalculator(q->MyMoneyFactory::create<KCurrencyCalculator>())
        , m_pages(numPages, true)
    {
    }

    ~KEndingBalanceDlgPrivate()
    {
        delete ui;
    }

    KEndingBalanceDlg* q;
    Ui::KEndingBalanceDlg* ui;
    KCurrencyCalculator* m_currencyCalculator;
    MyMoneyTransaction m_tInterest;
    MyMoneyTransaction m_tCharges;
    MyMoneyAccount m_account;
    QMap<QWidget*, QString> m_helpAnchor;
    QBitArray m_pages;
    QDate m_startDate;
};

KEndingBalanceDlg::KEndingBalanceDlg(const MyMoneyAccount& account, QWidget* parent)
    : QWizard(parent)
    , MyMoneyFactory(this)
    , d_ptr(new KEndingBalanceDlgPrivate(this, Page_InterestChargeCheckings + 1))
{
    Q_D(KEndingBalanceDlg);
    setModal(true);
    QString value;
    MyMoneyMoney endBalance, startBalance;

    d->ui->setupUi(this);
    d->m_account = account;

    MyMoneySecurity currency = MyMoneyFile::instance()->security(account.currencyId());
    //FIXME: port
    d->ui->m_statementInfoPageCheckings->ui->m_enterInformationLabel->setText(QString("<qt>") + i18n("Please enter the following fields with the information as you find them on your statement. Make sure to enter all values in <b>%1</b>.", currency.name()) + QString("</qt>"));

    bool skipIntroPage = false;
    KSharedConfigPtr kconfig = KSharedConfig::openConfig();
    if (kconfig) {
        skipIntroPage = kconfig->group(QLatin1String("Notification Messages")).readEntry(QLatin1String("SkipReconciliationIntro"), false);
    }
    setField(QLatin1String("skipIntroPage"), skipIntroPage);

    // If the previous reconciliation was postponed,
    // we show a different first page
    value = account.value("lastReconciledBalance");
    if (value.isEmpty()) {
        // if the last statement has been entered long enough ago (more than one month),
        // then take the last statement date and add one month and use that as statement
        // date.
        QDate lastStatementDate = account.lastReconciliationDate();
        if (lastStatementDate.isValid() && (lastStatementDate.addMonths(1) < QDate::currentDate())) {
            setField("statementDate", lastStatementDate.addMonths(1));
        }

        slotUpdateBalances();

        d->m_pages.clearBit(Page_PreviousPostpone);
        if (skipIntroPage) {
            d->m_pages.clearBit(Page_CheckingStart);
            setStartId(Page_CheckingStatementInfo);
        }
    } else {
        d->m_pages.clearBit(Page_CheckingStart);
        d->m_pages.clearBit(Page_InterestChargeCheckings);
        //removePage(d->ui->m_interestChargeCheckings);
        // make sure, we show the correct start page
        setStartId(Page_PreviousPostpone);

        MyMoneyMoney factor(1, 1);
        if (d->m_account.accountGroup() == eMyMoney::Account::Type::Liability)
            factor = -factor;

        startBalance = MyMoneyMoney(value) * factor;
        value = account.value("statementBalance");
        endBalance = MyMoneyMoney(value) * factor;

        //FIXME: port
        d->ui->m_statementInfoPageCheckings->ui->m_previousBalance->setValue(startBalance);
        d->ui->m_statementInfoPageCheckings->ui->m_endingBalance->setValue(endBalance);
    }

    // We don't need to add the default into the list (see ::help() why)
    // m_helpAnchor[m_startPageCheckings] = QString(QString());
    d->m_helpAnchor[d->ui->m_interestChargeCheckings] = QString("details.reconcile.wizard.interest");
    d->m_helpAnchor[d->ui->m_statementInfoPageCheckings] = QString("details.reconcile.wizard.statement");

    value = account.value("statementDate");
    if (!value.isEmpty()) {
        setField("statementDate", QDate::fromString(value, Qt::ISODate));
    }
    //FIXME: port
    d->ui->m_statementInfoPageCheckings->ui->m_lastStatementDate->setText(QString());
    if (account.lastReconciliationDate().isValid()) {
        d->ui->m_statementInfoPageCheckings->ui->m_lastStatementDate->setText(
            i18n("Last reconciled statement: %1", MyMoneyUtils::formatDate(account.lastReconciliationDate())));
    }

    // connect the signals with the slots
    connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, this, &KEndingBalanceDlg::slotReloadEditWidgets);
    connect(d->ui->m_statementInfoPageCheckings->ui->m_statementDate, &KMyMoneyDateEdit::dateChanged, this, &KEndingBalanceDlg::slotUpdateBalances);
    connect(d->ui->m_interestChargeCheckings->ui->m_interestCategoryEdit, &KMyMoneyCombo::createItem, this, &KEndingBalanceDlg::slotCreateInterestCategory);
    connect(d->ui->m_interestChargeCheckings->ui->m_chargesCategoryEdit, &KMyMoneyCombo::createItem, this, &KEndingBalanceDlg::slotCreateChargesCategory);
    connect(d->ui->m_interestChargeCheckings->ui->m_payeeEdit, &KMyMoneyMVCCombo::createItem, this, &KEndingBalanceDlg::slotNewPayee);

    KMyMoneyMVCCombo::setSubstringSearchForChildren(d->ui->m_interestChargeCheckings, !KMyMoneySettings::stringMatchFromStart());

    slotReloadEditWidgets();

    // preset payee if possible
    try {
        // if we find a payee with the same name as the institution,
        // than this is what we use as payee.
        if (!d->m_account.institutionId().isEmpty()) {
            MyMoneyInstitution inst = MyMoneyFile::instance()->institution(d->m_account.institutionId());
            MyMoneyPayee payee = MyMoneyFile::instance()->payeeByName(inst.name());
            setField("payeeEdit", payee.id());
        }
    } catch (const MyMoneyException &) {
    }

    KMyMoneyUtils::updateWizardButtons(this);

    // setup different text and icon on finish button
    setButtonText(QWizard::FinishButton, KStandardGuiItem::cont().text());
    button(QWizard::FinishButton)->setIcon(KStandardGuiItem::cont().icon());
}

KEndingBalanceDlg::~KEndingBalanceDlg()
{
    Q_D(KEndingBalanceDlg);
    delete d;
}

void KEndingBalanceDlg::slotUpdateBalances()
{
    Q_D(KEndingBalanceDlg);
    MYMONEYTRACER(tracer);

    // determine the beginning balance and ending balance based on the following
    // formulas:
    //
    // end balance   = current balance - sum(all non cleared transactions)
    //                                 - sum(all cleared transactions posted
    //                                        after statement date)
    // start balance = end balance - sum(all cleared transactions
    //                                        up to statement date)
    MyMoneyTransactionFilter filter(d->m_account.id());
    filter.addState((int)eMyMoney::TransactionFilter::State::NotReconciled);
    filter.setReportAllSplits(true);

    QList<QPair<MyMoneyTransaction, MyMoneySplit> > transactionList;
    QList<QPair<MyMoneyTransaction, MyMoneySplit> >::const_iterator it;

    // retrieve the list from the engine
    MyMoneyFile::instance()->transactionList(transactionList, filter);

    //first retrieve the oldest not reconciled transaction
    QDate oldestTransactionDate;
    it = transactionList.cbegin();
    if (it != transactionList.cend()) {
        oldestTransactionDate = (*it).first.postDate();
        d->ui->m_statementInfoPageCheckings->ui->m_oldestTransactionDate->setText(
            i18n("Oldest unmarked transaction: %1", MyMoneyUtils::formatDate(oldestTransactionDate)));
    }

    filter.clear();
    filter.addAccount(d->m_account.id());

    // retrieve the list from the engine to calculate the starting and ending balance
    MyMoneyFile::instance()->transactionList(transactionList, filter);

    // set the balance to the value of the account at the end of the ledger
    // This includes all transactions even those past the statement date.
    MyMoneyMoney balance = MyMoneyFile::instance()->balance(d->m_account.id());
    MyMoneyMoney factor(1, 1);
    if (d->m_account.accountGroup() == eMyMoney::Account::Type::Liability)
        factor = -factor;

    MyMoneyMoney endBalance, startBalance;
    balance = balance * factor;
    endBalance = startBalance = balance;

    tracer.printf("total balance = %s", qPrintable(endBalance.formatMoney(QString(), 2)));

    d->m_startDate = d->m_account.lastReconciliationDate();

    // now adjust the balances by reading all transactions referencing the account
    // from beginning to the end of the ledger
    for (it = transactionList.cbegin(); it != transactionList.cend(); ++it) {
        const MyMoneySplit& split = (*it).second;
        balance -= split.shares() * factor;
        if ((*it).first.postDate() > field("statementDate").toDate()) {
            // in case the transaction's post date is younger than the statement date
            // we need to subtract the value from the balances.

            tracer.printf("Reducing balances by %s because postdate of %s/%s(%s) is past statement date", qPrintable((split.shares() * factor).formatMoney(QString(), 2)), qPrintable((*it).first.id()), qPrintable(split.id()), qPrintable((*it).first.postDate().toString(Qt::ISODate)));
            endBalance -= split.shares() * factor;
            startBalance -= split.shares() * factor;

        } else {
            // in case the transaction's post date is older or equal to
            // the statement date we need to check what to do
            switch (split.reconcileFlag()) {
            case eMyMoney::Split::State::NotReconciled:
                // in case it is not marked at all we need to check if
                // it is necessary to adjust the start date of the
                // display.
                if ((*it).first.postDate() < d->m_startDate) {
                    d->m_startDate = (*it).first.postDate();
                }

                // subtract it from the balances
                tracer.printf("Reducing balances by %s because %s/%s(%s) is not reconciled", qPrintable((split.shares() * factor).formatMoney(QString(), 2)), qPrintable((*it).first.id()), qPrintable(split.id()), qPrintable((*it).first.postDate().toString(Qt::ISODate)));
                endBalance -= split.shares() * factor;
                startBalance -= split.shares() * factor;
                break;

            case eMyMoney::Split::State::Cleared:
                // in case it is marked as cleared we need to check if
                // it is necessary to adjust the start date of the
                // display. It could be, that this transaction is
                // older than the statement date.
                if ((*it).first.postDate() < d->m_startDate) {
                    d->m_startDate = (*it).first.postDate();
                }
                Q_FALLTHROUGH();

            case eMyMoney::Split::State::Reconciled:
            case eMyMoney::Split::State::Frozen:
                // in case it is marked as cleared, reconciled or frozen
                // we need to check if the transaction is found after
                // the current start date. If so, we need to adjust the
                // startBalance but don't touch the endBalance
                if ((*it).first.postDate() > d->m_startDate) {
                    tracer.printf("Reducing start balance by %s because %s/%s(%s) is cleared/reconciled",
                                  qPrintable((split.shares() * factor).formatMoney(QString(), 2)),
                                  qPrintable((*it).first.id()),
                                  qPrintable(split.id()),
                                  qPrintable((*it).first.postDate().toString(Qt::ISODate)));
                    startBalance -= split.shares() * factor;
                }
                break;

            default:
                break;
            }
        }
    }
    //FIXME: port
    d->ui->m_statementInfoPageCheckings->ui->m_previousBalance->setValue(startBalance);
    d->ui->m_statementInfoPageCheckings->ui->m_endingBalance->setValue(endBalance);
    tracer.printf("total balance = %s", qPrintable(endBalance.formatMoney(QString(), 2)));
    tracer.printf("start balance = %s", qPrintable(startBalance.formatMoney(QString(), 2)));

    setField("interestDateEdit", field("statementDate").toDate());
    setField("chargesDateEdit", field("statementDate").toDate());
}

void KEndingBalanceDlg::accept()
{
    Q_D(KEndingBalanceDlg);
    if ((!field("interestEditValid").toBool() || createTransaction(d->m_tInterest, -1, field("interestEdit").value<MyMoneyMoney>(), field("interestCategoryEdit").toString(), field("interestDateEdit").toDate()))
            && (!field("chargesEditValid").toBool() || createTransaction(d->m_tCharges, 1, field("chargesEdit").value<MyMoneyMoney>(), field("chargesCategoryEdit").toString(), field("chargesDateEdit").toDate())))
        QWizard::accept();
}

void KEndingBalanceDlg::slotCreateInterestCategory(const QString& txt, QString& id)
{
    createCategory(txt, id, MyMoneyFile::instance()->income());
}

void KEndingBalanceDlg::slotCreateChargesCategory(const QString& txt, QString& id)
{
    createCategory(txt, id, MyMoneyFile::instance()->expense());
}

void KEndingBalanceDlg::createCategory(const QString& txt, QString& id, const MyMoneyAccount& parent)
{
    MyMoneyAccount acc;
    acc.setName(txt);

    KNewAccountDlg::createCategory(acc, parent);

    id = acc.id();
}

void KEndingBalanceDlg::slotNewPayee(const QString& newnameBase, QString& id)
{
    bool ok;
    std::tie(ok, id) = KMyMoneyUtils::newPayee(newnameBase);
}

MyMoneyMoney KEndingBalanceDlg::endingBalance() const
{
    Q_D(const KEndingBalanceDlg);
    return adjustedReturnValue(d->ui->m_statementInfoPageCheckings->ui->m_endingBalance->value());
}

MyMoneyMoney KEndingBalanceDlg::previousBalance() const
{
    Q_D(const KEndingBalanceDlg);
    return adjustedReturnValue(d->ui->m_statementInfoPageCheckings->ui->m_previousBalance->value());
}

QDate KEndingBalanceDlg::statementDate() const
{
    return field("statementDate").toDate();
}

MyMoneyMoney KEndingBalanceDlg::adjustedReturnValue(const MyMoneyMoney& v) const
{
    Q_D(const KEndingBalanceDlg);
    return d->m_account.accountGroup() == eMyMoney::Account::Type::Liability ? -v : v;
}

void KEndingBalanceDlg::slotReloadEditWidgets()
{
    Q_D(KEndingBalanceDlg);
    QString payeeId, interestId, chargesId;

    // keep current selected items
    payeeId = field("payeeEdit").toString();
    interestId = field("interestCategoryEdit").toString();
    chargesId = field("chargesCategoryEdit").toString();

    // load the payee and category widgets with data from the engine
    //FIXME: port
    d->ui->m_interestChargeCheckings->ui->m_payeeEdit->loadPayees(MyMoneyFile::instance()->payeeList());

    // a user request to show all categories in both selectors due to a valid use case.
    AccountSet aSet;
    aSet.addAccountGroup(eMyMoney::Account::Type::Expense);
    aSet.addAccountGroup(eMyMoney::Account::Type::Income);
    //FIXME: port
    aSet.load(d->ui->m_interestChargeCheckings->ui->m_interestCategoryEdit->selector());
    aSet.load(d->ui->m_interestChargeCheckings->ui->m_chargesCategoryEdit->selector());

    // reselect currently selected items
    if (!payeeId.isEmpty())
        setField("payeeEdit", payeeId);
    if (!interestId.isEmpty())
        setField("interestCategoryEdit", interestId);
    if (!chargesId.isEmpty())
        setField("chargesCategoryEdit", chargesId);
}

MyMoneyTransaction KEndingBalanceDlg::interestTransaction()
{
    Q_D(KEndingBalanceDlg);
    return d->m_tInterest;
}

MyMoneyTransaction KEndingBalanceDlg::chargeTransaction()
{
    Q_D(KEndingBalanceDlg);
    return d->m_tCharges;
}

bool KEndingBalanceDlg::createTransaction(MyMoneyTransaction &t, const int sign, const MyMoneyMoney& amount, const QString& category, const QDate& date)
{
    Q_D(KEndingBalanceDlg);
    t = MyMoneyTransaction();

    if (category.isEmpty() || !date.isValid())
        return true;

    MyMoneySplit s1, s2;
    MyMoneyMoney val = amount * MyMoneyMoney(sign, 1);
    try {
        t.setPostDate(date);
        t.setCommodity(d->m_account.currencyId());

        s1.setPayeeId(field("payeeEdit").toString());
        s1.setReconcileFlag(eMyMoney::Split::State::Cleared);
        s1.setAccountId(d->m_account.id());
        s1.setValue(-val);
        s1.setShares(-val);

        s2 = s1;
        s2.setAccountId(category);
        s2.setValue(val);

        t.addSplit(s1);
        t.addSplit(s2);

        QMap<QString, MyMoneyMoney> priceInfo; // just empty
        MyMoneyMoney shares;
        if (!d->m_currencyCalculator->setupSplitPrice(shares, t, s2, priceInfo, this)) {
            t = MyMoneyTransaction();
            return false;
        }

        s2.setShares(shares);
        t.modifySplit(s2);

    } catch (const MyMoneyException &e) {
        qDebug("%s", e.what());
        t = MyMoneyTransaction();
        return false;
    }

    return true;
}

void KEndingBalanceDlg::help()
{
    Q_D(KEndingBalanceDlg);
    QString anchor = d->m_helpAnchor[currentPage()];
    if (anchor.isEmpty())
        anchor = QString("details.reconcile.whatis");

    KHelpClient::invokeHelp(anchor);
}

int KEndingBalanceDlg::nextId() const
{
    Q_D(const KEndingBalanceDlg);
    // Starting from the current page, look for the first enabled page
    // and return that value
    // If the end of the list is encountered first, then return -1.
    for (int i = currentId() + 1; i < d->m_pages.size() && i < pageIds().size(); ++i) {
        if (d->m_pages.testBit(i))
            return pageIds()[i];
    }
    return -1;
}

QDate KEndingBalanceDlg::startDate() const
{
    Q_D(const KEndingBalanceDlg);
    return d->m_startDate;
}
