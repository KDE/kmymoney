/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KINVESTMENTDETAILSWIZARDPAGE_H
#define KINVESTMENTDETAILSWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneySecurity;

namespace Ui {
class KInvestmentDetailsWizardPage;
}

/**
 * This class implements the investment details page  of the
 * @ref KNewInvestmentWizard.
 */
class KInvestmentDetailsWizardPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit KInvestmentDetailsWizardPage(QWidget *parent = nullptr);
    ~KInvestmentDetailsWizardPage();

    void init2(const MyMoneySecurity& security);

    /**
     * Overload isComplete to handle the required fields
     */
    bool isComplete() const final override;

    /**
     * Functions to control or read the m_priceMode widget
     */
    int priceMode() const;
    void setCurrentPriceMode(int mode);
    void setPriceModeEnabled(bool enabled);

    /**
     * load or set the name of the m_investmentName item widget. The difference
     * can be seen in the @ref KMyMoneyLineEdit type.
     */
    void loadName(const QString& name);
    void setName(const QString& name);

    void setupInvestmentSymbol();

Q_SIGNALS:
    void checkForExistingSymbol(const QString& symbol);

private:
    Ui::KInvestmentDetailsWizardPage  *ui;
};

#endif
