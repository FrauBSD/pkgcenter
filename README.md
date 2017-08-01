[//]: # ($FrauBSD: README.md 2017-07-31 18:23:19 -0700 freebsdfrau $)

# Welcome to FrauBSD pkgcenter!

pkgcenter (pronounced "package center") is a cross-platform framework for
creating and/or remastering native packages for NetBSD, FreeBSD, and
RedHat/CentOS Linux.

## Foreword

The following is required before using `git commit` in this project.

> `$ git config user.name USERNAME`  
> `$ git config user.email USERNAME@fraubsd.org`  
> `$ \ls .git-hooks | xargs -n1 -Ifile ln -sfv ../../.git-hooks/file .git/hooks`

**NOTE:** The leading backslash (e.g., `\ls`) prevents common alias issues  
**NOTE:** Last command should be run from top of project checkout directory

This will ensure the FrauBSD keyword is expanded/updated for each commit.

## Introduction

If you write software and like to share it, you may be interested in the topic
of software packaging.

Software packaging frameworks commonly come in two flavors:

 1. Native
 2. 3rd-party

Some exampes of Native frameworks:

> Platform            | Name      | Install Tool
> ------------------- | --------  | --------------------
> NetBSD              | pkgsrc\*  | `pkg_add` or `pkgin`
> FreeBSD             | ports     | `pkg`
> RedHat/CentOS Linux | rpm       | `rpm` or `yum`
> Mac OS X            | App Store | `App Store.app`
> Debian/Ubuntu Linux | Aptitude  | `dpkg` or `apt-get`

\* Framework has been released for additional platforms but is Native on listed
platform.

Some examples of 3rd-party frameworks:

> Platform   | Name                  | Install Tool
> ---------- | --------------------- | ------------
> Mac OS X   | homebrew              | `brew`
> Mac OS X   | MacPorts              | `port`
> Node.js    | Node Package Manager  | `npm`
> Python     | PIP Installs Packages | `pip`

pkgcenter is a unique framework that blends the above "3rd-party" and "native"
categories to produce a new type -- a framework that is 3rd-party but targets
native install tools.

Using native frameworks to generate packages:

> |                  | NetBSD               | FreeBSD            | RedHat/CentOS Linux
> | ---------------- | -------------------- | ------------------ | -------------------
> | **Input**        | _pkgsrc_ `Makefile`  | _ports_ `Makefile` | `*.src.rpm`
> | **Build Tool**   | `pkg_create`         | `pkg`              | `rpmbuild`
> | **Output**       | `*.tgz`              | `*.txz`            | `*.rpm`
> | **Install Tool** | `pkg_add` or `pkgin` | `pkg`              | `rpm`

Using pkgcenter to generate packages:

> |                  | NetBSD                 | FreeBSD                | RedHat/CentOS Linux
> | ---------------- | ---------------------- | ---------------------- | ------------------------------
> | **Input**        | _pkgcenter_ `Makefile` | _pkgcenter_ `Makefile` | _pkgcenter_ `Makefile`
> | **Build Tool**   | `tar`                  | `tar`                  | `rpmbuild`
> | **Output**       | `*.tgz`                | `*.txz`                | `*.rpm`
> | **Install Tool** | `pkg_add` or `pkgin`   | `pkg`                  | `rpm`

The output and install tools above do not differ. Only the input and build
method changes to become more cross-platform compatible.

pkgcenter has been tested successfully to work with:

> |                       | pkgsrc | ports | pkgcenter
> | --------------------- |:------:|:-----:|:---------:
> | **[b]make (NetBSD)**  | X      | X     | X
> | **[b]make (FreeBSD)** | X      | X     | X
> | **[f]make (FreeBSD)** |        |       | X
> | **make (GNU/Linux)**  |        |       | X
> | **make (Mac OS X)**   |        |       | X

Using the following flavors of UNIX shell:

 * Linux Bourn Again SHell (bash-4.x)
 * Apple Mac OS X shell (bash-3.x)
 * Debian Almquist SHell (dash)
 * FreeBSD bourne shell (sh)

NetBSD/FreeBSD packages are built with `tar` and RPM packages are built with
`rpmbuild` (but you do not need a source RPM file to build one).

Unlike pkgsrc from NetBSD and ports from FreeBSD which focus on:

 * Acquisition
 * Compilation
 * Installation
 * Packaging

pkgcenter instead focuses on:

 * Versioning
 * Auditing
 * Packaging

By not strictly controlling the process of acquisition and compilation, a wider
variety of software can be packaged. The package framework does not need to be
taught how to acquire/compile software each time a new transport/compilation
method is developed.

By relying on the native package managers for installation, the package
framework does not need to know how to install the package, only how to
re/create it.

While it can be said that pkgsrc, ports, and pkgcenter all use a system of
Makefiles and shell scripts, only pkgcenter is cross-platform compatible and
agnostic to `make` and `sh` flavor.

## Versioning

One of the problems with frameworks such as pkgsrc and ports is the way
pre-packaged metadata is stored.

The metadata for the final output (`*.tgz` for pkgsrc and `*.txz` for ports) is
stored in the framework Makefile. This means every time the format of the
Makefile is changed, the metadata may potentially have to be updated to conform
to the new syntax in the framework.

For example, `games/an/Makefile` contains a `COMMENT` variable which sets the
one-line description of the software. This is an example of metadata which gets
compiled into the package. In NetBSD the comment is put inside a file named
`+COMMENT` at the top-level of the `tar` archive while FreeBSD puts it in the
`+MANIFEST` and `+COMPACT_MANIFEST` files.

If the framework is changed to instead change the variable "COMMENT" to "BLURB"
it would require changing each/every Makefile.

In pkgcenter, no such change would ever be required because the metadata is not
stored in the Makefile but instead in the final output format.

For a NetBSD package, the one-line description is stored in the `+COMMENT` file
where it will ultimately end-up and for a FreeBSD package it is in the
precompiled MANIFEST where it belongs.

There is no need to chase framework changes in each software entry because the
metadata is in a format that gets as close to the finalized product as possible.

Changes to software entries are only required if the format expected by the
install tool changes.

Frameworks tend to iterate change faster than the tool used to install packages
built by said framework.

## Auditing

Since the metadata only changes when the install tool changes, it becomes
easier to track entry-specific changes using version control tags.

pkgcenter supports version control software such as CVS, Perforce, and Git.

In the SPECFILE for RPMs, MANIFEST for FreeBSD packages, and PLIST for NetBSD
packages are all the metadata necessary to determine the shipping package
version. Unique to pkgcenter is the ability to say "make tag" (or "make label"
for perforce users) to produce a tag for the current state of tree that
produced the package.

Making it easier to audit old code, you can freely backup to a previous state
of the tree to produce an exact version.

The ability to fast-forward and rewind the version-control system that the
framework lives within is supposed to be an inherited trait but as many have
found, pkgsrc and ports are not designed to work that way.

In reality, you run into serious problems when you try and rewind pkgsrc/ports
to a previous state to produce an old package:

 * `DISTSITE` may no longer be available
 * `Mk` files may become misaligned with software entry `Makefile`
 * `Mk` files too old to produce a working package
 * `pkg` (FreeBSD) or `pkg_create` (NetBSD) too old/new
 * Dependent software entries misaligned

None of these issues affect pkgcenter.

The `Makefile` for every software entry in pkgcenter is byte-for-byte the same.
Any/all changes to the framework itself never touch any file containing
metadata.

These are just some of the ways that pkgcenter improves on audit abilities for
package frameworks.

