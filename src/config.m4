dnl $Id$
dnl config.m4 for extension rockets

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(rockets, for rockets support,
dnl Make sure that the comment is aligned:
dnl [  --with-rockets             Include rockets support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(rockets, whether to enable rockets support,
dnl Make sure that the comment is aligned:
[  --enable-rockets           Enable rockets support])

if test "$PHP_ROCKETS" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-rockets -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/rockets.h"  # you most likely want to change this
  dnl if test -r $PHP_ROCKETS/$SEARCH_FOR; then # path given as parameter
  dnl   ROCKETS_DIR=$PHP_ROCKETS
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for rockets files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       ROCKETS_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$ROCKETS_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the rockets distribution])
  dnl fi

  dnl # --with-rockets -> add include path
  dnl PHP_ADD_INCLUDE($ROCKETS_DIR/include)

  dnl # --with-rockets -> check for lib and symbol presence
  dnl LIBNAME=rockets # you may want to change this
  dnl LIBSYMBOL=rockets # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $ROCKETS_DIR/lib, ROCKETS_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_ROCKETSLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong rockets lib version or lib not found])
  dnl ],[
  dnl   -L$ROCKETS_DIR/lib -lm
  dnl ])
  dnl
  dnl PHP_SUBST(ROCKETS_SHARED_LIBADD)

  PHP_NEW_EXTENSION(rockets, rockets.c, $ext_shared)
fi
