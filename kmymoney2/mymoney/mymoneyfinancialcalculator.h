/***************************************************************************
                          mymoneyfinancialcalculator.h  -  description
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

#ifndef MYMONEYFINANCIALCALCULATOR_H
#define MYMONEYFINANCIALCALCULATOR_H

#include <config-kmymoney.h>

#include <cmath>
// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/export.h>

#ifdef _GLIBCPP_HAVE_MODFL
#define HAVE_LONG_DOUBLE  1
#endif

#ifndef HAVE_LONG_DOUBLE
#define HAVE_LONG_DOUBLE  0
#endif

#if HAVE_LONG_DOUBLE
#define FCALC_DOUBLE  long double
#else
#define FCALC_DOUBLE  double
#define modfl(a,b)    modf(a,b)
#define roundl(a)     round(a)
#define powl(a,b)     pow(a,b)
#define expl(a)       exp(a)
#define logl(a)       log(a)
#define floorl(a)     floor(a)
#define fabsl(a)      fabs(a)
#endif

/**
  * @author Thomas Baumgart
  */

/**
  * This class implements the financial calculator as found in GNUCash.
  * For a detailed description of the algorithms see
  * gnucash-1.8.5/src/doc/finutil.html.
  */
class KMYMONEY_EXPORT MyMoneyFinancialCalculator
{
public:
  MyMoneyFinancialCalculator();
  ~MyMoneyFinancialCalculator();

  /**
    * This method calculates the number of payments required to amortize
    * the loan. ir, pv, fv and pmt must be set. It sets the member variable
    * m_npp with the calculated value.
    *
    * @return number of periodic payments
    *
    * @exception If one of the required values is not set, a MyMoneyException
    *             will be thrown
    */
  FCALC_DOUBLE numPayments();

  /**
    * This method calculates the amount of the payment (amortization and interest)
    * for the loan. ir, pv, fv and npp must be set. It sets the member variable
    * m_pmt with the calculated value.
    *
    * @return amount of payment
    *
    * @exception If one of the required values is not set, a MyMoneyException
    *             will be thrown
    */
  FCALC_DOUBLE payment();

  /**
    * This method calculates the present value
    * for the loan. ir, pmt, fv and npp must be set. It sets the member variable
    * m_pv with the calculated value.
    *
    * @return present value of loan
    *
    * @exception If one of the required values is not set, a MyMoneyException
    *             will be thrown
    */
  FCALC_DOUBLE presentValue();

  /**
    * This method calculates the future value
    * for the loan. ir, pmt, pv and npp must be set. It sets the member variable
    * m_fv with the calculated value.
    *
    * @return future value of loan
    *
    * @exception If one of the required values is not set, a MyMoneyException
    *             will be thrown
    */
  FCALC_DOUBLE futureValue();

  /**
    * This method calculates the nominal interest rate
    * for the loan. fv, pmt, pv and npp must be set. It sets the member variable
    * m_ir with the calculated value.
    *
    * @return interest rate of the loan
    *
    * @exception If one of the required values is not set, a MyMoneyException
    *             will be thrown
    */
  FCALC_DOUBLE interestRate();

  /**
    * This method calculates the interest due for the next payment according
    * to the equation
    *
    *   id[n] = (pv[n-1] + (X * pmt)) * i
    *
    *   with
    *
    *   - pv[n-1]\n
    *     the present value at the end of the last period
    *   - X\n
    *     0 for end of period payments, 1 for beginning of period payments
    *   - pmt\n
    *     the periodic payment amount and
    *   - i\n
    *     the effective interest rate
    *
    * pv[n-1] will be the value as set with setPv(), i will be calculated
    * from the nominal interest rate as set with setIr(), pmt will be the
    * value as set with setPmt() and X is determined by the argument to
    * setBep().
    *
    * @return the interest amount
    */
  FCALC_DOUBLE interestDue(void) const;

  /**
    * This method sets the rounding precision to @p prec fractional
    * digits. The default of @p is 2. Rounding is applied to pv, pmt
    * and fv.
    *
    * @param prec Number of fractional digits after rounding.
    */
  void setPrec(const unsigned short prec = 2);

  /**
    * This method sets the number of payment periods to the value
    * passed in parameter @p npp. The length of a period is controlled
    * via setPF().
    *
    * @param npp number of payment periods
    */
  void setNpp(const FCALC_DOUBLE npp);

