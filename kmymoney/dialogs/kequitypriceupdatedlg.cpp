/*
    SPDX-FileCopyrightText: 2004-2017 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2004 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2006 Ace Jones <acejones@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kequitypriceupdatedlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>
#include <QPointer>
#include <QPushButton>
#include <QTimer>
#include <QToolButton>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>
#include <KTextEdit>
#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>
#include <KStandardGuiItem>
#include <KLocalizedString>
#include <KColorScheme>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kequitypriceupdatedlg.h"

#include "dialogenums.h"
#include "icons.h"
#include "kequitypriceupdateconfdlg.h"
#include "kmymoneyutils.h"
#include "mymoneyaccount.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneyprice.h"
#include "mymoneysecurity.h"
#include "webpricequote.h"

#define WEBID_COL       0
#define NAME_COL        1
#define PRICE_COL       2
#define DATE_COL        3
#define KMMID_COL       4
#define SOURCE_COL      5

class KEquityPriceUpdateDlgPrivate
{
    Q_DISABLE_COPY(KEquityPriceUpdateDlgPrivate)
    Q_DECLARE_PUBLIC(KEquityPriceUpdateDlg)

public:
    explicit KEquityPriceUpdateDlgPrivate(KEquityPriceUpdateDlg* qq)
        : q_ptr(qq)
        , ui(new Ui::KEquityPriceUpdateDlg)
        , m_fUpdateAll(false)
        , m_updatingPricePolicy(eDialogs::UpdatePrice::All)
        , m_splitRegex("([0-9a-z\\.]+)[^a-z0-9]+([0-9a-z\\.]+)", QRegularExpression::CaseInsensitiveOption)
    {
    }

    ~KEquityPriceUpdateDlgPrivate()
    {
        delete ui;
    }

    void init(const QString& securityId)
    {
        Q_Q(KEquityPriceUpdateDlg);
        ui->setupUi(q);
        m_fUpdateAll = false;
        QStringList headerList;
        headerList << i18n("ID") << i18nc("Equity name", "Name")
                   << i18n("Price") << i18n("Date");

        ui->lvEquityList->header()->setSortIndicator(0, Qt::AscendingOrder);
        ui->lvEquityList->setColumnWidth(NAME_COL, 125);

        // This is a "get it up and running" hack.  Will replace this in the future.
        headerList << i18nc("Internal identifier", "Internal ID")
                   << i18nc("Online quote source", "Source");
        ui->lvEquityList->setColumnWidth(KMMID_COL, 0);

        ui->lvEquityList->setHeaderLabels(headerList);

        ui->lvEquityList->setSelectionMode(QAbstractItemView::MultiSelection);
        ui->lvEquityList->setAllColumnsShowFocus(true);

        ui->btnUpdateAll->setEnabled(false);

        ui->closeStatusButton->setIcon(Icons::get(Icons::Icon::DialogClose));
        q->connect(ui->closeStatusButton, &QToolButton::clicked, ui->statusContainer, &QWidget::hide);

        // hide the status widgets until there is activity
        ui->statusContainer->hide();

        auto file = MyMoneyFile::instance();

        //
        // Add each price pair that we know about
        //

        // send in securityId == "XXX YYY" to get a single-shot update for XXX to YYY.
        // for consistency reasons, this accepts the same delimiters as WebPriceQuote::launch()
        QRegularExpressionMatch match(m_splitRegex.match(securityId));
        MyMoneySecurityPair currencyIds;
        if (match.hasMatch()) {
            currencyIds = MyMoneySecurityPair(match.captured(1), match.captured(2));
        }

        MyMoneyPriceList prices = file->priceList();
        for (MyMoneyPriceList::ConstIterator it_price = prices.constBegin(); it_price != prices.constEnd(); ++it_price) {
            const MyMoneySecurityPair& pair = it_price.key();
            if (file->security(pair.first).isCurrency() && (securityId.isEmpty() || (pair == currencyIds))) {
                if (pair.first.trimmed().isEmpty() || pair.second.trimmed().isEmpty()) {
                    qDebug() << "A currency pair" << pair << "has one of its elements present, while the other one is empty. Omitting.";
                    continue;
                }
                if (!file->security(pair.second).isCurrency()) {
                    qDebug() << "A currency pair" << pair << "is invalid (from currency to equity). Omitting.";
                    continue;
                }
                const MyMoneyPriceEntries& entries = (*it_price);
                if (entries.count() > 0 && entries.begin().key() <= QDate::currentDate()) {
                    addPricePair(pair, false);
                    ui->btnUpdateAll->setEnabled(true);
                }
            }
        }

        //
        // Add each investment
        //

        QList<MyMoneySecurity> securities = file->securityList();
        for (QList<MyMoneySecurity>::const_iterator it = securities.constBegin(); it != securities.constEnd(); ++it) {
            if (!(*it).isCurrency() //
                    && (securityId.isEmpty() || ((*it).id() == securityId)) //
                    && !(*it).value("kmm-online-source").isEmpty()
               ) {
                addInvestment(*it);
                ui->btnUpdateAll->setEnabled(true);
            }
        }

        // if list is empty and a price pair has been requested, add it
        if (ui->lvEquityList->invisibleRootItem()->childCount() == 0 && !securityId.isEmpty()) {
            addPricePair(currencyIds, true);
        }

        q->connect(ui->btnUpdateSelected, &QAbstractButton::clicked, q, &KEquityPriceUpdateDlg::slotUpdateSelectedClicked);
        q->connect(ui->btnUpdateAll, &QAbstractButton::clicked, q, &KEquityPriceUpdateDlg::slotUpdateAllClicked);

        q->connect(ui->m_fromDate, &KMyMoneyDateInput::dateChanged, q, &KEquityPriceUpdateDlg::slotDateChanged);
        q->connect(ui->m_toDate, &KMyMoneyDateInput::dateChanged, q, &KEquityPriceUpdateDlg::slotDateChanged);

        q->connect(&m_webQuote, &WebPriceQuote::csvquote,
                   q, &KEquityPriceUpdateDlg::slotReceivedCSVQuote);
        q->connect(&m_webQuote, &WebPriceQuote::quote,
                   q, &KEquityPriceUpdateDlg::slotReceivedQuote);
        q->connect(&m_webQuote, &WebPriceQuote::failed,
                   q, &KEquityPriceUpdateDlg::slotQuoteFailed);
        q->connect(&m_webQuote, &WebPriceQuote::status,
                   q, &KEquityPriceUpdateDlg::logStatusMessage);
        q->connect(&m_webQuote, &WebPriceQuote::error,
                   q, &KEquityPriceUpdateDlg::logErrorMessage);

        q->connect(ui->lvEquityList, &QTreeWidget::itemSelectionChanged, q, &KEquityPriceUpdateDlg::slotUpdateSelection);

        q->connect(ui->btnConfigure, &QAbstractButton::clicked, q, &KEquityPriceUpdateDlg::slotConfigureClicked);

        if (!securityId.isEmpty()) {
            ui->btnUpdateSelected->hide();
            ui->btnUpdateAll->hide();
            // delete layout1;

            QTimer::singleShot(100, q, SLOT(slotUpdateAllClicked()));
        }

        // Hide OK button until we have received the first update
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        if (ui->lvEquityList->invisibleRootItem()->childCount() == 0) {
            ui->btnUpdateAll->setEnabled(false);
        }
        q->slotUpdateSelection();

        // previous versions of this dialog allowed to store a "Don't ask again" switch.
        // Since we don't support it anymore, we just get rid of it
        KSharedConfigPtr config = KSharedConfig::openConfig();
        KConfigGroup grp = config->group("Notification Messages");
        grp.deleteEntry("KEquityPriceUpdateDlg::slotQuoteFailed::Price Update Failed");
        grp.sync();
        grp = config->group("Equity Price Update");
        int policyValue = grp.readEntry("PriceUpdatingPolicy", (int)eDialogs::UpdatePrice::Missing);
        if (policyValue > (int)eDialogs::UpdatePrice::Ask || policyValue < (int)eDialogs::UpdatePrice::All)
            m_updatingPricePolicy = eDialogs::UpdatePrice::Missing;
        else
            m_updatingPricePolicy = static_cast<eDialogs::UpdatePrice>(policyValue);
    }

    void addPricePair(const MyMoneySecurityPair& pair, bool dontCheckExistance)
    {
        Q_ASSERT(!pair.first.trimmed().isEmpty());
        Q_ASSERT(!pair.second.trimmed().isEmpty());

        auto file = MyMoneyFile::instance();

        const auto symbol = QString::fromLatin1("%1 > %2").arg(pair.first, pair.second);
        const auto id = QString::fromLatin1("%1 %2").arg(pair.first, pair.second);
        // Check that the pair does not already exist
        if (ui->lvEquityList->findItems(id, Qt::MatchExactly, KMMID_COL).empty()) {
            const MyMoneyPrice &pr = file->price(pair.first, pair.second);
            if (pr.source() != QLatin1String("KMyMoney")) {
                bool keep = true;
                if ((pair.first == file->baseCurrency().id())
                        || (pair.second == file->baseCurrency().id())) {
                    const QString& foreignCurrency = file->foreignCurrency(pair.first, pair.second);
                    // check that the foreign currency is still in use
                    QList<MyMoneyAccount>::const_iterator it_a;
                    QList<MyMoneyAccount> list;
                    file->accountList(list);
                    for (it_a = list.constBegin(); !dontCheckExistance && it_a != list.constEnd(); ++it_a) {
                        // if it's an account denominated in the foreign currency
                        // keep it
                        if (((*it_a).currencyId() == foreignCurrency)
                                && !(*it_a).isClosed())
                            break;
                        // if it's an investment traded in the foreign currency
                        // keep it
                        if ((*it_a).isInvest() && !(*it_a).isClosed()) {
                            MyMoneySecurity sec = file->security((*it_a).currencyId());
                            if (sec.tradingCurrency() == foreignCurrency)
                                break;
                        }
                    }
                    // if it is in use, it_a is not equal to list.end()
                    if (it_a == list.constEnd() && !dontCheckExistance)
                        keep = false;
                }

                if (keep) {
                    auto item = new QTreeWidgetItem();
                    item->setText(WEBID_COL, symbol);
                    item->setText(NAME_COL, i18n("%1 units in %2", pair.first, pair.second));
                    if (pr.isValid()) {
                        MyMoneySecurity fromCurrency = file->currency(pair.second);
                        MyMoneySecurity toCurrency = file->currency(pair.first);
                        item->setText(PRICE_COL, pr.rate(pair.second).formatMoney(fromCurrency.tradingSymbol(), toCurrency.pricePrecision()));
                        item->setText(DATE_COL, pr.date().toString(Qt::ISODate));
                    }
                    item->setText(KMMID_COL, id);
                    item->setText(SOURCE_COL, "KMyMoney Currency");  // This string value should not be localized
                    ui->lvEquityList->invisibleRootItem()->addChild(item);
                }
            }
        }
    }

    void addInvestment(const MyMoneySecurity& inv)
    {
        const auto id = inv.id();
        // Check that the pair does not already exist
        if (ui->lvEquityList->findItems(id, Qt::MatchExactly, KMMID_COL).empty()) {
            auto file = MyMoneyFile::instance();
            // check that the security is still in use
            QList<MyMoneyAccount>::const_iterator it_a;
            QList<MyMoneyAccount> list;
            file->accountList(list);
            for (it_a = list.constBegin(); it_a != list.constEnd(); ++it_a) {
                if ((*it_a).isInvest()
                        && ((*it_a).currencyId() == inv.id())
                        && !(*it_a).isClosed())
                    break;
            }
            // if it is in use, it_a is not equal to list.end()
            if (it_a != list.constEnd()) {
                QString webID;
                WebPriceQuoteSource onlineSource(inv.value("kmm-online-source"));
                if (onlineSource.m_webIDBy == WebPriceQuoteSource::identifyBy::IdentificationNumber)
                    webID = inv.value("kmm-security-id");   // insert ISIN number...
                else if (onlineSource.m_webIDBy == WebPriceQuoteSource::identifyBy::Name)
                    webID = inv.name();                     // ...or name...
                else
                    webID = inv.tradingSymbol();            // ...or symbol

                QTreeWidgetItem* item = new QTreeWidgetItem();
                item->setForeground(WEBID_COL, KColorScheme(QPalette::Normal).foreground(KColorScheme::NormalText));
                if (webID.isEmpty()) {
                    webID = i18n("[No identifier]");
                    item->setForeground(WEBID_COL, KColorScheme(QPalette::Normal).foreground(KColorScheme::NegativeText));
                }
                item->setText(WEBID_COL, webID);
                item->setText(NAME_COL, inv.name());
                MyMoneySecurity currency = file->currency(inv.tradingCurrency());
                const MyMoneyPrice &pr = file->price(id.toUtf8(), inv.tradingCurrency());
                if (pr.isValid()) {
                    item->setText(PRICE_COL, pr.rate(currency.id()).formatMoney(currency.tradingSymbol(), inv.pricePrecision()));
                    item->setText(DATE_COL, pr.date().toString(Qt::ISODate));
                }
                item->setText(KMMID_COL, id);
                if (inv.value("kmm-online-quote-system") == "Finance::Quote")
                    item->setText(SOURCE_COL, QString("Finance::Quote %1").arg(inv.value("kmm-online-source")));
                else
                    item->setText(SOURCE_COL, inv.value("kmm-online-source"));

                ui->lvEquityList->invisibleRootItem()->addChild(item);

                // If this investment is denominated in a foreign currency, ensure that
                // the appropriate price pair is also on the list

                if (currency.id() != file->baseCurrency().id()) {
                    addPricePair(MyMoneySecurityPair(currency.id(), file->baseCurrency().id()), false);
                }
            }
        }
    }

    KEquityPriceUpdateDlg* q_ptr;
    Ui::KEquityPriceUpdateDlg* ui;
    bool m_fUpdateAll;
    eDialogs::UpdatePrice m_updatingPricePolicy;
    WebPriceQuote m_webQuote;
    QRegularExpression m_splitRegex;
};

KEquityPriceUpdateDlg::KEquityPriceUpdateDlg(QWidget *parent, const QString& securityId) :
    QDialog(parent),
    d_ptr(new KEquityPriceUpdateDlgPrivate(this))
{
    Q_D(KEquityPriceUpdateDlg);
    d->init(securityId);
}

KEquityPriceUpdateDlg::~KEquityPriceUpdateDlg()
{
    Q_D(KEquityPriceUpdateDlg);
    auto config = KSharedConfig::openConfig();
    auto grp = config->group("Equity Price Update");
    grp.writeEntry("PriceUpdatingPolicy", static_cast<int>(d->m_updatingPricePolicy));
    grp.sync();
    delete d;
}

void KEquityPriceUpdateDlg::logErrorMessage(const QString& message)
{
    logStatusMessage(QString("<font color=\"red\"><b>") + message + QString("</b></font>"));
}

void KEquityPriceUpdateDlg::logStatusMessage(const QString& message)
{
    Q_D(KEquityPriceUpdateDlg);
    d->ui->lbStatus->append(message);
}

MyMoneyPrice KEquityPriceUpdateDlg::price(const QString& id) const
{
    Q_D(const KEquityPriceUpdateDlg);
    MyMoneyPrice price;
    QTreeWidgetItem* item = nullptr;
    QList<QTreeWidgetItem*> foundItems = d->ui->lvEquityList->findItems(id, Qt::MatchExactly, KMMID_COL);

    if (! foundItems.empty())
        item = foundItems.at(0);

    if (item) {
        MyMoneyMoney rate(item->text(PRICE_COL));
        if (!rate.isZero()) {
            QString kmm_id = item->text(KMMID_COL).toUtf8();

            // if the ID has a space, then this is TWO ID's, so it's a currency quote
            if (kmm_id.contains(" ")) {
                QStringList ids = kmm_id.split(' ', QString::SkipEmptyParts);
                QString fromid = ids[0].toUtf8();
                QString toid = ids[1].toUtf8();
                price = MyMoneyPrice(fromid, toid, QDate().fromString(item->text(DATE_COL), Qt::ISODate), rate, item->text(SOURCE_COL));
            } else
                // otherwise, it's a security quote
            {
                MyMoneySecurity security = MyMoneyFile::instance()->security(kmm_id);
                price = MyMoneyPrice(kmm_id, security.tradingCurrency(), QDate().fromString(item->text(DATE_COL), Qt::ISODate), rate, item->text(SOURCE_COL));
            }
        }
    }
    return price;
}

void KEquityPriceUpdateDlg::storePrices()
{
    Q_D(KEquityPriceUpdateDlg);
    // update the new prices into the equities

    auto file = MyMoneyFile::instance();
    QString name;

    MyMoneyFileTransaction ft;
    try {
        for (auto i = 0; i < d->ui->lvEquityList->invisibleRootItem()->childCount(); ++i) {
            QTreeWidgetItem* item = d->ui->lvEquityList->invisibleRootItem()->child(i);
            // turn on signals before we modify the last entry in the list
            file->blockSignals(i < d->ui->lvEquityList->invisibleRootItem()->childCount() - 1);

            MyMoneyMoney rate(item->text(PRICE_COL));
            if (!rate.isZero()) {
                QString id = item->text(KMMID_COL);
                QString fromid;
                QString toid;

                // if the ID has a space, then this is TWO ID's, so it's a currency quote
                if (id.contains(QLatin1Char(' '))) {
                    QStringList ids = id.split(QLatin1Char(' '), QString::SkipEmptyParts);
                    fromid = ids.at(0);
                    toid = ids.at(1);
                    name = QString::fromLatin1("%1 --> %2").arg(fromid, toid);
                } else { // otherwise, it's a security quote
                    MyMoneySecurity security = file->security(id);
                    name = security.name();
                    fromid = id;
                    toid = security.tradingCurrency();
                }
                // TODO (Ace) Better handling of the case where there is already a price
                // for this date.  Currently, it just overrides the old value.  Really it
                // should check to see if the price is the same and prompt the user.
                file->addPrice(MyMoneyPrice(fromid, toid, QDate::fromString(item->text(DATE_COL), Qt::ISODate), rate, item->text(SOURCE_COL)));
            }
        }
        ft.commit();

    } catch (const MyMoneyException &) {
        qDebug("Unable to add price information for %s", qPrintable(name));
    }
}

void KEquityPriceUpdateDlg::slotConfigureClicked()
{
    Q_D(KEquityPriceUpdateDlg);
    QPointer<EquityPriceUpdateConfDlg> dlg = new EquityPriceUpdateConfDlg(d->m_updatingPricePolicy);
    if (dlg->exec() == QDialog::Accepted)
        d->m_updatingPricePolicy = dlg->policy();
    delete dlg;
}

void KEquityPriceUpdateDlg::slotUpdateSelection()
{
    Q_D(KEquityPriceUpdateDlg);
    // Only enable the update button if there is a selection
    d->ui->btnUpdateSelected->setEnabled(false);

    if (! d->ui->lvEquityList->selectedItems().empty())
        d->ui->btnUpdateSelected->setEnabled(true);
}

void KEquityPriceUpdateDlg::slotUpdateSelectedClicked()
{
    Q_D(KEquityPriceUpdateDlg);
    // disable sorting while the update is running to maintain the current order of items on which
    // the update process depends and which could be changed with sorting enabled due to the updated values
    d->ui->lvEquityList->setSortingEnabled(false);

    // show the status widgets
    d->ui->statusContainer->show();
    d->ui->prgOnlineProgress->show();

    auto item = d->ui->lvEquityList->invisibleRootItem()->child(0);
    auto skipCnt = 1;
    while (item && !item->isSelected()) {
        item = d->ui->lvEquityList->invisibleRootItem()->child(skipCnt);
        ++skipCnt;
    }
    d->m_webQuote.setDate(d->ui->m_fromDate->date(), d->ui->m_toDate->date());
    if (item) {
        d->ui->prgOnlineProgress->setMaximum(1 + d->ui->lvEquityList->invisibleRootItem()->childCount());
        d->ui->prgOnlineProgress->setValue(skipCnt);
        d->m_webQuote.launch(item->text(WEBID_COL), item->text(KMMID_COL), item->text(SOURCE_COL));

    } else {

        logErrorMessage("No security selected.");
    }
}

void KEquityPriceUpdateDlg::slotUpdateAllClicked()
{
    Q_D(KEquityPriceUpdateDlg);
    // disable sorting while the update is running to maintain the current order of items on which
    // the update process depends and which could be changed with sorting enabled due to the updated values
    d->ui->lvEquityList->setSortingEnabled(false);

    // show the status widgets
    d->ui->statusContainer->show();
    d->ui->prgOnlineProgress->show();

    QTreeWidgetItem* item = d->ui->lvEquityList->invisibleRootItem()->child(0);
    if (item) {
        d->ui->prgOnlineProgress->setMaximum(1 + d->ui->lvEquityList->invisibleRootItem()->childCount());
        d->ui->prgOnlineProgress->setValue(1);
        d->m_fUpdateAll = true;
        d->m_webQuote.launch(item->text(WEBID_COL), item->text(KMMID_COL), item->text(SOURCE_COL));

    } else {
        logErrorMessage("Security list is empty.");
    }
}

void KEquityPriceUpdateDlg::slotDateChanged()
{
    Q_D(KEquityPriceUpdateDlg);
    QSignalBlocker blockFrom(d->ui->m_fromDate);
    QSignalBlocker blockTo(d->ui->m_toDate);
    if (d->ui->m_toDate->date() > QDate::currentDate())
        d->ui->m_toDate->setDate(QDate::currentDate());
    if (d->ui->m_fromDate->date() > d->ui->m_toDate->date())
        d->ui->m_fromDate->setDate(d->ui->m_toDate->date());
}

void KEquityPriceUpdateDlg::slotQuoteFailed(const QString& _kmmID, const QString& _webID)
{
    Q_D(KEquityPriceUpdateDlg);
    auto foundItems = d->ui->lvEquityList->findItems(_kmmID, Qt::MatchExactly, KMMID_COL);
    QTreeWidgetItem* item = nullptr;

    if (! foundItems.empty())
        item = foundItems.at(0);

    // Give the user some options
    int result;
    if (_kmmID.contains(" ")) {
        if (item)
            result = KMessageBox::warningContinueCancel(this, i18n("Failed to retrieve an exchange rate for %1 from %2. It will be skipped this time.", _webID, item->text(SOURCE_COL)), i18n("Price Update Failed"));
        else
            return;
    } else if (!item) {
        return;
    } else {
        result = KMessageBox::questionYesNoCancel(this, QString::fromLatin1("<qt>%1</qt>").arg(i18n("Failed to retrieve a quote for %1 from %2.  Press <b>No</b> to remove the online price source from this security permanently, <b>Yes</b> to continue updating this security during future price updates or <b>Cancel</b> to stop the current update operation.", _webID, item->text(SOURCE_COL))), i18n("Price Update Failed"), KStandardGuiItem::yes(), KStandardGuiItem::no());
    }


    if (result == KMessageBox::No) {
        // Disable price updates for this security

        MyMoneyFileTransaction ft;
        try {
            // Get this security (by ID)
            MyMoneySecurity security = MyMoneyFile::instance()->security(_kmmID.toUtf8());

            // Set the quote source to blank
            security.setValue("kmm-online-source", QString());
            security.setValue("kmm-online-quote-system", QString());

            // Re-commit the security
            MyMoneyFile::instance()->modifySecurity(security);
            ft.commit();
        } catch (const MyMoneyException &e) {
            KMessageBox::error(this, QString("<qt>") + i18n("Cannot update security <b>%1</b>: %2", _webID, QString::fromLatin1(e.what())) + QString("</qt>"), i18n("Price Update Failed"));
        }
    }

    // As long as the user doesn't want to cancel, move on!
    if (result != KMessageBox::Cancel) {
        QTreeWidgetItem* next = nullptr;
        d->ui->prgOnlineProgress->setValue(d->ui->prgOnlineProgress->value() + 1);
        item->setSelected(false);

        // launch the NEXT one ... in case of m_fUpdateAll == false, we
        // need to parse the list to find the next selected one
        next = d->ui->lvEquityList->invisibleRootItem()->child(d->ui->lvEquityList->invisibleRootItem()->indexOfChild(item) + 1);
        if (!d->m_fUpdateAll) {
            while (next && !next->isSelected()) {
                d->ui->prgOnlineProgress->setValue(d->ui->prgOnlineProgress->value() + 1);
                next = d->ui->lvEquityList->invisibleRootItem()->child(d->ui->lvEquityList->invisibleRootItem()->indexOfChild(next) + 1);
            }
        }
        if (next) {
            d->m_webQuote.launch(next->text(WEBID_COL), next->text(KMMID_COL), next->text(SOURCE_COL));
        } else {
            finishUpdate();
        }
    } else {
        finishUpdate();
    }
}

void KEquityPriceUpdateDlg::slotReceivedCSVQuote(const QString& _kmmID, const QString& _webID, MyMoneyStatement& st)
{
    Q_D(KEquityPriceUpdateDlg);
    auto foundItems = d->ui->lvEquityList->findItems(_kmmID, Qt::MatchExactly, KMMID_COL);
    QTreeWidgetItem* item = nullptr;

    if (! foundItems.empty())
        item = foundItems.at(0);

    QTreeWidgetItem* next = nullptr;

    if (item) {
        auto file = MyMoneyFile::instance();
        MyMoneySecurity fromCurrency, toCurrency;

        if (!_kmmID.contains(QLatin1Char(' '))) {
            try {
                toCurrency = MyMoneyFile::instance()->security(_kmmID);
                fromCurrency = MyMoneyFile::instance()->security(toCurrency.tradingCurrency());
            } catch (const MyMoneyException &) {
                fromCurrency = toCurrency = MyMoneySecurity();
            }

        } else {
            QRegularExpressionMatch match(d->m_splitRegex.match(_kmmID));
            if (match.hasMatch()) {
                try {
                    fromCurrency = MyMoneyFile::instance()->security(match.captured(2).toUtf8());
                    toCurrency = MyMoneyFile::instance()->security(match.captured(1).toUtf8());
                } catch (const MyMoneyException &) {
                    fromCurrency = toCurrency = MyMoneySecurity();
                }
            }
        }

        if (d->m_updatingPricePolicy != eDialogs::UpdatePrice::All) {
            QStringList qSources = WebPriceQuote::quoteSources();
            for (auto it = st.m_listPrices.begin(); it != st.m_listPrices.end();) {
                MyMoneyPrice storedPrice = file->price(toCurrency.id(), fromCurrency.id(), (*it).m_date, true);
                bool priceValid = storedPrice.isValid();
                if (!priceValid)
                    ++it;
                else {
                    switch(d->m_updatingPricePolicy) {
                    case eDialogs::UpdatePrice::Missing:
                        it = st.m_listPrices.erase(it);
                        break;
                    case eDialogs::UpdatePrice::Downloaded:
                        if (!qSources.contains(storedPrice.source()))
                            it = st.m_listPrices.erase(it);
                        else
                            ++it;
                        break;
                    case eDialogs::UpdatePrice::SameSource:
                        if (storedPrice.source().compare((*it).m_sourceName) != 0)
                            it = st.m_listPrices.erase(it);
                        else
                            ++it;
                        break;
                    case eDialogs::UpdatePrice::Ask:
                    {
                        auto result = KMessageBox::questionYesNoCancel(this,
                                      i18n("For <b>%1</b> on <b>%2</b> price <b>%3</b> already exists.<br>"
                                           "Do you want to replace it with <b>%4</b>?",
                                           storedPrice.from(), storedPrice.date().toString(Qt::ISODate),
                                           QString().setNum(storedPrice.rate(storedPrice.to()).toDouble(), 'g', 10),
                                           QString().setNum((*it).m_amount.toDouble(), 'g', 10)),
                                      i18n("Price Already Exists"));
                        switch(result) {
                        case KMessageBox::ButtonCode::Yes:
                            ++it;
                            break;
                        case KMessageBox::ButtonCode::No:
                            it = st.m_listPrices.erase(it);
                            break;
                        default:
                        case KMessageBox::ButtonCode::Cancel:
                            finishUpdate();
                            return;
                            break;
                        }
                        break;
                    }
                    default:
                        ++it;
                        break;
                    }
                }
            }
        }

        if (!st.m_listPrices.isEmpty()) {
            MyMoneyFileTransaction ft;
            KMyMoneyUtils::processPriceList(st);
            ft.commit();

            // latest price could be in the last or in the first row
            MyMoneyStatement::Price priceClass;
            if (st.m_listPrices.first().m_date > st.m_listPrices.last().m_date)
                priceClass = st.m_listPrices.first();
            else
                priceClass = st.m_listPrices.last();

            // update latest price in dialog if applicable
            auto latestDate = QDate::fromString(item->text(DATE_COL),Qt::ISODate);
            if (latestDate <= priceClass.m_date && priceClass.m_amount.isPositive()) {
                item->setText(PRICE_COL, priceClass.m_amount.formatMoney(fromCurrency.tradingSymbol(), toCurrency.pricePrecision()));
                item->setText(DATE_COL, priceClass.m_date.toString(Qt::ISODate));
                item->setText(SOURCE_COL, priceClass.m_sourceName);
            }
            logStatusMessage(i18n("Price for %1 updated (id %2)", _webID, _kmmID));
            // make sure to make OK button available
        }
        d->ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);

        d->ui->prgOnlineProgress->setValue(d->ui->prgOnlineProgress->value() + 1);
        item->setSelected(false);

        // launch the NEXT one ... in case of m_fUpdateAll == false, we
        // need to parse the list to find the next selected one
        next = d->ui->lvEquityList->invisibleRootItem()->child(d->ui->lvEquityList->invisibleRootItem()->indexOfChild(item) + 1);
        if (!d->m_fUpdateAll) {
            while (next && !next->isSelected()) {
                d->ui->prgOnlineProgress->setValue(d->ui->prgOnlineProgress->value() + 1);
                next = d->ui->lvEquityList->invisibleRootItem()->child(d->ui->lvEquityList->invisibleRootItem()->indexOfChild(next) + 1);
            }
        }
    } else {
        logErrorMessage(i18n("Received a price for %1 (id %2), but this symbol is not on the list. Aborting entire update.", _webID, _kmmID));
    }

    if (next) {
        d->m_webQuote.launch(next->text(WEBID_COL), next->text(KMMID_COL), next->text(SOURCE_COL));
    } else {
        finishUpdate();
    }
}

void KEquityPriceUpdateDlg::slotReceivedQuote(const QString& _kmmID, const QString& _webID, const QDate& _date, const double& _price)
{
    Q_D(KEquityPriceUpdateDlg);
    auto foundItems = d->ui->lvEquityList->findItems(_kmmID, Qt::MatchExactly, KMMID_COL);
    QTreeWidgetItem* item = nullptr;

    if (! foundItems.empty())
        item = foundItems.at(0);

    QTreeWidgetItem* next = 0;

    if (item) {
        if (_price > 0.0f && _date.isValid()) {
            QDate date = _date;
            if (date > QDate::currentDate())
                date = QDate::currentDate();

            MyMoneyMoney price = MyMoneyMoney::ONE;
            QString id = _kmmID.toUtf8();
            MyMoneySecurity fromCurrency, toCurrency;
            if (_kmmID.contains(" ") == 0) {
                MyMoneySecurity security = MyMoneyFile::instance()->security(id);
                QString factor = security.value("kmm-online-factor");
                if (!factor.isEmpty()) {
                    price = price * MyMoneyMoney(factor);
                }
                try {
                    toCurrency = MyMoneyFile::instance()->security(id);
                    fromCurrency = MyMoneyFile::instance()->security(toCurrency.tradingCurrency());
                } catch (const MyMoneyException &) {
                    fromCurrency = toCurrency = MyMoneySecurity();
                }

            } else {
                QRegularExpressionMatch match(d->m_splitRegex.match(_kmmID));
                if (match.hasMatch()) {
                    try {
                        fromCurrency = MyMoneyFile::instance()->security(match.captured(2).toUtf8());
                        toCurrency = MyMoneyFile::instance()->security(match.captured(1).toUtf8());
                    } catch (const MyMoneyException &) {
                        fromCurrency = toCurrency = MyMoneySecurity();
                    }
                }
            }
            price *= MyMoneyMoney(_price, MyMoneyMoney::precToDenom(toCurrency.pricePrecision()));

            item->setText(PRICE_COL, price.formatMoney(fromCurrency.tradingSymbol(), toCurrency.pricePrecision()));
            item->setText(DATE_COL, date.toString(Qt::ISODate));
            logStatusMessage(i18n("Price for %1 updated (id %2)", _webID, _kmmID));
            // make sure to make OK button available
            d->ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        } else {
            logErrorMessage(i18n("Received an invalid price for %1, unable to update.", _webID));
        }

        d->ui->prgOnlineProgress->setValue(d->ui->prgOnlineProgress->value() + 1);
        item->setSelected(false);

        // launch the NEXT one ... in case of m_fUpdateAll == false, we
        // need to parse the list to find the next selected one
        next = d->ui->lvEquityList->invisibleRootItem()->child(d->ui->lvEquityList->invisibleRootItem()->indexOfChild(item) + 1);
        if (!d->m_fUpdateAll) {
            while (next && !next->isSelected()) {
                d->ui->prgOnlineProgress->setValue(d->ui->prgOnlineProgress->value() + 1);
                next = d->ui->lvEquityList->invisibleRootItem()->child(d->ui->lvEquityList->invisibleRootItem()->indexOfChild(next) + 1);
            }
        }
    } else {
        logErrorMessage(i18n("Received a price for %1 (id %2), but this symbol is not on the list. Aborting entire update.", _webID, _kmmID));
    }

    if (next) {
        d->m_webQuote.launch(next->text(WEBID_COL), next->text(KMMID_COL), next->text(SOURCE_COL));
    } else {
        finishUpdate();
    }
}

void KEquityPriceUpdateDlg::finishUpdate()
{
    Q_D(KEquityPriceUpdateDlg);
    // we've run past the end, reset to the default value.
    d->m_fUpdateAll = false;

    // force progress bar to show 100% and hide it after a while
    d->ui->prgOnlineProgress->setValue(d->ui->prgOnlineProgress->maximum());
    QTimer::singleShot(500, d->ui->prgOnlineProgress, &QProgressBar::hide);

    // re-enable the sorting that was disabled during the update process
    d->ui->lvEquityList->setSortingEnabled(true);
}

// Make sure, that these definitions are only used within this file
// this does not seem to be necessary, but when building RPMs the
// build option 'final' is used and all CPP files are concatenated.
// So it could well be, that in another CPP file these definitions
// are also used.
#undef WEBID_COL
#undef NAME_COL
#undef PRICE_COL
#undef DATE_COL
#undef KMMID_COL
#undef SOURCE_COL
