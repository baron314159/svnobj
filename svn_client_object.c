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

/* $Id: svn_client_object.c 83 2009-05-16 15:07:28Z utz.m.christopher $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_svnobj_std_defs.h"
#include "svn_exception_object.h"
#include "svn_config_object.h"
#include "svn_auth_baton_object.h"
#include "svn_client_object.h"
#include "svn_php_utils.h"

#ifdef HAVE_SVNOBJ

#define SVNCLIENT_ME(method, visibility) PHP_ME(SvnClient, method, SVNOBJ_ARGS(SvnClient, method), visibility)
#define SVNCLIENT_BEGIN_ARGS(method, req_args) SVNOBJ_BEGIN_ARG_INFO_EX(SvnClient, method, 0, req_args)
#define SVNCLIENT_END_ARGS() SVNOBJ_END_ARG_INFO()
#define SVNCLIENT_CONST_LONG(name, value) zend_declare_class_constant_long(svn_client_object_ce, ZEND_STRL(#name), value TSRMLS_CC)

#	define SVNCLIENT_FETCH_THIS_AND_INTERN() \
	zval *this = getThis(); \
	svn_client_object *intern = zend_object_store_get_object(this TSRMLS_CC);

#	define REVISION_KIND_CHECK(is_url, opt_revision, revision_name) \
	{ \
		if (_is_revision_kind_compatible((is_url), (&opt_revision))) { \
			zend_throw_exception_ex(svn_invalid_argument_exception_object_ce, \
				0 TSRMLS_CC, \
				"%s kind value is not compatible with URLs", \
				revision_name); \
			apr_pool_destroy(tmp_pool); \
			RETURN_FALSE; \
		} \
	}

#define OPEN_UNIQUE_FILE(f, unique_name_p, path, tmp_pool) svn_error = svn_io_open_unique_file2(&f, (const char **) &unique_name_p, path, ".tmp", svn_io_file_del_on_pool_cleanup, tmp_pool);

#define SET_PROPERTY_CALLABLE (1<<0)

static zend_class_entry *svn_client_object_ce;
static zend_object_handlers svn_client_object_handlers;

static svn_opt_revision_t svn_opt_revision_head_default = { svn_opt_revision_head, { 0 } };
static svn_opt_revision_t svn_opt_revision_unspecified_default = { svn_opt_revision_unspecified, { 0 } };
static svn_opt_revision_t svn_opt_revision_number_default = { svn_opt_revision_number, { 1 } };
static svn_opt_revision_t svn_opt_revision_working_default = { svn_opt_revision_working, { 0 } };
static svn_opt_revision_t svn_opt_revision_base_default = { svn_opt_revision_base, { 0 } };

/**
 * svn_wc_status_func2_t does not receive an apr_pool as a parameter, so we need
 * to fudge it.
 */
typedef struct _svn_wc_status_func2_t_baton {
	zval *arr;
	apr_pool_t *pool;
} svn_wc_status_func2_t_baton;

