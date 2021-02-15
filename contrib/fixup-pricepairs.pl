#!/usr/bin/perl
#
# SPDX-FileCopyrightText: 2018 Thomas Baumgart <tbaumgart@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
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

