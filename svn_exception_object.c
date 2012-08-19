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

/* $Id: svn_exception_object.c 56 2009-02-15 17:06:01Z baron314159@yahoo.com $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_svnobj_std_defs.h"
#include "svn_exception_object.h"

#ifdef HAVE_SVNOBJ

zend_class_entry *svn_exception_object_ce;
zend_class_entry *svn_invalid_argument_exception_object_ce;
zend_class_entry *svn_library_exception_object_ce;

static zend_function_entry svn_exception_object_fe[] = {
	{ NULL, NULL, NULL }
};

static zend_function_entry svn_invalid_argument_exception_object_fe[] = {
	{ NULL, NULL, NULL }
};

SVNOBJ_BEGIN_ARG_INFO_EX(SvnLibraryException, getChildErrors, 0, 0)
SVNOBJ_END_ARG_INFO()

static zend_function_entry svn_library_exception_object_fe[] = {
	PHP_ME(SvnLibraryException, getChildErrors, SVNOBJ_ARGS(SvnLibraryException, getChildErrors), ZEND_ACC_PUBLIC) 
	{ NULL, NULL, NULL }
};

zval * svn_exception_from_svn_error_t(svn_error_t *svn_error TSRMLS_DC) /* {{{ */
{
	zval *exception, *errors_arr;
	svn_error_t *child_svn_error = svn_error->child;
	int i;

	ALLOC_INIT_ZVAL(exception);
	object_init_ex(exception, svn_library_exception_object_ce);

	ALLOC_INIT_ZVAL(errors_arr);
	array_init(errors_arr);

	if (svn_error->message) {
		zend_update_property_string(svn_exception_object_ce,
			exception,
			ZEND_STRL("message"),
			(char *) svn_error->message TSRMLS_CC);
	}

	zend_update_property_long(svn_exception_object_ce,
		exception,
		ZEND_STRL("code"),
		svn_error->apr_err TSRMLS_CC);

	while (child_svn_error) {
		zval *error_arr;

		ALLOC_INIT_ZVAL(error_arr);
		array_init(error_arr);

		ADD_ASSOC_STRING_OR_NULL(error_arr, "message", child_svn_error->message);
		add_assoc_long(error_arr, "code", child_svn_error->apr_err);
		/* does not increment reference count of error_arr */
		add_next_index_zval(errors_arr, error_arr);
		
		child_svn_error = child_svn_error->child;
	}

	/* adding zval to object increments reference count */
	zend_update_property(svn_library_exception_object_ce,
		exception,
		ZEND_STRL("children"),
		errors_arr TSRMLS_CC);

	zval_ptr_dtor(&errors_arr);

	return exception;
}
/* }}} */

PHP_MINIT_FUNCTION(svn_exception) /* {{{ */
{
	SVNOBJ_REGISTER_CLASS(SvnException, 
		svn_exception_object, 
		(zend_class_entry *) ZEND_EXCEPTION_GET_DEFAULT(), 
		0);

	SVNOBJ_REGISTER_CLASS(SvnInvalidArgumentException,
		svn_invalid_argument_exception_object,
		svn_exception_object_ce,
		0);

	SVNOBJ_REGISTER_CLASS(SvnLibraryException,
		svn_library_exception_object,
		svn_exception_object_ce,
		0);

	SVNOBJ_DECLARE_PROPERTY_NULL(svn_library_exception_object, "children");

	return SUCCESS;
}
/* }}} */

/* {{{ proto public array SvnLibraryException::getChildErrors() */
PHP_METHOD(SvnLibraryException, getChildErrors)
{
	zval *this = getThis(), *ret_zval;

	ret_zval = zend_read_property(svn_library_exception_object_ce,
		this,
		ZEND_STRL("children"),
		0 TSRMLS_CC);

	*return_value = *ret_zval;
	zval_copy_ctor(return_value);
	INIT_PZVAL(return_value);

	if (ret_zval != EG(uninitialized_zval_ptr)) {
		zval_add_ref(&ret_zval);
		zval_ptr_dtor(&ret_zval);
	}

	RETVAL_ZVAL(ret_zval, 0, 0);
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
