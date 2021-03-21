/*
    SPDX-FileCopyrightText: 2003-2012 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneyfinancialcalculator.h"
#include "mymoneyfinancialcalculator_p.h"

#include <qglobal.h>

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyexception.h"

static inline double dabs(const double x)
{
    return (x >= 0.0) ? x : -x;
}

MyMoneyFinancialCalculator::MyMoneyFinancialCalculator() :
    d_ptr(new MyMoneyFinancialCalculatorPrivate)
{
    Q_D(MyMoneyFinancialCalculator);
    setPrec();
    setPF();
    setCF();
    setBep();
    setDisc();

    setNpp(0.0);
    setIr(0.0);
    setPv(0.0);
    setPmt(0.0);
    setFv(0.0);

    // clear the mask
    d->m_mask = 0;
}

MyMoneyFinancialCalculator::~MyMoneyFinancialCalculator()
{
    Q_D(MyMoneyFinancialCalculator);
    delete d;
}

void MyMoneyFinancialCalculator::setPrec(const unsigned short prec)
{
    Q_D(MyMoneyFinancialCalculator);
    d->m_prec = prec;
}

void MyMoneyFinancialCalculator::setPF(const unsigned short PF)
{
    Q_D(MyMoneyFinancialCalculator);
    d->m_PF = PF;
}

void MyMoneyFinancialCalculator::setCF(const unsigned short CF)
{
    Q_D(MyMoneyFinancialCalculator);
    d->m_CF = CF;
}

void MyMoneyFinancialCalculator::setBep(const bool bep)
{
    Q_D(MyMoneyFinancialCalculator);
    d->m_bep = bep;
}

void MyMoneyFinancialCalculator::setDisc(const bool disc)
{
    Q_D(MyMoneyFinancialCalculator);
    d->m_disc = disc;
}

void MyMoneyFinancialCalculator::setIr(const double ir)
{
    Q_D(MyMoneyFinancialCalculator);
    d->m_ir = ir;
    d->m_mask |= IR_SET;
}

double MyMoneyFinancialCalculator::ir() const
{
    Q_D(const MyMoneyFinancialCalculator);
    return d->m_ir;
}

void MyMoneyFinancialCalculator::setPv(const double pv)
{
    Q_D(MyMoneyFinancialCalculator);
    d->m_pv = pv;
    d->m_mask |= PV_SET;
}

double MyMoneyFinancialCalculator::pv() const
{
    Q_D(const MyMoneyFinancialCalculator);
    return d->m_pv;
}

void MyMoneyFinancialCalculator::setPmt(const double pmt)
{
    Q_D(MyMoneyFinancialCalculator);
    d->m_pmt = pmt;
    d->m_mask |= PMT_SET;
}

double MyMoneyFinancialCalculator::pmt() const
{
    Q_D(const MyMoneyFinancialCalculator);
    return d->m_pmt;
}

void MyMoneyFinancialCalculator::setNpp(const double npp)
{
    Q_D(MyMoneyFinancialCalculator);
    d->m_npp = npp;
    d->m_mask |= NPP_SET;
}

double MyMoneyFinancialCalculator::npp() const
{
    Q_D(const MyMoneyFinancialCalculator);
    return d->m_npp;
}

void MyMoneyFinancialCalculator::setFv(const double fv)
{
    Q_D(MyMoneyFinancialCalculator);
    d->m_fv = fv;
    d->m_mask |= FV_SET;
}

double MyMoneyFinancialCalculator::fv() const
{
    Q_D(const MyMoneyFinancialCalculator);
    return d->m_fv;
}

double MyMoneyFinancialCalculator::numPayments()
{
    Q_D(MyMoneyFinancialCalculator);
    const unsigned short mask = PV_SET | IR_SET | PMT_SET | FV_SET;

    if ((d->m_mask & mask) != mask)
        throw MYMONEYEXCEPTION_CSTRING("Not all parameters set for calculation of numPayments");

    double eint = d->eff_int();

    //add exception for zero interest
    if (eint == 0.0) {
        d->m_npp = -(d->m_pv / d->m_pmt);

    } else {
        double CC = d->_Cx(eint);

        CC = (CC - d->m_fv) / (CC + d->m_pv);
        d->m_npp = (CC > 0.0) ? log(CC) / log(eint + 1.0) : 0.0;

        d->m_mask |= NPP_SET;
    }
    return d->m_npp;
}

double MyMoneyFinancialCalculator::payment()
{
    Q_D(MyMoneyFinancialCalculator);
    const unsigned short mask = PV_SET | IR_SET | NPP_SET | FV_SET;

    if ((d->m_mask & mask) != mask)
        throw MYMONEYEXCEPTION_CSTRING("Not all parameters set for calculation of payment");

    double eint = d->eff_int();

    //add exception for zero interest
    if (eint == 0.0) {
        d->m_pmt = -(d->m_pv / d->m_npp);
    } else {
        double AA = d->_Ax(eint);
        double BB = d->_Bx(eint);

        d->m_pmt = -d->rnd((d->m_fv + d->m_pv * (AA + 1.0)) / (AA * BB));
    }

    d->m_mask |= PMT_SET;
    return d->m_pmt;
}

double MyMoneyFinancialCalculator::presentValue()
{
    Q_D(MyMoneyFinancialCalculator);
    const unsigned short mask = PMT_SET | IR_SET | NPP_SET | FV_SET;

    if ((d->m_mask & mask) != mask)
        throw MYMONEYEXCEPTION_CSTRING("Not all parameters set for calculation of payment");

    double eint = d->eff_int();

    //add exception for zero interest
    if (eint == 0.0) {
        d->m_pv = -(d->m_fv + (d->m_npp * d->m_pmt));
    } else {
        double AA = d->_Ax(eint);
        double CC = d->_Cx(eint);

        d->m_pv = d->rnd(-(d->m_fv + (AA * CC)) / (AA + 1.0));

    }

    d->m_mask |= PV_SET;
    return d->m_pv;
}

double MyMoneyFinancialCalculator::futureValue()
{
    Q_D(MyMoneyFinancialCalculator);
    const unsigned short mask = PMT_SET | IR_SET | NPP_SET | PV_SET;

    if ((d->m_mask & mask) != mask)
        throw MYMONEYEXCEPTION_CSTRING("Not all parameters set for calculation of payment");

    double eint = d->eff_int();

    //add exception for zero interest
    if (eint == 0.0) {
        d->m_fv = d->rnd(-(d->m_pv + (d->m_npp * d->m_pmt)));
    } else {
        double AA = d->_Ax(eint);
        double CC = d->_Cx(eint);
        d->m_fv = d->rnd(-(d->m_pv + AA * (d->m_pv + CC)));
    }

    d->m_mask |= FV_SET;
    return d->m_fv;
}

double MyMoneyFinancialCalculator::interestRate()
{
    Q_D(MyMoneyFinancialCalculator);
    double eint = 0.0;
    double a = 0.0;
    double dik = 0.0;

    const double ratio = 1e4;
    int ri;

    if (d->m_pmt == 0.0) {
        eint = pow((dabs(d->m_fv) / dabs(d->m_pv)), (1.0 / d->m_npp)) - 1.0;
    } else {
        if ((d->m_pmt * d->m_fv) < 0.0) {
            if (d->m_pv)
                a = -1.0;
            else
                a = 1.0;
            eint =

                dabs((d->m_fv + a * d->m_npp * d->m_pmt) / //
                     (3.0 * //
                      ((d->m_npp - 1.0) * (d->m_npp - 1.0) * d->m_pmt + d->m_pv - //
                       d->m_fv)));
        } else {
            if ((d->m_pv * d->m_pmt) < 0.0) {
                eint = dabs((d->m_npp * d->m_pmt + d->m_pv + d->m_fv) / (d->m_npp * d->m_pv));
            } else {
                a = dabs(d->m_pmt / (dabs(d->m_pv) + dabs(d->m_fv)));
                eint = a + 1.0 / (a * d->m_npp * d->m_npp * d->m_npp);
            }
        }
        do {
            try {
                dik = d->_fi(eint) / d->_fip(eint);
                eint -= dik;
            } catch (const MyMoneyException &) {
                eint = 0;
            }
            (void) modf(ratio *(dik / eint), &a);
            ri = static_cast<unsigned>(a);
        } while (ri);
    }
    d->m_mask |= IR_SET;
    d->m_ir = d->rnd(d->nom_int(eint) * 100.0);
    return d->m_ir;
}

double MyMoneyFinancialCalculator::interestDue() const
{
    Q_D(const MyMoneyFinancialCalculator);
    double eint = d->eff_int();

    return (d->m_pv + (d->m_bep ? d->m_pmt : 0.0)) * eint;
}

