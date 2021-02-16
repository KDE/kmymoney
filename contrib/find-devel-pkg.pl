#!/usr/bin/perl
#
# SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
#
# ---
#
# Figure out the -devel packages needed to compile KMyMoney
#
# Must be started in a git repo in the base directory


use Getopt::Long;

my $packageManager;
my %headerNames;
my %packageNames;
my %nonPackagedFiles;
my @blacklistedPatterns;
my @packageManagers;

# don't report old stuff and our own files
@blacklistedPatterns = ("KDELibs4Support", "mymoney");

# the directories where to search for header files
@packageIncludeDirectory = ("/usr/include", "/usr/local/include");

# the package managers supported
%packageManagers = ("rpm" => \&findPackage_rpm);

sub printAvailablePackageManagers()
{
    print "The following package managers are available:\n\n";
    for $p (keys %packageManagers) {
        print "$p ";
    }
    print "\n";
}

sub isBlackListed($)
{
    my $absoluteFilename = shift @_;
    for $pattern (@blacklistedPatterns) {
        # print "Check blacklisted $pattern in $absoluteFilename\n";
        if ($absoluteFilename =~ /$pattern/) {
            # print "$absoluteFilename is blacklisted\n";
            return 1;
        }
    }
    return 0;
}

sub getAbsoluteFilenames()
{
    for $h (keys %headerNames) {
        print "Searching $h\n";
        for $incDir (@packageIncludeDirectory) {
            if ($h =~ /^(.+)\/([^\/]+)$/) {
                my $name = $2;
                # include contains a (partial) path
                open(ABS, "find $incDir -name $name |") or die;
                while (<ABS>) {
                    chomp($_);
                    if (index($_, $h) != -1) {
                        next  if (isBlackListed($_) == 1);
                        $headerNames{$h} = $_;
                    }
                }
                close ABS;

            } else {
                open(ABS, "find $incDir -name $h |") or die;
                while (<ABS>) {
                    chomp($_);
                    next  if (isBlackListed($_) == 1);
                    $headerNames{$h} = $_;
                }
                close ABS;
            }
        }
    }
}

# a package manager must return an empty string ""
# in case it did not find a package or the package name
sub findPackage_rpm($)
{
    my $absoluteFilename = shift @_;
    my $packageName = "";
    open(RPM, "rpm -qf $absoluteFilename |") or die();
    while (<RPM>) {
        chomp($_);
        if ($_ !~ /^file /) {
            $packageName = $_;
        }
    }
    close RPM;
    return $packageName
}


# add more package managers above this line and don't forget to add
# them to the packageManagers hash in the config section above

# --------------------------------------------
# Here starts main()
# --------------------------------------------

GetOptions('packageManager=s' => \$packageManager);

if ($packageManager eq "") {
    print "No package manager selected: Please specify with option -p (e.g. -p rpm)\n";
    printAvailablePackageManagers();
    exit 1;
}

for $p (keys %packageManagers) {
    if ($p eq $packageManager) {
        $usePackageManager = $p;
        last;
    }
}

if ($usePackageManager eq "") {
    print "Selected package manager '$packageManager' is not supported. Please implement.\n";
    printAvailablePackageManagers();
    exit 1;
}


open(INCLUDES, "git grep -h '#include <' | sort | uniq |") or die;
# the next line is for debugging only
# open(INCLUDES, "cat includes |") or die;
while(<INCLUDES>) {
    # skip commented includes
    next if ($_ =~ /^\s*\/\/\s*#inc/);
    $_ =~ /<([^>]+)>/;
    $headerNames{$1} = $1;
}
close INCLUDES;

getAbsoluteFilenames();

for $h (keys %headerNames) {
    my $pkg;
    print "Locate package for $h - ";
    $pkg = $packageManagers{$usePackageManager}->($headerNames{$h});
    print "$pkg\n";
    # print "$h - $headerNames{$h}- $pkg\n";
    if ($pkg eq "") {
        $nonPackagedFiles{$headerNames{$h}} = $pkg;
    } else {
        $packageNames{$pkg} = $pkg;
    }

}

print "\n\n";
print "List of required packages:\n";
print "--------------------------\n";

for $p (sort keys %packageNames) {
    print "$p\n";
}

print "\n\n";
print "List of non-packaged files:\n";
print "---------------------------\n";

for $p (sort keys %nonPackagedFiles) {
    print "$p\n";
}
