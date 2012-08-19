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

/* $Id: svn_config_object.c 63 2009-02-16 17:15:40Z baron314159@yahoo.com $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_svnobj_std_defs.h"
#include "svn_exception_object.h"
#include "svn_config_object.h"

#ifdef HAVE_SVNOBJ

#define SVNCONFIG_ME(method, visibility) PHP_ME(SvnConfig, method, SVNOBJ_ARGS(SvnConfig, method), visibility)
#define SVNCONFIG_BEGIN_ARGS(method, req_args) SVNOBJ_BEGIN_ARG_INFO_EX(SvnConfig, method, 0, req_args)
#define SVNCONFIG_END_ARGS() SVNOBJ_END_ARG_INFO()
#define SVNCONFIG_CONST_LONG(name, value) zend_declare_class_constant_long(svn_config_object_ce, ZEND_STRL(#name), value TSRMLS_CC)

#	define SVNCONFIG_FETCH_THIS_AND_INTERN() \
	zval *this = getThis(); \
	svn_config_object *intern = zend_object_store_get_object(this TSRMLS_CC);

static zend_class_entry *svn_config_object_ce;
static zend_object_handlers svn_config_object_handlers;

SVNCONFIG_BEGIN_ARGS(__construct, 0)
	SVNOBJ_ARG_VAL(config_dir, 0)
SVNCONFIG_END_ARGS()

static zend_function_entry svn_config_object_fe[] = {
	SVNCONFIG_ME(__construct, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	{ NULL, NULL, NULL }
};

zend_bool svn_config_is_instance(zval *obj TSRMLS_DC) /* {{{ */
{
	zend_class_entry *ce = zend_get_class_entry(obj TSRMLS_CC);
	return instanceof_function(ce, svn_config_object_ce TSRMLS_CC);
}
/* }}} */

static void _svn_config_object_free_storage(void *object TSRMLS_DC) /* {{{ */
{
	svn_config_object *intern = (svn_config_object *) object;

	if (!intern) {
		return;
	}

	intern->cfg_hash = NULL;

	if (intern->pool) {
		apr_pool_destroy(intern->pool);
		intern->pool = NULL;
	}

	/* requires PHP >= 5.1.2 */
	zend_object_std_dtor(&intern->zo TSRMLS_CC);

	efree(intern);
}
/* }}} */

static zend_object_value _svn_config_object_new(zend_class_entry *class_type TSRMLS_DC) /* {{{ */
{
	zend_object_value retval;
	svn_config_object *intern;
	zval *tmp;

	svn_error_t *svn_error;

	intern = emalloc(sizeof(svn_config_object));
	memset(&intern->zo, 0, sizeof(zend_object));

	intern->pool = NULL;

	apr_pool_create(&intern->pool, NULL);
	intern->cfg_hash = apr_hash_make(intern->pool);

	zend_object_std_init(&intern->zo, class_type TSRMLS_CC);
	zend_hash_copy(intern->zo.properties, 
			&class_type->default_properties, 
			(copy_ctor_func_t) zval_add_ref,
			(void *) &tmp,
			sizeof(zval *));

	retval.handle = zend_objects_store_put(intern,
			NULL,
			(zend_objects_free_object_storage_t) _svn_config_object_free_storage,
			NULL TSRMLS_CC);
	retval.handlers = (zend_object_handlers *) &svn_config_object_handlers;

	return retval;
}
/* }}} */

PHP_MINIT_FUNCTION(svn_config) /* {{{ */
{
	SVNOBJ_REGISTER_CLASS_EX(SvnConfig, svn_config_object, NULL, 0);

	return SUCCESS;
}
/* }}} */

/* {{{ proto public void SvnConfig::__construct([string config_dir]) */    
PHP_METHOD(SvnConfig, __construct)
{
	SVNCONFIG_FETCH_THIS_AND_INTERN();
	char *config_dir = NULL;
	int config_dir_len = 0;

	svn_error_t *svn_error;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"|s", 
		&config_dir, &config_dir_len)) {
		RETURN_FALSE;
	}

	svn_error = svn_config_ensure(config_dir, intern->pool);

	if (svn_error) {
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}

	svn_error = svn_config_get_config(&intern->cfg_hash, config_dir, intern->pool);

	if (svn_error) {
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}
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
