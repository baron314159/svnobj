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

/* $Id: svn_fs_root_object.h 82 2009-05-11 01:53:09Z utz.m.christopher $ */

#ifndef SVN_FS_ROOT_OBJECT_H
#define SVN_FS_ROOT_OBJECT_H 1

#include "svn_repos.h"

typedef struct _svn_fs_root_object {
	zend_object zo;

	apr_pool_t *pool;
	svn_fs_root_t *root;
		
} svn_fs_root_object;

extern zend_class_entry *svn_fs_root_object_ce;

PHP_MINIT_FUNCTION(svn_fs_root);

PHP_METHOD(SvnFsRoot, __construct);
PHP_METHOD(SvnFsRoot, isFile);
PHP_METHOD(SvnFsRoot, isDir);
PHP_METHOD(SvnFsRoot, checkPath);
PHP_METHOD(SvnFsRoot, getFileLength);
PHP_METHOD(SvnFsRoot, makeDir);
PHP_METHOD(SvnFsRoot, makeFile);

#endif
