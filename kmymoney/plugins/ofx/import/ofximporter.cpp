/*
    SPDX-FileCopyrightText: 2005 Ace Jones acejones @users.sourceforge.net
    SPDX-FileCopyrightText: 2010-2019 Thomas Baumgart tbaumgart @kde.org
    SPDX-FileCopyrightText: 2021-2022 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <config-kmymoney.h>
#include "ofximporter.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QByteArray>
#include <QFile>
#include <QRadioButton>
#include <QSpinBox>
#include <QTextCodec>
#include <QTextStream>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KActionCollection>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPluginFactory>
#include <KSharedConfig>
#include <QUrl>

// ----------------------------------------------------------------------------
// Project Includes

#include "importinterface.h"
#include "kmmkeychain.h"
#include "kmymoneyutils.h"
#include "kofxdirectconnectdlg.h"
#include "konlinebankingsetupwizard.h"
#include "konlinebankingstatus.h"
#include "mymoneyaccount.h"
#include "mymoneyexception.h"
#include "mymoneystatement.h"
#include "mymoneystatementreader.h"
#include "statementinterface.h"
#include "ui_importoption.h"
#include "viewinterface.h"
#include <libofx/libofx.h>

// #define DEBUG_LIBOFX

typedef enum  {
    UniqueIdUnknown = -1,
    UniqueIdOfx = 0,
    UniqueIdKMyMoney,
} UniqueTransactionIdSource;

class OFXImporter::Private
{
public:
    Private()
        : m_valid(false)
        , m_preferName(PreferId)
        , m_uniqueIdSource(UniqueIdUnknown)
        , m_invertAmount(false)
        , m_fixBuySellSignage(false)
        , m_statusDlg(nullptr)
        , m_action(nullptr)
        , m_updateStartDate(QDate(1900, 1, 1))
        , m_timestampOffset(0)
    {
    }

    bool m_valid;
    enum NamePreference {
        PreferId = 0,
        PreferName,
        PreferMemo,
    } m_preferName;
    UniqueTransactionIdSource m_uniqueIdSource;
    bool m_invertAmount;
    bool m_fixBuySellSignage;
    QList<MyMoneyStatement> m_statementlist;
    QList<MyMoneyStatement::Security> m_securitylist;
    QString m_fatalerror;
    QStringList m_infos;
    QStringList m_warnings;
    QStringList m_errors;
    KOnlineBankingStatus* m_statusDlg;
    QAction* m_action;
    QDate m_updateStartDate;
    int m_timestampOffset;
    QSet<QString> m_hashes;

    int constructTimeOffset(const QTimeEdit* timestampOffset, const KComboBox* timestampOffsetSign) const
    {
        // get offset in minutes
        int offset = timestampOffset->time().msecsSinceStartOfDay() / 1000 / 60;
        if (timestampOffsetSign->currentText() == QStringLiteral("-")) {
            offset = -offset;
        }
        return offset;
    }

    QString sanitizedString(const char* str)
    {
        auto text = QString::fromUtf8(str);
        return text.replace(QChar(0), QString());
    }
};


static UniqueTransactionIdSource defaultIdSource()
{
    KSharedConfigPtr config = KSharedConfig::openConfig(QStringLiteral("kmymoney/ofximporterrc"));
    KConfigGroup grp = config->group("General");

    return (grp.readEntry<bool>("useOwnFITID", false) == true) ? UniqueIdKMyMoney : UniqueIdOfx;
}


OFXImporter::OFXImporter(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args) :
    KMyMoneyPlugin::Plugin(parent, metaData, args),
    /*
     * the string in the line above must be the same as
     * X-KDE-PluginInfo-Name and the provider name assigned in
     * OfxImporterPlugin::onlineBankingSettings()
     */
    KMyMoneyPlugin::ImporterPlugin(),
    d(new Private)
{
    Q_INIT_RESOURCE(ofximporter);

    const auto rcFileName = QLatin1String("ofximporter.rc");

    setXMLFile(rcFileName);

    createActions();

    // For ease announce that we have been loaded.
    qDebug("Plugins: ofximporter loaded");
}

OFXImporter::~OFXImporter()
{
    delete d;
    qDebug("Plugins: ofximporter unloaded");
}

void OFXImporter::createActions()
{
    d->m_action = actionCollection()->addAction(QStringLiteral("file_import_ofx"));
    d->m_action->setText(i18n("OFX..."));
    connect(d->m_action, &QAction::triggered, this, static_cast<void (OFXImporter::*)()>(&OFXImporter::slotImportFile));
    connect(viewInterface(), &KMyMoneyPlugin::ViewInterface::viewStateChanged, d->m_action, &QAction::setEnabled);
}

