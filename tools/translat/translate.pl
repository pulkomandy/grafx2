#!/usr/bin/perl
# Translate words in file(s) given on command-line. Saves older as .bak
# The dictionary is read as 2 files, french.txt and english.txt
# Method: regexp search, ^word then ^non-word then ^word then ...
# Each word is translated according to a hash dictionary

# Released to public domain.

unless ( open (FRENCH, "french.txt")) {
  die "Impossible to open 'french.txt' file.\n";
}
unless ( open (ENGLISH, "english.txt")) {
  die "Impossible to open 'english.txt' file.\n";
}

# Build french->english dictionary (a hash)
while(($french = <FRENCH>) & ($english=<ENGLISH>)){
  chop($french);
  chop($english);
  if ("$french" ne "$english")
  {
    $dictionary{$french} = $english;
  }
}

#foreach $name (@ARGV) {
#print "$name";

$^I = ".bak";
# Read input
while ($line = <>){
  chop $line;
  while ($line ne "") {
    # word
    if ($line =~ m/^(\w+)(.*)$/) {
      if (defined $dictionary{$1}) {
        print $dictionary{$1};
      }
      else {
        print "$1";
      }
      $line=$2;
    }
    # non-word
    if ($line =~ m/^([^\w]+)(.*)$/) {
      print "$1";
      $line=$2;
    }
  }
  print "\n";
}
#}