  FCALC_DOUBLE npp(void) const { return m_npp; };

  /**
    * This method sets the payment frequency. The parameter @p PF
    * specifies the payments per year.
    *
    *  - 1 == annual
    *  - 2 == semi-annual
    *  - 3 == tri-annual
    *  - 4 == quaterly
    *  - 6 == bi-monthly
    *  - 12 == monthly
    *  - 24 == semi-monthly
    *  - 26 == bi-weekly
    *  - 52 == weekly
    *  - 360 == daily
    *  - 365 == daily
    *
    * @param PF length of payment period (default is 12 - monthly)
    */
  void setPF(const unsigned short PF = 12);

  /**
    * This method sets the compounding frequency. The parameter @p CF
    * specifies the compounding period per year.
    *
    *  - 1 == annual
    *  - 2 == semi-annual
    *  - 3 == tri-annual
    *  - 4 == quaterly
    *  - 6 == bi-monthly
    *  - 12 == monthly
    *  - 24 == semi-monthly
    *  - 26 == bi-weekly
    *  - 52 == weekly
    *  - 360 == daily
    *  - 365 == daily
    *
    * @param CF length of compounding period (default is 12 - monthly)
    */
  void setCF(const unsigned short CF = 12);

  /**
    * This method controls whether the interest will be calculated
    * at the end of the payment period of at it's beginning.
    *
    * @param bep if @p false (default) then the interest is due at the
    *            end of the payment period, if @p true at it's beginning.
    */
  void setBep(const bool bep = false);

  /**
    * This method controls whether the interest is compounded in periods
    * or continously.
    *
    * @param disc if @p true (default) then the interest is compounded in
    *             periods, if @p false continously.
    */
  void setDisc(const bool disc = true);

  /**
    * This method sets the nominal interest rate to the value passed
    * in the argument @p ir.
    *
    * @param ir nominal interest rate
    */
  void setIr(const FCALC_DOUBLE ir);

  FCALC_DOUBLE ir(void) const { return m_ir; };

  /**
    * This method sets the present value to the value passed
    * in the argument @p pv.
    *
    * @param pv present value
    */
  void setPv(const FCALC_DOUBLE pv);

  FCALC_DOUBLE pv(void) const { return m_pv; };

  /**
    * This method sets the payment amount to the value passed
    * in the argument @p pmt.
    *
    * @param pmt payment amount
    */
  void setPmt(const FCALC_DOUBLE pmt);

  FCALC_DOUBLE pmt(void) const { return m_pmt; };

  /**
    * This method sets the future value to the value passed
    * in the argument @p fv.
    *
    * @param fv future value
    */
  void setFv(const FCALC_DOUBLE fv);

  FCALC_DOUBLE fv(void) const { return m_fv; };

private:
  FCALC_DOUBLE eff_int(void) const;
  FCALC_DOUBLE nom_int(const FCALC_DOUBLE eint) const;
  FCALC_DOUBLE rnd(const FCALC_DOUBLE x) const;

  FCALC_DOUBLE _Ax(const FCALC_DOUBLE eint) const;
  FCALC_DOUBLE _Bx(const FCALC_DOUBLE eint) const;
  FCALC_DOUBLE _Cx(const FCALC_DOUBLE eint) const;
  FCALC_DOUBLE _fi(const FCALC_DOUBLE eint) const;
  FCALC_DOUBLE _fip(const FCALC_DOUBLE eint) const;

private:
  FCALC_DOUBLE          m_ir;   // nominal interest rate
  FCALC_DOUBLE          m_pv;   // present value
  FCALC_DOUBLE          m_pmt;  // periodic payment
  FCALC_DOUBLE          m_fv;   // future value
  FCALC_DOUBLE          m_npp;  // number of payment periods

  unsigned short        m_CF;   // compounding frequency
  unsigned short        m_PF;   // payment frequency

  unsigned short        m_prec; // precision for roundoff for pv, pmt and fv
                                // i is not rounded, n is integer

  bool                  m_bep;  // beginning/end of period payment flag
  bool                  m_disc; // discrete/continous compounding flag

  unsigned short        m_mask; // available value mask
  #define PV_SET        0x0001
  #define IR_SET        0x0002
  #define PMT_SET       0x0004
  #define NPP_SET       0x0008
  #define FV_SET        0x0010
};

#endif
