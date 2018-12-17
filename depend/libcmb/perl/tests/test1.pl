#!/usr/bin/env perl
use strict;
use warnings;
use Cmb;

my $config = {
	debug => 0,
	nul_terminate => 0,
	show_empty => 0,
	show_numbers => 1,
	delimiter => ",",
	prefix => "\t\t[",
	suffix => "]",
	size_min => 2,
	size_max => 3,
	count => 6,
	start => 4,
};
print "my \$config = {\n";
print join "", map { qq/\t$_ => "$config->{$_}",\n/ } sort keys %$config;
print "};\n";


my $vers = 1; # Long version
my @items = qw/a b c d/;
my $count = $#items + 1;
my $ilist = join ", ", @items;
my $seq = 1;
my $res;

#
# Method A:
#
printf "==> Method A:\n";
my $cmbA = new Cmb $config;
printf "\tnew Cmb created\n";
printf "\tCmb::cmb_version(): %s\n", Cmb::cmb_version();
printf "\tCmb::cmb_version(%i): %s\n", $vers, Cmb::cmb_version($vers);
printf "\tsize_min=%u size_max=%u\n", $config->{size_min}, $config->{size_max};
printf "\tCmb::cmb_count(config, %u) = %u\n", $count,
	Cmb::cmb_count($cmbA, $count);
printf "\tCmb::print(config, %u, %u, [%s]):\n", $seq, $count, $ilist;
$res = Cmb::print($cmbA, $seq, $count, \@items);
printf "\t\tRESULT: %i\n", $res;
printf "\tCmb::cmb(config, %u, [%s]):\n", $count, $ilist;
printf "\t\tNOTE: { start => %u, count => %u }\n",
	$config->{start}, $config->{count};
$res = Cmb::cmb($cmbA, $count, \@items);
printf "\t\tRESULT: %i\n", $res;

#
# Method B:
#
printf "==> Method B:\n";
my $cmbB = new Cmb;
printf "\tnew Cmb created\n";
$cmbB->config($config);
printf "\t\$cmb->version(): %s\n", $cmbB->version();
printf "\t\$cmb->version(%i): %s\n", $vers, $cmbB->version($vers);
printf "\tsize_min=%u size_max=%u\n", $config->{size_min}, $config->{size_max};
printf "\t\$cmb->count(%u) = %u\n", $count, $cmbB->count($count);
printf "\t\$cmb->print(%u, %u, [%s]):\n", $seq, $count, $ilist;
$res = $cmbB->print($seq, $count, \@items);
printf "\t\tRESULT: %i\n", $res;
printf "\t\$cmb->cmb(%u, [%s]):\n", $count, $ilist;
printf "\t\tNOTE: start=%u count=%u\n", $config->{start}, $config->{count};
$res = $cmbB->cmb($count, \@items);
printf "\t\tRESULT: %i\n", $res;

#
# Callback Method:
#
printf "==> Callback Method:\n";
my $cmbC = new Cmb $config;
$cmbC->config({ show_numbers => 0 });
printf "\tcmb->cmb_callback(%u, [%s], sum):\n", $count, $ilist;
our $num_calls = 0;
$res = $cmbC->cmb_callback($count, \@items, sub { $num_calls++; return 0 });
printf "\t\tnum_calls: %i\n", $num_calls;
printf "\t\tRESULT: %i\n", $res;
