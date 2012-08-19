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

/* $Id: svn_auth_provider_object.c 72 2009-03-26 01:00:17Z utz.m.christopher $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_svnobj_std_defs.h"
#include "svn_exception_object.h"
#include "svn_auth_provider_object.h"

#ifdef HAVE_SVNOBJ

#define SVNAUTHPROVIDER_ME(method, visibility) PHP_ME(SvnAuthProvider, method, SVNOBJ_ARGS(SvnAuthProvider, method), visibility)
#define SVNAUTHPROVIDER_BEGIN_ARGS(method, req_args) SVNOBJ_BEGIN_ARG_INFO_EX(SvnAuthProvider, method, 0, req_args)
#define SVNAUTHPROVIDER_END_ARGS() SVNOBJ_END_ARG_INFO()
#define SVNAUTHPROVIDER_CONST_LONG(name, value) zend_declare_class_constant_long(svn_auth_provider_object_ce, ZEND_STRL(#name), value TSRMLS_CC)

static zend_class_entry *svn_auth_provider_object_ce;
static zend_object_handlers svn_auth_provider_object_handlers;

SVNAUTHPROVIDER_BEGIN_ARGS(__construct, 0)
SVNAUTHPROVIDER_END_ARGS()

SVNAUTHPROVIDER_BEGIN_ARGS(createSimpleProvider, 0)
SVNAUTHPROVIDER_END_ARGS()

SVNAUTHPROVIDER_BEGIN_ARGS(createSimplePromptProvider, 0)
	SVNOBJ_ARG_VAL(prompt_callback, 0)
	SVNOBJ_ARG_VAL(retry_limit, 0)
SVNAUTHPROVIDER_END_ARGS()

SVNAUTHPROVIDER_BEGIN_ARGS(createUsernameProvider, 0)
SVNAUTHPROVIDER_END_ARGS()

SVNAUTHPROVIDER_BEGIN_ARGS(createUsernamePromptProvider, 0)
	SVNOBJ_ARG_VAL(prompt_callback, 0)
	SVNOBJ_ARG_VAL(retry_limit, 0)
SVNAUTHPROVIDER_END_ARGS()

SVNAUTHPROVIDER_BEGIN_ARGS(createSslServerTrustFileProvider, 0)
SVNAUTHPROVIDER_END_ARGS()

SVNAUTHPROVIDER_BEGIN_ARGS(createSslServerTrustPromptProvider, 0)
	SVNOBJ_ARG_VAL(prompt_callback, 0)
SVNAUTHPROVIDER_END_ARGS()

SVNAUTHPROVIDER_BEGIN_ARGS(createSslClientCertFileProvider, 0)
SVNAUTHPROVIDER_END_ARGS()

SVNAUTHPROVIDER_BEGIN_ARGS(createSslClientCertPromptProvider, 0)
	SVNOBJ_ARG_VAL(prompt_callback, 0)
	SVNOBJ_ARG_VAL(retry_limit, 0)
SVNAUTHPROVIDER_END_ARGS()

SVNAUTHPROVIDER_BEGIN_ARGS(createSslClientCertPwFileProvider, 0)
SVNAUTHPROVIDER_END_ARGS()

SVNAUTHPROVIDER_BEGIN_ARGS(createSslClientCertPwPromptProvider, 0)
	SVNOBJ_ARG_VAL(prompt_callback, 0)
	SVNOBJ_ARG_VAL(retry_limit, 0)
SVNAUTHPROVIDER_END_ARGS()

static zend_function_entry svn_auth_provider_object_fe[] = {
	SVNAUTHPROVIDER_ME(createSimpleProvider, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	SVNAUTHPROVIDER_ME(createSimplePromptProvider, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	SVNAUTHPROVIDER_ME(createUsernameProvider, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	SVNAUTHPROVIDER_ME(createUsernamePromptProvider, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	SVNAUTHPROVIDER_ME(createSslServerTrustFileProvider, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	SVNAUTHPROVIDER_ME(createSslServerTrustPromptProvider, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	SVNAUTHPROVIDER_ME(createSslClientCertFileProvider, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	SVNAUTHPROVIDER_ME(createSslClientCertPromptProvider, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	SVNAUTHPROVIDER_ME(createSslClientCertPwFileProvider, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	SVNAUTHPROVIDER_ME(createSslClientCertPwPromptProvider, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{ NULL, NULL, NULL }
};

zend_bool svn_auth_provider_is_instance(zval *obj TSRMLS_DC) /* {{{ */
{
	zend_class_entry *ce = zend_get_class_entry(obj TSRMLS_CC);
	return instanceof_function(ce, svn_auth_provider_object_ce TSRMLS_CC);
}
/* }}} */

