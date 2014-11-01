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

while (1) {
    echo "rockets_accept ";
    var_dump(
        rockets_accept($serverFd)
    );
}