/**
 * rockets.c
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
	PHP_FE(rockets_getsockopt, NULL)
	PHP_FE(rockets_recv, NULL)
	PHP_FE(rockets_send, NULL)
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
    php_info_print_table_header(2, "rockets support", "enabled");
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
	int fd = 0, type, domain, protocol;
	long arg1, arg2, arg3 = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll|l", &arg1, &arg2, &arg3) == FAILURE) {
		return;
	}
	type = (int)arg1;
	domain = (int)arg2;
	if (arg3 != NULL) {
		protocol = arg3;
	}

	RETURN_LONG((long)socket(type, domain, protocol));
}

/* {{{ proto int rockets_bind()
		Give the socket FD the local address ADDR (which is LEN bytes long).  */
PHP_FUNCTION(rockets_bind)
{
	char ip[INET_ADDRSTRLEN];
	int fd, ip_len, port, family = AF_INET;
    struct sockaddr_in sin = { 0 };

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lsl|l", &fd, &ip, &ip_len, &port, &family) == FAILURE) {
		return;
	}

    sin.sin_family = family;
    sin.sin_port = htons(port);
    inet_pton(AF_INET, ip, &(sin.sin_addr));

    RETURN_LONG((long)bind(fd, (struct sockaddr*)&sin, sizeof(sin)));
}

/* {{{ proto int rockets_listen()
		Prepare to accept connections on socket FD.
		N connection requests will be queued before further requests are refused.
		Returns 0 on success, -1 for errors.  */
PHP_FUNCTION(rockets_listen)
{
	long arg1, arg2 = NULL;
	int fd, backlog = 128;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l|l", &arg1, &arg2) == FAILURE) {
		return;
	}
	fd = (int)arg1;
	if (arg2 != NULL) {
		backlog = (int)arg2;
	}

	RETURN_LONG((long)listen(fd, backlog));
}

/* {{{ proto int rockets_accept()
		Await a connection on socket FD.
		When a connection arrives, open a new socket to communicate with it,
		set *ADDR (which is *ADDR_LEN bytes long) to the address of the connecting
		peer and *ADDR_LEN to the address's actual length, and return the
		new socket's descriptor, or -1 for errors. */
PHP_FUNCTION(rockets_accept)
{
	long arg1;
	int listenfd = 0, connfd = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &arg1) == FAILURE) {
		return;
	}
	listenfd = (int)arg1;

	connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
	RETURN_LONG((long)connfd);
}

/* {{{ proto boolean rockets_close()
		Close the file descriptor FD.
		This function is a cancellation point and therefore not marked with */
PHP_FUNCTION(rockets_close)
{
	long arg1;
	int fd;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &arg1) == FAILURE) {
	    return;
	}
	fd = (int)arg1;
	if (close(fd)) {
		RETURN_TRUE;
	} else  {
		RETURN_FALSE;
	}
}

/* {{{ proto boolean rockets_setsockopt()
		Set socket FD's option OPTNAME at protocol level LEVEL
		to *OPTVAL (which is OPTLEN bytes long).
		Returns 0 on success, -1 for errors.  */
PHP_FUNCTION(rockets_setsockopt)
{
	long arg1, arg2, arg3;
	char *arg4, opt_val;
	int fd, level, opt, arg4_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "llls", &arg1, &arg2, &arg3, &arg4, &arg4_len) == FAILURE) {
		return;
	}

	fd = (int)arg1;
	level = (int)arg2;
	opt = (int)arg3;

	if (setsockopt(fd, level, opt, &arg4, &arg4_len)) {
		RETURN_FALSE;
	} else  {
		RETURN_TRUE;
	}
}

/* {{{ proto boolean rockets_getsockopt()
		Put the current value for socket FD's option OPTNAME at protocol level LEVEL
		into OPTVAL (which is *OPTLEN bytes long), and set *OPTLEN to the value's
		actual length.  Returns 0 on success, -1 for errors.  */
PHP_FUNCTION(rockets_getsockopt)
{
	char* opt_val;
	int opt_val_len, fd, level, opt;
	long arg1, arg2, arg3;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lll", &arg1, &arg2, &arg3) == FAILURE) {
		return;
	}

	fd = (int)arg1;
	level = (int)arg2;
	opt = (int)arg3;

	getsockopt(fd, level, opt, &opt_val, &opt_val_len);

	RETURN_LONG((long)opt_val);
	//RETURN_STRINGL(opt_val, opt_val_len, 0);
}

/* {{{ proto boolean rockets_recv()
		Read N bytes into BUF from socket FD.
		Returns the number read or -1 for errors. */
PHP_FUNCTION(rockets_recv)
{
	int fd, byte_count, flags = 0, recv_len = 128;
	long arg1, arg2 = NULL, arg3 = NULL;
	char buf[4096];

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l|ll", &arg1, &arg2, &arg3) == FAILURE) {
		return;
	}

	fd = (int)arg1;
	if (arg2 != NULL) {
		recv_len = (int)arg2;
	}
	if (arg3 != NULL) {
		flags = (int)arg3;
	}

	bzero(buf, sizeof(buf));
	byte_count = recv(fd, buf, sizeof buf, flags);

	RETURN_STRINGL(buf, byte_count, 0);
}

/* {{{ proto boolean rockets_send()
		Send N bytes of BUF to socket FD.  Returns the number sent or -1.*/
PHP_FUNCTION(rockets_send)
{
	int fd, byte_count, flags = 0, recv_len = 128, arg2_len;
	long arg1, arg3 = NULL;
	char *arg2;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ls|l", &arg1, &arg2, &arg2_len, &arg3) == FAILURE) {
		return;
	}

	fd = (int)arg1;
	if (arg3 != NULL) {
		flags = (int)arg3;
	}

	byte_count = send(fd, arg2, arg2_len, flags);
	RETURN_LONG((long)byte_count);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
