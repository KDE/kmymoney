/*
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2020 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "icons.h"

#include <QDir>
#include <QHash>
#include <QIcon>
#include <QPainter>
#include <QPixmapCache>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QString>

namespace Icons {
QHash<Icon, QString> sStandardIcons;

uint qHash(const Icon key, uint seed)
{
    return ::qHash(static_cast<uint>(key), seed);
}

uint qHash(const IconSet key, uint seed)
{
    return ::qHash(static_cast<uint>(key), seed);
}

struct iconDescription {
    Icon baseIcon;
    Icon overlayIcon;
    Qt::Corner corner;
};

const QHash<Icon, QHash<IconSet, QString>> iconMappings{
    {Icon::AccountClosed,
     {{IconSet::Common, QStringLiteral("account-types-closed")},
      {IconSet::Oxygen, QStringLiteral("dialog-close")},
      {IconSet::Breeze, QStringLiteral("dialog-close")}}},
    {Icon::ArrowDown, {{IconSet::Common, QStringLiteral("arrow-down")}, {IconSet::Tango, QStringLiteral("go-down")}}},
    {Icon::ArrowLeft, {{IconSet::Common, QStringLiteral("arrow-left")}, {IconSet::Tango, QStringLiteral("go-previous")}}},
    {Icon::ArrowRight, {{IconSet::Common, QStringLiteral("arrow-right")}, {IconSet::Tango, QStringLiteral("go-next")}}},
    {Icon::ArrowUp, {{IconSet::Common, QStringLiteral("arrow-up")}, {IconSet::Tango, QStringLiteral("go-up")}}},
    {Icon::Budget,
     {{IconSet::Common, QStringLiteral("budget")},
      {IconSet::Oxygen, QStringLiteral("view-time-schedule-calculus")},
      {IconSet::Breeze, QStringLiteral("financial-budget")}}},
    {Icon::Calculator, {{IconSet::Common, QStringLiteral("accessories-calculator")}}},
    {Icon::Configure, {{IconSet::Common, QStringLiteral("configure")}, {IconSet::Tango, QStringLiteral("preferences-system")}}},
    {Icon::DialogCancel, {{IconSet::Common, QStringLiteral("dialog-cancel")}, {IconSet::Tango, QStringLiteral("stop")}}},
    {Icon::DialogClose, {{IconSet::Common, QStringLiteral("dialog-close")}}},
    {Icon::DialogError, {{IconSet::Common, QStringLiteral("dialog-error")}}},
    {Icon::DialogInformation, {{IconSet::Common, QStringLiteral("dialog-information")}}},
    {Icon::DialogOK, {{IconSet::Common, QStringLiteral("dialog-ok")}, {IconSet::Tango, QStringLiteral("finish")}}},
    {Icon::DialogOKApply, {{IconSet::Common, QStringLiteral("dialog-ok-apply")}}},
    {Icon::DialogWarning, {{IconSet::Common, QStringLiteral("dialog-warning")}}},
    {Icon::DocumentClose, {{IconSet::Common, QStringLiteral("document-close")}, {IconSet::Tango, QStringLiteral("stop")}}},
    {Icon::DocumentEdit, {{IconSet::Common, QStringLiteral("document-edit")}, {IconSet::Tango, QStringLiteral("text-editor")}}},
    {Icon::DocumentExport,
     {{IconSet::Common, QStringLiteral("format-indent-more")},
      {IconSet::Oxygen, QStringLiteral("document-export")},
      {IconSet::Breeze, QStringLiteral("document-export")}}},
    {Icon::DocumentImport,
     {{IconSet::Common, QStringLiteral("format-indent-less")},
      {IconSet::Oxygen, QStringLiteral("document-import")},
      {IconSet::Breeze, QStringLiteral("document-import")}}},
    {Icon::DocumentNew, {{IconSet::Common, QStringLiteral("document-new")}}},
    {Icon::DocumentOpen, {{IconSet::Common, QStringLiteral("document-open")}}},
    {Icon::DocumentProperties, {{IconSet::Common, QStringLiteral("document-properties")}}},
    {Icon::DocumentSave, {{IconSet::Common, QStringLiteral("document-save")}}},
    {Icon::Download,
     {{IconSet::Breeze, QStringLiteral("edit-download")}, {IconSet::Common, QStringLiteral("go-down")}, {IconSet::Oxygen, QStringLiteral("download")}}},
    {Icon::EditClear, {{IconSet::Common, QStringLiteral("edit-clear")}}},
    {Icon::EditCopy, {{IconSet::Common, QStringLiteral("edit-copy")}}},
    {Icon::EditDelete, {{IconSet::Common, QStringLiteral("edit-delete")}}},
    {Icon::Find, {{IconSet::Common, QStringLiteral("edit-find")}}},
    {Icon::EditRename, {{IconSet::Common, QStringLiteral("edit-rename")}, {IconSet::Tango, QStringLiteral("text-editor")}}},
    {Icon::EditUndo, {{IconSet::Common, QStringLiteral("edit-undo")}}},
    {Icon::Folder, {{IconSet::Common, QStringLiteral("folder")}}},
    {Icon::GoTo, {{IconSet::Common, QStringLiteral("go-jump")}}},
    {Icon::Help, {{IconSet::Common, QStringLiteral("help-contents")}}},
    {Icon::HideCategories, {{IconSet::Common, QStringLiteral("hide-categories")}}},
    {Icon::HideReconciled, {{IconSet::Common, QStringLiteral("hide-reconciled")}}},
    {Icon::ShowReconciledBalances, {{IconSet::Breeze, QStringLiteral("gnumeric-autosum")}}},
    {Icon::KMyMoney, {{IconSet::Common, QStringLiteral("kmymoney")}}},
    {Icon::KeyEnter,
     {{IconSet::Common, QStringLiteral("input-keyboard")}, {IconSet::Oxygen, QStringLiteral("key-enter")}, {IconSet::Breeze, QStringLiteral("key-enter")}}},
    {Icon::ListAdd, {{IconSet::Common, QStringLiteral("list-add")}}},
    {Icon::ListAddTag, {{IconSet::Common, QStringLiteral("list-add-tag")}}},
    {Icon::ListAddUser, {{IconSet::Common, QStringLiteral("list-add-user")}}},
    {Icon::ListCollapse, {{IconSet::Common, QStringLiteral("zoom-out")}, {IconSet::Tango, QStringLiteral("list-remove")}}},
    {Icon::ListExpand, {{IconSet::Common, QStringLiteral("zoom-in")}, {IconSet::Tango, QStringLiteral("list-add")}}},
    {Icon::ListRemoveTag, {{IconSet::Common, QStringLiteral("list-remove-tag")}}},
    {Icon::ListRemoveUser, {{IconSet::Common, QStringLiteral("list-remove-user")}}},
    {Icon::MailMessage,
     {{IconSet::Common, QStringLiteral("internet-mail")},
      {IconSet::Oxygen, QStringLiteral("mail-message")},
      {IconSet::Breeze, QStringLiteral("mail-message")}}},
    {Icon::MailMessageNew, {{IconSet::Common, QStringLiteral("mail-message-new")}}},
    {Icon::MailReceive, {{IconSet::Common, QStringLiteral("mail-receive")}}},
    {Icon::MapOnlineAccount, {{IconSet::Common, QStringLiteral("news-subscribe")}}},
    {Icon::Merge, {{IconSet::Common, QStringLiteral("reconcile")}, {IconSet::Oxygen, QStringLiteral("merge")}, {IconSet::Breeze, QStringLiteral("merge")}}},
    {Icon::NewSchedule, {{IconSet::Common, QStringLiteral("appointment-new")}}},
    {Icon::OfficeChartLine,
     {{IconSet::Common, QStringLiteral("account-types-investments")},
      {IconSet::Oxygen, QStringLiteral("office-chart-line")},
      {IconSet::Breeze, QStringLiteral("office-chart-line")},
      {IconSet::Tango, QStringLiteral("report-line")}}},
    {Icon::OpenDatabase, {{IconSet::Common, QStringLiteral("server-database")}}},
    {Icon::Pause, {{IconSet::Common, QStringLiteral("media-playback-pause")}}},
    {Icon::PayeeMerge, {{IconSet::Common, QStringLiteral("merge")}}},
    {Icon::PayeeRename, {{IconSet::Common, QStringLiteral("user-properties")}, {IconSet::Oxygen, QStringLiteral("payee-rename")}}},
    {Icon::PerformanceTest, {{IconSet::Common, QStringLiteral("fork")}, {IconSet::Breeze, QStringLiteral("speedometer")}}},
    {Icon::PreferencesColors, {{IconSet::Common, QStringLiteral("preferences-desktop-color")}}},
    {Icon::PreferencesFonts, {{IconSet::Common, QStringLiteral("preferences-desktop-font")}}},
    {Icon::PreferencesGeneral, {{IconSet::Common, QStringLiteral("system-run")}, {IconSet::Tango, QStringLiteral("media-playback-start")}}},
    {Icon::PreferencesIcons, {{IconSet::Common, QStringLiteral("preferences-desktop-icon")}}},
    {Icon::PreferencesNetwork, {{IconSet::Common, QStringLiteral("preferences-system-network")}}},
    {Icon::PreferencesPlugins, {{IconSet::Common, QStringLiteral("network-disconnect")}}},
    {Icon::Reconcile, {{IconSet::Common, QStringLiteral("reconcile")}, {IconSet::Oxygen, QStringLiteral("merge")}, {IconSet::Breeze, QStringLiteral("merge")}}},
    {Icon::Reconciled,
     {{IconSet::Common, QStringLiteral("reconciled")}, {IconSet::Oxygen, QStringLiteral("flag-green")}, {IconSet::Breeze, QStringLiteral("flag-green")}}},
    {Icon::Refresh, {{IconSet::Breeze, QStringLiteral("view-refresh")}, {IconSet::Oxygen, QStringLiteral("refresh")}}},
    {Icon::Report, {{IconSet::Common, QStringLiteral("application-vnd.oasis.opendocument.spreadsheet")}}},
    {Icon::Reverse, {{IconSet::Common, QStringLiteral("reverse")}}},
    {Icon::SeekForward, {{IconSet::Common, QStringLiteral("media-seek-forward")}}},
    {Icon::SkipForward, {{IconSet::Common, QStringLiteral("media-skip-forward")}}},
    {Icon::SortAscending,
     {{IconSet::Common, QStringLiteral("go-up")},
      {IconSet::Oxygen, QStringLiteral("view-sort-ascending")},
      {IconSet::Breeze, QStringLiteral("view-sort-ascending")}}},
    {Icon::SortDescending,
     {{IconSet::Common, QStringLiteral("go-down")},
      {IconSet::Oxygen, QStringLiteral("view-sort-descending")},
      {IconSet::Breeze, QStringLiteral("view-sort-descending")}}},
    {Icon::Split,
     {{IconSet::Common, QStringLiteral("transaction-split")}, {IconSet::Oxygen, QStringLiteral("split")}, {IconSet::Breeze, QStringLiteral("split")}}},
    {Icon::TagRename, {{IconSet::Common, QStringLiteral("tag-rename")}}},
    {Icon::TaskAccepted, {{IconSet::Common, QStringLiteral("task-accepted")}}},
    {Icon::TaskComplete, {{IconSet::Common, QStringLiteral("task-complete")}}},
    {Icon::TaskOngoing, {{IconSet::Common, QStringLiteral("task-ongoing")}}},
    {Icon::TaskReject, {{IconSet::Common, QStringLiteral("task-reject")}}},
    {Icon::Warning, {{IconSet::Common, QStringLiteral("dialog-warning")}}},
    {Icon::Tip, {{IconSet::Common, QStringLiteral("info")}, {IconSet::Oxygen, QStringLiteral("ktip")}}},
    {Icon::Unknown, {{IconSet::Common, QStringLiteral("unknown")}}},
    {Icon::UnmapOnlineAccount, {{IconSet::Common, QStringLiteral("news-unsubscribe")}}},
    {Icon::UserProperties,
     {{IconSet::Common, QStringLiteral("system-users")},
      {IconSet::Oxygen, QStringLiteral("user-properties")},
      {IconSet::Breeze, QStringLiteral("user-properties")}}},
    {Icon::Accounts,
     {{IconSet::Common, QStringLiteral("account")},
      {IconSet::Oxygen, QStringLiteral("view-bank-account")},
      {IconSet::Breeze, QStringLiteral("financial-account")}}},
    {Icon::Asset,
     {{IconSet::Common, QStringLiteral("account-types-asset")},
      {IconSet::Oxygen, QStringLiteral("view-bank-account")},
      {IconSet::Breeze, QStringLiteral("view-financial-account-asset")}}},
    {Icon::Bank,
     {{IconSet::Common, QStringLiteral("bank")}, {IconSet::Oxygen, QStringLiteral("view-bank")}, {IconSet::Breeze, QStringLiteral("view-institution")}}},
    {Icon::BankAccount,
     {{IconSet::Common, QStringLiteral("account")},
      {IconSet::Oxygen, QStringLiteral("view-bank-account")},
      {IconSet::Breeze, QStringLiteral("view-financial-account")}}},
    {Icon::AccountNew,
     {{IconSet::Common, QStringLiteral("account")},
      {IconSet::Oxygen, QStringLiteral("view-bank-account")},
      {IconSet::Breeze, QStringLiteral("view-financial-account-add")}}},
    {Icon::AccountEdit,
     {{IconSet::Common, QStringLiteral("account")},
      {IconSet::Oxygen, QStringLiteral("view-bank-account")},
      {IconSet::Breeze, QStringLiteral("view-financial-account-edit")}}},
    {Icon::AccountDelete,
     {{IconSet::Common, QStringLiteral("account")},
      {IconSet::Oxygen, QStringLiteral("view-bank-account")},
      {IconSet::Breeze, QStringLiteral("view-financial-account-delete")}}},
    {Icon::Calendar, {{IconSet::Common, QStringLiteral("view-calendar")}}},
    {Icon::CalendarDay,
     {{IconSet::Common, QStringLiteral("office-calendar")},
      {IconSet::Oxygen, QStringLiteral("view-calendar-day")},
      {IconSet::Breeze, QStringLiteral("view-calendar-day")}}},
    {Icon::Cash, {{IconSet::Common, QStringLiteral("account-types-cash")}, {IconSet::Breeze, QStringLiteral("view-financial-account-cash")}}},
    {Icon::Checking,
     {{IconSet::Common, QStringLiteral("account-types-checking")},
      {IconSet::Oxygen, QStringLiteral("view-bank-account-checking")},
      {IconSet::Breeze, QStringLiteral("view-financial-account-checking")}}},
    {Icon::Close, {{IconSet::Common, QStringLiteral("view-close")}}},
    {Icon::CreditCard,
     {{IconSet::Breeze, QStringLiteral("skrooge_credit_card")},
      {IconSet::Common, QStringLiteral("account-types-credit-card")},
      {IconSet::Oxygen, QStringLiteral("view-financial-account-credit-card")}}},
    {Icon::Currencies, {{IconSet::Common, QStringLiteral("view-currency-list")}}},
    {Icon::Equity,
     {{IconSet::Common, QStringLiteral("account")},
      {IconSet::Oxygen, QStringLiteral("view-bank-account")},
      {IconSet::Breeze, QStringLiteral("view-financial-account")}}},
    {Icon::Expense,
     {{IconSet::Breeze, QStringLiteral("view-financial-category-expense")},
      {IconSet::Common, QStringLiteral("account-types-expense")},
      {IconSet::Oxygen, QStringLiteral("view-expenses-categories")}}},
    {Icon::Filter, {{IconSet::Common, QStringLiteral("view-filter")}}},
    {Icon::FinancialCategories,
     {{IconSet::Common, QStringLiteral("categories")},
      {IconSet::Oxygen, QStringLiteral("view-categories")},
      {IconSet::Breeze, QStringLiteral("financial-categories")}}},
    {Icon::CategoryNew,
     {{IconSet::Common, QStringLiteral("categories")},
      {IconSet::Oxygen, QStringLiteral("view-categories")},
      {IconSet::Breeze, QStringLiteral("financial-category-add")}}},
    {Icon::CategoryEdit,
     {{IconSet::Common, QStringLiteral("categories")},
      {IconSet::Oxygen, QStringLiteral("view-categories")},
      {IconSet::Breeze, QStringLiteral("financial-category-edit")}}},
    {Icon::CategoryDelete,
     {{IconSet::Common, QStringLiteral("categories")},
      {IconSet::Oxygen, QStringLiteral("view-categories")},
      {IconSet::Breeze, QStringLiteral("financial-category-delete")}}},
    {Icon::Ledger,
     {{IconSet::Common, QStringLiteral("ledger")},
      {IconSet::Oxygen, QStringLiteral("view-financial-list")},
      {IconSet::Breeze, QStringLiteral("financial-list")}}},
    {Icon::Transaction,
     {{IconSet::Common, QStringLiteral("ledger")},
      {IconSet::Oxygen, QStringLiteral("view-financial-transfer")},
      {IconSet::Breeze, QStringLiteral("view-financial-transfer")}}},
    {Icon::Forecast,
     {{IconSet::Common, QStringLiteral("forecast")},
      {IconSet::Oxygen, QStringLiteral("view-financial-forecast")},
      {IconSet::Breeze, QStringLiteral("financial-forecast")}}},
    {Icon::Home, {{IconSet::Common, QStringLiteral("home")}, {IconSet::Oxygen, QStringLiteral("go-home")}, {IconSet::Breeze, QStringLiteral("home")}}},
    {Icon::Income,
     {{IconSet::Breeze, QStringLiteral("view-financial-category-income")},
      {IconSet::Common, QStringLiteral("account-types-income")},
      {IconSet::Oxygen, QStringLiteral("view-income-categories")}}},
    {Icon::Institution,
     {{IconSet::Common, QStringLiteral("institution")}, {IconSet::Oxygen, QStringLiteral("view-bank")}, {IconSet::Breeze, QStringLiteral("view-institution")}}},
    {Icon::InstitutionNew,
     {{IconSet::Common, QStringLiteral("institution")},
      {IconSet::Oxygen, QStringLiteral("view-bank")},
      {IconSet::Breeze, QStringLiteral("view-institution-add")}}},
    {Icon::InstitutionEdit,
     {{IconSet::Common, QStringLiteral("institution")},
      {IconSet::Oxygen, QStringLiteral("view-bank")},
      {IconSet::Breeze, QStringLiteral("view-institution-edit")}}},
    {Icon::InstitutionDelete,
     {{IconSet::Common, QStringLiteral("institution")},
      {IconSet::Oxygen, QStringLiteral("view-bank")},
      {IconSet::Breeze, QStringLiteral("view-institution-delete")}}},
    {Icon::Institutions,
     {{IconSet::Common, QStringLiteral("institution")}, {IconSet::Oxygen, QStringLiteral("view-bank")}, {IconSet::Breeze, QStringLiteral("institution")}}},
    {Icon::Investment,
     {{IconSet::Common, QStringLiteral("investment")},
      {IconSet::Oxygen, QStringLiteral("view-investment")},
      {IconSet::Breeze, QStringLiteral("view-financial-account-investment")}}},
    {Icon::Investments,
     {{IconSet::Common, QStringLiteral("investment")},
      {IconSet::Oxygen, QStringLiteral("view-investment")},
      {IconSet::Breeze, QStringLiteral("financial-investments")}}},
    {Icon::Liability,
     {{IconSet::Common, QStringLiteral("account-types-liability")},
      {IconSet::Oxygen, QStringLiteral("view-loan")},
      {IconSet::Breeze, QStringLiteral("view-financial-account-liability")}}},
    {Icon::Loan,
     {{IconSet::Common, QStringLiteral("account-types-loan")},
      {IconSet::Oxygen, QStringLiteral("view-loan")},
      {IconSet::Breeze, QStringLiteral("view-financial-account-loan")}}},
    {Icon::LoanAsset,
     {{IconSet::Common, QStringLiteral("account-types-loan")},
      {IconSet::Oxygen, QStringLiteral("view-loan-asset")},
      {IconSet::Breeze, QStringLiteral("view-financial-account-loan")}}},
    {Icon::OnlineJobOutbox, {{IconSet::Common, QStringLiteral("online-banking")}, {IconSet::Breeze, QStringLiteral("internet-mail")}}},
    {Icon::Payees,
     {{IconSet::Common, QStringLiteral("payee")}, {IconSet::Oxygen, QStringLiteral("system-users")}, {IconSet::Breeze, QStringLiteral("financial-payees")}}},
    {Icon::Reports,
     {{IconSet::Common, QStringLiteral("report")},
      {IconSet::Oxygen, QStringLiteral("office-chart-bar")},
      {IconSet::Breeze, QStringLiteral("financial-report")}}},
    {Icon::Savings,
     {{IconSet::Common, QStringLiteral("account-types-savings")},
      {IconSet::Oxygen, QStringLiteral("view-bank-account-savings")},
      {IconSet::Breeze, QStringLiteral("view-financial-account-savings")}}},
    {Icon::Schedule,
     {{IconSet::Common, QStringLiteral("schedule")},
      {IconSet::Oxygen, QStringLiteral("view-pim-calendar")},
      {IconSet::Breeze, QStringLiteral("financial-schedule")}}},
    {Icon::Stock,
     {{IconSet::Common, QStringLiteral("account-types-investments")},
      {IconSet::Oxygen, QStringLiteral("view-stock-account")},
      {IconSet::Breeze, QStringLiteral("view-financial-account-investment-security")}}},
    {Icon::Tags,
     {{IconSet::Common, QStringLiteral("bookmark-new")}, {IconSet::Oxygen, QStringLiteral("mail-tagged")}, {IconSet::Breeze, QStringLiteral("tag")}}},
    {Icon::TransactionDetails,
     {{IconSet::Common, QStringLiteral("edit-find")}, {IconSet::Oxygen, QStringLiteral("zoom-in")}, {IconSet::Breeze, QStringLiteral("zoom-in")}}},
    {Icon::UpcomingEvents, {{IconSet::Common, QStringLiteral("view-calendar-upcoming-events")}}},
    {Icon::ZoomIn, {{IconSet::Common, QStringLiteral("zoom-in")}}},
    {Icon::ZoomOut, {{IconSet::Common, QStringLiteral("zoom-out")}}},
    {Icon::Visibility, {{IconSet::Common, QStringLiteral("visibility")}}},
    {Icon::NoVisibility, {{IconSet::Common, QStringLiteral("hint")}}},
    {Icon::SelectAll, {{IconSet::Common, QStringLiteral("edit-select-all")}}}};

const QHash<Icon, iconDescription> sComposedIcons{

    {Icon::AccountFinishReconciliation, {Icon::Merge, Icon::DialogOK, Qt::BottomRightCorner}},
    {Icon::AccountClose, {Icon::BankAccount, Icon::DialogClose, Qt::BottomRightCorner}},
    {Icon::AccountReopen, {Icon::BankAccount, Icon::DialogOK, Qt::BottomRightCorner}},
    {Icon::AccountUpdate, {Icon::BankAccount, Icon::Download, Qt::BottomRightCorner}},
    {Icon::AccountUpdateAll, {Icon::BankAccount, Icon::Download, Qt::BottomRightCorner}},
    {Icon::AccountCreditTransfer, {Icon::BankAccount, Icon::MailMessageNew, Qt::BottomRightCorner}},
    {Icon::TransactionMatch, {Icon::Transaction, Icon::DocumentImport, Qt::BottomRightCorner}},
    {Icon::TransactionAccept, {Icon::Transaction, Icon::DialogOKApply, Qt::BottomRightCorner}},
    {Icon::InvestmentNew, {Icon::Investment, Icon::ListAdd, Qt::TopRightCorner}},
    {Icon::InvestmentEdit, {Icon::Investment, Icon::DocumentEdit, Qt::BottomRightCorner}},
    {Icon::InvestmentDelete, {Icon::Investment, Icon::EditDelete, Qt::BottomRightCorner}},
    {Icon::InvestmentOnlinePrice, {Icon::Investment, Icon::Download, Qt::BottomRightCorner}},
    {Icon::InvestmentOnlinePriceAll, {Icon::Investment, Icon::Download, Qt::BottomRightCorner}},
    {Icon::BudgetNew, {Icon::Budget, Icon::ListAdd, Qt::TopRightCorner}},
    {Icon::BudgetRename, {Icon::Budget, Icon::DocumentEdit, Qt::BottomRightCorner}},
    {Icon::BudgetDelete, {Icon::Budget, Icon::EditDelete, Qt::BottomRightCorner}},
    {Icon::BudgetCopy, {Icon::Budget, Icon::EditCopy, Qt::BottomRightCorner}},
    {Icon::PriceUpdate, {Icon::Currencies, Icon::Download, Qt::BottomRightCorner}}};

KMM_ICONS_EXPORT void setUpMappings(const QString& themeName)
{
    IconSet themeIconSet;

    if (themeName.contains(QStringLiteral("oxygen"), Qt::CaseInsensitive))
        themeIconSet = IconSet::Oxygen;
    else if (themeName.contains(QStringLiteral("tango"), Qt::CaseInsensitive))
        themeIconSet = IconSet::Tango;
    // default to breeze mappings
    else
        themeIconSet = IconSet::Breeze;

    for (auto iconDef = iconMappings.cbegin(); iconDef != iconMappings.cend(); ++iconDef) {
        const auto icon = iconDef.key();
        auto name = iconDef.value().value(themeIconSet);

        // get common mapping if theme-specific does not exist
        if (name.isEmpty())
            name = iconDef.value().value(IconSet::Common);

        sStandardIcons.insert(icon, name);
    }
}

/**
 * This method overlays an icon over another one, to get a composite one
 * eg. an icon to add accounts
 */
