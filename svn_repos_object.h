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

/* $Id: svn_repos_object.h 82 2009-05-11 01:53:09Z utz.m.christopher $ */

#ifndef SVN_REPOS_OBJECT_H
#define SVN_REPOS_OBJECT_H 1

#include "svn_repos.h"

typedef struct _svn_repos_object {
	zend_object zo;

	apr_pool_t *pool;
	svn_repos_t *repos;
		
} svn_repos_object;

PHP_MINIT_FUNCTION(svn_repos);

PHP_METHOD(SvnRepos, __construct);
PHP_METHOD(SvnRepos, create);
PHP_METHOD(SvnRepos, open);
PHP_METHOD(SvnRepos, delete);
PHP_METHOD(SvnRepos, hotCopy);
PHP_METHOD(SvnRepos, getPath);
PHP_METHOD(SvnRepos, getFs);
PHP_METHOD(SvnRepos, beginTxnForCommit);

#endif