static void _svn_auth_provider_object_free_storage(void *object TSRMLS_DC) /* {{{ */
{
	svn_auth_provider_object *intern = (svn_auth_provider_object *) object;

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

static zend_object_value _svn_auth_provider_object_new(zend_class_entry *class_type TSRMLS_DC) /* {{{ */
{
	zend_object_value retval;
	svn_auth_provider_object *intern;

	apr_array_header_t *auth_providers;

	intern = emalloc(sizeof(svn_auth_provider_object));
	memset(&intern->zo, 0, sizeof(zend_object));

	intern->pool = NULL;
	intern->auth_provider = NULL;

	apr_pool_create(&intern->pool, NULL);

	SVNOBJ_STD_NEW_OBJECT(intern, class_type, retval, svn_auth_provider_object);
	return retval;
}
/* }}} */

static svn_error_t * _svn_auth_provider_object_simple_prompt_func(svn_auth_cred_simple_t **cred, void *baton, const char *realm, const char *username, svn_boolean_t may_save, apr_pool_t *pool) /* {{{ */
{
	zval *prompt_callback = (zval *) baton;
	zval *realm_zval, *username_zval, *may_save_zval;
	zval *callback_retval = NULL;
	zval **callback_params[3];
	TSRMLS_FETCH();

	if (!prompt_callback) {
		return SVN_NO_ERROR;
	}

	if (!zend_is_callable(prompt_callback, 0, NULL SVNOBJ_IS_CALLABLE_CC)) {
		php_error_docref(NULL TSRMLS_CC, 
			E_WARNING, 
			"Prompt callback is not callable");
		return SVN_NO_ERROR;
	}

	ALLOC_INIT_ZVAL(realm_zval);
	if (realm) {
		ZVAL_STRING(realm_zval, (char *) realm, 1);
	} else {
		ZVAL_NULL(realm_zval);
	}

	ALLOC_INIT_ZVAL(username_zval);
	if (username) {
		ZVAL_STRING(username_zval, (char *) username, 1);
	} else {
		ZVAL_NULL(username_zval);
	}

	ALLOC_INIT_ZVAL(may_save_zval);
	ZVAL_BOOL(may_save_zval, may_save);

	callback_params[0] = &realm_zval;
	callback_params[1] = &username_zval;
	callback_params[2] = &may_save_zval;

	if (SUCCESS == call_user_function_ex(EG(function_table), NULL, 
		prompt_callback, &callback_retval, 3, callback_params, 0, 
		NULL TSRMLS_CC) && callback_retval) {
		switch (Z_TYPE_P(callback_retval)) {
			case IS_NULL: {
				break;
			}
			case IS_ARRAY: {
				HashTable *hash = Z_ARRVAL_P(callback_retval);
				zval **hash_zval;

				(*cred) = apr_pcalloc(pool, sizeof(svn_auth_cred_simple_t));

				if (SUCCESS == zend_hash_find(hash, ZEND_STRS("username"), (void **) &hash_zval)) {
					zval tmp = **hash_zval;
					zval_copy_ctor(&tmp);
					convert_to_string(&tmp);
					(*cred)->username = apr_pstrndup(pool, Z_STRVAL(tmp), Z_STRLEN(tmp));
					zval_dtor(&tmp);
				} else {
					(*cred)->username = apr_pstrdup(pool, "");
				}
				if (SUCCESS == zend_hash_find(hash, ZEND_STRS("password"), (void **) &hash_zval)) {
					zval tmp = **hash_zval;
					zval_copy_ctor(&tmp);
					convert_to_string(&tmp);
					(*cred)->password = apr_pstrndup(pool, Z_STRVAL(tmp), Z_STRLEN(tmp));
					zval_dtor(&tmp);
				} else {
					(*cred)->password = apr_pstrdup(pool, "");
				}
				if (SUCCESS == zend_hash_find(hash, ZEND_STRS("may_save"), (void **) &hash_zval)) {
					zval tmp = **hash_zval;
					zval_copy_ctor(&tmp);
					convert_to_boolean(&tmp);
					(*cred)->may_save = Z_BVAL(tmp);
					zval_dtor(&tmp);
				}
				break;
			}
			default: {
				php_error_docref(NULL TSRMLS_CC,
					E_WARNING,
					"Prompt callback's return value must be an array or null");
				break;
			}
		}
		zval_ptr_dtor(&callback_retval);
	} else {
		if (!EG(exception)) {
			php_error_docref(NULL TSRMLS_CC,
				E_WARNING,
				"Failed to call prompt callback");
		}
	}

	zval_ptr_dtor(&realm_zval);
	zval_ptr_dtor(&username_zval);
	zval_ptr_dtor(&may_save_zval);

	return SVN_NO_ERROR;
}
/* }}} */

static svn_error_t * _svn_auth_provider_object_username_prompt_func(svn_auth_cred_username_t **cred, void *baton, const char *realm, svn_boolean_t may_save, apr_pool_t *pool) /* {{{ */
{
	zval *prompt_callback = (zval *) baton;
	zval *realm_zval, *may_save_zval;
	zval *callback_retval = NULL;
	zval **callback_params[2];
	TSRMLS_FETCH();

	if (!prompt_callback) {
		return SVN_NO_ERROR;
	}

	if (!zend_is_callable(prompt_callback, 0, NULL SVNOBJ_IS_CALLABLE_CC)) {
		php_error_docref(NULL TSRMLS_CC, 
			E_WARNING, 
			"Prompt callback is not callable");
		return SVN_NO_ERROR;
	}

	ALLOC_INIT_ZVAL(realm_zval);
	if (realm) {
		ZVAL_STRING(realm_zval, (char *) realm, 1);
	} else {
		ZVAL_NULL(realm_zval);
	}

	ALLOC_INIT_ZVAL(may_save_zval);
	ZVAL_BOOL(may_save_zval, may_save);

	callback_params[0] = &realm_zval;
	callback_params[1] = &may_save_zval;

	if (SUCCESS == call_user_function_ex(EG(function_table), NULL, 
		prompt_callback, &callback_retval, 2, callback_params, 0, 
		NULL TSRMLS_CC) && callback_retval) {
		switch (Z_TYPE_P(callback_retval)) {
			case IS_NULL: {
				break;
			}
			case IS_ARRAY: {
				HashTable *hash = Z_ARRVAL_P(callback_retval);
				zval **hash_zval;

				(*cred) = apr_pcalloc(pool, sizeof(svn_auth_cred_username_t));

				if (SUCCESS == zend_hash_find(hash, ZEND_STRS("username"), (void **) &hash_zval)) {
					zval tmp = **hash_zval;
					zval_copy_ctor(&tmp);
					convert_to_string(&tmp);
					(*cred)->username = apr_pstrndup(pool, Z_STRVAL(tmp), Z_STRLEN(tmp));
					zval_dtor(&tmp);
				} else {
					(*cred)->username = apr_pstrdup(pool, "");
				}
				if (SUCCESS == zend_hash_find(hash, ZEND_STRS("may_save"), (void **) &hash_zval)) {
					zval tmp = **hash_zval;
					zval_copy_ctor(&tmp);
					convert_to_boolean(&tmp);
					(*cred)->may_save = Z_BVAL(tmp);
					zval_dtor(&tmp);
				}
				break;
			}
			default: {
				php_error_docref(NULL TSRMLS_CC,
					E_WARNING,
					"Prompt callback's return value must be an array or null");
				break;
			}
		}
		zval_ptr_dtor(&callback_retval);
	} else {
		if (!EG(exception)) {
			php_error_docref(NULL TSRMLS_CC,
				E_WARNING,
				"Failed to call prompt callback");
		}
	}

	zval_ptr_dtor(&realm_zval);
	zval_ptr_dtor(&may_save_zval);

	return SVN_NO_ERROR;
}
/* }}} */

static svn_error_t * _svn_auth_provider_object_ssl_server_trust_prompt_func(svn_auth_cred_ssl_server_trust_t **cred, void *baton, const char *realm, apr_uint32_t failures, const svn_auth_ssl_server_cert_info_t *cert_info, svn_boolean_t may_save, apr_pool_t *pool) /* {{{ */
{
	zval *prompt_callback = (zval *) baton;
	zval *realm_zval, *failures_zval, *cert_info_zval, *may_save_zval;
	zval *callback_retval = NULL;
	zval **callback_params[4];
	TSRMLS_FETCH();

	if (!prompt_callback) {
		return SVN_NO_ERROR;
	}

	if (!zend_is_callable(prompt_callback, 0, NULL SVNOBJ_IS_CALLABLE_CC)) {
		php_error_docref(NULL TSRMLS_CC, 
			E_WARNING, 
			"Prompt callback is not callable");
		return SVN_NO_ERROR;
	}

	ALLOC_INIT_ZVAL(realm_zval);
	if (realm) {
		ZVAL_STRING(realm_zval, (char *) realm, 1);
	} else {
		ZVAL_NULL(realm_zval);
	}

	ALLOC_INIT_ZVAL(failures_zval);
	ZVAL_LONG(failures_zval, APR_UINT32_T_TO_LONG(failures));

	ALLOC_INIT_ZVAL(cert_info_zval);
	array_init(cert_info_zval);

	ADD_ASSOC_STRING_OR_NULL(cert_info_zval, "hostname", cert_info->hostname);
	ADD_ASSOC_STRING_OR_NULL(cert_info_zval, "fingerprint", cert_info->fingerprint);
	ADD_ASSOC_STRING_OR_NULL(cert_info_zval, "valid_from", cert_info->valid_from);
	ADD_ASSOC_STRING_OR_NULL(cert_info_zval, "valid_until", cert_info->valid_until);
	ADD_ASSOC_STRING_OR_NULL(cert_info_zval, "issuer_dname", cert_info->issuer_dname);
	ADD_ASSOC_STRING_OR_NULL(cert_info_zval, "ascii_cert", cert_info->ascii_cert);

	ALLOC_INIT_ZVAL(may_save_zval);
	ZVAL_BOOL(may_save_zval, may_save);

	callback_params[0] = &realm_zval;
	callback_params[1] = &failures_zval;
	callback_params[2] = &cert_info_zval;
	callback_params[3] = &may_save_zval;

	if (SUCCESS == call_user_function_ex(EG(function_table), NULL, 
		prompt_callback, &callback_retval, 4, callback_params, 0, 
		NULL TSRMLS_CC) && callback_retval) {
		switch (Z_TYPE_P(callback_retval)) {
			case IS_NULL: {
				break;
			}
			case IS_ARRAY: {
				HashTable *hash = Z_ARRVAL_P(callback_retval);
				zval **hash_zval;

				(*cred) = apr_pcalloc(pool, sizeof(svn_auth_cred_ssl_server_trust_t));

				if (SUCCESS == zend_hash_find(hash, ZEND_STRS("accepted_failures"), (void **) &hash_zval)) {
					zval tmp = **hash_zval;
					zval_copy_ctor(&tmp);
					convert_to_long(&tmp);
					(*cred)->accepted_failures = LONG_TO_APR_UINT32_T(Z_LVAL(tmp));
					zval_dtor(&tmp);
				} 
				if (SUCCESS == zend_hash_find(hash, ZEND_STRS("may_save"), (void **) &hash_zval)) {
					zval tmp = **hash_zval;
					zval_copy_ctor(&tmp);
					convert_to_boolean(&tmp);
					(*cred)->may_save = Z_BVAL(tmp);
					zval_dtor(&tmp);
				}
				break;
			}
			default: {
				php_error_docref(NULL TSRMLS_CC,
					E_WARNING,
					"Prompt callback's return value must be an array or null");
				break;
			}
		}
		zval_ptr_dtor(&callback_retval);
	} else {
		if (!EG(exception)) {
			php_error_docref(NULL TSRMLS_CC,
				E_WARNING,
				"Failed to call prompt callback");
		}
	}

	zval_ptr_dtor(&realm_zval);
	zval_ptr_dtor(&failures_zval);
	zval_ptr_dtor(&cert_info_zval);
	zval_ptr_dtor(&may_save_zval);

	return SVN_NO_ERROR;
}
/* }}} */

static svn_error_t * _svn_auth_provider_object_ssl_client_cert_prompt_func(svn_auth_cred_ssl_client_cert_t **cred, void *baton, const char *realm, svn_boolean_t may_save, apr_pool_t *pool) /* {{{ */
{
	zval *prompt_callback = (zval *) baton;
	zval *realm_zval, *may_save_zval;
	zval *callback_retval = NULL;
	zval **callback_params[2];
	TSRMLS_FETCH();

	if (!prompt_callback) {
		return SVN_NO_ERROR;
	}

	if (!zend_is_callable(prompt_callback, 0, NULL SVNOBJ_IS_CALLABLE_CC)) {
		php_error_docref(NULL TSRMLS_CC, 
			E_WARNING, 
			"Prompt callback is not callable");
		return SVN_NO_ERROR;
	}

	ALLOC_INIT_ZVAL(realm_zval);
	if (realm) {
		ZVAL_STRING(realm_zval, (char *) realm, 1);
	} else {
		ZVAL_NULL(realm_zval);
	}

	ALLOC_INIT_ZVAL(may_save_zval);
	ZVAL_BOOL(may_save_zval, may_save);

	callback_params[0] = &realm_zval;
	callback_params[1] = &may_save_zval;

	if (SUCCESS == call_user_function_ex(EG(function_table), NULL, 
		prompt_callback, &callback_retval, 2, callback_params, 0, 
		NULL TSRMLS_CC) && callback_retval) {
		switch (Z_TYPE_P(callback_retval)) {
			case IS_NULL: {
				break;
			}
			case IS_ARRAY: {
				HashTable *hash = Z_ARRVAL_P(callback_retval);
				zval **hash_zval;

				(*cred) = apr_pcalloc(pool, sizeof(svn_auth_cred_ssl_client_cert_t));

				if (SUCCESS == zend_hash_find(hash, ZEND_STRS("cert_file"), (void **) &hash_zval)) {
					zval tmp = **hash_zval;
					zval_copy_ctor(&tmp);
					convert_to_string(&tmp);
					(*cred)->cert_file = apr_pstrndup(pool, Z_STRVAL(tmp), Z_STRLEN(tmp));
					zval_dtor(&tmp);
				} else {
					(*cred)->cert_file = apr_pstrdup(pool, "");
				}
				if (SUCCESS == zend_hash_find(hash, ZEND_STRS("may_save"), (void **) &hash_zval)) {
					zval tmp = **hash_zval;
					zval_copy_ctor(&tmp);
					convert_to_boolean(&tmp);
					(*cred)->may_save = Z_BVAL(tmp);
					zval_dtor(&tmp);
				}
				break;
			}
			default: {
				php_error_docref(NULL TSRMLS_CC,
					E_WARNING,
					"Prompt callback's return value must be an array or null");
				break;
			}
		}
		zval_ptr_dtor(&callback_retval);
	} else {
		if (!EG(exception)) {
			php_error_docref(NULL TSRMLS_CC,
				E_WARNING,
				"Failed to call prompt callback");
		}
	}

	zval_ptr_dtor(&realm_zval);
	zval_ptr_dtor(&may_save_zval);

	return SVN_NO_ERROR;
}
/* }}} */

static svn_error_t * _svn_auth_provider_object_ssl_client_cert_pw_prompt_func(svn_auth_cred_ssl_client_cert_pw_t **cred, void *baton, const char *realm, svn_boolean_t may_save, apr_pool_t *pool) /* {{{ */
{
	zval *prompt_callback = (zval *) baton;
	zval *realm_zval, *may_save_zval;
	zval *callback_retval = NULL;
	zval **callback_params[2];
	TSRMLS_FETCH();

	if (!prompt_callback) {
		return SVN_NO_ERROR;
	}

	if (!zend_is_callable(prompt_callback, 0, NULL SVNOBJ_IS_CALLABLE_CC)) {
		php_error_docref(NULL TSRMLS_CC, 
			E_WARNING, 
			"Prompt callback is not callable");
		return SVN_NO_ERROR;
	}

	ALLOC_INIT_ZVAL(realm_zval);
	if (realm) {
		ZVAL_STRING(realm_zval, (char *) realm, 1);
	} else {
		ZVAL_NULL(realm_zval);
	}

	ALLOC_INIT_ZVAL(may_save_zval);
	ZVAL_BOOL(may_save_zval, may_save);

	callback_params[0] = &realm_zval;
	callback_params[1] = &may_save_zval;

	if (SUCCESS == call_user_function_ex(EG(function_table), NULL, 
		prompt_callback, &callback_retval, 2, callback_params, 0, 
		NULL TSRMLS_CC) && callback_retval) {
		switch (Z_TYPE_P(callback_retval)) {
			case IS_NULL: {
				break;
			}
			case IS_ARRAY: {
				HashTable *hash = Z_ARRVAL_P(callback_retval);
				zval **hash_zval;

				(*cred) = apr_pcalloc(pool, sizeof(svn_auth_cred_ssl_client_cert_pw_t));

				if (SUCCESS == zend_hash_find(hash, ZEND_STRS("password"), (void **) &hash_zval)) {
					zval tmp = **hash_zval;
					zval_copy_ctor(&tmp);
					convert_to_string(&tmp);
					(*cred)->password = apr_pstrndup(pool, Z_STRVAL(tmp), Z_STRLEN(tmp));
					zval_dtor(&tmp);
				} else {
					(*cred)->password = apr_pstrdup(pool, "");
				}
				if (SUCCESS == zend_hash_find(hash, ZEND_STRS("may_save"), (void **) &hash_zval)) {
					zval tmp = **hash_zval;
					zval_copy_ctor(&tmp);
					convert_to_boolean(&tmp);
					(*cred)->may_save = Z_BVAL(tmp);
					zval_dtor(&tmp);
				}
				break;
			}
			default: {
				php_error_docref(NULL TSRMLS_CC,
					E_WARNING,
					"Prompt callback's return value must be an array or null");
				break;
			}
		}
		zval_ptr_dtor(&callback_retval);
	} else {
		if (!EG(exception)) {
			php_error_docref(NULL TSRMLS_CC,
				E_WARNING,
				"Failed to call prompt callback");
		}
	}

	zval_ptr_dtor(&realm_zval);
	zval_ptr_dtor(&may_save_zval);

	return SVN_NO_ERROR;
}
/* }}} */

#define PROMPT_TYPE_SIMPLE 0
#define PROMPT_TYPE_USERNAME 1
#define PROMPT_TYPE_SSL_SERVER_TRUST 2
#define PROMPT_TYPE_SSL_CLIENT_CERT 3
#define PROMPT_TYPE_SSL_CLIENT_CERT_PW 4

static void _svn_auth_provider_create_prompt_with_callback_provider(int prompt_type, INTERNAL_FUNCTION_PARAMETERS) /* {{{ */
{
	zval *prompt_callback = NULL;
	long retry_limit = 0;
	svn_auth_provider_object *intern;

	switch (prompt_type) {
		case PROMPT_TYPE_SIMPLE:
		case PROMPT_TYPE_USERNAME:
		case PROMPT_TYPE_SSL_CLIENT_CERT:
		case PROMPT_TYPE_SSL_CLIENT_CERT_PW: {
			if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
				"|zl", 
				&prompt_callback,
				&retry_limit)) {
				RETURN_FALSE;
			}
			break;
		}
		case PROMPT_TYPE_SSL_SERVER_TRUST: {
			if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
				"|z", 
				&prompt_callback)) {
				RETURN_FALSE;
			}
			break;
		}
	}

	object_init_ex(return_value, svn_auth_provider_object_ce);
	intern = zend_object_store_get_object(return_value TSRMLS_CC);

	if (prompt_callback) {
		zend_update_property(svn_auth_provider_object_ce,
			return_value,
			ZEND_STRL("prompt_callback"),
			prompt_callback TSRMLS_CC);

		if (!zend_is_callable(prompt_callback, IS_CALLABLE_CHECK_SYNTAX_ONLY, NULL SVNOBJ_IS_CALLABLE_CC)) {
			zend_throw_exception_ex(svn_invalid_argument_exception_object_ce,
				0 TSRMLS_CC,
				"prompt_callback must be callable");
			RETVAL_FALSE;
		}
	}

	switch (prompt_type) {
		case PROMPT_TYPE_SIMPLE: {
			svn_auth_get_simple_prompt_provider(&intern->auth_provider,
				_svn_auth_provider_object_simple_prompt_func,
				prompt_callback,
				retry_limit,
				intern->pool);
			break;
		}
		case PROMPT_TYPE_USERNAME: {
			svn_auth_get_username_prompt_provider(&intern->auth_provider,
				_svn_auth_provider_object_username_prompt_func,
				prompt_callback,
				retry_limit,
				intern->pool);
			break;
		}
		case PROMPT_TYPE_SSL_SERVER_TRUST: {
			svn_auth_get_ssl_server_trust_prompt_provider(&intern->auth_provider,
				_svn_auth_provider_object_ssl_server_trust_prompt_func,
				prompt_callback,
				intern->pool);
			break;
		}
		case PROMPT_TYPE_SSL_CLIENT_CERT: {
			svn_auth_get_ssl_client_cert_prompt_provider(&intern->auth_provider,
				_svn_auth_provider_object_ssl_client_cert_prompt_func,
				prompt_callback,
				retry_limit,
				intern->pool);
			break;
		}
		case PROMPT_TYPE_SSL_CLIENT_CERT_PW: {
			svn_auth_get_ssl_client_cert_pw_prompt_provider(&intern->auth_provider,
				_svn_auth_provider_object_ssl_client_cert_pw_prompt_func,
				prompt_callback,
				retry_limit,
				intern->pool);
			break;
		}
	}
}
/* }}} */