QIcon overlayIcon(iconDescription description, const int size = 64)
{
    const auto iconName = sStandardIcons[description.baseIcon];
    const auto overlayName = sStandardIcons[description.overlayIcon];
    const auto corner = description.corner;

    QPixmap pxIcon;
    QString kyIcon = iconName + overlayName;

    // If found in the cache, return quickly
    if (QPixmapCache::find(kyIcon, pxIcon))
        return pxIcon;

    // try to retrieve the main icon from cache
    if (!QPixmapCache::find(iconName, pxIcon)) {
        pxIcon = QIcon::fromTheme(iconName).pixmap(size);
        QPixmapCache::insert(iconName, pxIcon);
    }

    if (overlayName.isEmpty()) // call from MyMoneyAccount::accountPixmap can have no overlay icon, so handle that appropriately
        return pxIcon;

    QPainter pixmapPainter(&pxIcon);
    QPixmap pxOverlay = QIcon::fromTheme(overlayName).pixmap(size);

    int x, y;
    switch (corner) {
    case Qt::TopLeftCorner:
        x = 0;
        y = 0;
        break;
    case Qt::TopRightCorner:
        x = pxIcon.width() / 2;
        y = 0;
        break;
    case Qt::BottomLeftCorner:
        x = 0;
        y = pxIcon.height() / 2;
        break;
    case Qt::BottomRightCorner:
    default:
        x = pxIcon.width() / 2;
        y = pxIcon.height() / 2;
        break;
    }
    pixmapPainter.drawPixmap(x, y, pxIcon.width() / 2, pxIcon.height() / 2, pxOverlay);

    // save for later use
    QPixmapCache::insert(kyIcon, pxIcon);

    return pxIcon;
}

