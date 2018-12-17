#!/usr/bin/env perl
use strict;
use warnings;
use Cmb;

my @items = (1..10000);
my $choose = 2;
my $nitems = $#items + 1;

our $DPV;
my $total = qx/cmb -k $choose -T $nitems/;
chomp $total;
open $DPV, "|-", "dpv -l $total:-" or die "dpv: $!";

my $cmb = new Cmb { size_min => $choose, size_max => $choose };
$cmb->cmb_callback($nitems, \@items, sub { print $DPV "@_\n" });
close $DPV;
