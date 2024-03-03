/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PAYMENTFREQUENCYWIZARDPAGE_H
#define PAYMENTFREQUENCYWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui {
class PaymentFrequencyWizardPage;
}

class PaymentFrequencyWizardPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit PaymentFrequencyWizardPage(QWidget *parent = nullptr);
    ~PaymentFrequencyWizardPage();

    Ui::PaymentFrequencyWizardPage *ui;
};

#endif
