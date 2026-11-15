/*
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ONLINEPRICEUPDATER_H
#define ONLINEPRICEUPDATER_H

#include "kmm_webconnect_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>

// ----------------------------------------------------------------------------
// Project Includes

#include <alkimia/alkonlinequote.h>

#include "mymoneyprice.h"

/**
 * Updates the prices of all securities and currencies that have an online
 * source configured, without any user interaction.
 *
 * The quotes are requested one after the other. A failed quote is only
 * reported in the result, it never changes the configuration of the
 * security. All dates a source returns for a request are stored, but only
 * the days for which the file has no price yet: an existing price, entered
 * manually or downloaded earlier, is never replaced. The received prices are
 * stored in a single transaction once all quotes have been processed.
 */
class KMM_WEBCONNECT_EXPORT OnlinePriceUpdater : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(OnlinePriceUpdater)

public:
    struct Request {
        QString kmmId; ///< id of the security or "<from> <to>" for a currency pair
        QString symbol; ///< identifier used by the online source
        QString name; ///< name used in messages
        QString source; ///< name of the online source
    };

    explicit OnlinePriceUpdater(QObject* parent = nullptr);
    ~OnlinePriceUpdater() override;

    bool isRunning() const;

    /**
     * Returns the day on which an update stored prices in the file
     * currently loaded, or an invalid date if there is none.
     */
    QDate lastUpdateDate() const;

public Q_SLOTS:
    /**
     * Starts the update. Does nothing if an update is already running.
     */
    void start();

    /**
     * Stops a running update without storing anything. Use this when the
     * file is closed while an update is running.
     */
    void cancel();

Q_SIGNALS:
    /**
     * Emitted when the update is complete and the prices have been stored.
     * @a updated is the number of stored prices, @a failed contains the
     * names of the quotes that could not be retrieved. @a storeFailed is
     * true if the file stayed busy (another transaction was active) or
     * the prices could not be stored for another reason.
     */
    void finished(int updated, const QStringList& failed, bool storeFailed);

private Q_SLOTS:
    void launchNext();
    void slotQuote(const QString& kmmId, const QString& symbol, const QDate& date, double price);
    void slotFailed(const QString& kmmId, const QString& symbol);
    void slotRequestComplete();
    void tryStore();

private:
    /**
     * Returns the quotes to request for the file currently loaded: the
     * securities in use by an open investment account and the currency
     * pairs that are still in use, each with its configured online source.
     */
    QList<Request> collectRequests() const;

    /**
     * Converts a quote received for @a kmmId into a price. The returned
     * price is invalid if the quote cannot be used.
     */
    MyMoneyPrice priceFromQuote(const QString& kmmId, const QDate& quoteDate, double quote, const QString& source) const;

    bool isCurrent(const QString& kmmId) const;
    void finishCurrent();
    void finish();

    /**
     * Stores the received prices. Returns false, and stores nothing, if the
     * file is busy with another transaction or the prices cannot be added.
     */
    bool storePrices(int& stored);

    AlkOnlineQuote m_quote;
    QTimer m_timeout;
    QTimer m_settle;
    QTimer m_retryStore;
    QList<Request> m_requests;
    QList<MyMoneyPrice> m_prices;
    QStringList m_failed;
    QList<MyMoneyPrice> m_received;
    int m_current;
    int m_storeAttempts;
    bool m_running;
    bool m_inLaunch;
    bool m_currentFailed;
    bool m_currentDone;
};

#endif // ONLINEPRICEUPDATER_H
