use 5.008005;
use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile(
    NAME              => 'Cmb',
    VERSION_FROM      => 'lib/Cmb.pm', # finds $VERSION, requires EU::MM from perl >= 5.5
    PREREQ_PM         => {}, # e.g., Module::Name => 1.1
    ABSTRACT_FROM     => 'lib/Cmb.pm', # retrieve abstract from module
    AUTHOR            => 'Devin Teske <dteske@FreeBSD.org>',
    LICENSE           => 'bsd',
    #Value must be from legacy list of licenses here
    #http://search.cpan.org/perldoc?Module%3A%3ABuild%3A%3AAPI
    LIBS              => [], # e.g., '-lm'
    CCFLAGS           => "-I/usr/local/include",
    dynamic_lib => {
        OTHERLDFLAGS  => "-L/usr/local/lib -lcmb -fstack-protector",
    },
    DEFINE            => '', # e.g., '-DHAVE_SOMETHING'
    INC               => '-I.', # e.g., '-I. -I/usr/include/other'
	# Un-comment this if you add C files to link with later:
    # OBJECT            => '$(O_FILES)', # link all the C files too
);
if  (eval {require ExtUtils::Constant; 1}) {
  # If you edit these definitions to change the constants used by this module,
  # you will need to use the generated const-c.inc and const-xs.inc
  # files to replace their "fallback" counterparts before distributing your
  # changes.
  my @names = (qw(CMB_DEBUG CMB_H_VERSION_MAJOR CMB_H_VERSION_MINOR
		 CMB_H_VERSION_PATCH CMB_OPT_DEBUG CMB_OPT_EMPTY
		 CMB_OPT_NULPARSE CMB_OPT_NULPRINT CMB_OPT_NUMBERS
		 CMB_OPT_OPTION1 CMB_OPT_OPTION2 CMB_OPT_RESERVED CMB_VERSION
		 CMB_VERSION_LONG FALSE HAVE_OPENSSL_BN_H HAVE_OPENSSL_CRYPTO_H
		 TRUE));
  ExtUtils::Constant::WriteConstants(
                                     NAME         => 'Cmb',
                                     NAMES        => \@names,
                                     DEFAULT_TYPE => 'IV',
                                     C_FILE       => 'const-c.inc',
                                     XS_FILE      => 'const-xs.inc',
                                  );

}
else {
  use File::Copy;
  use File::Spec;
  foreach my $file ('const-c.inc', 'const-xs.inc') {
    my $fallback = File::Spec->catfile('fallback', $file);
    copy ($fallback, $file) or die "Can't copy $fallback to $file: $!";
  }
}
