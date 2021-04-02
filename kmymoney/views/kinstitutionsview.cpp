/*
    SPDX-FileCopyrightText: 2007 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kinstitutionsview_p.h"

#include <typeinfo>

// ----------------------------------------------------------------------------
// QT Includes

#include <QTimer>
#include <QMenu>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneysettings.h"
#include "mymoneyexception.h"
#include "knewbankdlg.h"
#include "menuenums.h"

using namespace Icons;

KInstitutionsView::KInstitutionsView(QWidget *parent) :
    KMyMoneyAccountsViewBase(*new KInstitutionsViewPrivate(this), parent)
{
    Q_D(KInstitutionsView);
    d->ui->setupUi(this);

    connect(pActions[eMenu::Action::NewInstitution],    &QAction::triggered, this, &KInstitutionsView::slotNewInstitution);
    connect(pActions[eMenu::Action::EditInstitution],   &QAction::triggered, this, &KInstitutionsView::slotEditInstitution);
    connect(pActions[eMenu::Action::DeleteInstitution], &QAction::triggered, this, &KInstitutionsView::slotDeleteInstitution);
}

KInstitutionsView::~KInstitutionsView()
{
}

void KInstitutionsView::executeCustomAction(eView::Action action)
{
    Q_D(KInstitutionsView);
    switch(action) {
    case eView::Action::Refresh:
        refresh();
        break;

    case eView::Action::SetDefaultFocus:
        QTimer::singleShot(0, d->ui->m_accountTree, SLOT(setFocus()));
        break;

    case eView::Action::EditInstitution:
        slotEditInstitution();
        break;

    default:
        break;
    }
}

void KInstitutionsView::refresh()
{
    Q_D(KInstitutionsView);
    if (!isVisible()) {
        d->m_needsRefresh = true;
        return;
    }
    d->m_needsRefresh = false;

    d->m_proxyModel->invalidate();
    d->m_proxyModel->setHideEquityAccounts(!KMyMoneySettings::expertMode());
    d->m_proxyModel->setHideClosedAccounts(KMyMoneySettings::hideClosedAccounts() && !KMyMoneySettings::showAllAccounts());
}

void KInstitutionsView::showEvent(QShowEvent * event)
{
    Q_D(KInstitutionsView);
    if (!d->m_proxyModel)
        d->init();

    emit customActionRequested(View::Institutions, eView::Action::AboutToShow);

    if (d->m_needsRefresh)
        refresh();

    // don't forget base class implementation
    QWidget::showEvent(event);
}

void KInstitutionsView::updateActions(const MyMoneyObject& obj)
{
    Q_D(KInstitutionsView);
    if (typeid(obj) != typeid(MyMoneyInstitution) ||
            (obj.id().isEmpty() && d->m_currentInstitution.id().isEmpty())) // do not disable actions that were already disabled
        return;

    const auto& inst = static_cast<const MyMoneyInstitution&>(obj);

    pActions[eMenu::Action::NewInstitution]->setEnabled(true);
    auto b = inst.id().isEmpty() ? false : true;
    pActions[eMenu::Action::EditInstitution]->setEnabled(b);
    pActions[eMenu::Action::DeleteInstitution]->setEnabled(b && !MyMoneyFile::instance()->isReferenced(inst));
    d->m_currentInstitution = inst;
}

void KInstitutionsView::slotNetWorthChanged(const MyMoneyMoney &netWorth)
{
    Q_D(KInstitutionsView);
    d->netBalProChanged(netWorth, d->ui->m_totalProfitsLabel, View::Institutions);
}

void KInstitutionsView::slotNewInstitution()
{
    Q_D(KInstitutionsView);
    MyMoneyInstitution institution;

    QPointer<KNewBankDlg> dlg = new KNewBankDlg(institution);
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

void KInstitutionsView::slotShowInstitutionsMenu(const MyMoneyInstitution& inst)
{
    Q_UNUSED(inst);
    pMenus[eMenu::Menu::Institution]->exec(QCursor::pos());
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
        QPointer<KNewBankDlg> dlg = new KNewBankDlg(institution);
        if (dlg->exec() == QDialog::Accepted && dlg != 0) {
            MyMoneyFileTransaction ft;
            try {
                file->modifyInstitution(dlg->institution());
                ft.commit();
                emit selectByObject(dlg->institution(), eView::Intent::None);
            } catch (const MyMoneyException &e) {
                KMessageBox::information(this, i18n("Unable to store institution: %1", QString::fromLatin1(e.what())));
            }
        }
        delete dlg;

    } catch (const MyMoneyException &e) {
        KMessageBox::information(this, i18n("Unable to edit institution: %1", QString::fromLatin1(e.what())));
    }
}

void KInstitutionsView::slotSelectByObject(const MyMoneyObject& obj, eView::Intent intent)
{
    switch(intent) {
    case eView::Intent::UpdateActions:
        updateActions(obj);
        break;

    case eView::Intent::OpenContextMenu:
        slotShowInstitutionsMenu(static_cast<const MyMoneyInstitution&>(obj));
        break;

    default:
        break;
    }
}

void KInstitutionsView::slotSelectByVariant(const QVariantList& variant, eView::Intent intent)
{
    switch (intent) {
    case eView::Intent::UpdateNetWorth:
        if (variant.count() == 1)
            slotNetWorthChanged(variant.first().value<MyMoneyMoney>());
        break;
    default:
        break;
    }
}

void KInstitutionsView::slotDeleteInstitution()
{
    Q_D(KInstitutionsView);
    const auto file = MyMoneyFile::instance();
    try {
        auto institution = file->institution(d->m_currentInstitution.id());
        if ((KMessageBox::questionYesNo(this, i18n("<p>Do you really want to delete the institution <b>%1</b>?</p>", institution.name()))) == KMessageBox::No)
            return;
        MyMoneyFileTransaction ft;

        try {
            file->removeInstitution(institution);
            emit selectByObject(MyMoneyInstitution(), eView::Intent::None);
            ft.commit();
        } catch (const MyMoneyException &e) {
            KMessageBox::information(this, i18n("Unable to delete institution: %1", QString::fromLatin1(e.what())));
        }
    } catch (const MyMoneyException &e) {
        KMessageBox::information(this, i18n("Unable to delete institution: %1", QString::fromLatin1(e.what())));
    }
}
