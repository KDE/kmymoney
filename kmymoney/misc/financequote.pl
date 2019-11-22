#!/usr/bin/perl
######################################################################
### financequote.pl - KMyMoney interface to Finance::Quote
###
### derived from GnuCash finance-quote-helper script which is
### Copyright 2001 Rob Browning <rlb@cs.utexas.edu>
### 
### This program is free software; you can redistribute it and/or    
### modify it under the terms of the GNU General Public License as   
### published by the Free Software Foundation; either version 2 of   
### the License, or (at your option) any later version.              
###                                                                  
### This program is distributed in the hope that it will be useful,  
### but WITHOUT ANY WARRANTY; without even the implied warranty of   
### MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    
### GNU General Public License for more details.                     
###                                                                  
### You should have received a copy of the GNU General Public License
### along with this program.
### If not, see <http://www.gnu.org/licenses/>.
######################################################################

#use diagnostics; # while testing
use strict;
use Data::Dumper;

my $prgnam = "kmymoneyfq.pl";
my $version = "1.00";
# perl modules required by this routine and Finance::Quote
my @modules = qw(Date::Manip Finance::Quote LWP XML::Parser XML::Writer);

# main - check command line arguments

my $testonly;
my $listonly;
# analyze the arguments
foreach my $arg (@ARGV) {
  my $listopt = "-l"; # I had a much slicker way of doing this but it stopped working...
  my $testopt = "-t";
  $testonly = 1 if $arg =~ $testopt;
  $listonly = 1 if $arg =~ $listopt;
}

# test call; check that all required modules are present
if ($testonly) {
  my @absent_modules; # to build list of missing modules

  foreach my $module (@modules) {
    if (!eval "require $module") {
      push (@absent_modules, $module);
    }
  }
  if (@absent_modules) {
    foreach my $module (@absent_modules) {
      print STDERR "  ".$module."\n";
    }
    exit 254; # missing modules exit code for kmymoney
  }
  exit 0;
}

# load the required modules
foreach my $module (@modules) {
  eval "require $module";
  $module->import();
}

# create a finance quote object and set required parameters
my $q = Finance::Quote->new();
$q->set_currency();  #  disable any currency conversion
$q->timeout(60);     # timeout 60 seconds
$q->failover(0);     # disable failover

# process call for exchange list only
if ($listonly) {
  my @sources = $q->sources();
  foreach my $source (@sources) {
    print "$source\n";
  }
  exit 0;
}

my $source = $ARGV[0];
my $symbol = $ARGV[1]; 

#print "\tfinding price for <$symbol> from <$source>\n";
my  %qhash = $q->fetch($source, $symbol); # get price data from F::Q
#my %qhash = ("RHATsuccess" => 1, "RHATdate" => "4/4/2004", "RHATcurrency" => "USD",
                        #"RHATbid" => "25.55", "RHATask" => "26.04");
#print Dumper(%qhash);
my $errcode;
$errcode = 0;

if (!%qhash) { $errcode = 1;} # no data from fq (?bad exchange?)
    elsif ($qhash {$symbol, "success"} != 1) {$errcode = 2;} # got data but quote failed (?bad symbol?)
    elsif (!$qhash{$symbol, "last"} and !$qhash{$symbol, "price"} ) {$errcode = 3;} # can't find a price (?hmmm?)
if ($errcode != 0) {
    print "Error " => "$errcode";
} else {
    # extract the date and convert from m/d/yyyy to yyyy-mm-dd
    my ($usdate, $month, $day, $year, $yyyymmdd);
    $usdate = $qhash{$symbol, "date"};
    ($month,$day,$year) = ($usdate =~ /([0-9]+)\/([0-9]+)\/([0-9]+)/);
    # i'm sure I can do the following with a regex but I'm just too idle...
    $month = "0$month" if ($month < 9);
    $day = "0$day" if ($day < 9);
    $yyyymmdd = "$year-$month-$day";
    # and the price
    # (tried having bid and ask here, but could be undef for some stocks (IBM)
    # and looked pretty unrealistic for others (e.g. RHAT on 15/5/04 was 12.09-38.32!))
    my $price = $qhash {$symbol, "last"};
    # if no price was found, try to extract it from the price symbol
    # see https://bugs.kde.org/show_bug.cgi?id=234268 for details
    $price = $qhash {$symbol, "price"} if (!$price);

    print "\"$symbol\",\"$yyyymmdd\",\"$price\"";
}

