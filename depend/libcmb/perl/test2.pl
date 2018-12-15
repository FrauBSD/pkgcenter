#!/usr/bin/env perl
use strict;
use warnings;
use Cmb;

my $cmb = new Cmb { size_min => 2, size_max => 2 };
my @items = (1..10000);
our $n = 1;
$cmb->cmb_callback($#items+1, \@items, sub { print $n++ . "\n" });
