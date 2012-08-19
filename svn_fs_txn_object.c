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

/* $Id: svn_fs_txn_object.c 81 2009-05-10 16:35:25Z utz.m.christopher $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_svnobj_std_defs.h"
#include "svn_repos_object.h"
#include "svn_fs_txn_object.h"
#include "svn_fs_root_object.h"

#ifdef HAVE_SVNOBJ

#define SVNFSTXN_ME(method, visibility) PHP_ME(SvnFsTxn, method, SVNOBJ_ARGS(SvnFsTxn, method), visibility)
#define SVNFSTXN_BEGIN_ARGS(method, req_args) SVNOBJ_BEGIN_ARG_INFO_EX(SvnFsTxn, method, 0, req_args)
#define SVNFSTXN_END_ARGS() SVNOBJ_END_ARG_INFO()
#define SVNFSTXN_CONST_LONG(name, value) zend_declare_class_constant_long(svn_fs_txn_object_ce, ZEND_STRL(#name), value TSRMLS_CC)
#define SVNFSTXN_CONST_STRING(name, value) zend_declare_class_constant_stringl(svn_fs_txn_object_ce, ZEND_STRL(#name), ZEND_STRL(value) TSRMLS_CC)

zend_class_entry *svn_fs_txn_object_ce;
static zend_object_handlers svn_fs_txn_object_handlers;

SVNFSTXN_BEGIN_ARGS(__construct, 0)
SVNFSTXN_END_ARGS()

SVNFSTXN_BEGIN_ARGS(getTxnRoot, 0)
SVNFSTXN_END_ARGS()

SVNFSTXN_BEGIN_ARGS(commitTxn, 0)
SVNFSTXN_END_ARGS()

SVNFSTXN_BEGIN_ARGS(abortTxn, 0)
SVNFSTXN_END_ARGS()

static zend_function_entry svn_fs_txn_object_fe[] = {
	SVNFSTXN_ME(__construct, ZEND_ACC_PRIVATE | ZEND_ACC_CTOR)
	SVNFSTXN_ME(getTxnRoot, ZEND_ACC_PUBLIC)
	SVNFSTXN_ME(commitTxn, ZEND_ACC_PUBLIC)
	SVNFSTXN_ME(abortTxn, ZEND_ACC_PUBLIC)
	{ NULL, NULL, NULL }
};

static void _svn_fs_txn_object_free_storage(void *object TSRMLS_DC) /* {{{ */
{
	svn_fs_txn_object *intern = (svn_fs_txn_object *) object;

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

static zend_object_value _svn_fs_txn_object_new(zend_class_entry *class_type TSRMLS_DC) /* {{{ */
{
	zend_object_value retval;
	svn_fs_txn_object *intern;

	intern = emalloc(sizeof(svn_fs_txn_object));
	memset(&intern->zo, 0, sizeof(zend_object));

	intern->pool = NULL;
	intern->txn = NULL;

	apr_pool_create(&intern->pool, NULL);

	SVNOBJ_STD_NEW_OBJECT(intern, class_type, retval, svn_fs_txn_object);
	return retval;
}
/* }}} */

PHP_MINIT_FUNCTION(svn_fs_txn) /* {{{ */
{
	SVNOBJ_REGISTER_CLASS_EX(SvnFsTxn, svn_fs_txn_object, NULL, 0);
	SVNOBJ_DECLARE_PROPERTY_NULL(svn_fs_txn_object, "repos");

	SVNFSTXN_CONST_LONG(FS_TXN_CHECK_OOD, SVN_FS_TXN_CHECK_OOD);
	SVNFSTXN_CONST_LONG(FS_TXN_CHECK_LOCKS, SVN_FS_TXN_CHECK_LOCKS);

	return SUCCESS;
}
/* }}} */

/* {{{ proto private void SvnFsTxn::__construct() */
PHP_METHOD(SvnFsTxn, __construct)
{
}
/* }}} */

/* {{{ proto public SvnFsRoot SvnFsTxn::getTxnRoot() */
PHP_METHOD(SvnFsTxn, getTxnRoot)
{
	zval *this = getThis(), *repos_zval;
	svn_fs_txn_object *txn_intern = zend_object_store_get_object(this TSRMLS_CC);
	svn_fs_root_t *fs_root;
	svn_error_t *svn_error;
	svn_fs_root_object *fs_root_intern;

	object_init_ex(return_value, svn_fs_root_object_ce);
	fs_root_intern = zend_object_store_get_object(return_value TSRMLS_CC);

	svn_error = svn_fs_txn_root(&fs_root,
		txn_intern->txn,
		fs_root_intern->pool);

	if (svn_error) {
		zval_dtor(return_value);
		RETVAL_FALSE;
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}

	repos_zval = zend_read_property(svn_fs_txn_object_ce,
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

/* {{{ proto public int SvnFsTxn::commitTxn() */
PHP_METHOD(SvnFsTxn, commitTxn)
{
	zval *this = getThis(), *repos_zval;
	svn_fs_txn_object *intern = zend_object_store_get_object(this TSRMLS_CC);
	svn_repos_object *repos_intern;

	apr_pool_t *tmp_pool;
	svn_revnum_t new_rev;
	svn_error_t *svn_error;
	const char *conflicts;

	apr_pool_create(&tmp_pool, intern->pool);

	repos_zval = zend_read_property(svn_fs_txn_object_ce,
		this,
		ZEND_STRL("repos"),
		0 TSRMLS_CC);

	repos_intern = zend_object_store_get_object(repos_zval TSRMLS_CC);

	svn_error = svn_repos_fs_commit_txn(&conflicts,
		repos_intern->repos,
		&new_rev,
		intern->txn,
		tmp_pool);

	apr_pool_destroy(tmp_pool);

	if (svn_error) {
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}

	RETURN_LONG(SVN_REVNUM_T_TO_LONG(new_rev));
}
/* }}} */

/* {{{ proto public bool SvnFsTxn::abortTxn() */
PHP_METHOD(SvnFsTxn, abortTxn)
{
	zval *this = getThis();
	svn_fs_txn_object *intern = zend_object_store_get_object(this TSRMLS_CC);

	apr_pool_t *tmp_pool;
	svn_error_t *svn_error;

	apr_pool_create(&tmp_pool, intern->pool);

	svn_error = svn_fs_abort_txn(intern->txn, tmp_pool);

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
