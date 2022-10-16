/*
    SPDX-FileCopyrightText: 2007-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2020 Robert Szczesiak <dev.rszczesiak@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kinstitutionsview.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QAction>
#include <QPointer>
#include <QKeyEvent>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kinstitutionsview.h"
#include "kmymoneyviewbase_p.h"
#include "mymoneyfile.h"
#include "mymoneymoney.h"
#include "kmymoneysettings.h"
#include "mymoneyexception.h"
#include "mymoneyinstitution.h"
#include "knewinstitutiondlg.h"
#include "menuenums.h"
#include "accountdelegate.h"
#include "institutionsmodel.h"
#include "accountsproxymodel.h"
#include "institutionsproxymodel.h"
#include "icons.h"
#include "columnselector.h"
#include "kmymoneyaccounttreeview.h"

using namespace Icons;


class KInstitutionsViewPrivate : public KMyMoneyViewBasePrivate
{
    Q_DECLARE_PUBLIC(KInstitutionsView)

public:
    explicit KInstitutionsViewPrivate(KInstitutionsView *qq)
        : KMyMoneyViewBasePrivate(qq)
        , ui(new Ui::KInstitutionsView)
        , m_proxyModel(nullptr)
    {
    }

    ~KInstitutionsViewPrivate()
    {
        delete ui;
    }

    void init()
    {
        Q_Q(KInstitutionsView);
        ui->setupUi(q);

        // setup icons for buttons
        ui->m_collapseButton->setIcon(Icons::get(Icon::ListCollapse));
        ui->m_expandButton->setIcon(Icons::get(Icon::ListExpand));
        // prepare the filter container
        ui->m_closeButton->setIcon(Icons::get(Icon::DialogClose));
        ui->m_filterContainer->hide();
        ui->m_searchWidget->installEventFilter(q);

        ui->m_accountTree->setProxyModel(new InstitutionsProxyModel);
        m_proxyModel = ui->m_accountTree->proxyModel();
        q->connect(ui->m_searchWidget, &QLineEdit::textChanged, m_proxyModel, &QSortFilterProxyModel::setFilterFixedString);

        auto columnSelector = new ColumnSelector(ui->m_accountTree, q->metaObject()->className());
        columnSelector->setAlwaysVisible(QVector<int>({ AccountsModel::Column::AccountName }));
        columnSelector->setAlwaysHidden(QVector<int>({
            AccountsModel::Column::PostedValue,
            AccountsModel::Column::Type,
            AccountsModel::Column::CostCenter,
            AccountsModel::Column::Tax,
            AccountsModel::Column::Vat,
            AccountsModel::Column::Number,
            AccountsModel::Column::HasOnlineMapping,
        }));

        ui->m_accountTree->setModel(MyMoneyFile::instance()->institutionsModel());
        m_proxyModel->addAccountGroup(AccountsProxyModel::assetLiabilityEquity());

        columnSelector->setModel(m_proxyModel);
        q->slotSettingsChanged();

        // forward the widget requests
        q->connect(ui->m_accountTree, &KMyMoneyAccountTreeView::requestCustomContextMenu, q, &KInstitutionsView::requestCustomContextMenu);
        q->connect(ui->m_accountTree, &KMyMoneyAccountTreeView::requestSelectionChange, q, &KInstitutionsView::requestSelectionChange);
        q->connect(ui->m_accountTree, &KMyMoneyAccountTreeView::requestActionTrigger, q, &KInstitutionsView::requestActionTrigger);

        m_focusWidget = ui->m_accountTree;
    }

    Ui::KInstitutionsView   *ui;
    MyMoneyInstitution      m_currentInstitution;
    AccountsProxyModel*     m_proxyModel;
};



KInstitutionsView::KInstitutionsView(QWidget *parent) :
    KMyMoneyViewBase(*new KInstitutionsViewPrivate(this), parent)
{
    Q_D(KInstitutionsView);
    d->init();

    connect(d->ui->m_closeButton, &QToolButton::clicked, this, [&]() {
        Q_D(KInstitutionsView);
        d->ui->m_searchWidget->clear();
        d->ui->m_filterContainer->hide();
    });

    connect(pActions[eMenu::Action::ShowFilterWidget], &QAction::triggered, this, [&]() {
        Q_D(KInstitutionsView);
        // only react if this is the current view
        if (isVisible()) {
            d->ui->m_filterContainer->show();
            d->ui->m_searchWidget->setFocus();
        }
    });

    connect(pActions[eMenu::Action::NewInstitution],    &QAction::triggered, this, &KInstitutionsView::slotNewInstitution);
    connect(pActions[eMenu::Action::EditInstitution],   &QAction::triggered, this, &KInstitutionsView::slotEditInstitution);
    connect(pActions[eMenu::Action::DeleteInstitution], &QAction::triggered, this, &KInstitutionsView::slotDeleteInstitution);

    d->ui->m_accountTree->setItemDelegate(new AccountDelegate(d->ui->m_accountTree));
    connect(MyMoneyFile::instance()->accountsModel(), &AccountsModel::netWorthChanged, this, &KInstitutionsView::slotNetWorthChanged);

    d->m_sharedToolbarActions.insert(eMenu::Action::FileNew, pActions[eMenu::Action::NewInstitution]);
}

KInstitutionsView::~KInstitutionsView()
{
}

bool KInstitutionsView::eventFilter(QObject* watched, QEvent* event)
{
    Q_D(KInstitutionsView);
    if (watched == d->ui->m_searchWidget) {
        if (event->type() == QEvent::KeyPress) {
            const auto kev = static_cast<QKeyEvent*>(event);
            if (kev->modifiers() == Qt::NoModifier && kev->key() == Qt::Key_Escape) {
                d->ui->m_closeButton->animateClick();
                return true;
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}


void KInstitutionsView::slotSettingsChanged()
{
    Q_D(KInstitutionsView);
    d->m_proxyModel->setHideClosedAccounts(!KMyMoneySettings::showAllAccounts());
    d->m_proxyModel->setHideEquityAccounts(!KMyMoneySettings::expertMode());
    d->m_proxyModel->setHideFavoriteAccounts(true);

    MyMoneyFile::instance()->institutionsModel()->setColorScheme(AccountsModel::Positive, KMyMoneySettings::schemeColor(SchemeColor::Positive));
    MyMoneyFile::instance()->institutionsModel()->setColorScheme(AccountsModel::Negative, KMyMoneySettings::schemeColor(SchemeColor::Negative));
}


void KInstitutionsView::updateActions(const SelectedObjects& selections)
{
    Q_D(KInstitutionsView);

    pActions[eMenu::Action::EditInstitution]->setEnabled(false);
    pActions[eMenu::Action::DeleteInstitution]->setEnabled(false);

    // check if there is anything todo and quit if not
    if (selections.selection(SelectedObjects::Institution).count() < 1
            && d->m_currentInstitution.id().isEmpty() ) {
        return;
    }

    const auto ids = selections.selection(SelectedObjects::Institution);
    if (ids.isEmpty()) {
        d->m_currentInstitution = MyMoneyInstitution();
        return;
    }

    const auto file = MyMoneyFile::instance();
    const auto inst = file->institutionsModel()->itemById(ids.at(0));

    auto b = inst.id().isEmpty() ? false : true;
    pActions[eMenu::Action::EditInstitution]->setEnabled(b);
    pActions[eMenu::Action::DeleteInstitution]->setEnabled(b && !MyMoneyFile::instance()->isReferenced(inst));
    d->m_currentInstitution = inst;
}

void KInstitutionsView::slotNetWorthChanged(const MyMoneyMoney &netWorth, bool isApproximate)
{
    Q_D(KInstitutionsView);
    const auto formattedValue = d->formatViewLabelValue(netWorth);
    d->updateViewLabel(d->ui->m_totalProfitsLabel,
                       isApproximate ? i18nc("Approximate net worth", "Net Worth: ~%1", formattedValue)
                       : i18n("Net Worth: %1", formattedValue));
}

void KInstitutionsView::slotNewInstitution()
{
    Q_D(KInstitutionsView);
    MyMoneyInstitution institution;

    QPointer<KNewInstitutionDlg> dlg = new KNewInstitutionDlg(institution);
    if (dlg->exec() == QDialog::Accepted && dlg != 0) {
        d->m_currentInstitution = dlg->institution();

        const auto file = MyMoneyFile::instance();
        MyMoneyFileTransaction ft;

        try {
            file->addInstitution(d->m_currentInstitution);
            ft.commit();

        } catch (const MyMoneyException &e) {
            KMessageBox::information(this, i18n("Cannot add institution: %1", QString::fromLatin1(e.what())));
        }
    }
    delete dlg;
}

void KInstitutionsView::slotEditInstitution()
{
    Q_D(KInstitutionsView);

    // make sure the selected object has an id
    if (d->m_currentInstitution.id().isEmpty())
        return;

    try {
        const auto file = MyMoneyFile::instance();

        //grab a pointer to the view, regardless of it being a account or institution view.
        auto institution = file->institution(d->m_currentInstitution.id());

        // bankSuccess is not checked anymore because d->m_file->institution will throw anyway
        QPointer<KNewInstitutionDlg> dlg = new KNewInstitutionDlg(institution);
        if (dlg->exec() == QDialog::Accepted && dlg != 0) {
            MyMoneyFileTransaction ft;
            try {
                file->modifyInstitution(dlg->institution());
                ft.commit();
            } catch (const MyMoneyException &e) {
                KMessageBox::information(this, i18n("Unable to store institution: %1", QString::fromLatin1(e.what())));
            }
        }
        delete dlg;

    } catch (const MyMoneyException &e) {
        KMessageBox::information(this, i18n("Unable to edit institution: %1", QString::fromLatin1(e.what())));
    }
}

void KInstitutionsView::slotDeleteInstitution()
{
    Q_D(KInstitutionsView);
    const auto file = MyMoneyFile::instance();
    try {
        auto institution = file->institution(d->m_currentInstitution.id());
        if ((KMessageBox::questionTwoActions(this,
                                             i18n("<p>Do you really want to delete the institution <b>%1</b>?</p>", institution.name()),
                                             i18nc("@title:window", "Delete institution"),
                                             KStandardGuiItem::yes(),
                                             KStandardGuiItem::no()))
            == KMessageBox::SecondaryAction)
            return;
        MyMoneyFileTransaction ft;

        try {
            file->removeInstitution(institution);
            ft.commit();
        } catch (const MyMoneyException &e) {
            KMessageBox::information(this, i18n("Unable to delete institution: %1", QString::fromLatin1(e.what())));
        }
    } catch (const MyMoneyException &e) {
        KMessageBox::information(this, i18n("Unable to delete institution: %1", QString::fromLatin1(e.what())));
    }
}
