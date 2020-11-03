/*
 * Copyright 2007-2019  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 * Copyright 2020       Robert Szczesiak <dev.rszczesiak@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "kinstitutionsview_p.h"

#include <typeinfo>

// ----------------------------------------------------------------------------
// QT Includes

#include <QMenu>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "mymoneymoney.h"
#include "kmymoneysettings.h"
#include "mymoneyexception.h"
#include "knewinstitutiondlg.h"
#include "menuenums.h"
#include "accountdelegate.h"

using namespace Icons;

KInstitutionsView::KInstitutionsView(QWidget *parent) :
    KMyMoneyViewBase(*new KInstitutionsViewPrivate(this), parent)
{
  Q_D(KInstitutionsView);
  d->init();

  connect(pActions[eMenu::Action::NewInstitution],    &QAction::triggered, this, &KInstitutionsView::slotNewInstitution);
  connect(pActions[eMenu::Action::EditInstitution],   &QAction::triggered, this, &KInstitutionsView::slotEditInstitution);
  connect(pActions[eMenu::Action::DeleteInstitution], &QAction::triggered, this, &KInstitutionsView::slotDeleteInstitution);

  d->ui->m_accountTree->setItemDelegate(new AccountDelegate(d->ui->m_accountTree));
  connect(MyMoneyFile::instance()->accountsModel(), &AccountsModel::netWorthChanged, this, &KInstitutionsView::slotNetWorthChanged);
}

KInstitutionsView::~KInstitutionsView()
{
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


void KInstitutionsView::executeCustomAction(eView::Action action)
{
  switch(action) {
    case eView::Action::Refresh:
      refresh();
      break;

    case eView::Action::SetDefaultFocus:
      {
        Q_D(KInstitutionsView);
        QMetaObject::invokeMethod(d->ui->m_accountTree, "setFocus", Qt::QueuedConnection);
      }
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

/// @todo port to new model code or cleanup
#if 0
  d->m_proxyModel->setHideEquityAccounts(!KMyMoneySettings::expertMode());
  d->m_proxyModel->setHideClosedAccounts(KMyMoneySettings::hideClosedAccounts() && !KMyMoneySettings::showAllAccounts());
#endif
}

void KInstitutionsView::showEvent(QShowEvent * event)
{
  /// @todo port to new model code or cleanup
#if 0
  Q_D(KInstitutionsView);
  if (!d->m_proxyModel)
    d->init();

  emit customActionRequested(View::Institutions, eView::Action::AboutToShow);

  if (d->m_needsRefresh)
    refresh();
#endif

  // don't forget base class implementation
  QWidget::showEvent(event);
}

void KInstitutionsView::updateActions(const SelectedObjects& selections)
{
  /// @todo updateActions
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
      if (MyMoneyFile::instance()->accountsModel()->indexById(obj.id()).isValid()) {
        pMenus[eMenu::Menu::Account]->exec(QCursor::pos());

      } else if (MyMoneyFile::instance()->institutionsModel()->indexById(obj.id()).isValid()) {
        pMenus[eMenu::Menu::Institution]->exec(QCursor::pos());
      }
      break;

    default:
      break;
  }
}

void KInstitutionsView::slotSelectByVariant(const QVariantList& variant, eView::Intent intent)
{
  /// @todo cleanup
#if 0
  switch (intent) {
    case eView::Intent::UpdateNetWorth:
      if (variant.count() == 1)
        slotNetWorthChanged(variant.first().value<MyMoneyMoney>());
      break;
    default:
      break;
  }
#endif
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
