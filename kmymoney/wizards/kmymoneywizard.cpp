/*
    SPDX-FileCopyrightText: 2006 Thomas Baumagrt <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kmymoneywizard.h"
#include "kmymoneywizard_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QLabel>
#include <QFont>
#include <QList>
#include <QVBoxLayout>
#include <QPushButton>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KHelpClient>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneywizardpage.h"


KMyMoneyWizard::KMyMoneyWizard(QWidget *parent, bool modal, Qt::WindowFlags f) :
    QDialog(parent, f),
    d_ptr(new KMyMoneyWizardPrivate(this))
{
    Q_D(KMyMoneyWizard);
    d->init(modal);
}

KMyMoneyWizard::KMyMoneyWizard(KMyMoneyWizardPrivate &dd, QWidget* parent, bool modal, Qt::WindowFlags f) :
    QDialog(parent, f),
    d_ptr(&dd)
{
    Q_D(KMyMoneyWizard);
    d->init(modal);
}

KMyMoneyWizard::~KMyMoneyWizard()
{
    Q_D(KMyMoneyWizard);
    delete d;
}

void KMyMoneyWizard::setTitle(const QString& txt)
{
    this->setWindowTitle(txt);
}

void KMyMoneyWizard::addStep(const QString& text)
{
    Q_D(KMyMoneyWizard);
    QLabel* step = new QLabel(text, d->m_stepFrame);
    step->setFrameStyle(QFrame::Panel | QFrame::Raised);
    step->setAlignment(Qt::AlignHCenter);
    step->setFrameStyle(QFrame::Box | QFrame::Sunken);
    step->setMargin(2);
    step->setPalette(d->m_stepPalette);

    d->m_steps.append(step);
    d->m_stepLayout->insertWidget(d->m_steps.count(), step);

    QFont font(step->font());
    font.setBold(true);
    QFontMetrics fm(font);
    int w = fm.width(text) + 30;
    if (d->m_stepFrame->minimumWidth() < w) {
        d->m_stepFrame->setMinimumWidth(w);
    }
}

QList<KMyMoneyWizardPage*> KMyMoneyWizard::historyPages() const
{
    Q_D(const KMyMoneyWizard);
    return d->m_history;
}

void KMyMoneyWizard::reselectStep()
{
    Q_D(KMyMoneyWizard);
    d->selectStep(d->m_step);
}

void KMyMoneyWizard::setHelpContext(const QString& ctx)
{
    Q_D(KMyMoneyWizard);
    d->m_helpContext = ctx;
}

void KMyMoneyWizard::backButtonClicked()
{
    Q_D(KMyMoneyWizard);
    KMyMoneyWizardPage* oldPage = d->m_history.back();
    d->m_history.pop_back();
    oldPage->leavePage();
    oldPage->resetPage();
    d->switchPage(oldPage);
}

void KMyMoneyWizard::nextButtonClicked()
{
    Q_D(KMyMoneyWizard);
    // make sure it is really complete. Some widgets only change state during focusOutEvent,
    // so we just create such an animal by changing the focus to the next button and
    // check again for completeness
    d->m_nextButton->setFocus();
    KMyMoneyWizardPage* oldPage = d->m_history.back();
    if (oldPage->isComplete()) {
        KMyMoneyWizardPage* newPage = oldPage->nextPage();
        d->m_history.append(newPage);
        newPage->enterPage();
        newPage->resetPage();
        d->switchPage(oldPage);
    }
}

void KMyMoneyWizard::helpButtonClicked()
{
    Q_D(KMyMoneyWizard);
    KMyMoneyWizardPage* currentPage = d->m_history.back();
    QString ctx = currentPage->helpContext();
    if (ctx.isEmpty())
        ctx = d->m_helpContext;
    KHelpClient::invokeHelp(ctx);
}

void KMyMoneyWizard::completeStateChanged()
{
    Q_D(KMyMoneyWizard);
    KMyMoneyWizardPage* currentPage = d->m_history.back();
    bool lastPage = currentPage->isLastPage();

    d->m_finishButton->setVisible(lastPage);
    d->m_nextButton->setVisible(!lastPage);

    QPushButton* button;

    button = lastPage ? d->m_finishButton : d->m_nextButton;

    auto rc = currentPage->isComplete();
    button->setEnabled(rc);

    d->m_backButton->setEnabled(d->m_history.count() > 1);
}

void KMyMoneyWizard::accept()
{
    Q_D(KMyMoneyWizard);
    // make sure it is really complete. Some widgets only change state during focusOutEvent,
    // so we just create such an animal by changing the focus to the finish button and
    // check again for completeness.
    d->m_finishButton->setFocus();
    KMyMoneyWizardPage* page = d->m_history.back();
    if (page->isComplete())
        QDialog::accept();
}
