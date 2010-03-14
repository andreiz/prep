dnl
dnl $ Id: $
dnl vim:se ts=2 sw=2 et:

PHP_ARG_ENABLE(prep, whether to enable prep functions,
[  --enable-prep         Enable prep support])

if test "$PHP_PREP" != "no"; then
  export OLD_CPPFLAGS="$CPPFLAGS"
  export CPPFLAGS="$CPPFLAGS $INCLUDES -DHAVE_PREP"

  AC_MSG_CHECKING(PHP version)
  AC_TRY_COMPILE([#include <php_version.h>], [
#if PHP_VERSION_ID < 50300
#error  this extension requires at least PHP version 5.3.0
#endif
],
[AC_MSG_RESULT(ok)],
[AC_MSG_ERROR([need at least PHP 5.3.0])])

  export CPPFLAGS="$OLD_CPPFLAGS"

  PHP_SUBST(PREP_SHARED_LIBADD)
  AC_DEFINE(HAVE_PREP, 1, [ ])

  PHP_NEW_EXTENSION(prep, prep.c, $ext_shared)

fi