void OFXImporter::slotImportFile()
{
    QWidget * widget = new QWidget;
    Ui_ImportOption* option = new Ui_ImportOption;
    option->setupUi(widget);

    // initially set to global default option
    option->m_uniqueIdSource->setCurrentIndex(defaultIdSource());

    QUrl url = importInterface()->selectFile(i18n("OFX import file selection"),
               QString(),
               QStringLiteral("*.ofx *.qfx *.ofc|OFX files (*.ofx *.qfx *.ofc);;*|All files (*)"),
               QFileDialog::ExistingFile,
               widget);

    d->m_preferName = static_cast<OFXImporter::Private::NamePreference>(option->m_preferName->currentIndex());
    d->m_uniqueIdSource = static_cast<UniqueTransactionIdSource>(option->m_uniqueIdSource->currentIndex());
    d->m_timestampOffset = d->constructTimeOffset(option->m_timestampOffset, option->m_timestampOffsetSign);
    d->m_invertAmount = option->m_invertAmount->isChecked();
    d->m_fixBuySellSignage = option->m_fixBuySellSignage->isChecked();

    if (url.isValid()) {
        const QString filename(url.toLocalFile());
        if (isMyFormat(filename)) {
            statementInterface()->resetMessages();
            slotImportFile(filename);
            statementInterface()->showMessages(d->m_statementlist.count());

        } else {
            KMessageBox::error(0, i18n("Unable to import %1 using the OFX importer plugin.  This file is not the correct format.", url.toDisplayString()), i18n("Incorrect format"));
        }
    }
    delete option;
    delete widget;
}

QStringList OFXImporter::formatMimeTypes() const
{
    return {"application/x-ofx", "application/vnd.intu.qfx", "application/x-ofc"};
}


bool OFXImporter::isMyFormat(const QString& filename) const
{
    // filename is considered an Ofx file if it contains
    // the tag "<OFX>" or "<OFC>" in the first 20 lines.
    // which contain some data
    bool result = false;

    QFile f(filename);
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream ts(&f);

        int lineCount = 20;
        while (!ts.atEnd() && !result  && lineCount != 0) {
            // get a line of data and remove all unnecessary whitepace chars
            QString line = ts.readLine().simplified();
            if (line.contains(QStringLiteral("<OFX>"), Qt::CaseInsensitive)
                    || line.contains(QStringLiteral("<OFC>"), Qt::CaseInsensitive))
                result = true;
            // count only lines that contain some non white space chars
            if (!line.isEmpty())
                lineCount--;
        }
        f.close();
    } else {
        qDebug() << "OFXImporter::isMyFormat: unable to open" << filename << "with" << f.errorString();
    }

    return result;
}

bool OFXImporter::import(const QString& filename)
{
    d->m_fatalerror = i18n("Unable to parse file");
    d->m_valid = false;
    d->m_errors.clear();
    d->m_warnings.clear();
    d->m_infos.clear();

    d->m_statementlist.clear();
    d->m_securitylist.clear();

    QByteArray filename_deep = QFile::encodeName(filename);

#ifndef Q_OS_WIN

    // setting the global variables in the windows version
    // created on binary-factory.kde.org causes the following
    // assignment statements to create an access violation exception
    // which crashes the application. So we avoid setting
    // them at all under Windows.
    ofx_STATUS_msg = true;
    ofx_INFO_msg  = true;
    ofx_WARNING_msg = true;
    ofx_ERROR_msg = true;

    // Don't show the position that caused a message to be shown
    // This has no setter (see libofx.h)
    ofx_show_position = false;

#ifdef DEBUG_LIBOFX
    ofx_PARSER_msg = true;
    ofx_DEBUG_msg = true;
    ofx_DEBUG1_msg = true;
    ofx_DEBUG2_msg = true;
    ofx_DEBUG3_msg = true;
    ofx_DEBUG4_msg = true;
    ofx_DEBUG5_msg = true;
#endif
#endif

    LibofxContextPtr ctx = libofx_get_new_context();
    Q_CHECK_PTR(ctx);

    d->m_hashes.clear();

    qDebug("setup callback routines");
    ofx_set_transaction_cb(ctx, ofxTransactionCallback, this);
    ofx_set_statement_cb(ctx, ofxStatementCallback, this);
    ofx_set_account_cb(ctx, ofxAccountCallback, this);
    ofx_set_security_cb(ctx, ofxSecurityCallback, this);
    ofx_set_status_cb(ctx, ofxStatusCallback, this);
    qDebug("process data");

#ifdef Q_OS_LINUX
    // libofx needs to know where to pick up the DTD
    // files when running in APPIMAGE mode
    const auto env = qgetenv("APPDIR");
    if (!env.isEmpty()) {
        QByteArray dir(env);
        dir.append("/usr/share/libofx/dtd/");
        qDebug() << "Set DTD dir to" << dir;
        libofx_set_dtd_dir(ctx, dir.data());
    }
#endif

#ifdef Q_OS_MACOS
    // libofx needs to know where to pick up the DTD
    // files when running on MacOS
    QByteArray dir("/Applications/kmymoney.app/Contents/Resources/libofx/dtd/");
    qDebug() << "Set DTD dir to" << dir;
    libofx_set_dtd_dir(ctx, dir.data());
#endif

#ifdef Q_OS_WIN
    const auto dir = QFile::encodeName(QStringLiteral("%1/data/libofx/dtd/").arg(QCoreApplication::applicationDirPath()));
    qDebug() << "Set DTD dir to" << dir;
    libofx_set_dtd_dir(ctx, dir);
#endif

    qDebug() << "Start processing OFX data from" << filename_deep;
    libofx_proc_file(ctx, filename_deep, AUTODETECT);
    qDebug() << "Processing OFX data done";
    libofx_free_context(ctx);

    if (d->m_valid) {
        d->m_fatalerror.clear();
        d->m_valid = storeStatements(d->m_statementlist);
    }
    return d->m_valid;
}

