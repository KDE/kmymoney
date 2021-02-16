# -*- coding: utf-8 -*-
"""
SPDX-FileCopyrightText: 2014-2015 Christian Dávid <christian-david@web.de>
SPDX-License-Identifier: GPL-2.0-or-later
"""

"""


@author: Christian David
"""

import sqlite3
import codecs
import argparse
import csv


def createTable():
    """ Create table structure
    """
    cursor = db.cursor()
    cursor.execute("DROP TABLE IF EXISTS institutions")
    cursor.execute(
        "CREATE TABLE institutions ("
        " country CHAR(2) NOT NULL CONSTRAINT wrongCountryCode CHECK(length(country) = 2),"
        " bankcode CHAR(0) DEFAULT NULL CONSTRAINT noBankCodes CHECK(bankcode is NULL),"
        " bic CHAR(11) NOT NULL UNIQUE CONSTRAINT bicLength CHECK(length(bic) = 11),"
        " name VARCHAR(256),"
        " PRIMARY KEY(bic)"
        " )"
    )
    db.commit()


def processFile(fileName):
    """ Fills the database with institutions saved in fileName
    """

    rowsInserted = 0

    cursor = db.cursor()
    cursor.execute("BEGIN")

    def submitInstitute(country, bankName, bic):
        try:
            cursor.execute("INSERT INTO institutions (country, bic, name) VALUES(?,?,?)", (country, bic, bankName))
        except sqlite3.Error as e:
            print("Error: {0} while inserting {1}-{2} (\"{3}\")".format(e.args[0], country, bic, bankName))

    with codecs.open(fileName, "r", encoding=args.encoding) as csvfile:
        reader = csv.reader(csvfile)
        for institute in reader:
          submitInstitute(institute[0][4:6], institute[1], institute[0])
          rowsInserted += 1

    db.commit()
    return rowsInserted


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Creates a SQLite database for KMyMoney with information about IBAN and BICs based on the list of participants of Target2."
        " You can download the csv file from https://www.ecb.europa.eu/paym/t2/professional/participation/html/index.en.html"
    )

    parser.add_argument(dest='file', help='File to load')
    parser.add_argument('-o', '--output', default="bankdata.target2.db", help='SQLite database to open/generate')
    parser.add_argument('-e', '--encoding', default="ascii", help='Charset of file')
    args = parser.parse_args()

    print("Read data from \"{0}\" with \"{1}\" encoding".format(args.file, args.encoding))
    db = sqlite3.connect(args.output)

    createTable()
    institutions = processFile( args.file )
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
