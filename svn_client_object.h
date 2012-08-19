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

/* $Id: svn_client_object.h 83 2009-05-16 15:07:28Z utz.m.christopher $ */

#ifndef SVN_CLIENT_OBJECT_H
#define SVN_CLIENT_OBJECT_H 1

#include "svn_client.h"
#include "svn_utf.h"

typedef struct _svn_client_object {
	zend_object zo;

	apr_pool_t *pool;
	svn_client_ctx_t *ctx;
		
} svn_client_object;

PHP_MINIT_FUNCTION(svn_client);

PHP_METHOD(SvnClient, getVersion);
PHP_METHOD(SvnClient, urlFromPath);
PHP_METHOD(SvnClient, getConfig);
PHP_METHOD(SvnClient, setConfig);
PHP_METHOD(SvnClient, getAuthBaton);
PHP_METHOD(SvnClient, setAuthBaton);
PHP_METHOD(SvnClient, getProgressCallback);
PHP_METHOD(SvnClient, setProgressCallback);
PHP_METHOD(SvnClient, getProgressBaton);
PHP_METHOD(SvnClient, setProgressBaton);
PHP_METHOD(SvnClient, getNotifyCallback);
PHP_METHOD(SvnClient, setNotifyCallback);
PHP_METHOD(SvnClient, getNotifyBaton);
PHP_METHOD(SvnClient, setNotifyBaton);
PHP_METHOD(SvnClient, getLogMsgCallback);
PHP_METHOD(SvnClient, setLogMsgCallback);
PHP_METHOD(SvnClient, getLogMsgBaton);
PHP_METHOD(SvnClient, setLogMsgBaton);
PHP_METHOD(SvnClient, getCancelCallback);
PHP_METHOD(SvnClient, setCancelCallback);
PHP_METHOD(SvnClient, getCancelBaton);
PHP_METHOD(SvnClient, setCancelBaton);
PHP_METHOD(SvnClient, checkout);
PHP_METHOD(SvnClient, update);
PHP_METHOD(SvnClient, switch);
PHP_METHOD(SvnClient, add);
PHP_METHOD(SvnClient, mkdir);
PHP_METHOD(SvnClient, delete);
PHP_METHOD(SvnClient, import);
PHP_METHOD(SvnClient, commit);
PHP_METHOD(SvnClient, status);
PHP_METHOD(SvnClient, log);
PHP_METHOD(SvnClient, blame);
PHP_METHOD(SvnClient, diff);
PHP_METHOD(SvnClient, diffPeg);
PHP_METHOD(SvnClient, diffSummarize);
PHP_METHOD(SvnClient, diffSummarizePeg);
PHP_METHOD(SvnClient, merge);
PHP_METHOD(SvnClient, mergePeg);
PHP_METHOD(SvnClient, cleanup);
PHP_METHOD(SvnClient, relocate);
PHP_METHOD(SvnClient, revert);
PHP_METHOD(SvnClient, resolved);
PHP_METHOD(SvnClient, copy);
PHP_METHOD(SvnClient, move);
PHP_METHOD(SvnClient, propSet);
PHP_METHOD(SvnClient, propGet);
PHP_METHOD(SvnClient, propList);
PHP_METHOD(SvnClient, export);
PHP_METHOD(SvnClient, list);
PHP_METHOD(SvnClient, cat);
PHP_METHOD(SvnClient, lock);
PHP_METHOD(SvnClient, unlock);
PHP_METHOD(SvnClient, info);

#endif