QString OFXImporter::lastError() const
{
    if (d->m_errors.count() == 0)
        return d->m_fatalerror;
    return d->m_errors.join(QStringLiteral("<p>"));
}

/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *
 * Static callbacks for LibOFX
 *
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */

int OFXImporter::ofxTransactionCallback(struct OfxTransactionData data, void * pv)
{
//   kDebug(2) << Q_FUNC_INFO;

    OFXImporter* pofx = reinterpret_cast<OFXImporter*>(pv);
    OFXImporter::Private* d = pofx->d;
    MyMoneyStatement& s = pofx->back();

    MyMoneyStatement::Transaction t;

    if (data.date_posted_valid) {
        QDateTime dt;
        dt.setSecsSinceEpoch(data.date_posted - d->m_timestampOffset * 60);
        t.m_datePosted = dt.date();
    } else if (data.date_initiated_valid) {
        QDateTime dt;
        dt.setSecsSinceEpoch(data.date_initiated - d->m_timestampOffset * 60);
        t.m_datePosted = dt.date();
    }
    if (t.m_datePosted.isValid()) {
        // verify the transaction date is one we want
        if (t.m_datePosted < d->m_updateStartDate) {
            //kDebug(0) << "discarding transaction dated" << qPrintable(t.m_datePosted.toString(Qt::ISODate));
            return 0;
        }
    }

    bool unhandledtype = false;
    QString type;

    if (data.invtransactiontype_valid) {
        switch (data.invtransactiontype) {
        case OFX_BUYDEBT:
        case OFX_BUYMF:
        case OFX_BUYOPT:
        case OFX_BUYOTHER:
        case OFX_BUYSTOCK:
            t.m_eAction = eMyMoney::Transaction::Action::Buy;
            break;
        case OFX_REINVEST:
            t.m_eAction = eMyMoney::Transaction::Action::ReinvestDividend;
            break;
        case OFX_SELLDEBT:
        case OFX_SELLMF:
        case OFX_SELLOPT:
        case OFX_SELLOTHER:
        case OFX_SELLSTOCK:
            t.m_eAction = eMyMoney::Transaction::Action::Sell;
            break;
        case OFX_INCOME:
            t.m_eAction = eMyMoney::Transaction::Action::CashDividend;
            // NOTE: With CashDividend, the amount of the dividend should
            // be in data.amount.  Since I've never seen an OFX file with
            // cash dividends, this is an assumption on my part. (acejones)
            break;

#if QT_VERSION_CHECK(LIBOFX_MAJOR_VERSION, LIBOFX_MINOR_VERSION, LIBOFX_MICRO_VERSION) >= QT_VERSION_CHECK(0, 10, 0)
        case OFX_INVBANKTRAN:
            // This is a regular transaction within an investment account
            // so there is nothing special to do here.
            break;
#endif

        //
        // These types are all not handled.  We will generate a warning for them.
        //
        case OFX_CLOSUREOPT:
            unhandledtype = true;
            type = QStringLiteral("CLOSUREOPT (Close a position for an option)");
            break;
        case OFX_INVEXPENSE:
            unhandledtype = true;
            type = QStringLiteral("INVEXPENSE (Misc investment expense that is associated with a specific security)");
            break;
        case OFX_JRNLFUND:
            unhandledtype = true;
            type = QStringLiteral("JRNLFUND (Journaling cash holdings between subaccounts within the same investment account)");
            break;
        case OFX_MARGININTEREST:
            unhandledtype = true;
            type = QStringLiteral("MARGININTEREST (Margin interest expense)");
            break;
        case OFX_RETOFCAP:
            unhandledtype = true;
            type = QStringLiteral("RETOFCAP (Return of capital)");
            break;
        case OFX_SPLIT:
            unhandledtype = true;
            type = QStringLiteral("SPLIT (Stock or mutial fund split)");
            break;
        case OFX_TRANSFER:
            unhandledtype = true;
            type = QStringLiteral("TRANSFER (Transfer holdings in and out of the investment account)");
            break;
        default:
            unhandledtype = true;
            type = QString("UNKNOWN %1").arg(data.invtransactiontype);
            break;
        }
    } else
        t.m_eAction = eMyMoney::Transaction::Action::None;

    t.m_shares = MyMoneyMoney();
    if (data.units_valid) {
        auto shares = MyMoneyMoney(data.units, 1000000000).reduce();
        const auto denominator = MyMoneyMoney(shares.toString().remove(QRegularExpression("^[+-]?[\\d]+/")));
        t.m_shareDenominator = MyMoneyMoney::ONE;
        while (t.m_shareDenominator < denominator) {
            t.m_shareDenominator *= MyMoneyMoney(10, 1);
        }
        t.m_shares = shares;
    }

    t.m_amount = MyMoneyMoney();
    if (data.amount_valid) {
        t.m_amount = MyMoneyMoney(data.amount, 1000);

        if (d->m_fixBuySellSignage) {
            if (t.m_eAction == eMyMoney::Transaction::Action::Buy
                    || t.m_eAction == eMyMoney::Transaction::Action::ReinvestDividend) {
                t.m_amount = -t.m_amount.abs();
                t.m_shares = t.m_shares.abs();
            }
            else if (t.m_eAction == eMyMoney::Transaction::Action::Sell) {
                t.m_amount = t.m_amount.abs();
                t.m_shares = -t.m_shares.abs();
            }
        }

        if (d->m_invertAmount) {
            t.m_amount = -t.m_amount;
        }
    }

    if (data.check_number_valid) {
        t.m_strNumber = d->sanitizedString(data.check_number);
    }

    unsigned long h;
    QString tmpString;
    // in case the unique transaction id source is yet unknown we
    // use the global preset
    UniqueTransactionIdSource idSource = d->m_uniqueIdSource;
    if (idSource == UniqueIdUnknown) {
        idSource = defaultIdSource();
    }
    switch (idSource) {
    default:
    case UniqueIdOfx:
        if (data.fi_id_valid) {
            t.m_strBankID = QStringLiteral("ID ") + d->sanitizedString(data.fi_id);
        } else if (data.reference_number_valid) {
            t.m_strBankID = QStringLiteral("REF ") + d->sanitizedString(data.reference_number);
        }
        break;

    case UniqueIdKMyMoney:
        if (data.payee_id_valid) {
            tmpString = d->sanitizedString(data.payee_id);
        } else if (data.name_valid) {
            tmpString = d->sanitizedString(data.name);
        } else if (data.memo_valid) {
            tmpString = d->sanitizedString(data.memo);
        }
        h = MyMoneyTransaction::hash(tmpString.trimmed());
        if (data.memo_valid)
            h = MyMoneyTransaction::hash(QString::fromUtf8(data.memo), h);

        h = MyMoneyTransaction::hash(t.m_amount.toString(), h);
        // make hash value unique in case we don't have one already

        const auto hashBase(QStringLiteral("%1-%2").arg(t.m_datePosted.toString(Qt::ISODate)).arg(h, 7, 16, QLatin1Char('0')));
        int idx = 1;
        QString hash;
        for (;;) {
            hash = QString("%1-%2").arg(hashBase).arg(idx);
            if (!d->m_hashes.contains(hash)) {
                d->m_hashes += hash;
                break;
            }
            ++idx;
        }
        t.m_strBankID = QString("KMM %1").arg(hash);
        break;
    }

    // Decide whether to use NAME, PAYEEID or MEMO to construct the payee
    bool validity[3] = {false, false, false};
    QStringList values;
    switch (d->m_preferName) {
    case OFXImporter::Private::PreferId:  // PAYEEID
    default:
        validity[0] = data.payee_id_valid;
        validity[1] = data.name_valid;
        validity[2] = data.memo_valid;
        values += d->sanitizedString(data.payee_id);
        values += d->sanitizedString(data.name);
        values += d->sanitizedString(data.memo);
        break;

    case OFXImporter::Private::PreferName:  // NAME
        validity[0] = data.name_valid;
        validity[1] = data.payee_id_valid;
        validity[2] = data.memo_valid;
        values += d->sanitizedString(data.name);
        values += d->sanitizedString(data.payee_id);
        values += d->sanitizedString(data.memo);
        break;

    case OFXImporter::Private::PreferMemo:  // MEMO
        validity[0] = data.memo_valid;
        validity[1] = data.payee_id_valid;
        validity[2] = data.name_valid;
        values += d->sanitizedString(data.memo);
        values += d->sanitizedString(data.payee_id);
        values += d->sanitizedString(data.name);
        break;
    }

    // for investment transactions we don't use the meme as payee
    if (data.invtransactiontype_valid) {
        values.clear();
        validity[0] = data.payee_id_valid;
        validity[1] = data.name_valid;
        validity[2] = false;
        values += d->sanitizedString(data.payee_id);
        values += d->sanitizedString(data.name);
    }

    for (int idx = 0; idx < 3; ++idx) {
        if (validity[idx]) {
            t.m_strPayee = values[idx];
            break;
        }
    }

    // extract memo field if we haven't used it as payee
    if ((data.memo_valid) && (d->m_preferName != OFXImporter::Private::PreferMemo)) {
        t.m_strMemo = d->sanitizedString(data.memo);
    }

    // If the payee or memo fields are blank, set them to
    // the other one which is NOT blank.  (acejones)
    if (t.m_strPayee.isEmpty()) {
        // But we only create a payee for non-investment transactions (ipwizard)
        if (! t.m_strMemo.isEmpty() && data.invtransactiontype_valid == false)
            t.m_strPayee = t.m_strMemo;
    } else {
        if (t.m_strMemo.isEmpty())
            t.m_strMemo = t.m_strPayee;
    }

    if (data.security_data_valid) {
        struct OfxSecurityData* secdata = data.security_data_ptr;

        if (secdata->unique_id_valid) {
            t.m_strSecurityId = d->sanitizedString(secdata->unique_id);
        }
        if (secdata->ticker_valid) {
            t.m_strSymbol = d->sanitizedString(secdata->ticker);
        }

        if (secdata->secname_valid) {
            t.m_strSecurity = d->sanitizedString(secdata->secname);
        }

        // scan over securities and check if security is there
        // and possibly adjust the smallestFraction setting
        for (auto& security : s.m_listSecurities) {
            if ((t.m_strSecurityId == security.m_strId) || (t.m_strSecurity == security.m_strName)) {
                if (t.m_shareDenominator > security.m_smallestFraction) {
                    security.m_smallestFraction = t.m_shareDenominator;
                    break;
                }
            }
        }
    }

    t.m_price = MyMoneyMoney();
    if (data.unitprice_valid) {
        t.m_price = MyMoneyMoney(data.unitprice, 100000).reduce();
    }

    t.m_fees = MyMoneyMoney();
    if (data.fees_valid) {
        t.m_fees += MyMoneyMoney(data.fees, 1000).reduce();
    }

    if (data.commission_valid) {
        t.m_fees += MyMoneyMoney(data.commission, 1000).reduce();
    }

    // In the case of investment transactions, the 'total' is supposed to be the total amount
    // of the transaction.  units * unitprice +/- commission.  Easy, right?  Sadly, it seems
    // some ofx creators do not follow this in all circumstances.  Therefore, we have to double-
    // check the total here and adjust it if it's wrong.

#if 0
    // Even more sadly, this logic is BROKEN.  It consistently results in bogus total
    // values, because of rounding errors in the price.  A more through solution would
    // be to test if the commission alone is causing a discrepancy, and adjust in that case.

    if (data.invtransactiontype_valid && data.unitprice_valid) {
        double proper_total = t.m_dShares * data.unitprice + t.m_moneyFees;
        if (proper_total != t.m_moneyAmount) {
            pofx->addWarning(QString("Transaction %1 has an incorrect total of %2. Using calculated total of %3 instead.").arg(t.m_strBankID).arg(t.m_moneyAmount).arg(proper_total));
            t.m_moneyAmount = proper_total;
        }
    }
#endif

    if (unhandledtype)
        pofx->addWarning(QString("Transaction %1 has an unsupported type (%2).").arg(t.m_strBankID, type));
    else
        s.m_listTransactions += t;

//   kDebug(2) << Q_FUNC_INFO << "return 0 ";

    return 0;
}

