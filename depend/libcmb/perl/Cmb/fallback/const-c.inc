#define PERL_constant_NOTFOUND	1
#define PERL_constant_NOTDEF	2
#define PERL_constant_ISIV	3
#define PERL_constant_ISNO	4
#define PERL_constant_ISNV	5
#define PERL_constant_ISPV	6
#define PERL_constant_ISPVN	7
#define PERL_constant_ISSV	8
#define PERL_constant_ISUNDEF	9
#define PERL_constant_ISUV	10
#define PERL_constant_ISYES	11

#ifndef NVTYPE
typedef double NV; /* 5.6 and later define NVTYPE, and typedef NV to it.  */
#endif
#ifndef aTHX_
#define aTHX_ /* 5.6 or later define this for threading support.  */
#endif
#ifndef pTHX_
#define pTHX_ /* 5.6 or later define this for threading support.  */
#endif

static int
constant_15 (pTHX_ const char *name, IV *iv_return) {
  /* When generated this function returned values for the list of names given
     here.  However, subsequent manual editing may have added or removed some.
     CMB_OPT_NUMBERS CMB_OPT_OPTION1 CMB_OPT_OPTION2 */
  /* Offset 14 gives the best switch position.  */
  switch (name[14]) {
  case '1':
    if (memEQ(name, "CMB_OPT_OPTION", 14)) {
    /*                             1      */
#ifdef CMB_OPT_OPTION1
      *iv_return = CMB_OPT_OPTION1;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case '2':
    if (memEQ(name, "CMB_OPT_OPTION", 14)) {
    /*                             2      */
#ifdef CMB_OPT_OPTION2
      *iv_return = CMB_OPT_OPTION2;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'S':
    if (memEQ(name, "CMB_OPT_NUMBER", 14)) {
    /*                             S      */
#ifdef CMB_OPT_NUMBERS
      *iv_return = CMB_OPT_NUMBERS;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  }
  return PERL_constant_NOTFOUND;
}

static int
constant_16 (pTHX_ const char *name, IV *iv_return) {
  /* When generated this function returned values for the list of names given
     here.  However, subsequent manual editing may have added or removed some.
     CMB_OPT_NULPARSE CMB_OPT_NULPRINT CMB_OPT_RESERVED CMB_VERSION_LONG */
  /* Offset 13 gives the best switch position.  */
  switch (name[13]) {
  case 'I':
    if (memEQ(name, "CMB_OPT_NULPRINT", 16)) {
    /*                            ^         */
#ifdef CMB_OPT_NULPRINT
      *iv_return = CMB_OPT_NULPRINT;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'O':
    if (memEQ(name, "CMB_VERSION_LONG", 16)) {
    /*                            ^         */
#ifdef CMB_VERSION_LONG
      *iv_return = CMB_VERSION_LONG;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'R':
    if (memEQ(name, "CMB_OPT_NULPARSE", 16)) {
    /*                            ^         */
#ifdef CMB_OPT_NULPARSE
      *iv_return = CMB_OPT_NULPARSE;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'V':
    if (memEQ(name, "CMB_OPT_RESERVED", 16)) {
    /*                            ^         */
#ifdef CMB_OPT_RESERVED
      *iv_return = CMB_OPT_RESERVED;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  }
  return PERL_constant_NOTFOUND;
}

static int
constant_19 (pTHX_ const char *name, IV *iv_return) {
  /* When generated this function returned values for the list of names given
     here.  However, subsequent manual editing may have added or removed some.
     CMB_H_VERSION_MAJOR CMB_H_VERSION_MINOR CMB_H_VERSION_PATCH */
  /* Offset 16 gives the best switch position.  */
  switch (name[16]) {
  case 'J':
    if (memEQ(name, "CMB_H_VERSION_MAJOR", 19)) {
    /*                               ^         */
#ifdef CMB_H_VERSION_MAJOR
      *iv_return = CMB_H_VERSION_MAJOR;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'N':
    if (memEQ(name, "CMB_H_VERSION_MINOR", 19)) {
    /*                               ^         */
#ifdef CMB_H_VERSION_MINOR
      *iv_return = CMB_H_VERSION_MINOR;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'T':
    if (memEQ(name, "CMB_H_VERSION_PATCH", 19)) {
    /*                               ^         */
#ifdef CMB_H_VERSION_PATCH
      *iv_return = CMB_H_VERSION_PATCH;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  }
  return PERL_constant_NOTFOUND;
}

static int
constant (pTHX_ const char *name, STRLEN len, IV *iv_return) {
  /* Initially switch on the length of the name.  */
  /* When generated this function returned values for the list of names given
     in this section of perl code.  Rather than manually editing these functions
     to add or remove constants, which would result in this comment and section
     of code becoming inaccurate, we recommend that you edit this section of
     code, and use it to regenerate a new set of constant functions which you
     then use to replace the originals.

     Regenerate these constant functions by feeding this entire source file to
     perl -x

#!/usr/bin/perl -w
use ExtUtils::Constant qw (constant_types C_constant XS_constant);

my $types = {map {($_, 1)} qw(IV)};
my @names = (qw(CMB_DEBUG CMB_H_VERSION_MAJOR CMB_H_VERSION_MINOR
	       CMB_H_VERSION_PATCH CMB_OPT_DEBUG CMB_OPT_EMPTY CMB_OPT_NULPARSE
	       CMB_OPT_NULPRINT CMB_OPT_NUMBERS CMB_OPT_OPTION1 CMB_OPT_OPTION2
	       CMB_OPT_RESERVED CMB_VERSION CMB_VERSION_LONG FALSE
	       HAVE_OPENSSL_BN_H HAVE_OPENSSL_CRYPTO_H TRUE));

print constant_types(), "\n"; # macro defs
foreach (C_constant ("Cmb", 'constant', 'IV', $types, undef, 3, @names) ) {
    print $_, "\n"; # C constant subs
}
print "\n#### XS Section:\n";
print XS_constant ("Cmb", $types);
__END__
   */

  switch (len) {
  case 4:
    if (memEQ(name, "TRUE", 4)) {
#ifdef TRUE
      *iv_return = TRUE;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 5:
    if (memEQ(name, "FALSE", 5)) {
#ifdef FALSE
      *iv_return = FALSE;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 9:
    if (memEQ(name, "CMB_DEBUG", 9)) {
#ifdef CMB_DEBUG
      *iv_return = CMB_DEBUG;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 11:
    if (memEQ(name, "CMB_VERSION", 11)) {
#ifdef CMB_VERSION
      *iv_return = CMB_VERSION;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 13:
    /* Names all of length 13.  */
    /* CMB_OPT_DEBUG CMB_OPT_EMPTY */
    /* Offset 8 gives the best switch position.  */
    switch (name[8]) {
    case 'D':
      if (memEQ(name, "CMB_OPT_DEBUG", 13)) {
      /*                       ^           */
#ifdef CMB_OPT_DEBUG
        *iv_return = CMB_OPT_DEBUG;
        return PERL_constant_ISIV;
#else
        return PERL_constant_NOTDEF;
#endif
      }
      break;
    case 'E':
      if (memEQ(name, "CMB_OPT_EMPTY", 13)) {
      /*                       ^           */
#ifdef CMB_OPT_EMPTY
        *iv_return = CMB_OPT_EMPTY;
        return PERL_constant_ISIV;
#else
        return PERL_constant_NOTDEF;
#endif
      }
      break;
    }
    break;
  case 15:
    return constant_15 (aTHX_ name, iv_return);
    break;
  case 16:
    return constant_16 (aTHX_ name, iv_return);
    break;
  case 17:
    if (memEQ(name, "HAVE_OPENSSL_BN_H", 17)) {
#ifdef HAVE_OPENSSL_BN_H
      *iv_return = HAVE_OPENSSL_BN_H;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 19:
    return constant_19 (aTHX_ name, iv_return);
    break;
  case 21:
    if (memEQ(name, "HAVE_OPENSSL_CRYPTO_H", 21)) {
#ifdef HAVE_OPENSSL_CRYPTO_H
      *iv_return = HAVE_OPENSSL_CRYPTO_H;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  }
  return PERL_constant_NOTFOUND;
}

