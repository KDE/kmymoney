#!/usr/bin/perl
#
# Copyright 2019       Thomas Baumgart <tbaumgart@kde.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

die ("Field '$field' not supported.") if ($fields !~ /\;$field\;/);
die ("Missing account id. Use --acc= to specify.") if ($accountid eq "");
die ("Missing field name. Use --field= to specify.") if ($field eq "");

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
                        print "$transactionid;$splitid;$value;$result\n";
                    }
                } else {
                    print "$transactionid;$splitid;$value\n";
                }
            }
        }
    }
}