int OFXImporter::ofxStatementCallback(struct OfxStatementData data, void* pv)
{
//   kDebug(2) << Q_FUNC_INFO;

    OFXImporter* pofx = reinterpret_cast<OFXImporter*>(pv);
    OFXImporter::Private* d = pofx->d;

    MyMoneyStatement& s = pofx->back();

    pofx->setValid();

    if (data.currency_valid) {
        s.m_strCurrency = d->sanitizedString(data.currency);
    }
    if (data.account_id_valid) {
        // only use the account_id if it is filled with non-blank data
        // see https://bugs.kde.org/show_bug.cgi?id=428156
        const auto account_id = d->sanitizedString(data.account_id).trimmed();
        if (!account_id.isEmpty()) {
            s.m_strAccountNumber = account_id;
        }
    }

    if (data.date_start_valid) {
        QDateTime dt;
        dt.setSecsSinceEpoch(data.date_start - d->m_timestampOffset * 60);
        s.m_dateBegin = dt.date();
    }

    if (data.date_end_valid) {
        QDateTime dt;
        dt.setSecsSinceEpoch(data.date_end - d->m_timestampOffset * 60);
        s.m_dateEnd = dt.date();
    }

    if (data.ledger_balance_valid && data.ledger_balance_date_valid) {
        s.m_closingBalance = MyMoneyMoney(data.ledger_balance);
        QDateTime dt;
        dt.setSecsSinceEpoch(data.ledger_balance_date);
        s.m_dateEnd = dt.date();
    }

//   kDebug(2) << Q_FUNC_INFO << " return 0";

    return 0;
}

