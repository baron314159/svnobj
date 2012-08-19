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

/* $Id: svn_fs_object.c 81 2009-05-10 16:35:25Z utz.m.christopher $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_svnobj_std_defs.h"
#include "svn_fs_object.h"
#include "svn_fs_txn_object.h"
#include "svn_fs_root_object.h"
#include "svn_exception_object.h"

#ifdef HAVE_SVNOBJ

#define SVNFS_ME(method, visibility) PHP_ME(SvnFs, method, SVNOBJ_ARGS(SvnFs, method), visibility)
#define SVNFS_BEGIN_ARGS(method, req_args) SVNOBJ_BEGIN_ARG_INFO_EX(SvnFs, method, 0, req_args)
#define SVNFS_END_ARGS() SVNOBJ_END_ARG_INFO()
#define SVNFS_CONST_LONG(name, value) zend_declare_class_constant_long(svn_fs_object_ce, ZEND_STRL(#name), value TSRMLS_CC)
#define SVNFS_CONST_STRING(name, value) zend_declare_class_constant_stringl(svn_fs_object_ce, ZEND_STRL(#name), ZEND_STRL(value) TSRMLS_CC)

zend_class_entry *svn_fs_object_ce;
static zend_object_handlers svn_fs_object_handlers;

SVNFS_BEGIN_ARGS(__construct, 0)
SVNFS_END_ARGS()

SVNFS_BEGIN_ARGS(beginTxn, 1)
	SVNOBJ_ARG_VAL(rev, 0)
	SVNOBJ_ARG_VAL(flags, 0)
SVNFS_END_ARGS()

SVNFS_BEGIN_ARGS(getRevisionRoot, 1)
	SVNOBJ_ARG_VAL(rev, 0)
SVNFS_END_ARGS()

SVNFS_BEGIN_ARGS(getYoungestRev, 0)
SVNFS_END_ARGS()

static zend_function_entry svn_fs_object_fe[] = {
	SVNFS_ME(__construct, ZEND_ACC_PRIVATE | ZEND_ACC_CTOR)
	SVNFS_ME(beginTxn, ZEND_ACC_PUBLIC)
	SVNFS_ME(getRevisionRoot, ZEND_ACC_PUBLIC)
	SVNFS_ME(getYoungestRev, ZEND_ACC_PUBLIC)
	{ NULL, NULL, NULL }
};

static void _svn_fs_object_free_storage(void *object TSRMLS_DC) /* {{{ */
{
	svn_fs_object *intern = (svn_fs_object *) object;

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

static zend_object_value _svn_fs_object_new(zend_class_entry *class_type TSRMLS_DC) /* {{{ */
{
	zend_object_value retval;
	svn_fs_object *intern;

	intern = emalloc(sizeof(svn_fs_object));
	memset(&intern->zo, 0, sizeof(zend_object));

	intern->pool = NULL;
	intern->fs = NULL;

	apr_pool_create(&intern->pool, NULL);

	SVNOBJ_STD_NEW_OBJECT(intern, class_type, retval, svn_fs_object);
	return retval;
}
/* }}} */

PHP_MINIT_FUNCTION(svn_fs) /* {{{ */
{
	SVNOBJ_REGISTER_CLASS_EX(SvnFs, svn_fs_object, NULL, 0);
	SVNOBJ_DECLARE_PROPERTY_NULL(svn_fs_object, "repos");

	return SUCCESS;
}
/* }}} */

/* {{{ proto private void SvnFs::__construct() */
PHP_METHOD(SvnFs, __construct)
{
}
/* }}} */

/* {{{ proto public SvnFsTxn SvnFs::beginTxn(int rev [, int flags]) */
PHP_METHOD(SvnFs, beginTxn)
{
	zval *repos_zval;
	long rev = 0, flags = 0;

	zval *this = getThis();
	svn_fs_object *fs_intern = zend_object_store_get_object(this TSRMLS_CC);
	svn_fs_txn_t *fs_txn;
	svn_error_t *svn_error;
	svn_fs_txn_object *fs_txn_intern;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"l|l", 
		&rev,
		&flags)) {
		RETURN_FALSE;
	}

	object_init_ex(return_value, svn_fs_txn_object_ce);
	fs_txn_intern = zend_object_store_get_object(return_value TSRMLS_CC);

	svn_error = svn_fs_begin_txn2(&fs_txn,
		fs_intern->fs,
		LONG_TO_SVN_REVNUM_T(rev),
		LONG_TO_APR_UINT32_T(flags),
		fs_txn_intern->pool);

	if (svn_error) {
		zval_dtor(return_value);
		RETVAL_FALSE;
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}

	repos_zval = zend_read_property(svn_fs_object_ce,
		this,
		ZEND_STRL("repos"),
		0 TSRMLS_CC);

	zend_update_property(svn_fs_txn_object_ce,
		this,
		ZEND_STRL("repos"),
		repos_zval TSRMLS_CC);

	fs_txn_intern->txn = fs_txn;
}
/* }}} */

/* {{{ proto public SvnFsRoot SvnFs::getRevisionRoot(int rev) */
PHP_METHOD(SvnFs, getRevisionRoot)
{
	zval *repos_zval;
	long rev = 0;

	zval *this = getThis();
	svn_fs_object *fs_intern = zend_object_store_get_object(this TSRMLS_CC);
	svn_fs_root_t *fs_root;
	svn_error_t *svn_error;
	svn_fs_root_object *fs_root_intern;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"l", 
		&rev)) {
		RETURN_FALSE;
	}

	object_init_ex(return_value, svn_fs_root_object_ce);
	fs_root_intern = zend_object_store_get_object(return_value TSRMLS_CC);

	svn_error = svn_fs_revision_root(&fs_root,
		fs_intern->fs,
		rev,
		fs_root_intern->pool);

	if (svn_error) {
		zval_dtor(return_value);
		RETVAL_FALSE;
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}

	repos_zval = zend_read_property(svn_fs_object_ce,
		this,
		ZEND_STRL("repos"),
		0 TSRMLS_CC);

	zend_update_property(svn_fs_root_object_ce,
		return_value,
		ZEND_STRL("repos"),
		repos_zval TSRMLS_CC);

	fs_root_intern->root = fs_root;
}
/* }}} */

/* {{{ proto public int SvnFs::getYoungestRev() */
PHP_METHOD(SvnFs, getYoungestRev)
{
	zval *this = getThis();

	svn_revnum_t youngest_rev;
	apr_pool_t *tmp_pool;
	svn_fs_object *intern = zend_object_store_get_object(this TSRMLS_CC);
	svn_error_t *svn_error;

	apr_pool_create(&tmp_pool, intern->pool);

	svn_error = svn_fs_youngest_rev(&youngest_rev,
		intern->fs,
		tmp_pool);

	apr_pool_destroy(tmp_pool);

	if (svn_error) {
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}

	RETURN_LONG(SVN_REVNUM_T_TO_LONG(youngest_rev));
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