SVNCLIENT_BEGIN_ARGS(getVersion, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(urlFromPath, 1)
	SVNOBJ_ARG_VAL(path_or_url, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(getConfig, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(setConfig, 1)
	SVNOBJ_ARG_VAL(config, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(getAuthBaton, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(setAuthBaton, 1)
	SVNOBJ_ARG_VAL(auth_baton, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(getProgressCallback, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(setProgressCallback, 1)
	SVNOBJ_ARG_VAL(callback, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(getProgressBaton, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(setProgressBaton, 1)
	SVNOBJ_ARG_VAL(baton, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(getNotifyCallback, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(setNotifyCallback, 1)
	SVNOBJ_ARG_VAL(callback, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(getNotifyBaton, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(setNotifyBaton, 1)
	SVNOBJ_ARG_VAL(baton, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(getLogMsgCallback, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(setLogMsgCallback, 1)
	SVNOBJ_ARG_VAL(callback, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(getLogMsgBaton, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(setLogMsgBaton, 1)
	SVNOBJ_ARG_VAL(baton, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(getCancelCallback, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(setCancelCallback, 1)
	SVNOBJ_ARG_VAL(callback, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(getCancelBaton, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(setCancelBaton, 1)
	SVNOBJ_ARG_VAL(baton, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(checkout, 2)
	SVNOBJ_ARG_VAL(url, 0)
	SVNOBJ_ARG_VAL(path, 0)
	SVNOBJ_ARG_VAL(peg_revision, 0)
	SVNOBJ_ARG_VAL(revision, 0)
	SVNOBJ_ARG_VAL(recurse, 0)
	SVNOBJ_ARG_VAL(ignore_externals, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(update, 1)
	SVNOBJ_ARG_VAL(path, 0)
	SVNOBJ_ARG_VAL(revision, 0)
	SVNOBJ_ARG_VAL(recurse, 0)
	SVNOBJ_ARG_VAL(ignore_externals, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(switch, 2)
	SVNOBJ_ARG_VAL(path, 0)
	SVNOBJ_ARG_VAL(url, 0)
	SVNOBJ_ARG_VAL(revision, 0)
	SVNOBJ_ARG_VAL(recurse, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(add, 1)
	SVNOBJ_ARG_VAL(path, 0)
	SVNOBJ_ARG_VAL(recursive, 0)
	SVNOBJ_ARG_VAL(force, 0)
	SVNOBJ_ARG_VAL(no_ignore, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(mkdir, 1)
	SVNOBJ_ARG_VAL(paths, 0)
	SVNOBJ_ARG_VAL(log_msg, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(delete, 1)
	SVNOBJ_ARG_VAL(paths, 0)
	SVNOBJ_ARG_VAL(force, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(import, 2)
	SVNOBJ_ARG_VAL(path, 0)
	SVNOBJ_ARG_VAL(url, 0)
	SVNOBJ_ARG_VAL(nonrecursive, 0)
	SVNOBJ_ARG_VAL(no_ignore, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(commit, 1)
	SVNOBJ_ARG_VAL(target, 0)
	SVNOBJ_ARG_VAL(log_msg, 0)
	SVNOBJ_ARG_VAL(recurse, 0)
	SVNOBJ_ARG_VAL(keep_locks, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(status, 1)
	SVNOBJ_ARG_VAL(path, 0)
	SVNOBJ_ARG_VAL(recurse, 0)
	SVNOBJ_ARG_VAL(get_all, 0)
	SVNOBJ_ARG_VAL(update, 0)
	SVNOBJ_ARG_VAL(no_ignore, 0)
	SVNOBJ_ARG_VAL(ignore_externals, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(log, 1)
	SVNOBJ_ARG_VAL(targets, 0)
	SVNOBJ_ARG_VAL(peg_revision, 0)
	SVNOBJ_ARG_VAL(start, 0)
	SVNOBJ_ARG_VAL(end, 0)
	SVNOBJ_ARG_VAL(limit, 0)
	SVNOBJ_ARG_VAL(discover_changed_paths, 0)
	SVNOBJ_ARG_VAL(strict_node_discovery, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(blame, 1)
	SVNOBJ_ARG_VAL(path_or_url, 0)
	SVNOBJ_ARG_VAL(peg_revision, 0)
	SVNOBJ_ARG_VAL(start, 0)
	SVNOBJ_ARG_VAL(end, 0)
	SVNOBJ_ARG_VAL(diff_options, 0)
	SVNOBJ_ARG_VAL(ignore_mime_type, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(diff, 2)
	SVNOBJ_ARG_VAL(tmp_path, 0)
	SVNOBJ_ARG_VAL(path1, 0)
	SVNOBJ_ARG_VAL(revision1, 0)
	SVNOBJ_ARG_VAL(path2, 0)
	SVNOBJ_ARG_VAL(revision2, 0)
	SVNOBJ_ARG_VAL(recurse, 0)
	SVNOBJ_ARG_VAL(ignore_ancestry, 0)
	SVNOBJ_ARG_VAL(no_diff_related, 0)
	SVNOBJ_ARG_VAL(ignore_content_type, 0)
	SVNOBJ_ARG_VAL(header_encoding, 0)
	SVNOBJ_ARG_VAL(diff_options, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(diffPeg, 2)
	SVNOBJ_ARG_VAL(tmp_path, 0)
	SVNOBJ_ARG_VAL(path, 0)
	SVNOBJ_ARG_VAL(peg_revision, 0)
	SVNOBJ_ARG_VAL(start_revision, 0)
	SVNOBJ_ARG_VAL(end_revision, 0)
	SVNOBJ_ARG_VAL(recurse, 0)
	SVNOBJ_ARG_VAL(ignore_ancestry, 0)
	SVNOBJ_ARG_VAL(no_diff_related, 0)
	SVNOBJ_ARG_VAL(ignore_content_type, 0)
	SVNOBJ_ARG_VAL(header_encoding, 0)
	SVNOBJ_ARG_VAL(diff_options, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(diffSummarize, 1)
	SVNOBJ_ARG_VAL(path1, 0)
	SVNOBJ_ARG_VAL(revision1, 0)
	SVNOBJ_ARG_VAL(path2, 0)
	SVNOBJ_ARG_VAL(revision2, 0)
	SVNOBJ_ARG_VAL(recurse, 0)
	SVNOBJ_ARG_VAL(ignore_ancestry, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(diffSummarizePeg, 1)
	SVNOBJ_ARG_VAL(path, 0)
	SVNOBJ_ARG_VAL(peg_revision, 0)
	SVNOBJ_ARG_VAL(start_revision, 0)
	SVNOBJ_ARG_VAL(end_revision, 0)
	SVNOBJ_ARG_VAL(recurse, 0)
	SVNOBJ_ARG_VAL(ignore_ancestry, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(merge, 5)
	SVNOBJ_ARG_VAL(source1, 0)
	SVNOBJ_ARG_VAL(revision1, 0)
	SVNOBJ_ARG_VAL(source2, 0)
	SVNOBJ_ARG_VAL(revision2, 0)
	SVNOBJ_ARG_VAL(target_wcpath, 0)
	SVNOBJ_ARG_VAL(recurse, 0)
	SVNOBJ_ARG_VAL(ignore_ancestry, 0)
	SVNOBJ_ARG_VAL(force, 0)
	SVNOBJ_ARG_VAL(dry_run, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(mergePeg, 4)
	SVNOBJ_ARG_VAL(source, 0)
	SVNOBJ_ARG_VAL(revision1, 0)
	SVNOBJ_ARG_VAL(revision2, 0)
	SVNOBJ_ARG_VAL(peg_revision, 0)
	SVNOBJ_ARG_VAL(target_wcpath, 0)
	SVNOBJ_ARG_VAL(recurse, 0)
	SVNOBJ_ARG_VAL(ignore_ancestry, 0)
	SVNOBJ_ARG_VAL(force, 0)
	SVNOBJ_ARG_VAL(dry_run, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(cleanup, 1)
	SVNOBJ_ARG_VAL(dir, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(relocate, 3)
	SVNOBJ_ARG_VAL(dir, 0)
	SVNOBJ_ARG_VAL(from, 0)
	SVNOBJ_ARG_VAL(to, 0)
	SVNOBJ_ARG_VAL(recurse, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(revert, 1)
	SVNOBJ_ARG_VAL(paths, 0)
	SVNOBJ_ARG_VAL(recurse, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(resolved, 1)
	SVNOBJ_ARG_VAL(path, 0)
	SVNOBJ_ARG_VAL(recursive, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(copy, 2)
	SVNOBJ_ARG_VAL(src_path, 0)
	SVNOBJ_ARG_VAL(dst_path, 0)
	SVNOBJ_ARG_VAL(src_revision, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(move, 2)
	SVNOBJ_ARG_VAL(src_path, 0)
	SVNOBJ_ARG_VAL(dst_path, 0)
	SVNOBJ_ARG_VAL(force, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(propSet, 3)
	SVNOBJ_ARG_VAL(propname, 0)
	SVNOBJ_ARG_VAL(propval, 0)
	SVNOBJ_ARG_VAL(target, 0)
	SVNOBJ_ARG_VAL(recurse, 0)
	SVNOBJ_ARG_VAL(skip_checks, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(propGet, 2)
	SVNOBJ_ARG_VAL(propname, 0)
	SVNOBJ_ARG_VAL(target, 0)
	SVNOBJ_ARG_VAL(peg_revision, 0)
	SVNOBJ_ARG_VAL(revision, 0)
	SVNOBJ_ARG_VAL(recurse, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(propList, 1)
	SVNOBJ_ARG_VAL(target, 0)
	SVNOBJ_ARG_VAL(peg_revision, 0)
	SVNOBJ_ARG_VAL(revision, 0)
	SVNOBJ_ARG_VAL(recurse, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(export, 2)
	SVNOBJ_ARG_VAL(from, 0)
	SVNOBJ_ARG_VAL(to, 0)
	SVNOBJ_ARG_VAL(peg_revision, 0)
	SVNOBJ_ARG_VAL(revision, 0)
	SVNOBJ_ARG_VAL(overwrite, 0)
	SVNOBJ_ARG_VAL(ignore_externals, 0)
	SVNOBJ_ARG_VAL(recurse, 0)
	SVNOBJ_ARG_VAL(native_eol, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(list, 1)
	SVNOBJ_ARG_VAL(path_or_url, 0)
	SVNOBJ_ARG_VAL(peg_revision, 0)
	SVNOBJ_ARG_VAL(revision, 0)
	SVNOBJ_ARG_VAL(recurse, 0)
	SVNOBJ_ARG_VAL(fetch_locks, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(cat, 1)
	SVNOBJ_ARG_VAL(path_or_url, 0)
	SVNOBJ_ARG_VAL(peg_revision, 0)
	SVNOBJ_ARG_VAL(revision, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(lock, 2)
	SVNOBJ_ARG_VAL(targets, 0)
	SVNOBJ_ARG_VAL(comment, 0)
	SVNOBJ_ARG_VAL(steal_lock, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(unlock, 1)
	SVNOBJ_ARG_VAL(targets, 0)
	SVNOBJ_ARG_VAL(break_lock, 0)
SVNCLIENT_END_ARGS()

SVNCLIENT_BEGIN_ARGS(info, 1)
	SVNOBJ_ARG_VAL(path_or_url, 0)
	SVNOBJ_ARG_VAL(peg_revision, 0)
	SVNOBJ_ARG_VAL(revision, 0)
	SVNOBJ_ARG_VAL(recurse, 0)
SVNCLIENT_END_ARGS()

static zend_function_entry svn_client_object_fe[] = {
	SVNCLIENT_ME(getVersion, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	SVNCLIENT_ME(urlFromPath, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	SVNCLIENT_ME(getConfig, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(setConfig, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(getAuthBaton, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(setAuthBaton, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(getProgressCallback, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(setProgressCallback, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(getProgressBaton, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(setProgressBaton, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(getNotifyCallback, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(setNotifyCallback, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(getNotifyBaton, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(setNotifyBaton, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(getLogMsgCallback, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(setLogMsgCallback, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(getLogMsgBaton, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(setLogMsgBaton, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(getCancelCallback, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(setCancelCallback, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(getCancelBaton, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(setCancelBaton, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(checkout, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(update, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(switch, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(add, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(mkdir, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(delete, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(import, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(commit, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(status, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(log, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(blame, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(diff, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(diffPeg, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(diffSummarize, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(diffSummarizePeg, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(merge, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(mergePeg, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(cleanup, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(relocate, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(revert, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(resolved, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(copy, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(move, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(propSet, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(propGet, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(propList, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(export, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(list, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(cat, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(lock, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(unlock, ZEND_ACC_PUBLIC)
	SVNCLIENT_ME(info, ZEND_ACC_PUBLIC)
	{ NULL, NULL, NULL }
};

static int _is_revision_kind_compatible(int is_url, svn_opt_revision_t *revision) /* {{{ */
{
	if (is_url) {
		if (	revision->kind == svn_opt_revision_working ||
				revision->kind == svn_opt_revision_base) {
			return 1;
		}
	}

	return 0;
}
/* }}} */

static void _zval_to_svn_opt_revision_t(svn_opt_revision_t *result_p, zval *val, const svn_opt_revision_t *default_value) /* {{{ */
{
	*result_p = *default_value;

	if (!val) {
		return;
	}

	switch (Z_TYPE_P(val)) {
		case IS_NULL: {
			break;
		}
		case IS_LONG: {
			result_p->kind = Z_LVAL_P(val);
			result_p->value.number = 0;
			break;
		}
		case IS_ARRAY: {
			HashTable *arr_hash = Z_ARRVAL_P(val);
			HashPosition pointer;
			zval **data;
			int i;

			for (zend_hash_internal_pointer_reset_ex(arr_hash, &pointer), i=0;
				zend_hash_get_current_data_ex(arr_hash, (void **) &data, &pointer) == SUCCESS;
				zend_hash_move_forward_ex(arr_hash, &pointer), i++) {
				if (i == 0) {
					zval tmp = **data;
					zval_copy_ctor(&tmp);
					convert_to_long(&tmp);
					result_p->kind = Z_LVAL(tmp);
					zval_dtor(&tmp);
				} else if (i == 1) {
					zval tmp = **data;
					zval_copy_ctor(&tmp);
					convert_to_long(&tmp);
					switch (result_p->kind) {
						case svn_opt_revision_number:
							result_p->value.number = LONG_TO_SVN_REVNUM_T(Z_LVAL(tmp));
							break;
						case svn_opt_revision_date:
							apr_time_ansi_put(&result_p->value.date, 
								LONG_TO_TIME_T(Z_LVAL(tmp)));
							break;
					}
					zval_dtor(&tmp);
				} else {
					break;
				}
			}

			break;
		}
		default: {
			zval tmp = *val;
			zval_copy_ctor(&tmp);
			convert_to_long(&tmp);
			result_p->kind = Z_LVAL(tmp);
			result_p->value.number = 0;
			zval_dtor(&tmp);
			break;
		}
	}
}
/* }}} */

static void _zval_to_svn_diff_file_options_t(svn_diff_file_options_t *result_p, zval *val) /* {{{ */
{
	TSRMLS_FETCH();

	result_p->ignore_eol_style = FALSE;
	result_p->ignore_space = svn_diff_file_ignore_space_none;

	if (!val) {
		return;
	}

	switch (Z_TYPE_P(val)) {
		case IS_ARRAY: {
			HashTable *hash = Z_ARRVAL_P(val);
			zval **hash_zval;

			if (SUCCESS == zend_hash_find(hash, ZEND_STRS("ignore_eol_style"), (void **) &hash_zval)) {
				zval tmp = **hash_zval;
				zval_copy_ctor(&tmp);
				convert_to_bool(&tmp);
				result_p->ignore_eol_style = Z_BVAL(tmp);
				zval_dtor(&tmp);
			}
			if (SUCCESS == zend_hash_find(hash, ZEND_STRS("ignore_space"), (void **) &hash_zval)) {
				zval tmp = **hash_zval;
				zval_copy_ctor(&tmp);
				convert_to_long(&tmp);
				result_p->ignore_eol_style = Z_LVAL(tmp);
				zval_dtor(&tmp);
			}
		}
		case IS_NULL:
			break;
		default: {
			php_error_docref(NULL TSRMLS_CC, 
				E_WARNING, 
				"diff_options must be an array or null");
			break;
		}
	}
}
/* }}} */

static int _zval_to_apr_array_of_strings(apr_array_header_t **result_pp, zval *paths_zval, int treat_as_paths, apr_pool_t *pool) /* {{{ */
{
	if (Z_TYPE_P(paths_zval) == IS_ARRAY) {
		HashTable *hash = Z_ARRVAL_P(paths_zval);
		HashPosition pointer;
		zval **data;

		*result_pp = apr_array_make(pool, zend_hash_num_elements(hash), sizeof(char *));

		for (zend_hash_internal_pointer_reset_ex(hash, &pointer);
			zend_hash_get_current_data_ex(hash, (void **) &data, &pointer) == SUCCESS;
			zend_hash_move_forward_ex(hash, &pointer)) {
			if (treat_as_paths) {
				APR_ARRAY_PUSH(*result_pp, char *) = normalize_path_zval(*data, pool);
			} else {
				APR_ARRAY_PUSH(*result_pp, char *) = apr_pstrndup(pool, Z_STRVAL_PP(data), Z_STRLEN_PP(data));
			}
		}
		return 1;
	} else {
		*result_pp = apr_array_make(pool, 1, sizeof(char *));
		if (treat_as_paths) {
			APR_ARRAY_PUSH(*result_pp, char *) = normalize_path_zval(paths_zval, pool);
		} else {
			APR_ARRAY_PUSH(*result_pp, char *) = apr_pstrndup(pool, Z_STRVAL_P(paths_zval), Z_STRLEN_P(paths_zval));
		}
		return 0;
	}
}
/* }}} */

static void _apr_time_t_to_zval(zval **result_pp, apr_time_t time) /* {{{ */
{
	char tmp_str[100];

	snprintf(tmp_str, 
		100, 
		"%d %d", 
		(apr_int32_t) apr_time_sec(time),
		(apr_int32_t) apr_time_usec(time));

	ALLOC_INIT_ZVAL(*result_pp);
	ZVAL_STRING(*result_pp, tmp_str, 1);
}
/* }}} */

static void _svn_lock_t_to_zval(zval **result_pp, svn_lock_t *lock) /* {{{ */
{
	zval *result_p, *tmp_zval;

	ALLOC_INIT_ZVAL(*result_pp);
	array_init(*result_pp);

	result_p = *result_pp;

	ADD_ASSOC_STRING_OR_NULL(result_p, "path", lock->path);
	ADD_ASSOC_STRING_OR_NULL(result_p, "token", lock->token);
	ADD_ASSOC_STRING_OR_NULL(result_p, "owner", lock->owner);
	ADD_ASSOC_STRING_OR_NULL(result_p, "comment", lock->comment);
	add_assoc_bool(result_p, "is_dav_comment", lock->is_dav_comment);

	_apr_time_t_to_zval(&tmp_zval, lock->creation_date);
	add_assoc_zval(result_p, "creation_date", tmp_zval);

	_apr_time_t_to_zval(&tmp_zval, lock->expiration_date);
	add_assoc_zval(result_p, "expiration_date", tmp_zval);
}
/* }}} */

static void _svn_commit_info_t_to_zval(zval **result_pp, svn_commit_info_t *commit_info) /* {{{ */
{
	zval *result_p;

	if (!*result_pp) {
		ALLOC_INIT_ZVAL(*result_pp);
	}

	if (!commit_info) {
		ZVAL_NULL(*result_pp);
		return;
	}

	array_init(*result_pp);
	result_p = *result_pp;

	add_assoc_long(result_p, "revision", 
		SVN_REVNUM_T_TO_LONG(commit_info->revision));

	/* The string elements of svn_commit_info_t can be null if no changes
	 * were committed (in this case revision will equal SVN_INVALID_REVNUM).
	 */
	ADD_ASSOC_STRING_OR_NULL(result_p, "date", commit_info->date);
	ADD_ASSOC_STRING_OR_NULL(result_p, "author", commit_info->author);
	ADD_ASSOC_STRING_OR_NULL(result_p, "post_commit_err", 
		commit_info->post_commit_err);
}
/* }}} */

static void _svn_wc_entry_t_to_zval(zval **result_pp, svn_wc_entry_t *entry, apr_pool_t *pool) /* {{{ */
{
	zval *result_p, *tmp_zval;

	ALLOC_INIT_ZVAL(*result_pp);
	array_init(*result_pp);
	result_p = *result_pp;

	ADD_ASSOC_PATH_STRING_OR_NULL(result_p, "name", entry->name, pool);
	add_assoc_long(result_p, "revision", SVN_REVNUM_T_TO_LONG(entry->revision));
	ADD_ASSOC_STRING_OR_NULL(result_p, "url", entry->url);
	ADD_ASSOC_STRING_OR_NULL(result_p, "repos", entry->repos);
	ADD_ASSOC_STRING_OR_NULL(result_p, "uuid", entry->uuid);
	add_assoc_long(result_p, "kind", entry->kind);
	add_assoc_long(result_p, "schedule", entry->schedule);
	add_assoc_bool(result_p, "copied", entry->copied);
	add_assoc_bool(result_p, "deleted", entry->deleted);
	add_assoc_bool(result_p, "absent", entry->absent);
	add_assoc_bool(result_p, "incomplete", entry->incomplete);
	ADD_ASSOC_STRING_OR_NULL(result_p, "copyfrom_url", entry->copyfrom_url);
	add_assoc_long(result_p, "copyfrom_rev", SVN_REVNUM_T_TO_LONG(entry->copyfrom_rev));
	ADD_ASSOC_PATH_STRING_OR_NULL(result_p, "conflict_old", entry->conflict_old, pool);
	ADD_ASSOC_PATH_STRING_OR_NULL(result_p, "conflict_new", entry->conflict_new, pool);
	ADD_ASSOC_PATH_STRING_OR_NULL(result_p, "conflict_wrk", entry->conflict_wrk, pool);
	ADD_ASSOC_PATH_STRING_OR_NULL(result_p, "prejfile", entry->prejfile, pool);

	_apr_time_t_to_zval(&tmp_zval, entry->text_time);
	add_assoc_zval(result_p, "text_time", tmp_zval);

	_apr_time_t_to_zval(&tmp_zval, entry->prop_time);
	add_assoc_zval(result_p, "prop_time", tmp_zval);

	ADD_ASSOC_STRING_OR_NULL(result_p, "checksum", entry->checksum);
	add_assoc_long(result_p, "cmt_rev", SVN_REVNUM_T_TO_LONG(entry->cmt_rev));

	_apr_time_t_to_zval(&tmp_zval, entry->cmt_date);
	add_assoc_zval(result_p, "cmt_date", tmp_zval);

	ADD_ASSOC_STRING_OR_NULL(result_p, "cmt_author", entry->cmt_author);
	ADD_ASSOC_STRING_OR_NULL(result_p, "lock_token", entry->lock_token);
	ADD_ASSOC_STRING_OR_NULL(result_p, "lock_owner", entry->lock_owner);
	ADD_ASSOC_STRING_OR_NULL(result_p, "lock_comment", entry->lock_comment);

	_apr_time_t_to_zval(&tmp_zval, entry->lock_creation_date);
	add_assoc_zval(result_p, "lock_creation_date", tmp_zval);

	add_assoc_bool(result_p, "has_props", entry->has_props);
	add_assoc_bool(result_p, "has_prop_mods", entry->has_prop_mods);
	ADD_ASSOC_STRING_OR_NULL(result_p, "cachable_props", entry->cachable_props);
	ADD_ASSOC_STRING_OR_NULL(result_p, "present_props", entry->present_props);
}
/* }}} */

static void _svn_wc_status2_t_to_zval(zval **result_pp, svn_wc_status2_t *status, apr_pool_t *pool) /* {{{ */
{
	zval *result_p, *tmp_zval;

	ALLOC_INIT_ZVAL(*result_pp);
	array_init(*result_pp);
	result_p = *result_pp;

	if (status->entry) {
		_svn_wc_entry_t_to_zval(&tmp_zval, status->entry, pool);
		add_assoc_zval(result_p, "entry", tmp_zval);
	} else {
		add_assoc_null(result_p, "entry");
	}

	add_assoc_bool(result_p, "locked", status->locked);
	add_assoc_bool(result_p, "copied", status->copied);
	add_assoc_bool(result_p, "switched", status->switched);

	if (status->repos_lock) {
		_svn_lock_t_to_zval(&tmp_zval, status->repos_lock);
		add_assoc_zval(result_p, "repos_lock", tmp_zval);
	} else {
		add_assoc_null(result_p, "repos_lock");
	}

	ADD_ASSOC_STRING_OR_NULL(result_p, "url", status->url);
	add_assoc_long(result_p, "ood_last_cmt_rev", SVN_REVNUM_T_TO_LONG(status->ood_last_cmt_rev));

	_apr_time_t_to_zval(&tmp_zval, status->ood_last_cmt_date);
	add_assoc_zval(result_p, "ood_last_cmt_date", tmp_zval);

	add_assoc_long(result_p, "ood_kind", status->ood_kind);
	ADD_ASSOC_STRING_OR_NULL(result_p, "ood_last_cmt_author", status->ood_last_cmt_author);
}
/* }}} */

static void _svn_prop_t_to_zval(zval **result_pp, svn_prop_t *prop) /* {{{ */
{
	zval *result_p;

	ALLOC_INIT_ZVAL(*result_pp);
	array_init(*result_pp);
	result_p = *result_pp;

	ADD_ASSOC_STRING_OR_NULL(result_p, "name", prop->name);

	if (prop->value) {
		add_assoc_stringl(result_p, "value", (char *) prop->value->data, 
			APR_SIZE_T_TO_LONG(prop->value->len), 1);
	} else {
		add_assoc_null(result_p, "value");
	}
}
/* }}} */

static void _svn_client_commit_item2_t_to_zval(zval **result_pp, svn_client_commit_item2_t *commit_item, apr_pool_t *pool) /* {{{ */
{
	zval *result_p;

	ALLOC_INIT_ZVAL(*result_pp);
	array_init(*result_pp);
	result_p = *result_pp;

	ADD_ASSOC_STRING_OR_NULL(result_p, "path", commit_item->path);
	add_assoc_long(result_p, "kind", commit_item->kind);
	ADD_ASSOC_STRING_OR_NULL(result_p, "url", commit_item->url);
	add_assoc_long(result_p, "revision", SVN_REVNUM_T_TO_LONG(commit_item->revision));
	ADD_ASSOC_STRING_OR_NULL(result_p, "copyfrom_url", commit_item->copyfrom_url);
	add_assoc_long(result_p, "copyfrom_rev", SVN_REVNUM_T_TO_LONG(commit_item->copyfrom_rev));
	add_assoc_long(result_p, "state_flags", commit_item->state_flags);

	if (commit_item->wcprop_changes) {
		zval *wcprop_changes_zval;
		int i;

		ALLOC_INIT_ZVAL(wcprop_changes_zval);
		array_init(wcprop_changes_zval);
		
		for (i=0; i < commit_item->wcprop_changes->nelts; i++) {
			svn_prop_t *prop = APR_ARRAY_IDX(commit_item->wcprop_changes, i, svn_prop_t *);
			zval *prop_zval;

			_svn_prop_t_to_zval(&prop_zval, prop);
			add_next_index_zval(wcprop_changes_zval, prop_zval);
		}

		add_assoc_zval(result_p, "wcprop_changes", wcprop_changes_zval);
	} else {
		add_assoc_null(result_p, "wcprop_changes");
	}
}
/* }}} */

static void _apr_prop_hash_to_zval(zval **result_pp, apr_hash_t *props, apr_pool_t *pool) /* {{{ */
{
	apr_hash_index_t *hash_index = apr_hash_first(pool, props);

	array_init(*result_pp);

	while (hash_index) {
		char *path;
		apr_ssize_t path_len;
		svn_string_t *prop_val;

		apr_hash_this(hash_index, 
			(const void **) &path, 
			&path_len, 
			(void **) &prop_val);

		add_assoc_stringl(*result_pp, 
			path, 
			(char *) prop_val->data,
			prop_val->len,
			1);

		hash_index = apr_hash_next(hash_index);
	}
}
/* }}} */

static svn_error_t * _svn_client_object_list_func(void *baton, const char *path, const svn_dirent_t *dirent, const svn_lock_t *lock, const char *abs_path, apr_pool_t *pool) /* {{{ */
{
	zval *arr = (zval *) baton, *dirent_arr;

	ALLOC_INIT_ZVAL(dirent_arr);
	array_init(dirent_arr);

	ADD_ASSOC_STRING_OR_NULL(dirent_arr, "path", path);
	add_assoc_long(dirent_arr, "kind", dirent->kind);
	add_assoc_long(dirent_arr, "size", SVN_FILESIZE_T_TO_LONG(dirent->size));
	add_assoc_bool(dirent_arr, "has_props", dirent->has_props);
	add_assoc_long(dirent_arr, "created_rev", SVN_REVNUM_T_TO_LONG(dirent->created_rev));
	add_assoc_long(dirent_arr, "time", APR_TIME_T_TO_LONG(dirent->time));
	ADD_ASSOC_STRING_OR_NULL(dirent_arr, "last_author", dirent->last_author);

	add_next_index_zval(arr, dirent_arr);

	return SVN_NO_ERROR;
}
/* }}} */

static void _svn_client_object_status_func(void *baton_in, const char *path, svn_wc_status2_t *status) /* {{{ */ 
{
	svn_wc_status_func2_t_baton *baton = (svn_wc_status_func2_t_baton *) baton_in;
	zval *elem_arr, *status_zval;

	ALLOC_INIT_ZVAL(elem_arr);
	array_init(elem_arr);

	add_assoc_string(elem_arr, "path", (char *) path, 1);

	_svn_wc_status2_t_to_zval(&status_zval, status, baton->pool);
	add_assoc_zval(elem_arr, "status", status_zval);

	add_next_index_zval(baton->arr, elem_arr);
}
/* }}} */

static void _svn_client_object_progress_func(apr_off_t progress, apr_off_t total, void *baton, apr_pool_t *pool) /* {{{ */
{
	zval *this = (zval *) baton, *progress_callback_zval, *progress_baton_zval;
	zval *null_baton_zval = NULL, *progress_zval = NULL, *total_zval = NULL;
	TSRMLS_FETCH();
	svn_client_object *intern = zend_object_store_get_object(this TSRMLS_CC);
	zval *callback_retval = NULL, **callback_params[3];

	progress_callback_zval = zend_read_property(svn_client_object_ce,
		this,	
		ZEND_STRL("progress_callback"),
		0 TSRMLS_CC);

	progress_baton_zval = zend_read_property(svn_client_object_ce,
		this,
		ZEND_STRL("progress_baton"),
		0 TSRMLS_CC);

	if (!progress_callback_zval || Z_TYPE_P(progress_callback_zval) == IS_NULL) {
		return;
	}

	if (!zend_is_callable(progress_callback_zval, 0, NULL SVNOBJ_IS_CALLABLE_CC)) {
		php_error_docref(NULL TSRMLS_CC, 
			E_WARNING, 
			"Progress callback is not callable");
		return;
	}

	ALLOC_INIT_ZVAL(progress_zval);
	ZVAL_LONG(progress_zval, APR_OFF_T_TO_LONG(progress));
	callback_params[0] = &progress_zval;

	ALLOC_INIT_ZVAL(total_zval);
	ZVAL_LONG(total_zval, APR_OFF_T_TO_LONG(total));
	callback_params[1] = &total_zval;

	if (!progress_baton_zval) {
		ALLOC_INIT_ZVAL(null_baton_zval);
		ZVAL_NULL(null_baton_zval);
		callback_params[2] = &null_baton_zval;
	} else {
		callback_params[2] = &progress_baton_zval;
	}

	if (SUCCESS == call_user_function_ex(EG(function_table), NULL, 
		progress_callback_zval, &callback_retval, 3, callback_params, 0, 
		NULL TSRMLS_CC) && callback_retval) {
	
		zval_ptr_dtor(&callback_retval);
	} else {
		if (!EG(exception)) {
			php_error_docref(NULL TSRMLS_CC,
				E_WARNING,
				"Failed to call progress callback");
		}
	}

	zval_ptr_dtor(&progress_zval);
	zval_ptr_dtor(&total_zval);

	if (null_baton_zval) {
		zval_ptr_dtor(&null_baton_zval);
	}
}
/* }}} */

static void _svn_client_object_notify_func(void *baton, const svn_wc_notify_t *notify, apr_pool_t *pool) /* {{{ */
{
	zval *this = (zval *) baton, *notify_callback_zval, *notify_baton_zval;
	zval *null_baton_zval = NULL;
	TSRMLS_FETCH();
	svn_client_object *intern = zend_object_store_get_object(this TSRMLS_CC);
	zval *callback_retval = NULL, **callback_params[2];
	zval *notify_arr;

	notify_callback_zval = zend_read_property(svn_client_object_ce,
		this,	
		ZEND_STRL("notify_callback"),
		0 TSRMLS_CC);

	notify_baton_zval = zend_read_property(svn_client_object_ce,
		this,
		ZEND_STRL("notify_baton"),
		0 TSRMLS_CC);

	if (!notify_callback_zval || Z_TYPE_P(notify_callback_zval) == IS_NULL) {
		return;
	}

	if (!zend_is_callable(notify_callback_zval, 0, NULL SVNOBJ_IS_CALLABLE_CC)) {
		php_error_docref(NULL TSRMLS_CC, 
			E_WARNING, 
			"Notify callback is not callable");
		return;
	}

	ALLOC_INIT_ZVAL(notify_arr);
	array_init(notify_arr);

	ADD_ASSOC_STRING_OR_NULL(notify_arr, "path", notify->path);
	add_assoc_long(notify_arr, "action", notify->action);
	add_assoc_long(notify_arr, "kind", notify->kind);
	ADD_ASSOC_STRING_OR_NULL(notify_arr, "mime_type", notify->mime_type);

	if (notify->lock) {
		zval *lock_zval;
	   	_svn_lock_t_to_zval(&lock_zval, (svn_lock_t *) notify->lock);
		add_assoc_zval(notify_arr, "lock", lock_zval);
	} else {
		add_assoc_null(notify_arr, "lock");
	}

	if (notify->err) {
		TSRMLS_FETCH();
		zval *e = svn_exception_from_svn_error_t(notify->err TSRMLS_CC);
		add_assoc_zval(notify_arr, "err", e);
	} else {
		add_assoc_null(notify_arr, "err");
	}

	add_assoc_long(notify_arr, "content_state", notify->content_state);
	add_assoc_long(notify_arr, "prop_state", notify->prop_state);
	add_assoc_long(notify_arr, "lock_state", notify->lock_state);
	add_assoc_long(notify_arr, "revision", SVN_REVNUM_T_TO_LONG(notify->revision));

	if (!notify_baton_zval) {
		ALLOC_INIT_ZVAL(null_baton_zval);
		ZVAL_NULL(null_baton_zval);
		callback_params[0] = &null_baton_zval;
	} else {
		callback_params[0] = &notify_baton_zval;
	}

	callback_params[1] = &notify_arr;

	if (SUCCESS == call_user_function_ex(EG(function_table), NULL, 
		notify_callback_zval, &callback_retval, 2, callback_params, 0, 
		NULL TSRMLS_CC) && callback_retval) {
	
		zval_ptr_dtor(&callback_retval);
	} else {
		if (!EG(exception)) {
			php_error_docref(NULL TSRMLS_CC,
				E_WARNING,
				"Failed to call notify callback");
		}
	}

	if (null_baton_zval) {
		zval_ptr_dtor(&null_baton_zval);
	}

	zval_ptr_dtor(&notify_arr);
}
/* }}} */

static svn_error_t * _svn_client_object_log_message_receiver_func(void *baton, apr_hash_t *changed_paths, svn_revnum_t revision, const char *author, const char *date, const char *message, apr_pool_t *pool) /* {{{ */
{
	zval *arr = (zval *) baton, *elem_arr;

	ALLOC_INIT_ZVAL(elem_arr);
	array_init(elem_arr);

	if (changed_paths) {
		zval *changed_paths_zval;
		apr_hash_index_t *hash_index = apr_hash_first(pool, changed_paths);
		char action_tmp[2];

		ALLOC_INIT_ZVAL(changed_paths_zval);
		array_init(changed_paths_zval);

		while (hash_index) {
			zval *changed_path_zval;
			const char *key;
			apr_ssize_t key_len;
			svn_log_changed_path_t *changed_path;

			apr_hash_this(hash_index, (const void **) &key, &key_len, 
				(void **) &changed_path);

			ALLOC_INIT_ZVAL(changed_path_zval);
			array_init(changed_path_zval);

			action_tmp[0] = changed_path->action;
			action_tmp[1] = 0;

			ADD_ASSOC_STRING_OR_NULL(changed_path_zval, "action", action_tmp);
			ADD_ASSOC_STRING_OR_NULL(changed_path_zval, "copyfrom_path", 
				changed_path->copyfrom_path);
			add_assoc_long(changed_path_zval, "copyfrom_rev", 
				SVN_REVNUM_T_TO_LONG(changed_path->copyfrom_rev));

			add_assoc_zval(changed_paths_zval, (char *) key, changed_path_zval);

			hash_index = apr_hash_next(hash_index);
		}

		add_assoc_zval(elem_arr, "changed_paths", changed_paths_zval);
	} else {
		add_assoc_null(elem_arr, "changed_paths");
	}

	add_assoc_long(elem_arr, "revision", SVN_REVNUM_T_TO_LONG(revision));
	ADD_ASSOC_STRING_OR_NULL(elem_arr, "author", author);
	ADD_ASSOC_STRING_OR_NULL(elem_arr, "date", date);
	ADD_ASSOC_STRING_OR_NULL(elem_arr, "message", message);

	add_next_index_zval(arr, elem_arr);

	return SVN_NO_ERROR;
}
/* }}} */

static svn_error_t * _svn_client_object_blame_receiver_func(void *baton, apr_int64_t line_no, svn_revnum_t revision, const char *author, const char *date, const char *line, apr_pool_t *pool) /* {{{ */
{
	zval *arr = (zval *) baton, *elem_arr;

	ALLOC_INIT_ZVAL(elem_arr);
	array_init(elem_arr);

	add_assoc_long(elem_arr, "line_no", APR_INT64_T_TO_LONG(line_no));
	add_assoc_long(elem_arr, "revision", SVN_REVNUM_T_TO_LONG(revision));
	ADD_ASSOC_STRING_OR_NULL(elem_arr, "author", author);
	ADD_ASSOC_STRING_OR_NULL(elem_arr, "date", date);
	ADD_ASSOC_STRING_OR_NULL(elem_arr, "line", line);

	add_next_index_zval(arr, elem_arr);

	return SVN_NO_ERROR;
}
/* }}} */

static svn_error_t * _svn_client_object_diff_summarize_func(const svn_client_diff_summarize_t *diff, void *baton, apr_pool_t *pool) /* {{{ */
{
	zval *arr = (zval *) baton, *elem_arr;

	ALLOC_INIT_ZVAL(elem_arr);
	array_init(elem_arr);

	ADD_ASSOC_STRING_OR_NULL(elem_arr, "path", diff->path);
	add_assoc_long(elem_arr, "summarize_kind", diff->summarize_kind);
	add_assoc_bool(elem_arr, "prop_changed", diff->prop_changed);
	add_assoc_long(elem_arr, "node_kind", diff->node_kind);

	add_next_index_zval(arr, elem_arr);

	return SVN_NO_ERROR;
}
/* }}} */

static svn_error_t * _svn_client_object_info_receiver_func(void *baton, const char *path, const svn_info_t *info, apr_pool_t *pool) /* {{{ */
{
	zval *arr = (void *) baton, *elem_arr, *tmp_zval;

	ALLOC_INIT_ZVAL(elem_arr);
	array_init(elem_arr);

	ADD_ASSOC_STRING_OR_NULL(elem_arr, "URL", info->URL);
	add_assoc_long(elem_arr, "rev", SVN_REVNUM_T_TO_LONG(info->rev));
	add_assoc_long(elem_arr, "kind", info->kind);
	ADD_ASSOC_STRING_OR_NULL(elem_arr, "repos_root_URL", info->repos_root_URL);
	ADD_ASSOC_STRING_OR_NULL(elem_arr, "repos_UUID", info->repos_UUID);
	add_assoc_long(elem_arr, "last_changed_rev", info->last_changed_rev);

	_apr_time_t_to_zval(&tmp_zval, info->last_changed_date);
	add_assoc_zval(elem_arr, "last_changed_date", tmp_zval);

	ADD_ASSOC_STRING_OR_NULL(elem_arr, "last_changed_author", info->last_changed_author);

	if (info->lock) {
		_svn_lock_t_to_zval(&tmp_zval, info->lock);
		add_assoc_zval(elem_arr, "lock", tmp_zval);
	} else {
		add_assoc_null(elem_arr, "lock");
	}

	add_assoc_bool(elem_arr, "has_wc_info", info->has_wc_info);

	if (info->has_wc_info) {
		add_assoc_long(elem_arr, "schedule", info->schedule);
		ADD_ASSOC_STRING_OR_NULL(elem_arr, "copyfrom_url", info->copyfrom_url);
		add_assoc_long(elem_arr, "copyfrom_rev", SVN_REVNUM_T_TO_LONG(info->copyfrom_rev));
		
		_apr_time_t_to_zval(&tmp_zval, info->text_time);
		add_assoc_zval(elem_arr, "text_time", tmp_zval);

		_apr_time_t_to_zval(&tmp_zval, info->prop_time);
		add_assoc_zval(elem_arr, "prop_time", tmp_zval);

		ADD_ASSOC_STRING_OR_NULL(elem_arr, "checksum", info->checksum);
		ADD_ASSOC_STRING_OR_NULL(elem_arr, "conflict_old", info->conflict_old);
		ADD_ASSOC_STRING_OR_NULL(elem_arr, "conflict_new", info->conflict_new);
		ADD_ASSOC_STRING_OR_NULL(elem_arr, "conflict_wrk", info->conflict_wrk);
		ADD_ASSOC_STRING_OR_NULL(elem_arr, "prejfile", info->prejfile);
	}

	path = (const char *) svn_path_local_style(path, pool);
	add_assoc_zval(arr, (char *) path, elem_arr);

	return SVN_NO_ERROR;
}
/* }}} */

static svn_error_t * _svn_client_object_log_msg_func(const char **log_msg, const char **tmp_file, const apr_array_header_t *commit_items, void *baton, apr_pool_t *pool) /* {{{ */
{
	zval *this = (zval *) baton, *log_msg_callback_zval, *log_msg_baton_zval;
	zval *null_baton_zval = NULL;
	TSRMLS_FETCH();
	svn_client_object *intern = zend_object_store_get_object(this TSRMLS_CC);
	zval *callback_retval = NULL, **callback_params[2];
	zval *commit_items_zval;
	int i;

	*log_msg = NULL;
	*tmp_file = NULL;

	log_msg_callback_zval = zend_read_property(svn_client_object_ce,
		this,	
		ZEND_STRL("log_msg_callback"),
		0 TSRMLS_CC);

	log_msg_baton_zval = zend_read_property(svn_client_object_ce,
		this,
		ZEND_STRL("log_msg_baton"),
		0 TSRMLS_CC);

	if (!log_msg_callback_zval || Z_TYPE_P(log_msg_callback_zval) == IS_NULL) {
		return SVN_NO_ERROR;
	}

	if (!zend_is_callable(log_msg_callback_zval, 0, NULL SVNOBJ_IS_CALLABLE_CC)) {
		php_error_docref(NULL TSRMLS_CC, 
			E_WARNING, 
			"Log msg callback is not callable");
		return SVN_NO_ERROR;
	}

	if (!log_msg_baton_zval) {
		ALLOC_INIT_ZVAL(null_baton_zval);
		ZVAL_NULL(null_baton_zval);
		callback_params[0] = &null_baton_zval;
	} else {
		callback_params[0] = &log_msg_baton_zval;
	}

	ALLOC_INIT_ZVAL(commit_items_zval);
	array_init(commit_items_zval);

	for (i=0; i < commit_items->nelts; i++) {
		svn_client_commit_item2_t *commit_item = APR_ARRAY_IDX(commit_items, i, 
			svn_client_commit_item2_t *);
		zval *commit_item_zval;

		_svn_client_commit_item2_t_to_zval(&commit_item_zval, 
			commit_item, pool);
		add_next_index_zval(commit_items_zval, commit_item_zval);
	}

	callback_params[1] = &commit_items_zval;

	if (SUCCESS == call_user_function_ex(EG(function_table), NULL, 
		log_msg_callback_zval, &callback_retval, 2, callback_params, 0, 
		NULL TSRMLS_CC) && callback_retval) {

		if (Z_TYPE_P(callback_retval) == IS_ARRAY) {
			HashTable *hash = Z_ARRVAL_P(callback_retval);
			zval **hash_zval;

			if (SUCCESS == zend_hash_find(hash, ZEND_STRS("log_msg"), (void **) &hash_zval)
				&& Z_TYPE_PP(hash_zval) != IS_NULL) {
				zval tmp = **hash_zval;
				zval_copy_ctor(&tmp);
				convert_to_string(&tmp);
				*log_msg = apr_pstrndup(pool, Z_STRVAL(tmp), Z_STRLEN(tmp));
				zval_dtor(&tmp);
			}
			if (SUCCESS == zend_hash_find(hash, ZEND_STRS("tmp_file"), (void **) &hash_zval)
				&& Z_TYPE_PP(hash_zval) != IS_NULL) {
				zval tmp = **hash_zval;
				zval_copy_ctor(&tmp);
				convert_to_string(&tmp);
				*tmp_file = apr_pstrndup(pool, Z_STRVAL(tmp), Z_STRLEN(tmp));
				zval_dtor(&tmp);
			}
		} else if (Z_TYPE_P(callback_retval) != IS_NULL) {
			php_error_docref(NULL TSRMLS_CC, 
				E_WARNING, 
				"Log msg callback's return value must be an array or null");
		}

		zval_ptr_dtor(&callback_retval);
	} else {
		if (!EG(exception)) {
			php_error_docref(NULL TSRMLS_CC,
				E_WARNING,
				"Failed to call log msg callback");
		}
	}

	if (null_baton_zval) {
		zval_ptr_dtor(&null_baton_zval);
	}

	zval_ptr_dtor(&commit_items_zval);

	return SVN_NO_ERROR;
}
/* }}} */

static svn_error_t * _svn_client_object_return_log_msg_func(const char **log_msg, const char **tmp_file, const apr_array_header_t *commit_items, void *baton, apr_pool_t *pool) /* {{{ */
{
	*log_msg = apr_pstrdup(pool, (char *) baton);
	*tmp_file = NULL;
	return SVN_NO_ERROR;
}
/* }}} */

static svn_error_t * _svn_client_object_default_log_msg_func(const char **log_msg, const char **tmp_file, const apr_array_header_t *commit_items, void *baton, apr_pool_t *pool) /* {{{ */
{
	*log_msg = NULL;
	*tmp_file = NULL;
	return SVN_NO_ERROR;
}
/* }}} */

static svn_error_t * _svn_client_object_cancel_func(void *baton) /* {{{ */
{
	zval *this = (zval *) baton, *cancel_callback_zval, *cancel_baton_zval;
	zval *null_baton_zval = NULL;
	TSRMLS_FETCH();
	svn_client_object *intern = zend_object_store_get_object(this TSRMLS_CC);
	zval *callback_retval = NULL, **callback_params[1];
	zend_bool ret_cancel = 1;

	cancel_callback_zval = zend_read_property(svn_client_object_ce,
		this,	
		ZEND_STRL("cancel_callback"),
		0 TSRMLS_CC);

	cancel_baton_zval = zend_read_property(svn_client_object_ce,
		this,
		ZEND_STRL("cancel_baton"),
		0 TSRMLS_CC);

	if (!cancel_callback_zval || Z_TYPE_P(cancel_callback_zval) == IS_NULL) {
		return SVN_NO_ERROR;
	}

	if (!zend_is_callable(cancel_callback_zval, 0, NULL SVNOBJ_IS_CALLABLE_CC)) {
		php_error_docref(NULL TSRMLS_CC, 
			E_WARNING, 
			"Cancel callback is not callable");
		return SVN_NO_ERROR;
	}

	if (!cancel_baton_zval) {
		ALLOC_INIT_ZVAL(null_baton_zval);
		ZVAL_NULL(null_baton_zval);
		callback_params[0] = &null_baton_zval;
	} else {
		callback_params[0] = &cancel_baton_zval;
	}

	if (SUCCESS == call_user_function_ex(EG(function_table), NULL, 
		cancel_callback_zval, &callback_retval, 1, callback_params, 0, 
		NULL TSRMLS_CC) && callback_retval) {

		if (Z_TYPE_P(callback_retval) == IS_BOOL) {
			ret_cancel = Z_BVAL_P(callback_retval);
		} else {
			zval tmp = *callback_retval;
			zval_copy_ctor(&tmp);
			convert_to_bool(&tmp);
			ret_cancel = Z_BVAL(tmp);
			zval_dtor(&tmp);
		}

		zval_ptr_dtor(&callback_retval);
	} else {
		if (!EG(exception)) {
			php_error_docref(NULL TSRMLS_CC,
				E_WARNING,
				"Failed to call cancel callback");
		}
	}

	if (null_baton_zval) {
		zval_ptr_dtor(&null_baton_zval);
	}


	if (ret_cancel) {
		return svn_error_create(SVN_ERR_CANCELLED, NULL, "Operation was cancelled by user");
	} else {
		return SVN_NO_ERROR;
	}
}
/* }}} */

static void _svn_client_object_get_property(char *name, int name_len, INTERNAL_FUNCTION_PARAMETERS) /* {{{ */
{
	zval *this = getThis(), *ret_zval;

	ret_zval = zend_read_property(svn_client_object_ce,
		this,
		name,
		name_len,
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

static zval * _svn_client_object_set_property(char *name, int name_len, const char *value_type, int validation_flag, INTERNAL_FUNCTION_PARAMETERS) /* {{{ */
{
	SVNCLIENT_FETCH_THIS_AND_INTERN();
	zval *value = NULL;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
		(char *) value_type,
		&value)) {
		RETVAL_FALSE;
		return NULL;
	}

	if (validation_flag & SET_PROPERTY_CALLABLE) {
		if (!zend_is_callable(value, IS_CALLABLE_CHECK_SYNTAX_ONLY, NULL SVNOBJ_IS_CALLABLE_CC)) {
			zend_throw_exception_ex(svn_invalid_argument_exception_object_ce,
				0 TSRMLS_CC,
				"%s must be callable",
				name);
			RETVAL_FALSE;
			return NULL;
		}
	}

	zend_update_property(svn_client_object_ce,
		this,
		name, name_len,
		value TSRMLS_CC);

	RETVAL_TRUE;
	return value;
}
/* *}}} */

static void _svn_client_object_free_storage(void *object TSRMLS_DC) /* {{{ */
{
	svn_client_object *intern = (svn_client_object *) object;

	if (!intern) {
		return;
	}

	intern->ctx = NULL;

	if (intern->pool) {
		apr_pool_destroy(intern->pool);
		intern->pool = NULL;
	}

	/* requires PHP >= 5.1.2 */
	zend_object_std_dtor(&intern->zo TSRMLS_CC);

	efree(intern);
}
/* }}} */

static zend_object_value _svn_client_object_new(zend_class_entry *class_type TSRMLS_DC) /* {{{ */
{
	zend_object_value retval;
	svn_client_object *intern;
	zval *tmp;

	apr_array_header_t *auth_providers;
	svn_error_t *svn_error;

	intern = emalloc(sizeof(svn_client_object));
	memset(&intern->zo, 0, sizeof(zend_object));

	intern->pool = NULL;
	intern->ctx  = NULL;

	apr_pool_create(&intern->pool, NULL);

	svn_error = svn_client_create_context(&intern->ctx, intern->pool);

	if (svn_error) {
		php_error_docref(NULL TSRMLS_CC, 
			E_ERROR, 
			"Failed to create svn_client_ctx_t: [%d] %s",
			svn_error->apr_err,
			svn_error->message);
	}

	intern->ctx->log_msg_func2 = _svn_client_object_default_log_msg_func;

	auth_providers = apr_array_make(intern->pool, 
			0, 
			sizeof(svn_auth_provider_object_t));

	svn_auth_open(&intern->ctx->auth_baton,
		auth_providers,
		intern->pool);

	zend_object_std_init(&intern->zo, class_type TSRMLS_CC);
	zend_hash_copy(intern->zo.properties, 
			&class_type->default_properties, 
			(copy_ctor_func_t) zval_add_ref,
			(void *) &tmp,
			sizeof(zval *));

	retval.handle = zend_objects_store_put(intern,
			NULL,
			(zend_objects_free_object_storage_t) _svn_client_object_free_storage,
			NULL TSRMLS_CC);
	retval.handlers = (zend_object_handlers *) &svn_client_object_handlers;

	return retval;
}
/* }}} */

PHP_MINIT_FUNCTION(svn_client) /* {{{ */
{
	SVNOBJ_REGISTER_CLASS_EX(SvnClient, svn_client_object, NULL, 0);

	SVNOBJ_DECLARE_PROPERTY_NULL(svn_client_object, "config");
	SVNOBJ_DECLARE_PROPERTY_NULL(svn_client_object, "auth_baton");
	SVNOBJ_DECLARE_PROPERTY_NULL(svn_client_object, "notify_callback");
	SVNOBJ_DECLARE_PROPERTY_NULL(svn_client_object, "notify_baton");
	SVNOBJ_DECLARE_PROPERTY_NULL(svn_client_object, "progress_callback");
	SVNOBJ_DECLARE_PROPERTY_NULL(svn_client_object, "progress_baton");
	SVNOBJ_DECLARE_PROPERTY_NULL(svn_client_object, "log_msg_callback");
	SVNOBJ_DECLARE_PROPERTY_NULL(svn_client_object, "log_msg_baton");
	SVNOBJ_DECLARE_PROPERTY_NULL(svn_client_object, "cancel_callback");
	SVNOBJ_DECLARE_PROPERTY_NULL(svn_client_object, "cancel_baton");

	SVNCLIENT_CONST_LONG(INVALID_REVNUM, SVN_INVALID_REVNUM);

	/* enum svn_node_kind_t */
	SVNCLIENT_CONST_LONG(NODE_NONE, svn_node_none);
	SVNCLIENT_CONST_LONG(NODE_FILE, svn_node_file);
	SVNCLIENT_CONST_LONG(NODE_DIR, svn_node_dir);
	SVNCLIENT_CONST_LONG(NODE_UNKNOWN, svn_node_unknown);

	/* enum svn_opt_revision_kind_t */
	SVNCLIENT_CONST_LONG(OPT_REVISION_UNSPECIFIED, svn_opt_revision_unspecified);
	SVNCLIENT_CONST_LONG(OPT_REVISION_NUMBER, svn_opt_revision_number);
	SVNCLIENT_CONST_LONG(OPT_REVISION_DATE, svn_opt_revision_date);
	SVNCLIENT_CONST_LONG(OPT_REVISION_COMMITTED, svn_opt_revision_committed);
	SVNCLIENT_CONST_LONG(OPT_REVISION_PREVIOUS, svn_opt_revision_previous);
	SVNCLIENT_CONST_LONG(OPT_REVISION_BASE, svn_opt_revision_base);
	SVNCLIENT_CONST_LONG(OPT_REVISION_WORKING, svn_opt_revision_working);
	SVNCLIENT_CONST_LONG(OPT_REVISION_HEAD, svn_opt_revision_head);

	/* enum svn_wc_notify_action_t */
	SVNCLIENT_CONST_LONG(WC_NOTIFY_ADD, svn_wc_notify_add);
	SVNCLIENT_CONST_LONG(WC_NOTIFY_COPY, svn_wc_notify_copy);
	SVNCLIENT_CONST_LONG(WC_NOTIFY_DELETE, svn_wc_notify_delete);
	SVNCLIENT_CONST_LONG(WC_NOTIFY_RESTORE, svn_wc_notify_restore);
	SVNCLIENT_CONST_LONG(WC_NOTIFY_REVERT, svn_wc_notify_revert);
	SVNCLIENT_CONST_LONG(WC_NOTIFY_FAILED_REVERT, svn_wc_notify_failed_revert);
	SVNCLIENT_CONST_LONG(WC_NOTIFY_RESOLVED, svn_wc_notify_resolved);
	SVNCLIENT_CONST_LONG(WC_NOTIFY_SKIP, svn_wc_notify_skip);
	SVNCLIENT_CONST_LONG(WC_NOTIFY_UPDATE_DELETE, svn_wc_notify_update_delete);
	SVNCLIENT_CONST_LONG(WC_NOTIFY_UPDATE_ADD, svn_wc_notify_update_add);
	SVNCLIENT_CONST_LONG(WC_NOTIFY_UPDATE_UPDATE, svn_wc_notify_update_update);
	SVNCLIENT_CONST_LONG(WC_NOTIFY_UPDATE_COMPLETED, svn_wc_notify_update_completed);
	SVNCLIENT_CONST_LONG(WC_NOTIFY_UPDATE_EXTERNAL, svn_wc_notify_update_external);
	SVNCLIENT_CONST_LONG(WC_NOTIFY_STATUS_COMPLETED, svn_wc_notify_status_completed);
	SVNCLIENT_CONST_LONG(WC_NOTIFY_STATUS_EXTERNAL, svn_wc_notify_status_external);
	SVNCLIENT_CONST_LONG(WC_NOTIFY_COMMIT_MODIFIED, svn_wc_notify_commit_modified);
	SVNCLIENT_CONST_LONG(WC_NOTIFY_COMMIT_ADDED, svn_wc_notify_commit_added);
	SVNCLIENT_CONST_LONG(WC_NOTIFY_COMMIT_DELETED, svn_wc_notify_commit_deleted);
	SVNCLIENT_CONST_LONG(WC_NOTIFY_COMMIT_REPLACED, svn_wc_notify_commit_replaced);
	SVNCLIENT_CONST_LONG(WC_NOTIFY_COMMIT_POSTFIX_TXDELTA, svn_wc_notify_commit_postfix_txdelta);
	SVNCLIENT_CONST_LONG(WC_NOTIFY_BLAME_REVISION, svn_wc_notify_blame_revision);
	SVNCLIENT_CONST_LONG(WC_NOTIFY_LOCKED, svn_wc_notify_locked);
	SVNCLIENT_CONST_LONG(WC_NOTIFY_UNLOCKED, svn_wc_notify_unlocked);
	SVNCLIENT_CONST_LONG(WC_NOTIFY_FAILED_LOCK, svn_wc_notify_failed_lock);
	SVNCLIENT_CONST_LONG(WC_NOTIFY_FAILED_UNLOCK, svn_wc_notify_failed_unlock);

	/* enum svn_wc_notify_state_t */
	SVNCLIENT_CONST_LONG(WC_NOTIFY_STATE_UNKNOWN, svn_wc_notify_state_unknown);
	SVNCLIENT_CONST_LONG(WC_NOTIFY_STATE_UNCHANGED, svn_wc_notify_state_unchanged);
	SVNCLIENT_CONST_LONG(WC_NOTIFY_STATE_MISSING, svn_wc_notify_state_missing);
	SVNCLIENT_CONST_LONG(WC_NOTIFY_STATE_OBSTRUCTED, svn_wc_notify_state_obstructed);
	SVNCLIENT_CONST_LONG(WC_NOTIFY_STATE_CHANGED, svn_wc_notify_state_changed);
	SVNCLIENT_CONST_LONG(WC_NOTIFY_STATE_MERGED, svn_wc_notify_state_merged);
	SVNCLIENT_CONST_LONG(WC_NOTIFY_STATE_CONFLICTED, svn_wc_notify_state_conflicted);

	/* enum svn_wc_notify_lock_state_t */
	SVNCLIENT_CONST_LONG(WC_NOTIFY_LOCK_STATE_UNCHANGED, svn_wc_notify_lock_state_unchanged);
	SVNCLIENT_CONST_LONG(WC_NOTIFY_LOCK_STATE_LOCKED, svn_wc_notify_lock_state_locked);
	SVNCLIENT_CONST_LONG(WC_NOTIFY_LOCK_STATE_UNLOCKED, svn_wc_notify_lock_state_unlocked);

	/* enum svn_wc_schedule_t */
	SVNCLIENT_CONST_LONG(WC_SCHEDULE_NORMAL, svn_wc_schedule_normal);
	SVNCLIENT_CONST_LONG(WC_SCHEDULE_ADD, svn_wc_schedule_add);
	SVNCLIENT_CONST_LONG(WC_SCHEDULE_DELETE, svn_wc_schedule_delete);
	SVNCLIENT_CONST_LONG(WC_SCHEDULE_REPLACE, svn_wc_schedule_replace);

	/* enum svn_diff_file_ignore_space_t */
	SVNCLIENT_CONST_LONG(DIFF_FILE_IGNORE_SPACE_NONE, svn_diff_file_ignore_space_none);
	SVNCLIENT_CONST_LONG(DIFF_FILE_IGNORE_SPACE_CHANGE, svn_diff_file_ignore_space_change);
	SVNCLIENT_CONST_LONG(DIFF_FILE_IGNORE_SPACE_ALL, svn_diff_file_ignore_space_all);

	/* enum svn_client_diff_summarize_kind_t */
	SVNCLIENT_CONST_LONG(CLIENT_DIFF_SUMMARIZE_KIND_NORMAL, svn_client_diff_summarize_kind_normal);
	SVNCLIENT_CONST_LONG(CLIENT_DIFF_SUMMARIZE_KIND_ADDED, svn_client_diff_summarize_kind_added);
	SVNCLIENT_CONST_LONG(CLIENT_DIFF_SUMMARIZE_KIND_MODIFIED, svn_client_diff_summarize_kind_modified);
	SVNCLIENT_CONST_LONG(CLIENT_DIFF_SUMMARIZE_KIND_DELETED, svn_client_diff_summarize_kind_deleted);

	/* commit state flags */
	SVNCLIENT_CONST_LONG(CLIENT_COMMIT_ITEM_ADD, SVN_CLIENT_COMMIT_ITEM_ADD);
	SVNCLIENT_CONST_LONG(CLIENT_COMMIT_ITEM_DELETE, SVN_CLIENT_COMMIT_ITEM_DELETE);
	SVNCLIENT_CONST_LONG(CLIENT_COMMIT_ITEM_TEXT_MODS, SVN_CLIENT_COMMIT_ITEM_TEXT_MODS);
	SVNCLIENT_CONST_LONG(CLIENT_COMMIT_ITEM_PROP_MODS, SVN_CLIENT_COMMIT_ITEM_PROP_MODS);
	SVNCLIENT_CONST_LONG(CLIENT_COMMIT_ITEM_IS_COPY, SVN_CLIENT_COMMIT_ITEM_IS_COPY);
	SVNCLIENT_CONST_LONG(CLIENT_COMMIT_ITEM_LOCK_TOKEN, SVN_CLIENT_COMMIT_ITEM_LOCK_TOKEN);

#include "svn_error_codes_map.h"

	return SUCCESS;
}
/* }}} */

/* {{{ proto public static array SvnClient::getVersion() */
PHP_METHOD(SvnClient, getVersion)
{
	const svn_version_t *svn_version = svn_client_version();

	array_init(return_value);
	add_assoc_long(return_value, "major", svn_version->major);
	add_assoc_long(return_value, "minor", svn_version->minor);
	add_assoc_long(return_value, "patch", svn_version->patch);
	add_assoc_string(return_value, "tag", (char *) svn_version->tag, 1);
}
/* }}} */

/* {{{ proto public static string SvnClient::urlFromPath() */
PHP_METHOD(SvnClient, urlFromPath)
{
	char *path_or_url = NULL;
	int path_or_url_len = 0;

	char *url;
	svn_error_t *svn_error;
	apr_pool_t *tmp_pool;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"s", 
		&path_or_url, &path_or_url_len)) {
		RETURN_FALSE;
	}

	apr_pool_create(&tmp_pool, NULL);

	path_or_url = (char *) svn_normalized_path(path_or_url, tmp_pool);

	svn_error = svn_client_url_from_path((const char **) &url,
		path_or_url,
		tmp_pool);

	if (svn_error) {
		apr_pool_destroy(tmp_pool);
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}

	if (url) {
		RETVAL_STRING(url, 1);
	}

	apr_pool_destroy(tmp_pool);
}
/* }}} */

/* {{{ proto public SvnConfig SvnClient::getConfig() */
PHP_METHOD(SvnClient, getConfig)
{
	_svn_client_object_get_property(ZEND_STRL("config"), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto public bool SvnClient::setConfig(SvnConfig config) */
PHP_METHOD(SvnClient, setConfig)
{
	SVNCLIENT_FETCH_THIS_AND_INTERN();
	svn_config_object *intern_config;
	zval *config_zval = NULL;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"o", 
		&config_zval)) {
		RETURN_FALSE;
	}

	if (!svn_config_is_instance(config_zval TSRMLS_CC)) {
		zend_throw_exception_ex(svn_invalid_argument_exception_object_ce,
			0 TSRMLS_CC,
			"config must be an instance of SvnConfig");
		RETURN_FALSE;
	}

	zend_update_property(svn_client_object_ce,
		this,
		ZEND_STRL("config"),
		config_zval TSRMLS_CC);

	intern_config = zend_object_store_get_object(config_zval TSRMLS_CC);
	intern->ctx->config = intern_config->cfg_hash;

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto public SvnAuthBaton SvnClient::getAuthBaton() */
PHP_METHOD(SvnClient, getAuthBaton)
{
	_svn_client_object_get_property(ZEND_STRL("auth_baton"), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto public bool SvnClient::setAuthBaton(SvnAuthBaton auth_baton) */
PHP_METHOD(SvnClient, setAuthBaton)
{
	SVNCLIENT_FETCH_THIS_AND_INTERN();
	svn_auth_baton_object *intern_baton;
	zval *auth_baton_zval = NULL;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"o", 
		&auth_baton_zval)) {
		RETURN_FALSE;
	}

	if (!svn_auth_baton_is_instance(auth_baton_zval TSRMLS_CC)) {
		zend_throw_exception_ex(svn_invalid_argument_exception_object_ce,
			0 TSRMLS_CC,
			"auth_baton must be an instance of SvnAuthBaton");
		RETURN_FALSE;
	}

	zend_update_property(svn_client_object_ce,
		this,
		ZEND_STRL("auth_baton"),
		auth_baton_zval TSRMLS_CC);

	intern_baton = zend_object_store_get_object(auth_baton_zval TSRMLS_CC);
	intern->ctx->auth_baton = intern_baton->auth_baton;

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto public mixed SvnClient::getProgressCallback() */
PHP_METHOD(SvnClient, getProgressCallback)
{
	_svn_client_object_get_property(ZEND_STRL("progress_callback"), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto public bool SvnClient::setProgressCallback(callback callback) */
PHP_METHOD(SvnClient, setProgressCallback)
{
	SVNCLIENT_FETCH_THIS_AND_INTERN();
	zval *prop_zval;

	prop_zval = _svn_client_object_set_property(ZEND_STRL("progress_callback"),
		"z",
		SET_PROPERTY_CALLABLE,
		INTERNAL_FUNCTION_PARAM_PASSTHRU);

	if (!prop_zval || Z_TYPE_P(prop_zval) == IS_NULL) {
		intern->ctx->progress_func = NULL;
		intern->ctx->progress_baton = NULL;
	} else {
		intern->ctx->progress_func = _svn_client_object_progress_func;
		intern->ctx->progress_baton = this;
	}
}
/* }}} */

/* {{{ proto public mixed SvnClient::getProgressBaton() */
PHP_METHOD(SvnClient, getProgressBaton)
{
	_svn_client_object_get_property(ZEND_STRL("progress_baton"), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto public bool SvnClient::setProgressBaton(mixed baton) */
PHP_METHOD(SvnClient, setProgressBaton)
{
	_svn_client_object_set_property(ZEND_STRL("progress_baton"),
		"z",
		0,
		INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto public mixed SvnClient::getNotifyCallback() */
PHP_METHOD(SvnClient, getNotifyCallback)
{
	_svn_client_object_get_property(ZEND_STRL("notify_callback"), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto public bool SvnClient::setNotifyCallback(mixed callback) */
PHP_METHOD(SvnClient, setNotifyCallback)
{
	SVNCLIENT_FETCH_THIS_AND_INTERN();
	zval *prop_zval;

	prop_zval = _svn_client_object_set_property(ZEND_STRL("notify_callback"),
		"z",
		SET_PROPERTY_CALLABLE,
		INTERNAL_FUNCTION_PARAM_PASSTHRU);

	intern->ctx->notify_baton2 = this;

	if (!prop_zval || Z_TYPE_P(prop_zval) == IS_NULL) {
		intern->ctx->notify_func2 = NULL;
	} else {
		intern->ctx->notify_func2 = _svn_client_object_notify_func;
	}
}
/* }}} */

/* {{{ proto public mixed SvnClient::getNotifyBaton() */
PHP_METHOD(SvnClient, getNotifyBaton)
{
	_svn_client_object_get_property(ZEND_STRL("notify_baton"), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto public bool SvnClient::setNotifyBaton(mixed baton) */
PHP_METHOD(SvnClient, setNotifyBaton)
{
	_svn_client_object_set_property(ZEND_STRL("notify_baton"),
		"z",
		0,
		INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto public mixed SvnClient::getLogMsgCallback() */
PHP_METHOD(SvnClient, getLogMsgCallback)
{
	_svn_client_object_get_property(ZEND_STRL("log_msg_callback"), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto public bool SvnClient::setLogMsgCallback(mixed callback) */
PHP_METHOD(SvnClient, setLogMsgCallback)
{
	SVNCLIENT_FETCH_THIS_AND_INTERN();
	zval *prop_zval;

	prop_zval = _svn_client_object_set_property(ZEND_STRL("log_msg_callback"),
		"z",
		SET_PROPERTY_CALLABLE,
		INTERNAL_FUNCTION_PARAM_PASSTHRU);

	intern->ctx->log_msg_baton2 = this;

	if (!prop_zval || Z_TYPE_P(prop_zval) == IS_NULL) {
		intern->ctx->log_msg_func2 = _svn_client_object_default_log_msg_func;
	} else {
		intern->ctx->log_msg_func2 = _svn_client_object_log_msg_func;
	}
}
/* }}} */

/* {{{ proto public mixed SvnClient::getLogMsgBaton() */
PHP_METHOD(SvnClient, getLogMsgBaton)
{
	_svn_client_object_get_property(ZEND_STRL("log_msg_baton"), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto public bool SvnClient::setLogMsgBaton(mixed baton) */
PHP_METHOD(SvnClient, setLogMsgBaton)
{
	_svn_client_object_set_property(ZEND_STRL("log_msg_baton"),
		"z",
		0,
		INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto public mixed SvnClient::getCancelCallback()  */
PHP_METHOD(SvnClient, getCancelCallback)
{
	_svn_client_object_get_property(ZEND_STRL("cancel_callback"), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto public bool SvnClient::setCancelCallback(mixed callback) */
PHP_METHOD(SvnClient, setCancelCallback)
{
	SVNCLIENT_FETCH_THIS_AND_INTERN();
	zval *prop_zval;

	prop_zval = _svn_client_object_set_property(ZEND_STRL("cancel_callback"),
		"z",
		SET_PROPERTY_CALLABLE,
		INTERNAL_FUNCTION_PARAM_PASSTHRU);

	intern->ctx->cancel_baton = this;

	if (!prop_zval || Z_TYPE_P(prop_zval) == IS_NULL) {
		intern->ctx->cancel_func = NULL;
	} else {
		intern->ctx->cancel_func = _svn_client_object_cancel_func;
	}
}
/* }}} */

/* {{{ proto public mixed SvnClient::getCancelBaton() */
PHP_METHOD(SvnClient, getCancelBaton)
{
	_svn_client_object_get_property(ZEND_STRL("cancel_baton"), INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto public bool SvnClient::setCancelBaton(mixed baton) */
PHP_METHOD(SvnClient, setCancelBaton)
{
	_svn_client_object_set_property(ZEND_STRL("cancel_baton"),
		"z",
		0,
		INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/*{{{ proto public int SvnClient::checkout(string url, string path [, mixed peg_revision, mixed revision, bool recurse, bool ignore_externals]) */
PHP_METHOD(SvnClient, checkout)
{
	SVNCLIENT_FETCH_THIS_AND_INTERN();
	char *url = NULL, *path = NULL;
	int url_len = 0, path_len = 0;
	zval *peg_revision_zval = NULL, *revision_zval = NULL;
	zend_bool recurse = 1, ignore_externals = 0;

	svn_error_t *svn_error;
	svn_revnum_t result_rev;
	svn_opt_revision_t revision, peg_revision;
	apr_pool_t *tmp_pool;
	int is_url;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"ss|zzbb", 
		&url, &url_len,
		&path, &path_len,
		&peg_revision_zval,
		&revision_zval,
		&recurse,
		&ignore_externals)) {
		RETURN_FALSE;
	}

	apr_pool_create(&tmp_pool, intern->pool);

	url = (char *) svn_normalized_path(url, tmp_pool);
	path = (char *) svn_normalized_path(path, tmp_pool);

	_zval_to_svn_opt_revision_t(&revision, 
		revision_zval, 
		&svn_opt_revision_head_default);
	_zval_to_svn_opt_revision_t(&peg_revision, 
		peg_revision_zval, 
		&revision);

	is_url = svn_path_is_url(url);
	REVISION_KIND_CHECK(is_url, peg_revision, "peg_revision");
	REVISION_KIND_CHECK(is_url, revision, "revision");

	/* 1.4 API */
	svn_error = svn_client_checkout2(&result_rev,
		url,
		path,
		&peg_revision,
		&revision,
		recurse,
		ignore_externals,
		intern->ctx,
		tmp_pool);

	if (svn_error) {
		apr_pool_destroy(tmp_pool);
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}

	RETVAL_LONG(SVN_REVNUM_T_TO_LONG(result_rev));
	apr_pool_destroy(tmp_pool);
}
/*}}}*/

/* {{{ proto public mixed SvnClient::update(mixed paths [, mixed revision, bool recurse, bool ignore_externals]) */
PHP_METHOD(SvnClient, update)
{
	SVNCLIENT_FETCH_THIS_AND_INTERN();
	zval *paths_zval = NULL, *revision_zval = NULL;
	zend_bool recurse = 1, ignore_externals = 0;

	apr_pool_t *tmp_pool;
	apr_array_header_t *result_revs = NULL, *paths;
	svn_opt_revision_t revision;
	svn_error_t *svn_error;
	int should_return_array;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"z|zbb", 
		&paths_zval,
		&revision_zval,
		&recurse,
		&ignore_externals)) {
		RETURN_FALSE;
	}

	apr_pool_create(&tmp_pool, intern->pool);

	should_return_array = _zval_to_apr_array_of_strings(&paths, 
		paths_zval, 
		1,
		tmp_pool);

	_zval_to_svn_opt_revision_t(&revision, 
		revision_zval, 
		&svn_opt_revision_head_default);

	svn_error = svn_client_update2(&result_revs,
		paths,
		&revision,
		recurse,
		ignore_externals,
		intern->ctx,
		tmp_pool);

	if (svn_error) {
		apr_pool_destroy(tmp_pool);
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}

	if (should_return_array) {
		int i;

		array_init(return_value);

		for (i = 0; i < result_revs->nelts; i++) {
			add_next_index_long(return_value, SVN_REVNUM_T_TO_LONG(
				APR_ARRAY_IDX(result_revs, i, svn_revnum_t)));
		}
	} else {
		RETVAL_LONG(SVN_REVNUM_T_TO_LONG(
			APR_ARRAY_IDX(result_revs, 0, svn_revnum_t)));
	}

	apr_pool_destroy(tmp_pool);
}
/* }}} */

/* {{{ proto public int SvnClient::switch(string path, string url [, mixed revision, bool recurse]) */
PHP_METHOD(SvnClient, switch)
{
	SVNCLIENT_FETCH_THIS_AND_INTERN();
	char *url = NULL, *path = NULL;
	int url_len = 0, path_len = 0;
	zval *revision_zval = NULL;
	zend_bool recurse = 1;

	apr_pool_t *tmp_pool;
	svn_error_t *svn_error;
	svn_revnum_t result_rev;
	svn_opt_revision_t revision;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"ss|zb", 
		&path, &path_len,
		&url, &url_len,
		&revision_zval,
		&recurse)) {
		RETURN_FALSE;
	}
	
	apr_pool_create(&tmp_pool, intern->pool);

	url = (char *) svn_normalized_path(url, tmp_pool);
	path = (char *) svn_normalized_path(path, tmp_pool);

	_zval_to_svn_opt_revision_t(&revision, 
		revision_zval, 
		&svn_opt_revision_head_default);

	/* 1.4 API */
	svn_error = svn_client_switch(&result_rev,
		path,
		url,
		&revision,
		recurse,
		intern->ctx,
		tmp_pool);

	if (svn_error) {
		apr_pool_destroy(tmp_pool);
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}

	RETVAL_LONG(SVN_REVNUM_T_TO_LONG(result_rev));
	apr_pool_destroy(tmp_pool);
}
/* }}} */

/* {{{ proto public bool SvnClient::add(string path [, bool recursive, bool force, bool no_ignore]) */
PHP_METHOD(SvnClient, add)
{
	SVNCLIENT_FETCH_THIS_AND_INTERN();
	char *path = NULL;
	int path_len = 0;
	zend_bool recursive = 1, force = 0, no_ignore = 0;

	apr_pool_t *tmp_pool;
	svn_error_t *svn_error;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"s|bbb", 
		&path, &path_len,
		&recursive,
		&force,
		&no_ignore)) {
		RETURN_FALSE;
	}

	apr_pool_create(&tmp_pool, intern->pool);

	path = (char *) svn_normalized_path(path, tmp_pool);

	/* 1.4 API */
	svn_error = svn_client_add3(path,
		recursive,
		force,
		no_ignore,
		intern->ctx,
		tmp_pool);

	if (svn_error) {
		apr_pool_destroy(tmp_pool);
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}

	apr_pool_destroy(tmp_pool);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto public mixed SvnClient::mkdir(mixed paths [, string log_msg]) */
PHP_METHOD(SvnClient, mkdir)
{
	SVNCLIENT_FETCH_THIS_AND_INTERN();
	zval *paths_zval = NULL;
	char *log_msg = NULL;
	int log_msg_len = 0;

	svn_client_get_commit_log2_t orig_log_msg_func = NULL;
	void *orig_log_msg_baton = NULL;

	apr_pool_t *tmp_pool;
	apr_array_header_t *paths;
	svn_commit_info_t *commit_info = NULL;
	svn_error_t *svn_error;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"z|s", 
		&paths_zval,
		&log_msg, &log_msg_len)) {
		RETURN_FALSE;
	}

	apr_pool_create(&tmp_pool, intern->pool);

	_zval_to_apr_array_of_strings(&paths, 
		paths_zval, 
		1,
		tmp_pool);

	if (log_msg) {
		orig_log_msg_func = intern->ctx->log_msg_func2;
		orig_log_msg_baton = intern->ctx->log_msg_baton2;
		intern->ctx->log_msg_func2 = _svn_client_object_return_log_msg_func;
		intern->ctx->log_msg_baton2 = log_msg;
	}

	/* 1.4 API */
	svn_error = svn_client_mkdir2(&commit_info,
		paths,
		intern->ctx,
		tmp_pool);

	if (log_msg) {
		intern->ctx->log_msg_func2 = orig_log_msg_func;
		intern->ctx->log_msg_baton2 = orig_log_msg_baton;
	}

	if (svn_error) {
		apr_pool_destroy(tmp_pool);
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}

	_svn_commit_info_t_to_zval(&return_value, commit_info);

	apr_pool_destroy(tmp_pool);
}
/* }}} */

/* {{{ proto public mixed SvnClient::delete(mixed paths [, bool force]) */
PHP_METHOD(SvnClient, delete)
{
	SVNCLIENT_FETCH_THIS_AND_INTERN();
	zval *paths_zval = NULL;
	zend_bool force = 0;

	apr_pool_t *tmp_pool;
	apr_array_header_t *paths;
	svn_commit_info_t *commit_info = NULL;
	svn_error_t *svn_error;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"z|b", 
		&paths_zval,
		&force)) {
		RETURN_FALSE;
	}

	apr_pool_create(&tmp_pool, intern->pool);

	_zval_to_apr_array_of_strings(&paths, 
		paths_zval, 
		1,
		tmp_pool);

	/* 1.4 API */
	svn_error = svn_client_delete2(&commit_info,
		paths,
		force,
		intern->ctx,
		tmp_pool);

	if (svn_error) {
		apr_pool_destroy(tmp_pool);
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}

	_svn_commit_info_t_to_zval(&return_value, commit_info);

	apr_pool_destroy(tmp_pool);
}
/* }}} */

/* {{{ proto public mixed SvnClient::import(string path, string url, [bool nonrecursive, bool no_ignore]) */
PHP_METHOD(SvnClient, import)
{
	SVNCLIENT_FETCH_THIS_AND_INTERN();
	char *url = NULL, *path = NULL;
	int url_len = 0, path_len = 0;
	zend_bool nonrecursive = 0, no_ignore = 1;

	apr_pool_t *tmp_pool;
	svn_commit_info_t *commit_info = NULL;
	svn_error_t *svn_error;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"ss|bb", 
		&path, &path_len,
		&url, &url_len,
		&nonrecursive,
		&no_ignore)) {
		RETURN_FALSE;
	}
	
	apr_pool_create(&tmp_pool, intern->pool);

	url = (char *) svn_normalized_path(url, tmp_pool);
	path = (char *) svn_normalized_path(path, tmp_pool);

	/* 1.4 API */
	svn_error = svn_client_import2(&commit_info,
		path,
		url,
		nonrecursive,
		no_ignore,
		intern->ctx,
		tmp_pool);

	if (svn_error) {
		apr_pool_destroy(tmp_pool);
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}

	if (commit_info) {
		_svn_commit_info_t_to_zval(&return_value, commit_info);
	} else {
		RETVAL_FALSE;
	}

	apr_pool_destroy(tmp_pool);
}
/* }}} */

/* {{{ proto public array SvnClient::commit(mixed targets [, string log_msg, bool recurse, bool keep_locks]) */
PHP_METHOD(SvnClient, commit)
{
	SVNCLIENT_FETCH_THIS_AND_INTERN();
	zval *targets_zval = NULL;
	char *log_msg = NULL;
	int log_msg_len = 0;
	zend_bool recurse = 1, keep_locks = 1;

	svn_client_get_commit_log2_t orig_log_msg_func = NULL;
	void *orig_log_msg_baton = NULL;

	apr_pool_t *tmp_pool;
	apr_array_header_t *targets;
	/* commit_info must be NULL or a segfault occurs. */
	svn_commit_info_t *commit_info = NULL;
	svn_error_t *svn_error;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"z|sbb", 
		&targets_zval,
		&log_msg, &log_msg_len,
		&recurse,
		&keep_locks)) {
		RETURN_FALSE;
	}

	apr_pool_create(&tmp_pool, intern->pool);

	_zval_to_apr_array_of_strings(&targets, 
		targets_zval, 
		1,
		tmp_pool);

	if (log_msg) {
		orig_log_msg_func = intern->ctx->log_msg_func2;
		orig_log_msg_baton = intern->ctx->log_msg_baton2;
		intern->ctx->log_msg_func2 = _svn_client_object_return_log_msg_func;
		intern->ctx->log_msg_baton2 = log_msg;
	}

	/* 1.4 API */
	svn_error = svn_client_commit3(&commit_info,
		targets,
		recurse,
		keep_locks,
		intern->ctx,
		tmp_pool);

	if (log_msg) {
		intern->ctx->log_msg_func2 = orig_log_msg_func;
		intern->ctx->log_msg_baton2 = orig_log_msg_baton;
	}

	if (svn_error) {
		apr_pool_destroy(tmp_pool);
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}

	_svn_commit_info_t_to_zval(&return_value, commit_info);

	apr_pool_destroy(tmp_pool);
}
/* }}} */

/* {{{ proto public array SvnClient::status(string path [, bool recurse, bool get_all, bool update, bool no_ignore, bool ignore_externals])*/
PHP_METHOD(SvnClient, status)
{
	SVNCLIENT_FETCH_THIS_AND_INTERN();
	char *path = NULL;
	int path_len = 0;
	zend_bool recurse = 1, get_all = 1, update = 0, no_ignore = 1, ignore_externals = 0;

	svn_error_t *svn_error;
	svn_revnum_t result_rev;
	svn_wc_status_func2_t_baton baton;
	apr_pool_t *tmp_pool;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"s|bbbbb", 
		&path, &path_len,
		&recurse,
		&get_all,
		&update,
		&no_ignore,
		&ignore_externals)) {
		RETURN_FALSE;
	}

	apr_pool_create(&tmp_pool, intern->pool);

	array_init(return_value);

	baton.arr  = return_value;
	baton.pool = tmp_pool;

	svn_error = svn_client_status2(&result_rev,
		path,
		&svn_opt_revision_head_default,
		_svn_client_object_status_func,
		(void *) &baton,
		recurse,
		get_all,
		update,
		no_ignore,
		ignore_externals,
		intern->ctx,
		tmp_pool);

	apr_pool_destroy(tmp_pool);

	if (svn_error) {
		zval_dtor(return_value);
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}
}
/* }}} */

/* {{{ proto public array SvnClient::log(mixed targets [, mixed peg_revision, mixed start, mixed end, int limit, bool discover_changed_paths, bool strict_node_discovery]) */
PHP_METHOD(SvnClient, log)
{
	SVNCLIENT_FETCH_THIS_AND_INTERN();
	zval *targets_zval = NULL;
	zval *peg_revision_zval = NULL, *start_zval = NULL, *end_zval = NULL;
	long limit = 0;
	zend_bool discover_changed_paths = 0, strict_node_discovery = 1;

	apr_pool_t *tmp_pool;
	apr_array_header_t *targets;
	svn_opt_revision_t peg_revision, start, end;
	svn_error_t *svn_error;

	int i;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"z|zzzlbb", 
		&targets_zval,
		&peg_revision_zval,
		&start_zval,
		&end_zval,
		&limit,
		&discover_changed_paths,
		&strict_node_discovery)) {
		RETURN_FALSE;
	}

	apr_pool_create(&tmp_pool, intern->pool);

	_zval_to_apr_array_of_strings(&targets, 
		targets_zval, 
		1,
		tmp_pool);

	_zval_to_svn_opt_revision_t(&peg_revision, peg_revision_zval, &svn_opt_revision_unspecified_default);
	_zval_to_svn_opt_revision_t(&start, start_zval, &svn_opt_revision_head_default);
	_zval_to_svn_opt_revision_t(&end, end_zval, &svn_opt_revision_number_default);

	for (i = 0; i < targets->nelts; i++) {
		char *target = APR_ARRAY_IDX(targets, i, char *);
		int is_url = svn_path_is_url(target);
		REVISION_KIND_CHECK(is_url, peg_revision, "peg_revision");
		REVISION_KIND_CHECK(is_url, start, "start");
		REVISION_KIND_CHECK(is_url, end, "end");
	}

	array_init(return_value);

	svn_error = svn_client_log3(targets,
		&peg_revision,
		&start,
		&end,
		limit,
		discover_changed_paths,
		strict_node_discovery,
		_svn_client_object_log_message_receiver_func,
		(void *) return_value,
		intern->ctx,
		tmp_pool);

	apr_pool_destroy(tmp_pool);

	if (svn_error) {
		zval_dtor(return_value);
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}
}
/* }}} */

/* {{{ proto public string SvnClient::blame(string path_or_url [, mixed peg_revision, mixed start, mixed end, array diff_options, bool ignore_mime_type]) */
PHP_METHOD(SvnClient, blame)
{
	SVNCLIENT_FETCH_THIS_AND_INTERN();
	char *path_or_url = NULL;
	int path_or_url_len = 0;
	zval *peg_revision_zval = NULL, *start_zval = NULL, *end_zval = NULL;
	zval *diff_options_zval = NULL;
	zend_bool ignore_mime_type = 0;

	apr_pool_t *tmp_pool;
	svn_opt_revision_t peg_revision, start, end;
	svn_diff_file_options_t diff_options;
	svn_error_t *svn_error;
	int is_url;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"s|zzzzb", 
		&path_or_url, &path_or_url_len,
		&peg_revision_zval,
		&start_zval,
		&end_zval,
		&diff_options_zval,
		&ignore_mime_type)) {
		RETURN_FALSE;
	}

	apr_pool_create(&tmp_pool, intern->pool);

	_zval_to_svn_opt_revision_t(&start, 
		start_zval, 
		&svn_opt_revision_number_default);
	_zval_to_svn_opt_revision_t(&end, 
		end_zval, 
		&svn_opt_revision_head_default);
	_zval_to_svn_opt_revision_t(&peg_revision, 
		peg_revision_zval, 
		&end);

	is_url = svn_path_is_url(path_or_url);
	REVISION_KIND_CHECK(is_url, peg_revision, "peg_revision");
	REVISION_KIND_CHECK(is_url, start, "start");
	REVISION_KIND_CHECK(is_url, end, "end");

	_zval_to_svn_diff_file_options_t(&diff_options, diff_options_zval);

	array_init(return_value);

	/* 1.4 API */
	svn_error = svn_client_blame3(path_or_url,
		&peg_revision,
		&start,
		&end,
		&diff_options,
		ignore_mime_type,
		_svn_client_object_blame_receiver_func,
		(void *) return_value,
		intern->ctx,
		tmp_pool);

	apr_pool_destroy(tmp_pool);

	if (svn_error) {
		zval_dtor(return_value);
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}
}
/* }}} */

/* {{{ proto public string SvnClient::diff(string tmp_path, string path1 [, mixed revision1, string path2, mixed revision2, bool recurse, bool ignore_ancestry, bool no_diff_related, bool ignore_content_type, string header_encoding, array diff_options]) */
PHP_METHOD(SvnClient, diff)
{
	SVNCLIENT_FETCH_THIS_AND_INTERN();
	char *tmp_path = NULL, *path1 = NULL, *path2 = NULL, *header_encoding = NULL;
	int tmp_path_len = 0, path1_len = 0, path2_len = 0, header_encoding_len = 0;
	zval *revision1_zval = NULL, *revision2_zval = NULL, *diff_options_zval = NULL;
	zend_bool recurse = 1, ignore_ancestry = 1, no_diff_related = 1, ignore_content_type = 0;

	apr_pool_t *tmp_pool;
	apr_array_header_t *diff_options;
	apr_status_t apr_status = APR_SUCCESS;
	svn_stringbuf_t *stringbuf;
	svn_opt_revision_t revision1, revision2;
	apr_file_t *out_file = NULL, *err_file = NULL;
	char *out_file_name, *err_file_name;
	svn_error_t *svn_error = NULL;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"ss|zszbbbbsa", 
		&tmp_path, &tmp_path_len,
		&path1, &path1_len,
		&revision1_zval,
		&path2, &path2_len,
		&revision2_zval,
		&recurse,
		&ignore_ancestry,
		&no_diff_related,
		&ignore_content_type,
		&header_encoding, &header_encoding_len,
		&diff_options_zval)) {
		RETURN_FALSE;
	}

	apr_pool_create(&tmp_pool, intern->pool);

	if (diff_options_zval) {
		_zval_to_apr_array_of_strings(&diff_options, 
			diff_options_zval, 
			0, 
			tmp_pool);
	} else {
		diff_options = apr_array_make(tmp_pool, 0, sizeof (const char *));
	}

	tmp_path = (char *) svn_normalized_path(tmp_path, tmp_pool);
	path1 = (char *) svn_normalized_path(path1, tmp_pool);

	if (path2) {
		path2 = (char *) svn_normalized_path(path2, tmp_pool);
	} else {
		path2 = path1;
	}

	svn_error = OPEN_UNIQUE_FILE(out_file, out_file_name, tmp_path, tmp_pool);

	if (svn_error) {
		goto cleanup;
	}

	svn_error = OPEN_UNIQUE_FILE(err_file, err_file_name, tmp_path, tmp_pool);

	if (svn_error) {
		goto cleanup;
	}

	_zval_to_svn_opt_revision_t(&revision1, 
		revision1_zval, 
		&svn_opt_revision_base_default);
	_zval_to_svn_opt_revision_t(&revision2, 
		revision2_zval, 
		&svn_opt_revision_working_default);

	if (!header_encoding) {
		header_encoding = (char *) SVN_APR_LOCALE_CHARSET;
	}

	/* 1.4 API */
	svn_error = svn_client_diff3(diff_options,
		path1,
		&revision1,
		path2,
		&revision2,
		recurse,
		ignore_ancestry,
		no_diff_related,
		ignore_content_type,
		header_encoding,
		out_file,
		err_file,
		intern->ctx,
		tmp_pool);

	if (svn_error) {
		goto cleanup;
	}

	apr_status = apr_file_close(out_file);

	if (apr_status != APR_SUCCESS) {
		zend_throw_exception_ex(svn_exception_object_ce,
			apr_status TSRMLS_CC,
			"Failed to close temporary file %s",
			out_file_name);
		goto cleanup;
	}

	apr_status = apr_file_open(&out_file, 
		out_file_name, 
		APR_READ, 
		APR_OS_DEFAULT, 
		tmp_pool);

	if (apr_status != APR_SUCCESS) {
		out_file = NULL;
		zend_throw_exception_ex(svn_exception_object_ce,
			apr_status TSRMLS_CC,
			"Failed to reopen temporary file %s",
			out_file_name);
		goto cleanup;
	}

	svn_error = svn_stringbuf_from_aprfile(&stringbuf,
		out_file,
		tmp_pool);

	if (svn_error) {
		goto cleanup;
	}

	RETVAL_STRINGL(stringbuf->data, stringbuf->len, 1);
cleanup:
	if (out_file) {
		apr_file_close(out_file);
	}
	if (err_file) {
		apr_file_close(err_file);
	}

	apr_pool_destroy(tmp_pool);

	if (svn_error) {
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}
}
/* }}} */

/* {{{ proto public string SvnClient::diffPeg(string tmp_path, string path [, mixed peg_revision, mixed start_revision, mixed end_revision, bool recurse, bool ignore_ancestry, bool no_diff_related, bool ignore_content_type, string header_encoding, array diff_options]) */
PHP_METHOD(SvnClient, diffPeg)
{
	SVNCLIENT_FETCH_THIS_AND_INTERN();
	char *tmp_path = NULL, *path = NULL, *header_encoding = NULL;
	int tmp_path_len = 0, path_len = 0, header_encoding_len = 0;
	zval *peg_revision_zval = NULL, *start_revision_zval = NULL, *end_revision_zval = NULL, *diff_options_zval = NULL;
	zend_bool recurse = 1, ignore_ancestry = 1, no_diff_related = 1, ignore_content_type = 0;

	apr_pool_t *tmp_pool;
	apr_array_header_t *diff_options;
	apr_status_t apr_status = APR_SUCCESS;
	svn_stringbuf_t *stringbuf;
	svn_opt_revision_t peg_revision, start_revision, end_revision;
	apr_file_t *out_file = NULL, *err_file = NULL;
	char *out_file_name, *err_file_name;
	svn_error_t *svn_error = NULL;
	int is_url;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"ss|zzzbbbbsa", 
		&tmp_path, &tmp_path_len,
		&path, &path_len,
		&peg_revision_zval,
		&start_revision_zval,
		&end_revision_zval,
		&recurse,
		&ignore_ancestry,
		&no_diff_related,
		&ignore_content_type,
		&header_encoding, &header_encoding_len,
		&diff_options_zval)) {
		RETURN_FALSE;
	}

	apr_pool_create(&tmp_pool, intern->pool);

	_zval_to_svn_opt_revision_t(&start_revision, 
		start_revision_zval, 
		&svn_opt_revision_base_default);
	_zval_to_svn_opt_revision_t(&end_revision, 
		end_revision_zval, 
		&svn_opt_revision_working_default);
	_zval_to_svn_opt_revision_t(&peg_revision, 
		peg_revision_zval, 
		&end_revision);

	is_url = svn_path_is_url(path);
	REVISION_KIND_CHECK(is_url, start_revision, "start_revision");
	REVISION_KIND_CHECK(is_url, end_revision, "end_revision");
	REVISION_KIND_CHECK(is_url, peg_revision, "peg_revision");

	if (diff_options_zval) {
		_zval_to_apr_array_of_strings(&diff_options, 
			diff_options_zval, 
			0,
			tmp_pool);
	} else {
		diff_options = apr_array_make(tmp_pool, 0, sizeof (const char *));
	}

	tmp_path = (char *) svn_normalized_path(tmp_path, tmp_pool);
	path = (char *) svn_normalized_path(path, tmp_pool);

	svn_error = OPEN_UNIQUE_FILE(out_file, out_file_name, tmp_path, tmp_pool);

	if (svn_error) {
		goto cleanup;
	}

	svn_error = OPEN_UNIQUE_FILE(err_file, err_file_name, tmp_path, tmp_pool);

	if (svn_error) {
		goto cleanup;
	}

	if (!header_encoding) {
		header_encoding = (char *) SVN_APR_LOCALE_CHARSET;
	}

	/* 1.4 API */
	svn_error = svn_client_diff_peg3(diff_options,
		path,
		&peg_revision,
		&start_revision,
		&end_revision,
		recurse,
		ignore_ancestry,
		no_diff_related,
		ignore_content_type,
		header_encoding,
		out_file,
		err_file,
		intern->ctx,
		tmp_pool);

	if (svn_error) {
		goto cleanup;
	}

	apr_status = apr_file_close(out_file);

	if (apr_status != APR_SUCCESS) {
		zend_throw_exception_ex(svn_exception_object_ce,
			apr_status TSRMLS_CC,
			"Failed to close temporary file %s",
			out_file_name);
		goto cleanup;
	}

	apr_status = apr_file_open(&out_file, 
		out_file_name, 
		APR_READ, 
		APR_OS_DEFAULT, 
		tmp_pool);

	if (apr_status != APR_SUCCESS) {
		out_file = NULL;
		zend_throw_exception_ex(svn_exception_object_ce,
			apr_status TSRMLS_CC,
			"Failed to reopen temporary file %s",
			out_file_name);
		goto cleanup;
	}

	svn_error = svn_stringbuf_from_aprfile(&stringbuf,
		out_file,
		tmp_pool);

	if (svn_error) {
		goto cleanup;
	}

	RETVAL_STRINGL(stringbuf->data, stringbuf->len, 1);
cleanup:
	if (out_file) {
		apr_file_close(out_file);
	}
	if (err_file) {
		apr_file_close(err_file);
	}

	apr_pool_destroy(tmp_pool);

	if (svn_error) {
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}
}
/* }}} */

/* {{{ proto public array SvnClient::diffSummarize(string path1 [, mixed revision1, string path2, mixed revision2, bool recurse, bool ignore_ancestry]) */
PHP_METHOD(SvnClient, diffSummarize)
{
	SVNCLIENT_FETCH_THIS_AND_INTERN();
	char *path1 = NULL, *path2 = NULL;
	int path1_len = 0, path2_len = 0;
	zval *revision1_zval = NULL, *revision2_zval = NULL;
	zend_bool recurse = 1, ignore_ancestry = 1;

	apr_pool_t *tmp_pool;
	svn_opt_revision_t revision1, revision2;
	svn_error_t *svn_error = NULL;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"s|zszbb", 
		&path1, &path1_len,
		&revision1_zval,
		&path2, &path2_len,
		&revision2_zval,
		&recurse,
		&ignore_ancestry)) {
		RETURN_FALSE;
	}

	apr_pool_create(&tmp_pool, intern->pool);

	path1 = (char *) svn_normalized_path(path1, tmp_pool);

	if (path2) {
		path2 = (char *) svn_normalized_path(path2, tmp_pool);
	} else {
		path2 = path1;
	}

	_zval_to_svn_opt_revision_t(&revision1, 
		revision1_zval, 
		&svn_opt_revision_base_default);
	_zval_to_svn_opt_revision_t(&revision2, 
		revision2_zval, 
		&svn_opt_revision_working_default);

	array_init(return_value);

	/* 1.4 API */
	svn_error = svn_client_diff_summarize(path1,
		&revision1,
		path2,
		&revision2,
		recurse,
		ignore_ancestry,
		_svn_client_object_diff_summarize_func,
		(void *) return_value,
		intern->ctx,
		tmp_pool);

	apr_pool_destroy(tmp_pool);

	if (svn_error) {
		zval_dtor(return_value);
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}
}
/* }}} */

/* {{{ proto public string SvnClient::diffSummarizePeg(string path [, mixed peg_revision, mixed start_revision, mixed end_revision, bool recurse, bool ignore_ancestry]) */
PHP_METHOD(SvnClient, diffSummarizePeg)
{
	SVNCLIENT_FETCH_THIS_AND_INTERN();
	char *path = NULL;
	int path_len = 0;
	zval *peg_revision_zval = NULL, *start_revision_zval = NULL, *end_revision_zval = NULL;
	zend_bool recurse = 1, ignore_ancestry = 1;

	apr_pool_t *tmp_pool;
	svn_opt_revision_t peg_revision, start_revision, end_revision;
	svn_error_t *svn_error = NULL;
	int is_url;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"s|zzzbb", 
		&path, &path_len,
		&peg_revision_zval,
		&start_revision_zval,
		&end_revision_zval,
		&recurse,
		&ignore_ancestry)) {
		RETURN_FALSE;
	}

	apr_pool_create(&tmp_pool, intern->pool);

	path = (char *) svn_normalized_path(path, tmp_pool);

	_zval_to_svn_opt_revision_t(&start_revision, 
		start_revision_zval, 
		&svn_opt_revision_base_default);
	_zval_to_svn_opt_revision_t(&end_revision, 
		end_revision_zval, 
		&svn_opt_revision_working_default);
	_zval_to_svn_opt_revision_t(&peg_revision, 
		peg_revision_zval, 
		&end_revision);

	is_url = svn_path_is_url(path);
	REVISION_KIND_CHECK(is_url, start_revision, "start_revision");
	REVISION_KIND_CHECK(is_url, end_revision, "end_revision");
	REVISION_KIND_CHECK(is_url, peg_revision, "peg_revision");

	array_init(return_value);

	/* 1.4 API */
	svn_error = svn_client_diff_summarize_peg(path,
		&peg_revision,
		&start_revision,
		&end_revision,
		recurse,
		ignore_ancestry,
		_svn_client_object_diff_summarize_func,
		(void *) return_value,
		intern->ctx,
		tmp_pool);

	apr_pool_destroy(tmp_pool);

	if (svn_error) {
		zval_dtor(return_value);
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}
}
/* }}} */

/* {{{ proto public bool SvnClient::merge(string source1, mixed revision1, string source2, mixed revision2, string target_wcpath [, bool recurse, bool ignore_ancestry, bool force, bool dry_run]) */ 
PHP_METHOD(SvnClient, merge)
{
	SVNCLIENT_FETCH_THIS_AND_INTERN();
	char *source1 = NULL, *source2 = NULL, *target_wcpath = NULL;
	int source1_len = 0, source2_len = 0, target_wcpath_len = 0;
	zval *revision1_zval = NULL, *revision2_zval = NULL;
	zend_bool recurse = 1, ignore_ancestry = 1, force = 0, dry_run = 0;

	apr_pool_t *tmp_pool;
	apr_array_header_t *merge_options;
	svn_opt_revision_t revision1, revision2;
	svn_error_t *svn_error = NULL;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"szszs|bbbb", 
		&source1, &source1_len,
		&revision1_zval,
		&source2, &source2_len,
		&revision2_zval,
		&target_wcpath,
		&recurse,
		&ignore_ancestry,
		&force,
		&dry_run)) {
		RETURN_FALSE;
	}

	apr_pool_create(&tmp_pool, intern->pool);

	source1 = (char *) svn_normalized_path(source1, tmp_pool);
	source2 = (char *) svn_normalized_path(source2, tmp_pool);
	target_wcpath = (char *) svn_normalized_path(target_wcpath, tmp_pool);

	_zval_to_svn_opt_revision_t(&revision1,
		revision1_zval,
		&svn_opt_revision_head_default);
	_zval_to_svn_opt_revision_t(&revision2,
		revision2_zval,
		&svn_opt_revision_head_default);

	merge_options = apr_array_make(tmp_pool, 0, sizeof(const char *));

	/* 1.4 API */
	svn_error = svn_client_merge2(source1,
		&revision1,
		source2,
		&revision2,
		target_wcpath,
		recurse,
		ignore_ancestry,
		force,
		dry_run,
		merge_options,
		intern->ctx,
		tmp_pool);

	apr_pool_destroy(tmp_pool);

	if (svn_error) {
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto public bool SvnClient::mergePeg(string source, mixed revision1, mixed revision2, string target_wcpath [, bool recurse, bool ignore_ancestry, bool force, bool dry_run]) */ 
PHP_METHOD(SvnClient, mergePeg)
{
	SVNCLIENT_FETCH_THIS_AND_INTERN();
	char *source = NULL, *target_wcpath = NULL;
	int source_len = 0, target_wcpath_len = 0;
	zval *revision1_zval = NULL, *revision2_zval = NULL, *peg_revision_zval = NULL;
	zend_bool recurse = 1, ignore_ancestry = 1, force = 0, dry_run = 0;

	apr_pool_t *tmp_pool;
	apr_array_header_t *merge_options;
	svn_opt_revision_t revision1, revision2, peg_revision;
	svn_error_t *svn_error = NULL;
	int is_url;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"szzz|sbbbb", 
		&source, &source_len,
		&revision1_zval,
		&revision2_zval,
		&peg_revision_zval,
		&target_wcpath,
		&recurse,
		&ignore_ancestry,
		&force,
		&dry_run)) {
		RETURN_FALSE;
	}

	apr_pool_create(&tmp_pool, intern->pool);

	source = (char *) svn_normalized_path(source, tmp_pool);
	target_wcpath = (char *) svn_normalized_path(target_wcpath, tmp_pool);

	_zval_to_svn_opt_revision_t(&revision1,
		revision1_zval,
		&svn_opt_revision_head_default);
	_zval_to_svn_opt_revision_t(&revision2,
		revision2_zval,
		&svn_opt_revision_head_default);
	_zval_to_svn_opt_revision_t(&peg_revision,
		peg_revision_zval,
		&revision2);

	is_url = svn_path_is_url(source);
	REVISION_KIND_CHECK(is_url, revision1, "revision1");
	REVISION_KIND_CHECK(is_url, revision2, "revision2");
	REVISION_KIND_CHECK(is_url, peg_revision, "peg_revision");

	merge_options = apr_array_make(tmp_pool, 0, sizeof(const char *));

	/* 1.4 API */
	svn_error = svn_client_merge_peg2(source,
		&revision1,
		&revision2,
		&peg_revision,
		target_wcpath,
		recurse,
		ignore_ancestry,
		force,
		dry_run,
		merge_options,
		intern->ctx,
		tmp_pool);

	apr_pool_destroy(tmp_pool);

	if (svn_error) {
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto public bool SvnClient::cleanup(string dir) */ 
PHP_METHOD(SvnClient, cleanup)
{
	SVNCLIENT_FETCH_THIS_AND_INTERN();
	char *dir = NULL;
	int dir_len = 0;

	apr_pool_t *tmp_pool;
	svn_error_t *svn_error = NULL;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"s", 
		&dir, &dir_len)) {
		RETURN_FALSE;
	}

	apr_pool_create(&tmp_pool, intern->pool);

	dir = (char *) svn_normalized_path(dir, tmp_pool);

	/* 1.4 API */
	svn_error = svn_client_cleanup(dir, 
		intern->ctx, 
		tmp_pool);

	apr_pool_destroy(tmp_pool);

	if (svn_error) {
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto public bool SvnClient::relocate(string dir, string from, string to [, bool recurse]) */ 
PHP_METHOD(SvnClient, relocate)
{
	SVNCLIENT_FETCH_THIS_AND_INTERN();
	char *dir = NULL, *from = NULL, *to = NULL;
	int dir_len = 0, from_len = 0, to_len = 0;
	zend_bool recurse = 1;

	apr_pool_t *tmp_pool;
	svn_error_t *svn_error = NULL;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"sss|b", 
		&dir, &dir_len,
		&from, &from_len,
		&to, &to_len,
		&recurse)) {
		RETURN_FALSE;
	}

	apr_pool_create(&tmp_pool, intern->pool);

	dir = (char *) svn_normalized_path(dir, tmp_pool);

	/* 1.4 API */
	svn_error = svn_client_relocate(dir, 
		from,
		to,
		recurse,
		intern->ctx, 
		tmp_pool);

	apr_pool_destroy(tmp_pool);

	if (svn_error) {
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto public bool SvnClient::revert(mixed paths [, bool recurse]) */ 
PHP_METHOD(SvnClient, revert)
{
	SVNCLIENT_FETCH_THIS_AND_INTERN();
	zval *paths_zval = NULL;
	zend_bool recurse = 0; /* yes, 0 is correct for recurse. */

	apr_pool_t *tmp_pool;
	apr_array_header_t *paths;
	svn_error_t *svn_error = NULL;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"z|b", 
		&paths_zval,
		&recurse)) {
		RETURN_FALSE;
	}

	apr_pool_create(&tmp_pool, intern->pool);

	_zval_to_apr_array_of_strings(&paths, 
		paths_zval, 
		1, 
		tmp_pool);

	/* 1.4 API */
	svn_error = svn_client_revert(paths, 
		recurse,
		intern->ctx, 
		tmp_pool);

	apr_pool_destroy(tmp_pool);

	if (svn_error) {
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto public bool SvnClient::resolved(string path [, bool recursive]) */ 
PHP_METHOD(SvnClient, resolved)
{
	SVNCLIENT_FETCH_THIS_AND_INTERN();
	char *path = NULL;
	int path_len = 0;
	zend_bool recursive = 1;

	apr_pool_t *tmp_pool;
	svn_error_t *svn_error = NULL;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"s|b", 
		&path, &path_len,
		&recursive)) {
		RETURN_FALSE;
	}

	apr_pool_create(&tmp_pool, intern->pool);

	path = (char *) svn_normalized_path(path, tmp_pool);

	/* 1.4 API */
	svn_error = svn_client_resolved(path, 
		recursive,
		intern->ctx, 
		tmp_pool);

	apr_pool_destroy(tmp_pool);

	if (svn_error) {
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto public mixed SvnClient::copy(string src_path, string dst_path [, mixed src_revision]) */ 
PHP_METHOD(SvnClient, copy)
{
	SVNCLIENT_FETCH_THIS_AND_INTERN();
	char *src_path = NULL, *dst_path = NULL;
	int src_path_len = 0, dst_path_len = 0;
	zval *src_revision_zval = NULL;

	apr_pool_t *tmp_pool;
	svn_opt_revision_t src_revision;
	svn_commit_info_t *commit_info = NULL;
	svn_error_t *svn_error = NULL;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"ss|z", 
		&src_path, &src_path_len,
		&dst_path, &dst_path_len,
		&src_revision)) {
		RETURN_FALSE;
	}

	apr_pool_create(&tmp_pool, intern->pool);

	src_path = (char *) svn_normalized_path(src_path, tmp_pool);
	dst_path = (char *) svn_normalized_path(dst_path, tmp_pool);

	if (svn_path_is_url(src_path)) {
		_zval_to_svn_opt_revision_t(&src_revision,
			src_revision_zval,
			&svn_opt_revision_head_default);
	} else {
		_zval_to_svn_opt_revision_t(&src_revision,
			src_revision_zval,
			&svn_opt_revision_working_default);
	}

	/* 1.4 API */
	svn_error = svn_client_copy3(&commit_info,
		src_path,
		&src_revision,
		dst_path,
		intern->ctx,
		tmp_pool);

	if (svn_error) {
		apr_pool_destroy(tmp_pool);
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}

	_svn_commit_info_t_to_zval(&return_value, commit_info);

	apr_pool_destroy(tmp_pool);
}
/* }}} */

/* {{{ proto public mixed SvnClient::move(string src_path, string dst_path [, bool force]) */ 
PHP_METHOD(SvnClient, move)
{
	SVNCLIENT_FETCH_THIS_AND_INTERN();
	char *src_path = NULL, *dst_path = NULL;
	int src_path_len = 0, dst_path_len = 0;
	zend_bool force = 0;

	apr_pool_t *tmp_pool;
	svn_commit_info_t *commit_info = NULL;
	svn_error_t *svn_error = NULL;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"ss|b", 
		&src_path, &src_path_len,
		&dst_path, &dst_path_len,
		&force)) {
		RETURN_FALSE;
	}

	apr_pool_create(&tmp_pool, intern->pool);

	src_path = (char *) svn_normalized_path(src_path, tmp_pool);
	dst_path = (char *) svn_normalized_path(dst_path, tmp_pool);

	/* 1.4 API */
	svn_error = svn_client_move4(&commit_info,
		src_path,
		dst_path,
		force,
		intern->ctx,
		tmp_pool);

	if (svn_error) {
		apr_pool_destroy(tmp_pool);
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}

	_svn_commit_info_t_to_zval(&return_value, commit_info);

	apr_pool_destroy(tmp_pool);
}
/* }}} */

/* {{{ proto public bool SvnClient::propSet(string propname, string propval, string target [, bool recurse, bool skip_checks]) */
PHP_METHOD(SvnClient, propSet)
{
	SVNCLIENT_FETCH_THIS_AND_INTERN();
	char *propname = NULL, *propval = NULL, *target = NULL;
	int propname_len = 0, propval_len = 0, target_len = 0;
	zend_bool recurse = 0, skip_checks = 0;

	apr_pool_t *tmp_pool;
	svn_string_t *propval_svn_str;
	svn_error_t *svn_error = NULL;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"sss|bb", 
		&propname, &propname_len,
		&propval, &propval_len,
		&target, &target_len,
		&recurse,
		&skip_checks)) {
		RETURN_FALSE;
	}

	apr_pool_create(&tmp_pool, intern->pool);

	propval_svn_str = svn_string_ncreate(propval, propval_len, tmp_pool);
	target = (char *) svn_normalized_path(target, tmp_pool);

	/* 1.4 API */
	svn_error = svn_client_propset2(propname,
		propval_svn_str,
		target,
		recurse,
		skip_checks,
		intern->ctx,
		tmp_pool);

	apr_pool_destroy(tmp_pool);

	if (svn_error) {
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto public string SvnClient::propGet(string propname, string target [, mixed peg_revision, mixed revision, bool recurse]) */
PHP_METHOD(SvnClient, propGet)
{
	SVNCLIENT_FETCH_THIS_AND_INTERN();
	char *propname = NULL, *target = NULL;
	int propname_len = 0, target_len = 0;
	zval *peg_revision_zval = NULL, *revision_zval = NULL;
	zend_bool recurse = 0;

	apr_pool_t *tmp_pool;
	apr_hash_t *props = NULL;
	svn_opt_revision_t peg_revision, revision;
	svn_error_t *svn_error = NULL;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"ss|zzb", 
		&propname, &propname_len,
		&target, &target_len,
		&peg_revision_zval,
		&revision_zval,
		&recurse)) {
		RETURN_FALSE;
	}

	apr_pool_create(&tmp_pool, intern->pool);

	target = (char *) svn_normalized_path(target, tmp_pool);

	if (svn_path_is_url(target)) {
		_zval_to_svn_opt_revision_t(&revision,
			revision_zval,
			&svn_opt_revision_head_default);
	} else {
		_zval_to_svn_opt_revision_t(&revision,
			revision_zval,
			&svn_opt_revision_working_default);
	}

	_zval_to_svn_opt_revision_t(&peg_revision,
		peg_revision_zval,
		&revision);

	/* 1.4 API */
	svn_error = svn_client_propget2(&props,
		propname,
		target,
		&peg_revision,
		&revision,
		recurse,
		intern->ctx,
		tmp_pool);

	if (svn_error) {
		apr_pool_destroy(tmp_pool);
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}

	_apr_prop_hash_to_zval(&return_value, props, tmp_pool);
	apr_pool_destroy(tmp_pool);
}
/* }}} */

/* {{{ proto public bool SvnClient::propList(string target [, mixed peg_revision, mixed revision, bool recurse]) */
PHP_METHOD(SvnClient, propList)
{
	SVNCLIENT_FETCH_THIS_AND_INTERN();
	char *target = NULL;
	int target_len = 0;
	zval *peg_revision_zval = NULL, *revision_zval = NULL;
	zend_bool recurse = 0;

	apr_pool_t *tmp_pool;
	apr_array_header_t *props;
	svn_opt_revision_t peg_revision, revision;
	int prop_index;
	svn_error_t *svn_error = NULL;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"s|zzb", 
		&target, &target_len,
		&peg_revision_zval,
		&revision_zval,
		&recurse)) {
		RETURN_FALSE;
	}

	apr_pool_create(&tmp_pool, intern->pool);

	target = (char *) svn_normalized_path(target, tmp_pool);

	if (svn_path_is_url(target)) {
		_zval_to_svn_opt_revision_t(&revision,
			revision_zval,
			&svn_opt_revision_head_default);
	} else {
		_zval_to_svn_opt_revision_t(&revision,
			revision_zval,
			&svn_opt_revision_working_default);
	}

	_zval_to_svn_opt_revision_t(&peg_revision,
		peg_revision_zval,
		&revision);

	/* 1.4 API */
	svn_error = svn_client_proplist2(&props,
		target,
		&peg_revision,
		&revision,
		recurse,
		intern->ctx,
		tmp_pool);

	if (svn_error) {
		apr_pool_destroy(tmp_pool);
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}

	array_init(return_value);

	for (prop_index = 0; prop_index < props->nelts; prop_index++) {
		svn_client_proplist_item_t *item = 
			APR_ARRAY_IDX(props, prop_index, svn_client_proplist_item_t *);
		zval *elem_arr;
		char *node_name = item->node_name->data;

		ALLOC_INIT_ZVAL(elem_arr);
		_apr_prop_hash_to_zval(&elem_arr, item->prop_hash, tmp_pool);

		node_name = (char *) svn_path_local_style(node_name, tmp_pool);

		add_assoc_zval(return_value, node_name, elem_arr);
	}

	apr_pool_destroy(tmp_pool);
}
/* }}} */

/* {{{ proto int public SvnClient::export(string from, string to [, mixed peg_revision, mixed revision, bool overwrite, bool ignore_externals, bool recurse, string native_eol]) */
PHP_METHOD(SvnClient, export)
{
	SVNCLIENT_FETCH_THIS_AND_INTERN();
	char *from = NULL, *to = NULL, *native_eol = NULL;
	int from_len = 0, to_len = 0, native_eol_len = 0;
	zval *peg_revision_zval = NULL, *revision_zval = NULL;
	zend_bool overwrite = 0, ignore_externals = 0, recurse = 0;

	apr_pool_t *tmp_pool;
	svn_opt_revision_t peg_revision, revision;
	svn_revnum_t result_rev;
	svn_error_t *svn_error = NULL;
	int is_url;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"ss|zzbbbs", 
		&from, &from_len,
		&to, &to_len,
		&peg_revision_zval,
		&revision_zval,
		&overwrite,
		&ignore_externals,
		&recurse,
		&native_eol, &native_eol_len)) {
		RETURN_FALSE;
	}

	apr_pool_create(&tmp_pool, intern->pool);

	from = (char *) svn_normalized_path(from, tmp_pool);
	to = (char *) svn_normalized_path(to, tmp_pool);

	is_url = svn_path_is_url(from);

	if (is_url) {
		_zval_to_svn_opt_revision_t(&revision,
			revision_zval,
			&svn_opt_revision_head_default);
	} else {
		_zval_to_svn_opt_revision_t(&revision,
			revision_zval,
			&svn_opt_revision_working_default);
	}

	REVISION_KIND_CHECK(is_url, revision, "revision");

	if (native_eol) {
		if (strcmp(native_eol, "CR") != 0 &&
			strcmp(native_eol, "CRLF") != 0 &&
			strcmp(native_eol, "LF") != 0) {
			apr_pool_destroy(tmp_pool);
			zend_throw_exception_ex(svn_invalid_argument_exception_object_ce,
				0 TSRMLS_CC,
				"native_eol must equal \"CR\", \"CRLF\", or \"LF\"");
			RETURN_FALSE;
		}
	}

	_zval_to_svn_opt_revision_t(&peg_revision,
		peg_revision_zval,
		&revision);

	svn_error = svn_client_export3(&result_rev,
		from,
		to,
		&peg_revision,
		&revision,
		overwrite,
		ignore_externals,
		recurse,
		native_eol,
		intern->ctx,
		tmp_pool);

	apr_pool_destroy(tmp_pool);

	if (svn_error) {
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}

	RETURN_LONG(SVN_REVNUM_T_TO_LONG(result_rev));
}
/* }}} */

/* {{{ proto public array SvnClient::list(string path_or_url [, mixed peg_revision, mixed revision, bool recurse, bool fetch_locks])
    List the contents of a directory or url. */
PHP_METHOD(SvnClient, list)
{
	SVNCLIENT_FETCH_THIS_AND_INTERN();
	char *path_or_url = NULL;
	int path_or_url_len = 0;
	zval *peg_revision_zval = NULL, *revision_zval = NULL;
	zend_bool recurse = 0, fetch_locks = 0;

	svn_error_t *svn_error;
	svn_opt_revision_t revision, peg_revision;
	apr_pool_t *tmp_pool;
	int is_url;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"s|zzbb", 
		&path_or_url, &path_or_url_len,
		&peg_revision_zval,
		&revision_zval,
		&recurse,
		&fetch_locks)) {
		RETURN_FALSE;
	}

	apr_pool_create(&tmp_pool, intern->pool);
	path_or_url = (char *) svn_normalized_path(path_or_url, tmp_pool);

	is_url = svn_path_is_url(path_or_url);

	if (is_url) {
		_zval_to_svn_opt_revision_t(&revision, 
			revision_zval, 
			&svn_opt_revision_head_default);
	} else {
		_zval_to_svn_opt_revision_t(&revision, 
			revision_zval, 
			&svn_opt_revision_working_default);
	}

	_zval_to_svn_opt_revision_t(&peg_revision, 
		peg_revision_zval, 
		&svn_opt_revision_unspecified_default);

	REVISION_KIND_CHECK(is_url, revision, "revision");
	REVISION_KIND_CHECK(is_url, peg_revision, "peg_revision");

	array_init(return_value);

	/* 1.4 API */
	svn_error = svn_client_list(path_or_url,
		&peg_revision,
		&revision,
		recurse,
		SVN_DIRENT_ALL,
		fetch_locks,
		_svn_client_object_list_func,
		(void *) return_value,
		intern->ctx,
		tmp_pool);

	apr_pool_destroy(tmp_pool);

	if (svn_error) {
		zval_dtor(return_value);
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}
}
/* }}} */

/* {{{ proto public string SvnClient::cat(string path_or_url [, mixed peg_revision, mixed revision]) */
PHP_METHOD(SvnClient, cat)
{
	SVNCLIENT_FETCH_THIS_AND_INTERN();
	char *path_or_url = NULL;
	int path_or_url_len = 0;
	zval *peg_revision_zval = NULL, *revision_zval = NULL;

	svn_error_t *svn_error;
	svn_opt_revision_t revision, peg_revision;
	svn_stringbuf_t *stringbuf;
	svn_stream_t *stream;
	apr_pool_t *tmp_pool;
	int is_url;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"s|zz", 
		&path_or_url, &path_or_url_len,
		&peg_revision_zval,
		&revision_zval)) {
		RETURN_FALSE;
	}

	apr_pool_create(&tmp_pool, intern->pool);
	path_or_url = (char *) svn_normalized_path(path_or_url, tmp_pool);

	_zval_to_svn_opt_revision_t(&revision, 
		revision_zval,
		&svn_opt_revision_head_default);
	_zval_to_svn_opt_revision_t(&peg_revision, 
		peg_revision_zval,
		&revision);

	is_url = svn_path_is_url(path_or_url);
	REVISION_KIND_CHECK(is_url, revision, "revision");
	REVISION_KIND_CHECK(is_url, peg_revision, "peg_revision");

	stringbuf = svn_stringbuf_create("", tmp_pool);
	stream = svn_stream_from_stringbuf(stringbuf, tmp_pool);

	/* 1.4 API */
	svn_error = svn_client_cat2(stream,
		path_or_url,
		&peg_revision,
		&revision,
		intern->ctx,
		tmp_pool);

	if (svn_error) {
		apr_pool_destroy(tmp_pool);
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}

	RETVAL_STRINGL(stringbuf->data, stringbuf->len, 1);

	apr_pool_destroy(tmp_pool);
}
/* }}} */

/* {{{ proto public bool SvnClient::lock(mixed targets, string comment [, bool steal_lock]) */
PHP_METHOD(SvnClient, lock)
{
	SVNCLIENT_FETCH_THIS_AND_INTERN();
	zval *targets_zval = NULL;
	char *comment = NULL;
	int comment_len = 0;
	zend_bool steal_lock = 0;

	svn_error_t *svn_error;
	apr_array_header_t *targets;
	apr_pool_t *tmp_pool;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"zs|b", 
		&targets_zval,
		&comment, &comment_len,
		&steal_lock)) {
		RETURN_FALSE;
	}

	apr_pool_create(&tmp_pool, intern->pool);

	_zval_to_apr_array_of_strings(&targets,
		targets_zval,
		1,
		tmp_pool);

	svn_error = svn_client_lock(targets,
		comment,
		steal_lock,
		intern->ctx,
		tmp_pool);
	
	apr_pool_destroy(tmp_pool);

	if (svn_error) {
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto public bool SvnClient::unlock(mixed targets, [, bool break_lock]) */
PHP_METHOD(SvnClient, unlock)
{
	SVNCLIENT_FETCH_THIS_AND_INTERN();
	zval *targets_zval = NULL;
	zend_bool break_lock = 0;

	svn_error_t *svn_error;
	apr_array_header_t *targets;
	apr_pool_t *tmp_pool;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"z|b", 
		&targets_zval,
		&break_lock)) {
		RETURN_FALSE;
	}

	apr_pool_create(&tmp_pool, intern->pool);

	_zval_to_apr_array_of_strings(&targets,
		targets_zval,
		1,
		tmp_pool);

	svn_error = svn_client_unlock(targets,
		break_lock,
		intern->ctx,
		tmp_pool);
	
	apr_pool_destroy(tmp_pool);

	if (svn_error) {
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto public string SvnClient::info(string path_or_url [, mixed peg_revision, mixed revision, bool recurse]) */
PHP_METHOD(SvnClient, info)
{
	SVNCLIENT_FETCH_THIS_AND_INTERN();
	char *path_or_url = NULL;
	int path_or_url_len = 0;
	zval *peg_revision_zval = NULL, *revision_zval = NULL;
	zend_bool recurse = 1;

	svn_error_t *svn_error;
	svn_opt_revision_t revision, peg_revision;
	apr_pool_t *tmp_pool;
	int is_url;

	if (SUCCESS != zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
		"s|zzb", 
		&path_or_url, &path_or_url_len,
		&peg_revision_zval,
		&revision_zval,
		&recurse)) {
		RETURN_FALSE;
	}

	apr_pool_create(&tmp_pool, intern->pool);

	path_or_url = (char *) svn_normalized_path(path_or_url, tmp_pool);

	is_url = svn_path_is_url(path_or_url);

	if (is_url) {
		_zval_to_svn_opt_revision_t(&revision,
			revision_zval,
			&svn_opt_revision_head_default);
	} else {
		_zval_to_svn_opt_revision_t(&revision,
			revision_zval,
			&svn_opt_revision_unspecified_default);
	}

	_zval_to_svn_opt_revision_t(&peg_revision,
		peg_revision_zval,
		&revision);

	REVISION_KIND_CHECK(is_url, peg_revision, "peg_revision");
	REVISION_KIND_CHECK(is_url, revision, "revision");

	array_init(return_value);

	svn_error = svn_client_info(path_or_url,
		&peg_revision,
		&revision,
		_svn_client_object_info_receiver_func,
		(void *) return_value,
		recurse,
		intern->ctx,
		tmp_pool);

	apr_pool_destroy(tmp_pool);

	if (svn_error) {
		zval_dtor(return_value);
		THROW_SVN_LIBRARY_EXCEPTION_AND_RETURN(svn_error);
	}
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
