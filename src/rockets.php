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


$serverFd = rockets_server();

$clientFd = rockets_accept($serverFd);

var_dump(rockets_close($clientFd));
var_dump(rockets_close($serverFd));

var_dump($clientFd);