PHP_MINIT_FUNCTION(svn_auth_provider) /* {{{ */
{
	SVNOBJ_REGISTER_CLASS_EX(SvnAuthProvider, svn_auth_provider_object, NULL, 0);
	SVNOBJ_DECLARE_PROPERTY_NULL(svn_auth_provider_object, "prompt_callback");

	/* SSL server certificate failure bits */
	SVNAUTHPROVIDER_CONST_LONG(AUTH_SSL_NOTYETVALID, SVN_AUTH_SSL_NOTYETVALID);
	SVNAUTHPROVIDER_CONST_LONG(AUTH_SSL_EXPIRED, SVN_AUTH_SSL_EXPIRED);
	SVNAUTHPROVIDER_CONST_LONG(AUTH_SSL_CNMISMATCH, SVN_AUTH_SSL_CNMISMATCH);
	SVNAUTHPROVIDER_CONST_LONG(AUTH_SSL_UNKNOWNCA, SVN_AUTH_SSL_UNKNOWNCA);
	SVNAUTHPROVIDER_CONST_LONG(AUTH_SSL_OTHER, SVN_AUTH_SSL_OTHER);

	return SUCCESS;
}
/* }}} */

/* {{{ proto public static SvnAuthProvider SvnAuthProvider::createSimpleProvider() */
PHP_METHOD(SvnAuthProvider, createSimpleProvider)
{
	svn_auth_provider_object *intern;

	object_init_ex(return_value, svn_auth_provider_object_ce);
	intern = zend_object_store_get_object(return_value TSRMLS_CC);
	svn_auth_get_simple_provider(&intern->auth_provider, intern->pool);
}
/* }}} */

