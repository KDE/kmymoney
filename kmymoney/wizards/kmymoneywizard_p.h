/*
    SPDX-FileCopyrightText: 2006 Thomas Baumagrt <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMYMONEYWIZARD_P_H
#define KMYMONEYWIZARD_P_H

#include "kmymoneywizard.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QLabel>
#include <QFont>
#include <QHBoxLayout>
#include <QList>
#include <QVBoxLayout>
#include <QPushButton>
#include <QIcon>
#include <QStyle>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KStandardGuiItem>
#include <KColorScheme>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneywizardpage.h"
#include "kmymoneywizardpage_p.h"
#include "icons/icons.h"

using namespace Icons;

class KMyMoneyWizardPrivate
{
    Q_DISABLE_COPY(KMyMoneyWizardPrivate)
    Q_DECLARE_PUBLIC(KMyMoneyWizard)

public:
    explicit KMyMoneyWizardPrivate(KMyMoneyWizard* qq)
        : q_ptr(qq)
        , m_cancelButton(nullptr)
        , m_backButton(nullptr)
        , m_nextButton(nullptr)
        , m_finishButton(nullptr)
        , m_helpButton(nullptr)
        , m_wizardLayout(nullptr)
        , m_stepLayout(nullptr)
        , m_pageLayout(nullptr)
        , m_buttonLayout(nullptr)
        , m_stepFrame(nullptr)
        , m_stepLabel(nullptr)
        , m_step(0)
    {
    }

    virtual ~KMyMoneyWizardPrivate()
    {
    }

    void init(bool modal)
    {
        Q_Q(KMyMoneyWizard);
        q->setModal(modal);

        // enable the little grip in the right corner
        q->setSizeGripEnabled(true);

        // create buttons
        m_cancelButton = new QPushButton(i18n("&Cancel"), q);
        m_backButton = new QPushButton(i18nc("Go to previous page of the wizard", "&Back"), q);
        m_nextButton = new QPushButton(i18nc("Go to next page of the wizard", "&Next"), q);
        m_finishButton = new QPushButton(i18nc("Finish the wizard", "&Finish"), q);
        m_helpButton = new QPushButton(i18n("&Help"), q);

        if (q->style()->styleHint(QStyle::SH_DialogButtonBox_ButtonsHaveIcons, 0, q)) {
            m_backButton->setIcon(KStandardGuiItem::back(KStandardGuiItem::UseRTL).icon());
            m_nextButton->setIcon(KStandardGuiItem::forward(KStandardGuiItem::UseRTL).icon());
            m_finishButton->setIcon(Icons::get(Icon::DialogOKApply));
            m_cancelButton->setIcon(Icons::get(Icon::DialogCancel));
            m_helpButton->setIcon(Icons::get(Icon::Help));
        }

        // create button layout
        m_buttonLayout = new QHBoxLayout;
        m_buttonLayout->addWidget(m_helpButton);
        m_buttonLayout->addStretch(1);
        m_buttonLayout->addWidget(m_backButton);
        m_buttonLayout->addWidget(m_nextButton);
        m_buttonLayout->addWidget(m_finishButton);
        m_buttonLayout->addWidget(m_cancelButton);

        // create wizard layout
        m_wizardLayout = new QVBoxLayout(q);
        m_wizardLayout->setContentsMargins(6, 6, 6, 6);
        m_wizardLayout->setSpacing(0);
        m_wizardLayout->setObjectName("wizardLayout");

        QHBoxLayout* hboxLayout = new QHBoxLayout;
        hboxLayout->setContentsMargins(0, 0, 0, 0);
        hboxLayout->setSpacing(6);
        hboxLayout->setObjectName("hboxLayout");

        // create stage layout and frame
        m_stepFrame = new QFrame(q);
        m_stepFrame->setObjectName("stepFrame");
        QPalette palette = m_stepFrame->palette();
        palette.setColor(m_stepFrame->backgroundRole(), KColorScheme::NormalText);
        m_stepFrame->setPalette(palette);
        m_stepLayout = new QVBoxLayout(m_stepFrame);
        m_stepLayout->setContentsMargins(11, 11, 11, 11);
        m_stepLayout->setSpacing(6);
        m_stepLayout->setObjectName("stepLayout");
        m_stepLayout->addWidget(new QLabel(QString(), m_stepFrame));
        m_stepLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));
        m_stepLabel = new QLabel(m_stepFrame);
        m_stepLabel->setAlignment(Qt::AlignHCenter);
        m_stepLayout->addWidget(m_stepLabel);
        hboxLayout->addWidget(m_stepFrame);

        m_stepPalette = m_stepLabel->palette();

        // add a vertical line between the stepFrame and the pages
        QFrame* line = new QFrame(q);
        line->setObjectName("line");
        line->setFrameShadow(QFrame::Sunken);
        line->setFrameShape(QFrame::VLine);
        hboxLayout->addWidget(line);

        // create page layout
        m_pageLayout = new QVBoxLayout;
        m_pageLayout->setContentsMargins(0, 0, 0, 0);
        m_pageLayout->setSpacing(6);
        m_pageLayout->setObjectName("pageLayout");

        // the page will be inserted later dynamically above q line
        line = new QFrame(q);
        line->setObjectName("line");
        line->setFrameShadow(QFrame::Sunken);
        line->setFrameShape(QFrame::HLine);
        m_pageLayout->addWidget(line);
        m_pageLayout->addLayout(m_buttonLayout);

        // now glue everything together
        hboxLayout->addLayout(m_pageLayout);
        m_wizardLayout->addLayout(hboxLayout);

        q->resize(QSize(670, 550).expandedTo(q->minimumSizeHint()));

        m_finishButton->hide();

        q->connect(m_backButton, &QAbstractButton::clicked, q, &KMyMoneyWizard::backButtonClicked);
        q->connect(m_nextButton, &QAbstractButton::clicked, q, &KMyMoneyWizard::nextButtonClicked);
        q->connect(m_cancelButton, &QAbstractButton::clicked, q, &QDialog::reject);
        q->connect(m_finishButton, &QAbstractButton::clicked, q, &KMyMoneyWizard::accept);
        q->connect(m_helpButton, &QAbstractButton::clicked, q, &KMyMoneyWizard::helpButtonClicked);
    }

    /**
      * Switch to page which is currently the top of the history stack.
      * @p oldPage is a pointer to the current page or 0 if no page
      * is shown.
      *
      * @param oldPage pointer to currently displayed page
      */
    void switchPage(KMyMoneyWizardPage* oldPage)
    {
        Q_Q(KMyMoneyWizard);
        if (oldPage) {
            oldPage->widget()->hide();
            m_pageLayout->removeWidget(oldPage->widget());
            q->disconnect(oldPage->object(), SIGNAL(completeStateChanged()), q, SLOT(completeStateChanged()));
        }
        KMyMoneyWizardPage* newPage = m_history.back();
        if (newPage) {
            m_pageLayout->insertWidget(0, newPage->widget());
            q->connect(newPage->object(), SIGNAL(completeStateChanged()), q, SLOT(completeStateChanged()));
            newPage->widget()->show();
            selectStep(newPage->step());
            if (newPage->isLastPage()) {
                m_nextButton->setDefault(false);
                m_finishButton->setDefault(true);
            } else {
                m_finishButton->setDefault(false);
                m_nextButton->setDefault(true);
            }
            QWidget* w = newPage->initialFocusWidget();
            if (w)
                w->setFocus();
        }
        q->completeStateChanged();
    }

    /**
      * This method selects the step given by @p step.
      *
      * @param step step to be selected
      */
    void selectStep(int step)
    {
        if ((step < 1) || (step > m_steps.count()))
            return;

        m_step = step;
        QList<QLabel*>::iterator it_l;
        QFont f = m_steps[0]->font();
        for (it_l = m_steps.begin(); it_l != m_steps.end(); ++it_l) {
            f.setBold(false);
            (*it_l)->setFrameStyle(QFrame::NoFrame);
            if (--step == 0) {
                f.setBold(true);
                (*it_l)->setFrameStyle(QFrame::Box | QFrame::Sunken);
            }
            (*it_l)->setFont(f);
        }
        updateStepCount();
    }

    /**
      * This method sets up the first page after creation of the object
      *
      * @param page pointer to first page of wizard
      */
    void setFirstPage(KMyMoneyWizardPage* page)
    {
        page->resetPage();
        m_history.clear();
        m_history.append(page);
        switchPage(0);
    }

    /**
      * This method allows to hide or show a @p step.
      *
      * @param step step to be shown/hidden
      * @param hidden hide step if true (the default) or show it if false
      */
    void setStepHidden(int step, bool hidden = true)
    {
        if ((step < 1) || (step > m_steps.count()))
            return;

        m_steps[--step]->setHidden(hidden);
        updateStepCount();
    }

    void updateStepCount()
    {
        QList<QLabel*>::iterator it_l;
        int stepCount = 0;
        int hiddenAdjust = 0;
        int step = 0;
        for (it_l = m_steps.begin(); it_l != m_steps.end(); ++it_l) {
            if (!(*it_l)->isHidden())
                ++stepCount;
            else if (step < m_step)
                hiddenAdjust++;
            ++step;
        }
        m_stepLabel->setText(i18n("Step %1 of %2", (m_step - hiddenAdjust), stepCount));
    }

    KMyMoneyWizard       *q_ptr;

    /*
     * The buttons
     */
    QPushButton*          m_cancelButton;
    QPushButton*          m_backButton;
    QPushButton*          m_nextButton;
    QPushButton*          m_finishButton;
    QPushButton*          m_helpButton;

    /*
     * The layouts
     */
    QVBoxLayout*          m_wizardLayout;
    QVBoxLayout*          m_stepLayout;
    QVBoxLayout*          m_pageLayout;
    QHBoxLayout*          m_buttonLayout;

    /*
     * Some misc. widgets required
     */
    QFrame*               m_stepFrame;
    QLabel*               m_stepLabel;
    QPalette              m_stepPalette;

    QList<QLabel*>        m_steps;      // the list of step labels
    int                   m_step;       // the currently selected step

    /*
     * The history stack
     */
    QList<KMyMoneyWizardPage*> m_history;

    QString               m_helpContext;
};

#endif
