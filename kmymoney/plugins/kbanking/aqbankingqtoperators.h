/**
 * @defgroup aqbankingqtoperators Helper functions for using aqbanking with Qt
 * 
 * These functions are similar to the ones in aqbanking. They are meant as glue between aqbanking and qt.
 * 
 * @{
 */

#ifndef AQBANKINGQTOPERATORS_H
#define AQBANKINGQTOPERATORS_H

#include <aqbanking/value.h>

#include "mymoneymoney.h"

/**
 * @brief Create AB_VALUE from mymoneymoney
 * 
 * @todo is there a better method than using MyMoneyMoney::toDouble()?
 */
AB_VALUE* AB_Value_fromMyMoneyMoney(const MyMoneyMoney& input)
{
  return (AB_Value_fromDouble(input.toDouble()));
}

/** @} */ // end of group aqbankingqtoperators

#endif // AQBANKINGQTOPERATORS_H
