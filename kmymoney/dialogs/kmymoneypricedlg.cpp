/*
    SPDX-FileCopyrightText: 2004-2017 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kmymoneypricedlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QCheckBox>
#include <QIcon>
#include <QMenu>
#include <QSortFilterProxyModel>
#include <QVector>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>
#include <KLocalizedString>
#include <KTreeWidgetSearchLine>
#include <KTreeWidgetSearchLineWidget>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kmymoneypricedlg.h"
#include "ui_kupdatestockpricedlg.h"

#include "icons.h"
#include "kcurrencycalculator.h"
#include "kequitypriceupdatedlg.h"
#include "kmymoneycurrencyselector.h"
#include "kmymoneyutils.h"
#include "kpricetreeitem.h"
#include "kupdatestockpricedlg.h"
#include "menuenums.h"
#include "mymoneyaccount.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneymoney.h"
#include "mymoneyprice.h"
#include "mymoneysecurity.h"
#include "mymoneyutils.h"
#include "pricemodel.h"

#include "kmmyesno.h"

using namespace Icons;

class PriceSortFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    PriceSortFilterModel(QObject* parent)
        : QSortFilterProxyModel(parent)
        , m_showAllEntries(false)
    {
    }

protected:
    bool lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const override
    {
        switch (source_left.column()) {
        case PriceModel::Column::Date:
            return source_left.data(eMyMoney::Model::PriceDateRole).toDate() < source_right.data(eMyMoney::Model::PriceDateRole).toDate();

        case PriceModel::Column::Price:
            return source_left.data(eMyMoney::Model::PriceRateRole).value<MyMoneyMoney>()
                < source_right.data(eMyMoney::Model::PriceRateRole).value<MyMoneyMoney>();

        default:
            return QSortFilterProxyModel::lessThan(source_left, source_right);
        }
    }

    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override
    {
        if (!m_showAllEntries) {
            const auto thisIdx = sourceModel()->index(source_row, 0, source_parent);
            const auto nextIdx = sourceModel()->index(source_row + 1, 0, source_parent);
            if (nextIdx.isValid()) {
                const auto thisId = thisIdx.data(eMyMoney::Model::PricePairRole).toString();
                const auto nextId = nextIdx.data(eMyMoney::Model::PricePairRole).toString();
                if (thisId == nextId) {
                    return false;
                }
            }
        }
        return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
    }

public Q_SLOTS:
    void setShowAllEntries(bool showAll)
    {
        m_showAllEntries = showAll;
        invalidateFilter();
    }

private:
    bool m_showAllEntries;
};

class KMyMoneyPriceDlgPrivate
{
    Q_DISABLE_COPY(KMyMoneyPriceDlgPrivate)
    Q_DECLARE_PUBLIC(KMyMoneyPriceDlg)

public:
    explicit KMyMoneyPriceDlgPrivate(KMyMoneyPriceDlg* qq)
        : q_ptr(qq)
        , ui(new Ui::KMyMoneyPriceDlg)
        , m_sortModel(new PriceSortFilterModel(qq))
    {
    }

    ~KMyMoneyPriceDlgPrivate()
    {
        delete ui;
    }

    void editPrice()
    {
        const auto indexes = ui->m_priceList->selectionModel()->selectedRows();
        if (!indexes.isEmpty()) {
            const auto baseIdx = MyMoneyModelBase::mapToBaseSource(indexes.at(0));
            const auto price = MyMoneyFile::instance()->priceModel()->itemByIndex(baseIdx);
            editPrice(price);
        }
    }

    int editPrice(const MyMoneyPrice& price)
    {
        Q_Q(KMyMoneyPriceDlg);
        int rc = QDialog::Rejected;
        MyMoneySecurity from(MyMoneyFile::instance()->security(price.from()));
        MyMoneySecurity to(MyMoneyFile::instance()->security(price.to()));
        signed64 fract = MyMoneyMoney::precToDenom(from.pricePrecision());

        QPointer<KCurrencyCalculator> calc = new KCurrencyCalculator(from, to, MyMoneyMoney::ONE, price.rate(to.id()), price.date(), fract, q);
        calc->setupPriceEditor();

        rc = calc->exec();
        delete calc;

        // make sure old prices are hidden
        m_sortModel->invalidate();

        return rc;
    }

    void selectionChanged()
    {
        const auto indexes = ui->m_priceList->selectionModel()->selectedRows();
        ui->m_editButton->setEnabled(!indexes.isEmpty());
        ui->m_deleteButton->setEnabled(!indexes.isEmpty());

        // if one of the selected entries is a default, then deleting and editing is disabled
        for (const auto& idx : indexes) {
            if (idx.data(eMyMoney::Model::PriceSourceRole).toString() == QLatin1String("KMyMoney")) {
                ui->m_deleteButton->setEnabled(false);
                ui->m_editButton->setEnabled(false);
                break;
            }
        }

        // Multiple entries cannot be edited at once
        if (indexes.count() > 1) {
            ui->m_editButton->setEnabled(false);
        }
    }

    KMyMoneyPriceDlg* q_ptr;
    Ui::KMyMoneyPriceDlg* ui;
    PriceSortFilterModel* m_sortModel;
    QKeySequence m_searchShortCut;
};

KMyMoneyPriceDlg::KMyMoneyPriceDlg(QWidget* parent) :
    QDialog(parent),
    d_ptr(new KMyMoneyPriceDlgPrivate(this))
{
    Q_D(KMyMoneyPriceDlg);
    d->ui->setupUi(this);

    d->ui->m_deleteButton->setIcon(Icons::get(Icon::EditRemove));
    d->ui->m_newButton->setIcon(Icons::get(Icon::DocumentNew));
    d->ui->m_editButton->setIcon(Icons::get(Icon::DocumentEdit));

    d->ui->m_onlineQuoteButton->setIcon(Icons::get(Icon::OnlinePriceUpdate));

    connect(d->ui->m_editButton, &QAbstractButton::clicked, this, &KMyMoneyPriceDlg::slotEditPrice);
    connect(d->ui->m_deleteButton, &QAbstractButton::clicked, this, &KMyMoneyPriceDlg::slotDeletePrice);
    connect(d->ui->m_newButton, &QAbstractButton::clicked, this, &KMyMoneyPriceDlg::slotNewPrice);
    connect(d->ui->m_onlineQuoteButton, &QAbstractButton::clicked, this, &KMyMoneyPriceDlg::slotOnlinePriceUpdate);

    // setup model view stack
    d->m_sortModel->setSortLocaleAware(true);
    d->m_sortModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    d->m_sortModel->setSortRole(Qt::DisplayRole);
    d->m_sortModel->setSourceModel(MyMoneyFile::instance()->priceModel());
    d->m_sortModel->setFilterKeyColumn(-1); // filter on all columns

    d->ui->m_priceList->setModel(d->m_sortModel);
    d->ui->m_priceList->setSortingEnabled(true);

    d->ui->m_priceList->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    d->ui->m_priceList->sortByColumn(PriceModel::Column::Commodity, Qt::AscendingOrder);
    d->ui->m_priceList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(d->ui->m_priceList, &QWidget::customContextMenuRequested, this, &KMyMoneyPriceDlg::slotShowPriceMenu);

    connect(d->ui->m_showAllPrices, &QAbstractButton::toggled, d->m_sortModel, &PriceSortFilterModel::setShowAllEntries);
    connect(d->ui->m_priceList->selectionModel(), &QItemSelectionModel::selectionChanged, this, [&]() {
        Q_D(KMyMoneyPriceDlg);
        d->selectionChanged();
    });
    connect(d->ui->m_searchWidget, &KLineEdit::textChanged, d->m_sortModel, &QSortFilterProxyModel::setFilterFixedString);

    // in case we have items, select the first one
    if (d->ui->m_priceList->model()->hasChildren()) {
        const auto firstIdx = d->ui->m_priceList->model()->index(0, 0);
        d->ui->m_priceList->setCurrentIndex(firstIdx);
        d->ui->m_priceList->selectionModel()->select(firstIdx, QItemSelectionModel::Select);
    }
    d->selectionChanged();
}

KMyMoneyPriceDlg::~KMyMoneyPriceDlg()
{
    Q_D(KMyMoneyPriceDlg);
    delete d;
}

void KMyMoneyPriceDlg::slotNewPrice()
{
    Q_D(KMyMoneyPriceDlg);
    QPointer<KUpdateStockPriceDlg> dlg = new KUpdateStockPriceDlg(this);
    try {
        auto idx = d->ui->m_priceList->currentIndex();
        if (idx.isValid()) {
            MyMoneySecurity security;
            security = MyMoneyFile::instance()->security(idx.data(eMyMoney::Model::PriceFromRole).toString());
            dlg->ui->m_security->setSecurity(security);
            security = MyMoneyFile::instance()->security(idx.data(eMyMoney::Model::PriceToRole).toString());
            dlg->ui->m_currency->setSecurity(security);
        }

        if (dlg->exec()) {
            MyMoneyPrice price(dlg->ui->m_security->security().id(), dlg->ui->m_currency->security().id(), dlg->date(), MyMoneyMoney::ONE, QString());
            d->editPrice(price);
        }
    } catch (...) {
    }
    delete dlg;
}

void KMyMoneyPriceDlg::slotEditPrice()
{
    Q_D(KMyMoneyPriceDlg);
    d->editPrice();
}

void KMyMoneyPriceDlg::slotDeletePrice()
{
    Q_D(KMyMoneyPriceDlg);
    const auto indexes = d->ui->m_priceList->selectionModel()->selectedRows();

    if (!indexes.isEmpty()) {
        if (KMessageBox::questionTwoActions(
                this,
                i18np("Do you really want to delete the selected price entry?", "Do you really want to delete the selected price entries?", indexes.count()),
                i18n("Delete price information"),
                KMMYesNo::yes(),
                KMMYesNo::no(),
                "DeletePrice")
            == KMessageBox::PrimaryAction) {
            MyMoneyFileTransaction ft;
            try {
                // we cannot remove the prices on the fly since the indexes will change
                // while the model is updated. we simply collect all price objects in
                // a local list and delete from it
                QList<MyMoneyPrice> pricesToBeDeleted;
                for (const auto& idx : indexes) {
                    const auto baseIdx = MyMoneyModelBase::mapToBaseSource(idx);
                    const auto price = MyMoneyFile::instance()->priceModel()->itemByIndex(baseIdx);
                    pricesToBeDeleted << price;
                }
                // now we can safely delete
                for (const auto& price : pricesToBeDeleted) {
                    MyMoneyFile::instance()->removePrice(price);
                }
                ft.commit();

                // make sure current prices are shown
                d->m_sortModel->invalidate();

            } catch (const MyMoneyException &) {
                qDebug("Cannot delete price");
            }
        }
    }
}

void KMyMoneyPriceDlg::slotOnlinePriceUpdate()
{
    Q_D(KMyMoneyPriceDlg);
    QPointer<KEquityPriceUpdateDlg> dlg = new KEquityPriceUpdateDlg(this);
    dlg->setSearchShortcut(d->m_searchShortCut);

    if (dlg->exec() == Accepted && dlg) {
        dlg->storePrices();

        // make sure current prices are shown
        d->m_sortModel->invalidate();
    }
    delete dlg;
}

void KMyMoneyPriceDlg::slotShowPriceMenu(const QPoint& p)
{
    Q_D(KMyMoneyPriceDlg);
    const auto idx = d->ui->m_priceList->indexAt(p);

    d->ui->m_priceList->selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    const auto baseIdx = MyMoneyModelBase::mapToBaseSource(idx);
    const auto price = MyMoneyFile::instance()->priceModel()->itemByIndex(baseIdx);
    const auto cond1 = !price.from().isEmpty() && price.source() != QLatin1String("KMyMoney");
    const auto cond2 = cond1 && MyMoneyFile::instance()->security(price.from()).isCurrency();

    typedef void (KMyMoneyPriceDlg::*KMyMoneyPriceDlgFunc)();
    struct actionInfo {
        eMenu::Action action;
        KMyMoneyPriceDlgFunc callback;
        QString text;
        Icon icon;
        bool enabled;
    };

    // clang-format off
    const QVector<actionInfo> actionInfos {
        {eMenu::Action::NewPrice,    &KMyMoneyPriceDlg::slotNewPrice,          i18n("New price..."),           Icon::DocumentNew,       true},
        {eMenu::Action::EditPrice,   &KMyMoneyPriceDlg::slotEditPrice,         i18n("Edit price..."),          Icon::DocumentEdit,      cond1},
        {eMenu::Action::UpdatePrice, &KMyMoneyPriceDlg::slotOnlinePriceUpdate, i18n("Online Price Update..."), Icon::OnlinePriceUpdate, cond2},
        {eMenu::Action::DeletePrice, &KMyMoneyPriceDlg::slotDeletePrice,       i18n("Delete price..."),        Icon::EditRemove,        cond1},
    };
    // clang-format on

    QList<QAction*> LUTActions;
    for (const auto& info : actionInfos) {
        auto a = new QAction(Icons::get(info.icon), info.text, nullptr); // WARNING: no empty Icon::Empty here
        a->setEnabled(info.enabled);
        connect(a, &QAction::triggered, this, info.callback);
        LUTActions.append(a);
    }
    auto menu = new QMenu;
    menu->addSection(i18nc("@title:menu", "Price options"));
    menu->addActions(LUTActions);
    menu->exec(QCursor::pos());
}

void KMyMoneyPriceDlg::setSearchShortcut(const QKeySequence& shortcut)
{
    Q_D(KMyMoneyPriceDlg);
    d->m_searchShortCut = shortcut;
}

#include "kmymoneypricedlg.moc"
