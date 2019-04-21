#!/usr/bin/perl
#
# Copyright 2018       Thomas Baumgart <tbaumgart@kde.org>
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
# This tool reads a KMyMoney XML file on stdin and writes it to stdout
#
# While doing so it fixes broken PRICEPAIRs by removing the entries

my $skip = 0;

while(<>) {
  if ($_ =~ /<PRICEPAIR to=\"([^\"]*)\" from=\"([^\"]*)\">/) {
    my $from_id = $2;
    my $to_id = $1;

    $skip = 1 if ($to_id =~ /E\d{6}/);
    print $_ if ($skip == 0);

  } elsif ($_ =~ /<\/PRICEPAIR>/) {
    print $_ if ($skip == 0);
    $skip = 0;

  } else {
    print $_ if ($skip == 0);
  }
}

