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

/* $Id: svn_config_object.h 6 2009-01-19 16:48:02Z baron314159@yahoo.com $ */

#ifndef SVN_CONFIG_OBJECT_H
#define SVN_CONFIG_OBJECT_H 1

#include "svn_client.h"
#include "svn_config.h"

typedef struct _svn_config_object {
	zend_object zo;

	apr_hash_t *cfg_hash;
	apr_pool_t *pool;
		
} svn_config_object;

zend_bool svn_config_is_instance(zval *obj TSRMLS_DC);

PHP_MINIT_FUNCTION(svn_config);

PHP_METHOD(SvnConfig, __construct);

#endif
