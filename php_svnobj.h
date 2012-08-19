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

/* $Id: php_svnobj.h 3 2009-01-17 15:35:59Z baron314159@yahoo.com $ */

#ifndef PHP_SVNOBJ_H
#define PHP_SVNOBJ_H 1

#include "svn_client.h"

#define PHP_SVNOBJ_VERSION "0.1"
#define PHP_SVNOBJ_EXTNAME "svnobj"

extern zend_module_entry svnobj_module_entry;
#define phpext_svnobj_ptr &svnobj_module_entry;

PHP_MINIT_FUNCTION(svnobj);
PHP_MSHUTDOWN_FUNCTION(svnobj);
PHP_MINFO_FUNCTION(svnobj);

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