int OFXImporter::ofxAccountCallback(struct OfxAccountData data, void * pv)
{
//   kDebug(2) << Q_FUNC_INFO;

    OFXImporter* pofx = reinterpret_cast<OFXImporter*>(pv);
    OFXImporter::Private* d = pofx->d;

    pofx->addnew();
    MyMoneyStatement& s = pofx->back();

    // Having any account at all makes an ofx statement valid
    d->m_valid = true;
    // new account means new hashes
    d->m_hashes.clear();

    if (data.account_id_valid) {
        const auto account_name = d->sanitizedString(data.account_name);
        // in case libofx does not extract any value, it returns
        // one of the following fixed strings in this member.
        // See OfxAccountContainer::gen_account_id in
        // libofx/lib/ofx_container_account.cpp for details
        static const QStringList emptyEntries({
            "Credit card ",
            "Investment account at broker ",
            "Bank account ",
        });
        if (!emptyEntries.contains(account_name)) {
            s.m_strAccountName = account_name.trimmed();
        }

        // only use the account_id if it is filled with non-blank data
        // see https://bugs.kde.org/show_bug.cgi?id=428156
        const auto account_id = d->sanitizedString(data.account_id).trimmed();
        if (!account_id.isEmpty()) {
            s.m_strAccountNumber = account_id;
        }
    }
    if (data.bank_id_valid) {
        s.m_strBankCode = d->sanitizedString(data.bank_id);
    }
    if (data.broker_id_valid) {
        s.m_strBankCode = d->sanitizedString(data.broker_id);
    }
    if (data.currency_valid) {
        s.m_strCurrency = d->sanitizedString(data.currency);
    }

    if (data.account_type_valid) {
        switch (data.account_type) {
        case OfxAccountData::OFX_CHECKING :
            s.m_eType = eMyMoney::Statement::Type::Checkings;
            break;
        case OfxAccountData::OFX_SAVINGS :
            s.m_eType = eMyMoney::Statement::Type::Savings;
            break;
        case OfxAccountData::OFX_MONEYMRKT :
            s.m_eType = eMyMoney::Statement::Type::Investment;
            break;
        case OfxAccountData::OFX_CREDITLINE :
            s.m_eType = eMyMoney::Statement::Type::CreditCard;
            break;
        case OfxAccountData::OFX_CMA :
            s.m_eType = eMyMoney::Statement::Type::CreditCard;
            break;
        case OfxAccountData::OFX_CREDITCARD :
            s.m_eType = eMyMoney::Statement::Type::CreditCard;
            break;
        case OfxAccountData::OFX_INVESTMENT :
            s.m_eType = eMyMoney::Statement::Type::Investment;
            break;
        case OfxAccountData::OFX_401K:
            s.m_eType = eMyMoney::Statement::Type::Investment;
            break;
        }
    }

    // ask KMyMoney for an account id
    // but only if we have any information to ask for
    if (!s.m_strAccountNumber.isEmpty() || !s.m_strBankCode.isEmpty()) {
        s.m_accountId = pofx->account(QStringLiteral("kmmofx-acc-ref"), QString("%1-%2").arg(s.m_strBankCode, s.m_strAccountNumber)).id();
    }

    // copy over the securities
    s.m_listSecurities = d->m_securitylist;

    //   kDebug(2) << Q_FUNC_INFO << " return 0";

    return 0;
}

