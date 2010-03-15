/**
 * TODO:
 * - Add optional module dependencies on APC, xdebug, xhprof, and whatever else screws
 *   with the compile_file()
 * - Add function to get the filename of the processed file
 *   - Add hashtable mapping original path to temp file
 * - Handle shebang:
 *   - reset CG(start_lineno)
 *   - do cli_seek_file_begin logic again on processed results
 * - Handle script-is-a-directory
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_prep.h"

static int le_prep;

const zend_function_entry prep_functions[] = {
	{NULL, NULL, NULL}	/* Must be the last line in prep_functions[] */
};

zend_module_entry prep_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"prep",
	prep_functions,
	PHP_MINIT(prep),
	PHP_MSHUTDOWN(prep),
	PHP_RINIT(prep),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(prep),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(prep),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1", /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};

ZEND_DECLARE_MODULE_GLOBALS(prep)

#ifdef COMPILE_DL_PREP
ZEND_GET_MODULE(prep)
#endif

zend_op_array *(*prep_orig_compile_file)(zend_file_handle *file_handle, int type TSRMLS_DC);

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
	int exitstatus;

	env_suppress = getenv("PHP_SUPPRESS_PREP");

	if (!(prep_command && prep_command[0]) || env_suppress != NULL ||
		!file_handle || !file_handle->filename) {

		return prep_orig_compile_file(file_handle, type TSRMLS_CC);
	}

	putenv("PHP_SUPPRESS_PREP=1");
	f = *file_handle;
	resolved_path = zend_resolve_path(file_handle->filename, strlen(file_handle->filename) TSRMLS_CC);
	if (resolved_path) {
		FILE *fp;
		char *result = NULL;
		int numbytes = 0;
		long maxlen = PHP_STREAM_COPY_ALL, result_len;
		php_stream *in_stream;

		spprintf(&command, 0, "%s %s", prep_command, resolved_path);
		fp = VCWD_POPEN(command, "r");
		if (!fp) {
			failed = 1;
			goto prep_error;
		}
		in_stream = php_stream_fopen_from_pipe(fp, "r");

		if (in_stream == NULL)	{
			failed = 1;
			goto prep_error;
		}

		result_len = php_stream_copy_to_mem(in_stream, &result, maxlen, 0);
		exitstatus = php_stream_close(in_stream);
		if (!result) {
			failed = 1;
			goto prep_error;
		}

		/* if the pipe exits non-zero, use the original handle */
		if (0 == exitstatus) {
			tmp_stream = php_stream_fopen_tmpfile();
			numbytes = php_stream_write(tmp_stream, result, result_len);
			efree(result);
			if (numbytes != result_len) {
				failed = 1;
				goto prep_error;
			}
			new_file = tmp_stream->orig_path;

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
	}

prep_error:
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

	zend_try {
		failed = 0;
		res = prep_orig_compile_file(file_handle, type TSRMLS_CC);
	} zend_catch {
		failed = 1;
	} zend_end_try();

	if (failed) {
		zend_bailout();
	}

	if (tmp_stream) {
		php_stream_close(tmp_stream);
	}

	unsetenv("PHP_SUPPRESS_PREP");
	return res;
}
/* }}} */

/* {{{ INI entries */
PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("prep.command",    "",     PHP_INI_PERDIR,     OnUpdateString,             prep_command,     zend_prep_globals,  prep_globals)
PHP_INI_END()
/* }}} */


static void prep_init_globals(zend_prep_globals *prep_globals_p TSRMLS_DC)
{
	PREP_G(prep_command) = NULL;
}

PHP_MINIT_FUNCTION(prep)
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

PHP_MSHUTDOWN_FUNCTION(prep)
{
	if (zend_compile_file == prep_compile_file) {
		zend_compile_file = prep_orig_compile_file;
	}

	UNREGISTER_INI_ENTRIES();

	return SUCCESS;
}

PHP_RINIT_FUNCTION(prep)
{
	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(prep)
{
	return SUCCESS;
}

PHP_MINFO_FUNCTION(prep)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "prep support", "enabled");
	php_info_print_table_end();

}
