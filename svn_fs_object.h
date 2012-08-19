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

/* $Id: svn_fs_object.h 81 2009-05-10 16:35:25Z utz.m.christopher $ */

#ifndef SVN_FS_OBJECT_H
#define SVN_FS_OBJECT_H 1

#include "svn_repos.h"
#include "svn_fs.h"

typedef struct _svn_fs_object {
	zend_object zo;

	apr_pool_t *pool;
	svn_fs_t *fs;
		
} svn_fs_object;

extern zend_class_entry *svn_fs_object_ce;

PHP_MINIT_FUNCTION(svn_fs);

PHP_METHOD(SvnFs, __construct);
PHP_METHOD(SvnFs, beginTxn);
PHP_METHOD(SvnFs, getRevisionRoot);
PHP_METHOD(SvnFs, getYoungestRev);

#endif
