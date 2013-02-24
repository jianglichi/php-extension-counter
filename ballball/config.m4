dnl $Id$
dnl config.m4 for extension ballball

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(ballball, for ballball support,
dnl Make sure that the comment is aligned:
dnl [  --with-ballball             Include ballball support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(ballball, whether to enable ballball support,
Make sure that the comment is aligned:
[  --enable-ballball           Enable ballball support])

if test "$PHP_BALLBALL" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-ballball -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/ballball.h"  # you most likely want to change this
  dnl if test -r $PHP_BALLBALL/$SEARCH_FOR; then # path given as parameter
  dnl   BALLBALL_DIR=$PHP_BALLBALL
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for ballball files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       BALLBALL_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$BALLBALL_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the ballball distribution])
  dnl fi

  dnl # --with-ballball -> add include path
  dnl PHP_ADD_INCLUDE($BALLBALL_DIR/include)

  dnl # --with-ballball -> check for lib and symbol presence
  dnl LIBNAME=ballball # you may want to change this
  dnl LIBSYMBOL=ballball # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $BALLBALL_DIR/lib, BALLBALL_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_BALLBALLLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong ballball lib version or lib not found])
  dnl ],[
  dnl   -L$BALLBALL_DIR/lib -lm -ldl
  dnl ])
  dnl
  dnl PHP_SUBST(BALLBALL_SHARED_LIBADD)

  PHP_NEW_EXTENSION(ballball, ballball.c, $ext_shared)
fi