int OFXImporter::ofxSecurityCallback(struct OfxSecurityData data, void* pv)
{
    //   kDebug(2) << Q_FUNC_INFO;

    OFXImporter* pofx = reinterpret_cast<OFXImporter*>(pv);
    OFXImporter::Private* d = pofx->d;
    MyMoneyStatement::Security sec;

    if (data.unique_id_valid) {
        sec.m_strId = d->sanitizedString(data.unique_id);
    }
    if (data.secname_valid) {
        sec.m_strName = d->sanitizedString(data.secname);
    }
    if (data.ticker_valid) {
        sec.m_strSymbol = d->sanitizedString(data.ticker);
    }

    d->m_securitylist += sec;

    return 0;
}

int OFXImporter::ofxStatusCallback(struct OfxStatusData data, void * pv)
{
//   kDebug(2) << Q_FUNC_INFO;

    OFXImporter* pofx = reinterpret_cast<OFXImporter*>(pv);
    OFXImporter::Private* d = pofx->d;
    QString message;

    // if we got this far, we know we were able to parse the file.
    // so if it fails after here it can only because there were no actual
    // accounts in the file!
    pofx->d->m_fatalerror = i18n("No accounts found.");

    if (data.ofx_element_name_valid)
        message.prepend(QString("%1: ").arg(d->sanitizedString(data.ofx_element_name)));

    if (data.code_valid)
        message += QString("%1 (Code %2): %3").arg(d->sanitizedString(data.name)).arg(data.code).arg(d->sanitizedString(data.description));

    if (data.server_message_valid)
        message += QString(" (%1)").arg(d->sanitizedString(data.server_message));

    if (data.severity_valid) {
        switch (data.severity) {
        case OfxStatusData::INFO:
            pofx->addInfo(message);
            break;
        case OfxStatusData::ERROR:
            pofx->addError(message);
            break;
        case OfxStatusData::WARN:
            pofx->addWarning(message);
            break;
        default:
            pofx->addWarning(message);
            pofx->addWarning(QStringLiteral("Previous message was an unknown type.  'WARNING' was assumed."));
            break;
        }
    }

//   kDebug(2) << Q_FUNC_INFO << " return 0 ";

    return 0;
}

QStringList OFXImporter::importStatement(const MyMoneyStatement &s)
{
    qDebug("OfxImporterPlugin::importStatement start");
    return statementInterface()->import(s, false);
}

MyMoneyAccount OFXImporter::account(const QString& key, const QString& value) const
{
    return statementInterface()->account(key, value);
}

void OFXImporter::protocols(QStringList& protocolList) const
{
    protocolList.clear();
    protocolList << QStringLiteral("OFX");
}

QWidget* OFXImporter::accountConfigTab(const MyMoneyAccount& acc, QString& name)
{
    name = i18n("Online settings");
    d->m_statusDlg = new KOnlineBankingStatus(acc, 0);
    return d->m_statusDlg;
}

