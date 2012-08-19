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

/* $Id: svn_auth_baton_object.c 66 2009-02-16 19:33:39Z baron314159@yahoo.com $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_svnobj_std_defs.h"
#include "svn_exception_object.h"
#include "svn_auth_provider_object.h"
#include "svn_auth_baton_object.h"

#ifdef HAVE_SVNOBJ

#define SVNAUTHBATON_ME(method, visibility) PHP_ME(SvnAuthBaton, method, SVNOBJ_ARGS(SvnAuthBaton, method), visibility)
#define SVNAUTHBATON_BEGIN_ARGS(method, req_args) SVNOBJ_BEGIN_ARG_INFO_EX(SvnAuthBaton, method, 0, req_args)
#define SVNAUTHBATON_END_ARGS() SVNOBJ_END_ARG_INFO()
#define SVNAUTHBATON_CONST_LONG(name, value) zend_declare_class_constant_long(svn_auth_baton_object_ce, ZEND_STRL(#name), value TSRMLS_CC)

#	define AUTH_PARAM_METHODS(param_name, method_name, type) \
	PHP_METHOD(SvnAuthBaton, set ## method_name) { _svn_auth_baton_object_set_ ##type## _param(param_name, INTERNAL_FUNCTION_PARAM_PASSTHRU); } \
	PHP_METHOD(SvnAuthBaton, get ## method_name) { _svn_auth_baton_object_get_ ##type## _param(param_name, INTERNAL_FUNCTION_PARAM_PASSTHRU); }

static zend_class_entry *svn_auth_baton_object_ce;
static zend_object_handlers svn_auth_baton_object_handlers;

SVNAUTHBATON_BEGIN_ARGS(__construct, 0)
	SVNOBJ_ARG_VAL(providers, 0)
SVNAUTHBATON_END_ARGS()

SVNAUTHBATON_BEGIN_ARGS(getDefaultUsername, 0)
SVNAUTHBATON_END_ARGS()

SVNAUTHBATON_BEGIN_ARGS(setDefaultUsername, 0)
	SVNOBJ_ARG_VAL(value, 0)
SVNAUTHBATON_END_ARGS()

SVNAUTHBATON_BEGIN_ARGS(getDefaultPassword, 0)
SVNAUTHBATON_END_ARGS()

SVNAUTHBATON_BEGIN_ARGS(setDefaultPassword, 0)
	SVNOBJ_ARG_VAL(value, 0)
SVNAUTHBATON_END_ARGS()

SVNAUTHBATON_BEGIN_ARGS(getNonInteractive, 0)
SVNAUTHBATON_END_ARGS()

SVNAUTHBATON_BEGIN_ARGS(setNonInteractive, 1)
	SVNOBJ_ARG_VAL(value, 0)
SVNAUTHBATON_END_ARGS()

SVNAUTHBATON_BEGIN_ARGS(getDontStorePasswords, 0)
SVNAUTHBATON_END_ARGS()

SVNAUTHBATON_BEGIN_ARGS(setDontStorePasswords, 1)
	SVNOBJ_ARG_VAL(value, 0)
SVNAUTHBATON_END_ARGS()

SVNAUTHBATON_BEGIN_ARGS(getNoAuthCache, 0)
SVNAUTHBATON_END_ARGS()

SVNAUTHBATON_BEGIN_ARGS(setNoAuthCache, 1)
	SVNOBJ_ARG_VAL(value, 0)
SVNAUTHBATON_END_ARGS()

SVNAUTHBATON_BEGIN_ARGS(getConfigDir, 0)
SVNAUTHBATON_END_ARGS()

SVNAUTHBATON_BEGIN_ARGS(setConfigDir, 1)
	SVNOBJ_ARG_VAL(value, 0)
SVNAUTHBATON_END_ARGS()

static zend_function_entry svn_auth_baton_object_fe[] = {
	SVNAUTHBATON_ME(__construct, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	SVNAUTHBATON_ME(getDefaultUsername, ZEND_ACC_PUBLIC)
	SVNAUTHBATON_ME(setDefaultUsername, ZEND_ACC_PUBLIC)
	SVNAUTHBATON_ME(getDefaultPassword, ZEND_ACC_PUBLIC)
	SVNAUTHBATON_ME(setDefaultPassword, ZEND_ACC_PUBLIC)
	SVNAUTHBATON_ME(getNonInteractive, ZEND_ACC_PUBLIC)
	SVNAUTHBATON_ME(setNonInteractive, ZEND_ACC_PUBLIC)
	SVNAUTHBATON_ME(getDontStorePasswords, ZEND_ACC_PUBLIC)
	SVNAUTHBATON_ME(setDontStorePasswords, ZEND_ACC_PUBLIC)
	SVNAUTHBATON_ME(getNoAuthCache, ZEND_ACC_PUBLIC)
	SVNAUTHBATON_ME(setNoAuthCache, ZEND_ACC_PUBLIC)
	SVNAUTHBATON_ME(getConfigDir, ZEND_ACC_PUBLIC)
	SVNAUTHBATON_ME(setConfigDir, ZEND_ACC_PUBLIC)
	{ NULL, NULL, NULL }
};

/* ***** AUTH PARAMS IN 1.4 API
 * SVN_AUTH_PARAM_DEFAULT_USERNAME y
 * SVN_AUTH_PARAM_DEFAULT_PASSWORD y
 * SVN_AUTH_PARAM_NON_INTERACTIVE y
 * SVN_AUTH_PARAM_DONT_STORE_PASSWORDS y
 * SVN_AUTH_PARAM_NO_AUTH_CACHE y
 * SVN_AUTH_PARAM_SSL_SERVER_FAILURES
 * SVN_AUTH_PARAM_SSL_SERVER_CERT_INFO
 * SVN_AUTH_PARAM_CONFIG
 * SVN_AUTH_PARAM_SERVER_GROUP y
 * SVN_AUTH_PARAM_CONFIG_DIR y
 */

