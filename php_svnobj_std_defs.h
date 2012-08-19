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

/* $Id: php_svnobj_std_defs.h 81 2009-05-10 16:35:25Z utz.m.christopher $ */

#ifndef PHP_SVNOBJ_STD_DEFS_H
#define PHP_SVNOBJ_STD_DEFS_H 1

#define APR_TIME_T_TO_LONG(t) ((long)apr_time_sec(t))
#define SVN_REVNUM_T_TO_LONG(r) ((long)r)
#define SVN_FILESIZE_T_TO_LONG(f) ((long)f)
#define LONG_TO_TIME_T(l) ((time_t)l)
#define APR_UINT32_T_TO_LONG(u) ((long)u)
#define APR_INT64_T_TO_LONG(i) ((long)i)
#define APR_OFF_T_TO_LONG(o) ((long)o)
#define APR_SIZE_T_TO_LONG(s) ((long)s)
#define LONG_TO_APR_UINT32_T(l) ((apr_uint32_t)l)
#define LONG_TO_SVN_REVNUM_T(l) ((svn_revnum_t)l)

#define PHP_MINIT_CALL(func) PHP_MINIT(func)(INIT_FUNC_ARGS_PASSTHRU)
#define PHP_MSHUTDOWN_CALL(func) PHP_MSHUTDOWN(func)(SHUTDOWN_FUNC_ARGS_PASSTHRU)

#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 2) || PHP_MAJOR_VERSION > 5
#   define  ZEND_EXCEPTION_GET_DEFAULT() zend_exception_get_default(TSRMLS_C)
#else
#   define  ZEND_EXCEPTION_GET_DEFAULT() zend_exception_get_default()
#endif

#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 2) || PHP_MAJOR_VERSION > 5
#   define SVNOBJ_IS_CALLABLE_CC TSRMLS_CC
#else
#   define SVNOBJ_IS_CALLABLE_CC
#endif

#	define SVNOBJ_REGISTER_CLASS_EX(classname, name, parent, flags) \
	{ \
		zend_class_entry ce; \
		memset(&ce, 0, sizeof(zend_class_entry)); \
		INIT_CLASS_ENTRY(ce, #classname, name## _fe); \
		ce.create_object = _ ##name## _new; \
		name## _ce = zend_register_internal_class_ex(&ce, parent, NULL TSRMLS_CC); \
		name## _ce->ce_flags |= flags;  \
		memcpy(& name## _handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers)); \
	}

#	define SVNOBJ_REGISTER_CLASS(classname, name, parent, flags) \
	{ \
		zend_class_entry ce; \
		memset(&ce, 0, sizeof(zend_class_entry)); \
		INIT_CLASS_ENTRY(ce, #classname, name## _fe); \
		ce.create_object = NULL; \
		name## _ce = zend_register_internal_class_ex(&ce, parent, NULL TSRMLS_CC); \
		name## _ce->ce_flags |= flags;  \
	}

#	define SVNOBJ_REGISTER_EXCEPTION(classname, cename, parent) \
	{ \
		zend_class_entry ce; \
		memset(&ce, 0, sizeof(zend_class_entry)); \
		INIT_CLASS_ENTRY(ce, #classname, NULL); \
		ce.create_object = NULL; \
		cename = zend_register_internal_class_ex(&ce, parent, NULL TSRMLS_CC); \
	}

#	define SVNOBJ_STD_NEW_OBJECT(intern, class_type, obj_val, name) \
	{ \
		zval tmp; \
		zend_object_std_init(&intern->zo, class_type TSRMLS_CC); \
		zend_hash_copy(intern->zo.properties, \
				&class_type->default_properties, \
				(copy_ctor_func_t) zval_add_ref, \
				(void *) &tmp, \
				sizeof(zval *)); \
		obj_val.handle = zend_objects_store_put(intern, \
				NULL, \
				(zend_objects_free_object_storage_t) _ ##name## _free_storage, \
				NULL TSRMLS_CC); \
		obj_val.handlers = (zend_object_handlers *) & name## _handlers; \
	}

#	define ADD_ASSOC_STRING_OR_NULL(arr, key, value) \
	{ \
		if (value) { \
			add_assoc_string(arr, key, (char *) value, 1); \
		} else { \
			add_assoc_null(arr, key); \
		} \
	}

#	define ADD_ASSOC_PATH_STRING_OR_NULL(arr, key, value, pool) \
	{ \
		if (value) { \
			add_assoc_string(arr, key, (char *) svn_path_local_style(value, pool), 1); \
		} else { \
			add_assoc_null(arr, key); \
		} \
	}

#define SVNOBJ_ARGS(class, method) args_for_ ##class## _ ##method
#define SVNOBJ_BEGIN_ARG_INFO_EX(class, method, ret_ref, req_args) ZEND_BEGIN_ARG_INFO_EX(args_for_ ##class## _ ##method, 0, ret_ref, req_args)
#define SVNOBJ_END_ARG_INFO() ZEND_END_ARG_INFO()
#define SVNOBJ_ARG_VAL(name, pass_ref) ZEND_ARG_INFO(pass_ref, name)

#define SVNOBJ_DECLARE_PROPERTY_NULL(ce, prop_name) zend_declare_property_null(ce ## _ce, ZEND_STRL(prop_name), ZEND_ACC_PRIVATE TSRMLS_CC);

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
