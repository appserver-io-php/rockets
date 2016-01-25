/**
 * php_rockets.h
 *
 * NOTICE OF LICENSE
 *
 * This source file is subject to version 3.01 of the PHP license,
 * that is bundled with this package in the file LICENSE, and is
 * available through the world-wide-web at the following url:
 * http://www.php.net/license/3_01.txt
 *
 * If you did not receive a copy of the PHP license and are unable to
 * obtain it through the world-wide-web, please send a note to
 * license@php.net so we can mail you a copy immediately.
 */

/**
 * A PHP extension to get real system socket functionality in php userland.
 *
 * @copyright  	Copyright (c) 2014 <info@techdivision.com> - TechDivision GmbH
 * @license    	http://www.php.net/license/3_01.txt
 *              PHP License (PHP 3_01)
 * @author      Johann Zelger <jz@appserver.io>
 */

#ifndef PHP_ROCKETS_H
#define PHP_ROCKETS_H

extern zend_module_entry rockets_module_entry;
#define phpext_rockets_ptr &rockets_module_entry

#define ROCKETS_NAME "${php.ext.name}"
#define ROCKETS_VERSION "${release.version}"

#ifdef PHP_WIN32
#    define PHP_ROCKETS_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#    define PHP_ROCKETS_API __attribute__ ((visibility("default")))
#else
#    define PHP_ROCKETS_API
#endif


#if PHP_VERSION_ID >= 50300
# define AS_SAPI_HEADER_OP_DC   , sapi_header_op_enum op
# define AS_SAPI_HEADER_OP_CC   , op
#else
# define AS_SAPI_HEADER_OP_DC
# define AS_SAPI_HEADER_OP_CC
#endif

#include "php.h"
#include "main/SAPI.h"

#include "zend.h"
#include "zend_API.h"
#include "zend_execute.h"
#include "zend_compile.h"
#include "zend_extensions.h"

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(rockets);
PHP_MSHUTDOWN_FUNCTION(rockets);
PHP_RINIT_FUNCTION(rockets);
PHP_RSHUTDOWN_FUNCTION(rockets);
PHP_MINFO_FUNCTION(rockets);

PHP_FUNCTION(rockets_test);
PHP_FUNCTION(rockets_socket);
PHP_FUNCTION(rockets_bind);
PHP_FUNCTION(rockets_listen);
PHP_FUNCTION(rockets_accept);
PHP_FUNCTION(rockets_close);
PHP_FUNCTION(rockets_setsockopt);
PHP_FUNCTION(rockets_getsockopt);
PHP_FUNCTION(rockets_recv);
PHP_FUNCTION(rockets_send);
PHP_FUNCTION(rockets_SSL_new);
PHP_FUNCTION(rockets_SSL_set_fd);
PHP_FUNCTION(rockets_SSL_CTX_new);
PHP_FUNCTION(rockets_SSL_CTX_set_options);
PHP_FUNCTION(rockets_SSL_CTX_use_certificate_file);
PHP_FUNCTION(rockets_SSL_CTX_use_PrivateKey_file);


ZEND_BEGIN_MODULE_GLOBALS(rockets)
	unsigned long	counter;
ZEND_END_MODULE_GLOBALS(rockets)

PHPAPI ZEND_EXTERN_MODULE_GLOBALS(apd)

#ifdef ZTS
#define ROCKETS_GLOBALS(v) TSRMG(rockets_globals_id, zend_rockets_globals*, v)
#else
#define ROCKETS_GLOBALS(v) (rockets_globals.v)
#endif

#define ROCKETS_TSRMLS_PARAM(param) (param) TSRMLS_CC

#endif    /* PHP_ROCKETS_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
