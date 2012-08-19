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

/* $Id: svn_php_utils.c 77 2009-04-15 03:00:35Z utz.m.christopher $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_svnobj_std_defs.h"
#include "svn_php_utils.h"

#ifdef HAVE_SVNOBJ

const char * svn_normalized_path(const char *unnormalized, apr_pool_t *pool) /* {{{ */
{
	if (svn_path_is_url(unnormalized)) {
		return (const char *) svn_path_canonicalize(unnormalized, pool);
	} else {
		return (const char *) svn_path_internal_style(unnormalized, pool);
	}
}
/* }}} */

char * normalize_path_zval(zval *path_zval, apr_pool_t *pool) /* {{{ */
{
	zval *tmp = NULL;
	char *normalized_path;

	if (Z_TYPE_P(path_zval) == IS_STRING) {
		tmp = path_zval;
	} else {
		ALLOC_ZVAL(tmp);
		*tmp = *path_zval;
		zval_copy_ctor(tmp);
		convert_to_string(tmp);
	}

	normalized_path = apr_pstrdup(pool, 
		svn_normalized_path(Z_STRVAL_P(tmp), pool));

	if (tmp != path_zval) {
		zval_ptr_dtor(&tmp);
	}

	return normalized_path;
}
/* }}} */

#endif
