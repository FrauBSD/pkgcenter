#!/usr/bin/env perl
use strict;
use warnings;
use Cmb;

my @items = (1..10000);
my $choose = 2;
my $nitems = $#items + 1;

my $cmb = new Cmb { size_min => $choose, size_max => $choose };
my $total = $cmb->count($nitems);
open our $DPV, "|-", "dpv -l $total:perl" or die "dpv: $!";
END { close $DPV }

$cmb->cmb_callback($nitems, \@items, sub { print $DPV "@_\n"; return 0 });
