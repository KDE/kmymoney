/*
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2020 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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

// clang-format off
enum class Icon { OpenDatabase, Link, Unlink, Reconcile, Split, Tip, PerformanceTest,
                  Calculator,
                  UserProperties, DocumentProperties,
                  ZoomIn, ZoomOut,
                  Pause, SeekForward, SeekBackward,
                  SkipForward, SkipBackward,
                  HideReconciled, HideCategories,
                  Home, Institution, Institutions,
                  Accounts,
                  Schedule, Tags,
                  Payee, Payees,
                  Investment, InvestmentClosed, Investments, Reports,
                  Budget, Budgets, Forecast,
                  OnlineJobOutbox, Filter,
                  Loan, LoanClosed, Security, SecurityClosed,
                  Checking, CheckingClosed,
                  Savings, SavingsClosed,
                  AssetLoan, AssetLoanClosed,CreditCard, CreditCardClosed,
                  Cash, CashClosed, Equity,
                  Income, Expense,
                  Asset, AssetClosed, Liability, LiabilityClosed,
                  ScheduleOverdue, ScheduleOnTime, CalendarDay,
                  Ledger, Ledgers, BankAccount, BankAccountClosed,
                  Currencies, FinancialCategories,
                  Transaction,
                  Calendar,
                  TransactionDetails, Close,
                  DialogOK, DialogClose, DialogCancel,
                  DialogOKApply, DialogError, DialogWarning,
                  DialogInformation,
                  ListExpand, ListCollapse,
                  ListAdd, PayeeNew, PayeeRemove,
                  TagNew, TagRemove,
                  GoTo, KeyEnter, Download, TagRename,
                  EditRemove, EditShred, EditCopy, EditRename,
                  Find, EditUndo, EditClear,
                  DocumentEdit,
                  DocumentNew, DocumentSave,
                  DocumentClose, DocumentOpen,
                  DocumentImport, DocumentExport,
                  OfficeCharBar, OfficeChartLineForecast,
                  MailMessage, MailSend, MailReceive,
                  MapOnlineAccount, UnmapOnlineAccount,
                  Globe,
                  NewSchedule, KMyMoney,
                  PayeeRename, PayeeMerge,
                  Configure,
                  TransactionStateReconciled, Unknown,
                  Report, Refresh, PreferencesGeneral,
                  SortAscending, SortDescending,
                  ArrowUp, ArrowDown, ArrowRight, ArrowLeft,
                  TaskComplete,
                  TaskReject, TaskAccepted, TaskOngoing,
                  Help, Community, Folder,
                  PreferencesFonts, PreferencesColors, PreferencesIcons,
                  PreferencesNetwork, PreferencesPlugins,
                  Empty,
                  InstitutionNew, InstitutionEdit, InstitutionRemove,
                  AccountNew, AccountEdit, AccountRemove,
                  AccountClose, AccountReopen,
                  AccountUpdate, AccountUpdateAll,
                  OnlineTransfer, Reconciled,
                  FinancialCategoryNew, FinancialCategoryEdit, FinancialCategoryRemove,
                  TransactionStateAny, TransactionStateImported, TransactionStateMatched,
                  TransactionStateErroneous, TransactionStateScheduled, TransactionStateNotReconciled,
                  TransactionStateNotMarked, TransactionStateCleared,
                  InvestmentNew, InvestmentEdit,
                  InvestmentRemove, OnlinePriceUpdate,
                  Reverse, Visibility, NoVisibility,
                  SelectAll, Backup,
                };
// clang-format on

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
