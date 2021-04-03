/*
    SPDX-FileCopyrightText: 2003-2012 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYFINANCIALCALCULATOR_P_H
#define MYMONEYFINANCIALCALCULATOR_P_H

#include "mymoneyfinancialcalculator.h"

#include <qglobal.h>

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyexception.h"

class MyMoneyFinancialCalculatorPrivate
{
    Q_DISABLE_COPY(MyMoneyFinancialCalculatorPrivate)

public:

    MyMoneyFinancialCalculatorPrivate() :
        m_ir(0.0),
        m_pv(0.0),
        m_pmt(0.0),
        m_fv(0.0),
        m_npp(0.0),
        m_CF(0),
        m_PF(0),
        m_prec(2),
        m_bep(false),
        m_disc(false),
        m_mask(0)
    {
    }

    double _fi(const double eint) const
    {
        return _Ax(eint) *(m_pv + _Cx(eint)) + m_pv + m_fv;
    }

    double _fip(const double eint) const
    {
        double AA = _Ax(eint);
        double CC = _Cx(eint);
        double D = (AA + 1.0) / (eint + 1.0);

        return m_npp *(m_pv + CC) * D - (AA * CC) / eint;
    }

    double _Ax(const double eint) const
    {
        return pow((eint + 1.0), m_npp) - 1.0;
    }

    double _Bx(const double eint) const
    {
        if (eint == 0.0)
            throw MYMONEYEXCEPTION_CSTRING("Zero interest");

        if (m_bep == false)
            return static_cast<double>(1.0) / eint;

        return (eint + 1.0) / eint;
    }

    double _Cx(const double eint) const
    {
        return m_pmt * _Bx(eint);
    }

    double eff_int() const
    {
        double nint = m_ir / 100.0;
        double eint;

        if (m_disc) {             // periodically compound?
            if (m_CF == m_PF) {     // same frequency?
                eint = nint / static_cast<double>(m_CF);

            } else {
                eint = pow((static_cast<double>(1.0) + (nint / static_cast<double>(m_CF))),
                           (static_cast<double>(m_CF) / static_cast<double>(m_PF))) - 1.0;

            }

        } else {
            eint = exp(nint / static_cast<double>(m_PF)) - 1.0;
        }

        return eint;
    }

    double nom_int(const double eint) const
    {
        double nint;

        if (m_disc) {
            if (m_CF == m_PF) {
                nint = m_CF * eint;

            } else {
                nint = m_CF * (pow((eint + 1.0), (static_cast<double>(m_PF) / static_cast<double>(m_CF))) - 1.0);
            }
        } else
            nint = log(pow(eint + 1.0, m_PF));

        return nint;
    }

    double rnd(const double x) const
    {
        double r, f;

        if (m_prec > 0) {
            f = pow(10.0, m_prec);
            r = static_cast<double>(qRound64(x * f) / f);
        } else {
            r = static_cast<double>(qRound64(x));
        }
        return r;
    }

    double          m_ir;   // nominal interest rate
    double          m_pv;   // present value
    double          m_pmt;  // periodic payment
    double          m_fv;   // future value
    double          m_npp;  // number of payment periods

    unsigned short  m_CF;   // compounding frequency
    unsigned short  m_PF;   // payment frequency

    unsigned short  m_prec; // precision for roundoff for pv, pmt and fv
    // i is not rounded, n is integer

    bool            m_bep;  // beginning/end of period payment flag
    bool            m_disc; // discrete/continuous compounding flag

    unsigned short m_mask; // available value mask

};

#endif