MyMoneyKeyValueContainer OFXImporter::onlineBankingSettings(const MyMoneyKeyValueContainer& current)
{
    MyMoneyKeyValueContainer kvp(current);
    // keep the provider name in sync with the one found in kmm_ofximport.desktop
    kvp[QStringLiteral("provider")] = objectName().toLower();
    if (d->m_statusDlg) {
        kvp.deletePair(QStringLiteral("appId"));
        kvp.deletePair(QStringLiteral("kmmofx-headerVersion"));
        kvp.deletePair(QStringLiteral("password"));

        const QString key = OFX_PASSWORD_KEY(kvp.value(QStringLiteral("url")), kvp.value(QStringLiteral("uniqueId")));
        auto keyChain = new KMMKeychain();
        if (d->m_statusDlg->m_storePassword->isChecked()) {
            keyChain->writeKey(key, d->m_statusDlg->m_password->password());
        } else {
            keyChain->deleteKey(key);
        }

        if (!d->m_statusDlg->appId().isEmpty())
            kvp.setValue(QStringLiteral("appId"), d->m_statusDlg->appId());
        kvp.setValue(QStringLiteral("kmmofx-headerVersion"), d->m_statusDlg->headerVersion());
        kvp.setValue(QStringLiteral("kmmofx-numRequestDays"), QString::number(d->m_statusDlg->m_numdaysSpin->value()));
        kvp.setValue(QStringLiteral("kmmofx-todayMinus"), QString::number(d->m_statusDlg->m_todayRB->isChecked()));
        kvp.setValue(QStringLiteral("kmmofx-lastUpdate"), QString::number(d->m_statusDlg->m_lastUpdateRB->isChecked()));
        kvp.setValue(QStringLiteral("kmmofx-pickDate"), QString::number(d->m_statusDlg->m_pickDateRB->isChecked()));
        kvp.setValue(QStringLiteral("kmmofx-specificDate"), d->m_statusDlg->m_specificDate->date().toString());
        kvp.setValue(QStringLiteral("kmmofx-preferName"), QString::number(d->m_statusDlg->m_preferredPayee->currentIndex()));
        kvp.setValue(QStringLiteral("kmmofx-uniqueIdSource"), QString::number(d->m_statusDlg->m_uniqueTransactionId->currentIndex()));
        if (!d->m_statusDlg->m_clientUidEdit->text().isEmpty())
            kvp.setValue(QStringLiteral("clientUid"), d->m_statusDlg->m_clientUidEdit->text());
        else
            kvp.deletePair(QStringLiteral("clientUid"));
        int offset = d->constructTimeOffset(d->m_statusDlg->m_timestampOffset, d->m_statusDlg->m_timestampOffsetSign);
        if (offset == 0) {
            kvp.deletePair(QStringLiteral("kmmofx-timestampOffset"));
        } else {
            kvp.setValue(QStringLiteral("kmmofx-timestampOffset"), QString::number(offset));
        }
        if (d->m_statusDlg->m_invertAmount->isChecked()) {
            kvp.setValue(QStringLiteral("kmmofx-invertamount"), QStringLiteral("yes"));
        } else {
            kvp.deletePair(QStringLiteral("kmmofx-invertamount"));
        }
        if (d->m_statusDlg->m_fixBuySellSignage->isChecked()) {
            kvp.setValue(QStringLiteral("kmmofx-fixbuysellsignage"), QStringLiteral("yes"));
        } else {
            kvp.deletePair(QStringLiteral("kmmofx-fixbuysellsignage"));
        }
        if (!d->m_statusDlg->m_userAgentEdit->text().isEmpty())
            kvp.setValue(QStringLiteral("kmmofx-useragent"), d->m_statusDlg->m_userAgentEdit->text());
        else
            kvp.deletePair(QStringLiteral("kmmofx-useragent"));
        // get rid of pre 4.6 values
        kvp.deletePair(QStringLiteral("kmmofx-preferPayeeid"));
    }
    return kvp;
}

bool OFXImporter::mapAccount(const MyMoneyAccount& acc, MyMoneyKeyValueContainer& settings)
{
    Q_UNUSED(acc);

    bool rc = false;
    QPointer<KOnlineBankingSetupWizard> wiz = new KOnlineBankingSetupWizard(0);
    if (wiz->isInit()) {
        if (wiz->exec() == QDialog::Accepted) {
            rc = wiz->chosenSettings(settings);
        }
    }

    delete wiz;

    return rc;
}

