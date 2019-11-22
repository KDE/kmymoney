#/usr/bin/python3
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
Uses the "Bankleitzahlendatei" of the german Bundesbank to create a bic
lookup table for KMyMoney

@author: Christian David
"""

import sqlite3
import codecs
import argparse


def createTable():
    """ Create table structure
    """
    cursor = db.cursor()
    cursor.execute("DROP TABLE IF EXISTS institutions")
    cursor.execute(
        "CREATE TABLE institutions ("
        " country CHAR(2) DEFAULT 'DE' CONSTRAINT germanCountryCode NOT NULL CHECK(country == 'DE'),"
        " bankcode CHAR(8) NOT NULL PRIMARY KEY CHECK(length(bankcode) = 8),"
        " bic CHAR(11),"
        " name VARCHAR(60)"
        " )"
    )
    db.commit()


def processFile(fileName):
    """ Fills the database with institutions saved in fileName
    """

    rowsInserted = 0

    cursor = db.cursor()
    cursor.execute("BEGIN")

    def submitInstitute(bankCode, bankName, bic):
        try:
            cursor.execute("INSERT INTO institutions (bankcode, bic, name) VALUES(?,?,?)", (bankCode, bic, bankName))
        except sqlite3.Error as e:
            print("Error: {0} while inserting {1} ({2})".format(e.args[0], bankCode, bic))

    institutesFile = codecs.open(fileName, "r", encoding=args.encoding)
    for institute in institutesFile:
        if institute[8:9] == "1":
            submitInstitute(institute[0:8], institute[9:67].strip(), institute[139:150])
            rowsInserted += 1

    db.commit()
    return rowsInserted


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Creates a SQLite database for KMyMoney with information about IBAN and BICs based on a fixed-column text file from the german central bank."
                                     " You can download the source file at https://www.bundesbank.de/de/aufgaben/unbarer-zahlungsverkehr/serviceangebot/bankleitzahlen/download-bankleitzahlen-602592"
                                     )
    parser.add_argument(dest='file', help='File to load')
    parser.add_argument('-o', '--output', default="bankdata.de.db", help='SQLite database to open/generate')
    parser.add_argument('-e', '--encoding', default="iso 8859-1", help='Charset of file')
    args = parser.parse_args()

    print("Read data from \"{0}\" with \"{1}\" encoding".format(args.file, args.encoding))
    db = sqlite3.connect(args.output)

    createTable()
    institutions = processFile(args.file)
    print("Inserted {0} institutions into database \"{1}\"".format(institutions, args.output))

    cursor = db.cursor()
    cursor.execute("ANALYZE institutions")
    cursor.execute("CREATE INDEX bic_index ON institutions (bic)")
    cursor.execute("REINDEX")
    cursor.execute("VACUUM")
    db.commit();
    db.close()
