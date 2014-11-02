<?php

/**
 * rockets.php
 *
 * NOTICE OF LICENSE
 *
 * This source file is subject to the Open Software License (OSL 3.0)
 * that is available through the world-wide-web at this URL:
 * http://opensource.org/licenses/osl-3.0.php
 */

/**
 * A php test script show functionality of the extension
 *
 * @copyright  	Copyright (c) 2014 <info@techdivision.com> - TechDivision GmbH
 * @license    	http://opensource.org/licenses/osl-3.0.php
 *              Open Software License (OSL 3.0)
 * @author      Johann Zelger <jz@appserver.io>
 */

// open server socket
echo "rockets_socket ";
var_dump(
    $serverFd = rockets_socket(AF_INET, SOCK_STREAM, SOL_TCP)
);


echo "socket get option SO_REUSEADDR";
var_dump(
    rockets_getsockopt($serverFd, SOL_SOCKET, SO_REUSEADDR)
);
echo "socket get option TCP_NODELAY";
var_dump(
    rockets_getsockopt($serverFd, SOL_TCP, TCP_NODELAY)
);

echo "socket set option ";
// socket set options
var_dump(
    rockets_setsockopt($serverFd, SOL_SOCKET, SO_REUSEADDR, true)
);
var_dump(
    rockets_setsockopt($serverFd, SOL_TCP, TCP_NODELAY, true)
);

echo "socket get option SO_REUSEADDR";
var_dump(
    rockets_getsockopt($serverFd, SOL_SOCKET, SO_REUSEADDR)
);
echo "socket get option TCP_NODELAY";
var_dump(
    rockets_getsockopt($serverFd, SOL_TCP, TCP_NODELAY)
);

echo "rockets_bind ";
// bind server socket to local address and port
var_dump(
    rockets_bind($serverFd, '0.0.0.0', 5556, AF_INET)
);

echo "rockets_listen ";
// listen for connections with backlog of 1024
var_dump(
    rockets_listen($serverFd, 1024)
);

$httpResponseMessage = <<<EOD
HTTP/1.1 200 OK
Server: Rockets/0.1.0
Content-Length: 0
Content-Type: text/html;charset=UTF-8

EOD;

define('HTTP_RESPONSE_MESSAGE', $httpResponseMessage);

class Acceptor extends Thread
{
    public function __construct($fd)
    {
        $this->fd = $fd;
    }

    public function run()
    {
        while (1) {
            if ($clientFd = rockets_accept($this->fd)) {
                rockets_recv($clientFd);
                rockets_send($clientFd, HTTP_RESPONSE_MESSAGE);
                rockets_close($clientFd);
            }
        }
    }
}

$acceptors = [];

for ($i = 0; $i < 16; $i++) {
    $acceptor = new Acceptor($serverFd);
    $acceptor->start();
    $acceptors[$acceptor];
}