bool OFXImporter::updateAccount(const MyMoneyAccount& acc, bool moreAccounts)
{
    Q_UNUSED(moreAccounts);

    qDebug("OfxImporterPlugin::updateAccount");
    try {
        d->m_uniqueIdSource = UniqueIdUnknown;
        if (!acc.id().isEmpty()) {
            // Save the value of preferName to be used by ofxTransactionCallback
            d->m_preferName = static_cast<OFXImporter::Private::NamePreference>(acc.onlineBankingSettings().value(QStringLiteral("kmmofx-preferName")).toInt());
            if (acc.onlineBankingSettings().value(QStringLiteral("kmmofx-uniqueIdSource")).isEmpty())
                d->m_uniqueIdSource = defaultIdSource();
            else
                d->m_uniqueIdSource = static_cast<UniqueTransactionIdSource>(acc.onlineBankingSettings().value(QStringLiteral("kmmofx-uniqueIdSource")).toInt());
            QPointer<KOfxDirectConnectDlg> dlg = new KOfxDirectConnectDlg(acc);

            connect(dlg.data(), &KOfxDirectConnectDlg::statementReady, this, static_cast<void (OFXImporter::*)(const QString &)>(&OFXImporter::slotImportFile));

            // get the date of the earliest transaction that we are interested in
            // as well as other parameters from the settings for this account
            MyMoneyKeyValueContainer settings = acc.onlineBankingSettings();
            if (!settings.value(QStringLiteral("provider")).isEmpty()) {
                if ((settings.value(QStringLiteral("kmmofx-todayMinus")).toInt() != 0) && !settings.value(QStringLiteral("kmmofx-numRequestDays")).isEmpty()) {
                    //kDebug(0) << "start date = today minus";
                    d->m_updateStartDate = QDate::currentDate().addDays(-settings.value(QStringLiteral("kmmofx-numRequestDays")).toInt());
                } else if ((settings.value(QStringLiteral("kmmofx-lastUpdate")).toInt() != 0) && !acc.value(QStringLiteral("lastImportedTransactionDate")).isEmpty()) {
                    //kDebug(0) << "start date = last update";
                    d->m_updateStartDate = QDate::fromString(acc.value(QStringLiteral("lastImportedTransactionDate")), Qt::ISODate);
                } else if ((settings.value(QStringLiteral("kmmofx-pickDate")).toInt() != 0) && !settings.value(QStringLiteral("kmmofx-specificDate")).isEmpty()) {
                    //kDebug(0) << "start date = pick date";
                    d->m_updateStartDate = QDate::fromString(settings.value(QStringLiteral("kmmofx-specificDate")));
                }
                else {
                    //kDebug(0) << "start date = today - 2 months";
                    d->m_updateStartDate = QDate::currentDate().addMonths(-2);
                }

                d->m_invertAmount = settings.value("kmmofx-invertamount").toLower() == QStringLiteral("yes");
                d->m_fixBuySellSignage= settings.value("kmmofx-fixbuysellsignage").toLower() == QStringLiteral("yes");
            }
            d->m_timestampOffset = settings.value("kmmofx-timestampOffset").toInt();
            //kDebug(0) << "ofx plugin: account" << acc.name() << "earliest transaction date to process =" << qPrintable(d->m_updateStartDate.toString(Qt::ISODate));

            if (dlg->init())
                dlg->exec();
            delete dlg;

            // reset the earliest-interesting-transaction date to the non-specific account setting
            d->m_updateStartDate = QDate(1900,1,1);
            d->m_timestampOffset = 0;
        }
    } catch (const MyMoneyException &e) {
        KMessageBox::information(0, i18n("Error connecting to bank: %1", QString::fromLatin1(e.what())));
    }

    return false;
}

void OFXImporter::slotImportFile(const QString& url)
{
    qDebug() << "OfxImporterPlugin::slotImportFile";
    if (!import(url)) {
        KMessageBox::error(0, QString("<qt>%1</qt>").arg(i18n("<p>Unable to import <b>'%1'</b> using the OFX importer plugin.  The plugin returned the following error:</p><p>%2</p>", url, lastError())), i18n("Importing error"));
    }
}

bool OFXImporter::storeStatements(const QList<MyMoneyStatement> &statements)
{
    if (statements.isEmpty())
        return true;
    auto ok = true;
    auto abort = false;

    // FIXME Deal with warnings/errors coming back from plugins
    /*if ( ofx.errors().count() )
    {
      if ( KMessageBox::warningContinueCancelList(this,i18n("The following errors were returned from your bank"),ofx.errors(),i18n("OFX Errors")) == KMessageBox::Cancel )
        abort = true;
    }

    if ( ofx.warnings().count() )
    {
      if ( KMessageBox::warningContinueCancelList(this,i18n("The following warnings were returned from your bank"),ofx.warnings(),i18n("OFX Warnings"),KStandardGuiItem::cont(),"ofxwarnings") == KMessageBox::Cancel )
        abort = true;
    }*/

    qDebug("OfxImporterPlugin::storeStatements() with %d statements called", statements.count());

    for (const auto& statement : statements) {
        if (abort)
            break;
        if (importStatement(statement).isEmpty())
            ok = false;
    }

    if (!ok)
        KMessageBox::error(nullptr, i18n("Importing process terminated unexpectedly."), i18n("Failed to import all statements."));

    return ok;
}

void OFXImporter::addnew()
{
    d->m_statementlist.push_back(MyMoneyStatement());
}
MyMoneyStatement& OFXImporter::back()
{
    return d->m_statementlist.back();
}
bool OFXImporter::isValid() const
{
    return d->m_valid;
}
void OFXImporter::setValid()
{
    d->m_valid = true;
}
void OFXImporter::addInfo(const QString& _msg)
{
    d->m_infos += _msg;
}
void OFXImporter::addWarning(const QString& _msg)
{
    d->m_warnings += _msg;
}
void OFXImporter::addError(const QString& _msg)
{
    d->m_errors += _msg;
}
const QStringList& OFXImporter::infos() const          // krazy:exclude=spelling
{
    return d->m_infos;
}
const QStringList& OFXImporter::warnings() const
{
    return d->m_warnings;
}
const QStringList& OFXImporter::errors() const
{
    return d->m_errors;
}

K_PLUGIN_CLASS_WITH_JSON(OFXImporter, "ofximporter.json")

#include "ofximporter.moc"
