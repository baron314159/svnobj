dnl $Id: config.m4 81 2009-05-10 16:35:25Z utz.m.christopher $

PHP_ARG_WITH(svnobj, [for SvnObj support],
[  --with-svnobj[=DIR]       Include SvnObj support. DIR is the subversion base directory.])
PHP_ARG_WITH(svn-apr-config, [for specified location of the apr-config script], 
[  --with-svn-apr-config=PATH SVNOBJ: Location of the apr-config script used to compile subversion.
                                     If unspecified, default locations are searched], , no)

if test "$PHP_SVNOBJ" != "no"; then
  SVNOBJ_SOURCES="svnobj.c \
  svn_php_utils.c \
	svn_exception_object.c \
	svn_auth_provider_object.c \
	svn_config_object.c \
	svn_auth_baton_object.c \
	svn_client_object.c \
  svn_fs_object.c \
  svn_fs_txn_object.c \
  svn_fs_root_object.c \
  svn_repos_object.c"

  if test -r $PHP_SVNOBJ/include/subversion-1/svn_client.h; then
    SVN_DIR=$PHP_SVNOBJ
  else
    AC_MSG_CHECKING(for subversion libs in default path)
    for i in /usr/local /usr; do
      if test -r $i/include/subversion-1/svn_client.h; then
        SVN_DIR=$i
        AC_MSG_RESULT(found in $i)
        break
      fi
    done
  fi

  if test -z "$SVN_DIR"; then
    AC_MSG_RESULT(not found)
    AC_MSG_ERROR(Please reinstall subversion - svn_client.h should be in 
    <svn-dir>/include/subversion-1/)
  fi

  if test -r $PHP_SVN_APR_CONFIG; then
    SVN_APR_CONFIG=$PHP_SVN_APR_CONFIG
  else
    AC_MSG_CHECKING(for apr-config in default path)
    for i in /usr/local/bin /usr/bin; do
      if test -r $i/apr-config; then
        SVN_APR_CONFIG=$i/apr-config
        AC_MSG_RESULT(found apr-config in $i)
        break
      fi
      if test -r $i/apr-1-config; then
        SVN_APR_CONFIG=$i/apr-1-config
        AC_MSG_RESULT(found apr-1-config in $i)
        break
      fi
    done
  fi

  if test -z "$SVN_APR_CONFIG"; then
    AC_MSG_RESULT(not found)
    AC_MSG_ERROR([Specify the location of the apr-config script used to compile subversion])
  fi

  PHP_NEW_EXTENSION(svnobj, $SVNOBJ_SOURCES, $ext_shared,, \\$(SVNOBJ_CFLAGS))

  AC_DEFINE(HAVE_SVNOBJ, 1, [Whether you have svnobj])

  SVNOBJ_CFLAGS=`$SVN_APR_CONFIG --cflags --cppflags`
  SVNOBJ_APR_INCLUDES=`$SVN_APR_CONFIG --includedir`
  PHP_ADD_INCLUDE($SVNOBJ_APR_INCLUDES)

  PHP_ADD_INCLUDE($SVN_DIR/include/subversion-1)
  PHP_ADD_LIBRARY_WITH_PATH(svn_client-1, $SVN_DIR/lib, SVNOBJ_SHARED_LIBADD)
  PHP_ADD_LIBRARY_WITH_PATH(svn_repos-1, $SVN_DIR/lib, SVNOBJ_SHARED_LIBADD)
  PHP_ADD_LIBRARY_WITH_PATH(svn_subr-1, $SVN_DIR/lib, SVNOBJ_SHARED_LIBADD)
  PHP_ADD_LIBRARY_WITH_PATH(svn_wc-1, $SVN_DIR/lib, SVNOBJ_SHARED_LIBADD)
  PHP_ADD_LIBRARY_WITH_PATH(svn_fs-1, $SVN_DIR/lib, SVNOBJ_SHARED_LIBADD)
  PHP_ADD_LIBRARY_WITH_PATH(svn_repos-1, $SVN_DIR/lib, SVNOBJ_SHARED_LIBADD)

  PHP_SUBST(SVNOBJ_CFLAGS)
  PHP_SUBST(SVNOBJ_SHARED_LIBADD)
fi
