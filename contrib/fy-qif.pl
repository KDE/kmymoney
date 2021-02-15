#!/usr/bin/perl -w

#***************************************************************************
# SPDX-FileCopyrightText: 2012 Joe W. Byers <financialseal@financialseal.com>
# SPDX-License-Identifier: GPL-2.0-or-later
#***************************************************************************/

# Simple script to download security prices from finance.yahoo and create a 
# qif format for KMyMoney.
# Run by ''./fy-qif.pl' .
# You are required to specify a directory and file location in line  57.
# You should edit the list of security symbols in line 42.
# This file can be scheduled as a cron job if needed

# Output format -
# !Type:Prices
# "HJU8.BE",61.62,"23.12.09"
# ^

# This uses perl-Finance-YahooQuote
# SPDX-FileCopyrightText: 2002 Dirk Eddelbuettel <edd@debian.org>, and GPL'ed
# Based on the original example by Dj Padzensky
#
# $Id: yahooquote,v 1.2 2002/12/24 17:50:28 edd Exp $

#Todo: create a file read to read a file

# Configuration section
# List of symbols to be downloaded
@syms=(WMB,AAON,CSCO,EP,FLVCX,FSCGX,ISSC,MSFT,NOEQX,NOSIX,NTCHX,PG,RHT,SLR,FCNTX,FPURX,FJPNX,FSENX,OSMVX,ALU,TCLFX,FSLBX,FDFAX,FDIVX,ARTMX,EPD,FDGFX,OBFVX,TCLFX,OBBC,FDIKX,FDGFX);

# Filename of outputfile
$file = "/var/datadl/quotes.csv";

# No more configuration beyond this point

#use strict;
use Getopt::Long;
use Finance::YahooQuote;

my $verbose = 0;
#GetOptions("verbose" => \$verbose);

#die "Usage: $0 [--verbose] symbol [symbol ...]\n" if $#ARGV == -1;

my @h = ("Symbol","Name","Last","Trade Date","Trade Time","Change","% Change",
	 "Volume","Avg. Daily Volume","Bid","Ask","Prev. Close","Open",
	 "Day's Range","52-Week Range","EPS","P/E Ratio","Div. Pay Date",
	 "Div/Share","Div. Yield","Mkt. Cap","Exchange");

$Finance::YahooQuote::TIMEOUT = 30;

my @q = getquote(@syms);
open(FH, ">> $file") || die $!;
foreach $a (@q) {
print FH "!Type:Prices\n";
  foreach (0..$#h) {
    if ($verbose) {
      print "$h[$_]: $$a[$_]\n";
    } else {
      print FH "\"$$a[$_]\"," if $h[$_] =~ /(Symbol)/m;
      print FH "\"$$a[$_]\"\n" if $h[$_] =~ /(Trade Date)/m;
#      print FH "$$a[$_]\n" if $h[$_] =~ /(Symbol)/m;
      print FH "$$a[$_]," if $h[$_] =~ /(Last)/m;
    }
  }
  print FH "^\n";
}
close(FH);
