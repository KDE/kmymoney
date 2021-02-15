#!/usr/bin/env python
# -*- coding: utf-8 -*-

#***************************************************************************
#                      acst2qif.py  -  description
#                         -------------------
# copyright             : (C) 2013 by Volker Paul
# email                 : volker.paul@v-paul.de
#
#***************************************************************************/
#
#***************************************************************************
#*                                                                         *
#*   SPDX-License-Identifier: GPL-2.0-or-later
#*                                                                         *
#***************************************************************************/

# Usage:
# 0. Prepare .acst2qif.cfg according to your needs
# 1. Get account statement from your bank as PDF file
# 2. python acst2qif.py <options>
# 3. import account_statement.qif into KMyMoney using File - Import - QIF
# (May be used with other QIF-importing applications as well, but tested only with KMyMoney)
# Investment transactions are not yet implemented.

# Step by step
# Monthly do:
# For each account:
# - acquire account statement as PDF
# - put it in the dir specified in the account's section; ordered chronologically
# Run acst2quif.py without arguments so it uses default .acst2qif.cfg config file.
# It produces output in outfile specified in [General] section.
# Import outfile in KMyMoney.
# Check each account in KMyMoney against PDF account statement:
# - If you missed an account statement, there will be a difference between
#   KMyMoney and the statement's balance.
# - If you import an account statement twice, KMyMoney will most likely report it.

# Needs pdftotext in the path. Tested under Linux only.

# Caveat: This script completely relies on the configuration file and the regexps in it.
# A basic understanding of Python regexps is required to use it.
# If you have problems writing regexps, maybe I can help.
# This script stores the PDF file converted to text in an .acst file.
# You can run the script with an .acst file as input instead of the PDF file.
# Send me the .acst file, your current .cfg file and a description of 
# what you expect the script to do.
# Note that I can't write all regexps for you, I can only help you 
# find errors and provide examples.

__author__     = "Volker Paul"
__copyright__  = "Copyright 2013, Volker Paul"
__license__    = "GPL 2"
__maintainer__ = "Volker Paul"
__email__      = "volker.paul@v-paul.de"
__docformat__  = 'restructuredtext'
__status__     = "Production"

import sys, os, os.path, copy, re, textwrap, datetime, subprocess
from optparse import OptionParser
import ConfigParser

def uc(s):
    """Try to get around "'ascii' codec can't encode character xyz".
    Convert to Unicode.
    """
    try:
        res = unicode(s.decode('utf-8'))
    except:
        return s
    else:
        return res

def getFileList(dir, encoding='utf-8'):
    """Get a list of all files (only filenames, not complete paths)
    in given directory. Subdirectories and their contents are ignored.
    Output is sorted alphabetically.
    """
    if not os.path.isdir(dir):
        print "ERROR: No such directory: " + dir
        return None
    for root,dirs,files in os.walk(dir):
        if root==dir:
            files.sort()
            return [uc(f) for f in files if f.endswith(".pdf")]

def outputTransaction(output, tdate, text, amount, categoryDic):
    #print "OUTPUT:",  text
    text = ' '.join(text.split())
    output.write("!Type:Cash\n")
    date = tdate.strftime("%d.%m.%Y") 
    output.write('D' + date + '\n')
    output.write('M' + text + '\n')
    output.write('T' + amount + '\n')
    categ = guessCategory(text, categoryDic)
    if categ: 
        output.write('L' + categ + '\n')
        nc = 0
    else:
        print "No category for: %s %s %s" % (date, text[:40], amount)
        nc = 1
    output.write('^\n')
    return nc

def getAmount(rawAmount, credit_regexp, debit_regexp):
    """Recognize credits by credit_regexp, debits by debit_regexp.
    These regular expressions also split up the amount in integer and fractional part.
    """
    mc = re.compile(credit_regexp).match(rawAmount)
    if not mc: 
        md = re.compile(debit_regexp).match(rawAmount)
        if not md:
            print "ERROR: Can't get amount from string: '%s'" % rawAmount
            print "  Matches neither credit_regexp: %s", credit_regexp 
            print "  nor debit_regexp: %s", debit_regexp
            return ''
    sign = '+' if mc else '-'
    m = mc or md
    int_part = m.group('int').replace('.','')   # delete all non-digit characters
#    print "amount: '%s'" % (sign + int_part + '.' + m.group('frac'))
    return sign + int_part + '.' + m.group('frac')

def guessCategory(text, categoryDic):
    """Get category resp. account name from text.
    Try to match with value from an entry of categoryDic.
    """
    for regexp, name in categoryDic.iteritems():
        if re.compile(regexp).match(text): 
            return name
    return None

def convert(inpath, output, options, config, accountName, type, categoryDic):
#        convert(inpath, output, options, config)
    # line types:
    # ihead         item head line, contains date, transfer type and value
    # iadd          item additional details
    # other         other line, to be ignored

    textfile = inpath
    (root,  ext) = os.path.splitext(textfile)
    if ext == ".pdf":
        # print "This is a PDF file, converting to text."
        pdffile = textfile
        textfile = root + ".acst"
        res = subprocess.Popen(["pdftotext", "-layout", pdffile,  textfile], stdout=subprocess.PIPE).communicate()[0]
    categoryDicString = config.get(type, 'categoryDic')
    categoryDicAdd = eval(categoryDicString)
    # Some categoryDic entries come from the individual account, some from the account type.
    categoryDic.update(categoryDicAdd)
