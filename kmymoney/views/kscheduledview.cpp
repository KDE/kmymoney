/*
    SPDX-FileCopyrightText: 2000-2002 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 Javier Campos Morales <javi_c@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 Felix Rodriguez <frodriguez@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 John C <thetacoturtle@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kscheduledview_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QMenu>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "keditloanwizard.h"
#include "kmymoneyutils.h"
#include "kmymoneysettings.h"
#include "mymoneyexception.h"
#include "keditscheduledlg.h"

KScheduledView::KScheduledView(QWidget *parent) :
    KMyMoneyViewBase(*new KScheduledViewPrivate(this), parent)
{
    typedef void(KScheduledView::*KScheduledViewFunc)();
    // clang-format off
    const QHash<eMenu::Action, KScheduledViewFunc> actionConnections {
        {eMenu::Action::NewSchedule,        &KScheduledView::slotNewSchedule},
        {eMenu::Action::EditSchedule,       &KScheduledView::slotEditSchedule},
        {eMenu::Action::EditScheduleForce,  &KScheduledView::slotEditLoanSchedule},
        {eMenu::Action::DeleteSchedule,     &KScheduledView::slotDeleteSchedule},
        {eMenu::Action::DuplicateSchedule,  &KScheduledView::slotDuplicateSchedule},
        {eMenu::Action::EnterSchedule,      &KScheduledView::slotEnterSchedule},
        {eMenu::Action::SkipSchedule,       &KScheduledView::slotSkipSchedule},
    };
    // clang-format on

    for (auto a = actionConnections.cbegin(); a != actionConnections.cend(); ++a)
        connect(pActions[a.key()], &QAction::triggered, this, a.value());

    Q_D(KScheduledView);
    d->m_balanceWarning.reset(new KBalanceWarning(this));

    d->m_sharedToolbarActions.insert(eMenu::Action::FileNew, pActions[eMenu::Action::NewSchedule]);
}

KScheduledView::~KScheduledView()
{
}

void KScheduledView::slotSettingsChanged()
{
    Q_D(KScheduledView);
    d->settingsChanged();
}

void KScheduledView::showEvent(QShowEvent* event)
{
    Q_D(KScheduledView);
    if (d->m_needLoad) {
        d->init();
        connect(d->ui->m_scheduleTree, &QWidget::customContextMenuRequested, this, [&](const QPoint& pos) {
            Q_D(KScheduledView);
            Q_EMIT requestCustomContextMenu(eMenu::Menu::Schedule, d->ui->m_scheduleTree->viewport()->mapToGlobal(pos));
        });
        connect(d->ui->m_scheduleTree, &KMyMoneyTreeView::startEdit, this, &KScheduledView::slotEditSchedule);

        connect(d->ui->m_scheduleTree->header(), &QHeaderView::sortIndicatorChanged, this, [&](int logicalIndex, Qt::SortOrder order) {
            Q_D(KScheduledView);
            d->m_filterModel->sort(logicalIndex, order);
        });
        connect(MyMoneyFile::instance()->schedulesModel(), &SchedulesModel::dataChanged, this, [&](const QModelIndex& from, const QModelIndex& to) {
            Q_UNUSED(from)
            Q_UNUSED(to)
            Q_D(KScheduledView);
            d->m_filterModel->invalidate();
        });
    }
    QWidget::showEvent(event);
}

void KScheduledView::updateActions(const SelectedObjects& selections)
{
    const QVector<eMenu::Action> actionsToBeDisabled {
        eMenu::Action::EditSchedule, eMenu::Action::DuplicateSchedule, eMenu::Action::DeleteSchedule,
        eMenu::Action::EnterSchedule, eMenu::Action::SkipSchedule,
    };

    for (const auto& a : actionsToBeDisabled)
        pActions[a]->setEnabled(false);

    MyMoneySchedule sch;
    if (!selections.selection(SelectedObjects::Schedule).isEmpty()) {
        sch = MyMoneyFile::instance()->schedulesModel()->itemById(selections.selection(SelectedObjects::Schedule).at(0));
        if (!sch.id().isEmpty()) {
            pActions[eMenu::Action::EditSchedule]->setEnabled(true);
            pActions[eMenu::Action::DuplicateSchedule]->setEnabled(true);
            pActions[eMenu::Action::DeleteSchedule]->setEnabled(!MyMoneyFile::instance()->isReferenced(sch));
            if (!sch.isFinished()) {
                pActions[eMenu::Action::EnterSchedule]->setEnabled(true);
                // a schedule with a single occurrence cannot be skipped
                if (sch.occurrence() != eMyMoney::Schedule::Occurrence::Once) {
                    pActions[eMenu::Action::SkipSchedule]->setEnabled(true);
                }
            }
        }
    }
}

eDialogs::ScheduleResultCode KScheduledView::enterSchedule(MyMoneySchedule& schedule, bool autoEnter, bool extendedKeys)
{
    Q_D(KScheduledView);
    return d->enterSchedule(schedule, autoEnter, extendedKeys);
}

void KScheduledView::slotListViewExpanded(const QModelIndex& idx)
{
    Q_D(KScheduledView);
    // we only need to check the top level items
    if (!idx.parent().isValid()) {
        const auto groupType = static_cast<eMyMoney::Schedule::Type>(idx.data(eMyMoney::Model::ScheduleTypeRole).toInt());
        d->m_expandedGroups[groupType] = true;
    }
}

void KScheduledView::slotListViewCollapsed(const QModelIndex& idx)
{
    Q_D(KScheduledView);
    // we only need to check the top level items
    if (!idx.parent().isValid()) {
        const auto groupType = static_cast<eMyMoney::Schedule::Type>(idx.data(eMyMoney::Model::ScheduleTypeRole).toInt());
        d->m_expandedGroups[groupType] = false;
    }
}

void KScheduledView::slotSetSelectedItem(const QItemSelection& selected, const QItemSelection& deselected )
{
    Q_UNUSED(deselected)
    SelectedObjects selections;

    if (!selected.isEmpty()) {
        QModelIndexList idxList = selected.indexes();
        if (!idxList.isEmpty()) {
            const auto objId = selected.indexes().front().data(eMyMoney::Model::IdRole).toString();
            selections.addSelection(SelectedObjects::Schedule, objId);
        }
    }
    Q_EMIT requestSelectionChange(selections);
}

void KScheduledView::slotNewSchedule()
{
    KEditScheduleDlg::newSchedule(MyMoneyTransaction(), eMyMoney::Schedule::Occurrence::Monthly);
}

void KScheduledView::slotEditLoanSchedule()
{
    Q_D(KScheduledView);
    const auto schedule = d->selectedSchedule(pActions[eMenu::Action::EditSchedule]);
    try {
        KEditScheduleDlg::editSchedule(schedule, true);
    } catch (const MyMoneyException& e) {
        KMessageBox::detailedError(this, i18n("Unknown scheduled transaction '%1'", schedule.name()), QString::fromLatin1(e.what()));
    }
}

void KScheduledView::slotEditSchedule()
{
    Q_D(KScheduledView);
    const auto schedule = d->selectedSchedule(pActions[eMenu::Action::EditSchedule]);
    try {
        KEditScheduleDlg::editSchedule(schedule);
    } catch (const MyMoneyException& e) {
        KMessageBox::detailedError(this, i18n("Unknown scheduled transaction '%1'", schedule.name()), QString::fromLatin1(e.what()));
    }
}

void KScheduledView::slotDeleteSchedule()
{
    Q_D(KScheduledView);
    const auto schedule = d->selectedSchedule(pActions[eMenu::Action::DeleteSchedule]);
    if (!schedule.id().isEmpty()) {
        MyMoneyFileTransaction ft;
        try {
            QString msg = i18n("<p>Are you sure you want to delete the scheduled transaction <b>%1</b>?</p>", schedule.name());
            if (schedule.type() == eMyMoney::Schedule::Type::LoanPayment) {
                msg += QString(" ");
                msg += i18n("In case of loan payments it is currently not possible to recreate the scheduled transaction.");
            }
            if (KMessageBox::questionTwoActions(parentWidget(), msg, i18nc("@title:window", "Delete schedule"), KMMYesNo::yes(), KMMYesNo::no())
                == KMessageBox::SecondaryAction)
                return;

            MyMoneyFile::instance()->removeSchedule(schedule);
            ft.commit();

        } catch (const MyMoneyException &e) {
            KMessageBox::detailedError(this, i18n("Unable to remove scheduled transaction '%1'", schedule.name()), QString::fromLatin1(e.what()));
        }
    }
}

void KScheduledView::slotDuplicateSchedule()
{
    Q_D(KScheduledView);
    // since we may jump here via code, we have to make sure to react only
    // if the action is enabled
    MyMoneySchedule schedule = d->selectedSchedule(pActions[eMenu::Action::DuplicateSchedule]);
    if (pActions[eMenu::Action::DuplicateSchedule]->isEnabled() && !schedule.id().isEmpty()) {
        schedule.clearId();
        schedule.setLastPayment(QDate());
        schedule.setName(i18nc("Copy of scheduled transaction name", "Copy of %1", schedule.name()));
        // make sure that we set a valid next due date if the original next due date is invalid
        if (!schedule.nextDueDate().isValid())
            schedule.setNextDueDate(QDate::currentDate());

        MyMoneyFileTransaction ft;
        try {
            MyMoneyFile::instance()->addSchedule(schedule);
            ft.commit();

            // the filterModel might not be initialized if we get here
            // by the ledger view's context menu
            if (d->m_filterModel) {
                // select the new schedule in the view
                const auto indexes = d->m_filterModel->match(d->m_filterModel->index(0, 0),
                                                             eMyMoney::Model::IdRole,
                                                             QVariant(schedule.id()),
                                                             1,
                                                             Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive | Qt::MatchRecursive));
                if (!indexes.isEmpty()) {
                    d->ui->m_scheduleTree->setCurrentIndex(indexes.at(0));
                }
            }

        } catch (const MyMoneyException &e) {
            KMessageBox::detailedError(this, i18n("Unable to duplicate scheduled transaction: '%1'", schedule.name()), QString::fromLatin1(e.what()));
        }
    }
}

void KScheduledView::slotEnterSchedule()
{
    Q_D(KScheduledView);

    auto schedule = d->selectedSchedule(pActions[eMenu::Action::EnterSchedule]);
    if (!schedule.id().isEmpty()) {
        try {
            d->enterSchedule(schedule);
        } catch (const MyMoneyException& e) {
            KMessageBox::detailedError(this, i18n("Unknown scheduled transaction '%1'", schedule.name()), QString::fromLatin1(e.what()));
        }
    }
}

void KScheduledView::slotSkipSchedule()
{
    Q_D(KScheduledView);
    auto schedule = d->selectedSchedule(pActions[eMenu::Action::SkipSchedule]);
    if (!schedule.id().isEmpty()) {
        try {
            d->skipSchedule(schedule);
        } catch (const MyMoneyException& e) {
            KMessageBox::detailedError(this, i18n("Unknown scheduled transaction '%1'", schedule.name()), QString::fromLatin1(e.what()));
        }
    }
}
