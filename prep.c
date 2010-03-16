/**
 * TODO:
 * - Add function to get the filename of the processed file
 *   - Add hashtable mapping original path to temp file
 * - Check exit status of preprocessor command:
 *   - If 255, capture error and raise as E_COMPILE_ERROR
 *   - If 1, use the original file
 * - Check if the file is actually a directory
 * - Change prep.command to fill in %s with the filename
 *   - If %s is not in the string, add filename at the end
 *   - Check return of spprintf() to see if it succeeded
 * 
 * FIXME:
 * - require script (absolute path)?
 * - Add tmpfile handle to PREP_G that gets cleared out on RSHUTDOWN
 *   - (to keep the file from auto-deleting, so scripts can introspect/debug)
 *
 * DONE:
 * - Add optional module dependencies on APC, xdebug, xhprof, and whatever else screws
 *   with the compile_file()
 * - Handle shebang (only in CLI):
 *   - reset CG(start_lineno)
 *   - do cli_seek_file_begin logic again on processed results
 * - allow multiple prep.command INI registrations
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "SAPI.h"
#include "php_prep.h"

#define PREP_EXIT_SKIP 0x1

/* {{{ Forward declarations */
zend_op_array *(*prep_orig_compile_file)(zend_file_handle *file_handle, int type TSRMLS_DC);
/* }}} */

/* {{{ Module set-up */
ZEND_DECLARE_MODULE_GLOBALS(prep)

ZEND_BEGIN_ARG_INFO(prep_get_file, 0)
	ZEND_ARG_INFO(0, from_file)
ZEND_END_ARG_INFO()

const zend_function_entry prep_functions[] = {
	PHP_FE(prep_get_file, prep_get_file)
	{NULL, NULL, NULL}	/* Must be the last line in prep_functions[] */
};

static const zend_module_dep prep_module_deps[] = {
    ZEND_MOD_OPTIONAL("apc")
    ZEND_MOD_OPTIONAL("xdebug")
    ZEND_MOD_OPTIONAL("xhprof")
    {NULL, NULL, NULL}
};

zend_module_entry prep_module_entry = {
    STANDARD_MODULE_HEADER_EX, NULL,
	prep_module_deps,
	"prep",
	prep_functions,
	PHP_MINIT(prep),
	PHP_MSHUTDOWN(prep),
	PHP_RINIT(prep),
	PHP_RSHUTDOWN(prep),
	PHP_MINFO(prep),
	PHP_PREP_VERSION,
    PHP_MODULE_GLOBALS(prep),
	NULL,
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};

#ifdef COMPILE_DL_PREP
ZEND_GET_MODULE(prep)
#endif
/* }}} */

/* {{{ INI entries */
PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("prep.command",    "",     PHP_INI_PERDIR,     OnUpdateString,             prep_command,     zend_prep_globals,  prep_globals)
PHP_INI_END()
/* }}} */

