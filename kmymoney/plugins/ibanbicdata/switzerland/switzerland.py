# -*- coding: utf-8 -*-
"""
SPDX-FileCopyrightText: 2014-2015 Christian Dávid <christian-david@web.de>
SPDX-License-Identifier: GPL-2.0-or-later
"""

"""
Reads swiss a bank-clearing-number file to generate a SQLite lookup table for
KMyMoney

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
        " country CHAR(2) DEFAULT 'CH' CONSTRAINT switzerlandCountryCode NOT NULL CHECK( country = 'CH'),"
        " bankcode CHAR(5) NOT NULL CONSTRAINT bcNumberLength CHECK(length(bankcode) == 5),"
        " bic CHAR(11) NOT NULL CONSTRAINT bicLength CHECK(length(bic) == 11),"
        " name VARCHAR(60),"
        " PRIMARY KEY(bankcode, bic)"
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
        bic = institute[284:295].strip()
        if len(bic) > 0:
            bcNumber = "{:0>5}".format(institute[2:7].strip() if institute[11:16] == "     " else institute[11:16].strip())
            name = "%s (%s)" % (institute[54:114].strip(), institute[194:229].strip())
            submitInstitute(bcNumber, name, bic)
            rowsInserted += 1

    db.commit()
    return rowsInserted


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Creates a SQLite database for KMyMoney with information about IBAN and BICs based on a swiss BC-Bankenstamm file."
        " You can get the BC-Bankenstamm file from https://www.six-group.com/interbank-clearing/de/home/bank-master-data/download-bc-bank-master.html"
    )

    parser.add_argument(dest='file', help='File to load')
    parser.add_argument('-o', '--output', default="bankdata.ch.db", help='SQLite database to open/generate')
    parser.add_argument('-e', '--encoding', default="iso 8859-15", help='Charset of file')
    args = parser.parse_args()

    print("Read data from \"{0}\" with \"{1}\" encoding".format(args.file, args.encoding))
    db = sqlite3.connect(args.output)

    createTable()
    institutions = processFile(args.file)
    print("Inserted {0} institutions into database \"{1}\"".format(institutions, args.output))

    cursor = db.cursor()
    cursor.execute("ANALYZE institutions")
    # This table is so small it should fit in the memory without any problems
    #cursor.execute("CREATE INDEX bic_index ON institutions (bic)")
    #cursor.execute("CREATE INDEX clearingnumber_index ON institutions (bankcode)")
    cursor.execute("REINDEX")
    cursor.execute("VACUUM")
    db.commit()
    db.close()
