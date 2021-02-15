#!/usr/bin/perl
#
# SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
#
# ----------------------------------------------------------------------
#
# Extract split field values for a specific account and field into a CSV file
#
# The script reads a plain text KMyMoney XML data file on stdin and writes
# the following columns as CSV format on stdout:
#
# transactionId;splitId;<selected-field>[;numeric amount]
#
# in case the selected field is either 'shares' or 'value' the column 'amount'
# will be added to the output which contains the value in a form that can be
# processed by spreadsheet programs
#
#
# usage: zcat data.kmy | getsplitpart.pl --acc=A000002 --field=shares > output.csv
#


my @args = @ARGV;
my $accountid;
my $field;
my $fields = ";shares;value;payee;";
my @files = ();

while ($#args >= 0) {
    my $a = shift @args;
    $accountid = $1 if ($a =~ /--acc=(.*)/);
    $field = $1 if ($a =~ /--field=(.*)/);
}

die ("Missing account id. Use --acc= to specify.") if ($accountid eq "");
die ("Missing field name. Use --field= to specify.") if ($field eq "");
die ("Field '$field' not supported.") if ($fields !~ /\;$field\;/);

my $transactionid;
my $splitid;

# print header line
print "transactionId;splitId;$field";
print ";amount" if ($field =~ /^(shares|value)$/);
print "\n";

while(<STDIN>) {
    if ($_ =~ /<SCHEDULED_TX .+=/) {
        $_ =~ /id="([^"]*)"/;
        $scheduleid = $1;
        next;
    }
    if ($_ =~ /<TRANSACTION [a-z]+=/) {
        $_ =~ /id="([^"]*)"/;
        $transactionid = $1;
        $transactionid = $scheduleid if($transactionid eq "");
        $_ =~ /postdate="([^"]*)"/;
        $postdate = $1;
        next;
    }
    if ($_ =~ /<SPLIT [a-z]+=/) {
        $_ =~ /id="([^"]*)"/;
        $splitid = $1;
        if ($_ =~ /account="$accountid"/) {
            if ($_ =~ /$field="([^"]*)"/) {
                my $value = $1;
                if ($value =~ /-?[0-9]+\/[0-9]+/) {
                    $result = eval $value;
                    if ($@) {
                        print "Invalid string: '$value'\n";
                    } else {
                        print "$postdate;$transactionid;$splitid;$value;$result\n";
                    }
                } else {
                    print "$postdate;$transactionid;$splitid;$value\n";
                }
            }
        }
    }
}