static zend_op_array *prep_compile_file(zend_file_handle *file_handle, int type TSRMLS_DC) /* {{{ */
{
	zend_op_array *res;
	char *resolved_path = NULL;
	zend_file_handle f;
	int failed = 0;
	char *new_file = NULL, *command = NULL;
	php_stream *tmp_stream = NULL;
	char *env_suppress = NULL;
	char *prep_command = PREP_G(prep_command);

	env_suppress = getenv("PHP_SUPPRESS_PREP");

	if (!(prep_command && prep_command[0]) ||
		env_suppress != NULL ||
		!file_handle || !file_handle->filename) {

		return prep_orig_compile_file(file_handle, type TSRMLS_CC);
	}

	putenv("PHP_SUPPRESS_PREP=1");
	f = *file_handle;
	resolved_path = zend_resolve_path(file_handle->filename, strlen(file_handle->filename) TSRMLS_CC);
	if (resolved_path) {
		FILE *fp;
		char *result = NULL;
		int num_written = 0;
		long maxlen = PHP_STREAM_COPY_ALL, result_len;
		php_stream *in_stream;
		long offset = 0;
		int exit_status = 0;

		spprintf(&command, 0, "%s %s", prep_command, resolved_path);
		fp = VCWD_POPEN(command, "r");
		if (!fp) {
			failed = 1;
			goto skip_prep;
		}
		in_stream = php_stream_fopen_from_pipe(fp, "r");

		if (in_stream == NULL)	{
			failed = 1;
			goto skip_prep;
		}

		result_len = php_stream_copy_to_mem(in_stream, &result, maxlen, 0);
		exit_status = php_stream_close(in_stream);

		if (exit_status == PREP_EXIT_SKIP) {
			goto skip_prep;
		}
		if (!result) {
			/* could not read file (e.g. might be a directory);
			 * skip preprocessing */
			goto prep_compile;
		}

		/* eat up shebang in CLI mode */
		if (!strcmp(sapi_module.name, "cli")) {
			char *ptr = result;

			CG(start_lineno) = 1;
			if (*ptr == '#' && *++ptr == '!') {
				do {
					ptr++;
				} while (*ptr != '\n' && *ptr != '\r' && *ptr != 0);

				/* handle situations where line is terminated by \r\n */
				if (*ptr++ == '\r' && *ptr++ != '\n') {
					ptr--;
				}
				offset = ptr-result;
			}
		}

		tmp_stream = php_stream_fopen_tmpfile();
		num_written = php_stream_write(tmp_stream, result+offset, result_len-offset);
		efree(result);
		if (num_written != result_len-offset) {
			failed = 1;
			goto skip_prep;
		}
		new_file = estrdup(tmp_stream->orig_path);

		/* add entry to hashtable */
		if (zend_hash_add(&PREP_G(orig_files), resolved_path, strlen(resolved_path),
						  (void*)&new_file, sizeof(char *), NULL) == FAILURE) {
			failed = 1;
			goto skip_prep;
		}

		if (SUCCESS == zend_stream_open_function((const char *)new_file, file_handle TSRMLS_CC)) {
			file_handle->filename = f.filename;
			if (file_handle->opened_path) {
				efree(file_handle->opened_path);
			}
			file_handle->opened_path = f.opened_path;
			file_handle->free_filename = f.free_filename;
		} else {
			*file_handle = f;
		}
	}

skip_prep:
	if (tmp_stream) {
		php_stream_close(tmp_stream);
	}

	if (failed) {
		unsetenv("PHP_SUPPRESS_PREP");
		php_error_docref(NULL TSRMLS_CC, E_COMPILE_ERROR, "Could not run preprocessor %s on %s", prep_command, resolved_path);
	}

	if (resolved_path) {
		efree(resolved_path);
	}
	if (command) {
		efree(command);
	}

prep_compile:
	zend_try {
		failed = 0;
		res = prep_orig_compile_file(file_handle, type TSRMLS_CC);
	} zend_catch {
		failed = 1;
	} zend_end_try();

	if (failed) {
		zend_bailout();
	}

	unsetenv("PHP_SUPPRESS_PREP");
	return res;
}
/* }}} */

static void prep_init_globals(zend_prep_globals *prep_globals_p TSRMLS_DC) /* {{{ */
{
	PREP_G(prep_command) = NULL;
}
/* }}} */

PHP_MINIT_FUNCTION(prep) /* {{{ */
{
#ifdef ZTS
	ts_allocate_id(&prep_globals_id, sizeof(zend_prep_globals), (ts_allocate_ctor) prep_init_globals, NULL);
#else
	prep_init_globals(&prep_globals TSRMLS_CC);
#endif

	REGISTER_INI_ENTRIES();

	prep_orig_compile_file = zend_compile_file;
	zend_compile_file = prep_compile_file;

	return SUCCESS;
}
/* }}} */

PHP_MSHUTDOWN_FUNCTION(prep) /* {{{ */
{
	if (zend_compile_file == prep_compile_file) {
		zend_compile_file = prep_orig_compile_file;
	}

	UNREGISTER_INI_ENTRIES();

	return SUCCESS;
}
/* }}} */

PHP_RINIT_FUNCTION(prep) /* {{{ */
{
	zend_hash_init(&PREP_G(orig_files), 64, NULL, (dtor_func_t)free_estring, 0);
	return SUCCESS;
}
/* }}} */

PHP_RSHUTDOWN_FUNCTION(prep) /* {{{ */
{
	zend_hash_destroy(&PREP_G(orig_files));
	return SUCCESS;
}
/* }}} */

PHP_MINFO_FUNCTION(prep) /* {{{ */
{
	php_info_print_table_start();
	php_info_print_table_header(2, "prep support", "enabled");
	php_info_print_table_end();

}
/* }}} */

PHP_FUNCTION(prep_get_file)
{
	char *from_file, *pData;
	int from_file_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &from_file, &from_file_len) == FAILURE) {
		RETURN_FALSE;
	}

	if (!zend_hash_exists(&PREP_G(orig_files), from_file, from_file_len)) {
		RETURN_FALSE;
	}

	if (zend_hash_find(&PREP_G(orig_files), from_file, from_file_len, (void**)&pData) == FAILURE) {
		RETURN_FALSE;
	}

	RETURN_STRING(pData, strlen(pData));
}

/* vim: set fdm=marker: */
