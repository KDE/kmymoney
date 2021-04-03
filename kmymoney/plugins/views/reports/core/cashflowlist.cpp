/*
    SPDX-FileCopyrightText: 2007 Sascha Pfau <MrPeacock@gmail.com>
    SPDX-FileCopyrightText: 2018 Ralf Habacker <ralf.habacker@freenet.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

/*
 * This file contains code from the func_xirr and related methods of
 * financial.cpp from KOffice by Sascha Pfau. Sascha agreed to relicense
 * those methods under GPLv2 or later.
 */

#include "cashflowlist.h"

#include "mymoneyexception.h"

#include <cmath>
#include <math.h>

#include <QtDebug>

/**
 * Calculates the internal rate of return for a non-periodic
 * series of cash flows. The calculation is based on a 365 days
 * per year basis, ignoring leap years.
 * In case the internal rate of return could not be calculated
 * a MyMoneyException is raised.
 *
 * @param rate optional guess for rate
 * @return internal rate of return
 */
double CashFlowList::XIRR(double rate) const
{
    if (size() < 2)
        throw MYMONEYEXCEPTION("illegal argument exception");

    double resultRate = rate;

    // define max epsilon
    static const double maxEpsilon = 1e-10;

    // max number of iterations
    static const int maxIter = 50;

    // Newton's method - try to find a res, with a accuracy of maxEpsilon
    double newRate, rateEpsilon, resultValue;
    int iter = 0;
    bool contLoop = false;
    int iterScan = 0;
    bool resultRateScanEnd = false;

    // First the inner while-loop will be executed using the default Value resultRate
    // or the user guessed resultRate if those do not deliver a solution for the
    // Newton's method then the range from -0.99 to +0.99 will be scanned with a
    // step size of 0.01 to find resultRate's value which can deliver a solution
    // source hint:
    // - outer loop from libreoffice
    // - inner loop from KOffice
    do {
        if (iterScan >=1)
            resultRate = -0.99 + (iterScan -1)* 0.01;

        do {
            resultValue = xirrResult(resultRate);
            newRate =  resultRate - resultValue / xirrResultDerive(resultRate);
            rateEpsilon = fabs(newRate - resultRate);
            resultRate = newRate;
            contLoop = (rateEpsilon > maxEpsilon) && (fabs(resultValue) > maxEpsilon);
        } while (contLoop && (++iter < maxIter));
        iter = 0;
        if (std::isinf(resultRate) || std::isnan(resultRate) ||
                std::isinf(resultValue) || std::isnan(resultValue))
            contLoop = true;
        iterScan++;
        resultRateScanEnd = (iterScan >= 200);
    } while(contLoop && !resultRateScanEnd);

    if (contLoop)
        throw MYMONEYEXCEPTION("illegal argument exception");
    return resultRate;
}

/**
 * Calculates the resulting amount for the passed interest rate
 *
 * @param rate interest rate
 * @return resulting amount
 */
double CashFlowList::xirrResult(double rate) const
{
    double r = rate + 1.0;
    double result = at(0).value().toDouble();
    const QDate &date0 = at(0).date();

    for(int i = 1; i < size(); i++) {
        double e_i = date0.daysTo(at(i).date()) / 365.0;
        result += at(i).value().toDouble() / pow(r, e_i);
    }
    return result;
}

/**
 * Calculates the first derivation of the resulting amount
 *
 * @param rate interest rate
 * @return first derivation of resulting amount
 */
double CashFlowList::xirrResultDerive(double rate) const
{
    double r = rate + 1.0;
    double result = 0;
    const QDate &date0 = at(0).date();

    for(int i = 1; i < size(); i++) {
        double e_i = date0.daysTo(at(i).date()) / 365.0;
        result -= e_i * at(i).value().toDouble() / pow(r, e_i + 1.0);
    }
    return result;
}

/**
 * Return the sum of all payments
 *
 * @return sum of all payments
 */
MyMoneyMoney CashFlowList::total() const
{
    MyMoneyMoney result;

    const_iterator it_cash = begin();
    while (it_cash != end()) {
        result += (*it_cash).value();
        ++it_cash;
    }

    return result;
}

/**
 * dump all payments
 */
void CashFlowList::dumpDebug() const
{
    const_iterator it_item = begin();
    while (it_item != end()) {
        qDebug() << (*it_item).date().toString(Qt::ISODate) << " " << (*it_item).value().toString();
        ++it_item;
    }
}