zend_bool svn_auth_baton_is_instance(zval *obj TSRMLS_DC) /* {{{ */
{
	zend_class_entry *ce = zend_get_class_entry(obj TSRMLS_CC);
	return instanceof_function(ce, svn_auth_baton_object_ce TSRMLS_CC);
}
/* }}} */

static void _svn_auth_baton_object_free_storage(void *object TSRMLS_DC) /* {{{ */
{
	svn_auth_baton_object *intern = (svn_auth_baton_object *) object;

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

static zend_object_value _svn_auth_baton_object_new(zend_class_entry *class_type TSRMLS_DC) /* {{{ */
{
	zend_object_value retval;
	svn_auth_baton_object *intern;

	apr_array_header_t *auth_providers;

	intern = emalloc(sizeof(svn_auth_baton_object));
	memset(&intern->zo, 0, sizeof(zend_object));

	intern->pool = NULL;
	intern->auth_baton = NULL;

	apr_pool_create(&intern->pool, NULL);

	/* make an auth baton with no providers to prevent segfaults in cases
	 * where the user subclasses this class and forgets to call the parent
	 * constructor.
	 */
	auth_providers = apr_array_make(intern->pool, 
			0, 
			sizeof(svn_auth_provider_object_t *));

	svn_auth_open(&intern->auth_baton,
		auth_providers,
		intern->pool);

	SVNOBJ_STD_NEW_OBJECT(intern, class_type, retval, svn_auth_baton_object);
	return retval;
}
/* }}} */

PHP_MINIT_FUNCTION(svn_auth_baton) /* {{{ */
{
	SVNOBJ_REGISTER_CLASS_EX(SvnAuthBaton, svn_auth_baton_object, NULL, 0);
	SVNOBJ_DECLARE_PROPERTY_NULL(svn_auth_baton_object, "auth_providers");


	return SUCCESS;
}
/* }}} */

/* {{{ proto public void SvnAuthBaton::__construct([array auth_providers]) 
    Create an SvnAuthBaton object instance. */
PHP_METHOD(SvnAuthBaton, __construct)
{
	zval *this = getThis();
	svn_auth_baton_object *intern = zend_object_store_get_object(this TSRMLS_CC);

	zval *auth_providers_zval = NULL;
	apr_array_header_t *auth_providers;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"|a!", 
		&auth_providers_zval)) {
		RETURN_FALSE;
	}

	auth_providers = apr_array_make(intern->pool, 
		1, 
		sizeof(svn_auth_provider_object_t *));

	if (auth_providers_zval) {
		if (Z_TYPE_P(auth_providers_zval) == IS_ARRAY) {
			HashTable *hash = Z_ARRVAL_P(auth_providers_zval);
			HashPosition pointer;
			zval **data;

			for (zend_hash_internal_pointer_reset_ex(hash, &pointer);
				zend_hash_get_current_data_ex(hash, (void **) &data, &pointer) == SUCCESS;
				zend_hash_move_forward_ex(hash, &pointer)) {

				svn_auth_provider_object *intern_provider;

				if (Z_TYPE_PP(data) != IS_OBJECT || 
					!svn_auth_provider_is_instance(*data TSRMLS_CC)) {

					zend_throw_exception_ex(svn_invalid_argument_exception_object_ce,
						0 TSRMLS_CC,
						"auth_providers must contain only SvnAuthProvider instances");
					RETURN_FALSE;
				}

				intern_provider = zend_object_store_get_object(*data TSRMLS_CC);
				APR_ARRAY_PUSH(auth_providers, 
					svn_auth_provider_object_t *) = intern_provider->auth_provider;
			}
		}
	}

	svn_auth_open(&intern->auth_baton,
		auth_providers,
		intern->pool);

	if (auth_providers_zval) {
		zend_update_property(svn_auth_baton_object_ce,
			this, 
			ZEND_STRL("auth_providers"), 
			auth_providers_zval TSRMLS_CC);
	}
}
/* }}} */

