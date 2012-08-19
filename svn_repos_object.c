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

/* $Id: svn_repos_object.c 82 2009-05-11 01:53:09Z utz.m.christopher $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_svnobj_std_defs.h"
#include "svn_exception_object.h"
#include "svn_php_utils.h"
#include "svn_fs_object.h"
#include "svn_fs_txn_object.h"
#include "svn_repos_object.h"

#ifdef HAVE_SVNOBJ

#define SVNREPOS_ME(method, visibility) PHP_ME(SvnRepos, method, SVNOBJ_ARGS(SvnRepos, method), visibility)
#define SVNREPOS_BEGIN_ARGS(method, req_args) SVNOBJ_BEGIN_ARG_INFO_EX(SvnRepos, method, 0, req_args)
#define SVNREPOS_END_ARGS() SVNOBJ_END_ARG_INFO()
#define SVNREPOS_CONST_LONG(name, value) zend_declare_class_constant_long(svn_repos_object_ce, ZEND_STRL(#name), value TSRMLS_CC)
#define SVNREPOS_CONST_STRING(name, value) zend_declare_class_constant_stringl(svn_repos_object_ce, ZEND_STRL(#name), ZEND_STRL(value) TSRMLS_CC)

static zend_class_entry *svn_repos_object_ce;
static zend_object_handlers svn_repos_object_handlers;

SVNREPOS_BEGIN_ARGS(__construct, 0)
SVNREPOS_END_ARGS()

SVNREPOS_BEGIN_ARGS(create, 1)
	SVNOBJ_ARG_VAL(path, 0)
	SVNOBJ_ARG_VAL(config, 0)
	SVNOBJ_ARG_VAL(fs_config, 0)
SVNREPOS_END_ARGS()

SVNREPOS_BEGIN_ARGS(open, 1)
	SVNOBJ_ARG_VAL(path, 0)
SVNREPOS_END_ARGS()

SVNREPOS_BEGIN_ARGS(delete, 1)
	SVNOBJ_ARG_VAL(path, 0)
SVNREPOS_END_ARGS()

SVNREPOS_BEGIN_ARGS(hotCopy, 3)
	SVNOBJ_ARG_VAL(src_path, 0)
	SVNOBJ_ARG_VAL(dest_path, 0)
	SVNOBJ_ARG_VAL(clean_logs, 0)
SVNREPOS_END_ARGS()

SVNREPOS_BEGIN_ARGS(getPath, 0)
SVNREPOS_END_ARGS()

SVNREPOS_BEGIN_ARGS(getFs, 0)
SVNREPOS_END_ARGS()

SVNREPOS_BEGIN_ARGS(beginTxnForCommit, 1)
	SVNOBJ_ARG_VAL(rev, 0)
	SVNOBJ_ARG_VAL(author, 0)
	SVNOBJ_ARG_VAL(log_msg, 0)
SVNREPOS_END_ARGS()

static zend_function_entry svn_repos_object_fe[] = {
	SVNREPOS_ME(__construct, ZEND_ACC_PRIVATE | ZEND_ACC_CTOR)
	SVNREPOS_ME(create, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	SVNREPOS_ME(open, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	SVNREPOS_ME(delete, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	SVNREPOS_ME(hotCopy, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	SVNREPOS_ME(getPath, ZEND_ACC_PUBLIC)
	SVNREPOS_ME(getFs, ZEND_ACC_PUBLIC)
	SVNREPOS_ME(beginTxnForCommit, ZEND_ACC_PUBLIC)
	{ NULL, NULL, NULL }
};

static void _zval_to_apr_hash_fs_config(apr_hash_t *fs_config, zval *val) /* {{{ */
{
	HashTable *val_hash = Z_ARRVAL_P(val);
	HashPosition pointer;

	zend_hash_internal_pointer_reset_ex(val_hash, &pointer);

	for (;; zend_hash_move_forward_ex(val_hash, &pointer)) {
		int lookup_result;
		char *key;
		ulong index;
		uint key_len;

		lookup_result = zend_hash_get_current_key_ex(val_hash,
			&key,
			&key_len,
			&index,
			0,
			&pointer);

		if (lookup_result == HASH_KEY_NON_EXISTANT) {
			break;
		}

		// All FS_CONFIG constants are strings. Ignore integer keys.
		if (lookup_result == HASH_KEY_IS_STRING) {
			zval tmp, **data;
			apr_pool_t *pool;
			char *data_str;

			zend_hash_get_current_data_ex(val_hash, (void **) &data, &pointer);

			tmp = **data;
			zval_copy_ctor(&tmp);
			convert_to_string(&tmp);

			pool = apr_hash_pool_get(fs_config);
			data_str = apr_pstrndup(pool, Z_STRVAL(tmp), Z_STRLEN(tmp));

			apr_hash_set(fs_config, key, key_len-1, data_str);

			zval_dtor(&tmp);
		}
	}
}
/* }}} */

