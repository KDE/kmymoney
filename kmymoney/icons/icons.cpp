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

  const QHash<Icon, QString> iconMappings{
      {Icon::AccountNew, QStringLiteral("view-bank-account-add")},
      {Icon::AccountEdit, QStringLiteral("view-bank-account-edit")},
      {Icon::AccountRemove, QStringLiteral("view-bank-account-delete")},
      {Icon::AccountClose, QStringLiteral("view-bank-account-close")},
      {Icon::AccountReopen, QStringLiteral("view-bank-account-reopen")},
      {Icon::AccountUpdate, QStringLiteral("edit-download")},
      {Icon::AccountUpdateAll, QStringLiteral("edit-download")},
      {Icon::ArrowDown, QStringLiteral("arrow-down")},
      {Icon::ArrowLeft, QStringLiteral("arrow-left")},
      {Icon::ArrowRight, QStringLiteral("arrow-right")},
      {Icon::ArrowUp, QStringLiteral("arrow-up")},
      {Icon::Budget, QStringLiteral("view-financial-budget")},
      {Icon::Calculator, QStringLiteral("accessories-calculator")},
      {Icon::Configure, QStringLiteral("configure")},
      {Icon::DialogCancel, QStringLiteral("dialog-cancel")},
      {Icon::DialogClose, QStringLiteral("dialog-close")},
      {Icon::DialogError, QStringLiteral("dialog-error")},
      {Icon::DialogInformation, QStringLiteral("dialog-information")},
      {Icon::DialogOK, QStringLiteral("dialog-ok")},
      {Icon::DialogOKApply, QStringLiteral("dialog-ok-apply")},
      {Icon::DialogWarning, QStringLiteral("dialog-warning")},
      {Icon::DocumentClose, QStringLiteral("document-close")},
      {Icon::DocumentEdit, QStringLiteral("document-edit")},
      {Icon::DocumentExport, QStringLiteral("document-export")},
      {Icon::DocumentImport, QStringLiteral("document-import")},
      {Icon::DocumentNew, QStringLiteral("document-new")},
      {Icon::DocumentOpen, QStringLiteral("document-open")},
      {Icon::DocumentProperties, QStringLiteral("document-properties")},
      {Icon::DocumentSave, QStringLiteral("document-save")},
      {Icon::Download, QStringLiteral("edit-download")},
      {Icon::EditClear, QStringLiteral("edit-clear")},
      {Icon::EditCopy, QStringLiteral("edit-copy")},
      {Icon::EditRemove, QStringLiteral("edit-delete")},
      {Icon::Find, QStringLiteral("edit-find")},
      {Icon::EditRename, QStringLiteral("edit-rename")},
      {Icon::EditUndo, QStringLiteral("edit-undo")},
      {Icon::Folder, QStringLiteral("folder")},
      {Icon::GoTo, QStringLiteral("go-jump")},
      {Icon::Help, QStringLiteral("help-contents")},
      {Icon::HideCategories, QStringLiteral("hide-categories")},
      {Icon::HideReconciled, QStringLiteral("view-financial-transfer-unreconciled")},
      {Icon::KMyMoney, QStringLiteral("kmymoney")},
      {Icon::KeyEnter, QStringLiteral("key-enter")},
      {Icon::ListAdd, QStringLiteral("list-add")},
      {Icon::ListCollapse, QStringLiteral("zoom-out")},
      {Icon::ListExpand, QStringLiteral("zoom-in")},
      {Icon::TagNew, QStringLiteral("tag-new")},
      {Icon::TagRemove, QStringLiteral("tag-delete")},
      {Icon::TagRename, QStringLiteral("tag-edit")},
      {Icon::MailMessage, QStringLiteral("mail-message")},
      {Icon::MailReceive, QStringLiteral("mail-receive")},
      {Icon::MapOnlineAccount, QStringLiteral("network-connect")},
      {Icon::OnlineTransfer, QStringLiteral("document-send")},
      {Icon::Merge, QStringLiteral("merge")},
      {Icon::NewSchedule, QStringLiteral("appointment-new")},
      {Icon::OfficeCharBar, QStringLiteral("office-chart-bar")},
      {Icon::OfficeChartLineForecast, QStringLiteral("office-chart-line-forecast")},
      {Icon::OpenDatabase, QStringLiteral("server-database")},
      {Icon::Pause, QStringLiteral("media-playback-pause")},
      {Icon::PayeeNew, QStringLiteral("list-add-user")},
      {Icon::PayeeRemove, QStringLiteral("list-remove-user")},
      {Icon::PayeeMerge, QStringLiteral("merge")},
      {Icon::PayeeRename, QStringLiteral("user-properties")},
      {Icon::PerformanceTest, QStringLiteral("speedometer")},
      {Icon::PreferencesColors, QStringLiteral("preferences-desktop-color")},
      {Icon::PreferencesFonts, QStringLiteral("preferences-desktop-font")},
      {Icon::PreferencesGeneral, QStringLiteral("preferences-other")},
      {Icon::PreferencesIcons, QStringLiteral("preferences-desktop-icons")},
      {Icon::PreferencesNetwork, QStringLiteral("preferences-system-network")},
      {Icon::PreferencesPlugins, QStringLiteral("preferences-plugin")},
      {Icon::Reconcile, QStringLiteral("view-financial-transfer-reconcile")},
      {Icon::Reconciled, QStringLiteral("view-financial-transfer-reconciled")},
      {Icon::Refresh, QStringLiteral("view-refresh-symbolic")},
      {Icon::Report, QStringLiteral("labplot-spreadsheet")},
      {Icon::Reverse, QStringLiteral("reverse")},
      {Icon::SeekForward, QStringLiteral("media-seek-forward")},
      {Icon::SkipForward, QStringLiteral("media-skip-forward")},
      {Icon::SortAscending, QStringLiteral("view-sort-ascending-symbolic")},
      {Icon::SortDescending, QStringLiteral("view-sort-descending-symbolic")},
      {Icon::Split, QStringLiteral("split")},
      {Icon::TaskAccepted, QStringLiteral("task-accepted")},
      {Icon::TaskComplete, QStringLiteral("task-complete")},
      {Icon::TaskOngoing, QStringLiteral("task-ongoing")},
      {Icon::TaskReject, QStringLiteral("task-reject")},
      {Icon::Tip, QStringLiteral("ktip")},
      {Icon::Unknown, QStringLiteral("unknown")},
      {Icon::UnmapOnlineAccount, QStringLiteral("network-disconnect")},
      {Icon::UserProperties, QStringLiteral("user-properties")},
      {Icon::Accounts, QStringLiteral("view-bank-account")},
      {Icon::Asset, QStringLiteral("view-asset-account")},
      {Icon::AssetClosed, QStringLiteral("view-asset-account-closed")},
      {Icon::BankAccount, QStringLiteral("view-bank-account")},
      {Icon::BankAccountClosed, QStringLiteral("view-bank-account-closed")},
      {Icon::Calendar, QStringLiteral("view-calendar")},
      {Icon::CalendarDay, QStringLiteral("view-calendar-day")},
      {Icon::Cash, QStringLiteral("view-cash-account")},
      {Icon::CashClosed, QStringLiteral("view-cash-account-closed")},
      {Icon::Checking, QStringLiteral("view-bank-account-checking")},
      {Icon::CheckingClosed, QStringLiteral("view-bank-account-checking-closed")},
      {Icon::Close, QStringLiteral("view-close")},
      {Icon::CreditCard, QStringLiteral("view-credit-card-account")},
      {Icon::CreditCardClosed, QStringLiteral("view-credit-card-account-closed")},
      {Icon::Currencies, QStringLiteral("view-currency-list")},
      {Icon::Equity, QStringLiteral("view-bank-account")},
      {Icon::Expense, QStringLiteral("view-categories-expenditures")},
      {Icon::Filter, QStringLiteral("view-filter")},
      {Icon::FinancialCategories, QStringLiteral("preferences-devices-tree")},
      {Icon::FinancialCategoryNew, QStringLiteral("view-categories-add")},
      {Icon::FinancialCategoryEdit, QStringLiteral("view-categories-edit")},
      {Icon::FinancialCategoryRemove, QStringLiteral("view-categories-delete")},
      {Icon::Ledger, QStringLiteral("view-financial-list")},
      {Icon::Transaction, QStringLiteral("view-financial-transfer")},
      {Icon::Forecast, QStringLiteral("weather-clouds-wind")},
      {Icon::Home, QStringLiteral("go-home")},
      {Icon::Income, QStringLiteral("view-categories-incomes")},
      {Icon::Institution, QStringLiteral("view-institution")},
      {Icon::InstitutionNew, QStringLiteral("view-institution-add")},
      {Icon::InstitutionEdit, QStringLiteral("view-institution-edit")},
      {Icon::InstitutionRemove, QStringLiteral("view-institution-delete")},
      {Icon::Institutions, QStringLiteral("view-institution")},
      {Icon::Investment, QStringLiteral("view-investment-account")},
      {Icon::InvestmentClosed, QStringLiteral("view-investment-account-closed")},
      {Icon::InvestmentNew, QStringLiteral("")},
      {Icon::InvestmentEdit, QStringLiteral("")},
      {Icon::InvestmentRemove, QStringLiteral("")},
      {Icon::Investments, QStringLiteral("massif-visualizer")},
      {Icon::Liability, QStringLiteral("view-liability-account")},
      {Icon::LiabilityClosed, QStringLiteral("view-liability-account-closed")},
      {Icon::Loan, QStringLiteral("view-loan-account")},
      {Icon::LoanClosed, QStringLiteral("view-loan-account-closed")},
      {Icon::AssetLoan, QStringLiteral("view-loan-account")},
      {Icon::AssetLoanClosed, QStringLiteral("view-loan-account-closed")},
      {Icon::OnlineJobOutbox, QStringLiteral("internet-mail")},
      {Icon::OnlinePriceUpdate, QStringLiteral("view-refresh-symbolic")},
      {Icon::Payees, QStringLiteral("preferences-system-users")},
      {Icon::Reports, QStringLiteral("kspread")},
      {Icon::Savings, QStringLiteral("view-bank-account-savings")},
      {Icon::SavingsClosed, QStringLiteral("view-bank-account-savings-closed")},
      {Icon::Schedule, QStringLiteral("office-calendar")},
      {Icon::Stock, QStringLiteral("view-stock-account")},
      {Icon::StockClosed, QStringLiteral("view-stock-account-closed")},
      {Icon::Tags, QStringLiteral("tag")},
      {Icon::TransactionDetails, QStringLiteral("zoom-in")},
      {Icon::ScheduleOverdue, QStringLiteral("view-calendar-upcoming-events")},
      {Icon::ScheduleOnTime, QStringLiteral("view-calendar-upcoming-events")},
      {Icon::ZoomIn, QStringLiteral("zoom-in")},
      {Icon::ZoomOut, QStringLiteral("zoom-out")},
      {Icon::Visibility, QStringLiteral("visibility")},
      {Icon::NoVisibility, QStringLiteral("hint")},
      {Icon::Backup, QStringLiteral("backup")},
      {Icon::TransactionStateAny, QStringLiteral("unknown")},
      {Icon::TransactionStateImported, QStringLiteral("document-import")},
      {Icon::TransactionStateMatched, QStringLiteral("merge")},
      {Icon::TransactionStateErroneous, QStringLiteral("dialog-warning-symbolic")},
      {Icon::TransactionStateScheduled, QStringLiteral("view-calendar-upcoming-days")},
      {Icon::TransactionStateReconciled, QStringLiteral("view-financial-transfer-reconciled")},
      {Icon::TransactionStateNotReconciled, QStringLiteral("view-financial-transfer-unreconciled")},
      {Icon::TransactionStateNotMarked, QStringLiteral("view-financial-transfer-notmarked")},
      {Icon::TransactionStateCleared,QStringLiteral("view-financial-transfer-cleared")}
  };

  KMM_ICONS_EXPORT QIcon get(Icon icon)
  {
    return QIcon::fromTheme(iconMappings[icon]);
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
