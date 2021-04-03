/*
    SPDX-FileCopyrightText: 2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

// This file is included as part of generated code and therefore
// does not contain the usually expected leadin of a class declaration
// and include guards.

// krazy:excludeall=includes

public:

static QColor schemeColor(const SchemeColor color);

static QFont listCellFontEx();
static QFont listHeaderFontEx();
static QStringList listOfItems();

/**
  * returns the number of the first month in the fiscal year
  */
static int firstFiscalMonth();

/**
  * returns the number of the first day of the fiscal year
  */
static int firstFiscalDay();

/**
  * returns the date of the first day in the current fiscal year
  */
static QDate firstFiscalDate();