static void _svn_repos_object_free_storage(void *object TSRMLS_DC) /* {{{ */
{
	svn_repos_object *intern = (svn_repos_object *) object;

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

static zend_object_value _svn_repos_object_new(zend_class_entry *class_type TSRMLS_DC) /* {{{ */
{
	zend_object_value retval;
	svn_repos_object *intern;

	intern = emalloc(sizeof(svn_repos_object));
	memset(&intern->zo, 0, sizeof(zend_object));

	intern->pool = NULL;

	apr_pool_create(&intern->pool, NULL);

	SVNOBJ_STD_NEW_OBJECT(intern, class_type, retval, svn_repos_object);
	return retval;
}
/* }}} */

PHP_MINIT_FUNCTION(svn_repos) /* {{{ */
{
	SVNOBJ_REGISTER_CLASS_EX(SvnRepos, svn_repos_object, NULL, 0);

	SVNREPOS_CONST_STRING(FS_CONFIG_FS_TYPE, SVN_FS_CONFIG_FS_TYPE);
	SVNREPOS_CONST_STRING(FS_CONFIG_BDB_TXN_NOSYNC, SVN_FS_CONFIG_BDB_TXN_NOSYNC);
	SVNREPOS_CONST_STRING(FS_CONFIG_BDB_LOG_AUTOREMOVE, SVN_FS_CONFIG_BDB_LOG_AUTOREMOVE);
	SVNREPOS_CONST_STRING(FS_CONFIG_PRE_1_4_COMPATIBLE, SVN_FS_CONFIG_PRE_1_4_COMPATIBLE);

	SVNREPOS_CONST_STRING(FS_TYPE_BDB, SVN_FS_TYPE_BDB);
	SVNREPOS_CONST_STRING(FS_TYPE_FSFS, SVN_FS_TYPE_FSFS);

	return SUCCESS;
}
/* }}} */

/* {{{ proto public void SvnRepos::__construct() */
PHP_METHOD(SvnRepos, __construct)
{
}
/* }}} */

/* {{{ proto public static SvnRepos SvnRepos::create(string path [, array config, array fs_config]) */
PHP_METHOD(SvnRepos, create)
{
	svn_repos_object *intern;
	char *path = NULL;
	int path_len = 0;
	zval *config_zval = NULL, *fs_config_zval = NULL;

	apr_pool_t *tmp_pool;
	svn_error_t *svn_error = NULL;
	apr_hash_t *config = NULL, *fs_config = NULL;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"s|aa", 
		&path, &path_len,
		&config_zval,
		&fs_config_zval)) {
		RETURN_FALSE;
	}

	object_init_ex(return_value, svn_repos_object_ce);
	intern = zend_object_store_get_object(return_value TSRMLS_CC);

	apr_pool_create(&tmp_pool, intern->pool);

	path = (char *) svn_normalized_path(path, tmp_pool);

	// config is unused; the parameter is ignored in svn_repos_create.
	
	if (fs_config) {
		fs_config = apr_hash_make(tmp_pool);
		_zval_to_apr_hash_fs_config(fs_config, fs_config_zval);
	}

	svn_error = svn_repos_create(&intern->repos,
		path,
		NULL,
		NULL,
		config,
		fs_config,
		intern->pool);

	apr_pool_destroy(tmp_pool);

	if (svn_error) {
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}
}
/* }}} */

/* {{{ proto public static SvnRepos SvnRepos::open(string path) */
PHP_METHOD(SvnRepos, open)
{
	svn_repos_object *intern;
	char *path = NULL;
	int path_len = 0;

	apr_pool_t *tmp_pool;
	svn_error_t *svn_error = NULL;
	apr_hash_t *config = NULL, *fs_config = NULL;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"s", 
		&path, &path_len)) {
		RETURN_FALSE;
	}

	object_init_ex(return_value, svn_repos_object_ce);
	intern = zend_object_store_get_object(return_value TSRMLS_CC);

	apr_pool_create(&tmp_pool, intern->pool);

	path = (char *) svn_normalized_path(path, tmp_pool);

	svn_error = svn_repos_open(&intern->repos,
		path,
		intern->pool);

	apr_pool_destroy(tmp_pool);

	if (svn_error) {
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}
}
/* }}} */

