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

#include <alkimia/alkonlinequote.h>
#include <alkimia/alkonlinequotesource.h>

#include "dialogenums.h"
#include "icons.h"
#include "kequitypriceupdateconfdlg.h"
#include "kmmonlinequotesprofilemanager.h"
#include "kmymoneyutils.h"
#include "mymoneyaccount.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneymoney.h"
#include "mymoneyprice.h"
#include "mymoneysecurity.h"
#include "mymoneystatement.h"
#include "mymoneyutils.h"
#include "onlinepricemodel.h"
#include "onlinesourcedelegate.h"
#include "widgethintframe.h"

#include "kmmyesno.h"

class KEquityPriceUpdateDlgPrivate : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(KEquityPriceUpdateDlgPrivate)
    Q_DECLARE_PUBLIC(KEquityPriceUpdateDlg)

public:
    explicit KEquityPriceUpdateDlgPrivate(KEquityPriceUpdateDlg* qq)
        : QObject(qq)
        , q_ptr(qq)
        , ui(new Ui::KEquityPriceUpdateDlg)
        , m_fUpdateAll(false)
        , m_abortUpdate(false)
        , m_updatingPricePolicy(eDialogs::UpdatePrice::All)
        , m_splitRegex("([0-9a-z\\.]+)[^a-z0-9]+([0-9a-z\\.]+)", QRegularExpression::CaseInsensitiveOption)
        , m_currentRow(-1)
        , m_fqName(QLatin1String("Finance::Quote"))
        , m_kmmName(QLatin1String("kmymoney5"))
    {
        m_progressDelay.setInterval(1000);
        m_progressDelay.setSingleShot(true);
    }

    ~KEquityPriceUpdateDlgPrivate()
    {
        delete ui;
    }

    AlkOnlineQuotesProfile* quoteProfile(const QString& profileName) const
    {
        auto& manager = KMMOnlineQuotesProfileManager::instance();
        if (profileName.isEmpty()) {
            return manager.profile(m_kmmName);
        }
        return manager.profile(profileName);
    }

    void init(const QString& securityId)
    {
        Q_Q(KEquityPriceUpdateDlg);
        ui->setupUi(q);

        ui->equityView->setModel(&m_onlinePriceModel);
        ui->equityView->setFocus();

        auto delegate = new OnlineSourceDelegate(ui->equityView);
        ui->equityView->setItemDelegateForColumn(OnlinePriceModel::Source, delegate);

        ui->closeStatusButton->setIcon(Icons::get(Icons::Icon::DialogClose));
        connect(ui->closeStatusButton, &QToolButton::clicked, ui->statusContainer, &QWidget::hide);

        // hide the status widgets until there is activity
        ui->statusContainer->hide();

        m_fUpdateAll = false;

        auto file = MyMoneyFile::instance();

        //
        // Add each price pair that we know about
        //

        // send in securityId == "XXX YYY" to get a single-shot update for XXX to YYY.
        // for consistency reasons, this accepts the same delimiters as AlkOnlineQuote::launch()
        QRegularExpressionMatch match(m_splitRegex.match(securityId));
        MyMoneySecurityPair currencyIds;
        if (match.hasMatch()) {
            currencyIds = MyMoneySecurityPair(match.captured(1), match.captured(2));
        }

        MyMoneyPriceList prices = file->priceList();
        for (MyMoneyPriceList::ConstIterator it_price = prices.constBegin(); it_price != prices.constEnd(); ++it_price) {
            const MyMoneySecurityPair& pair = it_price.key();
            try {
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
                        addCurrencyConversion(pair, true);
                    }
                }
            } catch (MyMoneyException& e) {
                qDebug() << "Problem with price entry" << pair << e.what();
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
            }
        }

        // if list is empty and a price pair has been requested, add it
        if (m_onlinePriceModel.rowCount() == 0 && !securityId.isEmpty()) {
            addCurrencyConversion(currencyIds, true);
        }

        connect(ui->btnUpdateSelected, &QAbstractButton::clicked, this, &KEquityPriceUpdateDlgPrivate::slotUpdateSelected);
        connect(ui->btnUpdateAll, &QAbstractButton::clicked, this, &KEquityPriceUpdateDlgPrivate::slotUpdateAll);

        connect(ui->m_fromDate, &KMyMoneyDateEdit::dateChanged, this, &KEquityPriceUpdateDlgPrivate::slotDateChanged);
        connect(ui->m_toDate, &KMyMoneyDateEdit::dateChanged, this, &KEquityPriceUpdateDlgPrivate::slotDateChanged);

        connect(&m_quote, &AlkOnlineQuote::status, this, &KEquityPriceUpdateDlgPrivate::logStatusMessage);
        connect(&m_quote, &AlkOnlineQuote::error, this, &KEquityPriceUpdateDlgPrivate::logErrorMessage);
        connect(&m_quote, &AlkOnlineQuote::failed, this, &KEquityPriceUpdateDlgPrivate::slotQuoteFailed);
        connect(&m_quote, &AlkOnlineQuote::quote, this, &KEquityPriceUpdateDlgPrivate::slotReceivedQuote);

        connect(ui->equityView->selectionModel(), &QItemSelectionModel::selectionChanged, this, [&]() {
            updateButtonState();
        });

        // initialize progress bar which is shown only in case the download takes longer
        connect(&m_progressDelay, &QTimer::timeout, ui->prgOnlineProgress, &QProgressBar::show);
        ui->prgOnlineProgress->hide();

        ui->buttonBox->button(QDialogButtonBox::RestoreDefaults)->setText(i18nc("@action:button Configuration of price update", "Configure"));
        connect(ui->buttonBox->button(QDialogButtonBox::RestoreDefaults), &QAbstractButton::clicked, this, [&]() {
            QPointer<EquityPriceUpdateConfDlg> dlg = new EquityPriceUpdateConfDlg(m_updatingPricePolicy);
            if (dlg->exec() == QDialog::Accepted && (dlg != nullptr)) {
                m_updatingPricePolicy = dlg->policy();
            }
            delete dlg;
        });

        if (!securityId.isEmpty()) {
            ui->btnUpdateSelected->hide();
            ui->btnUpdateAll->hide();

            /// @todo replace with QMetaObject::invokeMethod
            QTimer::singleShot(100, this, &KEquityPriceUpdateDlgPrivate::slotUpdateAll);
        }

        // Hide OK button until we have received the first update
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        updateButtonState();

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

        // initially resize all columns to the content
        const auto columns = m_onlinePriceModel.columnCount();
        for (int column = 0; column < columns; ++column) {
            ui->equityView->resizeColumnToContents(column);
        }
        updateDateWidgetVisibility();
    }

    void updateDateWidgetVisibility()
    {
        // we only show the date widgets when one of the
        // quote sources supports date ranges.
        ui->m_dateContainer->setVisible(haveSourceWithDateRange());
    }

    void updateButtonState()
    {
        if (ui->m_fromDate->date().isValid() && ui->m_toDate->date().isValid()) {
            // Only enable the update all button if there is an item in the list
            ui->btnUpdateAll->setEnabled(m_onlinePriceModel.rowCount() > 0);
            // Only enable the update button if there is a selection
            ui->btnUpdateSelected->setEnabled(!ui->equityView->selectionModel()->selectedRows().isEmpty());
        } else {
            // upon invalid date, disable all buttons
            ui->btnUpdateAll->setEnabled(false);
            ui->btnUpdateSelected->setEnabled(false);
        }
    }

    void addCurrencyConversion(const MyMoneySecurityPair& pair, bool dontCheckExistance)
    {
        Q_ASSERT(!pair.first.trimmed().isEmpty());
        Q_ASSERT(!pair.second.trimmed().isEmpty());

        auto file = MyMoneyFile::instance();

        const auto symbol = QString::fromLatin1("%1 > %2").arg(pair.first, pair.second);
        const auto id = QString::fromLatin1("%1 %2").arg(pair.first, pair.second);
        // Check that the pair does not already exist
        if (!m_onlinePriceModel.indexById(id).isValid()) {
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
                    QString price;
                    QString date;
                    QString source = QLatin1String("KMyMoney Currency");
                    if (pr.isValid()) {
                        MyMoneySecurity fromCurrency = file->currency(pair.first);
                        MyMoneySecurity toCurrency = file->currency(pair.second);
                        price = pr.rate(pair.second).formatMoney(fromCurrency.tradingSymbol(), toCurrency.pricePrecision());
                        date = MyMoneyUtils::formatDate(pr.date());
                        source = pr.source();
                    }
                    m_onlinePriceModel.addOnlinePrice(id, symbol, i18n("%1 units for one %2", pair.first, pair.second), price, date, source);
                }
            }
        }
    }

    void addInvestment(const MyMoneySecurity& inv)
    {
        const auto id = inv.id();
        // Check that the pair does not already exist
        if (!m_onlinePriceModel.indexById(id).isValid()) {
            auto file = MyMoneyFile::instance();
            // check that the security is still in use
            QList<MyMoneyAccount>::const_iterator it_a;
            QList<MyMoneyAccount> list;
            file->accountList(list);
            for (it_a = list.constBegin(); it_a != list.constEnd(); ++it_a) {
                if ((*it_a).isInvest() && ((*it_a).currencyId() == inv.id()) && !(*it_a).isClosed())
                    break;
            }
            // if it is in use, it_a is not equal to list.end()
            if (it_a != list.constEnd()) {
                QString webID;
                AlkOnlineQuoteSource alkOnlineSource(inv.value(QLatin1String("kmm-online-source")),
                                                     quoteProfile(inv.value(QLatin1String("kmm-online-quote-system"))));
                switch (alkOnlineSource.idSelector()) {
                case AlkOnlineQuoteSource::IdentificationNumber:
                    webID = inv.value("kmm-security-id");   // insert ISIN number...
                    break;
                case AlkOnlineQuoteSource::Name:
                    webID = inv.name();                     // ...or name...
                    break;
                default:
                    webID = inv.tradingSymbol();            // ...or symbol
                    break;
                }
                if (webID.isEmpty()) {
                    webID = i18n("[No identifier]");
                }

                MyMoneySecurity currency = file->currency(inv.tradingCurrency());
                const MyMoneyPrice pr = file->price(id, inv.tradingCurrency());
                QString price;
                QString date;
                QString source = QLatin1String("KMyMoney Currency");
                if (pr.isValid()) {
                    price = pr.rate(currency.id()).formatMoney(currency.tradingSymbol(), inv.pricePrecision());
                    date = MyMoneyUtils::formatDate(pr.date());
                }
                if (inv.value("kmm-online-quote-system") == m_fqName) {
                    source = QString("%1 %2").arg(m_fqName, inv.value("kmm-online-source"));
                } else {
                    source = inv.value("kmm-online-source");
                }
                m_onlinePriceModel.addOnlinePrice(id, webID, inv.name(), price, date, source);

                // If this investment is denominated in a foreign currency, ensure that
                // the appropriate price pair is also on the list
                if (currency.id() != file->baseCurrency().id()) {
                    addCurrencyConversion(MyMoneySecurityPair(currency.id(), file->baseCurrency().id()), false);
                }
            }
        }
    }

    bool haveSourceWithDateRange() const
    {
        const auto rows = m_onlinePriceModel.rowCount();
        for (int row = 0; row < rows; ++row) {
            const auto idx = m_onlinePriceModel.index(row, OnlinePriceModel::Source);
            const auto source = idx.data().toString();
            const auto profileName = source.startsWith(m_fqName) ? m_fqName : QString();
            AlkOnlineQuoteSource alkOnlineSource(source, quoteProfile(profileName));
            if (alkOnlineSource.dataFormat() == AlkOnlineQuoteSource::CSV) {
                return true;
            }
        }
        return false;
    }

    void slotDateChanged()
    {
        QSignalBlocker blockFrom(ui->m_fromDate);
        QSignalBlocker blockTo(ui->m_toDate);
        if (ui->m_toDate->date() > QDate::currentDate())
            ui->m_toDate->setDate(QDate::currentDate());
        if (ui->m_fromDate->date() > ui->m_toDate->date())
            ui->m_fromDate->setDate(ui->m_toDate->date());
    }

    void slotUpdateSelected()
    {
        auto selectedItems = [&]() {
            if (m_fUpdateAll) {
                QModelIndexList indexList;
                const auto rows = m_onlinePriceModel.rowCount();
                for (int row = 0; row < rows; ++row) {
                    indexList.append(m_onlinePriceModel.index(row, 0));
                }
                return indexList;
            } else {
                return ui->equityView->selectionModel()->selectedRows();
            }
        };

        // disable sorting while the update is running to maintain the current order of items on which
        // the update process depends and which could be changed with sorting enabled due to the updated values
        ui->equityView->setSortingEnabled(false);

        // show the status widgets
        ui->statusContainer->show();

        const auto selectedIndexes = selectedItems();
        const auto rows = selectedIndexes.count();

        // and kick start the progress bar display
        ui->prgOnlineProgress->setMaximum(rows + 1);
        m_progressDelay.start();

        m_quote.setDateRange(ui->m_fromDate->date(), ui->m_toDate->date());
        m_quote.setUseSingleQuoteSignal(true);

        m_abortUpdate = false;
        for (int row = 0; !m_abortUpdate && row < rows; ++row) {
            m_currentRow = selectedIndexes.at(row).row();
            const auto symbol = m_onlinePriceModel.index(m_currentRow, OnlinePriceModel::Symbol).data().toString();
            const auto id = m_onlinePriceModel.index(m_currentRow, OnlinePriceModel::Id).data().toString();
            const auto source = m_onlinePriceModel.index(m_currentRow, OnlinePriceModel::Source).data().toString();
            const auto profileName = source.startsWith(m_fqName) ? m_fqName : QString();
            m_quote.setProfile(quoteProfile(profileName));
            m_quote.launch(symbol, id, source);
        }

        // we've run past the end, reset to the default value.
        m_fUpdateAll = false;
        m_currentRow = -1;
        ui->equityView->selectionModel()->clearSelection();

        // force progress bar to show 100% and hide it after a while
        m_progressDelay.stop();
        ui->prgOnlineProgress->setValue(ui->prgOnlineProgress->maximum());
        QTimer::singleShot(1500, ui->prgOnlineProgress, &QProgressBar::hide);

        // re-enable the sorting that was disabled during the update process
        ui->equityView->setSortingEnabled(true);

        if (rows == 0) {
            logErrorMessage(i18nc("@info online update price info", "No security selected."));
        }
    }

    void slotUpdateAll()
    {
        if (m_onlinePriceModel.rowCount() > 0) {
            m_fUpdateAll = true;
            slotUpdateSelected();
        } else {
            logErrorMessage(i18nc("@info online update price info", "No Security to be updated."));
        }
    }

    void logErrorMessage(const QString& message)
    {
        logStatusMessage(QString("<font color=\"red\"><b>") + message + QString("</b></font>"));
    }

    void logStatusMessage(const QString& message)
    {
        ui->lbStatus->append(message);
    }

    void slotQuoteFailed(const QString& _kmmID, const QString& _webID)
    {
        Q_Q(KEquityPriceUpdateDlg);
        // Give the user some options
        int result;
        if (m_currentRow != -1) {
            const auto source = m_onlinePriceModel.index(m_currentRow, OnlinePriceModel::Source).data().toString();
            if (_kmmID.contains(" ")) {
                result = KMessageBox::warningContinueCancel(
                    q,
                    i18n("Failed to retrieve an exchange rate for %1 from %2. It will be skipped this time.", _webID, source),
                    i18n("Price Update Failed"));
            } else {
                result = KMessageBox::questionTwoActionsCancel(
                    q,
                    QString::fromLatin1("<qt>%1</qt>")
                        .arg(i18n(
                            "Failed to retrieve a quote for %1 from %2.  Press <b>No</b> to remove the online price source from this security permanently, "
                            "<b>Yes</b> to continue updating this security during future price updates or <b>Cancel</b> to stop the current update operation.",
                            _webID,
                            source)),
                    i18n("Price Update Failed"),
                    KMMYesNo::yes(),
                    KMMYesNo::no());
            }

            if (result == KMessageBox::SecondaryAction) {
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
                } catch (const MyMoneyException& e) {
                    KMessageBox::error(q,
                                       QString("<qt>") + i18n("Cannot update security <b>%1</b>: %2", _webID, QString::fromLatin1(e.what())) + QString("</qt>"),
                                       i18n("Price Update Failed"));
                }
            }

            // As long as the user doesn't want to cancel, move on!
            m_abortUpdate = (result == KMessageBox::Cancel);
        }
    }

    void slotReceivedQuote(const QString& _kmmID, const QString& _webID, const QDate& _date, const double& _price)
    {
        if (m_currentRow != -1) {
            if (_price > 0.0f && _date.isValid()) {
                QDate date = _date;
                if (date > QDate::currentDate())
                    date = QDate::currentDate();

                MyMoneyMoney price = MyMoneyMoney::ONE;
                QString id = _kmmID.toUtf8();
                MyMoneySecurity fromCurrency, toCurrency;
                int precision(2);

                if (_kmmID.contains(" ") == 0) {
                    MyMoneySecurity security = MyMoneyFile::instance()->security(id);
                    QString factor = security.value("kmm-online-factor");
                    if (!factor.isEmpty()) {
                        price = price * MyMoneyMoney(factor);
                    }
                    try {
                        fromCurrency = MyMoneyFile::instance()->security(id);
                        toCurrency = MyMoneyFile::instance()->security(toCurrency.tradingCurrency());
                        precision = fromCurrency.pricePrecision();
                    } catch (const MyMoneyException&) {
                        fromCurrency = toCurrency = MyMoneySecurity();
                    }

                } else {
                    QRegularExpressionMatch match(m_splitRegex.match(_kmmID));
                    if (match.hasMatch()) {
                        try {
                            fromCurrency = MyMoneyFile::instance()->security(match.captured(1));
                            toCurrency = MyMoneyFile::instance()->security(match.captured(2));
                            precision = toCurrency.pricePrecision();
                        } catch (const MyMoneyException&) {
                            fromCurrency = toCurrency = MyMoneySecurity();
                        }
                    }
                }
                price *= MyMoneyMoney(_price, MyMoneyMoney::precToDenom(precision));

                // add this price info to the list of prices to be stored
                const auto fromCurrencyId = fromCurrency.id();
                const auto toCurrencyId = toCurrency.id();
                const auto dateString = MyMoneyUtils::dateToIsoString(date);
                const auto key = QString("%1-%2-%3").arg(fromCurrencyId, toCurrencyId, dateString);
                const auto source = modelData(m_currentRow, OnlinePriceModel::Source);
                m_prices.insert(key, MyMoneyPrice(fromCurrencyId, toCurrencyId, date, price, source));

                // update model for the display purposes
                m_onlinePriceModel.setData(m_onlinePriceModel.index(m_currentRow, OnlinePriceModel::Price),
                                           price.formatMoney(toCurrency.tradingSymbol(), precision),
                                           Qt::EditRole);
                m_onlinePriceModel.setData(m_onlinePriceModel.index(m_currentRow, OnlinePriceModel::Date), MyMoneyUtils::formatDate(date), Qt::EditRole);

                logStatusMessage(i18nc("@info Online price update %1 online id, %2 internal id, %3 price, %4 date",
                                       "Price for %1 updated to %3 for %4 (id %2)",
                                       _webID,
                                       _kmmID,
                                       price.formatMoney(toCurrency.tradingSymbol(), precision),
                                       MyMoneyUtils::formatDate(date)));
            } else {
                logErrorMessage(i18nc("@info Online price update %1 online id", "Received an invalid price for %1, unable to update.", _webID));
            }
        }
    }

    /**
     * Helper method to extract string data from online price model
     *
     * @param row row to get data from
     * @param coloumn column to get data from
     * @returns data found in cell converted to string
     *
     * @sa QVariant::toString()
     */
    QString modelData(int row, int column) const
    {
        return m_onlinePriceModel.index(row, column).data().toString();
    }

    KEquityPriceUpdateDlg* q_ptr;
    Ui::KEquityPriceUpdateDlg* ui;
    bool m_fUpdateAll;
    bool m_abortUpdate;
    eDialogs::UpdatePrice m_updatingPricePolicy;
    QRegularExpression m_splitRegex;
    AlkOnlineQuote m_quote;
    int m_currentRow; // row of current item processed in model
    OnlinePriceModel m_onlinePriceModel;
    QString m_fqName;
    QString m_kmmName;
    QMap<QString, MyMoneyPrice> m_prices;
    QTimer m_progressDelay;
};