/* {{{ proto public static SvnAuthProvider SvnAuthProvider::createSimplePromptProvider([callback prompt_callback, int retry_limit]) */
PHP_METHOD(SvnAuthProvider, createSimplePromptProvider)
{
	_svn_auth_provider_create_prompt_with_callback_provider(PROMPT_TYPE_SIMPLE, INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto public static SvnAuthProvider SvnAuthProvider::createUsernameProvider() */
PHP_METHOD(SvnAuthProvider, createUsernameProvider)
{
	svn_auth_provider_object *intern;

	object_init_ex(return_value, svn_auth_provider_object_ce);
	intern = zend_object_store_get_object(return_value TSRMLS_CC);
	svn_auth_get_username_provider(&intern->auth_provider, intern->pool);
}
/* }}} */

/* {{{ proto public static SvnAuthProvider SvnAuthProvider::createUsernamePromptProvider([callback prompt_callback, int retry_limit]) */
PHP_METHOD(SvnAuthProvider, createUsernamePromptProvider)
{
	_svn_auth_provider_create_prompt_with_callback_provider(PROMPT_TYPE_USERNAME, INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto public static SvnAuthProvider SvnAuthProvider::createSslServerTrustFileProvider() */
PHP_METHOD(SvnAuthProvider, createSslServerTrustFileProvider)
{
	svn_auth_provider_object *intern;

	object_init_ex(return_value, svn_auth_provider_object_ce);
	intern = zend_object_store_get_object(return_value TSRMLS_CC);
	svn_auth_get_ssl_server_trust_file_provider(&intern->auth_provider, intern->pool);
}
/* }}} */

/* {{{ proto public static SvnAuthProvider SvnAuthProvider::createSslServerTrustPromptProvider([callback prompt_callback]) */
PHP_METHOD(SvnAuthProvider, createSslServerTrustPromptProvider)
{
	_svn_auth_provider_create_prompt_with_callback_provider(PROMPT_TYPE_SSL_SERVER_TRUST, INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto public static SvnAuthProvider SvnAuthProvider::createSslClientCertFileProvider() */
PHP_METHOD(SvnAuthProvider, createSslClientCertFileProvider)
{
	svn_auth_provider_object *intern;

	object_init_ex(return_value, svn_auth_provider_object_ce);
	intern = zend_object_store_get_object(return_value TSRMLS_CC);
	svn_auth_get_ssl_client_cert_file_provider(&intern->auth_provider, intern->pool);
}
/* }}} */

/* {{{ proto public static SvnAuthProvider SvnAuthProvider::createSslClientCertPromptProvider([callback prompt_callback, int retry_limit]) */
PHP_METHOD(SvnAuthProvider, createSslClientCertPromptProvider)
{
	_svn_auth_provider_create_prompt_with_callback_provider(PROMPT_TYPE_SSL_CLIENT_CERT, INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto public static SvnAuthProvider SvnAuthProvider::createSslClientCertPwFileProvider() */
PHP_METHOD(SvnAuthProvider, createSslClientCertPwFileProvider)
{
	svn_auth_provider_object *intern;

	object_init_ex(return_value, svn_auth_provider_object_ce);
	intern = zend_object_store_get_object(return_value TSRMLS_CC);
	svn_auth_get_ssl_client_cert_pw_file_provider(&intern->auth_provider, intern->pool);
}
/* }}} */

/* {{{ proto public static SvnAuthProvider SvnAuthProvider::createSslClientCertPwPromptProvider([callback prompt_callback, int retry_limit]) */
PHP_METHOD(SvnAuthProvider, createSslClientCertPwPromptProvider)
{
	_svn_auth_provider_create_prompt_with_callback_provider(PROMPT_TYPE_SSL_CLIENT_CERT_PW, INTERNAL_FUNCTION_PARAM_PASSTHRU);
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
