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

/* $Id: svn_fs_txn_object.h 81 2009-05-10 16:35:25Z utz.m.christopher $ */

#ifndef SVN_FS_TXN_OBJECT_H
#define SVN_FS_TXN_OBJECT_H 1

#include "svn_repos.h"

typedef struct _svn_fs_txn_object {
	zend_object zo;

	apr_pool_t *pool;
	svn_fs_txn_t *txn;
		
} svn_fs_txn_object;

extern zend_class_entry *svn_fs_txn_object_ce;

PHP_MINIT_FUNCTION(svn_fs_txn);

PHP_METHOD(SvnFsTxn, __construct);
PHP_METHOD(SvnFsTxn, getTxnRoot);
PHP_METHOD(SvnFsTxn, commitTxn);
PHP_METHOD(SvnFsTxn, abortTxn);

#endif
