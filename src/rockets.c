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

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

ZEND_DECLARE_MODULE_GLOBALS(rockets)

static int le_rockets;

int rockets_counter[1024];
char rockets_strs[255][256] = {"foo", "bar", "bletch"};
zval *rockets_global_zval;

HashTable rockets_global_ht;

const zend_function_entry rockets_functions[] = {
	PHP_FE(rockets_test, NULL)
	PHP_FE(rockets_socket, NULL)
	PHP_FE(rockets_bind, NULL)
	PHP_FE(rockets_listen, NULL)
	PHP_FE(rockets_accept, NULL)
	PHP_FE(rockets_close, NULL)
	PHP_FE(rockets_setsockopt, NULL)
	PHP_FE(rockets_getsockopt, NULL)
	PHP_FE(rockets_recv, NULL)
	PHP_FE(rockets_send, NULL)
	PHP_FE(rockets_SSL_new, NULL)
	PHP_FE(rockets_SSL_set_fd, NULL)
	PHP_FE(rockets_SSL_CTX_new, NULL)
	PHP_FE(rockets_SSL_CTX_set_options, NULL)
	PHP_FE(rockets_SSL_CTX_use_certificate_file, NULL)
	PHP_FE(rockets_SSL_CTX_use_PrivateKey_file, NULL)
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

static void php_rockets_globals_dtor(zend_rockets_globals *rockets_globals TSRMLS_DC)
{

}

static void php_rockets_globals_ctor(zend_rockets_globals *rockets_globals)
{
	rockets_globals->counter = 0;
}

PHP_MSHUTDOWN_FUNCTION(rockets)
{
	UNREGISTER_INI_ENTRIES();
#ifdef ZTS
	ts_free_id(rockets_globals_id);
#else
	php_rockets_globals_dtor(&rockets_globals TSRMLS_CC);
#endif

	return SUCCESS;
}

PHP_MINIT_FUNCTION(rockets)
{
	REGISTER_INI_ENTRIES();
	// init globals
	ZEND_INIT_MODULE_GLOBALS(rockets, php_rockets_globals_ctor, NULL);

	SSL_load_error_strings();
	SSL_library_init();
	OpenSSL_add_all_algorithms();


	ALLOC_INIT_ZVAL(rockets_global_zval);

	Z_TYPE_P(rockets_global_zval) = IS_LONG;
	Z_LVAL_P(rockets_global_zval) = 1234;

	rockets_counter[1] = 112;

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

PHP_FUNCTION(rockets_test)
{
	*return_value = *rockets_global_zval;
}

/* {{{ proto int rockets_socket()
		Create a new socket of type TYPE in domain DOMAIN, using
		protocol PROTOCOL. If PROTOCOL is zero, one is chosen automatically.
		Returns a file descriptor for the new socket, or -1 for errors.  */
PHP_FUNCTION(rockets_socket)
{
	long arg1, arg2, arg3 = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll|l", &arg1, &arg2, &arg3) == FAILURE) {
		return;
	}

	RETURN_LONG((long)socket((int)arg1, (int)arg2, (int)arg3));
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
	long arg1, arg2 = 0;
	int backlog = 128;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l|l", &arg1, &arg2) == FAILURE) {
		return;
	}
	if (arg2 > 0) {
		backlog = (int)arg2;
	}

	RETURN_LONG((long)listen((int)arg1, backlog));
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

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &arg1) == FAILURE) {
		return;
	}

	RETURN_LONG((long)accept((int)arg1, (struct sockaddr*)NULL, NULL));
}

/* {{{ proto boolean rockets_close()
		Close the file descriptor FD.
		This function is a cancellation point and therefore not marked with */
PHP_FUNCTION(rockets_close)
{
	long arg1;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &arg1) == FAILURE) {
		return;
	}

	RETURN_LONG((long)close((int)arg1));
}

/* {{{ proto boolean rockets_setsockopt()
		Set socket FD's option OPTNAME at protocol level LEVEL
		to *OPTVAL (which is OPTLEN bytes long).
		Returns 0 on success, -1 for errors. }}} */
PHP_FUNCTION(rockets_setsockopt)
{
	long arg1, arg2, arg3, arg4;
	int optVal;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "llll", &arg1, &arg2, &arg3, &arg4) == FAILURE) {
		return;
	}
	optVal = (int)optVal;

	RETURN_LONG((long)setsockopt((int)arg1, (int)arg2, (int)arg3, &optVal, sizeof(optVal)));
}

/* {{{ proto boolean rockets_getsockopt()
		Put the current value for socket FD's option OPTNAME at protocol level LEVEL
		into OPTVAL (which is *OPTLEN bytes long), and set *OPTLEN to the value's
		actual length.  Returns 0 on success, -1 for errors. }}} */
PHP_FUNCTION(rockets_getsockopt)
{
	void* opt_val;
	int opt_val_len;
	long arg1, arg2, arg3;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lll", &arg1, &arg2, &arg3) == FAILURE) {
		return;
	}

	getsockopt((int)arg1, (int)arg2, (int)arg3, &opt_val, &opt_val_len);

	RETURN_LONG((long)opt_val);
}

/* {{{ proto boolean rockets_recv()
		Read N bytes into BUF from socket FD.
		Returns the number read or -1 for errors. }}} */
PHP_FUNCTION(rockets_recv)
{
	int byte_count, flags = 0, recv_len = 128;
	long arg1, arg2 = 128, arg3 = 0;
	char buf[8192];

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l|ll", &arg1, &arg2, &arg3) == FAILURE) {
		return;
	}

	bzero(buf, sizeof(buf));
	byte_count = recv((int)arg1, buf, (int)arg2, (int)arg3);

	RETURN_STRINGL(buf, byte_count, 0);
}

/* {{{ proto boolean rockets_send()
		Send N bytes of BUF to socket FD.  Returns the number sent or -1. }}}	*/
PHP_FUNCTION(rockets_send)
{
	int byte_count, flags = 0, arg2_len;
	long arg1, arg3 = 128;
	char *arg2;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ls|l", &arg1, &arg2, &arg2_len, &arg3) == FAILURE) {
		return;
	}

	RETURN_LONG((long)send((int)arg1, arg2, arg2_len, (int)arg3));
}

PHP_FUNCTION(rockets_SSL_new)
{

}

PHP_FUNCTION(rockets_SSL_set_fd)
{

}

PHP_FUNCTION(rockets_SSL_CTX_new)
{

}

PHP_FUNCTION(rockets_SSL_CTX_set_options)
{

}

PHP_FUNCTION(rockets_SSL_CTX_use_certificate_file)
{

}

PHP_FUNCTION(rockets_SSL_CTX_use_PrivateKey_file)
{

}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
