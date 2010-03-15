#ifndef PHP_PREP_H
#define PHP_PREP_H

#define PHP_PREP_VERSION "0.0.1"

extern zend_module_entry prep_module_entry;
#define phpext_prep_ptr &prep_module_entry

#ifdef PHP_WIN32
#	define PHP_PREP_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_PREP_API __attribute__ ((visibility("default")))
#else
#	define PHP_PREP_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

ZEND_BEGIN_MODULE_GLOBALS(prep)
	char *prep_command;
ZEND_END_MODULE_GLOBALS(prep)

PHP_MINIT_FUNCTION(prep);
PHP_MSHUTDOWN_FUNCTION(prep);
PHP_RINIT_FUNCTION(prep);
PHP_RSHUTDOWN_FUNCTION(prep);
PHP_MINFO_FUNCTION(prep);


#ifdef ZTS
#define PREP_G(v) TSRMG(prep_globals_id, zend_prep_globals *, v)
#else
#define PREP_G(v) (prep_globals.v)
#endif

#endif	/* PHP_PREP_H */

