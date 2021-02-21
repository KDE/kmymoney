/*
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DIALOGENUMS_H
#define DIALOGENUMS_H

namespace eDialogs {
  /**
    * This enum is used to describe the bits of an account type filter mask.
    * Each bit is used to define a specific account class. Multiple classes
    * can be specified by OR'ing multiple entries. The special entry @p last
    * marks the left most bit in the mask and is used by scanners of this
    * bitmask to determine the end of processing.
    */
  enum Category : int {
    none =       0x000,         ///< no account class selected
    liability =  0x001,         ///< liability accounts selected
    asset =      0x002,         ///< asset accounts selected
    expense =    0x004,         ///< expense accounts selected
    income =     0x008,         ///< income accounts selected
    equity =     0x010,         ///< equity accounts selected
    checking =   0x020,         ///< checking accounts selected
    savings =    0x040,         ///< savings accounts selected
    investment = 0x080,         ///< investment accounts selected
    creditCard = 0x100,         ///< credit card accounts selected
    last =       0x200,         ///< the leftmost bit in the mask
  };

  enum class UpdatePrice {
    All = 0,
    Missing,
    Downloaded,
    SameSource,
    Ask,
  };

  enum class PriceMode {
    Price = 0,
    PricePerShare,
    PricePerTransaction,
  };

  enum class ScheduleResultCode {
    Cancel = 0,    // cancel the operation
    Enter,         // enter the schedule
    Skip,          // skip the schedule
    Ignore,        // ignore the schedule
  };

}

#endif
