dnl $Id$
dnl config.m4 for extension astview

PHP_ARG_ENABLE(astview, whether to enable astview support,
[  --disable-astview       Disable astview support], yes)

if test "$PHP_ASTVIEW" != "no"; then
  PHP_NEW_EXTENSION(astview, astview.c, $ext_shared)
fi
