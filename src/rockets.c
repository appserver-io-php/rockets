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
    PHP_FE(rockets_socket, NULL)
	PHP_FE(rockets_bind, NULL)
	PHP_FE(rockets_listen, NULL)
	PHP_FE(rockets_accept, NULL)
	PHP_FE(rockets_close, NULL)
	PHP_FE(rockets_setsockopt, NULL)
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

/* {{{ proto int rockets_socket()
		Create a new socket of type TYPE in domain DOMAIN, using
		protocol PROTOCOL.  If PROTOCOL is zero, one is chosen automatically.
		Returns a file descriptor for the new socket, or -1 for errors.  */
PHP_FUNCTION(rockets_socket)
{
	// init vars
	int fd = 0;
	long type;
	long domain;
	long protocol = 0;
	// parse params
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll|l", &type, &domain, &protocol) == FAILURE) {
		return;
	}
	// return fd
	RETURN_LONG((long)socket(type, domain, protocol));
}

/* {{{ proto int rockets_bind()
		Give the socket FD the local address ADDR (which is LEN bytes long).  */
PHP_FUNCTION(rockets_bind)
{
	int fd;
	char ip[INET_ADDRSTRLEN];
	int ip_len;
	int port;
	int family = AF_INET;
    struct sockaddr_in sin = { 0 };
    // parse params
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lsl|l", &fd, &ip, &ip_len, &port, &family) == FAILURE) {
		return;
	}
    // build up server address
    sin.sin_family = family;
    sin.sin_port = htons(port);
    inet_pton(AF_INET, ip, &(sin.sin_addr));
    // bind it
    RETURN_LONG((long)bind(fd, (struct sockaddr*)&sin, sizeof(sin)));
}

/* {{{ proto int rockets_listen()
		Prepare to accept connections on socket FD.
		N connection requests will be queued before further requests are refused.
		Returns 0 on success, -1 for errors.  */
PHP_FUNCTION(rockets_listen)
{
	int fd;
	int backlog = 128;
	int backlog_len;
	// parse params
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l|s", &fd, &backlog, &backlog_len) == FAILURE) {
		return;
	}
	// start listening
	RETURN_LONG(listen(fd, backlog));
}

/* {{{ proto int rockets_accept()
		Await a connection on socket FD.
		When a connection arrives, open a new socket to communicate with it,
		set *ADDR (which is *ADDR_LEN bytes long) to the address of the connecting
		peer and *ADDR_LEN to the address's actual length, and return the
		new socket's descriptor, or -1 for errors. */
PHP_FUNCTION(rockets_accept)
{
	int listenfd;
	// parse params
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &listenfd) == FAILURE) {
		return;
	}
	// accept connections
	RETURN_LONG((long)accept((int)listenfd, (struct sockaddr*)NULL, NULL));
}

/* {{{ proto boolean rockets_close()
 	 	 Returns if the fd was closed or not */
PHP_FUNCTION(rockets_close)
{
	int fd;

	// parse params
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &fd) == FAILURE) {
	    return;
	}

	if (close(fd)) {
		RETURN_TRUE;
	} else  {
		RETURN_FALSE;
	}
}

PHP_FUNCTION(rockets_setsockopt)
{
	int fd;
	int level;
	int val;

	// parse params
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lll", &fd, &level, &val) == FAILURE) {
	    return;
	}

	if (setsockopt(fd, SOL_SOCKET, level, &val, sizeof val) < 0) {
		RETURN_FALSE;
	} else  {
		RETURN_TRUE;
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
