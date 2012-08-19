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

/* $Id: svn_auth_baton_object.h 6 2009-01-19 16:48:02Z baron314159@yahoo.com $ */

#ifndef SVN_AUTH_BATON_OBJECT_H
#define SVN_AUTH_BATON_OBJECT_H 1

#include "svn_client.h"

typedef struct _svn_auth_baton_object {
	zend_object zo;

	apr_pool_t *pool;
	svn_auth_baton_t *auth_baton;
		
} svn_auth_baton_object;

zend_bool svn_auth_baton_is_instance(zval *obj TSRMLS_DC);

PHP_MINIT_FUNCTION(svn_auth_baton);

PHP_METHOD(SvnAuthBaton, __construct);
PHP_METHOD(SvnAuthBaton, getDefaultUsername);
PHP_METHOD(SvnAuthBaton, setDefaultUsername);
PHP_METHOD(SvnAuthBaton, getDefaultPassword);
PHP_METHOD(SvnAuthBaton, setDefaultPassword);
PHP_METHOD(SvnAuthBaton, getNonInteractive);
PHP_METHOD(SvnAuthBaton, setNonInteractive);
PHP_METHOD(SvnAuthBaton, getDontStorePasswords);
PHP_METHOD(SvnAuthBaton, setDontStorePasswords);
PHP_METHOD(SvnAuthBaton, getNoAuthCache);
PHP_METHOD(SvnAuthBaton, setNoAuthCache);
PHP_METHOD(SvnAuthBaton, getConfigDir);
PHP_METHOD(SvnAuthBaton, setConfigDir);

#endif
