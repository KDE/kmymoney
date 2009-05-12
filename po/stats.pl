#!/usr/bin/perl
#
# This script reads the xx.po file and generates some statistics
# information in form of XML output. This output is parsed by
# the translate.php file found on the KMyMoney web-site.
#
# (C) 2007 by Thomas Baumgart
#
# Syntax:
#         stats.pl <path-to-po-file>
#
# The script requires to find the kmymoney2.pot file in the same
# directory in order to check if the po file is based on the current
# pot file.
#
#***************************************************************************
#*   This program is free software; you can redistribute it and/or modify  *
#*   it under the terms of the GNU General Public License as published by  *
#*   the Free Software Foundation; either version 2 of the License, or     *
#*   (at your option) any later version.                                   *
#***************************************************************************
 
# total message counter
my $msgs = 0;

# fuzzy message counter
my $fuzzy = 0;

# translated message counter
my $translated = 0;

# untranslated message counter
my $untranslated = 0;

# state machine
# possible states:
#
# 0 - idle
# 1 - in message
# 2 - in msgid
# 3 - in empty msgstr
# 4 - in msgstr
my $state = 0;

# line counter
my $linecnt = 0;

# filename of file to process
my $fname;

# pot version information
my $potVersion;

# "po file matches current pot file" flag
my $poVersionOk = 0;

if($#ARGV == -1) {
  $fname = "-";
} elsif($#ARGV == 0) {
  $fname = $ARGV[0];
  $fname =~ /^(.*\/)?(.*)\.po$/;
  $basedir = $1;
  $langcode = $2;
} else {
  print STDERR "Can only process one file at a time\n";
  exit 1;
}


$country{"ca"} = "Catalan";
$country{"cs"} = "Czech";
$country{"da"} = "Denmark";
$country{"de"} = "Germany";
$country{"en_GB"} = "Great Britain";
$country{"es"} = "Spain";
$country{"es_AR"} = "Argentina";
$country{"fi"} = "Finnland";
$country{"fr"} = "France";
$country{"gl"} = "Galicia";
$country{"it"} = "Italy";
$country{"lt"} = "Lithuania";
$country{"nl"} = "Netherlands";
$country{"pl"} = "Poland";
$country{"pt_BR"} = "Brazil";
$country{"pt"} = "Portugal";
$country{"ro"} = "Romania";
$country{"ru"} = "Russia";
$country{"sk"} = "Slovakia";
$country{"sl"} = "Slovenia";
$country{"sv"} = "Sweden";
$country{"tr"} = "Turkey";
$country{"zh_CN"} = "China";

$language{"ca"} = "Catalan";
$language{"cs"} = "Czech";
$language{"da"} = "Danish";
$language{"de"} = "German";
$language{"en_GB"} = "British";
$language{"es"} = "Spanish (Spain)";
$language{"es_AR"} = "Spanish (Argentina)";
$language{"fi"} = "Suomi";
$language{"fr"} = "French";
$language{"gl"} = "Galician";
$language{"it"} = "Italian";
$language{"lt"} = "Lithuanian";
$language{"nl"} = "Dutch";
$language{"pl"} = "Polish";
$language{"pt_BR"} = "Portuguese (Brazil)";
$language{"pt"} = "Portuguese (Portugal)";
$language{"ro"} = "Romanian";
$language{"ru"} = "Russian";
$language{"sk"} = "Slovak";
$language{"sl"} = "Slovenian";
$language{"sv"} = "Swedish";
$language{"tr"} = "Turkish";
$language{"zh_CN"} = "Simplified Chinese";

$flags{"es_AR"} = "ar";
$flags{"en_GB"} = "gb";
$flags{"pt_BR"} = "br";
$flags{"pt"} = "pt";
$flags{"da"} = "dk";
$flags{"gl"} = "Galicia";
$flags{"zh_CN"} = "cn";

open(IN, "< $basedir/kmymoney2.pot") or die("POT file not found.");
while(<IN>) {
  if($_ =~ /POT-Creation-Date/) {
    chomp($_);
    $potVersion = $_;
    last;
  }
}
close IN;

$potVersionOk="0";

open(IN, "< $fname") or die("Cannot open $fname for reading");
while(<IN>) {
  $linecnt++;
  if($_ =~ /Language-Team: (.*) <.*>/) {
    $language = $1;
    next;
  }
  if($_ =~ /Last-Translator: (.*) <.*>/) {
    $author = $1;
    next;
  }
  if($_ =~ /POT-Creation-Date/) {
    chomp($_);
    $potVersionOk = "1" if($_ eq $potVersion);
    next;
  }
  # "PO-Revision-Date: 2007-02-21 21:57+0100\n"
  if($_ =~ /PO-Revision-Date: (\d+-\d+-\d+) /) {
    $poRevision = $1;
  }

  if($state == 0) {
    if($_ =~ /^\#:/) {
      $state = 1;
    }
  } elsif($state == 1) {
    if($_ =~ /^msgid /) {
      $msgs++;
      $state = 2;

    } elsif($_ =~ /^#, fuzzy/) {
      $fuzzy++;
    }
    
  } elsif($state == 2) {
    if($_ =~ /^msgstr ""$/) {
      # we have detected an emtpy msgstr. this can have two reasons
      # a) the message is untranslated, then the next line is empty
      # b) the message is too long and starts on the next line
      $state = 3;
    } elsif($_ =~ /^msgstr "[^"]/) {
      $translated++;
      $state = 4;
    }
  } elsif($state == 3) {
    if($_ =~ /^"[^"]/) {
     $translated++;
      $state = 4;
    } else {
     $untranslated++;
     $state = 0;
    }
  } elsif($state == 4) {
    chomp($_);
    if($_ =~ /^$/) {
      $state = 0;
    }
  }
}
close IN;

my $flag = $flags{$langcode};
$flag = $langcode if(length($flag) == 0);

$ptransdisp = (int (($translated - $fuzzy) * 10000 / $msgs)) / 100; 
;
$pfuzzy = int (($fuzzy * 100) / $msgs);
$puntrans = int (($untranslated * 100) / $msgs);

print "<translation>\n";
print " <flag>$flag</flag>\n";
print " <language>$language{$langcode}</language>\n";
print " <country>$country{$langcode}</country>\n";
print " <translator>$author</translator>\n";
print " <translated>$ptransdisp</translated>\n";
print " <fuzzy>$pfuzzy</fuzzy>\n";
print " <untranslated>$puntrans</untranslated>\n";
print " <potcurrent>$potVersionOk</potcurrent>\n";
print " <porevision>$poRevision</porevision>\n";
print "</translation>\n";

