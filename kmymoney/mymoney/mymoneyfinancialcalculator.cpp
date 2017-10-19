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

#include "mymoneyfinancialcalculator.h"

#include <qglobal.h>

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyexception.h"

double MyMoneyFinancialCalculator::rnd(const double x) const
{
  double r, f;

  if (m_prec > 0) {
    f = pow(10.0, m_prec);
    r = qRound64(x * f) / f;
  } else {
    r = qRound64(x);
  }
  return r;
}

static inline double dabs(const double x)
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

void MyMoneyFinancialCalculator::setIr(const double ir)
{
  m_ir = ir;
  m_mask |= IR_SET;
}

void MyMoneyFinancialCalculator::setPv(const double pv)
{
  m_pv = pv;
  m_mask |= PV_SET;
}

void MyMoneyFinancialCalculator::setPmt(const double pmt)
{
  m_pmt = pmt;
  m_mask |= PMT_SET;
}

void MyMoneyFinancialCalculator::setNpp(const double npp)
{
  m_npp = npp;
  m_mask |= NPP_SET;
}

void MyMoneyFinancialCalculator::setFv(const double fv)
{
  m_fv = fv;
  m_mask |= FV_SET;
}

double MyMoneyFinancialCalculator::numPayments()
{
  const unsigned short mask = PV_SET | IR_SET | PMT_SET | FV_SET;

  if ((m_mask & mask) != mask)
    throw MYMONEYEXCEPTION("Not all parameters set for calculation of numPayments");

  double eint = eff_int();

  //add exception for zero interest
  if (eint == 0.0) {
    m_npp = -(m_pv / m_pmt);

  } else {
    double CC = _Cx(eint);

    CC = (CC - m_fv) / (CC + m_pv);
    m_npp = (CC > 0.0) ? log(CC) / log(eint + 1.0) : 0.0;

    m_mask |= NPP_SET;
  }
  return m_npp;
}

double MyMoneyFinancialCalculator::payment()
{
  const unsigned short mask = PV_SET | IR_SET | NPP_SET | FV_SET;

  if ((m_mask & mask) != mask)
    throw MYMONEYEXCEPTION("Not all parameters set for calculation of payment");

  double eint = eff_int();

  //add exception for zero interest
  if (eint == 0.0) {
    m_pmt = -(m_pv / m_npp);
  } else {
    double AA = _Ax(eint);
    double BB = _Bx(eint);

    m_pmt = -rnd((m_fv + m_pv * (AA + 1.0)) / (AA * BB));
  }

  m_mask |= PMT_SET;
  return m_pmt;
}

double MyMoneyFinancialCalculator::presentValue()
{
  const unsigned short mask = PMT_SET | IR_SET | NPP_SET | FV_SET;

  if ((m_mask & mask) != mask)
    throw MYMONEYEXCEPTION("Not all parameters set for calculation of payment");

  double eint = eff_int();

  //add exception for zero interest
  if (eint == 0.0) {
    m_pv = -(m_fv + (m_npp * m_pmt));
  } else {
    double AA = _Ax(eint);
    double CC = _Cx(eint);

    m_pv = rnd(-(m_fv + (AA * CC)) / (AA + 1.0));

  }

  m_mask |= PV_SET;
  return m_pv;
}

double MyMoneyFinancialCalculator::futureValue()
{
  const unsigned short mask = PMT_SET | IR_SET | NPP_SET | PV_SET;

  if ((m_mask & mask) != mask)
    throw MYMONEYEXCEPTION("Not all parameters set for calculation of payment");

  double eint = eff_int();

  //add exception for zero interest
  if (eint == 0.0) {
    m_fv = rnd(-(m_pv + (m_npp * m_pmt)));
  } else {
    double AA = _Ax(eint);
    double CC = _Cx(eint);
    m_fv = rnd(-(m_pv + AA * (m_pv + CC)));
  }

  m_mask |= FV_SET;
  return m_fv;
}

double MyMoneyFinancialCalculator::interestRate()
{
  double eint = 0.0;
  double a = 0.0;
  double dik = 0.0;

  const double ratio = 1e4;
  int ri;

  if (m_pmt == 0.0) {
    eint = pow((dabs(m_fv) / dabs(m_pv)), (1.0 / m_npp)) - 1.0;
  } else {
    if ((m_pmt * m_fv) < 0.0) {
      if (m_pv)
        a = -1.0;
      else
        a = 1.0;
      eint =
        dabs((m_fv + a * m_npp * m_pmt) /
             (3.0 *
              ((m_npp - 1.0) * (m_npp - 1.0) * m_pmt + m_pv -
               m_fv)));
    } else {
      if ((m_pv * m_pmt) < 0.0) {
        eint = dabs((m_npp * m_pmt + m_pv + m_fv) / (m_npp * m_pv));
      } else {
        a = dabs(m_pmt / (dabs(m_pv) + dabs(m_fv)));
        eint = a + 1.0 / (a * m_npp * m_npp * m_npp);
      }
    }
    do {
      try {
        dik = _fi(eint) / _fip(eint);
        eint -= dik;
      } catch (const MyMoneyException &) {
        eint = 0;
      }
      (void) modf(ratio *(dik / eint), &a);
      ri = static_cast<unsigned>(a);
    } while (ri);
  }
  m_mask |= IR_SET;
  m_ir = rnd(nom_int(eint) * 100.0);
  return m_ir;
}

double MyMoneyFinancialCalculator::_fi(const double eint) const
{
  return _Ax(eint) *(m_pv + _Cx(eint)) + m_pv + m_fv;
}

double MyMoneyFinancialCalculator::_fip(const double eint) const
{
  double AA = _Ax(eint);
  double CC = _Cx(eint);
  double D = (AA + 1.0) / (eint + 1.0);

  return m_npp *(m_pv + CC) * D - (AA * CC) / eint;
}

double MyMoneyFinancialCalculator::_Ax(const double eint) const
{
  return pow((eint + 1.0), m_npp) - 1.0;
}

double MyMoneyFinancialCalculator::_Bx(const double eint) const
{
  if (eint == 0.0)
    throw MYMONEYEXCEPTION("Zero interest");

  if (m_bep == false)
    return static_cast<double>(1.0) / eint;

  return (eint + 1.0) / eint;
}

double MyMoneyFinancialCalculator::_Cx(const double eint) const
{
  return m_pmt * _Bx(eint);
}

double MyMoneyFinancialCalculator::eff_int() const
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

double MyMoneyFinancialCalculator::nom_int(const double eint) const
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

double MyMoneyFinancialCalculator::interestDue() const
{
  double eint = eff_int();

  return (m_pv + (m_bep ? m_pmt : 0.0)) * eint;
}

