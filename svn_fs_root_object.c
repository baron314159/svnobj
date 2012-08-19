/*
    +--------------------------------------------------------------------+
    | svnobj                                                             |
    +--------------------------------------------------------------------+
    | Redistribution and use in source and binary forms, with or without |
    | modification, are permitted provided that the conditions mentioned |
    | in the accompanying LICENSE file are met.                          |
    +--------------------------------------------------------------------+
    | Copyright (c) 2009, Christopher Utz <cutz@chrisutz.com>            |
    +--------------------------------------------------------------------+
*/

/* $Id: svn_fs_root_object.c 82 2009-05-11 01:53:09Z utz.m.christopher $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_svnobj_std_defs.h"
#include "svn_exception_object.h"
#include "svn_fs_root_object.h"
#include "svn_php_utils.h"

#ifdef HAVE_SVNOBJ

#define SVNFSROOT_ME(method, visibility) PHP_ME(SvnFsRoot, method, SVNOBJ_ARGS(SvnFsRoot, method), visibility)
#define SVNFSROOT_BEGIN_ARGS(method, req_args) SVNOBJ_BEGIN_ARG_INFO_EX(SvnFsRoot, method, 0, req_args)
#define SVNFSROOT_END_ARGS() SVNOBJ_END_ARG_INFO()
#define SVNFSROOT_CONST_LONG(name, value) zend_declare_class_constant_long(svn_fs_root_object_ce, ZEND_STRL(#name), value TSRMLS_CC)
#define SVNFSROOT_CONST_STRING(name, value) zend_declare_class_constant_stringl(svn_fs_root_object_ce, ZEND_STRL(#name), ZEND_STRL(value) TSRMLS_CC)

#	define SVNFSROOT_FETCH_THIS_AND_INTERN() \
	zval *this = getThis(); \
	svn_fs_root_object *intern = zend_object_store_get_object(this TSRMLS_CC);

zend_class_entry *svn_fs_root_object_ce;
static zend_object_handlers svn_fs_root_object_handlers;

SVNFSROOT_BEGIN_ARGS(__construct, 0)
SVNFSROOT_END_ARGS()

SVNFSROOT_BEGIN_ARGS(isFile, 1)
	SVNOBJ_ARG_VAL(path, 0)
SVNFSROOT_END_ARGS()

SVNFSROOT_BEGIN_ARGS(isDir, 1)
	SVNOBJ_ARG_VAL(path, 0)
SVNFSROOT_END_ARGS()

SVNFSROOT_BEGIN_ARGS(checkPath, 1)
	SVNOBJ_ARG_VAL(path, 0)
SVNFSROOT_END_ARGS()

SVNFSROOT_BEGIN_ARGS(getFileLength, 1)
	SVNOBJ_ARG_VAL(path, 0)
SVNFSROOT_END_ARGS()

SVNFSROOT_BEGIN_ARGS(makeDir, 1)
	SVNOBJ_ARG_VAL(path, 0)
SVNFSROOT_END_ARGS()

SVNFSROOT_BEGIN_ARGS(makeFile, 1)
	SVNOBJ_ARG_VAL(path, 0)
SVNFSROOT_END_ARGS()

static zend_function_entry svn_fs_root_object_fe[] = {
	SVNFSROOT_ME(__construct, ZEND_ACC_PRIVATE | ZEND_ACC_CTOR)
	SVNFSROOT_ME(isFile, ZEND_ACC_PUBLIC)
	SVNFSROOT_ME(isDir, ZEND_ACC_PUBLIC)
	SVNFSROOT_ME(checkPath, ZEND_ACC_PUBLIC)
	SVNFSROOT_ME(getFileLength, ZEND_ACC_PUBLIC)
	SVNFSROOT_ME(makeDir, ZEND_ACC_PUBLIC)
	SVNFSROOT_ME(makeFile, ZEND_ACC_PUBLIC)
	{ NULL, NULL, NULL }
};

static void _svn_fs_root_object_free_storage(void *object TSRMLS_DC) /* {{{ */
{
	svn_fs_root_object *intern = (svn_fs_root_object *) object;

	if (!intern) {
		return;
	}

	if (intern->pool) {
		apr_pool_destroy(intern->pool);
		intern->pool = NULL;
	}

	/* requires PHP >= 5.1.2 */
	zend_object_std_dtor(&intern->zo TSRMLS_CC);

	efree(intern);
}
/* }}} */

static zend_object_value _svn_fs_root_object_new(zend_class_entry *class_type TSRMLS_DC) /* {{{ */
{
	zend_object_value retval;
	svn_fs_root_object *intern;

	intern = emalloc(sizeof(svn_fs_root_object));
	memset(&intern->zo, 0, sizeof(zend_object));

	intern->pool = NULL;
	intern->root = NULL;

	apr_pool_create(&intern->pool, NULL);

	SVNOBJ_STD_NEW_OBJECT(intern, class_type, retval, svn_fs_root_object);
	return retval;
}
/* }}} */

PHP_MINIT_FUNCTION(svn_fs_root) /* {{{ */
{
	SVNOBJ_REGISTER_CLASS_EX(SvnFsRoot, svn_fs_root_object, NULL, 0);
	SVNOBJ_DECLARE_PROPERTY_NULL(svn_fs_root_object, "repos");

	return SUCCESS;
}
/* }}} */