static void _svn_auth_baton_object_set_string_param(const char *name, INTERNAL_FUNCTION_PARAMETERS) /* {{{ */
{
	char *value = NULL;
	int value_len = 0;
	zval *this = getThis();
	svn_auth_baton_object *intern = zend_object_store_get_object(this TSRMLS_CC);

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"|s", 
		&value, &value_len)) {
		RETURN_FALSE;
	}

	if (value) {
		svn_auth_set_parameter(intern->auth_baton, name,
			(void *) apr_pstrndup(intern->pool, value, value_len));
	} else {
		svn_auth_set_parameter(intern->auth_baton, name, NULL);
	}
}
/* }}} */

static void _svn_auth_baton_object_get_string_param(const char *name, INTERNAL_FUNCTION_PARAMETERS) /* {{{ */
{
	zval *this = getThis();
	const void *value;
	svn_auth_baton_object *intern = zend_object_store_get_object(this TSRMLS_CC);

	value = svn_auth_get_parameter(intern->auth_baton, name);

	if (!value) {
		RETURN_NULL();
	}

	RETURN_STRING((char *) value, 1);
}
/* }}} */

static void _svn_auth_baton_object_set_boolean_param(const char *name, INTERNAL_FUNCTION_PARAMETERS) /* {{{ */
{
	zend_bool value;
	zval *this = getThis();
	svn_auth_baton_object *intern = zend_object_store_get_object(this TSRMLS_CC);

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"b", 
		&value)) {
		RETURN_FALSE;
	}

	if (value) {
		svn_auth_set_parameter(intern->auth_baton, name, "y");
	} else {
		svn_auth_set_parameter(intern->auth_baton, name, NULL);
	}

}
/* }}} */

static void _svn_auth_baton_object_get_boolean_param(const char *name, INTERNAL_FUNCTION_PARAMETERS) /* {{{ */
{
	zval *this = getThis();
	svn_auth_baton_object *intern = zend_object_store_get_object(this TSRMLS_CC);

	if (svn_auth_get_parameter(intern->auth_baton, name)) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

AUTH_PARAM_METHODS(SVN_AUTH_PARAM_DEFAULT_USERNAME, DefaultUsername, string)
AUTH_PARAM_METHODS(SVN_AUTH_PARAM_DEFAULT_PASSWORD, DefaultPassword, string)
AUTH_PARAM_METHODS(SVN_AUTH_PARAM_NON_INTERACTIVE, NonInteractive, boolean)
AUTH_PARAM_METHODS(SVN_AUTH_PARAM_DONT_STORE_PASSWORDS, DontStorePasswords, boolean)
AUTH_PARAM_METHODS(SVN_AUTH_PARAM_NO_AUTH_CACHE, NoAuthCache, boolean)
AUTH_PARAM_METHODS(SVN_AUTH_PARAM_CONFIG_DIR, ConfigDir, string)

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
