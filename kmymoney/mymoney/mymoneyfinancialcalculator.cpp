/***************************************************************************
                          mymoneyfinancialcalculator.cpp  -  description
                             -------------------
    begin                : Tue Oct 21 2003
    copyright            : (C) 2000-2003 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <config-kmymoney.h>

#include "mymoneyfinancialcalculator.h"

#include <math.h>
#include <stdio.h>

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyexception.h"

// #ifndef HAVE_ROUND
// #undef roundl
// #define roundl(a)  rnd(a)

FCALC_DOUBLE MyMoneyFinancialCalculator::rnd(const FCALC_DOUBLE x) const
{
  FCALC_DOUBLE r,f;

  if(m_prec > 0) {
#ifdef HAVE_ROUND
    f = powl(10.0, m_prec);
    r = roundl(x * f)/f;
#else
    char  buf[50];
#if HAVE_LONG_DOUBLE
    sprintf (buf, "%.*Lf", m_prec, x);
    sscanf (buf, "%Lf", &r);
#else
    sprintf (buf, "%.*f", m_prec, x);
    sscanf (buf, "%lf", &r);
#endif
#endif
  } else
    r = roundl(x);
  return r;
}
// #endif

static inline FCALC_DOUBLE dabs(const FCALC_DOUBLE x)
{
  return (x >= 0.0) ? x : -x;
}

MyMoneyFinancialCalculator::MyMoneyFinancialCalculator()
{
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
  m_mask = 0;
}

MyMoneyFinancialCalculator::~MyMoneyFinancialCalculator()
{
}

void MyMoneyFinancialCalculator::setPrec(const unsigned short prec)
{
  m_prec = prec;
}

void MyMoneyFinancialCalculator::setPF(const unsigned short PF)
{
  m_PF = PF;
}

void MyMoneyFinancialCalculator::setCF(const unsigned short CF)
{
  m_CF = CF;
}

void MyMoneyFinancialCalculator::setBep(const bool bep)
{
  m_bep = bep;
}

void MyMoneyFinancialCalculator::setDisc(const bool disc)
{
  m_disc = disc;
}

void MyMoneyFinancialCalculator::setIr(const FCALC_DOUBLE ir)
{
  m_ir = ir;
  m_mask |= IR_SET;
}

void MyMoneyFinancialCalculator::setPv(const FCALC_DOUBLE pv)
{
  m_pv = pv;
  m_mask |= PV_SET;
}

void MyMoneyFinancialCalculator::setPmt(const FCALC_DOUBLE pmt)
{
  m_pmt = pmt;
  m_mask |= PMT_SET;
}

void MyMoneyFinancialCalculator::setNpp(const FCALC_DOUBLE npp)
{
  m_npp = npp;
  m_mask |= NPP_SET;
}

void MyMoneyFinancialCalculator::setFv(const FCALC_DOUBLE fv)
{
  m_fv = fv;
  m_mask |= FV_SET;
}

FCALC_DOUBLE MyMoneyFinancialCalculator::numPayments(void)
{
  const unsigned short mask = PV_SET | IR_SET | PMT_SET | FV_SET;

  if((m_mask & mask) != mask)
    throw new MYMONEYEXCEPTION("Not all parameters set for calculation of numPayments");

  FCALC_DOUBLE eint = eff_int();
  FCALC_DOUBLE CC = _Cx(eint);

  CC = (CC - m_fv) / (CC + m_pv);
  m_npp = (CC > 0.0) ? logl (CC) / logl (eint +1.0) : 0.0;

  m_mask |= NPP_SET;
  return m_npp;
}

FCALC_DOUBLE MyMoneyFinancialCalculator::payment(void)
{
  const unsigned short mask = PV_SET | IR_SET | NPP_SET | FV_SET;

  if((m_mask & mask) != mask)
    throw new MYMONEYEXCEPTION("Not all parameters set for calculation of payment");

  FCALC_DOUBLE eint = eff_int();
  FCALC_DOUBLE AA = _Ax(eint);
  FCALC_DOUBLE BB = _Bx(eint);

  m_pmt = -rnd((m_fv + m_pv * (AA + 1.0)) / (AA * BB));
  //m_pmt = -floorl((m_fv + m_pv * (AA + 1.0)) / (AA * BB));

  m_mask |= PMT_SET;
  return m_pmt;
}

FCALC_DOUBLE MyMoneyFinancialCalculator::presentValue(void)
{
  const unsigned short mask = PMT_SET | IR_SET | NPP_SET | FV_SET;

  if((m_mask & mask) != mask)
    throw new MYMONEYEXCEPTION("Not all parameters set for calculation of payment");

  FCALC_DOUBLE eint = eff_int();
  FCALC_DOUBLE AA = _Ax(eint);
  FCALC_DOUBLE CC = _Cx(eint);

  m_pv = rnd(-(m_fv + (AA * CC)) / (AA + 1.0));

  m_mask |= PV_SET;
  return m_pv;
}

FCALC_DOUBLE MyMoneyFinancialCalculator::futureValue(void)
{
  const unsigned short mask = PMT_SET | IR_SET | NPP_SET | PV_SET;

  if((m_mask & mask) != mask)
    throw new MYMONEYEXCEPTION("Not all parameters set for calculation of payment");

  FCALC_DOUBLE eint = eff_int();
  FCALC_DOUBLE AA = _Ax(eint);
  FCALC_DOUBLE CC = _Cx(eint);
  m_fv = rnd(-(m_pv + AA * (m_pv + CC)));

  m_mask |= FV_SET;
  return m_fv;
}

FCALC_DOUBLE MyMoneyFinancialCalculator::interestRate(void)
{
  FCALC_DOUBLE eint = 0.0;
  FCALC_DOUBLE a = 0.0;
  FCALC_DOUBLE dik = 0.0;

  const FCALC_DOUBLE ratio = 1e4;
  int ri;

  if (m_pmt == 0.0) {
    eint = powl ((dabs (m_fv) / dabs (m_pv)), (1.0 / m_npp)) - 1.0;
  } else {
    if ((m_pmt * m_fv) < 0.0) {
      if(m_pv)
        a = -1.0;
      else
        a = 1.0;
      eint =
        dabs ((m_fv + a * m_npp * m_pmt) /
              (3.0 *
               ((m_npp - 1.0) * (m_npp - 1.0) * m_pmt + m_pv -
                m_fv)));
    } else {
      if ((m_pv * m_pmt) < 0.0) {
        eint = dabs ((m_npp * m_pmt + m_pv + m_fv) / (m_npp * m_pv));
      } else {
        a = dabs (m_pmt / (dabs(m_pv) + dabs(m_fv)));
        eint = a + 1.0 / (a * m_npp * m_npp * m_npp);
      }
    }
    do {
      try {
        dik = _fi(eint) / _fip(eint);
        eint -= dik;
      } catch(MyMoneyException *e) {
        delete e;
        eint = 0;
      }
      (void) modfl(ratio * (dik / eint), &a);
      ri = static_cast<unsigned> (a);
    }
    while (ri);
  }
  m_mask |= IR_SET;
  m_ir = rnd(nom_int(eint) * 100.0);
  return m_ir;
}

FCALC_DOUBLE MyMoneyFinancialCalculator::_fi(const FCALC_DOUBLE eint) const
{
  return _Ax(eint) * (m_pv + _Cx(eint)) + m_pv + m_fv;
}

FCALC_DOUBLE MyMoneyFinancialCalculator::_fip(const FCALC_DOUBLE eint) const
{
  double AA = _Ax(eint);
  double CC = _Cx(eint);
  double D = (AA + 1.0) / (eint + 1.0);

  return m_npp *(m_pv + CC) * D - (AA * CC) / eint;
}

FCALC_DOUBLE MyMoneyFinancialCalculator::_Ax(const FCALC_DOUBLE eint) const
{
  return powl ((eint + 1.0), m_npp) - 1.0;
}

FCALC_DOUBLE MyMoneyFinancialCalculator::_Bx(const FCALC_DOUBLE eint) const
{
  if(eint == 0.0)
    throw new MYMONEYEXCEPTION("Zero interest");

  if(m_bep == false)
    return static_cast<FCALC_DOUBLE>(1.0) / eint;

  return (eint + 1.0) / eint;
}

FCALC_DOUBLE MyMoneyFinancialCalculator::_Cx(const FCALC_DOUBLE eint) const
{
  return m_pmt * _Bx(eint);
}

FCALC_DOUBLE MyMoneyFinancialCalculator::eff_int(void) const
{
  FCALC_DOUBLE nint = m_ir / 100.0;
  FCALC_DOUBLE eint;

  if(m_disc) {              // periodically compound?
    if(m_CF == m_PF) {      // same frequency?
      eint = nint / static_cast<FCALC_DOUBLE>(m_CF);

    } else {
      eint = powl((static_cast<FCALC_DOUBLE>(1.0) + (nint / static_cast<FCALC_DOUBLE>(m_CF))),
                  (static_cast<FCALC_DOUBLE>(m_CF) / static_cast<FCALC_DOUBLE>(m_PF))) - 1.0;

    }

  } else {
    eint = expl(nint / static_cast<FCALC_DOUBLE>(m_PF)) - 1.0;
  }

  return eint;
}

FCALC_DOUBLE MyMoneyFinancialCalculator::nom_int(const FCALC_DOUBLE eint) const
{
  FCALC_DOUBLE nint;

  if(m_disc) {
    if(m_CF == m_PF) {
      nint = m_CF * eint;

    } else {
      nint = m_CF * (powl ((eint + 1.0), (static_cast<FCALC_DOUBLE>(m_PF) / static_cast<FCALC_DOUBLE>(m_CF))) - 1.0);
    }
  } else
    nint = logl (powl (eint + 1.0, m_PF));

  return nint;
}

FCALC_DOUBLE MyMoneyFinancialCalculator::interestDue(void) const
{
  FCALC_DOUBLE eint = eff_int();

  return (m_pv + (m_bep ? m_pmt : 0.0)) * eint;
}

