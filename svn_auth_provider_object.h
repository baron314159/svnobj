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

/* $Id: svn_auth_provider_object.h 61 2009-02-16 15:42:30Z baron314159@yahoo.com $ */

#ifndef SVN_AUTH_PROVIDER_OBJECT_H
#define SVN_AUTH_PROVIDER_OBJECT_H 1

#include "svn_client.h"

typedef struct _svn_auth_provider_object {
	zend_object zo;

	apr_pool_t *pool;
	svn_auth_provider_object_t *auth_provider;
		
} svn_auth_provider_object;

zend_bool svn_auth_provider_is_instance(zval *obj TSRMLS_DC);

PHP_MINIT_FUNCTION(svn_auth_provider);

PHP_METHOD(SvnAuthProvider, createSimpleProvider);
PHP_METHOD(SvnAuthProvider, createSimplePromptProvider);
PHP_METHOD(SvnAuthProvider, createUsernameProvider);
PHP_METHOD(SvnAuthProvider, createUsernamePromptProvider);
PHP_METHOD(SvnAuthProvider, createSslServerTrustFileProvider);
PHP_METHOD(SvnAuthProvider, createSslServerTrustPromptProvider);
PHP_METHOD(SvnAuthProvider, createSslClientCertFileProvider);
PHP_METHOD(SvnAuthProvider, createSslClientCertPromptProvider);
PHP_METHOD(SvnAuthProvider, createSslClientCertPwFileProvider);
PHP_METHOD(SvnAuthProvider, createSslClientCertPwPromptProvider);

#endif
