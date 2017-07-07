[//]: # ($FrauBSD: README.md 2017-07-06 20:29:14 -0700 freebsdfrau $)

# Welcome to FrauBSD pkgcenter!

**!! ATTENTION DEVELOPERS AND CONTRIBUTORS !!**

The following is required before using `git commit' in this project.

> `$ git config user.name **USERNAME**`
>
> `$ git config user.email **USERNAME@fraubsd.org**`
>
> `$ \ls .git-hooks | xargs -n1 -Ifile ln -sfv ../../.git-hooks/file .git/hooks`

**NOTE:** The leading backslash (e.g., `\ls') prevents common alias issues

**NOTE:** Last command should be run from top of project checkout directory

This will ensure the FrauBSD keyword is expanded/updated for each commit.

