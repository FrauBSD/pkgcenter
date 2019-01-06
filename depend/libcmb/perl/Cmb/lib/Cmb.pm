package Cmb;

use 5.008005;
use strict;
use warnings;
use Carp;

require Exporter;
use AutoLoader;

our @ISA = qw(Exporter);

# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# This allows declaration	use Cmb ':all';
# If you do not need this, moving things directly into @EXPORT or @EXPORT_OK
# will save memory.
our %EXPORT_TAGS = ( 'all' => [ qw(
	CMB_DEBUG
	CMB_VERSION
	CMB_VERSION_LONG
	FALSE
	HAVE_OPENSSL_BN_H
	TRUE
) ] );

our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our @EXPORT = qw(
	CMB_DEBUG
	CMB_VERSION
	CMB_VERSION_LONG
	FALSE
	HAVE_OPENSSL_BN_H
	TRUE
);

our $VERSION = '0.01';

sub AUTOLOAD {
    # This AUTOLOAD is used to 'autoload' constants from the constant()
    # XS function.

    my $constname;
    our $AUTOLOAD;
    ($constname = $AUTOLOAD) =~ s/.*:://;
    croak "&Cmb::constant not defined" if $constname eq 'constant';
    my ($error, $val) = constant($constname);
    if ($error) { croak $error; }
    {
	no strict 'refs';
	# Fixed between 5.005_53 and 5.005_61
#XXX	if ($] >= 5.00561) {
#XXX	    *$AUTOLOAD = sub () { $val };
#XXX	}
#XXX	else {
	    *$AUTOLOAD = sub { $val };
#XXX	}
    }
    goto &$AUTOLOAD;
}

require XSLoader;
XSLoader::load('Cmb', $VERSION);

# Preloaded methods go here.

# Autoload methods go after =cut, and are processed by the autosplit program.

1;
__END__
# Below is stub documentation for your module. You'd better edit it!

=head1 NAME

Cmb - Perl extension for cmb(3) combinatorics library

=head1 SYNOPSIS

  use Cmb;

  # Choose-2 from 3:
  my $cmb = new Cmb { size_min => 2, size_max => 2 };
  $cmb->cmb(3, [qw/a b c/]);

  # Alternatively:
  my $cmb = new Cmb;
  $cmb->config({ size_min => 2, size_max => 2});
  $cmb->cmb(3, [qw/a b c/]);

  # Result:
  a b
  a c
  b c

=head1 DESCRIPTION

Stub documentation for Cmb, created by h2xs. It looks like the
author of the extension was negligent enough to leave the stub
unedited.

Blah blah blah.

=head2 EXPORT

None by default.

=head2 Exportable constants

  CMB_DEBUG
  CMB_VERSION
  CMB_VERSION_LONG
  FALSE
  HAVE_OPENSSL_BN_H
  TRUE



=head1 SEE ALSO

cmb(3)

=head1 AUTHOR

Devin Teske, E<lt>dteske@freebsd.orgE<gt>

=head1 COPYRIGHT AND LICENSE

Copyright (c) 2018-2019 Devin Teske

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.


=cut
