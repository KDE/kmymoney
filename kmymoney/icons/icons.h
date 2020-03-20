/***************************************************************************
                          icons.h
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

#ifndef ICONS_H
#define ICONS_H

// ----------------------------------------------------------------------------
// QT Includes

#include <icons/kmm_icons_export.h>

// ----------------------------------------------------------------------------
// Project Includes

class QString;
class QIcon;

namespace Icons {

enum class Icon { SVNUpdate, Merge, Reconcile, Split, Tip, Fork,
                  FileArchiver, AccessoriesCalculator,
                  UserProperties, DocumentProperties,
                  ZoomIn, ZoomOut,
                  MediaPlaybackPause, MediaSeekForward,
                  MediaSkipForward,
                  HideReconciled, HideCategories,
                  ViewHome, ViewInstitutions,
                  ViewAccounts, ViewCategories,
                  ViewSchedules, ViewTags,
                  ViewPayees, ViewLedgers,
                  ViewInvestment, ViewReports,
                  ViewBudgets, ViewForecast,
                  ViewOutbox, ViewFilter,
                  ViewLoan, ViewStock,
                  ViewChecking,
                  ViewSaving,
                  ViewLoanAsset, ViewCreditCard,
                  ViewCash, ViewEquity,
                  ViewIncome, ViewExpense,
                  ViewAsset, ViewLiability,
                  ViewUpcominEvents, ViewCalendarDay,
                  ViewFinancialList, ViewBankAccount,
                  ViewCurrencyList, ViewFinancialCategories,
                  ViewFinancialTransfer, ViewBank,
                  ViewTimeScheduleCalculus, ViewCalendar,
                  ViewTransactionDetail, ViewClose,
                  DialogOK, DialogClose, DialogCancel,
                  DialogOKApply, DialogError, DialogWarning,
                  DialogInformation,
                  ListExpand, ListCollapse,
                  ListAdd, ListAddUser, ListRemoveUser,
                  ListAddTag, ListRemoveTag,
                  GoJump, KeyEnter, Download, TagRename,
                  EditDelete, EditCopy, EditRename,
                  EditFind, EditUndo, EditClear,
                  DocumentEdit,
                  DocumentNew, DocumentSave,
                  DocumentClose, DocumentOpen,
                  DocumentImport, DocumentExport,
                  OfficeChartLine,
                  MailMessageNew, MailMessage, MailReceive,
                  NewsSubscribe, NewsUnsubscribe,
                  AppointmentNew, KMyMoney,
                  PayeeRename, PayeeMerge,
                  PathEnter, PathSkip, Configure,
                  FlagGreen, AccountClosed, Unknown,
                  Spreadsheet, Refresh, SystemRun,
                  SortAscending, SortDescending,
                  ArrowUp, ArrowDown, ArrowRight, ArrowLeft,
                  TaskAttention, TaskComplete,
                  TaskReject, TaskAccepted, TaskOngoing,
                  HelpContents, Folder, InvestApplet,
                  PreferencesFont, PreferencesColor, PreferencesIcon,
                  PreferencesNetwork, NetworkDisconect, Kgpg,
                  Empty, EditFindTransaction,
                  InstitutionNew, InstitutionEdit, InstitutionDelete,
                  AccountNew, AccountEdit, AccountDelete,
                  AccountClose, AccountReopen,
                  AccountUpdateMenu, AccountUpdate, AccountUpdateAll,
                  AccountCreditTransfer, AccountFinishReconciliation,
                  CategoryNew, CategoryEdit, CategoryDelete,
                  TransactionNew, TransactionEdit,
                  TransactionMatch, TransactionAccept,
                  InvestmentNew, InvestmentEdit,
                  InvestmentDelete, InvestmentOnlinePrice,
                  BudgetNew, BudgetRename, BudgetDelete, BudgetCopy,
                  PriceUpdate, ToolUpdatePrices, Reverse
                };

KMM_ICONS_EXPORT void setIconThemeNames(const QString &_themeName);
KMM_ICONS_EXPORT QIcon get(Icons::Icon icon);


/**
 * return an icon from the application local cache or an icon provided
 * by the application. The @a name is formatted as @c type:iconName.
 * The following types are supported
 *
 * - enum
 * - favicon
 *
 * @sa storeIconInApplicationCache(const QString& name, const QIcon& icon)
 */
KMM_ICONS_EXPORT QIcon loadIconFromApplicationCache(const QString& name);

/**
 * store the @a icon in the applications local cache directory under the given @a name.
 * The @a name is formatted as @c type:iconName.
 * The icon will be stored in the file "type-iconName".
 *
 * @sa loadIconFromApplicationCache(const QString& name)
 */
KMM_ICONS_EXPORT bool storeIconInApplicationCache(const QString& name, const QIcon& icon);
}

#endif
