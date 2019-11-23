#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
This file is part of KMyMoney, A Personal Finance Manager by KDE
Copyright (C) 2014-2015 Christian Dávid <christian-david@web.de>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
"""

"""
Uses the " BIC-lijstvanNederlandsebankenvoorSEPA " of the dutch Betaalvereniging Nederland bank to create a bic lookup table for KMyMoney .

@author: Christian David
"""
# importing important packages
import sqlite3
import argparse
import xlrd

def createTable():
    """ Create table structure
    """
    cursor = db.cursor()
    cursor.execute("DROP TABLE IF EXISTS institutions")
    cursor.execute(
        "CREATE TABLE institutions ("
        " country CHAR(2) DEFAULT 'NL' CONSTRAINT dutchCountryCode NOT NULL CHECK(country == 'NL'),"
        " bankcode CHAR(4) NOT NULL PRIMARY KEY CHECK(length(bankcode) = 4),"
        " bic CHAR(11),"
        " name VARCHAR(60)"
        " )"
    )
    db.commit()

def processFile(fileName):
    """ Fills the database with institutions saved in fileName
    """

    cursor = db.cursor()
    cursor.execute("BEGIN")
    institutionCounter = 0

    def submitInstitute(bankCode, bankName, bic):
        try:
            cursor.execute("INSERT INTO institutions (bankCode, bic, name) VALUES(?,?,?)", (bankCode, bic, bankName))
        except sqlite3.Error as e:
            print("Sorry , Error: {0} while inserting {1} ({2})".format(e.args[0], bankCode, bic))

    book = xlrd.open_workbook(fileName, 'r')
    sheet = book.sheet_by_index(0)

    for row_index in range(2, sheet.nrows):
        submitInstitute(sheet.cell(row_index,0).value, sheet.cell(row_index,2).value, sheet.cell(row_index,1).value)
        institutionCounter += 1

    return institutionCounter

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Create an SQLite database for KMyMoney with information about IBAN and BICs based on an 'Excel 2007 sheet' from the Dutch Betaalvereniging for the banks in the netherlands."
                                     " You can download the source (.xlsx) file at https://www.betaalvereniging.nl/betalingsverkeer/giraal-betalingsverkeer/bic-sepa-transacties/"
                                     )
    parser.add_argument(dest='file', help='File to load')
    parser.add_argument('-o', '--output', default="bankdata.nl.db", help='SQLite database to open/generate')
    args = parser.parse_args()

    print("Read data from \"{0}\"".format(args.file))
    db = sqlite3.connect(args.output)

    createTable()
    institutions = processFile(args.file)
    print("Inserted {0} institutions into database \"{1}\"".format(institutions, args.output))

    cursor = db.cursor()
    cursor.execute("ANALYZE institutions")
    cursor.execute("CREATE INDEX bic_index ON institutions (bic)")
    cursor.execute("REINDEX")
    cursor.execute("VACUUM")
    db.commit()
    db.close()
