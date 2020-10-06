/***************************************************************************
                          icons.cpp
                             -------------------
    begin                : Sun Jun 25 2017
    copyright            : (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
                           (C) 2020 by Dawid Wróbel <me@dawidwrobel.com>

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
#include <QDir>
#include <QStandardPaths>

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

  const QHash<Icon, QHash<IconSet, QString> > iconMappings{
      {Icon::AccountNew,
       {{IconSet::Breeze, QStringLiteral("view-bank-account-add")}}},
      {Icon::AccountEdit,
       {{IconSet::Breeze, QStringLiteral("view-bank-account-edit")}}},
      {Icon::AccountRemove,
       {{IconSet::Breeze, QStringLiteral("view-bank-account-delete")}}},
      {Icon::AccountClose,
       {{IconSet::Breeze, QStringLiteral("view-bank-account-close")}}},
      {Icon::AccountReopen,
       {{IconSet::Breeze, QStringLiteral("view-bank-account-reopen")}}},
      {Icon::AccountUpdate,
       {{IconSet::Breeze, QStringLiteral("edit-download")}}},
      {Icon::AccountUpdateAll,
       {{IconSet::Breeze, QStringLiteral("edit-download")}}},
      {Icon::ArrowDown,
       {{IconSet::Common, QStringLiteral("arrow-down")}}},
      {Icon::ArrowLeft,
       {{IconSet::Common, QStringLiteral("arrow-left")}}},
      {Icon::ArrowRight,
       {{IconSet::Common, QStringLiteral("arrow-right")}}},
      {Icon::ArrowUp,
       {{IconSet::Common, QStringLiteral("arrow-up")}}},
      {Icon::Budget,
       {{IconSet::Breeze, QStringLiteral("view-financial-budget")}}},
      {Icon::Calculator,
       {{IconSet::Common, QStringLiteral("accessories-calculator")}}},
      {Icon::Configure,
       {{IconSet::Common, QStringLiteral("configure")},
        {IconSet::Tango, QStringLiteral("preferences-system")}}},
      {Icon::DialogCancel,
       {{IconSet::Common, QStringLiteral("dialog-cancel")},
        {IconSet::Tango, QStringLiteral("stop")}}},
      {Icon::DialogClose, {{IconSet::Common, QStringLiteral("dialog-close")}}},
      {Icon::DialogError, {{IconSet::Common, QStringLiteral("dialog-error")}}},
      {Icon::DialogInformation,
       {{IconSet::Common, QStringLiteral("dialog-information")}}},
      {Icon::DialogOK,
       {{IconSet::Common, QStringLiteral("dialog-ok")}}},
      {Icon::DialogOKApply,
       {{IconSet::Common, QStringLiteral("dialog-ok-apply")}}},
      {Icon::DialogWarning,
       {{IconSet::Common, QStringLiteral("dialog-warning")}}},
      {Icon::DocumentClose,
       {{IconSet::Common, QStringLiteral("document-close")}}},
      {Icon::DocumentEdit,
       {{IconSet::Common, QStringLiteral("document-edit")}}},
      {Icon::DocumentExport,
       {{IconSet::Breeze, QStringLiteral("document-export")}}},
      {Icon::DocumentImport,
       {{IconSet::Breeze, QStringLiteral("document-import")}}},
      {Icon::DocumentNew,
       {{IconSet::Common, QStringLiteral("document-new")}}},
      {Icon::DocumentOpen,
       {{IconSet::Common, QStringLiteral("document-open")}}},
      {Icon::DocumentProperties,
       {{IconSet::Common, QStringLiteral("document-properties")}}},
      {Icon::DocumentSave,
       {{IconSet::Common, QStringLiteral("document-save")}}},
      {Icon::Download,
       {{IconSet::Breeze, QStringLiteral("edit-download")},
        {IconSet::Common, QStringLiteral("go-down")},
        {IconSet::Oxygen, QStringLiteral("download")}}},
      {Icon::EditClear, {{IconSet::Common, QStringLiteral("edit-clear")}}},
      {Icon::EditCopy, {{IconSet::Common, QStringLiteral("edit-copy")}}},
      {Icon::EditRemove, {{IconSet::Common, QStringLiteral("edit-delete")}}},
      {Icon::Find, {{IconSet::Common, QStringLiteral("edit-find")}}},
      {Icon::EditRename,
       {{IconSet::Common, QStringLiteral("edit-rename")},
        {IconSet::Tango, QStringLiteral("text-editor")}}},
      {Icon::EditUndo, {{IconSet::Common, QStringLiteral("edit-undo")}}},
      {Icon::Folder, {{IconSet::Common, QStringLiteral("folder")}}},
      {Icon::GoTo, {{IconSet::Common, QStringLiteral("go-jump")}}},
      {Icon::Help, {{IconSet::Common, QStringLiteral("help-contents")}}},
      {Icon::HideCategories,
       {{IconSet::Common, QStringLiteral("hide-categories")}}},
      {Icon::HideReconciled,
       {{IconSet::Breeze, QStringLiteral("view-financial-transfer-unreconciled")}}},
      {Icon::KMyMoney, {{IconSet::Common, QStringLiteral("kmymoney")}}},
      {Icon::KeyEnter,
       {{IconSet::Breeze, QStringLiteral("key-enter")}}},
      {Icon::ListAdd, {{IconSet::Common, QStringLiteral("list-add")}}},
      {Icon::ListCollapse,
       {{IconSet::Common, QStringLiteral("zoom-out")},
        {IconSet::Tango, QStringLiteral("list-remove")}}},
      {Icon::ListExpand,
       {{IconSet::Common, QStringLiteral("zoom-in")},
        {IconSet::Tango, QStringLiteral("list-add")}}},
      {Icon::TagNew, {{IconSet::Breeze, QStringLiteral("tag-new")}}},
      {Icon::TagRemove,
       {{IconSet::Breeze, QStringLiteral("tag-delete")}}},
      {Icon::TagRename, {{IconSet::Breeze, QStringLiteral("tag-edit")}}},
      {Icon::MailMessage,
       {{IconSet::Common, QStringLiteral("internet-mail")},
        {IconSet::Oxygen, QStringLiteral("mail-message")},
        {IconSet::Breeze, QStringLiteral("mail-message")}}},
      {Icon::MailReceive, {{IconSet::Common, QStringLiteral("mail-receive")}}},
      {Icon::MapOnlineAccount,
       {{IconSet::Common, QStringLiteral("network-connect")}}},
      { Icon::OnlineTransfer,
        {{IconSet::Breeze, QStringLiteral("document-send")}}},
      {Icon::Merge,
       {{IconSet::Common, QStringLiteral("reconcile")},
        {IconSet::Oxygen, QStringLiteral("merge")},
        {IconSet::Breeze, QStringLiteral("merge")}}},
      {Icon::NewSchedule,
       {{IconSet::Common, QStringLiteral("appointment-new")}}},
      {Icon::OfficeCharBar,
       {{IconSet::Breeze, QStringLiteral("office-chart-bar")}}},
      {Icon::OfficeChartLineForecast,
       {{IconSet::Breeze, QStringLiteral("office-chart-line-forecast")}}},
      {Icon::OpenDatabase, {{IconSet::Common, QStringLiteral("server-database")}}},
      {Icon::Pause,
       {{IconSet::Common, QStringLiteral("media-playback-pause")}}},
      {Icon::PayeeNew,
       {{IconSet::Breeze, QStringLiteral("list-add-user")}}},
      {Icon::PayeeRemove,
       {{IconSet::Breeze, QStringLiteral("list-remove-user")}}},
      {Icon::PayeeMerge, {{IconSet::Breeze, QStringLiteral("merge")}}},
      {Icon::PayeeRename,
       {{IconSet::Common, QStringLiteral("user-properties")},
        {IconSet::Oxygen, QStringLiteral("payee-rename")}}},
      {Icon::PerformanceTest,
       {{IconSet::Common, QStringLiteral("fork")},
        {IconSet::Breeze, QStringLiteral("speedometer")}}},
      {Icon::PreferencesColors,
       {{IconSet::Common, QStringLiteral("preferences-desktop-color")}}},
      {Icon::PreferencesFonts,
       {{IconSet::Common, QStringLiteral("preferences-desktop-font")}}},
      {Icon::PreferencesGeneral,
       {{IconSet::Common, QStringLiteral("system-run")},
        {IconSet::Breeze, QStringLiteral("preferences-other")}}},
      {Icon::PreferencesIcons,
       {{IconSet::Breeze, QStringLiteral("preferences-desktop-icons")}}},
      {Icon::PreferencesNetwork,
       {{IconSet::Breeze, QStringLiteral("preferences-system-network")}}},
      {Icon::PreferencesPlugins,
       {{IconSet::Breeze, QStringLiteral("preferences-plugin")}}},
      {Icon::Reconcile,
       {{IconSet::Breeze, QStringLiteral("view-financial-transfer-reconcile")}}},
      {Icon::Refresh,
       {{IconSet::Common, QStringLiteral("view-refresh-symbolic")}}},
      {Icon::Report,
       {{IconSet::Breeze, QStringLiteral("labplot-spreadsheet")}}},
      {Icon::Reverse, {{IconSet::Breeze, QStringLiteral("reverse")}}},
      {Icon::SeekForward,
       {{IconSet::Common, QStringLiteral("media-seek-forward")}}},
      {Icon::SkipForward,
       {{IconSet::Common, QStringLiteral("media-skip-forward")}}},
      {Icon::SortAscending,
       {{IconSet::Common, QStringLiteral("view-sort-ascending-symbolic")}}},
      {Icon::SortDescending,
       {{IconSet::Common, QStringLiteral("view-sort-descending-symbolic")}}},
      {Icon::Split,
       {{IconSet::Common, QStringLiteral("transaction-split")},
        {IconSet::Oxygen, QStringLiteral("split")},
        {IconSet::Breeze, QStringLiteral("split")}}},
      {Icon::TaskAccepted,
       {{IconSet::Common, QStringLiteral("task-accepted")}}},
      {Icon::TaskComplete,
       {{IconSet::Common, QStringLiteral("task-complete")}}},
      {Icon::TaskOngoing, {{IconSet::Common, QStringLiteral("task-ongoing")}}},
      {Icon::TaskReject, {{IconSet::Common, QStringLiteral("task-reject")}}},
      {Icon::Tip,
       {{IconSet::Common, QStringLiteral("info")},
        {IconSet::Breeze, QStringLiteral("ktip")}}},
      {Icon::Unknown, {{IconSet::Common, QStringLiteral("unknown")}}},
      {Icon::UnmapOnlineAccount,
       {{IconSet::Common, QStringLiteral("network-disconnect")}}},
      {Icon::UserProperties,
       {{IconSet::Common, QStringLiteral("system-users")},
        {IconSet::Oxygen, QStringLiteral("user-properties")},
        {IconSet::Breeze, QStringLiteral("user-properties")}}},
      {Icon::Accounts,
       {{IconSet::Common, QStringLiteral("account")},
        {IconSet::Breeze, QStringLiteral("view-bank-account")}}},
      {Icon::Asset,
       {{IconSet::Breeze, QStringLiteral("view-asset-account")}}},
      {Icon::AssetClosed,
       {{IconSet::Breeze, QStringLiteral("view-asset-account-closed")}}},
      {Icon::BankAccount,
       {{IconSet::Breeze, QStringLiteral("view-bank-account")}}},
      {Icon::BankAccountClosed,
       {{IconSet::Breeze, QStringLiteral("view-bank-account-closed")}}},
      {Icon::Calendar,
       {{IconSet::Common, QStringLiteral("view-calendar")}}},
      {Icon::CalendarDay,
       {{IconSet::Common, QStringLiteral("view-calendar")},
        {IconSet::Oxygen, QStringLiteral("view-calendar-day")},
        {IconSet::Breeze, QStringLiteral("view-calendar-day")}}},
      {Icon::Cash,
       {{IconSet::Breeze, QStringLiteral("view-cash-account")}}},
      {Icon::CashClosed,
       {{IconSet::Breeze, QStringLiteral("view-cash-account-closed")}}},
      {Icon::Checking,
       {{IconSet::Breeze, QStringLiteral("view-bank-account-checking")}}},
      {Icon::CheckingClosed,
       {{IconSet::Breeze, QStringLiteral("view-bank-account-checking-closed")}}},
      {Icon::Close, {{IconSet::Common, QStringLiteral("view-close")}}},
      {Icon::CreditCard,
       {{IconSet::Breeze, QStringLiteral("view-credit-card-account")}}},
      {Icon::CreditCardClosed,
       {{IconSet::Breeze, QStringLiteral("view-credit-card-account-closed")}}},
      {Icon::Currencies,
       {{IconSet::Breeze, QStringLiteral("view-currency-list")}}},
      {Icon::Equity,
       {{IconSet::Common, QStringLiteral("account")},
        {IconSet::Oxygen, QStringLiteral("view-bank-account")},
        {IconSet::Breeze, QStringLiteral("view-bank-account")}}},
      {Icon::Expense,
       {{IconSet::Breeze, QStringLiteral("view-categories-expenditures")}}},
      {Icon::Filter, {{IconSet::Common, QStringLiteral("view-filter")}}},
      {Icon::FinancialCategories,
       {{IconSet::Breeze, QStringLiteral("preferences-devices-tree")}}},
      {Icon::FinancialCategoryNew,
       {{IconSet::Breeze, QStringLiteral("view-categories-add")}}},
      {Icon::FinancialCategoryEdit,
       {{IconSet::Breeze, QStringLiteral("view-categories-edit")}}},
      {Icon::FinancialCategoryRemove,
       {{IconSet::Breeze, QStringLiteral("view-categories-delete")}}},
      {Icon::Ledger,
       {{IconSet::Common, QStringLiteral("ledger")},
        {IconSet::Oxygen, QStringLiteral("view-financial-list")},
        {IconSet::Breeze, QStringLiteral("view-financial-list")}}},
      {Icon::Transaction,
       {{IconSet::Common, QStringLiteral("ledger")},
        {IconSet::Oxygen, QStringLiteral("view-financial-transfer")}}},
      {Icon::Forecast,
       {{IconSet::Breeze, QStringLiteral("weather-clouds-wind")}}},
      {Icon::Home,
       {{IconSet::Common, QStringLiteral("home")},
        {IconSet::Oxygen, QStringLiteral("go-home")},
        {IconSet::Breeze, QStringLiteral("go-home")}}},
      {Icon::Income,
       {{IconSet::Breeze, QStringLiteral("view-categories-incomes")},
        {IconSet::Common, QStringLiteral("account-types-income")},
        {IconSet::Oxygen, QStringLiteral("view-income-categories")}}},
      {Icon::Institution,
       {{IconSet::Breeze, QStringLiteral("view-institution")}}},
       {Icon::InstitutionNew,
       {{IconSet::Breeze, QStringLiteral("view-institution-add")}}},
      {Icon::InstitutionEdit,
       {{IconSet::Breeze, QStringLiteral("view-institution-edit")}}},
      {Icon::InstitutionRemove,
       {{IconSet::Breeze, QStringLiteral("view-institution-delete")}}},
      {Icon::Institutions,
       {{IconSet::Breeze, QStringLiteral("view-institution")}}},
      {Icon::Investment,
       {{IconSet::Breeze, QStringLiteral("view-investment-account")}}},
      {Icon::InvestmentClosed,
       {{IconSet::Breeze, QStringLiteral("view-investment-account-closed")}}},
      {Icon::InvestmentNew,
       {{IconSet::Breeze, QStringLiteral("")}}},
      {Icon::InvestmentEdit,
       {{IconSet::Breeze, QStringLiteral("")}}},
      {Icon::InvestmentRemove,
       {{IconSet::Breeze, QStringLiteral("")}}},
      {Icon::Investments,
       {{IconSet::Breeze, QStringLiteral("massif-visualizer")}}},
      {Icon::Liability,
       {{IconSet::Breeze, QStringLiteral("view-liability-account")}}},
      {Icon::LiabilityClosed,
       {{IconSet::Breeze, QStringLiteral("view-liability-account-closed")}}},
      {Icon::Loan,
       {{IconSet::Breeze, QStringLiteral("view-loan-account")}}},
      {Icon::LoanClosed,
       {{IconSet::Breeze, QStringLiteral("view-loan-account-closed")}}},
      {Icon::AssetLoan,
       {{IconSet::Breeze, QStringLiteral("view-loan-account")}}},
      {Icon::AssetLoanClosed,
       {{IconSet::Breeze, QStringLiteral("view-loan-account-closed")}}},
      {Icon::OnlineJobOutbox,
       {{IconSet::Breeze, QStringLiteral("internet-mail")}}},
      {Icon::OnlinePriceUpdate,
       {{IconSet::Breeze, QStringLiteral("view-refresh-symbolic")}}},
      {Icon::Payees,
       {{IconSet::Common, QStringLiteral("payee")},
        {IconSet::Oxygen, QStringLiteral("system-users")},
        {IconSet::Breeze, QStringLiteral("preferences-system-users")}}},
      {Icon::Reports,
       {{IconSet::Breeze, QStringLiteral("kspread")}}},
      {Icon::Savings,
       {{IconSet::Breeze, QStringLiteral("view-bank-account-savings")}}},
      {Icon::SavingsClosed,
       {{IconSet::Breeze, QStringLiteral("view-bank-account-savings-closed")}}},
      {Icon::Schedule,
       {{IconSet::Common, QStringLiteral("schedule")},
        {IconSet::Oxygen, QStringLiteral("view-pim-calendar")},
        {IconSet::Breeze, QStringLiteral("office-calendar")}}},
      {Icon::Stock,
       {{IconSet::Breeze, QStringLiteral("view-stock-account")}}},
      {Icon::StockClosed,
       {{IconSet::Breeze, QStringLiteral("view-stock-account-closed")}}},
      {Icon::Tags,
       {{IconSet::Common, QStringLiteral("bookmark-new")},
        {IconSet::Oxygen, QStringLiteral("mail-tagged")},
        {IconSet::Breeze, QStringLiteral("tag")}}},
      {Icon::TransactionDetails,
       {{IconSet::Common, QStringLiteral("edit-find")},
        {IconSet::Oxygen, QStringLiteral("zoom-in")},
        {IconSet::Breeze, QStringLiteral("zoom-in")}}},
      {Icon::ScheduleOverdue,
       {{IconSet::Common, QStringLiteral("view-calendar-upcoming-events")}}},
      {Icon::ScheduleOnTime,
       {{IconSet::Common, QStringLiteral("view-calendar-upcoming-events")}}},
      {Icon::ZoomIn, {{IconSet::Common, QStringLiteral("zoom-in")}}},
      {Icon::ZoomOut, {{IconSet::Common, QStringLiteral("zoom-out")}}},
      {Icon::Visibility, {{IconSet::Common, QStringLiteral("visibility")}}},
      {Icon::NoVisibility, {{IconSet::Common, QStringLiteral("hint")}}},
      {Icon::TransactionStateAny, {{IconSet::Common, QStringLiteral("unknown")}}},
      {Icon::TransactionStateImported, {{IconSet::Breeze, QStringLiteral("document-import")}}},
      {Icon::TransactionStateMatched, {{IconSet::Breeze, QStringLiteral("view-financial-transfer-matched")}}},
      {Icon::TransactionStateErroneous, {{IconSet::Breeze, QStringLiteral("dialog-warning-symbolic")}}},
      {Icon::TransactionStateScheduled, {{IconSet::Breeze, QStringLiteral("view-calendar-upcoming-days")}}},
      {Icon::TransactionStateReconciled, {{IconSet::Breeze, QStringLiteral("view-financial-transfer-reconciled")}}},
      {Icon::TransactionStateNotReconciled, {{IconSet::Breeze, QStringLiteral("view-financial-transfer-unreconciled")}}},
      {Icon::TransactionStateNotMarked, {{IconSet::Breeze, QStringLiteral("view-financial-transfer-notmarked")}}},
      {Icon::TransactionStateCleared, {{IconSet::Breeze, QStringLiteral("view-financial-transfer-cleared")}}}
  };

  const QHash<Icon, iconDescription> sComposedIcons {
    {Icon::AccountFinishReconciliation, {Icon::Merge,                    Icon::DialogOK,       Qt::BottomRightCorner}},
    {Icon::TransactionNew,              {Icon::Transaction, Icon::ListAdd, Qt::TopRightCorner}},
    {Icon::TransactionEdit,             {Icon::Transaction, Icon::DocumentEdit, Qt::BottomRightCorner}},
    {Icon::TransactionMatch,            {Icon::Transaction, Icon::DocumentImport, Qt::BottomRightCorner}},
    {Icon::TransactionAccept,           {Icon::Transaction, Icon::DialogOKApply, Qt::BottomRightCorner}},
    {Icon::BudgetNew,                   {Icon::Budget, Icon::ListAdd, Qt::TopRightCorner}},
    {Icon::BudgetRename,                {Icon::Budget, Icon::DocumentEdit, Qt::BottomRightCorner}},
    {Icon::BudgetRemove,                {Icon::Budget, Icon::EditRemove, Qt::BottomRightCorner}},
    {Icon::BudgetCopy,                  {Icon::Budget, Icon::EditCopy, Qt::BottomRightCorner}}
  };

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

    if (overlayName.isEmpty()) // call from MyMoneyAccount::accountIcon can have no overlay icon, so handle that appropriately
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

    auto name = sStandardIcons[icon];
    //return QIcon::fromTheme(sStandardIcons[icon]);
    auto icona = QIcon::fromTheme(sStandardIcons[icon]);
    bool iconNull = icona.isNull();
    auto iconSize = icona.themeName();
    return icona;
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
      { QStringLiteral("Bank"), Icon::Institution },
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