#    ihead_regexp = config.get(type, 'ihead_regexp')
    ihead_re = re.compile(config.get(type, 'ihead_regexp'))
    iadd_re = re.compile(config.get(type, 'iadd_regexp'))
    #date_re = re.compile(config.get(type, 'date_regexp'))
    #balance_re = re.compile(config.get(type, 'balance_regexp'))
    f = open(textfile, 'r')
    now = datetime.date.today()
    statementDate = None
    year = now.year
    text = ''
    count = 0; noCat = 0
    for l in f.readlines():
        if options.verbose:
            print "line: ", l,
        m = ihead_re.match(l)
        if m:
            if text: # There is old text, output it first.
                noCat += outputTransaction(output, tdate, text, amount, categoryDic)
                text = ''
                count += 1
            if options.verbose:
                print "HEAD LINE: ", l
            day = int(m.group('day'))
            month = int(m.group('month'))
            if 'year' in m.groupdict(): 
                year = int(m.group('year'))
                if year<100: year = 2000+year 
            tdate = datetime.date(year, month, day)
            if tdate>now: tdate = datetime.date(year-1, month, day)
            text = m.group('detail').strip()
            rawAmount = m.group('amount')
            if options.verbose: print 'rawAmount: "%s"' % rawAmount
            amount = getAmount(rawAmount, config.get(type, 'credit_regexp'), config.get(type, 'debit_regexp'))
            if options.verbose: print "amount:", amount 
            if options.verbose:
                print "date:", tdate, "   text:", text, "   rawAmount:", rawAmount
            continue
        m = iadd_re.match(l)
        if m:
            addedtext = m.group(1)
            if options.verbose:
                print "ADDED TEXT:", addedtext
            if text and addedtext: text += ' ' + addedtext.strip()
            continue
        if text: # If we still have some text and data from an entry above, output it now.
            noCat += outputTransaction(output, tdate, text, amount, categoryDic)
            text = ''
            count += 1
    print "%d transactions, %d without category, in account %s, file: %s" % (count, noCat, accountName, uc(textfile))
    #print "%d transactions in account %s, file: %s" % (count, accountName, "omitted")
    f.close()

def main():
    usage = textwrap.dedent("""
        %prog [options]
        Converter of ACcount STatements to QIF format.
        Needs configuration file by default in ~/.acst2qif.cfg, 
        see comments there.
        Typical usage after setting up directories 
        and adapting the configuration file:
        1. Get account statements from your banks as PDF files, 
           save them to directories set up above.
        2. Run this program (usually without arguments).
        3. Import file following "Results written to: " 
           into KMyMoney or other financial software.
        4. Check results in financial software.
    """)[1:-1]
    parser = OptionParser(version="%prog "+__version__, usage=usage)
    parser.add_option("-v", "--verbose", action="store_true", dest="verbose",
        default=False, help="be verbose")
    parser.add_option("-l", "--list", action="store_true", dest="listAccounts",
        default=False, help="only list accounts in config file and quit")
    parser.add_option("-i", "--input", default=None, dest="input", help="input file (list)")
    parser.add_option("-o", "--output", default=None, dest="output", help="output file")
    parser.add_option("-a", "--account", dest="account", help="account (list)")
#    parser.add_option("-t", "--type", dest="type", help="account statement type, defines format")
    parser.add_option("-c", "--configfile", default=os.path.expanduser("~/.acst2qif.cfg"), 
        dest="configfile", help="configuration file, default ~/.acst2qif.cfg")
    (options, args) = parser.parse_args()
    if len(args)!=0: 
        parser.print_help()
        exit(1)
    config = ConfigParser.RawConfigParser()
    config.read(options.configfile)
    if options.listAccounts:
	print "Accounts: ", config.get("General", "accounts")
	exit(0)
    accountString = options.account or config.get("General", "accounts")
    accounts = [a.strip() for a in accountString.split(',')]
    outfile = options.output or config.get("General", "outfile")
    output = open(outfile, 'w')
    # User can specify a list of input files (.pdf or .acst) explicitly (exactly as many as accounts).
    filelist = [f.strip() for f in options.input.split(',')] if options.input else None
    # If no filelist is specified, the latest file from the account's directory is taken as input.
    if filelist and len(filelist) != len(accounts):
        print "There must be as many files (given %d) as accounts (%d)!" % (len(filelist), len(accounts))
        exit(2)
    for i, account in enumerate(accounts):
        accountName = config.get(account, "name")
        qifAccountType = config.get(account, "qif_account_type")
        accountType = config.get(account, "type")
        # print "name:",  accountName,  "  type:",  accountType 
        output.write('!Account\n')
        output.write('N%s\n' % accountName)
        output.write('T%s\n' % qifAccountType)
        output.write('^\n')
        if filelist:
            inpath = filelist[i]
        else:
            dir = uc(config.get(account, "dir"))
            fl = getFileList(dir)
            if not fl:
                print "ERROR: No input file"
                return
            infile = uc(fl[-1])
            inpath = dir + '/' + infile
        categoryDicString = config.get(account, 'categoryDic')
        categoryDic = eval(categoryDicString)
        convert(inpath, output, options, config, accountName, accountType, categoryDic)
    output.close()
    print "Results written to:", outfile

main()
