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

/* $Id: svnobj.c 81 2009-05-10 16:35:25Z utz.m.christopher $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"

#include "php_svnobj.h"
#include "apr_version.h"
#include "php_svnobj_std_defs.h"
#include "svn_auth_provider_object.h"
#include "svn_config_object.h"
#include "svn_auth_baton_object.h"
#include "svn_client_object.h"
#include "svn_repos_object.h"
#include "svn_fs_object.h"
#include "svn_fs_txn_object.h"
#include "svn_fs_root_object.h"
#include "svn_exception_object.h"

#ifdef HAVE_SVNOBJ

zend_module_entry svnobj_module_entry = {
	STANDARD_MODULE_HEADER,
	PHP_SVNOBJ_EXTNAME,
	NULL,
	PHP_MINIT(svnobj),
	PHP_MSHUTDOWN(svnobj),
	NULL,
	NULL,
	PHP_MINFO(svnobj),
	PHP_SVNOBJ_VERSION,
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_SVNOBJ
ZEND_GET_MODULE(svnobj)
#endif

PHP_MINIT_FUNCTION(svnobj) /* {{{ */
{
	apr_status_t apr_status;
	apr_version_t apr_ver;
   
	/* apr_initialize counts how many times it is called, so it only
	 * initializes itself once.
	 */ 
	apr_status = apr_initialize();

	if (apr_status != APR_SUCCESS) {
		return FAILURE;
	}

	apr_version(&apr_ver);

	if (apr_ver.major < APR_MAJOR_VERSION) {
		php_error_docref(NULL TSRMLS_CC,
			E_WARNING,
			"Svnobj was compiled against a newer version of APR than was loaded at runtime");
	}

	if (0 
		|| SUCCESS != PHP_MINIT_CALL(svn_exception)
		|| SUCCESS != PHP_MINIT_CALL(svn_auth_provider)
		|| SUCCESS != PHP_MINIT_CALL(svn_config)
		|| SUCCESS != PHP_MINIT_CALL(svn_auth_baton)
		|| SUCCESS != PHP_MINIT_CALL(svn_client)
		|| SUCCESS != PHP_MINIT_CALL(svn_repos)
		|| SUCCESS != PHP_MINIT_CALL(svn_fs)
		|| SUCCESS != PHP_MINIT_CALL(svn_fs_txn)
		|| SUCCESS != PHP_MINIT_CALL(svn_fs_root)) {
		return FAILURE;
	}

	return SUCCESS;
}
/* }}} */

PHP_MSHUTDOWN_FUNCTION(svnobj) /* {{{ */
{
	/* apr_terminate uses a count incremented by apr_initialize, and only
	 * cleans up when the count is 0.
	 */
	apr_terminate();
}
/* }}} */

PHP_MINFO_FUNCTION(svnobj) /* {{{ */
{
	php_info_print_table_start();
	php_info_print_table_row(2, "SvnObj support", "enabled");
	php_info_print_table_row(2, "APR library version", APR_VERSION_STRING);
	php_info_print_table_row(2, "SVN library version", SVN_VERSION);
	php_info_print_table_end();
}
/* }}} */

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
