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

#include <QString>

QHash<Icons::Icon, QString> g_Icons;

namespace Icons {

const QHash<Icon, QString> getCommonNames()
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
    {Icon::Folder, QStringLiteral("folder")}
  };
}

const QHash<Icon, QString> getOxygenNames()
{
  return {
    {Icon::Download, QStringLiteral("download")},
    {Icon::KeyEnter, QStringLiteral("key-enter")},
    {Icon::Split, QStringLiteral("split")},
    {Icon::Reconcile, QStringLiteral("merge")},
    {Icon::Tip, QStringLiteral("ktip")},
    {Icon::OfficeChartLine, QStringLiteral("office-chart-line")},
    {Icon::Merge, QStringLiteral("merge")},
    {Icon::ViewEquity, QStringLiteral("view-bank-account")},
    {Icon::ViewExpense, QStringLiteral("view-expenses-categories")},
    {Icon::ViewIncome, QStringLiteral("view-income-categories")},
    {Icon::ViewCreditCard, QStringLiteral("view-credit-card-account")},
    {Icon::ViewLoan, QStringLiteral("view-loan")},
    {Icon::ViewLoanAsset, QStringLiteral("view-loan-asset")},
    {Icon::ViewSaving, QStringLiteral("view-bank-account-savings")},
    {Icon::ViewChecking, QStringLiteral("view-bank-account-checking")},
    {Icon::ViewStock, QStringLiteral("view-stock-account")},
    {Icon::ViewLiability, QStringLiteral("view-loan")},
    {Icon::ViewAsset, QStringLiteral("view-bank-account")},
    {Icon::ViewBank, QStringLiteral("view-bank")},
    {Icon::ViewBankAccount, QStringLiteral("view-bank-account")},
    {Icon::ViewTimeScheduleCalculus, QStringLiteral("view-time-schedule-calculus")},
    {Icon::ViewForecast, QStringLiteral("view-financial-forecast")},
    {Icon::ViewBudgets, QStringLiteral("view-time-schedule-calculus")},
    {Icon::ViewReports, QStringLiteral("office-chart-bar")},
    {Icon::ViewInvestment, QStringLiteral("view-investment")},
    {Icon::ViewLedgers, QStringLiteral("view-financial-list")},
    {Icon::ViewPayees, QStringLiteral("system-users")},
    {Icon::ViewTags, QStringLiteral("mail-tagged")},
    {Icon::ViewCategories, QStringLiteral("view-financial-categories")},
    {Icon::ViewSchedules, QStringLiteral("view-pim-calendar")},
    {Icon::ViewAccounts, QStringLiteral("view-bank-account")},
    {Icon::ViewInstitutions, QStringLiteral("view-bank")},
    {Icon::ViewHome, QStringLiteral("go-home")},
    {Icon::ViewCalendarDay, QStringLiteral("view-calendar-day")},
    {Icon::ViewFinancialCategories, QStringLiteral("view-financial-categories")},
    {Icon::ViewFinancialTransfer, QStringLiteral("view-financial-transfer")},
    {Icon::ViewFinancialList, QStringLiteral("view-financial-list")},
    {Icon::ViewTransactionDetail, QStringLiteral("zoom-in")},
    {Icon::SortAscending, QStringLiteral("view-sort-ascending")},
    {Icon::SortDescending, QStringLiteral("view-sort-descending")},
    {Icon::UserProperties, QStringLiteral("user-properties")},
    {Icon::FileArchiver, QStringLiteral("utilities-file-archiver")},
    {Icon::FlagGreen, QStringLiteral("flag-green")},
    {Icon::AccountClosed, QStringLiteral("dialog-close")},
    {Icon::Refresh, QStringLiteral("refresh")},
    {Icon::MailMessage, QStringLiteral("mail-message")},
    {Icon::InvestApplet, QStringLiteral("invest-applet")},
    {Icon::DocumentImport, QStringLiteral("document-import")},
    {Icon::DocumentExport, QStringLiteral("document-export")}
  };
}

const QHash<Icon, QString> getTangoNames()
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
    {Icon::TaskAttention, QStringLiteral("dialog-warning")},
  };
}

KMM_ICONS_EXPORT void setIconThemeNames(const QString &_themeName)
{
  g_Icons = getCommonNames();
  QHash<Icon, QString> iconNames;
  // get icon replacements for specific theme
  if (_themeName == QLatin1String("oxygen"))
    iconNames = getOxygenNames();
  else if (_themeName == QLatin1String("Tango"))
    iconNames = getTangoNames();
  else
    return;

  for (auto it = iconNames.cbegin(); it != iconNames.cend(); ++it)
    g_Icons.insert(it.key(), it.value());
}
}