/* {{{ proto private void SvnFsRoot::__construct() */
PHP_METHOD(SvnFsRoot, __construct)
{
}
/* }}} */

/* {{{ proto public bool SvnFsRoot::isFile(string path) */
PHP_METHOD(SvnFsRoot, isFile)
{
	SVNFSROOT_FETCH_THIS_AND_INTERN();
	char *path = NULL;
	int path_len = 0;
	
	svn_error_t *svn_error;
	apr_pool_t *tmp_pool;
	svn_boolean_t is_file;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
		"s",
		&path, &path_len)) {
		RETURN_FALSE;
	}

	apr_pool_create(&tmp_pool, intern->pool);

	path = (char *) svn_normalized_path(path, tmp_pool);

	svn_error = svn_fs_is_file(&is_file,
		intern->root,
		path,
		tmp_pool);

	apr_pool_destroy(tmp_pool);

	if (svn_error) {
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}

	RETURN_BOOL(is_file);
}
/* }}} */

/* {{{ proto public bool SvnFsRoot::isDir(string path) */
PHP_METHOD(SvnFsRoot, isDir)
{
	SVNFSROOT_FETCH_THIS_AND_INTERN();
	char *path = NULL;
	int path_len = 0;
	
	svn_error_t *svn_error;
	apr_pool_t *tmp_pool;
	svn_boolean_t is_dir;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
		"s",
		&path, &path_len)) {
		RETURN_FALSE;
	}

	apr_pool_create(&tmp_pool, intern->pool);

	path = (char *) svn_normalized_path(path, tmp_pool);

	svn_error = svn_fs_is_dir(&is_dir,
		intern->root,
		path,
		tmp_pool);

	apr_pool_destroy(tmp_pool);

	if (svn_error) {
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}

	RETURN_BOOL(is_dir);
}
/* }}} */

/* {{{ proto public int SvnFsRoot::checkPath(string path) */
PHP_METHOD(SvnFsRoot, checkPath)
{
	SVNFSROOT_FETCH_THIS_AND_INTERN();
	char *path = NULL;
	int path_len = 0;
	
	svn_error_t *svn_error;
	apr_pool_t *tmp_pool;
	svn_node_kind_t kind;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
		"s",
		&path, &path_len)) {
		RETURN_FALSE;
	}

	apr_pool_create(&tmp_pool, intern->pool);

	path = (char *) svn_normalized_path(path, tmp_pool);

	svn_error = svn_fs_check_path(&kind,
		intern->root,
		path,
		tmp_pool);

	apr_pool_destroy(tmp_pool);

	if (svn_error) {
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}

	RETURN_LONG(kind);
}
/* }}} */

/* {{{ proto public int SvnFsRoot::getFileLength(string path) */
PHP_METHOD(SvnFsRoot, getFileLength)
{
	SVNFSROOT_FETCH_THIS_AND_INTERN();
	char *path = NULL;
	int path_len = 0;
	
	svn_error_t *svn_error;
	apr_pool_t *tmp_pool;
	svn_filesize_t length;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
		"s",
		&path, &path_len)) {
		RETURN_FALSE;
	}

	apr_pool_create(&tmp_pool, intern->pool);

	path = (char *) svn_normalized_path(path, tmp_pool);

	svn_error = svn_fs_file_length(&length,
		intern->root,
		path,
		tmp_pool);

	apr_pool_destroy(tmp_pool);

	if (svn_error) {
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}

	RETURN_LONG(SVN_FILESIZE_T_TO_LONG(length));
}
/* }}} */

/* {{{ proto public bool SvnFsRoot::makeDir(string path) */
PHP_METHOD(SvnFsRoot, makeDir)
{
	SVNFSROOT_FETCH_THIS_AND_INTERN();
	char *path = NULL;
	int path_len = 0;
	
	svn_error_t *svn_error;
	apr_pool_t *tmp_pool;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
		"s",
		&path, &path_len)) {
		RETURN_FALSE;
	}

	apr_pool_create(&tmp_pool, intern->pool);

	path = (char *) svn_normalized_path(path, tmp_pool);

	svn_error = svn_fs_make_dir(intern->root,
		path,
		tmp_pool);

	apr_pool_destroy(tmp_pool);

	if (svn_error) {
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto public bool SvnFsRoot::makeFile(string path) */
PHP_METHOD(SvnFsRoot, makeFile)
{
	SVNFSROOT_FETCH_THIS_AND_INTERN();
	char *path = NULL;
	int path_len = 0;
	
	svn_error_t *svn_error;
	apr_pool_t *tmp_pool;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
		"s",
		&path, &path_len)) {
		RETURN_FALSE;
	}

	apr_pool_create(&tmp_pool, intern->pool);

	path = (char *) svn_normalized_path(path, tmp_pool);

	svn_error = svn_fs_make_file(intern->root,
		path,
		tmp_pool);

	apr_pool_destroy(tmp_pool);

	if (svn_error) {
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}

	RETURN_TRUE;
}
/* }}} */

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