KEquityPriceUpdateDlg::KEquityPriceUpdateDlg(QWidget *parent, const QString& securityId) :
    QDialog(parent),
    d_ptr(new KEquityPriceUpdateDlgPrivate(this))
{
    Q_D(KEquityPriceUpdateDlg);
    d->init(securityId);

    auto decorateEditWidget = [&](const QDate& date, QWidget* widget) {
        if (widget) {
            WidgetHintFrame::hide(widget, QString());
            if (!date.isValid()) {
                WidgetHintFrame::show(widget, i18nc("@info:tooltip", "The date is invalid."));
            }
        }
    };

    auto frameCollection = new WidgetHintFrameCollection(this);
    frameCollection->addFrame(new WidgetHintFrame(d->ui->m_fromDate));
    frameCollection->addFrame(new WidgetHintFrame(d->ui->m_toDate));

    connect(d->ui->m_fromDate, &KMyMoneyDateEdit::dateValidityChanged, this, [&](const QDate& date) {
        Q_D(KEquityPriceUpdateDlg);
        decorateEditWidget(date, qobject_cast<QWidget*>(sender()));
        d->updateButtonState();
    });
    connect(d->ui->m_toDate, &KMyMoneyDateEdit::dateValidityChanged, this, [&](const QDate& date) {
        Q_D(KEquityPriceUpdateDlg);
        decorateEditWidget(date, qobject_cast<QWidget*>(sender()));
        d->updateButtonState();
    });
    connect(&d->m_onlinePriceModel, &QAbstractItemModel::dataChanged, this, [&](const QModelIndex& topLeft, const QModelIndex& bottomRight) {
        Q_D(KEquityPriceUpdateDlg);
        // if at least one item is dirty, we enable the OK button
        d->ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        for (int row = topLeft.row(); row <= bottomRight.row(); ++row) {
            if (d->m_onlinePriceModel.index(row, 0).data(eMyMoney::Model::IsDirtyRole).toBool()) {
                // make sure to make OK button available
                d->ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
                break;
            }
        }
        d->updateDateWidgetVisibility();
    });
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

MyMoneyPrice KEquityPriceUpdateDlg::price(const QString& id) const
{
    Q_D(const KEquityPriceUpdateDlg);
    MyMoneyPrice price;
    const auto indexes = d->m_onlinePriceModel.match(d->m_onlinePriceModel.index(0, OnlinePriceModel::Id), Qt::DisplayRole, id, 1, Qt::MatchExactly);

    if (!indexes.isEmpty()) {
        const auto row = indexes.at(0).row();
        MyMoneyMoney rate(d->modelData(row, OnlinePriceModel::Price));
        if (!rate.isZero()) {
            QString kmm_id = d->modelData(row, OnlinePriceModel::Id);
            const auto date = MyMoneyUtils::stringToDate(d->modelData(row, OnlinePriceModel::Date), QLocale::ShortFormat);
            const auto source = d->modelData(row, OnlinePriceModel::Source);
            // if the ID has a space, then this is TWO ID's, so it's a currency quote
            if (kmm_id.contains(" ")) {
                QStringList ids = kmm_id.split(' ', Qt::SkipEmptyParts);
                QString fromid = ids[0].toUtf8();
                QString toid = ids[1].toUtf8();
                price = MyMoneyPrice(fromid, toid, date, rate, source);
            } else
                // otherwise, it's a security quote
            {
                MyMoneySecurity security = MyMoneyFile::instance()->security(kmm_id);
                price = MyMoneyPrice(kmm_id, security.tradingCurrency(), date, rate, source);
            }
        }
    }
    return price;
}

void KEquityPriceUpdateDlg::storePrices()
{
    Q_D(KEquityPriceUpdateDlg);
    auto file = MyMoneyFile::instance();

    MyMoneyFileTransaction ft;
    try {
        const auto rows = d->m_prices.count();
        int row = 0;
        for (const auto& price : qAsConst(d->m_prices)) {
            // turn on MyMoneyFile signal emission on last entry only
            file->blockSignals(row < (rows - 1));
            ++row;
            file->addPrice(price);
        }
        ft.commit();

    } catch (const MyMoneyException &) {
        qDebug() << "Unable to add downloaded prices to storage";
    }
}

#include "kequitypriceupdatedlg.moc"
