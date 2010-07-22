#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/php_string.h"
#include "ext/standard/exec.h"
#include "SAPI.h"
#include "php_prep.h"

#define PREP_EXIT_NORMAL     0
#define PREP_EXIT_SKIP       1
#define PREP_EXIT_PHP_FALE 255

/* {{{ Forward declarations */
zend_op_array *(*prep_orig_compile_file)(zend_file_handle *file_handle, int type TSRMLS_DC);
/* }}} */

/* {{{ Module set-up */
ZEND_DECLARE_MODULE_GLOBALS(prep)

ZEND_BEGIN_ARG_INFO_EX(prep_get_file, 0, 0, 0)
    ZEND_ARG_INFO(0, orig_file)
ZEND_END_ARG_INFO()

const zend_function_entry prep_functions[] = {
    PHP_FE(prep_get_file, prep_get_file)
    {NULL, NULL, NULL}  /* Must be the last line in prep_functions[] */
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
static PHP_INI_MH(OnUpdateCommand)
{
    HashTable *commands;
    long index;
#ifndef ZTS
    char *base = (char *) mh_arg2;
#else
    char *base;

    base = (char *) ts_resource(*((int *) mh_arg2));
#endif

    if (!new_value) {
        return FAILURE;
    }

    commands = (HashTable *) (base+(size_t) mh_arg1);
    index = (long)mh_arg3;
    zend_hash_index_update(commands, index, &new_value, sizeof(char *), NULL);

    return SUCCESS;
}

PHP_INI_BEGIN()
    PHP_INI_ENTRY3("prep.command", "", PHP_INI_ALL, OnUpdateCommand, (void *) XtOffsetOf(zend_prep_globals, commands), (void *)&prep_globals, (void *)0)
    PHP_INI_ENTRY3("prep.command2", "", PHP_INI_ALL, OnUpdateCommand, (void *) XtOffsetOf(zend_prep_globals, commands), (void *)&prep_globals, (void *)1)
    PHP_INI_ENTRY3("prep.command3", "", PHP_INI_ALL, OnUpdateCommand, (void *) XtOffsetOf(zend_prep_globals, commands), (void *)&prep_globals, (void *)2)
    PHP_INI_ENTRY3("prep.command4", "", PHP_INI_ALL, OnUpdateCommand, (void *) XtOffsetOf(zend_prep_globals, commands), (void *)&prep_globals, (void *)3)
    PHP_INI_ENTRY3("prep.command5", "", PHP_INI_ALL, OnUpdateCommand, (void *) XtOffsetOf(zend_prep_globals, commands), (void *)&prep_globals, (void *)4)
    PHP_INI_ENTRY3("prep.command6", "", PHP_INI_ALL, OnUpdateCommand, (void *) XtOffsetOf(zend_prep_globals, commands), (void *)&prep_globals, (void *)5)
    PHP_INI_ENTRY3("prep.command7", "", PHP_INI_ALL, OnUpdateCommand, (void *) XtOffsetOf(zend_prep_globals, commands), (void *)&prep_globals, (void *)6)
    PHP_INI_ENTRY3("prep.command8", "", PHP_INI_ALL, OnUpdateCommand, (void *) XtOffsetOf(zend_prep_globals, commands), (void *)&prep_globals, (void *)7)
    PHP_INI_ENTRY3("prep.command9", "", PHP_INI_ALL, OnUpdateCommand, (void *) XtOffsetOf(zend_prep_globals, commands), (void *)&prep_globals, (void *)8)
PHP_INI_END()
/* }}} */

static zend_op_array *prep_compile_file(zend_file_handle *file_handle, int type TSRMLS_DC) /* {{{ */
{
    zend_op_array *res;
    char *resolved_path = NULL;
    zend_file_handle orig_file_handle;
    int failed = 0;
    char *output_file = NULL, *command = NULL;
    php_stream *tmp_stream = NULL;
    char *err_extra = NULL;
    char *env_suppress = NULL;
    char **prep_command = NULL;
    HashTable *commands = &PREP_G(commands);

    env_suppress = getenv("PHP_SUPPRESS_PREP");

    if (zend_hash_num_elements(commands) == 0 || env_suppress != NULL ||
        !file_handle || !file_handle->filename) {

        return prep_orig_compile_file(file_handle, type TSRMLS_CC);
    }

    putenv("PHP_SUPPRESS_PREP=1");
    orig_file_handle = *file_handle;
    resolved_path = zend_resolve_path(file_handle->filename, strlen(file_handle->filename) TSRMLS_CC);
    if (resolved_path) {
        char *input_file;
        FILE *fp;
        char *output = NULL;
        int num_written = 0;
        long maxlen = PHP_STREAM_COPY_ALL, output_len;
        php_stream *in_stream;
        long offset = 0;
        int exit_status = 0;
        int command_len;
        int replace_count = 0;

        input_file = php_escape_shell_arg(resolved_path);
        zend_hash_internal_pointer_reset(commands);
        while (zend_hash_get_current_data(commands, (void **)&prep_command) == SUCCESS) {

            if (!*prep_command || !(*prep_command)[0]) {
                zend_hash_move_forward(commands);
                continue;
            }

            if (command) {
                efree(command);
            }
            command = php_str_to_str_ex(*prep_command, strlen(*prep_command), "%s", sizeof("%s")-1,
                                        input_file, strlen(input_file), &command_len, 0, &replace_count);
            if (replace_count > 0) {
                char *tmp = command;
                command = php_str_to_str_ex(tmp, command_len, "%o", sizeof("%o")-1,
                                            resolved_path, strlen(resolved_path), &command_len, 0, NULL);
                efree(tmp);
            } else {
                efree(command);
                command = NULL;
                spprintf(&command, 0, "%s %s", *prep_command, input_file);
            }

            fp = VCWD_POPEN(command, "r");
            if (!fp) {
                failed = 1;
                break;
            }
            in_stream = php_stream_fopen_from_pipe(fp, "r");

            if (in_stream == NULL)  {
                failed = 1;
                break;
            }

            output_len = php_stream_copy_to_mem(in_stream, &output, maxlen, 0);
            exit_status = php_stream_close(in_stream);

            if (PREP_EXIT_PHP_FALE == exit_status) {
                failed = 1;
                if (output) {
                    err_extra = output;
                }
                break;
            } else if (PREP_EXIT_SKIP == exit_status) {
                if (output) {
                    efree(output);
                }
                break;
            } else if (PREP_EXIT_NORMAL != exit_status) {
                failed = 1;
                break;
            }

            if (!output) {
                /* could not read file (e.g. might be a directory);
                 * skip preprocessing */
                break;
            }


            /* eat up shebang in CLI mode */
            if (!strcmp(sapi_module.name, "cli")) {
                char *ptr = output;

                CG(start_lineno) = 1;
                if (*ptr == '#' && *++ptr == '!') {
                    do {
                        ptr++;
                    } while (*ptr != '\n' && *ptr != '\r' && *ptr != 0);

                    /* handle situations where line is terminated by \r\n */
                    if (*ptr++ == '\r' && *ptr++ != '\n') {
                        ptr--;
                    }
                    offset = ptr-output;
                }
            }

            if (tmp_stream) {
                php_stream_close(tmp_stream);
            }
            tmp_stream = php_stream_fopen_tmpfile();
            num_written = php_stream_write(tmp_stream, output+offset, output_len-offset);
            efree(output);
            if (num_written != output_len-offset) {
                failed = 1;
                break;
            }

            if (output_file) {
                efree(output_file);
            }
            if (input_file) {
                efree(input_file);
            }
            output_file = estrdup(tmp_stream->orig_path);
            input_file = estrdup(output_file);

            zend_hash_move_forward(commands);
        }

        if (failed) {
            unsetenv("PHP_SUPPRESS_PREP");
            if (err_extra) {
                php_error(E_COMPILE_ERROR, "Could not run preprocessor command %s: %s", command, err_extra);
                efree(err_extra);
            } else {
                php_error(E_COMPILE_ERROR, "Could not run preprocessor command %s (exit status %d)", command, exit_status);
            }
        } else {
            if (output_file) {
                char *final_output = estrdup(output_file);
                zend_hash_add(&PREP_G(orig_files), resolved_path, strlen(resolved_path)+1,
                              (void*)&final_output, sizeof(char *), NULL);

                if (SUCCESS == zend_stream_open_function((const char *)output_file, file_handle TSRMLS_CC)) {
                    file_handle->filename = orig_file_handle.filename;
                    if (file_handle->opened_path) {
                        efree(file_handle->opened_path);
                    }
                    file_handle->opened_path = estrdup(resolved_path);
                    file_handle->free_filename = orig_file_handle.free_filename;
                    if (orig_file_handle.opened_path) {
                        efree(orig_file_handle.opened_path);
                    }
                } else {
                    *file_handle = orig_file_handle;
                }
            }
        }

        efree(resolved_path);
        if (input_file) {
            efree(input_file);
        }
        if (output_file) {
            efree(output_file);
        }
        if (tmp_stream) {
            php_stream_close(tmp_stream);
        }
        if (command) {
            efree(command);
        }

    }

    /* XXX register a clean up handler on script shutdown to remove the temporary file */

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
    zend_hash_init(&PREP_G(commands), 1, NULL, NULL, 1);
}
/* }}} */

static void prep_destroy_globals(zend_prep_globals *prep_globals_p TSRMLS_DC) /* {{{ */
{
    zend_hash_destroy(&PREP_G(commands));
}
/* }}} */

PHP_MINIT_FUNCTION(prep) /* {{{ */
{
#ifdef ZTS
    ts_allocate_id(&prep_globals_id, sizeof(zend_prep_globals), (ts_allocate_ctor) prep_init_globals, (ts_allocate_dtor) prep_destroy_globals);
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

#ifndef ZTS
    prep_destroy_globals(&prep_globals TSRMLS_CC);
#endif

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
    php_info_print_table_row(2, "Version", PHP_PREP_VERSION);
    php_info_print_table_end();
    DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ PHP_FUNCTION(prep_get_file) */
static int add_temp_file(char **ptf TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
{
    zval *map = va_arg(args, zval *);

    if ((hash_key->nKeyLength==0 || hash_key->arKey[0]!=0)) {
        add_assoc_string_ex(map, hash_key->arKey, hash_key->nKeyLength+1, *ptf, 1);
    }
    return ZEND_HASH_APPLY_KEEP;
}

PHP_FUNCTION(prep_get_file)
{
    char *orig_file = NULL;
    int orig_file_len;
    char **temp;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s", &orig_file, &orig_file_len) == FAILURE) {
        RETURN_FALSE;
    }

    if (orig_file) {
        if (zend_hash_find(&PREP_G(orig_files), orig_file, orig_file_len+1, (void**)&temp) == FAILURE) {
            RETURN_FALSE;
        }
        RETURN_STRING(*temp, 1);
    } else {
        array_init(return_value);
        zend_hash_apply_with_arguments(&PREP_G(orig_files) TSRMLS_CC, (apply_func_args_t) add_temp_file, 1, return_value);
    }
}
/* }}} */

/* vim: set fdm=marker sw=4 ts=4 et: */
