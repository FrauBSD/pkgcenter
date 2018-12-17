#!/usr/bin/env perl
use strict;
use warnings;
use Cmb;

our $total = 0;
my $choice = 2;
my $num = 10000;

my @items = (1..$num);
my $cmb = new Cmb { size_min => $choice, size_max => $choice };

printf "Silently enumerating choose-%u from %u:\n", $choice, $num;
$cmb->cmb_callback($#items + 1, \@items, sub { $total++; return 0 });
printf "%u callbacks executed\n", $total;
