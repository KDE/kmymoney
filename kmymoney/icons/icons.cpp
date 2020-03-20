/***************************************************************************
                          icons.cpp
                             -------------------
    begin                : Sun Jun 25 2017
    copyright            : (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "icons.h"

#include <QHash>
#include <QString>
#include <QIcon>
#include <QPixmapCache>
#include <QPainter>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QDir>
#include <QStandardPaths>

namespace Icons {
  QHash<Icon, QString> sStandardIcons;

  uint qHash(const Icon key, uint seed)
  {
    return ::qHash(static_cast<uint>(key), seed);
  }

  struct iconDescription {
    Icon baseIcon;
    Icon overlayIcon;
    Qt::Corner corner;
  };

  const QHash<Icon, iconDescription> sComposedIcons {
    {Icon::EditFindTransaction,         {Icon::ViewFinancialTransfer,    Icon::EditFind,       Qt::BottomRightCorner}},
    {Icon::InstitutionNew,              {Icon::ViewBank,                 Icon::ListAdd,        Qt::BottomRightCorner}},
    {Icon::InstitutionEdit,             {Icon::ViewBank,                 Icon::DocumentEdit,   Qt::BottomRightCorner}},
    {Icon::InstitutionDelete,           {Icon::ViewBank,                 Icon::EditDelete,     Qt::BottomRightCorner}},
    {Icon::AccountNew,                  {Icon::ViewBankAccount,          Icon::ListAdd,        Qt::TopRightCorner}},
    {Icon::AccountFinishReconciliation, {Icon::Merge,                    Icon::DialogOK,       Qt::BottomRightCorner}},
    {Icon::AccountEdit,                 {Icon::ViewBankAccount,          Icon::DocumentEdit,   Qt::BottomRightCorner}},
    {Icon::AccountDelete,               {Icon::ViewBankAccount,          Icon::EditDelete,     Qt::BottomRightCorner}},
    {Icon::AccountClose,                {Icon::ViewBankAccount,          Icon::DialogClose,    Qt::BottomRightCorner}},
    {Icon::AccountReopen,               {Icon::ViewBankAccount,          Icon::DialogOK,       Qt::BottomRightCorner}},
    {Icon::AccountUpdateMenu,           {Icon::ViewBankAccount,          Icon::Download,       Qt::BottomRightCorner}},
    {Icon::AccountUpdate,               {Icon::ViewBankAccount,          Icon::Download,       Qt::BottomRightCorner}},
    {Icon::AccountUpdateAll,            {Icon::ViewBankAccount,          Icon::Download,       Qt::BottomRightCorner}},
    {Icon::AccountCreditTransfer,       {Icon::ViewBankAccount,          Icon::MailMessageNew, Qt::BottomRightCorner}},
    {Icon::CategoryNew,                 {Icon::ViewFinancialCategories,  Icon::ListAdd,        Qt::TopRightCorner}},
    {Icon::CategoryEdit,                {Icon::ViewFinancialCategories,  Icon::DocumentEdit,   Qt::BottomRightCorner}},
    {Icon::CategoryDelete,              {Icon::ViewFinancialCategories,  Icon::EditDelete,     Qt::BottomRightCorner}},
    {Icon::ToolUpdatePrices,            {Icon::ViewInvestment,           Icon::Download,       Qt::BottomRightCorner}},
    {Icon::TransactionNew,              {Icon::ViewFinancialTransfer,    Icon::ListAdd,        Qt::TopRightCorner}},
    {Icon::TransactionEdit,             {Icon::ViewFinancialTransfer,    Icon::DocumentEdit,   Qt::BottomRightCorner}},
    {Icon::TransactionMatch,            {Icon::ViewFinancialTransfer,    Icon::DocumentImport, Qt::BottomRightCorner}},
    {Icon::TransactionAccept,           {Icon::ViewFinancialTransfer,    Icon::DialogOKApply,  Qt::BottomRightCorner}},
    {Icon::InvestmentNew,               {Icon::ViewInvestment,           Icon::ListAdd,        Qt::TopRightCorner}},
    {Icon::InvestmentEdit,              {Icon::ViewInvestment,           Icon::DocumentEdit,   Qt::BottomRightCorner}},
    {Icon::InvestmentDelete,            {Icon::ViewInvestment,           Icon::EditDelete,     Qt::BottomRightCorner}},
    {Icon::InvestmentOnlinePrice,       {Icon::ViewInvestment,           Icon::Download,       Qt::BottomRightCorner}},
    {Icon::BudgetNew,                   {Icon::ViewTimeScheduleCalculus, Icon::ListAdd,        Qt::TopRightCorner}},
    {Icon::BudgetRename,                {Icon::ViewTimeScheduleCalculus, Icon::DocumentEdit,   Qt::BottomRightCorner}},
    {Icon::BudgetDelete,                {Icon::ViewTimeScheduleCalculus, Icon::EditDelete,     Qt::BottomRightCorner}},
    {Icon::BudgetCopy,                  {Icon::ViewTimeScheduleCalculus, Icon::EditCopy,       Qt::BottomRightCorner}},
    {Icon::PriceUpdate,                 {Icon::ViewCurrencyList,         Icon::Download,       Qt::BottomRightCorner}}
  };

  QHash<Icon, QString> getCommonNames();
  QHash<Icon, QString> getKDENames();
  QHash<Icon, QString> getOxygenNames();
  QHash<Icon, QString> getBreezeNames();
  QHash<Icon, QString> getTangoNames();

  QHash<Icon, QString> getCommonNames()
  {
    return {
      {Icon::ListCollapse, QStringLiteral("zoom-out")},
      {Icon::ListExpand, QStringLiteral("zoom-in")},
      {Icon::ListAdd, QStringLiteral("list-add")},
      {Icon::ListRemoveTag, QStringLiteral("list-remove-tag")},
      {Icon::ListAddTag, QStringLiteral("list-add-tag")},
      {Icon::ListRemoveUser, QStringLiteral("list-remove-user")},
      {Icon::ListAddUser, QStringLiteral("list-add-user")},
      {Icon::AppointmentNew, QStringLiteral("appointment-new")},
      {Icon::KeyEnter, QStringLiteral("input-keyboard")},
      {Icon::GoJump, QStringLiteral("go-jump")},
      {Icon::EditUndo, QStringLiteral("edit-undo")},
      {Icon::EditFind, QStringLiteral("edit-find")},
      {Icon::EditRename, QStringLiteral("edit-rename")},
      {Icon::EditCopy, QStringLiteral("edit-copy")},
      {Icon::EditDelete, QStringLiteral("edit-delete")},
      {Icon::EditClear, QStringLiteral("edit-clear")},
      {Icon::DialogClose, QStringLiteral("dialog-close")},
      {Icon::DialogCancel, QStringLiteral("dialog-cancel")},
      {Icon::DialogOK, QStringLiteral("dialog-ok")},
      {Icon::DialogOKApply, QStringLiteral("dialog-ok-apply")},
      {Icon::DialogWarning, QStringLiteral("dialog-warning")},
      {Icon::DialogError, QStringLiteral("dialog-error")},
      {Icon::DialogInformation, QStringLiteral("dialog-information")},
      {Icon::DocumentClose, QStringLiteral("document-close")},
      {Icon::DocumentOpen, QStringLiteral("document-open")},
      {Icon::DocumentSave, QStringLiteral("document-save")},
      {Icon::DocumentImport, QStringLiteral("format-indent-less")},
      {Icon::DocumentNew, QStringLiteral("document-new")},
      {Icon::DocumentEdit, QStringLiteral("document-edit")},
      {Icon::DocumentProperties, QStringLiteral("document-properties")},
      {Icon::DocumentImport, QStringLiteral("format-indent-less")},
      {Icon::DocumentExport, QStringLiteral("format-indent-more")},
      {Icon::NewsUnsubscribe, QStringLiteral("news-unsubscribe")},
      {Icon::NewsSubscribe, QStringLiteral("news-subscribe")},
      {Icon::OfficeChartLine, QStringLiteral("account-types-investments")},
      {Icon::MediaSkipForward, QStringLiteral("media-skip-forward")},
      {Icon::MediaSeekForward, QStringLiteral("media-seek-forward")},
      {Icon::MediaPlaybackPause, QStringLiteral("media-playback-pause")},
      {Icon::Reconcile, QStringLiteral("reconcile")},
      {Icon::Merge, QStringLiteral("reconcile")},
      {Icon::Fork, QStringLiteral("fork")},
      {Icon::ViewEquity, QStringLiteral("account")},
      {Icon::ViewExpense, QStringLiteral("account-types-expense")},
      {Icon::ViewIncome, QStringLiteral("account-types-income")},
      {Icon::ViewCash, QStringLiteral("account-types-cash")},
      {Icon::ViewCreditCard, QStringLiteral("account-types-credit-card")},
      {Icon::ViewLoan, QStringLiteral("account-types-loan")},
      {Icon::ViewLoanAsset, QStringLiteral("account-types-loan")},
      {Icon::ViewSaving, QStringLiteral("account-types-savings")},
      {Icon::ViewChecking, QStringLiteral("account-types-checking")},
      {Icon::ViewStock, QStringLiteral("account-types-investments")},
      {Icon::ViewLiability, QStringLiteral("account-types-liability")},
      {Icon::ViewAsset, QStringLiteral("account-types-asset")},
      {Icon::ViewOutbox, QStringLiteral("online-banking")},
      {Icon::ViewForecast, QStringLiteral("forecast")},
      {Icon::ViewBudgets, QStringLiteral("budget")},
      {Icon::ViewReports, QStringLiteral("report")},
      {Icon::ViewInvestment, QStringLiteral("investment")},
      {Icon::ViewLedgers, QStringLiteral("ledger")},
      {Icon::ViewPayees, QStringLiteral("payee")},
      {Icon::ViewTags, QStringLiteral("bookmark-new")},
      {Icon::ViewCategories, QStringLiteral("categories")},
      {Icon::ViewSchedules, QStringLiteral("schedule")},
      {Icon::ViewAccounts, QStringLiteral("account")},
      {Icon::ViewInstitutions, QStringLiteral("institution")},
      {Icon::ViewHome, QStringLiteral("home")},
      {Icon::ViewClose, QStringLiteral("view-close")},
      {Icon::ViewBank, QStringLiteral("bank")},
      {Icon::ViewBankAccount, QStringLiteral("account")},
      {Icon::ViewTimeScheduleCalculus, QStringLiteral("budget")},
      {Icon::ViewCalendar, QStringLiteral("view-calendar")},
      {Icon::ViewCurrencyList, QStringLiteral("view-currency-list")},
      {Icon::ViewUpcominEvents, QStringLiteral("view-calendar-upcoming-events")},
      {Icon::ViewCalendarDay, QStringLiteral("office-calendar")},
      {Icon::ViewFinancialTransfer, QStringLiteral("ledger")},
      {Icon::ViewFinancialCategories, QStringLiteral("categories")},
      {Icon::ViewFinancialList, QStringLiteral("ledger")},
      {Icon::ViewFilter, QStringLiteral("view-filter")},
      {Icon::SortAscending, QStringLiteral("go-up")},
      {Icon::SortDescending, QStringLiteral("go-down")},
      {Icon::HideCategories, QStringLiteral("hide-categories")},
      {Icon::HideReconciled, QStringLiteral("hide-reconciled")},
      {Icon::ViewTransactionDetail, QStringLiteral("edit-find")},
      {Icon::ZoomOut, QStringLiteral("zoom-out")},
      {Icon::ZoomIn, QStringLiteral("zoom-in")},
      {Icon::DocumentProperties, QStringLiteral("document-properties")},
      {Icon::TagRename, QStringLiteral("tag-rename")},
      {Icon::PayeeMerge, QStringLiteral("merge")},
      {Icon::PayeeRename, QStringLiteral("user-properties")},
      {Icon::UserProperties, QStringLiteral("system-users")},
      {Icon::FileArchiver, QStringLiteral("package")},
      {Icon::AccessoriesCalculator, QStringLiteral("accessories-calculator")},
      {Icon::MailReceive, QStringLiteral("mail-receive")},
      {Icon::MailMessageNew, QStringLiteral("mail-message-new")},
      {Icon::MailMessage, QStringLiteral("internet-mail")},
      {Icon::SVNUpdate, QStringLiteral("svn-update")},
      {Icon::Split, QStringLiteral("transaction-split")},
      {Icon::Download, QStringLiteral("go-down")},
      {Icon::Tip, QStringLiteral("info")},
      {Icon::KMyMoney, QStringLiteral("kmymoney")},
      {Icon::FlagGreen, QStringLiteral("reconciled")},
      {Icon::AccountClosed, QStringLiteral("account-types-closed")},
      {Icon::Unknown, QStringLiteral("unknown")},
      {Icon::Configure, QStringLiteral("configure")},
      {Icon::Spreadsheet, QStringLiteral("application-vnd.oasis.opendocument.spreadsheet")},
      {Icon::ArrowUp, QStringLiteral("arrow-up")},
      {Icon::ArrowDown, QStringLiteral("arrow-down")},
      {Icon::ArrowLeft, QStringLiteral("arrow-left")},
      {Icon::ArrowRight, QStringLiteral("arrow-right")},
      {Icon::SystemRun, QStringLiteral("system-run")},
      {Icon::TaskAttention, QStringLiteral("task-attention")},
      {Icon::TaskOngoing, QStringLiteral("task-ongoing")},
      {Icon::TaskComplete, QStringLiteral("task-complete")},
      {Icon::TaskReject, QStringLiteral("task-reject")},
      {Icon::TaskAccepted, QStringLiteral("task-accepted")},
      {Icon::HelpContents, QStringLiteral("help-contents")},
      {Icon::Kgpg, QStringLiteral("kgpg")},
      {Icon::PreferencesNetwork, QStringLiteral("preferences-system-network")},
      {Icon::PreferencesColor, QStringLiteral("preferences-desktop-color")},
      {Icon::PreferencesFont, QStringLiteral("preferences-desktop-font")},
      {Icon::PreferencesIcon, QStringLiteral("preferences-desktop-icon")},
      {Icon::NetworkDisconect, QStringLiteral("network-disconnect")},
      {Icon::Folder, QStringLiteral("folder")},
      {Icon::Reverse, QStringLiteral("reverse")},
    };
  }

  QHash<Icon, QString> getKDENames()
  {
    return {
      {Icon::ViewHome, QStringLiteral("go-home")},
      {Icon::KeyEnter, QStringLiteral("key-enter")},
      {Icon::Split, QStringLiteral("split")},
      {Icon::Reconcile, QStringLiteral("merge")},
      {Icon::OfficeChartLine, QStringLiteral("office-chart-line")},
      {Icon::Merge, QStringLiteral("merge")},
      {Icon::ViewEquity, QStringLiteral("view-bank-account")},
      {Icon::ViewSaving, QStringLiteral("view-bank-account-savings")},
      {Icon::ViewChecking, QStringLiteral("view-bank-account-checking")},
      {Icon::ViewAsset, QStringLiteral("view-bank-account")},
      {Icon::ViewBank, QStringLiteral("view-bank")},
      {Icon::ViewBankAccount, QStringLiteral("view-bank-account")},
      {Icon::ViewTimeScheduleCalculus, QStringLiteral("view-time-schedule-calculus")},
      {Icon::ViewBudgets, QStringLiteral("view-time-schedule-calculus")},
      {Icon::ViewCalendarDay, QStringLiteral("view-calendar-day")},
      {Icon::ViewTransactionDetail, QStringLiteral("zoom-in")},
      {Icon::ViewReports, QStringLiteral("office-chart-bar")},
      {Icon::ViewPayees, QStringLiteral("system-users")},
      {Icon::ViewTags, QStringLiteral("mail-tagged")},
      {Icon::ViewSchedules, QStringLiteral("view-pim-calendar")},
      {Icon::ViewAccounts, QStringLiteral("view-bank-account")},
      {Icon::ViewInstitutions, QStringLiteral("view-bank")},
      {Icon::ViewCategories, QStringLiteral("view-categories")},
      {Icon::ViewFinancialCategories, QStringLiteral("view-categories")},
      {Icon::FileArchiver, QStringLiteral("utilities-file-archiver")},
      {Icon::UserProperties, QStringLiteral("user-properties")},
      {Icon::SortAscending, QStringLiteral("view-sort-ascending")},
      {Icon::SortDescending, QStringLiteral("view-sort-descending")},
      {Icon::FlagGreen, QStringLiteral("flag-green")},
      {Icon::AccountClosed, QStringLiteral("dialog-close")},
      {Icon::MailMessage, QStringLiteral("mail-message")},
      {Icon::DocumentImport, QStringLiteral("document-import")},
      {Icon::DocumentExport, QStringLiteral("document-export")}
    };
  }

  QHash<Icon, QString> getOxygenNames()
  {
    return {
      {Icon::Download, QStringLiteral("download")},
      {Icon::Tip, QStringLiteral("ktip")},
      {Icon::ViewExpense, QStringLiteral("view-expenses-categories")},
      {Icon::ViewIncome, QStringLiteral("view-income-categories")},
      {Icon::ViewCreditCard, QStringLiteral("view-credit-card-account")},
      {Icon::ViewLoan, QStringLiteral("view-loan")},
      {Icon::ViewLoanAsset, QStringLiteral("view-loan-asset")},
      {Icon::ViewStock, QStringLiteral("view-stock-account")},
      {Icon::ViewLiability, QStringLiteral("view-loan")},
      {Icon::ViewForecast, QStringLiteral("view-financial-forecast")},
      {Icon::ViewInvestment, QStringLiteral("view-investment")},
      {Icon::ViewLedgers, QStringLiteral("view-financial-list")},
      {Icon::ViewCategories, QStringLiteral("view-financial-categories")},
      {Icon::ViewFinancialCategories, QStringLiteral("view-financial-categories")},
      {Icon::ViewFinancialTransfer, QStringLiteral("view-financial-transfer")},
      {Icon::ViewFinancialList, QStringLiteral("view-financial-list")},
      {Icon::Refresh, QStringLiteral("refresh")},
      {Icon::InvestApplet, QStringLiteral("invest-applet")}
    };
  }

  QHash<Icon, QString> getBreezeNames()
  {
    return {
      {Icon::ViewExpense, QStringLiteral("view-categories-expenditures")},
      {Icon::ViewIncome, QStringLiteral("view-categories-incomes")},
      {Icon::ViewCreditCard, QStringLiteral("skrooge_credit_card")},
      {Icon::Download, QStringLiteral("edit-download")},
      {Icon::Refresh, QStringLiteral("view-refresh")}
    };
  }

  QHash<Icon, QString> getTangoNames()
  {
    return {
      {Icon::OfficeChartLine, QStringLiteral("report-line")},
      {Icon::ListCollapse, QStringLiteral("list-remove")},
      {Icon::ListExpand, QStringLiteral("list-add")},
      {Icon::DocumentEdit, QStringLiteral("text-editor")},
      {Icon::DialogCancel, QStringLiteral("stop")},
      {Icon::DialogOK, QStringLiteral("finish")},
      {Icon::EditRename, QStringLiteral("text-editor")},
      {Icon::DocumentClose, QStringLiteral("stop")},
      {Icon::Configure, QStringLiteral("preferences-system")},
      {Icon::ArrowUp, QStringLiteral("go-up")},
      {Icon::ArrowDown, QStringLiteral("go-down")},
      {Icon::ArrowLeft, QStringLiteral("go-previous")},
      {Icon::ArrowRight, QStringLiteral("go-next")},
      {Icon::SystemRun, QStringLiteral("media-playback-start")},
      {Icon::TaskAttention, QStringLiteral("dialog-warning")}
    };
  }

  KMM_ICONS_EXPORT void setIconThemeNames(const QString &_themeName)
  {
    sStandardIcons = getCommonNames();
    auto hasIconsResource = false;

#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
    hasIconsResource = true;
#endif

    QStringList kdeThemes {QStringLiteral("oxygen"), QStringLiteral("breeze"), QStringLiteral("breeze-dark")};
    QHash<Icon, QString> iconNames;

    if (kdeThemes.contains(_themeName) || hasIconsResource) { // on Craft build system there is breeze icon theme, but it's in no way discoverable
      iconNames = getKDENames();
      for (auto it = iconNames.cbegin(); it != iconNames.cend(); ++it)
        sStandardIcons.insert(it.key(), it.value());
    }

    // get icon replacements for specific theme
    if (_themeName == kdeThemes.at(0))
      iconNames = getOxygenNames();
    else if (_themeName == kdeThemes.at(1) || _themeName == kdeThemes.at(2) || hasIconsResource)
      iconNames = getBreezeNames();
    else if (_themeName == QLatin1String("Tango"))
      iconNames = getTangoNames();
    else
      return;

    for (auto it = iconNames.cbegin(); it != iconNames.cend(); ++it)
      sStandardIcons.insert(it.key(), it.value());
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

    //save for later use
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
          return icon.pixmap(16).save(QString::fromLatin1("%1/%2-%3").arg(cacheDir, matcher.captured(QStringLiteral("type")), matcher.captured(QStringLiteral("name"))), "PNG");
        }
      }
    }
    return false;
  }

  KMM_ICONS_EXPORT QIcon loadIconFromApplicationCache(const QString& name)
  {
    const QHash<QString, Icon> sEnumIcons {
      { QStringLiteral("ViewBank"), Icon::ViewBank },
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
          const QString filename = QString::fromLatin1("%1/%2-%3").arg(cacheDir, matcher.captured(QStringLiteral("type")), matcher.captured(QStringLiteral("name")));
          if (QFile::exists(filename)) {
            return QIcon(filename);
          }
        }
      }
    }
    return QIcon();
  }
}
