// SPDX-FileCopyrightText: 2020 Shinjo Park <kde@peremen.name>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

ones = ["영", "일", "이", "삼", "사", "오", "육", "칠", "팔", "구"];
scales = ["", "만", "억", "조"];

function convertDigitGroup(number)
{
    ret = ""

    _thousands = (number / 1000) >> 0;
    number = number % 1000;
    _hundreds = (number / 100) >> 0;
    number = number % 100;
    _tens = (number / 10) >> 0;
    _ones = number % 10;

    if (_thousands != 0)
        ret += (ones[_thousands] + "천");

    if (_hundreds != 0)
        ret += (ones[_hundreds] + "백");

    if (_tens != 0)
        ret += (ones[_tens] + "십");

    if (_ones != 0)
        ret += ones[_ones];

    return ret;
}

function convertToNumeral(integer, fraction, denom)
{
    _integer = Ts.vals(0);
    _fraction = Ts.vals(1);
    _denom = Ts.vals(2);
    digitGroups = [];

    for (i = 0; i < 4; i++) {
        digitGroups.push(_integer % 10000);
        _integer = ((_integer / 10000) >> 0);
    }

    groupText = "";
    for (i = 0; i < 4; i++) {
        if (digitGroups[i] != 0)
            groupText = (convertDigitGroup(digitGroups[i]) + scales[i] + groupText);
    }

    if (_denom > 1)
        groupText += (" " + _fraction + "/" + _denom);

    return groupText;
}

Ts.setcall("금액", convertToNumeral);
