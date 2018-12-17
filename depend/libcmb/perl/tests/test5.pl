#!/usr/bin/env perl
use strict;
use warnings;
use Cmb;

our $total = 0;
my @items = qw/a b c/;
my $nitems = $#items + 1;
my $cmb = new Cmb;

printf "Testing non-zero callback return:\n";
$cmb->cmb_callback($nitems, \@items, sub { return $total++ });
printf "%u of %u callbacks executed\n", $total, $cmb->count($nitems);;
