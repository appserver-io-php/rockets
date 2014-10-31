/**
 * rockets.c
 *
 * NOTICE OF LICENSE
 *
 * This source file is subject to the Open Software License (OSL 3.0)
 * that is available through the world-wide-web at this URL:
 * http://opensource.org/licenses/osl-3.0.php
 */

/**
 * It provides modern and fast sockets functionality
 *
 * @copyright  	Copyright (c) 2014 <info@techdivision.com> - TechDivision GmbH
 * @license    	http://opensource.org/licenses/osl-3.0.php
 *              Open Software License (OSL 3.0)
 * @author      Johann Zelger <jz@appserver.io>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "php_rockets.h"
#include "TSRM.h"
#include "SAPI.h"

#include "zend_API.h"
#include "zend_hash.h"
#include "zend_alloc.h"
#include "zend_operators.h"
#include "zend_globals.h"
#include "zend_compile.h"
#include "zend_constants.h"
#include "zend_closures.h"
#include <netinet/in.h>
#include <assert.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>

ZEND_DECLARE_MODULE_GLOBALS(rockets)

static int le_rockets;

const zend_function_entry rockets_functions[] = {
    PHP_FE(rockets_test, NULL)
	PHP_FE(rockets_server, NULL)
	PHP_FE(rockets_accept, NULL)
	PHP_FE(rockets_close, NULL)
    PHP_FE_END
};

zend_module_entry rockets_module_entry = {
    STANDARD_MODULE_HEADER,
    ROCKETS_NAME,
	rockets_functions,
    PHP_MINIT(rockets),
    PHP_MSHUTDOWN(rockets),
    PHP_RINIT(rockets),
    PHP_RSHUTDOWN(rockets),
    PHP_MINFO(rockets),
    ROCKETS_VERSION,
    STANDARD_MODULE_PROPERTIES
};

PHP_INI_BEGIN()
PHP_INI_ENTRY("rockets.test", "", PHP_INI_ALL, NULL)
PHP_INI_END()

#ifdef COMPILE_DL_ROCKETS
ZEND_GET_MODULE(rockets)
#endif

static void php_rockets_shutdown_globals (zend_rockets_globals *rockets_globals TSRMLS_DC)
{

}

static void php_rockets_init_globals(zend_rockets_globals *rockets_globals)
{

}

PHP_MSHUTDOWN_FUNCTION(rockets)
{
    UNREGISTER_INI_ENTRIES();
#ifdef ZTS
    ts_free_id(rockets_globals_id);
#else
    php_rockets_shutdown_globals(&rockets_globals TSRMLS_CC);
#endif
    return SUCCESS;
}

PHP_MINIT_FUNCTION(rockets)
{
    REGISTER_INI_ENTRIES();

	/* init globals */
    ZEND_INIT_MODULE_GLOBALS(rockets, php_rockets_init_globals, NULL);

    return SUCCESS;
}

PHP_RINIT_FUNCTION(rockets)
{
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(rockets)
{
    return SUCCESS;
}

PHP_MINFO_FUNCTION(rockets)
{
	php_info_print_table_start();
    php_info_print_table_header(2, "rockets", "enabled");
    php_info_print_table_row(2, "Version", ROCKETS_VERSION);
    php_info_print_table_end();

    DISPLAY_INI_ENTRIES();
}

/* {{{ proto boolean rockets_test()
 	 	 Returns true */
PHP_FUNCTION(rockets_test)
{
	RETURN_TRUE;
}

/* {{{ proto int rockets_server()
 	 	 Returns the listen fd */
PHP_FUNCTION(rockets_server)
{
	int listenfd = 0;
	struct sockaddr_in serv_addr;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(5000);

	bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	listen(listenfd, 10);

	RETURN_LONG((long)listenfd);
}

/* {{{ proto int rockets_accept()
 	 	 Returns the accepted fd */
PHP_FUNCTION(rockets_accept)
{
	long listenfd;
	int fd = 0;

	// parse params
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &listenfd) == FAILURE) {
	    return;
	}

	fd = accept((int)listenfd, (struct sockaddr*)NULL, NULL);
	RETURN_LONG((long)fd);
}

/* {{{ proto boolean rockets_close()
 	 	 Returns if the fd was closed or not */
PHP_FUNCTION(rockets_close)
{
	long fd;

	// parse params
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &fd) == FAILURE) {
	    return;
	}

	if (close((int)fd)) {
		RETURN_TRUE;
	} else  {
		RETURN_FALSE;
	}
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
