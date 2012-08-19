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

/* $Id: svn_php_utils.h 77 2009-04-15 03:00:35Z utz.m.christopher $ */

#ifndef SVN_PHP_UTILS_H
#define SVN_PHP_UTILS_H 1

#include "svn_path.h"

const char * svn_normalized_path(const char *unnormalized, apr_pool_t *pool);
char * normalize_path_zval(zval *path_zval, apr_pool_t *pool);

#endif
