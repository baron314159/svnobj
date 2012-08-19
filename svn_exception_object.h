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

/* $Id: svn_exception_object.h 56 2009-02-15 17:06:01Z baron314159@yahoo.com $ */

#ifndef SVN_EXCEPTION_OBJECT_H
#define SVN_EXCEPTION_OBJECT_H 1

#include "svn_client.h"

#	define THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error) \
	{ \
		zval *exception; \
		exception = svn_exception_from_svn_error_t(svn_error TSRMLS_CC); \
		zend_throw_exception_object(exception TSRMLS_CC); \
		svn_error_clear(svn_error); \
		RETURN_FALSE; \
	}

extern zend_class_entry *svn_exception_object_ce;
extern zend_class_entry *svn_invalid_argument_exception_object_ce;
extern zend_class_entry *svn_library_exception_object_ce;

zval * svn_exception_from_svn_error_t(svn_error_t *svn_error TSRMLS_DC);

PHP_MINIT_FUNCTION(svn_exception);

PHP_METHOD(SvnLibraryException, getChildErrors);

#endif
