/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DURATIONWIZARDPAGE_H
#define DURATIONWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui {
class DurationWizardPage;
}

/**
 * This class implements the Duration page of the
 * @ref KNewLoanWizard.
 */

class DurationWizardPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit DurationWizardPage(QWidget *parent = nullptr);
    ~DurationWizardPage();

    QString updateTermWidgets(const double val);
    int term() const;

public Q_SLOTS:
    void resetCalculator();

private:
    Ui::DurationWizardPage *ui;
};

#endif
