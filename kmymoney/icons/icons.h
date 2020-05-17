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

enum class IconSet { Common, Oxygen, Tango, Breeze };

enum class Icon { OpenDatabase, Merge, Reconcile, Split, Tip, PerformanceTest,
                  Calculator,
                  UserProperties, DocumentProperties,
                  ZoomIn, ZoomOut,
                  Pause, SeekForward,
                  SkipForward,
                  HideReconciled, HideCategories,
                  ViewHome, ViewInstitutions,
                  ViewAccounts, ViewCategories,
                  ViewSchedules, ViewTags,
                  ViewPayees, ViewLedgers,
                  ViewInvestments, ViewReports,
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
                  Budget, ViewCalendar,
                  ViewTransactionDetail, ViewClose,
                  DialogOK, DialogClose, DialogCancel,
                  DialogOKApply, DialogError, DialogWarning,
                  DialogInformation,
                  ListExpand, ListCollapse,
                  ListAdd, ListAddUser, ListRemoveUser,
                  ListAddTag, ListRemoveTag,
                  GoTo, KeyEnter, Download, TagRename,
                  EditDelete, EditCopy, EditRename,
                  EditFind, EditUndo, EditClear,
                  DocumentEdit,
                  DocumentNew, DocumentSave,
                  DocumentClose, DocumentOpen,
                  DocumentImport, DocumentExport,
                  OfficeChartLine,
                  MailMessageNew, MailMessage, MailReceive,
                  MapOnlineAccount, UnmapOnlineAccount,
                  NewSchedule, KMyMoney,
                  PayeeRename, PayeeMerge,
                  Configure,
                  Reconciled, AccountClosed, Unknown,
                  Report, Refresh, PreferencesGeneral,
                  SortAscending, SortDescending,
                  ArrowUp, ArrowDown, ArrowRight, ArrowLeft,
                  TaskAttention, TaskComplete,
                  TaskReject, TaskAccepted, TaskOngoing,
                  Help, Folder,
                  PreferencesFonts, PreferencesColors, PreferencesIcons,
                  PreferencesNetwork, PreferencesPlugins,
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

KMM_ICONS_EXPORT void setUpMappings(const QString & themeName);
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
