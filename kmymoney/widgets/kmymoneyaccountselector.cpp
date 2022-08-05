/*
    SPDX-FileCopyrightText: 2003-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kmymoneyaccountselector.h"
#include "kmymoneyselector_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QIcon>
#include <QList>
#include <QPixmapCache>
#include <QPushButton>
#include <QVBoxLayout>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "dialogenums.h"
#include "icons.h"
#include "mymoneyaccount.h"
#include "mymoneyenums.h"
#include "mymoneyfile.h"
#include "widgetenums.h"

using namespace Icons;
using namespace eMyMoney;

class KMyMoneyAccountSelectorPrivate : public KMyMoneySelectorPrivate
{
    Q_DISABLE_COPY(KMyMoneyAccountSelectorPrivate)

public:
    KMyMoneyAccountSelectorPrivate(KMyMoneyAccountSelector *qq) :
        KMyMoneySelectorPrivate(qq),
        m_allAccountsButton(0),
        m_noAccountButton(0),
        m_incomeCategoriesButton(0),
        m_expenseCategoriesButton(0)
    {
    }

    QPushButton*              m_allAccountsButton;
    QPushButton*              m_noAccountButton;
    QPushButton*              m_incomeCategoriesButton;
    QPushButton*              m_expenseCategoriesButton;
    QList<int>                m_typeList;
    QStringList               m_accountList;
};

KMyMoneyAccountSelector::KMyMoneyAccountSelector(QWidget *parent, Qt::WindowFlags flags, const bool createButtons) :
    KMyMoneySelector(*new KMyMoneyAccountSelectorPrivate(this), parent, flags)
{
    Q_D(KMyMoneyAccountSelector);
    if (createButtons) {
        QVBoxLayout* buttonLayout = new QVBoxLayout();
        buttonLayout->setSpacing(6);

        d->m_allAccountsButton = new QPushButton(this);
        d->m_allAccountsButton->setObjectName("m_allAccountsButton");
        d->m_allAccountsButton->setText(i18nc("Select all accounts", "All"));
        buttonLayout->addWidget(d->m_allAccountsButton);

        d->m_incomeCategoriesButton = new QPushButton(this);
        d->m_incomeCategoriesButton->setObjectName("m_incomeCategoriesButton");
        d->m_incomeCategoriesButton->setText(i18n("Income"));
        buttonLayout->addWidget(d->m_incomeCategoriesButton);

        d->m_expenseCategoriesButton = new QPushButton(this);
        d->m_expenseCategoriesButton->setObjectName("m_expenseCategoriesButton");
        d->m_expenseCategoriesButton->setText(i18n("Expense"));
        buttonLayout->addWidget(d->m_expenseCategoriesButton);

        d->m_noAccountButton = new QPushButton(this);
        d->m_noAccountButton->setObjectName("m_noAccountButton");
        d->m_noAccountButton->setText(i18nc("No account", "None"));
        buttonLayout->addWidget(d->m_noAccountButton);

        QSpacerItem* spacer = new QSpacerItem(0, 67, QSizePolicy::Minimum, QSizePolicy::Expanding);
        buttonLayout->addItem(spacer);
        d->m_layout->addLayout(buttonLayout);

        connect(d->m_allAccountsButton, &QAbstractButton::clicked, this, &KMyMoneyAccountSelector::slotSelectAllAccounts);
        connect(d->m_noAccountButton, &QAbstractButton::clicked, this, &KMyMoneyAccountSelector::slotDeselectAllAccounts);
        connect(d->m_incomeCategoriesButton, &QAbstractButton::clicked, this, &KMyMoneyAccountSelector::slotSelectIncomeCategories);
        connect(d->m_expenseCategoriesButton, &QAbstractButton::clicked, this, &KMyMoneyAccountSelector::slotSelectExpenseCategories);
    }
}

KMyMoneyAccountSelector::~KMyMoneyAccountSelector()
{
}

void KMyMoneyAccountSelector::removeButtons()
{
    Q_D(KMyMoneyAccountSelector);
    delete d->m_allAccountsButton;
    delete d->m_incomeCategoriesButton;
    delete d->m_expenseCategoriesButton;
    delete d->m_noAccountButton;
}

void KMyMoneyAccountSelector::slotSelectAllAccounts()
{
    selectAllItems(true);
}

void KMyMoneyAccountSelector::slotDeselectAllAccounts()
{
    selectAllItems(false);
}

void KMyMoneyAccountSelector::selectCategories(const bool income, const bool expense)
{
    Q_D(KMyMoneyAccountSelector);
    QTreeWidgetItemIterator it_v(d->m_treeWidget);

    for (; *it_v != 0; ++it_v) {
        if ((*it_v)->text(0) == i18n("Income categories"))
            selectAllSubItems(*it_v, income);
        else if ((*it_v)->text(0) == i18n("Expense categories"))
            selectAllSubItems(*it_v, expense);
    }
    emit stateChanged();
}

void KMyMoneyAccountSelector::slotSelectIncomeCategories()
{
    selectCategories(true, false);
}

void KMyMoneyAccountSelector::slotSelectExpenseCategories()
{
    selectCategories(false, true);
}

void KMyMoneyAccountSelector::setSelectionMode(QTreeWidget::SelectionMode mode)
{
    Q_D(KMyMoneyAccountSelector);
    d->m_incomeCategoriesButton->setHidden(mode == QTreeWidget::MultiSelection);
    d->m_expenseCategoriesButton->setHidden(mode == QTreeWidget::MultiSelection);
    KMyMoneySelector::setSelectionMode(mode);
}

QStringList KMyMoneyAccountSelector::accountList(const  QList<Account::Type>& filterList) const
{
    Q_D(const KMyMoneyAccountSelector);
    QStringList    list;
    QTreeWidgetItemIterator it(d->m_treeWidget, QTreeWidgetItemIterator::Selectable);

    while (*it) {
        QVariant id = (*it)->data(0, (int)eWidgets::Selector::Role::Id);
        MyMoneyAccount acc = MyMoneyFile::instance()->account(id.toString());
        if (filterList.count() == 0 || filterList.contains(acc.accountType()))
            list << id.toString();
        it++;
    }
    return list;
}

QStringList KMyMoneyAccountSelector::accountList() const
{
    return accountList(QList<eMyMoney::Account::Type>());
}

bool KMyMoneyAccountSelector::match(const QRegularExpression& exp, QTreeWidgetItem* item) const
{
    if (!item->flags().testFlag(Qt::ItemIsSelectable))
        return false;
    const auto matchingItem(exp.match(item->data(0, static_cast<int>(eWidgets::Selector::Role::Key)).toString().mid(1)));
    return matchingItem.hasMatch();
}

bool KMyMoneyAccountSelector::contains(const QString& txt) const
{
    Q_D(const KMyMoneyAccountSelector);
    QTreeWidgetItemIterator it(d->m_treeWidget, QTreeWidgetItemIterator::Selectable);
    QTreeWidgetItem* it_v;

    QString baseName = i18n("Asset") + '|' +
                       i18n("Liability") + '|' +
                       i18n("Income") + '|' +
                       i18n("Expense") + '|' +
                       i18n("Equity") + '|' +
                       i18n("Security");

    while ((it_v = *it) != 0) {
        const QRegularExpression exp(QString("^(?:%1):%2$").arg(baseName).arg(QRegularExpression::escape(txt)));
        const auto matchingItem(exp.match(it_v->data(0, static_cast<int>(eWidgets::Selector::Role::Key)).toString().mid(1)));
        if (matchingItem.hasMatch()) {
            return true;
        }
        it++;
    }
    return false;
}

class AccountSetPrivate
{
    Q_DISABLE_COPY(AccountSetPrivate)

public:
    AccountSetPrivate()
        : m_count(0)
        , m_file(MyMoneyFile::instance())
        , m_favorites(0)
        , m_hideClosedAccounts(true)
        , m_showInvestments(false)
    {
    }

    int                      m_count;
    MyMoneyFile*             m_file;
    QList<eMyMoney::Account::Type> m_typeList;
    QTreeWidgetItem*         m_favorites;
    bool                     m_hideClosedAccounts;
    bool                     m_showInvestments;
};

AccountSet::AccountSet() :
    d_ptr(new AccountSetPrivate)
{
}

AccountSet::~AccountSet()
{
    Q_D(AccountSet);
    delete d;
}

void AccountSet::setShowInvestments(bool show)
{
    Q_D(AccountSet);
    d->m_showInvestments = show;
}

void AccountSet::addAccountGroup(Account::Type group)
{
    Q_D(AccountSet);
    if (group == Account::Type::Asset) {
        d->m_typeList << Account::Type::Checkings;
        d->m_typeList << Account::Type::Savings;
        d->m_typeList << Account::Type::Cash;
        d->m_typeList << Account::Type::AssetLoan;
        d->m_typeList << Account::Type::CertificateDep;
        d->m_typeList << Account::Type::Investment;
        d->m_typeList << Account::Type::Stock;
        d->m_typeList << Account::Type::MoneyMarket;
        d->m_typeList << Account::Type::Asset;
        d->m_typeList << Account::Type::Currency;

    } else if (group == Account::Type::Liability) {
        d->m_typeList << Account::Type::CreditCard;
        d->m_typeList << Account::Type::Loan;
        d->m_typeList << Account::Type::Liability;

    } else if (group == Account::Type::Income) {
        d->m_typeList << Account::Type::Income;

    } else if (group == Account::Type::Expense) {
        d->m_typeList << Account::Type::Expense;

    } else if (group == Account::Type::Equity) {
        d->m_typeList << Account::Type::Equity;
    }
}

void AccountSet::addAccountType(Account::Type type)
{
    Q_D(AccountSet);
    d->m_typeList << type;
}

void AccountSet::removeAccountType(Account::Type type)
{
    Q_D(AccountSet);
    int index = d->m_typeList.indexOf(type);
    if (index != -1) {
        d->m_typeList.removeAt(index);
    }
}

void AccountSet::clear()
{
    Q_D(AccountSet);
    d->m_typeList.clear();
}

int AccountSet::load(KMyMoneyAccountSelector* selector)
{
    Q_D(AccountSet);
    QStringList list;
    QStringList::ConstIterator it_l;
    int count = 0;
    int typeMask = 0;
    QString currentId;

    if (selector->selectionMode() == QTreeWidget::SingleSelection) {
        selector->selectedItems(list);
        if (!list.isEmpty())
            currentId = list.first();
    }
    // clang-format off
    if (d->m_typeList.contains(Account::Type::Checkings)
            || d->m_typeList.contains(Account::Type::Savings)
            || d->m_typeList.contains(Account::Type::Cash)
            || d->m_typeList.contains(Account::Type::AssetLoan)
            || d->m_typeList.contains(Account::Type::CertificateDep)
            || d->m_typeList.contains(Account::Type::Investment)
            || d->m_typeList.contains(Account::Type::Stock)
            || d->m_typeList.contains(Account::Type::MoneyMarket)
            || d->m_typeList.contains(Account::Type::Asset)
            || d->m_typeList.contains(Account::Type::Currency))
        typeMask |= eDialogs::Category::asset;

    if (d->m_typeList.contains(Account::Type::CreditCard)
            || d->m_typeList.contains(Account::Type::Loan)
            || d->m_typeList.contains(Account::Type::Liability))
        typeMask |= eDialogs::Category::liability;
    // clang-format on

    if (d->m_typeList.contains(Account::Type::Income))
        typeMask |= eDialogs::Category::income;

    if (d->m_typeList.contains(Account::Type::Expense))
        typeMask |= eDialogs::Category::expense;

    if (d->m_typeList.contains(Account::Type::Equity))
        typeMask |= eDialogs::Category::equity;

    selector->clear();
    QTreeWidget* lv = selector->listView();
    d->m_count = 0;
    QString key;
    QTreeWidgetItem* after = 0;

    // create the favorite section first and sort it to the beginning
    key = QString("A%1").arg(i18n("Favorites"));
    d->m_favorites = selector->newItem(i18n("Favorites"), key);

    //get the account icon from cache or insert it if it is not there
    QPixmap accountPixmap;
    if (!QPixmapCache::find("account", &accountPixmap)) {
        QIcon icon = Icons::get(Icon::BankAccount);
        if (!icon.availableSizes().isEmpty())
            accountPixmap = icon.pixmap(icon.availableSizes().first());
        QPixmapCache::insert("account", accountPixmap);
    }
    d->m_favorites->setIcon(0, QIcon(accountPixmap));

    for (auto mask = 0x01; mask != eDialogs::Category::last; mask <<= 1) {
        QTreeWidgetItem* item = 0;
        if ((typeMask & mask & eDialogs::Category::asset) != 0) {
            ++d->m_count;
            key = QString("B%1").arg(i18n("Asset"));
            item = selector->newItem(i18n("Asset accounts"), key);
            item->setIcon(0, d->m_file->asset().accountIcon());
            list = d->m_file->asset().accountList();
        }

        if ((typeMask & mask & eDialogs::Category::liability) != 0) {
            ++d->m_count;
            key = QString("C%1").arg(i18n("Liability"));
            item = selector->newItem(i18n("Liability accounts"), key);
            item->setIcon(0, d->m_file->liability().accountIcon());
            list = d->m_file->liability().accountList();
        }

        if ((typeMask & mask & eDialogs::Category::income) != 0) {
            ++d->m_count;
            key = QString("D%1").arg(i18n("Income"));
            item = selector->newItem(i18n("Income categories"), key);
            item->setIcon(0, d->m_file->income().accountIcon());
            list = d->m_file->income().accountList();
            if (selector->selectionMode() == QTreeWidget::MultiSelection) {
                selector->d_func()->m_incomeCategoriesButton->show();
            }
        }

        if ((typeMask & mask & eDialogs::Category::expense) != 0) {
            ++d->m_count;
            key = QString("E%1").arg(i18n("Expense"));
            item = selector->newItem(i18n("Expense categories"), key);
            item->setIcon(0, d->m_file->expense().accountIcon());
            list = d->m_file->expense().accountList();
            if (selector->selectionMode() == QTreeWidget::MultiSelection) {
                selector->d_func()->m_expenseCategoriesButton->show();
            }
        }

        if ((typeMask & mask & eDialogs::Category::equity) != 0) {
            ++d->m_count;
            key = QString("F%1").arg(i18n("Equity"));
            item = selector->newItem(i18n("Equity accounts"), key);
            item->setIcon(0, d->m_file->equity().accountIcon());
            list = d->m_file->equity().accountList();
        }

        if (!after)
            after = item;

        if (item != 0) {
            // scan all matching accounts found in the engine
            for (it_l = list.constBegin(); it_l != list.constEnd(); ++it_l) {
                const MyMoneyAccount& acc = d->m_file->account(*it_l);
                ++d->m_count;
                ++count;
                //this will include an account if it matches the account type and
                //if it is still open or it has been set to show closed accounts
                if (includeAccount(acc)
                        && (!isHidingClosedAccounts() || !acc.isClosed())) {
                    QString tmpKey;
                    tmpKey = key + MyMoneyFile::AccountSeparator + acc.name();
                    QTreeWidgetItem* subItem = selector->newItem(item, acc.name(), tmpKey, acc.id());
                    subItem->setIcon(0, acc.accountIcon());
                    if (acc.value("PreferredAccount") == "Yes"
                            && d->m_typeList.contains(acc.accountType())) {
                        selector->newItem(d->m_favorites, acc.name(), tmpKey, acc.id())->setIcon(0, acc.accountIcon());
                    }
                    if (acc.accountList().count() > 0) {
                        subItem->setExpanded(true);
                        count += loadSubAccounts(selector, subItem, tmpKey, acc.accountList());
                    }

                    // the item is not selectable if it has been added only because a subaccount matches the type
                    if (!d->m_typeList.contains(acc.accountType())) {
                        selector->setSelectable(subItem, false);
                    }
                    subItem->sortChildren(1, Qt::AscendingOrder);
                }
            }
            item->sortChildren(1, Qt::AscendingOrder);
        }
    }
    d->m_favorites->sortChildren(1, Qt::AscendingOrder);
    lv->invisibleRootItem()->sortChildren(1, Qt::AscendingOrder);

    // if we don't have a favorite account or the selector is for multi-mode
    // we get rid of the favorite entry and subentries.
    if (d->m_favorites->childCount() == 0 || selector->selectionMode() == QTreeWidget::MultiSelection) {
        delete d->m_favorites;
        d->m_favorites = 0;
    }

    if (lv->itemAt(0, 0)) {
        if (currentId.isEmpty()) {
            lv->setCurrentItem(lv->itemAt(0, 0));
            lv->clearSelection();
        } else {
            selector->setSelected(currentId);
        }
    }
    selector->update();
    return count;
}

int AccountSet::load(KMyMoneyAccountSelector* selector, const QString& baseName, const QList<QString>& accountIdList, const bool clear)
{
    Q_D(AccountSet);
    int count = 0;
    QTreeWidgetItem* item = 0;

    d->m_typeList.clear();
    if (clear) {
        d->m_count = 0;
        selector->clear();
    }

    item = selector->newItem(baseName);
    ++d->m_count;

    QList<QString>::ConstIterator it;
    for (it = accountIdList.constBegin(); it != accountIdList.constEnd(); ++it)   {
        const MyMoneyAccount& acc = d->m_file->account(*it);
        if (acc.isClosed())
            continue;
        QString tmpKey;
        // the first character must be preset. Since we don't know any sort order here, we just use A
        tmpKey = QString("A%1%2%3").arg(baseName, MyMoneyFile::AccountSeparator, acc.name());
        selector->newItem(item, acc.name(), tmpKey, acc.id())->setIcon(0, acc.accountIcon());
        ++d->m_count;
        ++count;
    }

    QTreeWidget* lv = selector->listView();
    if (lv->itemAt(0, 0)) {
        lv->setCurrentItem(lv->itemAt(0, 0));
        lv->clearSelection();
    }

    selector->update();
    return count;
}

int AccountSet::count() const
{
    Q_D(const AccountSet);
    return d->m_count;
}

void AccountSet::setHideClosedAccounts(bool _bool)
{
    Q_D(AccountSet);
    d->m_hideClosedAccounts = _bool;
}
bool AccountSet::isHidingClosedAccounts() const
{
    Q_D(const AccountSet);
    return d->m_hideClosedAccounts;
}

int AccountSet::loadSubAccounts(KMyMoneyAccountSelector* selector, QTreeWidgetItem* parent, const QString& key, const QStringList& list)
{
    Q_D(AccountSet);
    QStringList::ConstIterator it_l;
    int count = 0;

    for (it_l = list.constBegin(); it_l != list.constEnd(); ++it_l) {
        const MyMoneyAccount& acc = d->m_file->account(*it_l);
        // don't include stock accounts if not in expert mode
        if (acc.isInvest() && !d->m_showInvestments)
            continue;

        //this will include an account if it matches the account type and
        //if it is still open or it has been set to show closed accounts
        if (includeAccount(acc)
                && (!isHidingClosedAccounts() || !acc.isClosed())) {
            QString tmpKey;
            tmpKey = key + MyMoneyFile::AccountSeparator + acc.name();
            ++count;
            ++d->m_count;
            QTreeWidgetItem* item = selector->newItem(parent, acc.name(), tmpKey, acc.id());
            item->setIcon(0, acc.accountIcon());
            if (acc.value("PreferredAccount") == "Yes"
                    && d->m_typeList.contains(acc.accountType())) {
                selector->newItem(d->m_favorites, acc.name(), tmpKey, acc.id())->setIcon(0, acc.accountIcon());
            }
            if (acc.accountList().count() > 0) {
                item->setExpanded(true);
                count += loadSubAccounts(selector, item, tmpKey, acc.accountList());
            }

            // the item is not selectable if it has been added only because a subaccount matches the type
            if (!d->m_typeList.contains(acc.accountType())) {
                selector->setSelectable(item, false);
            }
            item->sortChildren(1, Qt::AscendingOrder);
        }
    }
    return count;
}

bool AccountSet::includeAccount(const MyMoneyAccount& acc)
{
    Q_D(AccountSet);
    if (d->m_typeList.contains(acc.accountType()))
        return true;

    foreach (const auto sAccount, acc.accountList())
        if (includeAccount(d->m_file->account(sAccount)))
            return true;

    return false;
}
