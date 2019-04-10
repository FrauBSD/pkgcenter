# Before 'make install' is performed this script should be runnable with
# 'make test'. After 'make install' it should work as 'perl Cmb.t'

#########################

# change 'tests => 2' to 'tests => last_test_to_print';

use strict;
use warnings;

use Test::More tests => 2;
BEGIN { use_ok('Cmb') };


my $fail = 0;
foreach my $constname (qw(
	CMB_DEBUG CMB_H_VERSION_MAJOR CMB_H_VERSION_MINOR CMB_H_VERSION_PATCH
	CMB_OPT_DEBUG CMB_OPT_EMPTY CMB_OPT_NULPARSE CMB_OPT_NULPRINT
	CMB_OPT_NUMBERS CMB_OPT_OPTION1 CMB_OPT_OPTION2 CMB_OPT_RESERVED
	CMB_VERSION CMB_VERSION_LONG FALSE HAVE_OPENSSL_BN_H
	HAVE_OPENSSL_CRYPTO_H TRUE)) {
  next if (eval "my \$a = $constname; 1");
  if ($@ =~ /^Your vendor has not defined Cmb macro $constname/) {
    print "# pass: $@";
  } else {
    print "# fail: $@";
    $fail = 1;
  }

}

ok( $fail == 0 , 'Constants' );
#########################

# Insert your test code below, the Test::More module is use()ed here so read
# its man page ( perldoc Test::More ) for help writing this test script.