/* {{{ proto public static bool SvnRepos::delete(string path) */
PHP_METHOD(SvnRepos, delete)
{
	char *path = NULL;
	int path_len = 0;

	apr_pool_t *tmp_pool;
	svn_error_t *svn_error = NULL;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"s", 
		&path, &path_len)) {
		RETURN_FALSE;
	}

	apr_pool_create(&tmp_pool, NULL);

	path = (char *) svn_normalized_path(path, tmp_pool);

	svn_error = svn_repos_delete(path, tmp_pool);

	apr_pool_destroy(tmp_pool);

	if (svn_error) {
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto public static bool SvnRepos::hotCopy(string src_path, string dest_path, bool clean_logs) */
PHP_METHOD(SvnRepos, hotCopy)
{
	char *src_path = NULL, *dest_path = NULL;
	int src_path_len = 0, dest_path_len = 0;
	zend_bool clean_logs = 1;

	apr_pool_t *tmp_pool;
	svn_error_t *svn_error = NULL;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"ssb", 
		&src_path, &src_path_len,
		&dest_path, &dest_path_len,
		&clean_logs)) {
		RETURN_FALSE;
	}

	apr_pool_create(&tmp_pool, NULL);

	src_path = (char *) svn_normalized_path(src_path, tmp_pool);
	dest_path = (char *) svn_normalized_path(dest_path, tmp_pool);

	svn_error = svn_repos_hotcopy(src_path,
		dest_path,
		clean_logs,
		tmp_pool);

	apr_pool_destroy(tmp_pool);

	if (svn_error) {
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto public string SvnRepos::getPath() */
PHP_METHOD(SvnRepos, getPath)
{
	zval *this = getThis();
	svn_repos_object *intern = zend_object_store_get_object(this TSRMLS_CC);
	apr_pool_t *tmp_pool;
	const char *path;

	apr_pool_create(&tmp_pool, intern->pool);

	path = svn_repos_path(intern->repos, tmp_pool);

	if (path) {
		RETVAL_STRING((char *) path, 1);
	} else {
		RETVAL_NULL();
	}

	apr_pool_destroy(tmp_pool);
}
/* }}} */

/* {{{ proto public SvnFs SvnRepos::getFs() */
PHP_METHOD(SvnRepos, getFs)
{
	zval *this = getThis();
	svn_repos_object *repos_intern = zend_object_store_get_object(this TSRMLS_CC);
	svn_fs_t *fs = svn_repos_fs(repos_intern->repos);
	svn_fs_object *fs_intern;

	object_init_ex(return_value, svn_fs_object_ce);
	fs_intern = zend_object_store_get_object(return_value TSRMLS_CC);
	fs_intern->fs = fs;

	zend_update_property(svn_fs_object_ce,
		return_value,
		ZEND_STRL("repos"),
		this TSRMLS_CC);
}
/* }}} */

/* {{{ proto public SvnFsTxn SvnRepos::beginTxnForCommit(int rev [, string author, string log_msg]) */
PHP_METHOD(SvnRepos, beginTxnForCommit)
{
	long rev = 0;
	char *author = NULL, *log_msg = NULL;
	int author_len = 0, log_msg_len = 0;

	zval *this = getThis();
	svn_repos_object *repos_intern = zend_object_store_get_object(this TSRMLS_CC);
	svn_fs_txn_t *fs_txn;
	svn_error_t *svn_error;
	svn_fs_txn_object *fs_txn_intern;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"l|ss", 
		&rev,
		&author, &author_len,
		&log_msg, &log_msg_len)) {
		RETURN_FALSE;
	}

	object_init_ex(return_value, svn_fs_txn_object_ce);
	fs_txn_intern = zend_object_store_get_object(return_value TSRMLS_CC);

	svn_error = svn_repos_fs_begin_txn_for_commit(&fs_txn,
		repos_intern->repos,
		LONG_TO_SVN_REVNUM_T(rev),
		author,
		log_msg,
		fs_txn_intern->pool);

	if (svn_error) {
		zval_dtor(return_value);
		RETVAL_FALSE;
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}

	zend_update_property(svn_fs_txn_object_ce,
		return_value,
		ZEND_STRL("repos"),
		this TSRMLS_CC);

	fs_txn_intern->txn = fs_txn;
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