KMM_ICONS_EXPORT QIcon get(Icon icon)
{
    if (sComposedIcons.contains(icon))
        return overlayIcon(sComposedIcons[icon]);

    return QIcon::fromTheme(sStandardIcons[icon]);
}

QString iconCacheDir()
{
    const QString cachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    if (QDir::root().mkpath(cachePath)) {
        return cachePath;
    }
    return QString();
}

KMM_ICONS_EXPORT bool storeIconInApplicationCache(const QString& name, const QIcon& icon)
{
    // split the icon name from the type
    QRegularExpression iconPath(QStringLiteral("^(?<type>[a-zA-Z]+):(?<name>.+)"), QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch matcher = iconPath.match(name);
    if (matcher.hasMatch()) {
        if (matcher.captured(QStringLiteral("type")).compare(QLatin1String("enum")) == 0) {
            return true;
        } else {
            const QString cacheDir = iconCacheDir();
            if (!cacheDir.isEmpty()) {
                return icon.pixmap(16).save(
                    QString::fromLatin1("%1/%2-%3").arg(cacheDir, matcher.captured(QStringLiteral("type")), matcher.captured(QStringLiteral("name"))),
                    "PNG");
            }
        }
    }
    return false;
}

KMM_ICONS_EXPORT QIcon loadIconFromApplicationCache(const QString& name)
{
    const QHash<QString, Icon> sEnumIcons{
        {QStringLiteral("Bank"), Icon::Bank},
    };

    // split the icon name from the type
    QRegularExpression iconPath(QStringLiteral("^(?<type>[a-zA-Z]+):(?<name>.+)"), QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch matcher = iconPath.match(name);
    if (matcher.hasMatch()) {
        if (matcher.captured(QStringLiteral("type")).compare(QLatin1String("enum")) == 0) {
            // type is enum, so we use our own set of icons
            const QString iconName = matcher.captured(QStringLiteral("name"));
            if (sEnumIcons.contains(iconName)) {
                return get(sEnumIcons[iconName]);
            }

        } else {
            // otherwise, we use the type as part of the filename
            const QString cacheDir = iconCacheDir();
            if (!cacheDir.isEmpty()) {
                const QString filename =
                    QString::fromLatin1("%1/%2-%3").arg(cacheDir, matcher.captured(QStringLiteral("type")), matcher.captured(QStringLiteral("name")));
                if (QFile::exists(filename)) {
                    return QIcon(filename);
                }
            }
        }
    }
    return QIcon();
}
}
