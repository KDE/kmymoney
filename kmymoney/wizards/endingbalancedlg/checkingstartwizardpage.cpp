/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "checkingstartwizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QCheckBox>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KConfigGroup>
#include <KSharedConfig>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_checkingstartwizardpage.h"

CheckingStartWizardPage::CheckingStartWizardPage(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::CheckingStartWizardPage)
{
    ui->setupUi(this);
    // Register the fields with the QWizard and connect the
    // appropriate signals to update the "Next" button correctly
    registerField(QLatin1String("skipIntroPage"), ui->m_skipIntroPage);
    connect(ui->m_skipIntroPage, &QCheckBox::toggled, this, [&](bool checked) {
        // we keep the setting with the notification messages so that
        // the "Enable all messages" function turns the intro page back on
        KSharedConfigPtr kconfig = KSharedConfig::openConfig();
        if (kconfig) {
            kconfig->group(QLatin1String("Notification Messages")).writeEntry(QLatin1String("SkipReconciliationIntro"), checked);
        }
    });
}

CheckingStartWizardPage::~CheckingStartWizardPage()
{
    delete ui;
}

/*
 * Providing an empty cleanupPage prevents resetting
 * the checkbox to unchecked when the dialog is cancelled
 */
void CheckingStartWizardPage::cleanupPage()
{
}
