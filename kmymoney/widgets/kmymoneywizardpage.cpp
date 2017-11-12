/***************************************************************************
                             kmymoneywizardpage.cpp
                             -------------------
    copyright            : (C) 2006 by Thomas Baumagrt
    email                : ipwizard@users.sourceforge.net
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kmymoneywizardpage.h"
#include "kmymoneywizard_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPushButton>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "kguiutils.h"
#include "kmymoneywizard.h"

KMyMoneyWizardPagePrivate::KMyMoneyWizardPagePrivate(QObject* parent) :
    QObject(parent)
{
}

void KMyMoneyWizardPagePrivate::emitCompleteStateChanged()
{
  emit completeStateChanged();
}


KMyMoneyWizardPage::KMyMoneyWizardPage(unsigned int step, QWidget* widget) :
    m_step(step),
    m_widget(widget),
    d(new KMyMoneyWizardPagePrivate(widget))
{
  m_mandatoryGroup = new KMandatoryFieldGroup(widget);
  QObject::connect(m_mandatoryGroup, SIGNAL(stateChanged()), object(), SIGNAL(completeStateChanged()));
  widget->hide();
}

KMyMoneyWizardPage::~KMyMoneyWizardPage()
{
}

QObject* KMyMoneyWizardPage::object() const
{
  return d;
}

QWidget* KMyMoneyWizardPage::initialFocusWidget() const
{
  return nullptr;
}

void KMyMoneyWizardPage::completeStateChanged() const
{
  d->emitCompleteStateChanged();
}

void KMyMoneyWizardPage::resetPage()
{
}

void KMyMoneyWizardPage::enterPage()
{
}

void KMyMoneyWizardPage::leavePage()
{
}

KMyMoneyWizardPage* KMyMoneyWizardPage::nextPage() const
{
  return 0;
}

bool KMyMoneyWizardPage::isLastPage() const
{
  return nextPage() == 0;
}

bool KMyMoneyWizardPage::isComplete() const
{
  if (!isLastPage())
    wizard()->m_nextButton->setToolTip(i18n("Continue with next page"));
  else
    wizard()->m_finishButton->setToolTip(i18n("Finish wizard"));
  return m_mandatoryGroup->isEnabled();
}

unsigned int KMyMoneyWizardPage::step() const
{
  return m_step;
}

QWidget* KMyMoneyWizardPage::widget() const
{
  return m_widget;
}

QString KMyMoneyWizardPage::helpContext() const
{
  return QString();
}
