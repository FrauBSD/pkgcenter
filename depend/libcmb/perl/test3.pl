#!/usr/bin/env perl
use strict;
use warnings;
use Cmb;

my @items = qw/a b c d/;
my $choose = 2;
my $nitems = $#items + 1;

printf STDERR "Enumerating choose-%s from %u:\n", $choose, $nitems;
my $cmb = new Cmb { size_min => $choose, size_max => $choose };
$cmb->cmb_callback($nitems, \@items, sub { printf "\t%s\n", "@_" });
