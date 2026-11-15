/*
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "onlinepriceupdater.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDebug>
#include <QRegularExpression>
#include <QSet>

#include <algorithm>

// ----------------------------------------------------------------------------
// Project Includes

#include <alkimia/alkonlinequotesource.h>

#include "kmmonlinequotesprofilemanager.h"
#include "mymoneyaccount.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneymoney.h"
#include "mymoneysecurity.h"

namespace {
// quotes that have not been delivered after this time are given up
constexpr int quoteTimeout = 30000;
// time to wait for further quotes after the first one of a request has arrived
constexpr int settleTime = 500;
// number of days to ask for from sources that deliver a range of dates
constexpr int dateRangeDays = 7;

const QLatin1String financeQuoteName("Finance::Quote");
const QLatin1String defaultProfileName("kmymoney5");
const QLatin1String manualPriceSource("KMyMoney");
const QLatin1String currencyPriceSource("KMyMoney Currency");
// key in the data file for the day of the last update that stored prices
const QLatin1String lastUpdateKey("kmm-last-auto-price-update");
// the stored prices are retried when the file is busy, this often and this far apart
constexpr int maxStoreAttempts = 12;
constexpr int storeRetryInterval = 5000;

AlkOnlineQuotesProfile* quoteProfile(const QString& profileName)
{
    auto& manager = KMMOnlineQuotesProfileManager::instance();
    return manager.profile(profileName.isEmpty() ? QString(defaultProfileName) : profileName);
}

AlkOnlineQuotesProfile* profileForSource(const QString& source)
{
    return quoteProfile(source.startsWith(financeQuoteName) ? QString(financeQuoteName) : QString());
}
}

OnlinePriceUpdater::OnlinePriceUpdater(QObject* parent)
    : QObject(parent)
    , m_current(-1)
    , m_storeAttempts(0)
    , m_running(false)
    , m_inLaunch(false)
    , m_currentFailed(false)
    , m_currentDone(false)
{
    m_timeout.setSingleShot(true);
    m_timeout.setInterval(quoteTimeout + 5000);
    m_settle.setSingleShot(true);
    m_settle.setInterval(settleTime);
    m_retryStore.setSingleShot(true);
    m_retryStore.setInterval(storeRetryInterval);

    m_quote.setTimeout(quoteTimeout);
    m_quote.setUseSingleQuoteSignal(true);

    connect(&m_quote, &AlkOnlineQuote::quote, this, &OnlinePriceUpdater::slotQuote);
    connect(&m_quote, &AlkOnlineQuote::failed, this, &OnlinePriceUpdater::slotFailed);
    connect(&m_timeout, &QTimer::timeout, this, &OnlinePriceUpdater::slotRequestComplete);
    connect(&m_settle, &QTimer::timeout, this, &OnlinePriceUpdater::slotRequestComplete);
    connect(&m_retryStore, &QTimer::timeout, this, &OnlinePriceUpdater::tryStore);
}

OnlinePriceUpdater::~OnlinePriceUpdater() = default;

bool OnlinePriceUpdater::isRunning() const
{
    return m_running;
}

QDate OnlinePriceUpdater::lastUpdateDate() const
{
    return QDate::fromString(MyMoneyFile::instance()->value(lastUpdateKey), Qt::ISODate);
}

QList<OnlinePriceUpdater::Request> OnlinePriceUpdater::collectRequests() const
{
    QList<Request> requests;
    QSet<QString> requestedIds;

    const auto file = MyMoneyFile::instance();
    const auto baseCurrencyId = file->baseCurrency().id();

    // currencies of open accounts and securities held in open investment accounts
    QSet<QString> currenciesInUse;
    QSet<QString> securitiesInUse;
    QList<MyMoneyAccount> accounts;
    file->accountList(accounts);
    for (const auto& account : std::as_const(accounts)) {
        if (account.isClosed()) {
            continue;
        }
        currenciesInUse.insert(account.currencyId());
        if (account.isInvest()) {
            securitiesInUse.insert(account.currencyId());
            try {
                currenciesInUse.insert(file->security(account.currencyId()).tradingCurrency());
            } catch (const MyMoneyException&) {
            }
        }
    }

    auto addCurrencyConversion = [&](const MyMoneySecurityPair& pair) {
        const auto id = QStringLiteral("%1 %2").arg(pair.first, pair.second);
        if (requestedIds.contains(id)) {
            return;
        }

        // prices entered by hand are not updated online
        const MyMoneyPrice price = file->price(pair.first, pair.second);
        if (price.source() == manualPriceSource) {
            return;
        }

        // only keep the exchange rates of currencies that are still in use
        if (pair.first == baseCurrencyId || pair.second == baseCurrencyId) {
            if (!currenciesInUse.contains(file->foreignCurrency(pair.first, pair.second))) {
                return;
            }
        }

        const auto source = price.isValid() ? price.source() : QString(currencyPriceSource);
        requestedIds.insert(id);
        requests.append({id, QStringLiteral("%1 > %2").arg(pair.first, pair.second), QStringLiteral("%1 > %2").arg(pair.first, pair.second), source});
    };

    // exchange rates that have been downloaded before
    const MyMoneyPriceList priceList = file->priceList();
    for (auto it = priceList.cbegin(); it != priceList.cend(); ++it) {
        const MyMoneySecurityPair& pair = it.key();
        try {
            if (pair.first.trimmed().isEmpty() || pair.second.trimmed().isEmpty()) {
                continue;
            }
            if (!file->security(pair.first).isCurrency() || !file->security(pair.second).isCurrency()) {
                continue;
            }
            const MyMoneyPriceEntries& entries = it.value();
            if (entries.count() > 0 && entries.begin().key() <= QDate::currentDate()) {
                addCurrencyConversion(pair);
            }
        } catch (const MyMoneyException& e) {
            qDebug() << "Problem with price entry" << pair << e.what();
        }
    }

    // securities with an online source
    const auto securities = file->securityList();
    for (const auto& security : securities) {
        const auto onlineSource = security.value(QStringLiteral("kmm-online-source"));
        if (security.isCurrency() || onlineSource.isEmpty() || !securitiesInUse.contains(security.id())) {
            continue;
        }

        const auto quoteSystem = security.value(QStringLiteral("kmm-online-quote-system"));
        AlkOnlineQuoteSource alkOnlineSource(onlineSource, quoteProfile(quoteSystem));
        QString symbol;
        switch (alkOnlineSource.idSelector()) {
        case AlkOnlineQuoteSource::IdentificationNumber:
            symbol = security.value(QStringLiteral("kmm-security-id"));
            break;
        case AlkOnlineQuoteSource::Name:
            symbol = security.name();
            break;
        default:
            symbol = security.tradingSymbol();
            break;
        }
        if (symbol.isEmpty()) {
            continue;
        }

        const auto source = (quoteSystem == financeQuoteName) ? QStringLiteral("%1 %2").arg(financeQuoteName, onlineSource) : onlineSource;
        requests.append({security.id(), symbol, security.name(), source});

        // a security traded in a foreign currency also needs the exchange rate
        const auto tradingCurrencyId = security.tradingCurrency();
        if (!tradingCurrencyId.isEmpty() && tradingCurrencyId != baseCurrencyId) {
            addCurrencyConversion(MyMoneySecurityPair(tradingCurrencyId, baseCurrencyId));
        }
    }

    return requests;
}

MyMoneyPrice OnlinePriceUpdater::priceFromQuote(const QString& kmmId, const QDate& quoteDate, double quote, const QString& source) const
{
    if (quote <= 0.0 || !quoteDate.isValid()) {
        return MyMoneyPrice();
    }

    const auto date = qMin(quoteDate, QDate::currentDate());
    MyMoneyMoney price = MyMoneyMoney::ONE;
    MyMoneySecurity fromCurrency;
    MyMoneySecurity toCurrency;
    int precision = 2;

    try {
        const auto file = MyMoneyFile::instance();
        if (!kmmId.contains(QLatin1Char(' '))) {
            // a security
            fromCurrency = file->security(kmmId);
            const auto factor = fromCurrency.value(QStringLiteral("kmm-online-factor"));
            if (!factor.isEmpty()) {
                price = price * MyMoneyMoney(factor);
            }
            toCurrency = file->security(fromCurrency.tradingCurrency());
            precision = fromCurrency.pricePrecision();
        } else {
            // a pair of currencies, separated by blanks
            static const QRegularExpression splitRegex(QStringLiteral("([0-9a-z\\.]+)[^a-z0-9]+([0-9a-z\\.]+)"), QRegularExpression::CaseInsensitiveOption);
            const auto match = splitRegex.match(kmmId);
            if (!match.hasMatch()) {
                return MyMoneyPrice();
            }
            fromCurrency = file->security(match.captured(1));
            toCurrency = file->security(match.captured(2));
            precision = toCurrency.pricePrecision();
        }
    } catch (const MyMoneyException&) {
        return MyMoneyPrice();
    }

    if (fromCurrency.id().isEmpty() || toCurrency.id().isEmpty()) {
        return MyMoneyPrice();
    }

    price *= MyMoneyMoney(quote, MyMoneyMoney::precToDenom(precision));
    return MyMoneyPrice(fromCurrency.id(), toCurrency.id(), date, price, source);
}

void OnlinePriceUpdater::start()
{
    if (m_running) {
        return;
    }

    m_requests = collectRequests();
    m_prices.clear();
    m_failed.clear();
    m_current = -1;
    m_running = true;
    launchNext();
}

void OnlinePriceUpdater::cancel()
{
    m_running = false;
    m_timeout.stop();
    m_settle.stop();
    m_retryStore.stop();
    m_requests.clear();
    m_prices.clear();
}

void OnlinePriceUpdater::launchNext()
{
    if (!m_running) {
        return;
    }

    ++m_current;
    if (m_current >= m_requests.size()) {
        finish();
        return;
    }

    const auto& request = m_requests.at(m_current);
    m_received.clear();
    m_currentFailed = false;
    m_currentDone = false;

    m_quote.setProfile(profileForSource(request.source));
    m_quote.setDateRange(QDate::currentDate().addDays(-dateRangeDays), QDate::currentDate());

    // The quotes may be delivered while launch() is still running or later,
    // depending on the kind of source.
    m_timeout.start();
    m_inLaunch = true;
    const bool launched = m_quote.launch(request.symbol, request.kmmId, request.source);
    m_inLaunch = false;

    if (m_currentDone || !m_running) {
        return;
    }
    if (!launched || m_currentFailed || !m_received.isEmpty()) {
        finishCurrent();
    }
}

bool OnlinePriceUpdater::isCurrent(const QString& kmmId) const
{
    return m_running && !m_currentDone && m_current >= 0 && m_current < m_requests.size() && m_requests.at(m_current).kmmId == kmmId;
}

void OnlinePriceUpdater::slotQuote(const QString& kmmId, const QString& symbol, const QDate& date, double price)
{
    Q_UNUSED(symbol)
    if (!isCurrent(kmmId)) {
        return;
    }

    const auto received = priceFromQuote(kmmId, date, price, m_requests.at(m_current).source);
    if (!received.isValid()) {
        return;
    }
    // keep one price per date, a later quote for the same date replaces the earlier one
    auto sameDate = std::find_if(m_received.begin(), m_received.end(), [&](const MyMoneyPrice& candidate) {
        return candidate.date() == received.date();
    });
    if (sameDate != m_received.end()) {
        *sameDate = received;
    } else {
        m_received.append(received);
    }

    // inside launch() the request is completed when launch() returns,
    // otherwise wait a moment for further quotes
    if (!m_inLaunch) {
        m_settle.start();
    }
}

void OnlinePriceUpdater::slotFailed(const QString& kmmId, const QString& symbol)
{
    Q_UNUSED(symbol)
    if (!isCurrent(kmmId)) {
        return;
    }
    m_currentFailed = true;
    if (!m_inLaunch && m_received.isEmpty()) {
        finishCurrent();
    }
}

void OnlinePriceUpdater::slotRequestComplete()
{
    finishCurrent();
}

void OnlinePriceUpdater::finishCurrent()
{
    if (!m_running || m_currentDone || m_current < 0 || m_current >= m_requests.size()) {
        return;
    }
    m_currentDone = true;
    m_timeout.stop();
    m_settle.stop();

    if (!m_received.isEmpty()) {
        m_prices.append(m_received);
    } else {
        m_failed.append(m_requests.at(m_current).name);
    }

    QMetaObject::invokeMethod(this, &OnlinePriceUpdater::launchNext, Qt::QueuedConnection);
}

void OnlinePriceUpdater::finish()
{
    m_storeAttempts = 0;
    tryStore();
}

void OnlinePriceUpdater::tryStore()
{
    if (!m_running) {
        return;
    }

    int stored = 0;
    if (!storePrices(stored)) {
        // the file may be busy with a transaction of the user, try again a little later
        if (++m_storeAttempts < maxStoreAttempts) {
            m_retryStore.start();
            return;
        }
        m_running = false;
        Q_EMIT finished(0, m_failed, true);
        return;
    }

    m_running = false;
    Q_EMIT finished(stored, m_failed, false);
}

bool OnlinePriceUpdater::storePrices(int& stored)
{
    stored = 0;
    if (m_prices.isEmpty()) {
        return true;
    }

    const auto file = MyMoneyFile::instance();
    if (file->hasTransaction()) {
        return false;
    }

    try {
        MyMoneyFileTransaction ft;

        // only add the missing days, an existing price is never replaced
        QList<MyMoneyPrice> toStore;
        for (const auto& price : std::as_const(m_prices)) {
            const MyMoneyPrice existing = file->price(price.from(), price.to(), price.date(), true);
            if (existing.isValid()) {
                continue;
            }
            toStore.append(price);
        }

        for (int i = 0; i < toStore.size(); ++i) {
            // emit the file's signals for the last price only
            file->blockSignals(i < toStore.size() - 1);
            file->addPrice(toStore.at(i));
            ++stored;
        }

        // remember in the file that this was done today
        if (stored > 0) {
            file->setValue(lastUpdateKey, QDate::currentDate().toString(Qt::ISODate));
        }
        ft.commit();
    } catch (const MyMoneyException&) {
        file->blockSignals(false);
        qDebug() << "Unable to add downloaded prices to storage";
        stored = 0;
        return false;
    }
    return true;
}
